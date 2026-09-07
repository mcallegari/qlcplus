/*
  Q Light Controller Plus
  snowprovisioner.cpp
*/

#include "snowprovisioner.h"

#include <QRandomGenerator>
#include <QTcpSocket>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#include "sig-net-constants.hpp"
#include "snowcodec.h"

namespace
{
QString sslError()
{
    const unsigned long code = ERR_get_error();
    if (code == 0) return QStringLiteral("Unknown TLS error");
    char buffer[256];
    ERR_error_string_n(code, buffer, sizeof(buffer));
    return QString::fromLatin1(buffer);
}

bool writeAll(SSL* ssl, QTcpSocket& socket, const QByteArray& data, QString* error)
{
    int offset = 0;
    while (offset < data.size())
    {
        const int result = SSL_write(ssl, data.constData() + offset, data.size() - offset);
        if (result > 0) { offset += result; continue; }
        const int sslResult = SSL_get_error(ssl, result);
        if (sslResult == SSL_ERROR_WANT_READ && socket.waitForReadyRead(5000)) continue;
        if (sslResult == SSL_ERROR_WANT_WRITE && socket.waitForBytesWritten(5000)) continue;
        if (error) *error = sslError();
        return false;
    }
    return true;
}

bool readOne(SSL* ssl, QTcpSocket& socket, QByteArray& buffer, SigNetSnow::CoapMessage& message, QString* error)
{
    for (;;)
    {
        if (SigNetSnow::parseMessage(buffer, message, error)) return true;
        char incoming[2048];
        const int result = SSL_read(ssl, incoming, sizeof(incoming));
        if (result > 0) { buffer.append(incoming, result); continue; }
        const int sslResult = SSL_get_error(ssl, result);
        if (sslResult == SSL_ERROR_WANT_READ && socket.waitForReadyRead(5000)) continue;
        if (sslResult == SSL_ERROR_WANT_WRITE && socket.waitForBytesWritten(5000)) continue;
        if (error && error->isEmpty()) *error = sslError();
        return false;
    }
}

QByteArray peerPublicKey(SSL* ssl)
{
    X509* certificate = SSL_get1_peer_certificate(ssl);
    if (!certificate) return QByteArray();
    EVP_PKEY* key = X509_get_pubkey(certificate);
    X509_free(certificate);
    if (!key) return QByteArray();
    const int size = i2d_PUBKEY(key, nullptr);
    QByteArray der(size, Qt::Uninitialized);
    unsigned char* output = reinterpret_cast<unsigned char*>(der.data());
    i2d_PUBKEY(key, &output);
    EVP_PKEY_free(key);
    return der;
}

void appendKey(QList<SigNetPacketizer::TLV>& tlvs, quint16 type, const QByteArray& key)
{
    if (!key.isEmpty()) tlvs.append({ type, key });
}

QByteArray derPublicKey(const QByteArray& encoded)
{
    const unsigned char* input = reinterpret_cast<const unsigned char*>(encoded.constData());
    EVP_PKEY* key = d2i_PUBKEY(nullptr, &input, encoded.size());
    if (!key || input != reinterpret_cast<const unsigned char*>(encoded.constData()) + encoded.size())
    {
        EVP_PKEY_free(key);
        return QByteArray();
    }
    const int size = i2d_PUBKEY(key, nullptr);
    QByteArray der(size, Qt::Uninitialized);
    unsigned char* output = reinterpret_cast<unsigned char*>(der.data());
    i2d_PUBKEY(key, &output);
    EVP_PKEY_free(key);
    return der;
}
}

