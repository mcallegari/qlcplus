//==============================================================================
// Sig-Net Protocol Framework - Type Definitions
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
// Description:  Data structures and type definitions including CoAP headers,
//               TLV structures, packet buffers, and receiver state tracking.
//               Used throughout the Sig-Net implementation.
//==============================================================================

#ifndef SIGNET_TYPES_HPP
#define SIGNET_TYPES_HPP

#include "sig-net-constants.hpp"

#include <QByteArray>
#include <QtEndian>

#include <stdint.h>
#include <string.h>

namespace SigNet {

//------------------------------------------------------------------------------
// CoAP Header Structure (RFC 7252 Section 3)
// 
// Packed 4-byte structure:
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Ver| T |  TKL  |      Code     |          Message ID           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//------------------------------------------------------------------------------

#pragma pack(push, 1)
struct CoAPHeader {
    uint8_t  version_type_tkl;  // Ver(2) | Type(2) | TKL(4)
    uint8_t  code;              // Request/Response code
    uint16_t message_id;        // Message ID (network byte order)
    
    // Helper methods for bit field access
    inline uint8_t GetVersion() const { return (version_type_tkl >> 6) & 0x03; }
    inline uint8_t GetType() const { return (version_type_tkl >> 4) & 0x03; }
    inline uint8_t GetTokenLength() const { return version_type_tkl & 0x0F; }
    
    inline void SetVersion(uint8_t ver) {
        version_type_tkl = (version_type_tkl & 0x3F) | ((ver & 0x03) << 6);
    }
    
    inline void SetType(uint8_t type) {
        version_type_tkl = (version_type_tkl & 0xCF) | ((type & 0x03) << 4);
    }
    
    inline void SetTokenLength(uint8_t tkl) {
        version_type_tkl = (version_type_tkl & 0xF0) | (tkl & 0x0F);
    }
};
#pragma pack(pop)

//------------------------------------------------------------------------------
// TLV (Type-Length-Value) Block Structure
// 
// Type:   2 bytes (network byte order)
// Length: 2 bytes (network byte order)
// Value:  Variable length data
//------------------------------------------------------------------------------

struct TLVBlock {
    uint16_t type_id;   // TID value (e.g., TID_LEVEL, TID_PRIORITY)
    uint16_t length;    // Length of value data in bytes
    const uint8_t* value;  // Pointer to value data (not owned by this struct)
    
    TLVBlock() : type_id(0), length(0), value(0) {}
    
    TLVBlock(uint16_t tid, uint16_t len, const uint8_t* val)
        : type_id(tid), length(len), value(val) {}
};

//------------------------------------------------------------------------------
// SigNet Custom Option Values
// 
// This structure holds the values for all six SigNet custom CoAP options
// that will be encoded into the packet.
//------------------------------------------------------------------------------

struct SigNetOptions {
    uint8_t  security_mode;              // SIGNET_OPTION_SECURITY_MODE (1 byte)
    uint8_t  sender_id[SENDER_ID_LENGTH]; // SIGNET_OPTION_SENDER_ID (8 bytes: TUID+endpoint)
    uint16_t mfg_code;                   // SIGNET_OPTION_MFG_CODE (2 bytes)
    uint32_t session_id;                 // SIGNET_OPTION_SESSION_ID (4 bytes)
    uint32_t seq_num;                    // SIGNET_OPTION_SEQ_NUM (4 bytes)
    uint8_t  hmac[HMAC_SHA256_LENGTH];   // SIGNET_OPTION_HMAC (32 bytes)
    
    SigNetOptions() : security_mode(0), mfg_code(0), session_id(0), seq_num(0) {
        memset(sender_id, 0, SENDER_ID_LENGTH);
        memset(hmac, 0, HMAC_SHA256_LENGTH);
    }
};

//------------------------------------------------------------------------------
// Node User Data Model (Phase 2)
//
// Shared storage for Root EP and EP1 TID data. Each TID uses the same blob
// carrier and has a manager_is_stale flag that is raised when changed by UI or
// manager traffic and cleared after proactive multicast response.
//------------------------------------------------------------------------------

enum TidBlobValueType {
    TID_BLOB_EMPTY = 0,
    TID_BLOB_U8,
    TID_BLOB_U16,
    TID_BLOB_U32,
    TID_BLOB_TEXT,
    TID_BLOB_BYTES
};

static const uint16_t TID_BLOB_MAX_BYTES = 512;

// 32-bit volatile flag for cross-thread (UI / RX) writes; aligned long is atomic on x86.
typedef volatile long SigNetAtomicFlag;

struct TidDataBlob {
    uint16_t tid;
    uint16_t length;
    uint8_t value_type;
    SigNetAtomicFlag manager_is_stale;  // Set when UI changes the value; cleared after proactive TX
    SigNetAtomicFlag ui_is_stale;       // Set when Sig-Net SET updates the value; cleared after UI sync
    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        char text[TID_BLOB_MAX_BYTES + 1];
        uint8_t bytes[TID_BLOB_MAX_BYTES];
    } data;

