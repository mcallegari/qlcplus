/*
  Q Light Controller
  fx5device.h

  Copyright (c)	Florian Euchner

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

#ifndef FX5DEVICE_H
#define FX5DEVICE_H

#include <QString>
#include <QDebug>

extern "C" {
    #include "fx5driver.h"
}

class FX5;

class FX5Device : public QObject
{
    Q_OBJECT

public:

    /*
        Initialization
    */

    FX5Device(char *serial, quint32 version, quint32 port);
    ~FX5Device();

    /*
        User Information
    */
    QString getName();
    QString getInfoText();

    /* The FX5Device class will automatically select the right mode for the interface
       based on calls to openOutput / openInput / closeOutput / closeInput            */
    void openOutput();
    void openInput();

    void closeOutput();
    void closeInput();

    void outputDMX(const QByteArray &universe);

    void notifyInput(FX5 *sender);

private:
    void updateMode();

    char m_serial[17]; // SN of the device
    quint32 m_version; // Version of the device
    quint32 m_port;    // Port number (from 0 - 32)

    TDMXArray m_dmx_in;  // Input  Environment
    TDMXArray m_dmx_out; // Output Environment
    uint8_t m_dmx_instate[512]; // saves last known input environment

    enum FX5mode
    {
        FX5_MODE_NONE   = 1 << 0,
        FX5_MODE_OUTPUT = 1 << 1,
        FX5_MODE_INPUT  = 1 << 2
    };

    int m_mode;
    bool m_link_open; // saves if the interface Link has been established
};

#endif
