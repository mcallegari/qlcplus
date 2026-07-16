//==============================================================================
// Sig-Net Protocol Framework - Security Layer
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
// Description:  Sig-Net custom CoAP options (2076-2236) encoding and HMAC-SHA256
//               signature calculation. Implements security per Section 8.5 of
//               Sig-Net Protocol Framework specification.
//==============================================================================

#ifndef SIGNET_SECURITY_HPP
#define SIGNET_SECURITY_HPP

#include "sig-net-constants.hpp"
#include "sig-net-types.hpp"
#include "sig-net-crypto.hpp"
#include "sig-net-coap.hpp"
#include <vector>

namespace SigNet {
namespace Security {

//------------------------------------------------------------------------------
// Build SigNet Custom Options (WITHOUT HMAC)
// 
// Encodes the first 5 SigNet custom options into the packet buffer.
// The HMAC option is NOT included here - it will be calculated and added
// after the payload is complete.
//
// Parameters:
//   buffer         - Packet buffer to write options to
//   options        - SigNet option values (HMAC field is ignored)
//   prev_option    - Previous option number (typically COAP_OPTION_URI_PATH = 11)
//
// Returns:
//   SIGNET_SUCCESS on success
//   Error code on failure
//------------------------------------------------------------------------------
int32_t BuildSigNetOptionsWithoutHMAC(
    PacketBuffer& buffer,
    const SigNetOptions& options,
    uint16_t prev_option
);

//------------------------------------------------------------------------------
// Calculate HMAC Input Buffer (Section 8.5)
// 
// Constructs the byte sequence that will be authenticated:
//   1. URI string (ASCII, including leading '/')
//   2. Security-Mode (1 byte)
//   3. Sender-ID (8 bytes)
//   4. Mfg-Code (2 bytes, network byte order)
//   5. Session-ID (4 bytes, network byte order)
//   6. Seq-Num (4 bytes, network byte order)
//   7. Application Payload (variable length)
//
// Parameters:
//   uri_string     - Full URI path (e.g., "/SigNet/v1/level/517")
//   options        - SigNet option values (HMAC field is ignored)
//   payload        - Application payload bytes (TLV encoded data)
//   payload_len    - Length of payload in bytes
//   output         - Buffer to receive HMAC input (caller must provide sufficient size)
//   output_size    - Size of output buffer (should be at least uri_len + 19 + payload_len)
//   bytes_written  - Receives actual number of bytes written to output
//
// Returns:
//   SIGNET_SUCCESS on success
//   Error code on failure
//------------------------------------------------------------------------------
int32_t BuildHMACInput(
    const char* uri_string,
    const SigNetOptions& options,
    const uint8_t* payload,
    uint16_t payload_len,
    uint8_t* output,
    uint32_t output_size,
    uint32_t* bytes_written
);

//------------------------------------------------------------------------------
// Calculate and Encode HMAC Option
// 
// This function:
//   1. Builds the HMAC input buffer
//   2. Computes HMAC-SHA256 using the sender key
//   3. Encodes the HMAC as option 2236
//
// Parameters:
//   buffer         - Packet buffer to write HMAC option to
//   uri_string     - Full URI path
//   options        - SigNet option values (HMAC will be updated)
//   payload        - Application payload bytes
//   payload_len    - Length of payload
//   sender_key     - 32-byte derived sender key (Ks)
//   prev_option    - Previous option number (typically SIGNET_OPTION_SEQ_NUM = 2204)
//
// Returns:
//   SIGNET_SUCCESS on success
//   Error code on failure
//------------------------------------------------------------------------------
int32_t CalculateAndEncodeHMAC(
    PacketBuffer& buffer,
    const char* uri_string,
    SigNetOptions& options,
    const uint8_t* payload,
    uint16_t payload_len,
    const uint8_t* sender_key,
    uint16_t prev_option
);

//------------------------------------------------------------------------------
// Helper: Build Sender-ID from TUID and Endpoint
// 
// Sender-ID format (8 bytes):
//   Bytes 0-5: TUID (6 bytes)
//   Bytes 6-7: Endpoint (2 bytes, network byte order)
//
// Parameters:
//   tuid           - 6-byte TUID
//   endpoint       - 16-bit endpoint value
//   sender_id      - Output buffer (must be 8 bytes)
//
// Returns:
//   SIGNET_SUCCESS on success
//------------------------------------------------------------------------------
int32_t BuildSenderID(
    const uint8_t* tuid,
    uint16_t endpoint,
    uint8_t* sender_id
);

} // namespace Security
} // namespace SigNet

#endif // SIGNET_SECURITY_HPP
