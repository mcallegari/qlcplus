/*
  Q Light Controller Plus
  snowprovisioner.h
*/

#ifndef SNOWPROVISIONER_H
#define SNOWPROVISIONER_H

#include <QByteArray>
#include <QHostAddress>
#include <QString>

struct SnowProvisioningRequest
{
    QHostAddress address;
    quint16 port = 0;
    quint8 roleCapability = 0;
    QString tuid;
    QString operationalScope;
    QByteArray localTuid;
    QByteArray trustedPublicKey;
    QByteArray senderKey;
    QByteArray citizenKey;
    QByteArray managerGlobalKey;
    QByteArray managerLocalKey;
    QByteArray pomPublicKey;
};

class SnowProvisioner
{
public:
    static bool provision(const SnowProvisioningRequest& request, QString* error = nullptr);
    static bool fetchPublicKey(const QHostAddress& address, quint16 port, QByteArray& publicKey, QString* error = nullptr);
};

#endif
