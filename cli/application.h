#ifndef CLI_APPLICATION_H
#define CLI_APPLICATION_H

#include "./args.h"

#include "../connector/syncthingconnection.h"
#include "../connector/syncthingconnectionsettings.h"

#include <QObject>

#include <tuple>

namespace Cli {

enum class OperationType { Status, PauseResume, WaitForIdle };

struct RelevantDir {
    RelevantDir(const Data::SyncthingDir *dir = nullptr, const QString &subDir = QString());
    operator bool() const;
    void notifyAboutRescan() const;

    const Data::SyncthingDir *dirObj;
    QString subDir;
};

inline RelevantDir::RelevantDir(const Data::SyncthingDir *dir, const QString &subDir)
    : dirObj(dir)
    , subDir(subDir)
{
}

inline RelevantDir::operator bool() const
{
    return dirObj != nullptr;
}

class Application : public QObject {
    Q_OBJECT

public:
    Application();
    ~Application();

    int exec(int argc, const char *const *argv);

private slots:
    void handleStatusChanged(Data::SyncthingStatus newStatus);
    void handleResponse();
    void handleError(
        const QString &message, Data::SyncthingErrorCategory category, int networkError, const QNetworkRequest &request, const QByteArray &response);
    void findRelevantDirsAndDevs(OperationType operationType);
    bool findPwd();

private:
    int loadConfig();
    bool waitForConnected(int timeout = 2000);
    bool waitForConfig(int timeout = 2000);
    bool waitForConfigAndStatus(int timeout = 2000);
    void requestLog(const ArgumentOccurrence &);
    void requestShutdown(const ArgumentOccurrence &);
    void requestRestart(const ArgumentOccurrence &);
    void requestRescan(const ArgumentOccurrence &occurrence);
    void requestRescanAll(const ArgumentOccurrence &);
    void requestPauseResume(bool pause);
    void printDir(const RelevantDir &relevantDir) const;
    void printDev(const Data::SyncthingDev *dev) const;
    void printStatus(const ArgumentOccurrence &);
    static void printLog(const std::vector<Data::SyncthingLogEntry> &logEntries);
    void printConfig(const ArgumentOccurrence &);
    void editConfig(const ArgumentOccurrence &);
    QByteArray editConfigViaEditor() const;
    QByteArray editConfigViaScript() const;
    void waitForIdle(const ArgumentOccurrence &);
    bool checkWhetherIdle() const;
    void checkPwdOperationPresent(const ArgumentOccurrence &occurrence);
    void printPwdStatus(const ArgumentOccurrence &occurrence);
    void requestRescanPwd(const ArgumentOccurrence &occurrence);
    void requestPausePwd(const ArgumentOccurrence &occurrence);
    void requestResumePwd(const ArgumentOccurrence &occurrence);
    void initDirCompletion(Argument &arg, const ArgumentOccurrence &);
    void initDevCompletion(Argument &arg, const ArgumentOccurrence &);
    RelevantDir findDirectory(const QString &dirIdentifier);

    Args m_args;
    Data::SyncthingConnectionSettings m_settings;
    Data::SyncthingConnection m_connection;
    size_t m_expectedResponse;
    bool m_preventDisconnect;
    bool m_callbacksInvoked;
    bool m_requiresMainEventLoop;
    std::vector<RelevantDir> m_relevantDirs;
    std::vector<const Data::SyncthingDev *> m_relevantDevs;
    RelevantDir m_pwd;
    QByteArray m_dirCompletion;
    QByteArray m_devCompletion;
    int m_idleDuration;
    int m_idleTimeout;
    bool m_argsRead;
};

} // namespace Cli

#endif // CLI_APPLICATION_H
