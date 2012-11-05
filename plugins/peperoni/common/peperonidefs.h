/*
  Q Light Controller
  peperonidefs.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
