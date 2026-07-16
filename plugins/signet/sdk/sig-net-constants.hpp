//==============================================================================
// Sig-Net Protocol Framework - Constants and Definitions
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
// Description:  Protocol constants, CoAP option numbers, TIDs, error codes,
//               and configuration parameters for Sig-Net implementation.
//               Values derived from Sig-Net Protocol Framework spec.
//==============================================================================

#ifndef SIGNET_CONSTANTS_HPP
#define SIGNET_CONSTANTS_HPP

#include <stdint.h>

namespace SigNet {

//------------------------------------------------------------------------------
// CoAP Protocol Constants (RFC 7252)
//------------------------------------------------------------------------------

// CoAP Version
static const uint8_t COAP_VERSION = 1;

// CoAP Message Types
static const uint8_t COAP_TYPE_CON = 0;  // Confirmable
static const uint8_t COAP_TYPE_NON = 1;  // Non-confirmable (used by Sig-Net)
static const uint8_t COAP_TYPE_ACK = 2;  // Acknowledgement
static const uint8_t COAP_TYPE_RST = 3;  // Reset

// CoAP Method Codes
static const uint8_t COAP_CODE_EMPTY = 0x00;
static const uint8_t COAP_CODE_GET   = 0x01;
static const uint8_t COAP_CODE_POST  = 0x02;  // Used by Sig-Net
static const uint8_t COAP_CODE_PUT   = 0x03;
static const uint8_t COAP_CODE_DELETE = 0x04;

// CoAP Standard Option Numbers
static const uint16_t COAP_OPTION_URI_PATH = 11;

// CoAP option extended encoding constants (RFC 7252)
static const uint8_t  COAP_OPTION_INLINE_MAX = 12;      // 0..12 encoded directly in nibble
static const uint8_t  COAP_OPTION_EXT8_NIBBLE = 13;     // 8-bit extended follows
static const uint8_t  COAP_OPTION_EXT16_NIBBLE = 14;    // 16-bit extended follows
static const uint16_t COAP_OPTION_EXT8_BASE = 13;       // value = ext + 13
static const uint16_t COAP_OPTION_EXT16_BASE = 269;     // value = ext + 269

// CoAP Payload Marker
static const uint8_t COAP_PAYLOAD_MARKER = 0xFF;

//------------------------------------------------------------------------------
// Sig-Net Custom CoAP Options (Private Use Range 2048-64999)
// These are Elective, Safe-to-Forward, NoCacheKey options
//------------------------------------------------------------------------------

static const uint16_t SIGNET_OPTION_SECURITY_MODE = 2076;  // 1 byte
static const uint16_t SIGNET_OPTION_SENDER_ID     = 2108;  // 8 bytes (TUID + endpoint)
static const uint16_t SIGNET_OPTION_MFG_CODE      = 2140;  // 2 bytes (ESTA Manufacturer ID)
static const uint16_t SIGNET_OPTION_SESSION_ID    = 2172;  // 4 bytes (boot counter)
static const uint16_t SIGNET_OPTION_SEQ_NUM       = 2204;  // 4 bytes (sequence number)
static const uint16_t SIGNET_OPTION_HMAC          = 2236;  // 32 bytes (HMAC-SHA256)

//------------------------------------------------------------------------------
// Sig-Net Security Modes
//------------------------------------------------------------------------------

static const uint8_t SECURITY_MODE_HMAC_SHA256    = 0x00;  // HMAC-SHA256, plaintext payload
static const uint8_t SECURITY_MODE_UNPROVISIONED  = 0xFF;  // Unprovisioned beacon mode

//------------------------------------------------------------------------------
// Sig-Net Type ID (TID) Definitions - Application Layer (Section 11)
// 16-bit big-endian. Upper byte = category, lower byte = subtype.
// Standard TIDs: 0x0000-0x7FFF. Manufacturer TIDs: 0x8000-0xFFFF.
// TLV total size = Length + 4 bytes (2-byte TID + 2-byte Length field).
//------------------------------------------------------------------------------

// Section 11.1 - Node-Discovery Type Identifiers
static const uint16_t TID_POLL                  = 0x0001;  // Poll: request node-discovery replies (25 bytes)
static const uint16_t TID_POLL_REPLY            = 0x0002;  // Poll reply: presence, TUID, SoemCode, CHANGE_COUNT (12 bytes)

// Section 11.2 - Sender Type Identifiers
static const uint16_t TID_LEVEL                 = 0x0101;  // DMX level data, zero start code (1-512 bytes)
static const uint16_t TID_PRIORITY              = 0x0102;  // Per-slot priority per E1.31-1 (1-512 bytes, 0-200)
static const uint16_t TID_SYNC                  = 0x0201;  // Synchronisation trigger - flush all buffers (0 bytes)
static const uint16_t TID_TIMECODE              = 0x0202;  // MIDI-style timecode: HH MM SS FF Type (5 bytes)

// Section 11.3 - RDM Type Identifiers
static const uint16_t TID_RDM_COMMAND           = 0x0301;  // Encapsulated E1.20 RDM request from Manager (26-257 bytes)
static const uint16_t TID_RDM_RESPONSE          = 0x0302;  // Encapsulated E1.20 RDM response from Node (26-257 bytes)
static const uint16_t TID_RDM_TOD_CONTROL       = 0x0303;  // TOD control: force discovery or flush (1 byte)
static const uint16_t TID_RDM_TOD_DATA          = 0x0304;  // RDM ToD block with packet index/total + UID array (2+N bytes)
static const uint16_t TID_RDM_TOD_BACKGROUND    = 0x0305;  // Enable/disable background RDM discovery (0/1 byte)
static const uint16_t TID_RDM_FLOW_CONTROL      = 0x0306;  // RDM FIFO capacity/availability report (0/2 bytes)

// Section 11.4 - Provisioning Type Identifiers (Root Endpoint only)
static const uint16_t TID_RT_UNPROVISION        = 0x0401;  // Wipe keys and return to unprovisioned state (4 bytes, magic 0x57495045)

// Section 11.5 - Network Configuration Type Identifiers (Root Endpoint only)
static const uint16_t TID_NW_MAC_ADDRESS        = 0x0501;  // Physical MAC address (0/6 bytes)
static const uint16_t TID_NW_IPV4_MODE          = 0x0502;  // IPv4 mode: 0x00=Static, 0x01=DHCP (0/1 byte)
static const uint16_t TID_NW_IPV4_ADDRESS       = 0x0503;  // Static IPv4 address (0/4 bytes)
static const uint16_t TID_NW_IPV4_NETMASK       = 0x0504;  // Static IPv4 subnet mask (0/4 bytes)
static const uint16_t TID_NW_IPV4_GATEWAY       = 0x0505;  // Static IPv4 default gateway (0/4 bytes)
static const uint16_t TID_NW_IPV4_CURRENT       = 0x0506;  // Active IPv4 address+mask+gateway (0/12 bytes)
static const uint16_t TID_NW_IPV6_MODE          = 0x0581;  // IPv6 mode: 0x00=Static, 0x01=SLAAC, 0x02=DHCPv6 (0/1 byte)
static const uint16_t TID_NW_IPV6_ADDRESS       = 0x0582;  // Static IPv6 address (0/16 bytes)
static const uint16_t TID_NW_IPV6_PREFIX        = 0x0583;  // Static IPv6 prefix length 0-128 (0/1 byte)
static const uint16_t TID_NW_IPV6_GATEWAY       = 0x0584;  // Static IPv6 default gateway (0/16 bytes)
static const uint16_t TID_NW_IPV6_CURRENT       = 0x0585;  // Active IPv6 address+prefix+gateway (0/33 bytes)

// Section 11.6 - Root Endpoint Type Identifiers (Root Endpoint only)
static const uint16_t TID_RT_SUPPORTED_TIDS     = 0x0601;  // Array of supported TIDs (multiples of 2 bytes)
static const uint16_t TID_RT_ENDPOINT_COUNT     = 0x0602;  // Total data endpoint count, not incl. EP0 (0/2 bytes)
static const uint16_t TID_RT_PROTOCOL_VERSION   = 0x0603;  // Supported Sig-Net major version (0/1 byte)
static const uint16_t TID_RT_FIRMWARE_VERSION   = 0x0604;  // Machine version ID + UTF-8 string (0/4-68 bytes)
static const uint16_t TID_RT_DEVICE_LABEL       = 0x0605;  // Human-readable device label, UTF-8 (0-64 bytes)
static const uint16_t TID_RT_MULT               = 0x0606;  // Multicast routing state: 0x00=Default, 0x01=Custom (0/1 byte)
static const uint16_t TID_RT_MULT_OVERRIDE       = 0x0606;  // Alias: v0.15 section 11.6.6 name for TID_RT_MULT
static const uint16_t TID_RT_IDENTIFY           = 0x0607;  // Identify state: 0x00=Off, 0x01=On (0/1 byte)
static const uint16_t TID_RT_STATUS             = 0x0608;  // Device health bitfield: Bit0=HW Fault, Bit1=Factory, Bit2=Locked (0/4 bytes)
static const uint16_t TID_RT_ROLE_CAPABILITY    = 0x0609;  // Role bitfield: Bit0=Node, Bit1=Sender, Bit2=Manager (0/1 byte)
static const uint16_t TID_RT_REBOOT             = 0x060A;  // Reboot command with BOOT magic (5 bytes)
static const uint16_t TID_RT_MODEL_NAME         = 0x060B;  // Product model UTF-8 string, max 64 bytes (0/1-64 bytes)
static const uint16_t TID_RT_SCOPE              = 0x060C;  // Operational URI scope UTF-8 string, max 32 bytes (0/1-32 bytes)

// Section 11.7 - Data Endpoint Type Identifiers (Data Endpoints 1-N only)
static const uint16_t TID_EP_UNIVERSE           = 0x0901;  // Assigned universe 1-63999, 0=unset (0/2 bytes)
static const uint16_t TID_EP_LABEL              = 0x0902;  // Endpoint label, UTF-8, max 64 bytes, not null-terminated (0-64 bytes)
static const uint16_t TID_EP_MULT_OVERRIDE      = 0x0903;  // Custom multicast IPv4 override, 0.0.0.0=clear (0/4 bytes)
static const uint16_t TID_EP_CAPABILITY           = 0x0904;  // Port capabilities bitfield: Bit0=ConsumeLevel, Bit1=SupplyLevel, Bit2=ConsumeRDM, Bit3=SupplyRDM, Bit4=Virtual (0/1 byte) [v0.15 name]
static const uint16_t TID_EP_DIRECTION_CAPABILITY = 0x0904;  // Legacy alias for TID_EP_CAPABILITY (pre-v0.15 name)
static const uint16_t TID_EP_DIRECTION          = 0x0905;  // Port direction: 0x00=Disabled, 0x01=Consumer, 0x02=Supplier (0/1 byte)
static const uint16_t TID_EP_INPUT_PRIORITY     = 0x0906;  // Per-slot E1.31-1 priority for input port (0/1-512 bytes)
static const uint16_t TID_EP_STATUS             = 0x0907;  // Endpoint health bitfield: Bit0=Activity, Bit1=HW Fault, Bit2=Locked (0/4 bytes)
static const uint16_t TID_EP_FAILOVER           = 0x0908;  // Endpoint stream-loss failover mode + optional scene (0/3 bytes)
static const uint16_t TID_EP_DMX_TIMING         = 0x0909;  // Endpoint DMX transmission mode and timing (0/2 bytes)
static const uint16_t TID_EP_REFRESH_CAPABILITY = 0x090A;  // Endpoint max refresh capability in Hz (0/1 byte)

// Section 11.8 - Diagnostic Type Identifiers
static const uint16_t TID_DG_SECURITY_EVENT     = 0xFF01;  // Security event report: EventCode+Counter+SourceIP (0/11-23 bytes)
static const uint16_t TID_DG_MESSAGE            = 0xFF02;  // Human-readable diagnostic message, UTF-8, not null-terminated (0-64 bytes)
static const uint16_t TID_DG_LEVEL_FOLDBACK     = 0xFF03;  // Copy of level buffer for the specified universe (0/1-512 bytes)

// Poll query levels (Section 11.9)
static const uint8_t QUERY_HEARTBEAT            = 0x00;
static const uint8_t QUERY_CONFIG               = 0x01;
static const uint8_t QUERY_FULL                 = 0x02;
static const uint8_t QUERY_EXTENDED             = 0x03;

//------------------------------------------------------------------------------
// Network Configuration
//------------------------------------------------------------------------------

static const uint16_t SIGNET_UDP_PORT = 5683;  // Standard CoAP port

// Multicast address range: 239.254.0.1 - 239.254.0.100
static const uint8_t MULTICAST_BASE_OCTET_0 = 239;
static const uint8_t MULTICAST_BASE_OCTET_1 = 254;
static const uint8_t MULTICAST_BASE_OCTET_2 = 0;
static const uint8_t MULTICAST_MIN_INDEX    = 1;
static const uint8_t MULTICAST_MAX_INDEX    = 109;

// Multicast TTL (Time To Live)
static const uint8_t MULTICAST_TTL = 32;

//------------------------------------------------------------------------------
// Protocol Limits
//------------------------------------------------------------------------------

static const uint16_t MAX_DMX_SLOTS         = 512;   // Maximum DMX slots per universe
static const uint16_t MIN_UNIVERSE          = 1;     // Minimum valid universe number
static const uint16_t MAX_UNIVERSE          = 63999; // Maximum valid universe number
static const uint16_t MAX_UDP_PAYLOAD       = 1400;  // Maximum single UDP packet size (bytes)
static const uint16_t COAP_HEADER_SIZE      = 4;     // CoAP header is always 4 bytes
static const uint16_t UNIVERSE_DECIMAL_BUFFER_SIZE = 8; // "63999" + null
static const uint16_t URI_STRING_MIN_BUFFER = 96;       // URI path buffer with scoped paths

//------------------------------------------------------------------------------
// Transmission Timing (Section 10.6.2)
//------------------------------------------------------------------------------

static const uint32_t MAX_ACTIVE_RATE_HZ    = 44;    // Maximum transmission rate when data changing
static const uint32_t KEEPALIVE_RATE_HZ     = 1;     // Keep-alive rate when idle
static const uint32_t STREAM_LOSS_TIMEOUT_MS = 3000; // Stream loss timeout (milliseconds)

//------------------------------------------------------------------------------
// Cryptographic Constants
//------------------------------------------------------------------------------

static const uint32_t K0_KEY_LENGTH         = 32;    // 256-bit root key (bytes)
static const uint32_t DERIVED_KEY_LENGTH    = 32;    // All derived keys are 256-bit (bytes)
static const uint32_t HMAC_SHA256_LENGTH    = 32;    // HMAC-SHA256 digest length (bytes)
static const uint32_t TUID_LENGTH           = 6;     // TUID is 6 bytes (48-bit)
static const uint32_t TUID_HEX_LENGTH       = 12;    // TUID hex chars (without null terminator)
static const uint32_t ENDPOINT_LENGTH       = 2;     // Endpoint is 2 bytes (16-bit)
static const uint32_t SENDER_ID_LENGTH      = 8;     // Sender-ID = TUID + Endpoint (bytes)
static const uint32_t HKDF_INFO_INPUT_MAX   = 63;    // info bytes before HKDF counter byte
static const uint8_t  HKDF_COUNTER_T1       = 0x01;  // HKDF counter for first block T(1)

//------------------------------------------------------------------------------
// Sig-Net URI Path Components
//------------------------------------------------------------------------------

static const char* SIGNET_URI_PREFIX    = "sig-net";
static const char* SIGNET_URI_VERSION   = "v1";
static const char* SIGNET_URI_SCOPE_DEFAULT = "local";
static const uint8_t SIGNET_URI_SCOPE_MAX_LENGTH = 32;
static const char* SIGNET_URI_LEVEL     = "level";     // For TID_LEVEL messages
static const char* SIGNET_URI_PRIORITY  = "priority";  // For TID_PRIORITY messages
static const char* SIGNET_URI_SYNC      = "sync";      // For TID_SYNC messages
static const char* SIGNET_URI_NODE      = "node";      // For /node/{tuid}/{endpoint} messages
static const char* SIGNET_URI_POLL      = "poll";      // For /poll discovery messages

// Fixed administrative multicast addresses (Appendix A)
static const char* MULTICAST_NODE_SEND_IP = "239.254.255.253";  // /sig-net/<version>/<scope>/node/{tuid}/{endpoint}
static const char* MULTICAST_MANAGER_POLL_IP = "239.254.255.252";  // /sig-net/<version>/<scope>/poll
static const char* MULTICAST_MANAGER_SEND_IP = "239.254.255.251";  // manager/{tuid}/{endpoint} -- manager commands to node
static const char* MULTICAST_TIME_IP         = "239.254.255.250";  // sync + timecode/{stream} -- time distribution
static const char* MULTICAST_NODE_BEACON_IP  = "239.254.255.255";  // node_beacon/{tuid} -- unprovisioned node beacon
static const char* MULTICAST_NODE_LOST_IP    = "239.254.255.254";  // node_lost/{tuid} -- node leaving network

//------------------------------------------------------------------------------
// Key Derivation Info Strings (Section 7.3)
//------------------------------------------------------------------------------

static const char* HKDF_INFO_SENDER          = "Sig-Net-Sender-v1";
static const char* HKDF_INFO_CITIZEN         = "Sig-Net-Citizen-v1";
static const char* HKDF_INFO_MANAGER_GLOBAL  = "Sig-Net-Manager-v1";
static const char* HKDF_INFO_MANAGER_LOCAL_PREFIX = "Sig-Net-Manager-v1-";  // Append 12-char hex TUID

//------------------------------------------------------------------------------
// Error Codes
//------------------------------------------------------------------------------

static const int32_t SIGNET_SUCCESS                =  0;
static const int32_t SIGNET_ERROR_INVALID_ARG      = -1;  // Invalid argument
static const int32_t SIGNET_ERROR_BUFFER_FULL      = -2;  // Packet buffer overflow
static const int32_t SIGNET_ERROR_CRYPTO           = -3;  // Cryptographic operation failed
static const int32_t SIGNET_ERROR_ENCODE           = -4;  // Encoding error
static const int32_t SIGNET_ERROR_NETWORK          = -5;  // Network transmission failed
static const int32_t SIGNET_ERROR_BUFFER_TOO_SMALL = -6;  // Insufficient data in buffer (parser)
static const int32_t SIGNET_ERROR_INVALID_PACKET   = -7;  // Malformed packet structure
static const int32_t SIGNET_ERROR_INVALID_OPTION   = -8;  // Missing or invalid CoAP option
static const int32_t SIGNET_ERROR_HMAC_FAILED      = -9;  // HMAC verification failed
static const int32_t SIGNET_TEST_FAILURE           = -99; // Self-test failed

//------------------------------------------------------------------------------
// Passphrase Validation Return Codes (Section 7.2.3)
// Used for real-time validation of K0 passphrase entry
//------------------------------------------------------------------------------

static const int32_t SIGNET_PASSPHRASE_VALID                    =  0;  // Passphrase meets all requirements
static const int32_t SIGNET_PASSPHRASE_TOO_SHORT                = -10; // Length < 10 characters
static const int32_t SIGNET_PASSPHRASE_TOO_LONG                 = -11; // Length > 64 characters
static const int32_t SIGNET_PASSPHRASE_INSUFFICIENT_CLASSES     = -12; // < 3 character classes used
static const int32_t SIGNET_PASSPHRASE_CONSECUTIVE_IDENTICAL    = -13; // > 2 consecutive identical chars
static const int32_t SIGNET_PASSPHRASE_CONSECUTIVE_SEQUENTIAL   = -14; // > 3 consecutive sequential chars

//------------------------------------------------------------------------------
// Passphrase to K0 Derivation Parameters (Section 7.2.3)
// PBKDF2-HMAC-SHA256 configuration for converting passphrases to K0
//------------------------------------------------------------------------------

static const char* PBKDF2_SALT = "Sig-Net-K0-Salt-v1";  // Fixed 18-byte salt for PBKDF2
static const uint32_t PBKDF2_ITERATIONS = 100000;       // PBKDF2 iteration count
static const uint32_t PASSPHRASE_MIN_LENGTH = 10;       // Minimum passphrase length
static const uint32_t PASSPHRASE_MAX_LENGTH = 64;       // Maximum passphrase length
static const uint32_t PASSPHRASE_GENERATED_LENGTH = 10; // Generated passphrase length

// Passphrase character sets (shared between validation and generator)
static const char* PASSPHRASE_SYMBOLS = "!@#$%^&*()-_=+[]{}|;:',.<>?/";  // Allowed symbols for validation
static const char* PASSPHRASE_GEN_UPPERCASE = "ABCDEFGHJKLMNPQRSTUVWXYZ"; // Removed I, O for clarity
static const char* PASSPHRASE_GEN_LOWERCASE = "abcdefghjkmnpqrstuvwxyz";  // Removed i, l, o for clarity
static const char* PASSPHRASE_GEN_DIGITS = "23456789";                     // Removed 0, 1 for clarity
static const char* PASSPHRASE_GEN_SYMBOLS = "!@#$%^&*-_=+";

//------------------------------------------------------------------------------
// Test K0 for Development/Testing
// K0: 32 bytes (64 hex chars)
//------------------------------------------------------------------------------
static const char* TEST_K0 = "52fcc2e7749f40358ba00b1d557dc11861e89868e139f23014f6a0cfe59cf173";
static const char* TEST_PASSPHRASE = "Ge2p$E$4*A";

//------------------------------------------------------------------------------
// Test TUID for Development/Testing
// Format: Manufacturer Code (2 bytes) + Device ID (4 bytes)
// 'S' 'L' (Singularity) = 0x534C + 000001
//------------------------------------------------------------------------------
static const char* TEST_TUID = "534C00000001";

//------------------------------------------------------------------------------
// SoemCode Assignments (Manufacturer + Product Variant)
//------------------------------------------------------------------------------
static const uint32_t SoemCodeNetWorkshop = 0x534c0001;
static const uint32_t SoemCodeSdkLevelTx  = 0x534c0010;
static const uint32_t SoemCodeSdkPoller   = 0x534c0011;
static const uint32_t SoemCodeSdkNode     = 0x534c0012;

//------------------------------------------------------------------------------
// Role Capability Bit Positions (TID_RT_ROLE_CAPABILITY, Section 11.6.9)
//------------------------------------------------------------------------------

static const uint8_t ROLE_CAP_NODE    = 0x01;  // Bit 0: Node role supported
static const uint8_t ROLE_CAP_SENDER  = 0x02;  // Bit 1: Sender role supported
static const uint8_t ROLE_CAP_MANAGER = 0x04;  // Bit 2: Manager role supported

//------------------------------------------------------------------------------
// Device Status Bit Positions (TID_RT_STATUS, Section 11.6.8)
//------------------------------------------------------------------------------

static const uint32_t RT_STATUS_HW_FAULT     = 0x00000001;  // Bit 0: Hardware fault
static const uint32_t RT_STATUS_FACTORY_BOOT = 0x00000002;  // Bit 1: Booted from factory defaults
static const uint32_t RT_STATUS_CONFIG_LOCK  = 0x00000004;  // Bit 2: Configuration locked via local UI

//------------------------------------------------------------------------------
// Endpoint Capability Bit Positions (TID_EP_CAPABILITY, Section 11.7.4)
//------------------------------------------------------------------------------

static const uint8_t EP_CAP_CONSUME_LEVEL = 0x01;  // Bit 0: Can consume TID_LEVEL
static const uint8_t EP_CAP_SUPPLY_LEVEL  = 0x02;  // Bit 1: Can supply TID_LEVEL
static const uint8_t EP_CAP_CONSUME_RDM   = 0x04;  // Bit 2: Can consume RDM
static const uint8_t EP_CAP_SUPPLY_RDM    = 0x08;  // Bit 3: Can supply RDM
static const uint8_t EP_CAP_VIRTUAL       = 0x10;  // Bit 4: Virtual endpoint (internal RDM responder)

//------------------------------------------------------------------------------
// Endpoint Direction Bit Positions (TID_EP_DIRECTION, Section 11.7.5)
// Bits 0-1 are the EpDirection enum below; Bit 2 is RDM enable
//------------------------------------------------------------------------------

static const uint8_t EP_DIR_RDM_ENABLE = 0x04;  // Bit 2: RDM processing enabled on this endpoint

//------------------------------------------------------------------------------
// Endpoint Status Bit Positions (TID_EP_STATUS, Section 11.7.7)
//------------------------------------------------------------------------------

static const uint32_t EP_STATUS_DATA_ACTIVE = 0x00000001;  // Bit 0: Data activity on endpoint
static const uint32_t EP_STATUS_HW_FAULT    = 0x00000002;  // Bit 1: Hardware fault
static const uint32_t EP_STATUS_CONFIG_LOCK = 0x00000004;  // Bit 2: Configuration locked via local UI

//------------------------------------------------------------------------------
// Magic Word Constants
//------------------------------------------------------------------------------

static const uint32_t UNPROVISION_MAGIC_WORD = 0x57495045;  // ASCII "WIPE" -- required payload for TID_RT_UNPROVISION
static const uint32_t REBOOT_MAGIC_WORD      = 0x424F4F54;  // ASCII "BOOT" -- required prefix for TID_RT_REBOOT

//------------------------------------------------------------------------------
// TID Payload Enumerations
//------------------------------------------------------------------------------

// TID_TIMECODE type byte values (Section 11.2.4)
enum TimecodeType {
	TIMECODE_FILM         = 0,    // Film: 24 fps
	TIMECODE_EBU          = 1,    // EBU: 25 fps
	TIMECODE_DROP_FRAME   = 2,    // Drop Frame: 29.97 fps
	TIMECODE_SMPTE        = 3     // SMPTE: 30 fps
};

// TID_RDM_TOD_CONTROL command byte values (Section 11.3.3)
enum TodControlCommand {
	TOD_CONTROL_SEND_DATA  = 0x00,  // Force node to send TID_RDM_TOD_DATA immediately
	TOD_CONTROL_FLUSH_FULL = 0x01   // Flush ToD and force a full RDM discovery cycle
};

// TID_NW_IPV4_MODE mode byte values (Section 11.5.2)
enum Ipv4Mode {
	IPV4_MODE_STATIC = 0x00,  // Use static IPv4 address configuration
	IPV4_MODE_DHCP   = 0x01   // Obtain IPv4 address via DHCP
};

// TID_NW_IPV6_MODE mode byte values (Section 11.5.7)
enum Ipv6Mode {
	IPV6_MODE_STATIC  = 0x00,  // Use static IPv6 address configuration
	IPV6_MODE_SLAAC   = 0x01,  // Stateless Address Autoconfiguration (SLAAC)
	IPV6_MODE_DHCPV6  = 0x02   // Obtain IPv6 address via DHCPv6
};

// TID_RT_MULT / TID_RT_MULT_OVERRIDE global routing state values (Section 11.6.6)
enum MultRoutingState {
	MULT_STATE_DEFAULT = 0x00,  // All endpoints using default folded multicast pool
	MULT_STATE_CUSTOM  = 0x01   // One or more endpoints using a custom multicast IP override
};

// TID_RT_IDENTIFY state byte values (Section 11.6.7)
enum IdentifyState {
	IDENTIFY_OFF    = 0x00,  // Normal operation
	IDENTIFY_SUBTLE = 0x01,  // Subtle: safe for show conditions
	IDENTIFY_FULL   = 0x02,  // Full: obvious identification
	IDENTIFY_MUTE   = 0x03   // Mute: suppress all indicators (dark-sky)
};

// TID_RT_REBOOT command type byte values (Section 11.6.10)
enum RebootType {
	REBOOT_HARDWARE = 0xFF,  // Hardware (cold) reset
	REBOOT_WARM     = 0xFE   // Warm (soft) reboot
};

// TID_EP_DIRECTION direction field (Bits 0-1) values (Section 11.7.5)
enum EpDirection {
	EP_DIR_DISABLED  = 0x00,  // Port disabled
	EP_DIR_CONSUMER  = 0x01,  // Consumer: receives Sig-Net, outputs physical DMX
	EP_DIR_SUPPLIER  = 0x02,  // Supplier: receives physical DMX, transmits Sig-Net
	EP_DIR_FALLBACK  = 0x03   // Fallback: monitor input, fall back to consumer output on loss
};

// TID_EP_FAILOVER mode byte values (Section 11.7.8)
enum FailoverMode {
	FAILOVER_HOLD_LAST  = 0x00,  // Hold last received state (default)
	FAILOVER_BLACKOUT   = 0x01,  // All slots to 0 (blackout)
	FAILOVER_FULL       = 0x02,  // All slots to 255 (full)
	FAILOVER_PLAY_SCENE = 0x03,  // Execute internal preset or scene
	FAILOVER_STOP_DMX   = 0x04   // Stop generating DMX frames
};

// TID_EP_DMX_TIMING transmission mode byte values (Section 11.7.9)
enum DmxTransmitMode {
	DMX_TIMING_CONTINUOUS = 0x00,  // Continuous full-frame transmission
	DMX_TIMING_DELTA      = 0x01   // Delta / change-only transmission
};

// TID_EP_DMX_TIMING output timing byte values (Section 11.7.9)
enum DmxOutputTiming {
	DMX_OUTPUT_MAXIMUM = 0x00,  // Maximum DMX512 refresh rate
	DMX_OUTPUT_MEDIUM  = 0x01,  // Medium DMX512 refresh rate
	DMX_OUTPUT_MINIMUM = 0x02   // Minimum DMX512 refresh rate
};

// TID_DG_SECURITY_EVENT event code values (Section 11.8.1)
enum SecurityEventCode {
	SECURITY_EVENT_HMAC_FAIL      = 0x0001,  // HMAC-SHA256 verification failure
	SECURITY_EVENT_REPLAY         = 0x0002,  // Replay attack: sequence or session anomaly
	SECURITY_EVENT_DOS_LIMITING   = 0x0003,  // DoS rate-limiting activated
	SECURITY_EVENT_UNPROV_ATTEMPT = 0x0004   // Unauthorised provisioning attempt
};

// TID_DG_SECURITY_EVENT source address type values (Section 11.8.1)
enum SecurityEventAddrType {
	SECURITY_ADDR_NONE = 0x00,  // No source address
	SECURITY_ADDR_IPV4 = 0x01,  // IPv4 address (4 bytes)
	SECURITY_ADDR_IPV6 = 0x02   // IPv6 address (16 bytes)
};

} // namespace SigNet

#endif // SIGNET_CONSTANTS_HPP
