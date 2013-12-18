/*
  Q Light Controller
  enttecdmxusbpro.h

  Copyright (C) Heikki Junnila

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

#ifndef ENTTECDMXUSBPRO_H
#define ENTTECDMXUSBPRO_H

#if defined(WIN32) || defined(Q_OS_WIN)
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
