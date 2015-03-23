/*
  Q Light Controller Plus
  euroliteusbdmxpro.h

  Copyright (C) Karri Kaksonen based on nanodmx.h by Massimo Callegari

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

#ifndef EUROLITEUSBDMXPRO_H
#define EUROLITEUSBDMXPRO_H

#include <QFile>

#include "dmxusbwidget.h"

#define EUROLITE_USB_DMX_PRO_DMX_ZERO      char(0x00)
#define EUROLITE_USB_DMX_PRO_SEND_DMX_RQ   char(0x06)
#define EUROLITE_USB_DMX_PRO_START_OF_MSG  char(0x7E)
#define EUROLITE_USB_DMX_PRO_END_OF_MSG    char(0xE7)

class QLCFTDI;

class EuroliteUSBDMXPro : public DMXUSBWidget
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    EuroliteUSBDMXPro(const QString& serial, const QString& name, const QString& vendor,
            void *usb_ref, quint32 id = 0);
    virtual ~EuroliteUSBDMXPro();

    /** @reimp */
    DMXUSBWidget::Type type() const;

    /************************************************************************
     * Widget functions
     ************************************************************************/
public:
    /** @reimp */
    bool open(quint32 line = 0, bool input = false);

    /** @reimp */
    bool close(quint32 line = 0, bool input = false);

    /** @reimp */
    QString uniqueName(ushort line = 0, bool input = false) const;

    /** @reimp */
    QString additionalInfo() const;

    /** @reimp */
    bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

private:
    QString getDeviceName();

private:
    /** File handle for /dev/ttyACMx */
    QFile m_file;

#ifdef LIBFTDI1
    libusb_device *m_device;
#else
    struct usb_device *m_device;
#endif

    QByteArray m_universe;
};

#endif
