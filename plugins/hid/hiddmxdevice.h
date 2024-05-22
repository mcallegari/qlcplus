/*
  Q Light Controller Plus
  hiddmxdevice.h

  Copyright (c) Massimo Callegari
                Florian Euchner
                Stefan Krupop

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

#ifndef HIDDMXDEVICE_H
#define HIDDMXDEVICE_H

#include <QObject>

#include "hiddevice.h"
#include "hidapi.h"

// Vendor IDs provided by http://www.linux-usb.org/usb.ids

#define HID_DMX_INTERFACE_VENDOR_ID    0x04B4  // Cypress Semiconductor Corp.
#define HID_DMX_INTERFACE_PRODUCT_ID   0x0F1F  // Digital Enlightenment USB DMX
#define HID_DMX_INTERFACE_VENDOR_ID_2  0x16C0  // Van Ooijen Technische Informatica
#define HID_DMX_INTERFACE_PRODUCT_ID_2 0x088B  // FX5
#define HID_DMX_INTERFACE_VENDOR_ID_3  0x16D0  // MCS
#define HID_DMX_INTERFACE_PRODUCT_ID_3 0x0830  // DMXControl Projects e.V. Nodle U1
#define HID_DMX_INTERFACE_VENDOR_ID_4  0x16D0  // MCS
#define HID_DMX_INTERFACE_PRODUCT_ID_4 0x0833  // DMXControl Projects e.V. Nodle R4S

#define HID_DMX_READ_TIMEOUT 100

class HIDPlugin;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDDMXDevice : public HIDDevice
{
    Q_OBJECT

public:
    HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString& name, const QString& path);
    virtual ~HIDDMXDevice();

protected:
    /** Initialize the device, find out its capabilities etc. */
    void init();

    /** @reimp */
    bool hasInput() { return true; }

    /** @reimp */
    bool hasOutput() { return true; }

    /** @reimp */
    bool hasMergerMode() { return true; /*DE, FX5, and Nodle have a merger mode*/ }

    /*********************************************************************
     * File operations
     *********************************************************************/
public:

    /** @reimp */
    bool isMergerModeEnabled();

    /** @reimp */
    void enableMergerMode(bool mergerModeEnabled);

    /** @reimp */
    bool openInput();

    /** @reimp */
    void closeInput();

    /** @reimp */
    bool openOutput();

    /** @reimp */
    void closeOutput();

    /** @reimp */
    bool readEvent();

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    /** @reimp */
    QString infoText();

    /*********************************************************************
     * Input data
     *********************************************************************/
public:
    /** @reimp */
    void feedBack(quint32 channel, uchar value);

private:
    /** @reimp */
    void run();

    /*********************************************************************
     * Output data
     *********************************************************************/
public:
    /** @reimp */
    void outputDMX(const QByteArray &data, bool forceWrite = false);

     /*********************************************************************
     * FX5 - specific functions and device handle
     *********************************************************************/
private:
    /** Interface mode specification */
    enum DMXmode
    {
        DMX_MODE_NONE   = 1 << 0,
        DMX_MODE_OUTPUT = 1 << 1,
        DMX_MODE_INPUT  = 1 << 2,
        DMX_MODE_MERGER = 1 << 3
    };

    /** mode selection function */
    void updateMode();

    /** The device current open mode */
    int m_mode;

    /** Last universe data that has been received */
    QByteArray m_dmx_in_cmp;

    /** Last universe data that has been output */
    QByteArray m_dmx_cmp;

    /** device handle for the interface */
    hid_device *m_handle;
};

#endif
