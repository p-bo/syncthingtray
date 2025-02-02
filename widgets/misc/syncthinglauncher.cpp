#include "./syncthinglauncher.h"

#include "../settings/settings.h"

#include <QtConcurrentRun>

#include <algorithm>
#include <limits>

using namespace std;
using namespace std::placeholders;
using namespace CppUtilities;

namespace Data {

SyncthingLauncher *SyncthingLauncher::s_mainInstance = nullptr;

/*!
 * \class SyncthingLauncher
 * \brief The SyncthingLauncher class starts a Syncthing instance either as an external process or using a library version of Syncthing.
 * \remarks
 * - This is *not* strictly a singleton class. However, one instance is supposed to be the "main instance" (see SyncthingLauncher::setMainInstance()).
 * - A SyncthingLauncher instance can only launch one Syncthing instance at a time.
 * - Using Syncthing as library is still under development and must be explicitely enabled by setting the CMake variable USE_LIBSYNCTHING.
 */

/*!
 * \brief Constructs a new Syncthing launcher.
 */
SyncthingLauncher::SyncthingLauncher(QObject *parent)
    : QObject(parent)
    , m_manuallyStopped(true)
    , m_emittingOutput(false)
{
    connect(&m_process, &SyncthingProcess::readyRead, this, &SyncthingLauncher::handleProcessReadyRead);
    connect(&m_process, static_cast<void (SyncthingProcess::*)(int exitCode, QProcess::ExitStatus exitStatus)>(&SyncthingProcess::finished), this,
        &SyncthingLauncher::handleProcessFinished);
    connect(&m_process, &SyncthingProcess::stateChanged, this, &SyncthingLauncher::handleProcessStateChanged);
    connect(&m_process, &SyncthingProcess::errorOccurred, this, &SyncthingLauncher::errorOccurred);
    connect(&m_process, &SyncthingProcess::confirmKill, this, &SyncthingLauncher::confirmKill);
}

/*!
 * \brief Sets whether the output/log should be emitted via outputAvailable() signal.
 */
void SyncthingLauncher::setEmittingOutput(bool emittingOutput)
{
    if (m_emittingOutput == emittingOutput || !(m_emittingOutput = emittingOutput) || m_outputBuffer.isEmpty()) {
        return;
    }
    QByteArray data;
    m_outputBuffer.swap(data);
    emit outputAvailable(move(data));
}

/*!
 * \brief Returns whether the built-in Syncthing library is available.
 */
bool SyncthingLauncher::isLibSyncthingAvailable()
{
#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
    return true;
#else
    return false;
#endif
}

/*!
 * \brief Returns the Syncthing version provided by libsyncthing or "Not built with libsyncthing support." if not built with libsyncthing support.
 */
QString SyncthingLauncher::libSyncthingVersionInfo()
{
#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
    return QString::fromStdString(LibSyncthing::longSyncthingVersion());
#else
    return tr("Not built with libsyncthing support.");
#endif
}

/*!
 * \brief Launches a Syncthing instance using the specified \a arguments.
 *
 * To use the internal library, leave \a program empty. In this case \a arguments are ignored.
 * Otherwise \a program must be the path the external Syncthing executable.
 *
 * \remarks Does nothing if already running an instance.
 */
void SyncthingLauncher::launch(const QString &program, const QStringList &arguments)
{
    if (isRunning()) {
        return;
    }
    m_manuallyStopped = false;

    // start external process
    if (!program.isEmpty()) {
        m_process.startSyncthing(program, arguments);
        return;
    }

    // use libsyncthing
    m_future = QtConcurrent::run(this, &SyncthingLauncher::runLibSyncthing, LibSyncthing::RuntimeOptions{});
}

/*!
 * \brief Launches a Syncthing instance according to the specified \a launcherSettings.
 * \remarks Does nothing if already running an instance.
 */
void SyncthingLauncher::launch(const Settings::Launcher &launcherSettings)
{
    if (isRunning()) {
        return;
    }
    if (!launcherSettings.useLibSyncthing && launcherSettings.syncthingPath.isEmpty()) {
        emit errorOccurred(QProcess::FailedToStart);
        return;
    }
    if (launcherSettings.useLibSyncthing) {
        LibSyncthing::RuntimeOptions options;
        options.configDir = launcherSettings.libSyncthing.configDir.toStdString();
        launch(options);
    } else {
        launch(launcherSettings.syncthingPath, SyncthingProcess::splitArguments(launcherSettings.syncthingArgs));
    }
}

/*!
 * \brief Launches a Syncthing instance using the internal library with the specified \a runtimeOptions.
 * \remarks Does nothing if already running an instance.
 */
void SyncthingLauncher::launch(const LibSyncthing::RuntimeOptions &runtimeOptions)
{
    if (isRunning()) {
        return;
    }
    m_manuallyStopped = false;
    m_future = QtConcurrent::run(this, &SyncthingLauncher::runLibSyncthing, runtimeOptions);
}

void SyncthingLauncher::terminate()
{
    if (m_process.isRunning()) {
        m_manuallyStopped = true;
        m_process.stopSyncthing();
    } else {
        tearDownLibSyncthing();
    }
}

void SyncthingLauncher::kill()
{
    if (m_process.isRunning()) {
        m_manuallyStopped = true;
        m_process.killSyncthing();
    } else {
        tearDownLibSyncthing();
    }
}

void SyncthingLauncher::tearDownLibSyncthing()
{
#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
    if (!m_future.isRunning()) {
        return;
    }
    m_manuallyStopped = true;
    QtConcurrent::run(this, &SyncthingLauncher::stopLibSyncthing);
#endif
}

void SyncthingLauncher::handleProcessReadyRead()
{
    handleOutputAvailable(m_process.readAll());
}

void SyncthingLauncher::handleProcessStateChanged(QProcess::ProcessState newState)
{
    switch (newState) {
    case QProcess::NotRunning:
        emit runningChanged(false);
        break;
    case QProcess::Starting:
        emit runningChanged(true);
        break;
    default:;
    }
}

void SyncthingLauncher::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit exited(exitCode, exitStatus);
}

