/*
  Q Light Controller Plus

  serial-osc-bridge-settings.cpp

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
#include <QFileInfo>
#include <QHostAddress>
#include <QRegularExpression>

#include "serial-osc-bridge-utils.h"
#include "serial-osc-bridge-settings.h"



SerialOscBridgeSettings::SerialOscBridgeSettings() :
    input_type(InputType_None),
    vendor_id(0),
    product_id(0),
    baudrate(0),
    osc_server_base_udp_port(0),
    osc_universe_num(0),
    max_input_buffer_size(0)
{
  frame_marker[0] = 0;
  frame_marker[1] = 0;
  frame_marker[2] = 0;
  frame_marker[3] = 0;
}

void SerialOscBridgeSettings::ResetToDefaults()
{
  input_type = InputType_None ;

  vendor_id = 0 ;
  product_id = 0 ;

  baudrate = 115200;
  serial_device_filename = "" ;

  osc_server_ip = QHostAddress("127.0.0.1");
  osc_server_base_udp_port = 7699 ; // i.e. "7700 + Universe-number - 1"
  osc_universe_num = 1;

  max_input_buffer_size = 4096 ;

  frame_marker[0] = 0x89;
  frame_marker[1] = 0x98;
  frame_marker[2] = 0x12;
  frame_marker[3] = 0xAB;
}



bool SerialOscBridgeSettings::input_values_ready()
{
  bool common_params_valid =
    (valid_baudrate(baudrate)) &&
    (max_input_buffer_size>0) &&
    (frame_marker[0] != 0) &&
    (frame_marker[1] != 0) &&
    (frame_marker[2] != 0) &&
    (frame_marker[3] != 0);

  switch (input_type)
    {
    case InputType_None:
      return false;

    case InputType_USB_Device:
      return (vendor_id != 0) && (product_id != 0) && common_params_valid;

    case InputType_Serial_Device:
      return (!serial_device_filename.isEmpty())
	&& valid_filename(serial_device_filename)
	&& common_params_valid;

    default:
      Q_ASSERT(!"Should not happen");
    }
}