bool SnowProvisioner::fetchPublicKey(const QHostAddress& address, quint16 port, QByteArray& publicKey, QString* error)
{
    publicKey.clear();
    if (address.isNull() || port == 0)
    {
        if (error) *error = QStringLiteral("The SNOW device address or TLS port is unavailable.");
        return false;
    }

    QTcpSocket socket;
    socket.connectToHost(address, port);
    if (!socket.waitForConnected(5000))
    {
        if (error) *error = socket.errorString();
        return false;
    }

    SSL_CTX* context = SSL_CTX_new(TLS_client_method());
    if (!context) { if (error) *error = sslError(); return false; }
    SSL_CTX_set_min_proto_version(context, TLS1_2_VERSION);
    SSL_CTX_set_verify(context, SSL_VERIFY_NONE, nullptr);
    SSL* ssl = SSL_new(context);
    if (!ssl || SSL_set_fd(ssl, int(socket.socketDescriptor())) != 1)
    {
        if (error) *error = sslError();
        SSL_free(ssl); SSL_CTX_free(context); return false;
    }

    bool connected = false;
    for (;;)
    {
        const int handshake = SSL_connect(ssl);
        if (handshake == 1) { connected = true; break; }
        const int sslResult = SSL_get_error(ssl, handshake);
        if (sslResult == SSL_ERROR_WANT_READ && socket.waitForReadyRead(5000)) continue;
        if (sslResult == SSL_ERROR_WANT_WRITE && socket.waitForBytesWritten(5000)) continue;
        break;
    }
    if (!connected)
    {
        if (error) *error = sslError();
        SSL_free(ssl); SSL_CTX_free(context); return false;
    }

    const QByteArray certificateKey = peerPublicKey(ssl);
    if (certificateKey.isEmpty())
    {
        if (error) *error = QStringLiteral("The SNOW device did not present a TLS certificate public key.");
        SSL_shutdown(ssl); SSL_free(ssl); SSL_CTX_free(context); return false;
    }

    // The certificate key is immediately available. A compliant peer also sends
    // TOTW_RT_PUBLIC_KEY after the handshake; when it is DER we verify it is
    // the same key. Raw encodings are permitted by SNOW and are represented by
    // the canonical certificate SPKI in the UI.
    QString transportError;
    if (writeAll(ssl, socket, SigNetSnow::buildCsm(), &transportError))
    {
        QByteArray receiveBuffer;
        SigNetSnow::CoapMessage message;
        bool receivedTunnelKey = false;
        for (int i = 0; i < 4 && !receivedTunnelKey && readOne(ssl, socket, receiveBuffer, message, &transportError); ++i)
        {
            for (const SigNetPacketizer::TLV& tlv : message.tlvs)
            {
                if (tlv.type != SigNet::TOTW_RT_PUBLIC_KEY)
                    continue;
                const QByteArray tunnelKey = derPublicKey(tlv.value);
                if (!tunnelKey.isEmpty() && tunnelKey != certificateKey)
                {
                    if (error) *error = QStringLiteral("The TLS certificate and TOTW public keys do not match.");
                    SSL_shutdown(ssl); SSL_free(ssl); SSL_CTX_free(context); return false;
                }
                receivedTunnelKey = true;
                break;
            }
        }
    }

    publicKey = certificateKey;
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(context);
    return true;
}

