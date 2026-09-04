#pragma once

#include <QObject>
#include <QProcess>

class UpdateChecker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateAvailableChanged)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateAvailableChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(bool checking READ checking NOTIFY checkingChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorOccurred)

public:
    explicit UpdateChecker(QObject *parent = nullptr);

    bool updateAvailable() const;
    QString latestVersion() const;
    QString currentVersion() const;
    bool checking() const;
    int downloadProgress() const;
    QString errorMessage() const;

    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void downloadAndInstall();

Q_SIGNALS:
    void updateAvailableChanged();
    void checkingChanged();
    void downloadProgressChanged();
    void errorOccurred();
    void updateInstalled();

private:
    static bool isNewerVersion(const QString &current, const QString &latest);
    void installPackage(const QString &packagePath);

    QProcess *m_checkProcess;
    QProcess *m_downloadProcess;

    bool m_updateAvailable;
    bool m_checking;
    int m_downloadProgress;
    QString m_latestVersion;
    QString m_downloadUrl;
    QString m_packageName;
    QString m_errorMessage;

    static constexpr const char *kRepoUrl = "https://api.github.com/repos/HOSIN2461/omafiles";
    static constexpr const char *kCurrentVersion = "0.4.11";
};