#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
static const char *const logLevelStrings[] = {
    "[DEBUG]   ",
    "[VERBOSE] ",
    "[INFO]    ",
    "[WARNING] ",
    "[FATAL]   ",
};
#endif

void SyncthingLauncher::handleLoggingCallback(LibSyncthing::LogLevel level, const char *message, size_t messageSize)
{
#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
    if (level < LibSyncthing::LogLevel::Info) {
        return;
    }
    QByteArray messageData;
    messageSize = min<size_t>(numeric_limits<int>::max() - 20, messageSize);
    messageData.reserve(static_cast<int>(messageSize) + 20);
    messageData.append(logLevelStrings[static_cast<int>(level)]);
    messageData.append(message, static_cast<int>(messageSize));
    messageData.append('\n');

    handleOutputAvailable(move(messageData));
#else
    CPP_UTILITIES_UNUSED(level)
    CPP_UTILITIES_UNUSED(message)
    CPP_UTILITIES_UNUSED(messageSize)
#endif
}

void SyncthingLauncher::handleOutputAvailable(QByteArray &&data)
{
    if (isEmittingOutput()) {
        emit outputAvailable(data);
    } else {
        m_outputBuffer += data;
    }
}

void SyncthingLauncher::runLibSyncthing(const LibSyncthing::RuntimeOptions &runtimeOptions)
{
#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
    LibSyncthing::setLoggingCallback(bind(&SyncthingLauncher::handleLoggingCallback, this, _1, _2, _3));
    emit runningChanged(true);
    const auto exitCode = LibSyncthing::runSyncthing(runtimeOptions);
    emit exited(static_cast<int>(exitCode), exitCode == 0 ? QProcess::NormalExit : QProcess::CrashExit);
    emit runningChanged(false);
#else
    CPP_UTILITIES_UNUSED(runtimeOptions)
    handleOutputAvailable(QByteArray("libsyncthing support not enabled"));
    emit exited(-1, QProcess::CrashExit);
#endif
}

void SyncthingLauncher::stopLibSyncthing()
{
#ifdef SYNCTHINGWIDGETS_USE_LIBSYNCTHING
    LibSyncthing::stopSyncthing();
    // no need to emit exited/runningChanged here; that is already done in runLibSyncthing()
#endif
}

} // namespace Data
