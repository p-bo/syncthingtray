#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include "./global.h"

#include <c++utilities/misc/traits.h>

#include <QJsonValue>
#include <QStringList>
#include <QUrl>

#include <limits>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QJsonObject)
QT_FORWARD_DECLARE_CLASS(QHostAddress)

namespace CppUtilities {
class DateTime;
}

namespace Data {

namespace Traits = CppUtilities::Traits;

struct SyncthingStatistics;
struct SyncthingDir;
struct SyncthingDev;

QString LIB_SYNCTHING_CONNECTOR_EXPORT agoString(CppUtilities::DateTime dateTime);
QString LIB_SYNCTHING_CONNECTOR_EXPORT trafficString(std::uint64_t total, double rate);
QString LIB_SYNCTHING_CONNECTOR_EXPORT directoryStatusString(const Data::SyncthingStatistics &stats);
QString LIB_SYNCTHING_CONNECTOR_EXPORT syncCompleteString(
    const std::vector<const SyncthingDir *> &completedDirs, const SyncthingDev *remoteDevice = nullptr);
QString LIB_SYNCTHING_CONNECTOR_EXPORT rescanIntervalString(int rescanInterval, bool fileSystemWatcherEnabled);
bool LIB_SYNCTHING_CONNECTOR_EXPORT isLocal(const QString &hostName);
bool LIB_SYNCTHING_CONNECTOR_EXPORT isLocal(const QString &hostName, const QHostAddress &hostAddress);
bool LIB_SYNCTHING_CONNECTOR_EXPORT setDirectoriesPaused(QJsonObject &syncthingConfig, const QStringList &dirIds, bool paused);
bool LIB_SYNCTHING_CONNECTOR_EXPORT setDevicesPaused(QJsonObject &syncthingConfig, const QStringList &dirs, bool paused);

/*!
 * \brief Returns whether the host specified by the given \a url is the local machine.
 */
inline bool isLocal(const QUrl &url)
{
    return isLocal(url.host());
}

template <typename IntType = quint64, Traits::EnableIf<std::is_integral<IntType>> * = nullptr>
inline IntType LIB_SYNCTHING_CONNECTOR_EXPORT jsonValueToInt(const QJsonValue &value, double defaultValue = 0.0)
{
    return static_cast<IntType>(value.toDouble(defaultValue));
}

constexpr int LIB_SYNCTHING_CONNECTOR_EXPORT trQuandity(quint64 quandity)
{
    return quandity > std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : static_cast<int>(quandity);
}

template <class Objects, class Accessor> QStringList LIB_SYNCTHING_CONNECTOR_EXPORT things(const Objects &objects, Accessor accessor)
{
    QStringList things;
    things.reserve(objects.size());
    for (const auto &object : objects) {
        things << accessor(object);
    }
    return things;
}

template <class Objects> QStringList LIB_SYNCTHING_CONNECTOR_EXPORT ids(const Objects &objects)
{
    return things(objects, [](const auto &object) { return Traits::dereferenceMaybe(object).id; });
}

template <class Objects> QStringList LIB_SYNCTHING_CONNECTOR_EXPORT displayNames(const Objects &objects)
{
    return things(objects, [](const auto &object) { return Traits::dereferenceMaybe(object).displayName(); });
}

} // namespace Data

#endif // DATA_UTILS_H
