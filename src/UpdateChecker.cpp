#include "UpdateChecker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_checkProcess(new QProcess(this))
    , m_downloadProcess(new QProcess(this))
    , m_updateAvailable(false)
    , m_checking(false)
    , m_downloadProgress(0)
{
    connect(m_checkProcess, &QProcess::readyReadStandardOutput,
            this, [this] {
        const QByteArray output = m_checkProcess->readAllStandardOutput();
        const QJsonDocument doc = QJsonDocument::fromJson(output);
        if (!doc.isObject()) {
            m_errorMessage = tr("Invalid response from GitHub");
            m_checking = false;
            Q_EMIT checkingChanged();
            Q_EMIT errorOccurred();
            return;
        }

        const QJsonObject release = doc.object();
        const QString tagName = release.value("tag_name").toString();

        if (tagName.isEmpty()) {
            m_errorMessage = tr("No release found");
            m_checking = false;
            Q_EMIT checkingChanged();
            Q_EMIT errorOccurred();
            return;
        }

        m_latestVersion = tagName;
        if (m_latestVersion.startsWith('v'))
            m_latestVersion = m_latestVersion.mid(1);

        m_downloadUrl.clear();
        const QJsonArray assets = release.value("assets").toArray();
        for (const QJsonValue &asset : assets) {
            const QJsonObject assetObj = asset.toObject();
            const QString name = assetObj.value("name").toString();
            if (name.endsWith(".pkg.tar.zst") && name.contains("omafiles")) {
                m_downloadUrl = assetObj.value("browser_download_url").toString();
                m_packageName = name;
                break;
            }
        }

        if (m_downloadUrl.isEmpty()) {
            m_errorMessage = tr("No package found in release");
            m_checking = false;
            Q_EMIT checkingChanged();
            Q_EMIT errorOccurred();
            return;
        }

        m_updateAvailable = isNewerVersion(QLatin1String(kCurrentVersion), m_latestVersion);
        m_checking = false;
        Q_EMIT checkingChanged();
        Q_EMIT updateAvailableChanged();
    });

    connect(m_checkProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) {
        if (exitCode != 0 && m_checking) {
            m_checking = false;
            m_errorMessage = QStringLiteral("curl: ") + m_checkProcess->readAllStandardError();
            Q_EMIT checkingChanged();
            Q_EMIT errorOccurred();
        }
    });

    connect(m_downloadProcess, &QProcess::readyReadStandardError,
            this, [this] {
        // curl -# writes progress to stderr
        const QByteArray err = m_downloadProcess->readAllStandardError();
        const QByteArrayList lines = err.split('\n');
        if (!lines.isEmpty()) {
            const QByteArray last = lines.last().trimmed();
            int pct = 0;
            if (last.size() > 0 && last[0] >= '0' && last[0] <= '9') {
                pct = last.mid(0, last.indexOf('%')).toInt();
                if (last.indexOf('%') != -1) {
                    pct = last.mid(0, last.indexOf('%')).toInt();
                }
            }
            if (pct > 0 && pct <= 100) {
                m_downloadProgress = pct;
                Q_EMIT downloadProgressChanged();
            }
        }
    });

    connect(m_downloadProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) {
        m_downloadProgress = 100;
        Q_EMIT downloadProgressChanged();

        if (exitCode != 0) {
            m_errorMessage = QStringLiteral("curl: ") + m_downloadProcess->readAllStandardError();
            Q_EMIT errorOccurred();
            return;
        }

        // Download succeeded, install the package
        const QString tmpDir = QDir::tempPath() + QStringLiteral("/omafiles-update");
        const QString packagePath = tmpDir + QLatin1Char('/') + m_packageName;
        installPackage(packagePath);
    });
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
    Q_EMIT checkingChanged();

    QStringList args;
    args << QStringLiteral("-sL")
         << QStringLiteral("-H") << QStringLiteral("Accept: application/vnd.github.v3+json")
         << QStringLiteral("%1/releases/latest").arg(QLatin1String(kRepoUrl));

    m_checkProcess->start(QStringLiteral("curl"), args);
}

void UpdateChecker::downloadAndInstall()
{
    if (m_downloadUrl.isEmpty())
        return;

    m_downloadProgress = 0;
    Q_EMIT downloadProgressChanged();

    const QDir tmpDir(QDir::tempPath() + QStringLiteral("/omafiles-update"));
    if (!tmpDir.exists())
        tmpDir.mkpath(tmpDir.absolutePath());

    const QString destFile = tmpDir.absoluteFilePath(m_packageName);

    QStringList args;
    args << QStringLiteral("-L")
         << QStringLiteral("-#")
         << QStringLiteral("--fail")
         << QStringLiteral("--create-dirs")
         << QStringLiteral("-o") << destFile
         << m_downloadUrl;

    m_downloadProcess->start(QStringLiteral("curl"), args);
}

void UpdateChecker::installPackage(const QString &packagePath)
{
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
            Q_EMIT errorOccurred();
        }
    });

    installProcess->start(QStringLiteral("pkexec"),
                          QStringList() << QStringLiteral("pacman") << QStringLiteral("-U")
                                        << QStringLiteral("--noconfirm") << packagePath);
}

bool UpdateChecker::isNewerVersion(const QString &current, const QString &latest)
{
    const QStringList curParts = current.split('.'); // e.g. ["0","1","0"]
    const QStringList latParts = latest.split('.');

    const int maxLen = qMax(curParts.size(), latParts.size());
    for (int i = 0; i < maxLen; ++i) {
        const int cur = i < curParts.size() ? curParts[i].toInt() : 0;
        const int lat = i < latParts.size() ? latParts[i].toInt() : 0;
        if (lat > cur)
            return true;
        if (lat < cur)
            return false;
    }
    return false;
}