    TidDataBlob() : tid(0), length(0), value_type(TID_BLOB_EMPTY),
                    manager_is_stale(0), ui_is_stale(0) {
        memset(data.bytes, 0, sizeof(data.bytes));
        data.text[0] = 0;
    }
};

struct RootTidStore {
    TidDataBlob tid_rt_supported_tids;
    TidDataBlob tid_rt_endpoint_count;
    TidDataBlob tid_rt_protocol_version;
    TidDataBlob tid_rt_firmware_version;
    TidDataBlob tid_rt_device_label;
    TidDataBlob tid_rt_mult;
    TidDataBlob tid_rt_identify;
    TidDataBlob tid_rt_status;
    TidDataBlob tid_rt_role_capability;
    TidDataBlob tid_rt_reboot;
    TidDataBlob tid_rt_model_name;
    TidDataBlob tid_rt_scope;
    TidDataBlob tid_rt_unprovision;

    TidDataBlob tid_nw_mac_address;
    TidDataBlob tid_nw_ipv4_mode;
    TidDataBlob tid_nw_ipv4_address;
    TidDataBlob tid_nw_ipv4_netmask;
    TidDataBlob tid_nw_ipv4_gateway;
    TidDataBlob tid_nw_ipv4_current;
    TidDataBlob tid_nw_ipv6_mode;
    TidDataBlob tid_nw_ipv6_address;
    TidDataBlob tid_nw_ipv6_prefix;
    TidDataBlob tid_nw_ipv6_gateway;
    TidDataBlob tid_nw_ipv6_current;

    TidDataBlob tid_dg_security_event;
    TidDataBlob tid_dg_message;

    RootTidStore() {
        tid_rt_supported_tids.tid = TID_RT_SUPPORTED_TIDS;
        tid_rt_endpoint_count.tid = TID_RT_ENDPOINT_COUNT;
        tid_rt_protocol_version.tid = TID_RT_PROTOCOL_VERSION;
        tid_rt_firmware_version.tid = TID_RT_FIRMWARE_VERSION;
        tid_rt_device_label.tid = TID_RT_DEVICE_LABEL;
        tid_rt_mult.tid = TID_RT_MULT;
        tid_rt_identify.tid = TID_RT_IDENTIFY;
        tid_rt_status.tid = TID_RT_STATUS;
        tid_rt_role_capability.tid = TID_RT_ROLE_CAPABILITY;
        tid_rt_reboot.tid = TID_RT_REBOOT;
        tid_rt_model_name.tid = TID_RT_MODEL_NAME;
        tid_rt_scope.tid = TID_RT_SCOPE;
        tid_rt_unprovision.tid = TID_RT_UNPROVISION;

        tid_nw_mac_address.tid = TID_NW_MAC_ADDRESS;
        tid_nw_ipv4_mode.tid = TID_NW_IPV4_MODE;
        tid_nw_ipv4_address.tid = TID_NW_IPV4_ADDRESS;
        tid_nw_ipv4_netmask.tid = TID_NW_IPV4_NETMASK;
        tid_nw_ipv4_gateway.tid = TID_NW_IPV4_GATEWAY;
        tid_nw_ipv4_current.tid = TID_NW_IPV4_CURRENT;
        tid_nw_ipv6_mode.tid = TID_NW_IPV6_MODE;
        tid_nw_ipv6_address.tid = TID_NW_IPV6_ADDRESS;
        tid_nw_ipv6_prefix.tid = TID_NW_IPV6_PREFIX;
        tid_nw_ipv6_gateway.tid = TID_NW_IPV6_GATEWAY;
        tid_nw_ipv6_current.tid = TID_NW_IPV6_CURRENT;

        tid_dg_security_event.tid = TID_DG_SECURITY_EVENT;
        tid_dg_message.tid = TID_DG_MESSAGE;
    }
};

