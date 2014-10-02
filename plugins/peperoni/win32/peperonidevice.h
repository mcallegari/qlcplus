/*
  Q Light Controller
  peperonidevice.h

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

#ifndef PEPERONIDEVICE_H
#define PEPERONIDEVICE_H

#include <Windows.h>
#include <QObject>
#include <QMutex>

class PeperoniDevice : public QObject
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    PeperoniDevice(QObject* parent, struct usbdmx_functions* usbdmx,
                   int output);
    virtual ~PeperoniDevice();

protected:
    struct usbdmx_functions* m_usbdmx;

    /********************************************************************
     * Properties
     ********************************************************************/
public:
    QString name() const;
    int output() const;
    QString infoText() const;

protected:
    void extractName();

protected:
    QString m_name;
    int m_output;
    bool m_deviceOK;

    /********************************************************************
     * Open & close
     ********************************************************************/
public:
    /** Open this device for DMX output */
    bool open();

    /** Close this device */
    void close();

    /** Re-extract the device's name and reopen it if necessary */
    void rehash();

    HANDLE handle() const;

protected:
    HANDLE m_handle;

    /********************************************************************
     * Write
     ********************************************************************/
public:
    void outputDMX(const QByteArray& universe);
};

#endif
