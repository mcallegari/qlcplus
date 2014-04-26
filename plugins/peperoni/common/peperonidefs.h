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

#define _DMX_RX  (1<<2) /** receiver commands */
#define _DMX_TX  (2<<2) /** transmitter commands */

#define _DMX_MEM       0
#define _DMX_STARTCODE 1
#define _DMX_SLOTS     2
#define _DMX_FRAMES    3

/** CONTROL MESSAGES: Request function */
#define PEPERONI_ID_LED          0x02   /** Read/write led usage */
#define PEPERONI_TX_MEM_REQUEST  (_DMX_TX | _DMX_MEM)       /** Write into transmitter data memory */
#define PEPERONI_TX_SLOTS        (_DMX_TX | _DMX_SLOTS)     /** Write transmitter slot counter */
#define PEPERONI_TX_STARTCODE    (_DMX_TX | _DMX_STARTCODE) /** Write transmitter startcode */
#define PEPERONI_TX_FRAMES       (_DMX_TX | _DMX_FRAMES)    /** Read transmitter frame counter */
#define PEPERONI_RX_MEM          (_DMX_RX | _DMX_MEM)       /** Read/write receiver data memory */
#define PEPERONI_RX_SLOTS        (_DMX_RX | _DMX_SLOTS)     /** Read/write receiver slots counter */
#define PEPERONI_RX_STARTCODE    (_DMX_RX | _DMX_STARTCODE) /** Read/write receiver startcode */
#define PEPERONI_RX_FRAMES       (_DMX_RX | _DMX_FRAMES)    /** Read receiver frame counter */

/** CONTROL MSG: Block until the DMX frame has been completely transmitted */
#define PEPERONI_TX_MEM_BLOCK    0x01
/** CONTROL MSG: Do not block during DMX frame send */
#define PEPERONI_TX_MEM_NONBLOCK 0x00
/** CONTROL MSG: Oldest firmware version with blocking write support */
#define PEPERONI_FW_BLOCKING_WRITE_SUPPORT 0x101

/** BULK WRITE: Bulk out endpoint */
#define PEPERONI_BULK_OUT_ENDPOINT 0x02
/** BULK WRITE: Bulk int endpoint */
#define PEPERONI_BULK_IN_ENDPOINT 0x82

/** BULK WRITE: Oldest firmware version with bulk pipe support */
#define PEPERONI_FW_OLD_BULK_SUPPORT 0x400

/** BULK WRITE: Newest firmware version with bulk pipe support */
#define PEPERONI_FW_NEW_BULK_SUPPORT 0x500

/** BULK PIPE: Size of the "old" bulk header */
#define PEPERONI_OLD_BULK_HEADER_SIZE 4
/** BULK PIPE: "Old" bulk protocol ID */
#define PEPERONI_OLD_BULK_HEADER_ID 0x01

/** BULK PIPE: "Old" commands */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_TX_SET   0x00   /** Write the transmitter memory */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_TX_GET   0x01   /** Read the transmitter memory */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_RX_SET   0x02   /** Write the receiver memory */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_RX_GET   0x03   /** Read the receiver memory */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_TX2_SET  0x04   /** Write the second universes transmitter memory */
#define PEPERONI_OLD_BULK_HEADER_REQUEST_TX2_GET  0x05   /** Read the second universes transmitter memory */

#endif
