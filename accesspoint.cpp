/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "accesspoint.h"
#include "dbus/nm-access-pointinterface.h"
#include "manager_p.h"
#include "wirelessdevice.h"

#include "nmdebug.h"

namespace NetworkManager {

class AccessPointPrivate
{
public:
    AccessPointPrivate( const QString & path ) : iface( NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus()), capabilities(0), wpaFlags(0), rsnFlags(0), frequency(0), maxBitRate(0), mode((WirelessDevice::OperationMode)0), signalStrength(0)
    {
    }
    OrgFreedesktopNetworkManagerAccessPointInterface iface;
    QString uni;
    AccessPoint::Capabilities capabilities;
    AccessPoint::WpaFlags wpaFlags;
    AccessPoint::WpaFlags rsnFlags;
    QString ssid;
    QByteArray rawSsid;
    uint frequency;
    QString hardwareAddress;
    uint maxBitRate;
    WirelessDevice::OperationMode mode;
    int signalStrength;
};
}

NetworkManager::AccessPoint::AccessPoint( const QString& path, QObject * parent ) : QObject(parent), d_ptr(new AccessPointPrivate( path ))
{
    Q_D(AccessPoint);
    d->uni = path;
    if (d->iface.isValid()) {
        d->capabilities = convertCapabilities( d->iface.flags() );
        d->wpaFlags = convertWpaFlags( d->iface.wpaFlags() );
        d->rsnFlags = convertWpaFlags( d->iface.rsnFlags() );
        d->signalStrength = d->iface.strength();
        d->ssid = QString::fromUtf8(d->iface.ssid());
        d->rawSsid = d->iface.ssid();
        d->frequency = d->iface.frequency();
        d->hardwareAddress = d->iface.hwAddress();
        d->maxBitRate = d->iface.maxBitrate();
        // make this a static on WirelessNetworkInterface
        d->mode = WirelessDevice::convertOperationMode(d->iface.mode());
        connect( &d->iface, SIGNAL(PropertiesChanged(QVariantMap)),
                this, SLOT(propertiesChanged(QVariantMap)));
    }
}

NetworkManager::AccessPoint::~AccessPoint()
{
    Q_D(AccessPoint);
    delete d;
}

QString NetworkManager::AccessPoint::uni() const
{
    Q_D(const AccessPoint);
    return d->uni;
}

QString NetworkManager::AccessPoint::hardwareAddress() const
{
    Q_D(const AccessPoint);
    return d->hardwareAddress;
}

NetworkManager::AccessPoint::Capabilities NetworkManager::AccessPoint::capabilities() const
{
    Q_D(const AccessPoint);
    return d->capabilities;
}

NetworkManager::AccessPoint::WpaFlags NetworkManager::AccessPoint::wpaFlags() const
{
    Q_D(const AccessPoint);
    return d->wpaFlags;
}

NetworkManager::AccessPoint::WpaFlags NetworkManager::AccessPoint::rsnFlags() const
{
    Q_D(const AccessPoint);
    return d->rsnFlags;
}

QString NetworkManager::AccessPoint::ssid() const
{
    Q_D(const AccessPoint);
    return d->ssid;
}

QByteArray NetworkManager::AccessPoint::rawSsid() const
{
    Q_D(const AccessPoint);
    return d->rawSsid;
}

uint NetworkManager::AccessPoint::frequency() const
{
    Q_D(const AccessPoint);
    return d->frequency;
}

uint NetworkManager::AccessPoint::maxBitRate() const
{
    Q_D(const AccessPoint);
    return d->maxBitRate;
}

NetworkManager::WirelessDevice::OperationMode NetworkManager::AccessPoint::mode() const
{
    Q_D(const AccessPoint);
    return d->mode;
}

int NetworkManager::AccessPoint::signalStrength() const
{
    Q_D(const AccessPoint);
    return d->signalStrength;
}

void NetworkManager::AccessPoint::propertiesChanged(const QVariantMap &properties)
{
    Q_D(AccessPoint);
    QStringList propKeys = properties.keys();
    //nmDebug() << propKeys;
    QLatin1String flagsKey("Flags"),
                  wpaFlagsKey("WpaFlags"),
                  rsnFlagsKey("RsnFlags"),
                  ssidKey("Ssid"),
                  freqKey("Frequency"),
                  hwAddrKey("HwAddress"),
                  modeKey("Mode"),
                  maxBitRateKey("MaxBitrate"),
                  strengthKey("Strength");
    QVariantMap::const_iterator it = properties.find(flagsKey);
    if (it != properties.end()) {
        d->capabilities = convertCapabilities(it->toUInt());
        emit capabilitiesChanged(d->capabilities);
        propKeys.removeOne(flagsKey);
    }
    it = properties.find(wpaFlagsKey);
    if (it != properties.end()) {
        d->wpaFlags = convertWpaFlags(it->toUInt());
        emit wpaFlagsChanged(d->wpaFlags);
        propKeys.removeOne(wpaFlagsKey);
    }
    it = properties.find(rsnFlagsKey);
    if (it != properties.end()) {
        d->rsnFlags = convertWpaFlags(it->toUInt());
        emit rsnFlagsChanged(d->rsnFlags);
        propKeys.removeOne(rsnFlagsKey);
    }
    it = properties.find(ssidKey);
    if (it != properties.end()) {
        d->ssid = it->toByteArray();
        emit ssidChanged(d->ssid);
        propKeys.removeOne(ssidKey);
    }
    it = properties.find(freqKey);
    if (it != properties.end()) {
        d->frequency = it->toUInt();
        emit frequencyChanged(d->frequency);
        propKeys.removeOne(freqKey);
    }
    it = properties.find(hwAddrKey);
    if (it != properties.end()) {
        d->hardwareAddress = it->toString();
        propKeys.removeOne(hwAddrKey);
    }
    it = properties.find(modeKey);
    if (it != properties.end()) {
        d->mode = WirelessDevice::convertOperationMode(it->toUInt());
        propKeys.removeOne(modeKey);
    }
    it = properties.find(maxBitRateKey);
    if (it != properties.end()) {
        d->maxBitRate = it->toUInt();
        emit bitRateChanged(d->maxBitRate);
        propKeys.removeOne(maxBitRateKey);
    }
    it = properties.find(strengthKey);
    if (it != properties.end()) {
        d->signalStrength = it->toInt();
        //nmDebug() << "UNI: " << d->uni << "MAC: " << d->hardwareAddress << "SignalStrength: " << d->signalStrength;
        emit signalStrengthChanged(d->signalStrength);
        propKeys.removeOne(strengthKey);
    }
    if (propKeys.count()) {
        nmDebug() << "Unhandled properties: " << propKeys;
    }
}

NetworkManager::AccessPoint::Capabilities NetworkManager::AccessPoint::convertCapabilities(int caps)
{
    if ( 1 == caps ) {
        return NetworkManager::AccessPoint::Privacy;
    } else {
        return 0;
    }
}
// Copied from wireless.h
// /* Modes of operation */
#define IW_MODE_AUTO    0   /* Let the driver decides */
#define IW_MODE_ADHOC   1   /* Single cell network */
#define IW_MODE_INFRA   2   /* Multi cell network, roaming, ... */
#define IW_MODE_MASTER  3   /* Synchronization master or Access Point */
#define IW_MODE_REPEAT  4   /* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND  5   /* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR 6   /* Passive monitor (listen only) */

NetworkManager::AccessPoint::WpaFlags NetworkManager::AccessPoint::convertWpaFlags(uint theirFlags)
{
    return (NetworkManager::AccessPoint::WpaFlags)theirFlags;
}

#include "accesspoint.moc"
