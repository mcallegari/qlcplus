//==============================================================================
// Sig-Net Protocol Framework - TLV Payload Construction Implementation
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
// Description:  Implementation of TLV encoding for Sig-Net application payloads.
//               Supports TID_LEVEL, TID_PRIORITY, TID_SYNC with network byte
//               order (big-endian) encoding.
//==============================================================================

#include "sig-net-tlv.hpp"
#include <string.h>

namespace SigNet {
namespace TLV {

//------------------------------------------------------------------------------
// Encode Generic TLV Block
//------------------------------------------------------------------------------
int32_t EncodeTLV(
    PacketBuffer& buffer,
    const TLVBlock& tlv
) {
    if (!tlv.value && tlv.length > 0) {
        return SIGNET_ERROR_INVALID_ARG;
    }
    
    int32_t result;
    
    // Write Type ID (2 bytes, network byte order)
    result = buffer.WriteUInt16(tlv.type_id);
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    
    // Write Length (2 bytes, network byte order)
    result = buffer.WriteUInt16(tlv.length);
    if (result != SIGNET_SUCCESS) {
        return result;
    }
    
    // Write Value (if length > 0)
    if (tlv.length > 0) {
        result = buffer.WriteBytes(tlv.value, tlv.length);
        if (result != SIGNET_SUCCESS) {
            return result;
        }
    }
    
    return SIGNET_SUCCESS;
}

//------------------------------------------------------------------------------
// Encode TID_LEVEL (DMX Level Data)
//------------------------------------------------------------------------------
int32_t EncodeTID_LEVEL(
    PacketBuffer& buffer,
    const uint8_t* dmx_data,
    uint16_t slot_count
) {
    if (!dmx_data) {
        return SIGNET_ERROR_INVALID_ARG;
    }
    
    if (slot_count < 1 || slot_count > MAX_DMX_SLOTS) {
        return SIGNET_ERROR_INVALID_ARG;
    }
    
    TLVBlock tlv(TID_LEVEL, slot_count, dmx_data);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Encode TID_PRIORITY (Priority Data)
//------------------------------------------------------------------------------
int32_t EncodeTID_PRIORITY(
    PacketBuffer& buffer,
    const uint8_t* priority_data,
    uint16_t slot_count
) {
    if (!priority_data) {
        return SIGNET_ERROR_INVALID_ARG;
    }
    
    if (slot_count < 1 || slot_count > MAX_DMX_SLOTS) {
        return SIGNET_ERROR_INVALID_ARG;
    }
    
    // Note: We don't validate that each priority value is 0-200 here
    // for performance reasons. The caller is responsible for ensuring
    // valid priority values per E1.31-1 specification.
    
    TLVBlock tlv(TID_PRIORITY, slot_count, priority_data);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Encode TID_SYNC (Synchronization Trigger)
//------------------------------------------------------------------------------
int32_t EncodeTID_SYNC(
    PacketBuffer& buffer
) {
    TLVBlock tlv(TID_SYNC, 0, 0);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Encode TID_POLL (Manager poll request)
//------------------------------------------------------------------------------
int32_t EncodeTID_POLL(
    PacketBuffer& buffer,
    const uint8_t* manager_tuid,
    uint16_t mfg_code,
    uint16_t product_variant_id,
    const uint8_t* tuid_lo,
    const uint8_t* tuid_hi,
    uint16_t target_endpoint,
    uint8_t query_level
) {
    if (!manager_tuid || !tuid_lo || !tuid_hi) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    if (query_level > QUERY_EXTENDED) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    uint8_t value[25];
    memcpy(&value[0], manager_tuid, TUID_LENGTH);

    uint32_t soem_code = (static_cast<uint32_t>(mfg_code) << 16) |
                         static_cast<uint32_t>(product_variant_id);
    value[6] = static_cast<uint8_t>((soem_code >> 24) & 0xFF);
    value[7] = static_cast<uint8_t>((soem_code >> 16) & 0xFF);
    value[8] = static_cast<uint8_t>((soem_code >> 8) & 0xFF);
    value[9] = static_cast<uint8_t>(soem_code & 0xFF);

    memcpy(&value[10], tuid_lo, TUID_LENGTH);
    memcpy(&value[16], tuid_hi, TUID_LENGTH);

    value[22] = static_cast<uint8_t>((target_endpoint >> 8) & 0xFF);
    value[23] = static_cast<uint8_t>(target_endpoint & 0xFF);
    value[24] = query_level;

    TLVBlock tlv(TID_POLL, 25, value);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Encode TID_POLL_REPLY (Startup Announce)
//------------------------------------------------------------------------------
int32_t EncodeTID_POLL_REPLY(
    PacketBuffer& buffer,
    const uint8_t* tuid,
    uint16_t mfg_code,
    uint16_t product_variant_id,
    uint16_t change_count
) {
    if (!tuid) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    uint8_t value[12];
    memcpy(value, tuid, TUID_LENGTH);

    uint32_t soem_code = (static_cast<uint32_t>(mfg_code) << 16) |
                         static_cast<uint32_t>(product_variant_id);
    value[6] = static_cast<uint8_t>((soem_code >> 24) & 0xFF);
    value[7] = static_cast<uint8_t>((soem_code >> 16) & 0xFF);
    value[8] = static_cast<uint8_t>((soem_code >> 8) & 0xFF);
    value[9] = static_cast<uint8_t>(soem_code & 0xFF);

    value[10] = static_cast<uint8_t>((change_count >> 8) & 0xFF);
    value[11] = static_cast<uint8_t>(change_count & 0xFF);

    TLVBlock tlv(TID_POLL_REPLY, 12, value);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Encode TID_RT_PROTOCOL_VERSION (Startup Announce)
//------------------------------------------------------------------------------
int32_t EncodeTID_RT_PROTOCOL_VERSION(
    PacketBuffer& buffer,
    uint8_t protocol_version
) {
    TLVBlock tlv(TID_RT_PROTOCOL_VERSION, 1, &protocol_version);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Encode TID_RT_FIRMWARE_VERSION (Startup Announce)
//------------------------------------------------------------------------------
int32_t EncodeTID_RT_FIRMWARE_VERSION(
    PacketBuffer& buffer,
    uint16_t machine_version_id,
    const char* version_string
) {
    if (!version_string) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    uint32_t str_len = static_cast<uint32_t>(strlen(version_string));
    if (str_len > 64) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    uint8_t value[68];
    uint32_t machine_version = static_cast<uint32_t>(machine_version_id);
    value[0] = static_cast<uint8_t>((machine_version >> 24) & 0xFF);
    value[1] = static_cast<uint8_t>((machine_version >> 16) & 0xFF);
    value[2] = static_cast<uint8_t>((machine_version >> 8) & 0xFF);
    value[3] = static_cast<uint8_t>(machine_version & 0xFF);

    if (str_len > 0) {
        memcpy(&value[4], version_string, str_len);
    }

    uint16_t tlv_len = static_cast<uint16_t>(4 + str_len);
    TLVBlock tlv(TID_RT_FIRMWARE_VERSION, tlv_len, value);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Encode TID_RT_ROLE_CAPABILITY (Startup Announce)
//------------------------------------------------------------------------------
int32_t EncodeTID_RT_ROLE_CAPABILITY(
    PacketBuffer& buffer,
    uint8_t role_capability_bits
) {
    TLVBlock tlv(TID_RT_ROLE_CAPABILITY, 1, &role_capability_bits);
    return EncodeTLV(buffer, tlv);
}

//------------------------------------------------------------------------------
// Build DMX Payload (TID_LEVEL)
//------------------------------------------------------------------------------
int32_t BuildDMXLevelPayload(
    PacketBuffer& payload,
    const uint8_t* dmx_data,
    uint16_t slot_count
) {
    payload.Reset();
    return EncodeTID_LEVEL(payload, dmx_data, slot_count);
}

//------------------------------------------------------------------------------
// Build Startup Announce Payload (Section 10.2.5)
//------------------------------------------------------------------------------
int32_t BuildStartupAnnouncePayload(
    PacketBuffer& payload,
    const uint8_t* tuid,
    uint16_t mfg_code,
    uint16_t product_variant_id,
    uint16_t firmware_version_id,
    const char* firmware_version_string,
    uint8_t protocol_version,
    uint8_t role_capability_bits,
    uint16_t change_count
) {
    if (!tuid || !firmware_version_string) {
        return SIGNET_ERROR_INVALID_ARG;
    }

    payload.Reset();

    int32_t result = EncodeTID_POLL_REPLY(payload, tuid, mfg_code, product_variant_id, change_count);
    if (result != SIGNET_SUCCESS) {
        return result;
    }

    result = EncodeTID_RT_FIRMWARE_VERSION(payload, firmware_version_id, firmware_version_string);
    if (result != SIGNET_SUCCESS) {
        return result;
    }

    result = EncodeTID_RT_PROTOCOL_VERSION(payload, protocol_version);
    if (result != SIGNET_SUCCESS) {
        return result;
    }

    return EncodeTID_RT_ROLE_CAPABILITY(payload, role_capability_bits);
}

//------------------------------------------------------------------------------
// Build Poll Payload (single TID_POLL TLV)
//------------------------------------------------------------------------------
int32_t BuildPollPayload(
    PacketBuffer& payload,
    const uint8_t* manager_tuid,
    uint16_t mfg_code,
    uint16_t product_variant_id,
    const uint8_t* tuid_lo,
    const uint8_t* tuid_hi,
    uint16_t target_endpoint,
    uint8_t query_level
) {
    payload.Reset();
    return EncodeTID_POLL(
        payload,
        manager_tuid,
        mfg_code,
        product_variant_id,
        tuid_lo,
        tuid_hi,
        target_endpoint,
        query_level
    );
}

//------------------------------------------------------------------------------
// Build Payload with Multiple TLV Blocks
//------------------------------------------------------------------------------
int32_t BuildPayload(
    PacketBuffer& buffer,
    const TLVBlock* tlv_blocks,
    uint16_t count
) {
    if (!tlv_blocks && count > 0) {
        return SIGNET_ERROR_INVALID_ARG;
    }
    
    for (uint16_t i = 0; i < count; ++i) {
        int32_t result = EncodeTLV(buffer, tlv_blocks[i]);
        if (result != SIGNET_SUCCESS) {
            return result;
        }
    }
    
    return SIGNET_SUCCESS;
}

} // namespace TLV
} // namespace SigNet
