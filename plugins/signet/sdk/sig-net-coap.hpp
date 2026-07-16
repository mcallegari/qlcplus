//==============================================================================
// Sig-Net Protocol Framework - CoAP Packet Building
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
// Description:  CoAP packet construction per RFC 7252, with extended delta
//               encoding for large option number gaps. Handles URI-Path options
//               and Sig-Net custom options in private use range (2048-64999).
//==============================================================================

#ifndef SIGNET_COAP_HPP
#define SIGNET_COAP_HPP
//==============================================================================

#include "sig-net-constants.hpp"
#include "sig-net-types.hpp"

namespace SigNet {
namespace CoAP {

// Configure URI scope segment used by packet builders.
// Scope must be 1-32 bytes, UTF-compatible bytes without '/'.
int32_t SetURIScope(const char* scope);

// Get current URI scope segment (default: "local").
const char* GetURIScope();

//------------------------------------------------------------------------------
// CoAP Header Building
//------------------------------------------------------------------------------

// Build a CoAP header for SigNet packets
// - Version: 1
// - Type: NON (Non-confirmable)
// - Token Length: 0 (no token)
// - Code: POST (0x02)
// - Message ID: Provided by caller, should increment per packet
int32_t BuildCoAPHeader(
    PacketBuffer& buffer,
    uint16_t message_id
);

//------------------------------------------------------------------------------
// CoAP Option Encoding
//------------------------------------------------------------------------------

// Encode a single CoAP option and write to buffer
// Handles extended delta and extended length encoding per RFC 7252 Section 3.1
//
// Parameters:
//   buffer         - Packet buffer to write to
//   option_number  - Absolute option number (e.g., 2076 for Security-Mode)
//   prev_option    - Previous option number (for delta calculation)
//   option_value   - Pointer to option value bytes
//   option_length  - Length of option value in bytes
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_BUFFER_FULL if buffer overflow
//   SIGNET_ERROR_ENCODE if option parameters invalid
int32_t EncodeCoAPOption(
    PacketBuffer& buffer,
    uint16_t option_number,
    uint16_t prev_option,
    const uint8_t* option_value,
    uint16_t option_length
);

//------------------------------------------------------------------------------
// URI Path Building
//------------------------------------------------------------------------------

// Build URI-Path options for a Sig-Net message
// For example, "/sig-net/v1/local/level/517" becomes 5 separate Uri-Path options:
//   - "sig-net"
//   - "v1"
//   - "local" (or configured scope)
//   - "level"
//   - "517"
//
// Parameters:
//   buffer    - Packet buffer to write to
//   universe  - Universe number (1-63999), used to build "/sig-net/v1/{scope}/level/{universe}"
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_BUFFER_FULL if buffer overflow
//   SIGNET_ERROR_INVALID_ARG if universe out of range
int32_t BuildURIPathOptions(
    PacketBuffer& buffer,
    uint16_t universe
);

// Build URI string for HMAC calculation (Section 8.5)
// Returns the full URI path as an ASCII string (e.g., "/sig-net/v1/local/level/517")
//
// Parameters:
//   universe   - Universe number (1-63999)
//   uri_output - Buffer to receive URI string (must be at least 32 bytes)
//   max_length - Size of uri_output buffer
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_INVALID_ARG if buffer too small or universe out of range
int32_t BuildURIString(
    uint16_t universe,
    char* uri_output,
    uint32_t max_length
);

bool DecodeCoapNibble(
    const uint8_t* packet,
    uint16_t packet_len,
    uint16_t& pos,
    uint8_t nibble,
    uint16_t& value
);

bool FindCoapOptionAndPayload(
    const uint8_t* packet,
    uint16_t packet_len,
    uint16_t target_option,
    uint16_t& option_offset,
    uint16_t& option_len,
    uint16_t& payload_offset
);

//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------

// Calculate the size needed for option delta encoding
// Returns number of bytes: 0 (in header), 1 (8-bit extended), or 2 (16-bit extended)
inline uint8_t GetDeltaExtendedSize(uint16_t delta) {
    if (delta <= COAP_OPTION_INLINE_MAX) {
        return 0;  // Encoded in 4-bit header field
    } else if (delta < COAP_OPTION_EXT16_BASE) {
        return 1;  // 8-bit extended delta
    } else {
        return 2;  // 16-bit extended delta
    }
}

// Calculate the size needed for option length encoding
// Returns number of bytes: 0 (in header), 1 (8-bit extended), or 2 (16-bit extended)
inline uint8_t GetLengthExtendedSize(uint16_t length) {
    if (length <= COAP_OPTION_INLINE_MAX) {
        return 0;  // Encoded in 4-bit header field
    } else if (length < COAP_OPTION_EXT16_BASE) {
        return 1;  // 8-bit extended length
    } else {
        return 2;  // 16-bit extended length
    }
}

} // namespace CoAP
} // namespace SigNet

#endif // SIGNET_COAP_HPP