bool SnowProvisioner::provision(const SnowProvisioningRequest& request, QString* error)
{
    if (request.address.isNull() || request.port == 0 || request.trustedPublicKey.isEmpty() ||
        request.localTuid.size() != SigNet::TUID_LENGTH || request.pomPublicKey.isEmpty())
    {
        if (error) *error = QStringLiteral("The SNOW provisioning request is incomplete.");
        return false;
    }

    QTcpSocket socket;
    socket.connectToHost(request.address, request.port);
    if (!socket.waitForConnected(5000))
    {
        if (error) *error = socket.errorString();
        return false;
    }

    SSL_CTX* context = SSL_CTX_new(TLS_client_method());
    if (!context) { if (error) *error = sslError(); return false; }
    SSL_CTX_set_min_proto_version(context, TLS1_2_VERSION);
    SSL_CTX_set_verify(context, SSL_VERIFY_NONE, nullptr); // Identity is verified against the SNOW directory below.
    SSL* ssl = SSL_new(context);
    if (!ssl || SSL_set_fd(ssl, int(socket.socketDescriptor())) != 1)
    {
        if (error) *error = sslError();
        SSL_free(ssl); SSL_CTX_free(context); return false;
    }
    bool connected = false;
    for (;;)
    {
        const int handshake = SSL_connect(ssl);
        if (handshake == 1) { connected = true; break; }
        const int sslResult = SSL_get_error(ssl, handshake);
        if (sslResult == SSL_ERROR_WANT_READ && socket.waitForReadyRead(5000)) continue;
        if (sslResult == SSL_ERROR_WANT_WRITE && socket.waitForBytesWritten(5000)) continue;
        break;
    }
    if (!connected)
    {
        if (error) *error = sslError();
        SSL_free(ssl); SSL_CTX_free(context); return false;
    }
    if (peerPublicKey(ssl) != request.trustedPublicKey)
    {
        if (error) *error = QStringLiteral("The TLS certificate public key does not match the trusted SNOW identity.");
        SSL_shutdown(ssl); SSL_free(ssl); SSL_CTX_free(context); return false;
    }

    QString transportError;
    if (!writeAll(ssl, socket, SigNetSnow::buildCsm(), &transportError))
    {
        if (error) *error = transportError;
        SSL_free(ssl); SSL_CTX_free(context); return false;
    }

    QList<SigNetPacketizer::TLV> tlvs;
    tlvs.append({ SigNet::TOTW_RT_SCOPE, request.operationalScope.toUtf8() });
    if (request.roleCapability & SigNet::ROLE_CAP_NODE)
    {
        appendKey(tlvs, SigNet::TOTW_RT_KEY_KC, request.citizenKey);
        appendKey(tlvs, SigNet::TOTW_RT_KEY_KM_GLOBAL, request.managerGlobalKey);
        appendKey(tlvs, SigNet::TOTW_RT_KEY_KM_LOCAL, request.managerLocalKey);
    }
    if (request.roleCapability & SigNet::ROLE_CAP_SENDER)
    {
        appendKey(tlvs, SigNet::TOTW_RT_KEY_KS, request.senderKey);
        if (!(request.roleCapability & SigNet::ROLE_CAP_NODE)) appendKey(tlvs, SigNet::TOTW_RT_KEY_KC, request.citizenKey);
    }
    if (request.roleCapability & SigNet::ROLE_CAP_MANAGER)
    {
        appendKey(tlvs, SigNet::TOTW_RT_KEY_KS, request.senderKey);
        appendKey(tlvs, SigNet::TOTW_RT_KEY_KC, request.citizenKey);
        appendKey(tlvs, SigNet::TOTW_RT_KEY_KM_GLOBAL, request.managerGlobalKey);
    }
    tlvs.append({ SigNet::TOTW_RT_POM_PUBLIC_KEY, request.pomPublicKey });

    QByteArray token(4, Qt::Uninitialized);
    for (int i = 0; i < token.size(); ++i) token[i] = char(QRandomGenerator::global()->generate() & 0xff);
    const QByteArray packet = SigNetSnow::buildRequest(QStringLiteral("local"),
                                                        QStringList() << QStringLiteral("manager") << request.tuid << QStringLiteral("0"),
                                                        tlvs, request.localTuid, 0x7ff8, token);
    // 2,000 bytes leaves room for TLS record overhead below SNOW's 2,048-byte limit.
    if (packet.isEmpty() || packet.size() > 2000 || !writeAll(ssl, socket, packet, &transportError))
    {
        if (error) *error = packet.isEmpty() ? QStringLiteral("Unable to construct the SNOW key-delivery request.") :
                             packet.size() > 2000 ? QStringLiteral("The SNOW key-delivery request exceeds the TLS record limit.") : transportError;
        SSL_free(ssl); SSL_CTX_free(context); return false;
    }

    QByteArray receiveBuffer;
    SigNetSnow::CoapMessage response;
    do
    {
        if (!readOne(ssl, socket, receiveBuffer, response, &transportError))
        {
            if (error) *error = transportError;
            SSL_free(ssl); SSL_CTX_free(context); return false;
        }
    } while (response.code == 0xE1); // RFC 8323 CSM

    const bool committed = response.code == 0x44 && response.token == token; // 2.04 Changed
    if (!committed && error)
        *error = QStringLiteral("The device did not acknowledge the key commit (CoAP code 0x%1).").arg(response.code, 2, 16, QLatin1Char('0'));
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(context);
    return committed;
}
