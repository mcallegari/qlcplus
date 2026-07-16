/*
  Qt-backed Sig-Net crypto glue for QLC+
*/

#include "sig-net-crypto.hpp"

#include <cstring>

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>

namespace
{
int hexNibble(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'a' && ch <= 'f')
        return 10 + (ch - 'a');
    if (ch >= 'A' && ch <= 'F')
        return 10 + (ch - 'A');
    return -1;
}
}

namespace SigNet {
namespace Crypto {

int32_t HMAC_SHA256(const uint8_t* key, uint32_t key_len, const uint8_t* message, uint32_t msg_len, uint8_t* output)
{
    if (!key || !message || !output)
        return SIGNET_ERROR_INVALID_ARG;

    const QByteArray result = QMessageAuthenticationCode::hash(
        QByteArray(reinterpret_cast<const char*>(message), msg_len),
        QByteArray(reinterpret_cast<const char*>(key), key_len),
        QCryptographicHash::Sha256);

    if (result.size() != int(HMAC_SHA256_LENGTH))
        return SIGNET_ERROR_CRYPTO;

    memcpy(output, result.constData(), HMAC_SHA256_LENGTH);
    return SIGNET_SUCCESS;
}

int32_t HKDF_Expand(const uint8_t* prk, uint32_t prk_len, const uint8_t* info, uint32_t info_len, uint8_t* output)
{
    if (!prk || !info || !output)
        return SIGNET_ERROR_INVALID_ARG;

    QByteArray message(reinterpret_cast<const char*>(info), info_len);
    message.append(char(HKDF_COUNTER_T1));
    return HMAC_SHA256(prk, prk_len,
                       reinterpret_cast<const uint8_t*>(message.constData()),
                       message.size(),
                       output);
}

int32_t DeriveSenderKey(const uint8_t* k0, uint8_t* sender_key)
{
    return HKDF_Expand(k0, DERIVED_KEY_LENGTH,
                       reinterpret_cast<const uint8_t*>(HKDF_INFO_SENDER),
                       strlen(HKDF_INFO_SENDER),
                       sender_key);
}

int32_t DeriveCitizenKey(const uint8_t* k0, uint8_t* citizen_key)
{
    return HKDF_Expand(k0, DERIVED_KEY_LENGTH,
                       reinterpret_cast<const uint8_t*>(HKDF_INFO_CITIZEN),
                       strlen(HKDF_INFO_CITIZEN),
                       citizen_key);
}

int32_t DeriveManagerGlobalKey(const uint8_t* k0, uint8_t* manager_global_key)
{
    return HKDF_Expand(k0, DERIVED_KEY_LENGTH,
                       reinterpret_cast<const uint8_t*>(HKDF_INFO_MANAGER_GLOBAL),
                       strlen(HKDF_INFO_MANAGER_GLOBAL),
                       manager_global_key);
}

void TUID_ToHexString(const uint8_t* tuid, char* hex_output, size_t hex_string_size)
{
    static const char digits[] = "0123456789ABCDEF";
    if (!tuid || !hex_output || hex_string_size < (TUID_HEX_LENGTH + 1))
        return;

    for (uint32_t i = 0; i < TUID_LENGTH; ++i)
    {
        hex_output[i * 2] = digits[(tuid[i] >> 4) & 0x0F];
        hex_output[(i * 2) + 1] = digits[tuid[i] & 0x0F];
    }
    hex_output[TUID_HEX_LENGTH] = '\0';
}

int32_t TUID_FromHexString(const char* hex_string, uint8_t* tuid)
{
    if (!hex_string || !tuid)
        return SIGNET_ERROR_INVALID_ARG;

    if (strlen(hex_string) != TUID_HEX_LENGTH)
        return SIGNET_ERROR_INVALID_ARG;

    for (uint32_t i = 0; i < TUID_LENGTH; ++i)
    {
        const int high = hexNibble(hex_string[i * 2]);
        const int low = hexNibble(hex_string[(i * 2) + 1]);
        if (high < 0 || low < 0)
            return SIGNET_ERROR_INVALID_ARG;
        tuid[i] = uint8_t((high << 4) | low);
    }

    return SIGNET_SUCCESS;
}

int32_t DeriveManagerLocalKey(const uint8_t* k0, const uint8_t* tuid, uint8_t* manager_local_key)
{
    if (!k0 || !tuid || !manager_local_key)
        return SIGNET_ERROR_INVALID_ARG;

    char tuidHex[TUID_HEX_LENGTH + 1];
    TUID_ToHexString(tuid, tuidHex, sizeof(tuidHex));

    QByteArray info(HKDF_INFO_MANAGER_LOCAL_PREFIX);
    info.append(tuidHex);
    return HKDF_Expand(k0, DERIVED_KEY_LENGTH,
                       reinterpret_cast<const uint8_t*>(info.constData()),
                       info.size(),
                       manager_local_key);
}

int32_t GenerateRandomK0(uint8_t* k0_output)
{
    if (!k0_output)
        return SIGNET_ERROR_INVALID_ARG;

    for (uint32_t i = 0; i < K0_KEY_LENGTH; ++i)
        k0_output[i] = uint8_t(QRandomGenerator::global()->bounded(256));

    return SIGNET_SUCCESS;
}

} // namespace Crypto
} // namespace SigNet
