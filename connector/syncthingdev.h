#ifndef DATA_SYNCTHINGDEV_H
#define DATA_SYNCTHINGDEV_H

#include "./global.h"

#include <c++utilities/chrono/datetime.h>

#include <QMetaType>
#include <QString>
#include <QStringList>

namespace Data {

enum class SyncthingDevStatus { Unknown, Disconnected, OwnDevice, Idle, Synchronizing, OutOfSync, Rejected };

QString statusString(SyncthingDevStatus status);

struct LIB_SYNCTHING_CONNECTOR_EXPORT SyncthingDev {
    SyncthingDev(const QString &id = QString(), const QString &name = QString());
    QString statusString() const;
    bool isConnected() const;
    const QString displayName() const;

    QString id;
    QString name;
    QStringList addresses;
    QString compression;
    QString certName;
    SyncthingDevStatus status = SyncthingDevStatus::Unknown;
    int progressPercentage = 0;
    int progressRate = 0;
    bool introducer = false;
    bool paused = false;
    std::uint64_t totalIncomingTraffic = 0;
    std::uint64_t totalOutgoingTraffic = 0;
    QString connectionAddress;
    QString connectionType;
    QString clientVersion;
    CppUtilities::DateTime lastSeen;
};

inline SyncthingDev::SyncthingDev(const QString &id, const QString &name)
    : id(id)
    , name(name)
{
}

inline bool SyncthingDev::isConnected() const
{
    switch (status) {
    case SyncthingDevStatus::Unknown:
    case SyncthingDevStatus::Disconnected:
    case SyncthingDevStatus::OwnDevice:
        return false;
    default:
        return true;
    }
}

inline const QString SyncthingDev::displayName() const
{
    return name.isEmpty() ? id : name;
}

} // namespace Data

Q_DECLARE_METATYPE(Data::SyncthingDev)

#endif // DATA_SYNCTHINGDEV_H
