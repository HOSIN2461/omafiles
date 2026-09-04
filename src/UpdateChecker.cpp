#include "UpdateChecker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDir>

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_updateAvailable(false)
    , m_checking(false)
    , m_downloadProgress(0)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onCheckFinished);
}

bool UpdateChecker::updateAvailable() const
{
    return m_updateAvailable;
}

QString UpdateChecker::latestVersion() const
{
    return m_latestVersion;
}

QString UpdateChecker::currentVersion() const
{
    return QLatin1String(kCurrentVersion);
}

bool UpdateChecker::checking() const
{
    return m_checking;
}

int UpdateChecker::downloadProgress() const
{
    return m_downloadProgress;
}

QString UpdateChecker::errorMessage() const
{
    return m_errorMessage;
}

void UpdateChecker::checkForUpdates()
{
    if (m_checking)
        return;

    m_checking = true;
    m_errorMessage.clear();
    emit checkingChanged();

    const QUrl url(QString("%1/releases/latest").arg(QLatin1String(kRepoUrl)));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/vnd.github.v3+json");

    m_networkManager->get(request);
}

void UpdateChecker::onCheckFinished(QNetworkReply *reply)
{
    m_checking = false;
    emit checkingChanged();

    if (reply->error() != QNetworkReply::NoError) {
        m_errorMessage = reply->errorString();
        emit errorOccurred();
        reply->deleteLater();
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();

    if (!doc.isObject()) {
        m_errorMessage = tr("Invalid response from GitHub");
        emit errorOccurred();
        return;
    }

    const QJsonObject release = doc.object();
    const QString tagName = release.value("tag_name").toString();
    
    if (tagName.isEmpty()) {
        m_errorMessage = tr("No release found");
        emit errorOccurred();
        return;
    }

    // Extract version from tag (remove 'v' prefix if present)
    m_latestVersion = tagName;
    if (m_latestVersion.startsWith('v'))
        m_latestVersion = m_latestVersion.mid(1);

    // Find the .pkg.tar.zst asset
    const QJsonArray assets = release.value("assets").toArray();
    m_downloadUrl.clear();
    
    for (const QJsonValue &asset : assets) {
        const QJsonObject assetObj = asset.toObject();
        const QString name = assetObj.value("name").toString();
        if (name.endsWith(".pkg.tar.zst") && name.contains("omafiles")) {
            m_downloadUrl = assetObj.value("browser_download_url").toString();
            break;
        }
    }

    if (m_downloadUrl.isEmpty()) {
        m_errorMessage = tr("No package found in release");
        emit errorOccurred();
        return;
    }

    m_updateAvailable = isNewerVersion(QLatin1String(kCurrentVersion), m_latestVersion);
    emit updateAvailableChanged();

    if (!m_updateAvailable) {
        m_errorMessage.clear();
    }
}

void UpdateChecker::downloadAndInstall()
{
    if (m_downloadUrl.isEmpty())
        return;

    m_downloadProgress = 0;
    emit downloadProgressChanged();

    const QUrl url(m_downloadUrl);
    QNetworkRequest request(url);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &UpdateChecker::onDownloadProgress);
    connect(reply, &QNetworkAccessManager::finished,
            this, &UpdateChecker::onDownloadFinished);
}

void UpdateChecker::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        m_downloadProgress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        emit downloadProgressChanged();
    }
}

void UpdateChecker::onDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_errorMessage = reply->errorString();
        emit errorOccurred();
        reply->deleteLater();
        return;
    }

    // Create temp directory for the download
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        m_errorMessage = tr("Could not create temporary directory");
        emit errorOccurred();
        reply->deleteLater();
        return;
    }

    // Extract filename from URL
    const QString urlPath = m_downloadUrl;
    const QString filename = urlPath.section('/', -1);
    const QString packagePath = tempDir.filePath(filename);

    // Save the downloaded file
    QFile file(packagePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_errorMessage = tr("Could not save package");
        emit errorOccurred();
        reply->deleteLater();
        return;
    }

    file.write(reply->readAll());
    file.close();
    reply->deleteLater();

    // Install the package
    installPackage(packagePath);
}

void UpdateChecker::installPackage(const QString &packagePath)
{
    // Use pkexec for graphical sudo prompt
    QProcess *installProcess = new QProcess(this);
    connect(installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, installProcess](int exitCode, QProcess::ExitStatus) {
        installProcess->deleteLater();
        
        if (exitCode == 0) {
            // Restart the application
            const QString appPath = QCoreApplication::applicationFilePath();
            QProcess::startDetached(appPath, QStringList());
            QCoreApplication::quit();
        } else {
            m_errorMessage = tr("Installation failed");
            emit errorOccurred();
        }
    });

    installProcess->start("pkexec", QStringList() << "pacman" << "-U" << "--noconfirm" << packagePath);
}
