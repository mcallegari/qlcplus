/*
  Q Light Controller Plus
  snowcodec.h
*/

#ifndef SNOWCODEC_H
#define SNOWCODEC_H

#include <QByteArray>
#include <QString>

#include "signetpacketizer.h"

namespace SigNetSnow
{
struct CoapMessage
{
    quint8 code = 0;
    QByteArray token;
    QString uri;
    QList<SigNetPacketizer::TLV> tlvs;
};

QByteArray buildCsm();
QByteArray buildRequest(const QString& scope,
                        const QStringList& resourceSegments,
                        const QList<SigNetPacketizer::TLV>& tlvs,
                        const QByteArray& localTuid,
                        quint16 manufacturerCode,
                        const QByteArray& token);
bool parseMessage(QByteArray& buffer, CoapMessage& message, QString* error = nullptr);
}

#endif
