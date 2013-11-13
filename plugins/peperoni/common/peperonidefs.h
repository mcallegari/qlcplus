/*
  Q Light Controller
  peperonidefs.h

  Copyright (c) Heikki Junnila

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

#ifndef PEPERONIDEFS_H
#define PEPERONIDEFS_H

/** Lighting Solutions/Peperoni Light Vendor ID */
#define PEPERONI_VID            0x0CE1

/* Recognized Product IDs */
#define PEPERONI_PID_XSWITCH    0x0001
#define PEPERONI_PID_RODIN1     0x0002
#define PEPERONI_PID_RODIN2     0x0003
#define PEPERONI_PID_RODINT     0x0008
#define PEPERONI_PID_USBDMX21   0x0004

/** Common interface */
#define PEPERONI_IFACE_EP0      0x00

#define PEPERONI_CONF_TXONLY    0x01
#define PEPERONI_CONF_TXRX      0x02
#define PEPERONI_CONF_RXONLY    0x03

/** CONTROL MSG: Control the internal DMX buffer */
#define PEPERONI_TX_MEM_REQUEST  0x04
/** CONTROL MSG: Set DMX startcode */
#define PEPERONI_TX_STARTCODE    0x09
/** CONTROL MSG: Block until the DMX frame has been completely transmitted */
#define PEPERONI_TX_MEM_BLOCK    0x01
/** CONTROL MSG: Do not block during DMX frame send */
#define PEPERONI_TX_MEM_NONBLOCK 0x00
/** CONTROL MSG: Oldest firmware version with blocking write support */
#define PEPERONI_FW_BLOCKING_WRITE_SUPPORT 0x101

/** BULK WRITE: Bulk out endpoint */
#define PEPERONI_BULK_OUT_ENDPOINT 0x02
/** BULK WRITE: Oldest firmware version with bulk write support */
#define PEPERONI_FW_BULK_SUPPORT 0x400
/** BULK WRITE: Size of the "old" bulk header */
#define PEPERONI_OLD_BULK_HEADER_SIZE 4
/** BULK WRITE: "Old" bulk protocol ID */
#define PEPERONI_OLD_BULK_HEADER_ID 0x01
/** BULK WRITE: "Old" bulk transmit request */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_TX 0x00

#endif
