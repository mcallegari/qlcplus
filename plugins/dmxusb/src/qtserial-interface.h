/*
  Q Light Controller Plus
  qtserial-interface.h

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

#ifndef QTSERIALINTERFACE_H
#define QTSERIALINTERFACE_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "dmxinterface.h"

class QtSerialInterface final : public DMXInterface
{
public:
    QtSerialInterface(const QString& serial, const QString& name, const QString& vendor,
                      quint16 VID, quint16 PID, quint32 id);

    /** Destructor */
    virtual ~QtSerialInterface();

    static QList<DMXInterface *> interfaces(QList<DMXInterface *> discoveredList);

    void setInfo(QSerialPortInfo info);

    /************************************************************************
     * DMX/Serial Interface Methods
     ************************************************************************/
public:
    /** @reimpl */
    DMXInterface::Type type() const override;

    /** @reimpl */
    QString typeString() const override;

    /** @reimpl */
    bool open() override;

    /** @reimpl */
    bool openByPID(const int FTDIPID) override;

    /** @reimpl */
    bool close() override;

    /** @reimpl */
    bool isOpen() const override;

    /** @reimpl */
    bool reset() override;

    /** @reimpl */
    bool setLineProperties() override;

    /** @reimpl */
    bool setBaudRate() override;

    /** @reimpl */
    bool setFlowControl() override;

    /** @reimpl */
    bool setLowLatency(bool lowLatency) override;

    /** @reimpl */
    bool clearRts() override;

    /** @reimpl */
    bool purgeBuffers() override;

    /** @reimpl */
    bool setBreak(bool on) override;

    /** @reimpl */
    bool write(const QByteArray& data) override;

    /** @reimpl */
    QByteArray read(int size) override;

    /** @reimpl */
    uchar readByte(bool* ok = NULL) override;

private:
    QSerialPort *m_handle;
    QSerialPortInfo m_info;
};

#endif

