//==============================================================================
// Sig-Net Protocol Framework - Security Layer Implementation
//==============================================================================
//
// Copyright (c) 2026 Singularity (UK) Ltd.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//==============================================================================
// Author:       Wayne Howell
// Date:         March 28, 2026
// Description:  Implementation of Sig-Net custom CoAP options encoding and
//               HMAC-SHA256 signature calculation per Section 8.5 of spec.
//               Handles Security-Mode, Sender-ID, Mfg-Code, Session, Seq, HMAC.
//==============================================================================

#include "sig-net-security.hpp"

#include <QtEndian>

#include <string.h>

namespace
{
QByteArray buildHmacInputData(const QByteArray& uri,
                              const SigNet::SigNetOptions& options,
                              const uint8_t* payload,
                              uint16_t payloadLen)
{
    QByteArray data;
    data.reserve(uri.size() + 1 + SigNet::SENDER_ID_LENGTH + 2 + 4 + 4 + payloadLen);
    data.append(uri);
    data.append(char(options.security_mode));
    data.append(reinterpret_cast<const char*>(options.sender_id), SigNet::SENDER_ID_LENGTH);

    const quint16 mfgCode = qToBigEndian(options.mfg_code);
    data.append(reinterpret_cast<const char*>(&mfgCode), sizeof(mfgCode));

    const quint32 sessionId = qToBigEndian(options.session_id);
    data.append(reinterpret_cast<const char*>(&sessionId), sizeof(sessionId));

    const quint32 seqNum = qToBigEndian(options.seq_num);
    data.append(reinterpret_cast<const char*>(&seqNum), sizeof(seqNum));

    if (payloadLen > 0)
        data.append(reinterpret_cast<const char*>(payload), payloadLen);

    return data;
}
}

namespace SigNet {
namespace Security {

//------------------------------------------------------------------------------
// Build SigNet Custom Options (Without HMAC)
//------------------------------------------------------------------------------
int32_t BuildSigNetOptionsWithoutHMAC(
    PacketBuffer& buffer,
    const SigNetOptions& options,
    uint16_t prev_option
) {
    int32_t result;
    
    // Option 1: Security-Mode (2076) - 1 byte
    result = CoAP::EncodeCoAPOption(
        buffer,
        SIGNET_OPTION_SECURITY_MODE,
        prev_option,
        &options.security_mode,
        1
    );
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    prev_option = SIGNET_OPTION_SECURITY_MODE;
    
    // Option 2: Sender-ID (2108) - 8 bytes (TUID + endpoint)
    result = CoAP::EncodeCoAPOption(
        buffer,
        SIGNET_OPTION_SENDER_ID,
        prev_option,
        options.sender_id,
        SENDER_ID_LENGTH
    );
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    prev_option = SIGNET_OPTION_SENDER_ID;
    
    // Option 3: Mfg-Code (2140) - 2 bytes (network byte order)
    const quint16 mfgCode = qToBigEndian(options.mfg_code);
    
    result = CoAP::EncodeCoAPOption(
        buffer,
        SIGNET_OPTION_MFG_CODE,
        prev_option,
        reinterpret_cast<const uint8_t*>(&mfgCode),
        sizeof(mfgCode)
    );
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    prev_option = SIGNET_OPTION_MFG_CODE;
    
    // Option 4: Session-ID (2172) - 4 bytes (network byte order)
    const quint32 sessionId = qToBigEndian(options.session_id);
    
    result = CoAP::EncodeCoAPOption(
        buffer,
        SIGNET_OPTION_SESSION_ID,
        prev_option,
        reinterpret_cast<const uint8_t*>(&sessionId),
        sizeof(sessionId)
    );
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    prev_option = SIGNET_OPTION_SESSION_ID;
    
    // Option 5: Seq-Num (2204) - 4 bytes (network byte order)
    const quint32 seqNum = qToBigEndian(options.seq_num);
    
    result = CoAP::EncodeCoAPOption(
        buffer,
        SIGNET_OPTION_SEQ_NUM,
        prev_option,
        reinterpret_cast<const uint8_t*>(&seqNum),
        sizeof(seqNum)
    );
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    
    // Note: HMAC option (2236) is added later by CalculateAndEncodeHMAC()
    
    return SIGNET_SUCCESS;
}

//------------------------------------------------------------------------------
// Build HMAC Input Buffer (Section 8.5)
//------------------------------------------------------------------------------
int32_t BuildHMACInput(
    const char* uri_string,
    const SigNetOptions& options,
    const uint8_t* payload,
    uint16_t payload_len,
    uint8_t* output,
    uint32_t output_size,
    uint32_t* bytes_written
) {
    if (!uri_string || !payload || !output || !bytes_written) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    const QByteArray data = buildHmacInputData(QByteArray(uri_string), options, payload, payload_len);
    if (data.size() > static_cast<int>(output_size)) {
        return SIGNET_ERROR_BUFFER_FULL;
    }

    memcpy(output, data.constData(), data.size());
    *bytes_written = static_cast<uint32_t>(data.size());
    return SIGNET_SUCCESS;
}

//------------------------------------------------------------------------------
// Calculate and Encode HMAC Option
//------------------------------------------------------------------------------
int32_t CalculateAndEncodeHMAC(
    PacketBuffer& buffer,
    const char* uri_string,
    SigNetOptions& options,
    const uint8_t* payload,
    uint16_t payload_len,
    const uint8_t* sender_key,
    uint16_t prev_option
) {
    if (!uri_string || !payload || !sender_key) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    const QByteArray hmacInput = buildHmacInputData(QByteArray(uri_string), options, payload, payload_len);
    
    // Calculate HMAC-SHA256
    int32_t result = Crypto::HMAC_SHA256(
        sender_key,
        DERIVED_KEY_LENGTH,
        reinterpret_cast<const uint8_t*>(hmacInput.constData()),
        static_cast<uint32_t>(hmacInput.size()),
        options.hmac
    );
    
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    
    // Encode HMAC as option 2236
    result = CoAP::EncodeCoAPOption(
        buffer,
        SIGNET_OPTION_HMAC,
        prev_option,
        options.hmac,
        HMAC_SHA256_LENGTH
    );
    
    return result;
}

//------------------------------------------------------------------------------
// Build Sender-ID from TUID and Endpoint
//------------------------------------------------------------------------------
int32_t BuildSenderID(
    const uint8_t* tuid,
    uint16_t endpoint,
    uint8_t* sender_id
) {
    if (!tuid || !sender_id) {
        return SIGNET_ERROR_INVALID_ARG;
    }
    
    memcpy(sender_id, tuid, TUID_LENGTH);
    qToBigEndian(endpoint, sender_id + TUID_LENGTH);
    
    return SIGNET_SUCCESS;
}

} // namespace Security
} // namespace SigNet
