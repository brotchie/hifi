//
//  HifiSockAddr.cpp
//  hifi
//
//  Created by Stephen Birarda on 11/26/2013.
//  Copyright (c) 2013 HighFidelity, Inc. All rights reserved.
//

#include "HifiSockAddr.h"

#include <QtCore/QDataStream>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkInterface>

static int hifiSockAddrMetaTypeId = qMetaTypeId<HifiSockAddr>();

HifiSockAddr::HifiSockAddr() :
    _address(),
    _port(0)
{
    
}

HifiSockAddr::HifiSockAddr(const QHostAddress& address, quint16 port) :
    _address(address),
    _port(port)
{
    
}

HifiSockAddr::HifiSockAddr(const HifiSockAddr& otherSockAddr) {
    _address = otherSockAddr._address;
    _port = otherSockAddr._port;
}

HifiSockAddr::HifiSockAddr(const QString& hostname, quint16 hostOrderPort) {
    // lookup the IP by the hostname
    QHostInfo hostInfo = QHostInfo::fromName(hostname);
    foreach(const QHostAddress& address, hostInfo.addresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            _address = address;
            _port = hostOrderPort;
        }
    }
}

HifiSockAddr& HifiSockAddr::operator=(const HifiSockAddr& rhsSockAddr) {
    _address = rhsSockAddr._address;
    _port = rhsSockAddr._port;
    
    return *this;
}

void HifiSockAddr::swap(HifiSockAddr& otherSockAddr) {
    using std::swap;
    
    swap(_address, otherSockAddr._address);
    swap(_port, otherSockAddr._port);
}

bool HifiSockAddr::operator==(const HifiSockAddr& rhsSockAddr) const {
    return _address == rhsSockAddr._address && _port == rhsSockAddr._port;
}

QDebug operator<<(QDebug debug, const HifiSockAddr& sockAddr) {
    debug.nospace() << sockAddr._address.toString().toLocal8Bit().constData() << ":" << sockAddr._port;
    return debug.space();
}

QDataStream& operator<<(QDataStream& dataStream, const HifiSockAddr& sockAddr) {
    dataStream << sockAddr._address << sockAddr._port;
    return dataStream;
}

QDataStream& operator>>(QDataStream& dataStream, HifiSockAddr& sockAddr) {
    dataStream >> sockAddr._address >> sockAddr._port;
    return dataStream;
}

quint32 getHostOrderLocalAddress() {
    
    static int localAddress = 0;
    
    if (localAddress == 0) {
        foreach(const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
            if (interface.flags() & QNetworkInterface::IsUp
                && interface.flags() & QNetworkInterface::IsRunning
                && interface.flags() & ~QNetworkInterface::IsLoopBack) {
                // we've decided that this is the active NIC
                // enumerate it's addresses to grab the IPv4 address
                foreach(const QNetworkAddressEntry &entry, interface.addressEntries()) {
                    // make sure it's an IPv4 address that isn't the loopback
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.ip().isLoopback()) {
                        qDebug("Node's local address is %s", entry.ip().toString().toLocal8Bit().constData());
                        
                        // set our localAddress and break out
                        localAddress = entry.ip().toIPv4Address();
                        break;
                    }
                }
            }
            
            if (localAddress != 0) {
                break;
            }
        }
    }
    
    // return the looked up local address
    return localAddress;
}
