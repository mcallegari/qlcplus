/*
  Q Light Controller Plus

  serial-osc-bridge-settings.h

  Copyright (c) House Gordon Software Company LTD

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#ifndef SERIAL_OSC_BRIDGE_SETTINGS_H
#define SERIAL_OSC_BRIDGE_SETTINGS_H

#include <QHostAddress>

enum SerialOscBridgeInputType
  {
   InputType_None = 0,
   InputType_USB_Device = 1,
   InputType_Serial_Device =2,
  };

#define SOSCB_INPUT_TYPE "inputType"
#define SOSCB_VENDOR_ID "vendorId"
#define SOSCB_PRODUCT_ID "productId"
#define SOSCB_SERIAL_DEVICE "serialDevice"
#define SOSCB_BAUDRATE "baudrate"
#define SOSCB_FRAME_MARKER_1 "frameMarker1"
#define SOSCB_FRAME_MARKER_2 "frameMarker2"
#define SOSCB_FRAME_MARKER_3 "frameMarker3"
#define SOSCB_FRAME_MARKER_4 "frameMarker4"
#define SOSCB_OSC_UNIVERSE_NUM "oscUniverseNum"
#define SOSCB_OSC_HOST_IP "oscHostIp"
#define SOSCB_OSC_BASE_UDP_PORT "oscBaseUdpPort"
#define SOSCB_MAX_INPUT_BUFFER_SIZE "maxInputBufferSize"

class SerialOscBridgeSettings
{
public:
  enum SerialOscBridgeInputType input_type;

  // For USB Device Auto detection
  quint16 vendor_id;
  quint16 product_id;

  // For fixed serial device filename
  QString serial_device_filename;

  unsigned int baudrate;

  unsigned char frame_marker[4];

  QHostAddress osc_server_ip  ;
  quint16 osc_server_base_udp_port;
  unsigned int osc_universe_num;

  int max_input_buffer_size;

public:
  SerialOscBridgeSettings();

  void ResetToDefaults();
  bool input_values_ready();
};

#endif
