/*
  Q Light Controller Plus
  hidfx5device.h

  Copyright (c) Massimo Callegari
                Florian Euchner

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

#ifndef HIDFX5DEVICE_H
#define HIDFX5DEVICE_H

#include <QObject>

#include "hiddevice.h"
#include "hidapi.h"

#define FX5_DMX_INTERFACE_VENDOR_ID 0x4B4
#define FX5_DMX_INTERFACE_PRODUCT_ID 0xF1F
#define FX5_DMX_INTERFACE_VENDOR_ID_2 0x16C0
#define FX5_DMX_INTERFACE_PRODUCT_ID_2 0x88B

#define FX5_READ_TIMEOUT 100

class HIDEventDevice;
class HID;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDFX5Device : public HIDDevice
{
    Q_OBJECT

public:
    HIDFX5Device(HID* parent, quint32 line, const QString& name, const QString& path);
    virtual ~HIDFX5Device();

protected:
    /** Initialize the device, find out its capabilities etc. */
    void init();

    /** @reimp */
    bool hasInput() { return true; }

    /** @reimp */
    bool hasOutput() { return true; }

    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** @reimp */
    bool openInput();

    /** @reimp */
    void closeInput();
    
    /** @reimp */
    void openOutput();

    /** @reimp */
    void closeOutput();

    /** @reimp */
    QString path() const;

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
    enum FX5mode
    {
        FX5_MODE_NONE   = 1 << 0,
        FX5_MODE_OUTPUT = 1 << 1,
        FX5_MODE_INPUT  = 1 << 2
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
