//==============================================================================
// Sig-Net Protocol Framework - Packet Parsing Functions
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
// Prot Version: v0.12
// Description:  Parsing and verification for received Sig-Net packets. CoAP
//               header/option parsing, custom option extraction, URI rebuild,
//               TLV parsing, HMAC verification. Complements sig-net-send.hpp.
//==============================================================================

#ifndef SIGNET_PARSE_HPP
#define SIGNET_PARSE_HPP

#include "sig-net-constants.hpp"
#include "sig-net-types.hpp"
#include <stdint.h>

namespace SigNet {
namespace Parse {

//------------------------------------------------------------------------------
// PacketReader - Helper class for reading from const buffer
//------------------------------------------------------------------------------
class PacketReader {
private:
    const uint8_t* buffer_;
    uint16_t position_;
    uint16_t size_;
    
public:
    PacketReader(const uint8_t* buffer, uint16_t size);
    
    // Position management
    uint16_t GetPosition() const { return position_; }
    uint16_t GetRemaining() const { return size_ - position_; }
    bool CanRead(uint16_t bytes) const { return (position_ + bytes) <= size_; }
    
    // Read operations (advance position)
    bool ReadByte(uint8_t& value);
    bool ReadUInt16(uint16_t& value);  // Network byte order
    bool ReadUInt32(uint32_t& value);  // Network byte order
    bool ReadBytes(uint8_t* dest, uint16_t count);
    bool Skip(uint16_t count);
    bool Seek(uint16_t new_position);

    // Peek operations (don't advance position)
    bool PeekByte(uint8_t& value) const;
    
