/*
  Q Light Controller Plus
  hidjsdevice.h

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#ifndef HIDJSDEVICE_H
#define HIDJSDEVICE_H

#include <QObject>
#include <QFile>
#include <QHash>

#include "qlcmacros.h"
#include "hiddevice.h"
#include "hidapi.h"

class HIDPlugin;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDJsDevice : public HIDDevice
{
    Q_OBJECT

public:
    HIDJsDevice(HIDPlugin* parent, quint32 line, struct hid_device_info *info);
    virtual ~HIDJsDevice();

protected:
    /** Initialize the device, find out its capabilities etc.
      * This is a pure virtual method because every subclass has
      * its own platform specific initialization */
    virtual void init() = 0;

    /** @reimp */
    bool hasInput() { return true; }

protected:
    struct hid_device_info *m_dev_info;
    unsigned char m_axesNumber;
    unsigned char m_buttonsNumber;

    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** @reimp */
    virtual bool openInput();

    /** @reimp */
    void closeInput();

    /** @reimp */
    QString path() const;

    /** @reimp */
    virtual bool readEvent() ;

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
    virtual void run();
};

#endif
