//==============================================================================
// Sig-Net Protocol Framework - TLV Payload Construction
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
// Description:  Type-Length-Value (TLV) encoding for application payloads.
//               Supports TID_LEVEL, TID_PRIORITY, TID_SYNC. Network byte order.
//               Format: 2-byte Type | 2-byte Length | Variable Value data.
//==============================================================================

#ifndef SIGNET_TLV_HPP
#define SIGNET_TLV_HPP

#include "sig-net-constants.hpp"
#include "sig-net-types.hpp"

namespace SigNet {
namespace TLV {

//------------------------------------------------------------------------------
// Generic TLV Encoding
//------------------------------------------------------------------------------

// Encode a TLV block into a buffer
// 
// Parameters:
//   buffer    - Packet buffer to write TLV to
//   tlv       - TLV block structure containing type, length, and value
//
// Returns:
//   SIGNET_SUCCESS on success
//   Error code on failure
int32_t EncodeTLV(
    PacketBuffer& buffer,
    const TLVBlock& tlv
);

//------------------------------------------------------------------------------
// TID_LEVEL Encoding
//------------------------------------------------------------------------------

// Encode TID_LEVEL (DMX level data)
// 
// TID_LEVEL contains DMX512 slot values (0-255) for lighting control.
// Valid length: 1-512 bytes
//
// Parameters:
//   buffer      - Packet buffer to write TLV to
//   dmx_data    - Pointer to DMX slot values (0-255)
//   slot_count  - Number of DMX slots (1-512)
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_INVALID_ARG if slot_count out of range
//   Error code on failure
int32_t EncodeTID_LEVEL(
    PacketBuffer& buffer,
    const uint8_t* dmx_data,
    uint16_t slot_count
);

//------------------------------------------------------------------------------
// TID_PRIORITY Encoding
//------------------------------------------------------------------------------

// Encode TID_PRIORITY (priority data)
// 
// TID_PRIORITY contains per-slot priority values per E1.31-1 specification.
// Valid values: 0-200 per slot
// Valid length: 1-512 bytes
//
// Note: In E1.31-1, priority 100 is default/neutral, lower values indicate
// lower priority, higher values (101-200) indicate higher priority.
//
// Parameters:
//   buffer         - Packet buffer to write TLV to
//   priority_data  - Pointer to priority values (0-200)
//   slot_count     - Number of priority slots (1-512)
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_INVALID_ARG if slot_count out of range
//   Error code on failure
int32_t EncodeTID_PRIORITY(
    PacketBuffer& buffer,
    const uint8_t* priority_data,
    uint16_t slot_count
);

//------------------------------------------------------------------------------
// TID_SYNC Encoding
//------------------------------------------------------------------------------

// Encode TID_SYNC (synchronization trigger)
// 
// TID_SYNC is a zero-length TLV that signals receivers to synchronously
// output previously received level data. Used for multi-universe synchronization.
//
// Parameters:
//   buffer - Packet buffer to write TLV to
//
// Returns:
//   SIGNET_SUCCESS on success
//   Error code on failure
int32_t EncodeTID_SYNC(
    PacketBuffer& buffer
);

// Encode TID_POLL (0x0001, length 25)
// Value layout:
// [0-5]   MANAGER_TUID
// [6-9]   MANAGER_SOEM_CODE
// [10-15] TUID_LO
// [16-21] TUID_HI
// [22-23] END_POINT (0xFFFF = broadcast all data endpoints)
// [24]    QUERY_LEVEL (QUERY_HEARTBEAT..QUERY_EXTENDED)
int32_t EncodeTID_POLL(
    PacketBuffer& buffer,
    const uint8_t* manager_tuid,
    uint16_t mfg_code,
    uint16_t product_variant_id,
    const uint8_t* tuid_lo,
    const uint8_t* tuid_hi,
    uint16_t target_endpoint,
    uint8_t query_level
);

//------------------------------------------------------------------------------
// Startup Announce TLVs (Section 10.2.5)
//------------------------------------------------------------------------------

// Encode TID_POLL_REPLY (0x0002, length 12)
// Value layout: [0-5]=TUID, [6-9]=SOEM_CODE, [10-11]=CHANGE_COUNT
int32_t EncodeTID_POLL_REPLY(
    PacketBuffer& buffer,
    const uint8_t* tuid,
    uint16_t mfg_code,
    uint16_t product_variant_id,
    uint16_t change_count
);

// Encode TID_RT_PROTOCOL_VERSION (0x0603, length 1)
int32_t EncodeTID_RT_PROTOCOL_VERSION(
    PacketBuffer& buffer,
    uint8_t protocol_version
);

// Encode TID_RT_FIRMWARE_VERSION (0x0604, length 4-68)
// Value layout: [0-3]=Machine Version ID (uint32), [4..]=UTF-8 version string (max 64 bytes)
int32_t EncodeTID_RT_FIRMWARE_VERSION(
    PacketBuffer& buffer,
    uint16_t machine_version_id,
    const char* version_string
);

// Encode TID_RT_ROLE_CAPABILITY (0x0609, length 1)
int32_t EncodeTID_RT_ROLE_CAPABILITY(
    PacketBuffer& buffer,
    uint8_t role_capability_bits
);

//------------------------------------------------------------------------------
// Common Payload Builders (library-level reusable composition)
//------------------------------------------------------------------------------

// Build a complete DMX payload (currently a single TID_LEVEL TLV).
int32_t BuildDMXLevelPayload(
    PacketBuffer& payload,
    const uint8_t* dmx_data,
    uint16_t slot_count
);

// Build startup announce payload with fixed TLV ordering (Section 10.2.5):
//   1) TID_POLL_REPLY
//   2) TID_RT_FIRMWARE_VERSION
//   3) TID_RT_PROTOCOL_VERSION
//   4) TID_RT_ROLE_CAPABILITY
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
);

// Build poll payload containing one TID_POLL TLV.
int32_t BuildPollPayload(
    PacketBuffer& payload,
    const uint8_t* manager_tuid,
    uint16_t mfg_code,
    uint16_t product_variant_id,
    const uint8_t* tuid_lo,
    const uint8_t* tuid_hi,
    uint16_t target_endpoint,
    uint8_t query_level
);

//------------------------------------------------------------------------------
// Payload Building Helper
//------------------------------------------------------------------------------

// Build a complete payload with multiple TLV blocks
// This is a convenience function for constructing payloads with multiple TIDs
//
// Parameters:
//   buffer     - Packet buffer to write payload to
//   tlv_blocks - Array of TLV blocks to encode
//   count      - Number of TLV blocks in array
//
// Returns:
//   SIGNET_SUCCESS on success
//   Error code on failure
int32_t BuildPayload(
    PacketBuffer& buffer,
    const TLVBlock* tlv_blocks,
    uint16_t count
);

} // namespace TLV
} // namespace SigNet

#endif // SIGNET_TLV_HPP
