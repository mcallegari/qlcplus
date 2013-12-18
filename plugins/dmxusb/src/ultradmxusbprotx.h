/*
  Q Light Controller
  ultradmxusbprotx.h

  Copyright (C) Massimo Callegari

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

#ifndef ULTRADMXUSBPROTX_H
#define ULTRADMXUSBPROTX_H

#include "enttecdmxusbpro.h"

#define SEND_DMX_PORT1          0x64
#define SEND_DMX_PORT2          0x65
#define USB_DEVICE_MANUFACTURER 0x4D
#define USB_DEVICE_NAME         0x4E

#define DMXKING_ESTA_ID         0x6A6B
#define ULTRADMX_DMX512A_DEV_ID 0x00
#define ULTRADMX_PRO_DEV_ID     0x02
#define ULTRADMX_MICRO_DEV_ID   0x03


class UltraDMXUSBProTx : public EnttecDMXUSBPro
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    UltraDMXUSBProTx(const QString& serial, const QString& name, const QString& vendor,
                     int port = 0, QLCFTDI *ftdi = NULL, quint32 id = 0);
    ~UltraDMXUSBProTx();

    /** @reimp */
    Type type() const;

private:
    int m_port;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    QString uniqueName() const;

    /****************************************************************************
     * Name & Serial
     ****************************************************************************/
public:
    /** @reimp */
    QString additionalInfo() const;

    /************************************************************************
     * Write universe
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(const QByteArray& universe);
};

#endif // ULTRADMXUSBPROTX_H
