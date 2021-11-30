/*
  Q Light Controller Plus

  serial-osc-bridge-utils.h

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
#ifndef SERIAL_OSC_BRIDGE_UTILS_H
#define SERIAL_OSC_BRIDGE_UTILS_H

#include <QtGlobal>
#include <QString>

QString int_to_hex(unsigned int value, int width=4);
inline static QString byte_to_hex(unsigned char value) { return int_to_hex(value,2); }
QString int_to_str(unsigned int value);

/*
Converts an unsigned integer to a thounsand-separated string,
e.g.   10045 -> "10,045"
*/
QString int_to_str_group_sep(unsigned int value);

/*
Converts an unsigned integer to a thounsand-separated string,
e.g.   10045 -> "10KB"
       4000000 -> "4MB"
*/
QString int_to_data_human_sizes(unsigned int value);

/*
Returns TRUE if the given string contains a valid network host address
e.g.  "127.0.0.1" => true
*/
bool valid_host_address(const QString& s);

/*
Returns TRUE if the given integer value is a valud UART baud rate, one of:
    1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
While other values might be technically valid, they are not accepted by
this function.
*/
bool valid_baudrate(int i);


/*
Returns TRUE if the given string points a valid filename,
with a VERY NARROW definition of "valid".
This is mostly meant to accept valid unix device filenames (e.g. "/dev/ttyFOO3").
The requirements are:
1. The path must be absolute (not relative)
2. The basename and extension must contain ONLY the following characters: A-Za-z0-9-_.
   (other characters are rejected, even if valid in the file system / operating system.
3. The directory MUST exist.
*/
bool valid_filename(QString s);

#endif