struct EP1TidStore {
    TidDataBlob tid_ep_universe;
    TidDataBlob tid_ep_label;
    TidDataBlob tid_ep_mult_override;
    TidDataBlob tid_ep_capability;
    TidDataBlob tid_ep_direction;
    TidDataBlob tid_ep_input_priority;
    TidDataBlob tid_ep_status;
    TidDataBlob tid_ep_failover;
    TidDataBlob tid_ep_dmx_timing;
    TidDataBlob tid_ep_refresh_capability;
    TidDataBlob tid_rdm_tod_background;  // TID_RDM_TOD_BACKGROUND (0x0305) – 1 byte
    TidDataBlob tid_rdm_flow_control;    // TID_RDM_FLOW_CONTROL (0x0306) – 2 bytes
    TidDataBlob tid_rdm_tod_data;        // TID_RDM_TOD_DATA (0x0304) – [index,total,UIDs...]
    TidDataBlob tid_dg_level_foldback;   // TID_DG_LEVEL_FOLDBACK (0xFF03) – 1..512 bytes

    TidDataBlob tid_level;
    TidDataBlob tid_priority;
    TidDataBlob tid_sync;

    EP1TidStore() {
        tid_ep_universe.tid = TID_EP_UNIVERSE;
        tid_ep_label.tid = TID_EP_LABEL;
        tid_ep_mult_override.tid = TID_EP_MULT_OVERRIDE;
        tid_ep_capability.tid = TID_EP_CAPABILITY;
        tid_ep_direction.tid = TID_EP_DIRECTION;
        tid_ep_input_priority.tid = TID_EP_INPUT_PRIORITY;
        tid_ep_status.tid = TID_EP_STATUS;
        tid_ep_failover.tid = TID_EP_FAILOVER;
        tid_ep_dmx_timing.tid = TID_EP_DMX_TIMING;
        tid_ep_refresh_capability.tid = TID_EP_REFRESH_CAPABILITY;
        tid_rdm_tod_background.tid = TID_RDM_TOD_BACKGROUND;
        tid_rdm_flow_control.tid = TID_RDM_FLOW_CONTROL;
        tid_rdm_tod_data.tid = TID_RDM_TOD_DATA;
        tid_dg_level_foldback.tid = TID_DG_LEVEL_FOLDBACK;

        tid_level.tid = TID_LEVEL;
        tid_priority.tid = TID_PRIORITY;
        tid_sync.tid = TID_SYNC;
    }
};

struct NodeUserData {
    RootTidStore root;
    EP1TidStore ep1;
};

//------------------------------------------------------------------------------
// Packet Buffer Class
// 
// Manages a static 1400-byte buffer for constructing SigNet packets.
// Provides bounds-checking to prevent buffer overflows.
//------------------------------------------------------------------------------

class PacketBuffer {
public:
    PacketBuffer()
        : write_position_(0)
    {
        buffer_.reserve(MAX_UDP_PAYLOAD);
    }

    // Reset buffer for new packet construction
    void Reset() {
        buffer_.clear();
        write_position_ = 0;
    }
    
    // Get current write position
    uint16_t GetPosition() const {
        return write_position_;
    }
    
    // Get total size of data written
    uint16_t GetSize() const {
        return write_position_;
    }
    
    // Get direct access to buffer (read-only)
    const uint8_t* GetBuffer() const {
        return reinterpret_cast<const uint8_t*>(buffer_.constData());
    }
    
    // Get direct access to buffer (mutable) - use with caution
    uint8_t* GetMutableBuffer() {
        return reinterpret_cast<uint8_t*>(buffer_.data());
    }
    
    // Check if there's enough space for 'size' bytes
    bool HasSpace(uint16_t size) const {
        return (write_position_ + size) <= MAX_UDP_PAYLOAD;
    }
    
    // Write a single byte
    int32_t WriteByte(uint8_t value) {
        if (!HasSpace(1)) {
            return SIGNET_ERROR_BUFFER_FULL;
        }
        buffer_.append(char(value));
        ++write_position_;
        return SIGNET_SUCCESS;
    }
    
    // Write multiple bytes
    int32_t WriteBytes(const uint8_t* data, uint16_t length) {
        if (!HasSpace(length)) {
            return SIGNET_ERROR_BUFFER_FULL;
        }
        buffer_.append(reinterpret_cast<const char*>(data), length);
        write_position_ += length;
        return SIGNET_SUCCESS;
    }
    
    // Write a uint16_t in network byte order (big-endian)
    int32_t WriteUInt16(uint16_t value) {
        if (!HasSpace(2)) {
            return SIGNET_ERROR_BUFFER_FULL;
        }
        const quint16 bigEndian = qToBigEndian(value);
        buffer_.append(reinterpret_cast<const char*>(&bigEndian), sizeof(bigEndian));
        write_position_ += sizeof(bigEndian);
        return SIGNET_SUCCESS;
    }
    
    // Write a uint32_t in network byte order (big-endian)
    int32_t WriteUInt32(uint32_t value) {
        if (!HasSpace(4)) {
            return SIGNET_ERROR_BUFFER_FULL;
        }
        const quint32 bigEndian = qToBigEndian(value);
        buffer_.append(reinterpret_cast<const char*>(&bigEndian), sizeof(bigEndian));
        write_position_ += sizeof(bigEndian);
        return SIGNET_SUCCESS;
    }
    
