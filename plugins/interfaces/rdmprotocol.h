/*
  Q Light Controller Plus
  rdmprotocol.h

  Copyright (c) Massimo Callegari

  Licensed under the Apache License Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing software
  distributed under the License is distributed on an "AS IS" BASIS
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QVariantList>
#include <QByteArray>

//#define DEBUG_RDM

#define RDM_START_CODE              0xCC
#define RDM_SC_SUB_MESSAGE          0x01

// RDM Commands - E1.20 Table A-1
#define DISCOVERY_COMMAND           0x10
#define DISCOVERY_COMMAND_RESPONSE  0x11
#define GET_COMMAND                 0x20
#define GET_COMMAND_RESPONSE        0x21
#define SET_COMMAND                 0x30
#define SET_COMMAND_RESPONSE        0x31

// RDM response types - E1.20 Table A-2
#define RESPONSE_TYPE_ACK           0x00
#define RESPONSE_TYPE_ACK_TIMER     0x01
#define RESPONSE_TYPE_NACK_REASON   0x02
#define RESPONSE_TYPE_ACK_OVERFLOW  0x03

// RDM PIDs - E1.20 Table A-3
#define PID_DISC_UNIQUE_BRANCH      0x0001
#define PID_DISC_MUTE               0x0002
#define PID_DISC_UN_MUTE            0x0003

// network management
#define PID_PROXIED_DEVICES         0x0010
#define PID_PROXIED_DEVICE_COUNT    0x0011
#define PID_COMMS_STATUS            0x0015

// status collection
#define PID_QUEUED_MESSAGE          0x0020
#define PID_STATUS_MESSAGES         0x0030
#define PID_STATUS_ID_DESCRIPTION   0x0031
#define PID_CLEAR_STATUS_ID         0x0032
#define PID_SUB_DEVICE_STATUS_REPORT_THRESHOLD 0x0033

// RDM information
#define PID_SUPPORTED_PARAMETERS    0x0050
#define PID_PARAMETER_DESCRIPTION   0x0051

// production information
#define PID_DEVICE_INFO             0x0060
#define PID_PRODUCT_DETAIL_ID_LIST  0x0070
#define PID_DEVICE_MODEL_DESCRIPTION 0x0080
#define PID_MANUFACTURER_LABEL      0x0081
#define PID_DEVICE_LABEL            0x0082
#define PID_FACTORY_DEFAULTS        0x0090
#define PID_LANGUAGE_CAPABILITIES   0x00A0
#define PID_LANGUAGE                0x00B0
#define PID_SOFTWARE_VERSION_LABEL  0x00C0
#define PID_BOOT_SOFTWARE_VERSION_ID 0x00C1
#define PID_BOOT_SOFTWARE_VERSION_LABEL 0x00C2

// DMX512
#define PID_DMX_PERSONALITY         0x00E0
#define PID_DMX_PERSONALITY_DESCRIPTION 0x00E1
#define PID_DMX_START_ADDRESS       0x00F0
#define PID_SLOT_INFO               0x0120
#define PID_SLOT_DESCRIPTION        0x0121
#define PID_DEFAULT_SLOT_VALUE      0x0122

// sensors
#define PID_SENSOR_DEFINITION       0x0200
#define PID_SENSOR_VALUE            0x0201
#define PID_RECORD_SENSORS          0x0202

// power/lamp settings
#define PID_DEVICE_HOURS            0x0400
#define PID_LAMP_HOURS              0x0401
#define PID_LAMP_STRIKES            0x0402
#define PID_LAMP_STATE              0x0403
#define PID_LAMP_ON_MODE            0x0404
#define PID_DEVICE_POWER_CYCLES     0x0405

// display settings
#define PID_DISPLAY_INVERT          0x0500
#define PID_DISPLAY_LEVEL           0x0501

// configuration
#define PID_PAN_INVERT              0x0600
#define PID_TILT_INVERT             0x0601
#define PID_PAN_TILT_SWAP           0x0602
#define PID_REAL_TIME_CLOCK         0x0603

// control
#define PID_IDENTIFY_DEVICE         0x1000
#define PID_RESET_DEVICE            0x1001
#define PID_POWER_STATE             0x1010
#define PID_PERFORM_SELFTEST        0x1020
#define PID_SELF_TEST_DESCRIPTION   0x1021
#define PID_CAPTURE_PRESET          0x1030
#define PID_PRESET_PLAYBACK         0x1031

// E1.37-1 PIDS
// DMX512 setup
#define PID_DMX_BLOCK_ADDRESS       0x0140
#define PID_DMX_FAIL_MODE           0x0141
#define PID_DMX_STARTUP_MODE        0x0142

// Dimmer Settings
#define PID_DIMMER_INFO             0x0340
#define PID_MINIMUM_LEVEL           0x0341
#define PID_MAXIMUM_LEVEL           0x0342
#define PID_CURVE                   0x0343
#define PID_CURVE_DESCRIPTION       0x0344

// Control
#define PID_OUTPUT_RESPONSE_TIME    0x0345
#define PID_OUTPUT_RESPONSE_TIME_DESCRIPTION 0x0346
#define PID_MODULATION_FREQUENCY    0x0347
#define PID_MODULATION_FREQUENCY_DESCRIPTION 0x0348

// Power/Lamp Settings
#define PID_BURN_IN                 0x0440

// Configuration
#define PID_LOCK_PIN                0x0640
#define PID_LOCK_STATE              0x0641
#define PID_LOCK_STATE_DESCRIPTION  0x0642
#define PID_IDENTIFY_MODE           0x1040
#define PID_PRESET_INFO             0x1041
#define PID_PRESET_STATUS           0x1042
#define PID_PRESET_MERGEMODE        0x1043
#define PID_POWER_ON_SELF_TEST      0x1044

#define QLCPLUS_ESTA_ID             0x7FF8
#define QLCPLUS_DEVICE_ID           0x01090709
#define BROADCAST_ESTA_ID           0xFFFF
#define BROADCAST_DEVICE_ID         0xFFFFFFFF

class RDMProtocol
{
public:

    RDMProtocol();

    /** Set a specific ESTA ID */
    void setEstaID(quint16 id);

    /** Set a specific device ID */
    void setDeviceId(quint32 id);

    /**
     * Create a byte array conforming to the E1.20 standard,
     * suitabile for transmission via a QLC+ plugin
     */
    bool packetizeCommand(ushort command, QVariantList params, bool startCode, QByteArray &buffer);

    bool parseDiscoveryReply(const QByteArray &buffer, QVariantMap &values);

    bool parsePacket(const QByteArray &buffer, QVariantMap &values);

    /** Convert a byte array to a string UID */
    static QString byteArrayToUID(QByteArray buffer, quint16 &ESTAId, quint32 &deviceId);

    /** Convenience method to get a broadcast UID as string */
    static QString broadcastAddress();

    /** Return a PID as a string */
    static QString pidToString(quint16 pid);

    /** Return the RDM command reply as a string */
    static QString responseToString(quint8 response);

    /** Return the device info category as string */
    static QString categoryToString(quint16 category);

private:
    QByteArray UIDToByteArray(quint16 ESTAId, quint32 deviceId);
    QByteArray shortToByteArray(quint16 data);
    QByteArray longToByteArray(quint32 data);
    quint16 byteArrayToShort(const QByteArray &buffer, int index);
    quint32 byteArrayToLong(const QByteArray &buffer, int index);
    quint16 calculateChecksum(bool startCode, const QByteArray &ba, int len);

protected:
    quint16 m_estaID;
    quint32 m_deviceID;

    /** The RDM transaction number */
    quint8 m_transactionNum;
};
