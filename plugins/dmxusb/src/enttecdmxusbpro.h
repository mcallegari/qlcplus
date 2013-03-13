/*
  Q Light Controller
  enttecdmxusbpro.h

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#ifndef ENTTECDMXUSBPRO_H
#define ENTTECDMXUSBPRO_H

#ifdef WIN32
#   include <windows.h>
#endif

#include <QByteArray>
#include <QObject>

#include "dmxusbwidget.h"

#define ENTTEC_PRO_DMX_ZERO      char(0x00)
#define ENTTEC_PRO_RECV_DMX_PKT  char(0x05)
#define ENTTEC_PRO_SEND_DMX_RQ   char(0x06)
#define ENTTEC_PRO_READ_SERIAL   char(0x0a)
#define ENTTEC_PRO_ENABLE_API2   char(0x0d)
#define ENTTEC_PRO_SEND_DMX_RQ2  char(0xa9)
#define ENTTEC_PRO_PORT_ASS_REQ  char(0xcb)
#define ENTTEC_PRO_START_OF_MSG  char(0x7e)
#define ENTTEC_PRO_END_OF_MSG    char(0xe7)

/**
 * This is the base interface class for ENTTEC USB DMX Pro widgets.
 */
class EnttecDMXUSBPro : public DMXUSBWidget
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    EnttecDMXUSBPro(const QString& serial, const QString& name, const QString& vendor,
                    QLCFTDI *ftdi = NULL, quint32 id = 0);
    virtual ~EnttecDMXUSBPro();

    /****************************************************************************
     * Open & Close
     ****************************************************************************/
public:
    /** @reimp */
    virtual bool open();

    /************************************************************************
     * Name & Serial
     ************************************************************************/
public:
    /** @reimp */
    virtual QString uniqueName() const;

private:
    /** Extract the widget's unique serial number (printed on the bottom) */
    bool extractSerial();

private:
    QString m_proSerial;
};

#endif
