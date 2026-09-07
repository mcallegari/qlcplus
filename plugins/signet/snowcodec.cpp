/*
  Q Light Controller Plus
  snowcodec.cpp
*/

#include "snowcodec.h"

#include <QtEndian>

#include "sig-net-constants.hpp"

namespace
{
bool appendOption(QByteArray& out, quint16 number, quint16& previous, const QByteArray& value)
{
    if (number < previous || value.size() > 65804)
        return false;
    const quint16 delta = number - previous;
    QByteArray extended;
    quint8 deltaNibble = 0;
    quint8 lengthNibble = 0;
    auto encode = [&extended](quint16 number, quint8& nibble) {
        if (number <= 12) { nibble = quint8(number); return; }
        if (number <= 268) { nibble = 13; extended.append(char(number - 13)); return; }
        nibble = 14;
        const quint16 bigEndian = qToBigEndian<quint16>(number - 269);
        extended.append(reinterpret_cast<const char*>(&bigEndian), 2);
    };
    encode(delta, deltaNibble);
    encode(quint16(value.size()), lengthNibble);
    out.append(char((deltaNibble << 4) | lengthNibble));
    out.append(extended);
    out.append(value);
    previous = number;
    return true;
}

void appendU16(QByteArray& out, quint16 value)
{
    const quint16 bigEndian = qToBigEndian(value);
    out.append(reinterpret_cast<const char*>(&bigEndian), sizeof(bigEndian));
}

bool readExtended(const QByteArray& data, int& pos, quint8 nibble, quint16& value)
{
    if (nibble <= 12) { value = nibble; return true; }
    if (nibble == 13 && pos < data.size()) { value = 13 + quint8(data.at(pos++)); return true; }
    if (nibble == 14 && pos + 1 < data.size())
    {
        value = 269 + qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(data.constData() + pos));
        pos += 2;
        return true;
    }
    return false;
}

QByteArray frame(quint8 code, const QByteArray& token, const QByteArray& optionsAndPayload)
{
    if (token.size() > 8 || optionsAndPayload.size() > 65804)
        return QByteArray();
    const quint16 length = optionsAndPayload.size();
    QByteArray result;
    quint8 lengthNibble = 0;
    if (length <= 12) lengthNibble = quint8(length);
    else if (length <= 268) { lengthNibble = 13; }
    else { lengthNibble = 14; }
    result.append(char((lengthNibble << 4) | token.size()));
    if (lengthNibble == 13) result.append(char(length - 13));
    else if (lengthNibble == 14)
    {
        const quint16 extended = qToBigEndian<quint16>(length - 269);
        result.append(reinterpret_cast<const char*>(&extended), 2);
    }
    result.append(char(code));
    result.append(token);
    result.append(optionsAndPayload);
    return result;
}
}

QByteArray SigNetSnow::buildCsm()
{
    return frame(0xE1, QByteArray(), QByteArray()); // 7.01 CSM
}

QByteArray SigNetSnow::buildRequest(const QString& scope,
                                    const QStringList& resourceSegments,
                                    const QList<SigNetPacketizer::TLV>& tlvs,
                                    const QByteArray& localTuid,
                                    quint16 manufacturerCode,
                                    const QByteArray& token)
{
    if (scope != QLatin1String(SigNet::SIGNET_URI_SCOPE_DEFAULT) || localTuid.size() != SigNet::TUID_LENGTH)
        return QByteArray();
    QByteArray options;
    quint16 previous = 0;
    const QStringList uriParts = QStringList() << QString(SigNet::SIGNET_URI_PREFIX)
                                                << QString(SigNet::SIGNET_URI_VERSION) << scope << resourceSegments;
    for (const QString& part : uriParts)
        if (!appendOption(options, SigNet::COAP_OPTION_URI_PATH, previous, part.toUtf8())) return QByteArray();

    QByteArray senderId = localTuid;
    senderId.append(2, '\0');
    QByteArray mfg; appendU16(mfg, manufacturerCode);
    QByteArray zero32(4, '\0');
    if (!appendOption(options, SigNet::SIGNET_OPTION_SECURITY_MODE, previous, QByteArray(1, char(SigNet::SECURITY_MODE_UNPROVISIONED))) ||
        !appendOption(options, SigNet::SIGNET_OPTION_SENDER_ID, previous, senderId) ||
        !appendOption(options, SigNet::SIGNET_OPTION_MFG_CODE, previous, mfg) ||
        !appendOption(options, SigNet::SIGNET_OPTION_SESSION_ID, previous, zero32) ||
        !appendOption(options, SigNet::SIGNET_OPTION_SEQ_NUM, previous, zero32) ||
        !appendOption(options, SigNet::SIGNET_OPTION_HMAC, previous, QByteArray())) return QByteArray();

    QByteArray payload;
    for (const auto& tlv : tlvs)
    {
        if (tlv.value.size() > 65535) return QByteArray();
        appendU16(payload, tlv.type);
        appendU16(payload, quint16(tlv.value.size()));
        payload.append(tlv.value);
    }
    if (!payload.isEmpty()) { options.append(char(0xFF)); options.append(payload); }
    return frame(0x02, token, options); // POST
}

bool SigNetSnow::parseMessage(QByteArray& buffer, CoapMessage& message, QString* error)
{
    message = CoapMessage();
    if (buffer.size() < 2) return false;
    int pos = 0;
    const quint8 header = quint8(buffer.at(pos++));
    quint16 length = 0;
    if (!readExtended(buffer, pos, header >> 4, length) || pos >= buffer.size()) return false;
    const int fullLength = pos + 1 + (header & 0x0F) + length;
    if (fullLength > buffer.size()) return false;
    message.code = quint8(buffer.at(pos++));
    const int tokenLength = header & 0x0F;
    if (tokenLength > 8 || pos + tokenLength > fullLength) { if (error) *error = QStringLiteral("Invalid CoAP/TCP token"); return false; }
    message.token = buffer.mid(pos, tokenLength); pos += tokenLength;
    quint16 previous = 0;
    QList<QByteArray> uriParts;
    while (pos < fullLength && quint8(buffer.at(pos)) != 0xFF)
    {
        const quint8 optionHeader = quint8(buffer.at(pos++));
        quint16 delta = 0, optionLength = 0;
        if (!readExtended(buffer, pos, optionHeader >> 4, delta) || !readExtended(buffer, pos, optionHeader & 0x0F, optionLength) ||
            pos + optionLength > fullLength) { if (error) *error = QStringLiteral("Malformed CoAP/TCP option"); return false; }
        previous += delta;
        const QByteArray value = buffer.mid(pos, optionLength); pos += optionLength;
        if (previous == SigNet::COAP_OPTION_URI_PATH) uriParts.append(value);
    }
    if (pos < fullLength && quint8(buffer.at(pos)) == 0xFF) ++pos;
    if (!uriParts.isEmpty()) message.uri = QStringLiteral("/") + QString::fromUtf8(uriParts.join('/'));
    while (pos < fullLength)
    {
        if (pos + 4 > fullLength) { if (error) *error = QStringLiteral("Malformed SNOW TLV"); return false; }
        const quint16 type = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(buffer.constData() + pos)); pos += 2;
        const quint16 valueLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(buffer.constData() + pos)); pos += 2;
        if (pos + valueLength > fullLength) { if (error) *error = QStringLiteral("Truncated SNOW TLV"); return false; }
        message.tlvs.append({ type, buffer.mid(pos, valueLength) }); pos += valueLength;
    }
    buffer.remove(0, fullLength);
    return true;
}