    // Seek to specific position (use with caution)
    int32_t Seek(uint16_t position) {
        if (position > MAX_UDP_PAYLOAD) {
            return SIGNET_ERROR_INVALID_ARG;
        }
        buffer_.resize(position);
        write_position_ = position;
        return SIGNET_SUCCESS;
    }
    
private:
    QByteArray buffer_;
    uint16_t write_position_;
};

//------------------------------------------------------------------------------
// Receiver Sender State
// 
// Tracks session/sequence state per unique Sender-ID (TUID+endpoint)
// for anti-replay protection per Section 8.6 Step 9.
//------------------------------------------------------------------------------

struct ReceiverSenderState {
    uint8_t  sender_id[SENDER_ID_LENGTH]; // TUID(6) + endpoint(2)
    uint32_t session_id;                  // Most recent valid session ID
    uint32_t seq_num;                     // Most recent valid sequence number
    uint32_t last_packet_time_ms;         // Timestamp of last accepted packet
    uint32_t total_packets_received;      // Total packets from this sender
    uint32_t total_packets_accepted;      // Total packets accepted (HMAC OK + fresh)
    
    ReceiverSenderState() : session_id(0), seq_num(0), last_packet_time_ms(0),
                           total_packets_received(0), total_packets_accepted(0) {
        memset(sender_id, 0, SENDER_ID_LENGTH);
    }
};

//------------------------------------------------------------------------------
// Receiver Statistics
// 
// Global receiver statistics for diagnostics.
//------------------------------------------------------------------------------

struct ReceiverStatistics {
    // Packet counts
    uint32_t total_packets;           // Total UDP packets received
    uint32_t accepted_packets;        // Packets accepted (all validation passed)
    
    // Rejection reasons
    uint32_t coap_version_errors;     // CoAP version != 1
    uint32_t coap_type_errors;        // CoAP type != NON (0)
    uint32_t coap_code_errors;        // CoAP code != POST (0.02)
    uint32_t uri_mismatches;          // URI not "/level/"
    uint32_t missing_options;         // Required options missing
    uint32_t hmac_failures;           // HMAC verification failed
    uint32_t replay_detected;         // Session/Sequence replay
    uint32_t parse_errors;            // Malformed packet structure
    
    // Timing
    uint32_t last_packet_time_ms;     // Timestamp of last packet
    
    ReceiverStatistics() : total_packets(0), accepted_packets(0),
                          coap_version_errors(0), coap_type_errors(0),
                          coap_code_errors(0), uri_mismatches(0),
                          missing_options(0), hmac_failures(0),
                          replay_detected(0), parse_errors(0),
                          last_packet_time_ms(0) {}
    
    void Reset() {
        total_packets = 0;
        accepted_packets = 0;
        coap_version_errors = 0;
        coap_type_errors = 0;
        coap_code_errors = 0;
        uri_mismatches = 0;
        missing_options = 0;
        hmac_failures = 0;
        replay_detected = 0;
        parse_errors = 0;
        last_packet_time_ms = 0;
    }
};

//------------------------------------------------------------------------------
// Received Packet Info
// 
// Information about a received packet for logging and diagnostics.
//------------------------------------------------------------------------------

struct ReceivedPacketInfo {
    // CoAP fields
    uint16_t message_id;              // CoAP Message ID
    
    // SigNet fields
    uint8_t  sender_tuid[6];          // Sender TUID (first 6 bytes of Sender-ID)
    uint16_t endpoint;                // Endpoint (last 2 bytes of Sender-ID)
    uint16_t mfg_code;                // Manufacturer code
    uint32_t session_id;              // Session ID
    uint32_t seq_num;                 // Sequence number
    
    // Payload info
    uint16_t dmx_slot_count;          // Number of DMX slots received
    
    // Validation results
    bool     hmac_valid;              // HMAC verification passed
    bool     session_fresh;           // Session/Sequence is fresh (not replay)
    
    // Rejection reason (if rejected)
    const char* rejection_reason;     // NULL if accepted, else description
    
    // Timing
    uint32_t timestamp_ms;            // Packet receive timestamp
    
    ReceivedPacketInfo() : message_id(0), endpoint(0), mfg_code(0),
                          session_id(0), seq_num(0), dmx_slot_count(0),
                          hmac_valid(false), session_fresh(false),
                          rejection_reason(0), timestamp_ms(0) {
        memset(sender_tuid, 0, 6);
    }
};

} // namespace SigNet

#endif // SIGNET_TYPES_HPP