    // Direct buffer access
    const uint8_t* GetCurrentPtr() const { return buffer_ + position_; }
};

//------------------------------------------------------------------------------
// Parse CoAP Header
// 
// Extracts the 4-byte CoAP header fields.
// 
// Parameters:
//   reader - PacketReader positioned at start of packet
//   header - Output: Receives parsed header structure
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_BUFFER_TOO_SMALL if insufficient data
//------------------------------------------------------------------------------
int32_t ParseCoAPHeader(PacketReader& reader, CoAPHeader& header);

//------------------------------------------------------------------------------
// Skip CoAP Token
// 
// Skips over the CoAP token bytes if present (based on TKL field).
// 
// Parameters:
//   reader - PacketReader positioned after CoAP header
//   token_length - Token length from CoAP header TKL field
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_BUFFER_TOO_SMALL if insufficient data
//------------------------------------------------------------------------------
int32_t SkipToken(PacketReader& reader, uint8_t token_length);

//------------------------------------------------------------------------------
// Parse CoAP Option
// 
// Parses a single CoAP option using delta encoding.
// Handles extended delta/length fields (13, 14).
// 
// Parameters:
//   reader - PacketReader positioned at option start
//   prev_option - Previous option number (for delta calculation)
//   option_num - Output: Receives calculated option number
//   option_value - Output: Pointer to option value bytes (no copy)
//   option_length - Output: Length of option value
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_BUFFER_TOO_SMALL if insufficient data
//   SIGNET_ERROR_INVALID_PACKET if payload marker (0xFF) encountered
//------------------------------------------------------------------------------
int32_t ParseCoAPOption(
    PacketReader& reader,
    uint16_t prev_option,
    uint16_t& option_num,
    const uint8_t*& option_value,
    uint16_t& option_length
);

//------------------------------------------------------------------------------
// Extract URI String
// 
// Rebuilds the full URI string from CoAP Uri-Path options.
// 
// Parameters:
//   reader - PacketReader positioned after CoAP header + token
//   uri_string - Output buffer to receive URI string
//   uri_buffer_size - Size of uri_string buffer
//   uri_length - Output: Receives actual URI string length
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_BUFFER_FULL if uri_string buffer too small
//   SIGNET_ERROR_BUFFER_TOO_SMALL if packet data insufficient
//------------------------------------------------------------------------------
int32_t ExtractURIString(
    PacketReader& reader,
    char* uri_string,
    uint16_t uri_buffer_size,
    uint16_t& uri_length
);

//------------------------------------------------------------------------------
// Parse SigNet Options
//
// Parses all 6 SigNet custom options (2076-2236) from a packet.
//
// initial_prev_option seeds the delta accumulator: pass 0 when starting from
// the option run, or COAP_OPTION_URI_PATH when chained after ExtractURIString.
//
// Returns SIGNET_SUCCESS, or SIGNET_ERROR_INVALID_OPTION / SIGNET_ERROR_BUFFER_TOO_SMALL.
//------------------------------------------------------------------------------
int32_t ParseSigNetOptions(
    PacketReader& reader,
    SigNetOptions& options,
    uint16_t initial_prev_option = 0
);

//------------------------------------------------------------------------------
// Parse TLV Block
// 
// Parses the next Type-Length-Value block from the payload.
// 
// Parameters:
//   reader - PacketReader positioned at TLV start
//   tlv - Output: Receives parsed TLV (value pointer references buffer)
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_BUFFER_TOO_SMALL if insufficient data
//------------------------------------------------------------------------------
int32_t ParseTLVBlock(PacketReader& reader, TLVBlock& tlv);

//------------------------------------------------------------------------------
// Parse TID_LEVEL Payload
// 
// Extracts DMX level data from a TID_LEVEL TLV block.
// 
// Parameters:
//   tlv - Parsed TLV block with type_id == TID_LEVEL
//   dmx_data - Output buffer to receive DMX data (must be >= 512 bytes)
//   slot_count - Output: Receives number of DMX slots (1-512)
//
// Returns:
//   SIGNET_SUCCESS on success
//   SIGNET_ERROR_INVALID_ARG if TLV type is not TID_LEVEL
//   SIGNET_ERROR_INVALID_PACKET if length > 512
//------------------------------------------------------------------------------
int32_t ParseTID_LEVEL(
    const TLVBlock& tlv,
    uint8_t* dmx_data,
    uint16_t& slot_count
);

//------------------------------------------------------------------------------
// Parse fixed-size hex byte arrays from text.
//
// Accepts optional leading "0x" and surrounding whitespace.
// Expects exactly (byte_count * 2) hex digits after normalization.
//------------------------------------------------------------------------------
int32_t ParseHexBytes(
    const char* text,
    uint8_t* out_bytes,
    uint16_t byte_count
);

// Parse 32-byte K0 hex value (64 hex chars).
int32_t ParseK0Hex(const char* text, uint8_t out_k0[32]);

// Parse 6-byte TUID hex value (12 hex chars).
int32_t ParseTUIDHex(const char* text, uint8_t out_tuid[6]);

// Parse endpoint text (decimal or prefixed hex, e.g. "65535", "0xFFFF", "$FFFF").
int32_t ParseEndpointValue(const char* text, uint16_t& endpoint_out);

// Parse 16-bit hex word with optional "0x" prefix.
int32_t ParseHexWord(const char* text, uint16_t& value_out);

// Validate Sig-Net URI shape and scope match.
// Expected prefix: /sig-net/<version>/<scope>/...
int32_t ValidateSigNetURI(const char* uri_string);

//------------------------------------------------------------------------------
// Verify Packet HMAC
// 
// Calculates expected HMAC and compares with received HMAC using constant-time
// comparison to prevent timing attacks.
// 
// Parameters:
//   uri_string - Full URI path string
//   options - Parsed SigNet options (including received HMAC)
//   payload - Application payload bytes
//   payload_len - Length of payload
//   role_key - 32-byte derived role key (Ks, Kn, Km_global, or Km_local)
//
// Returns:
//   SIGNET_SUCCESS if HMAC matches
//   SIGNET_ERROR_HMAC_FAILED if HMAC does not match
//   Error code if HMAC calculation fails
//------------------------------------------------------------------------------
int32_t VerifyPacketHMAC(
    const char* uri_string,
    const SigNetOptions& options,
    const uint8_t* payload,
    uint16_t payload_len,
    const uint8_t* role_key
);

} // namespace Parse
} // namespace SigNet

#endif // SIGNET_PARSE_HPP
