/*
  Q Light Controller Plus
  ftd2xx-interface.h

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

#ifndef FTD2XXINTERFACE_H
#define FTD2XXINTERFACE_H

#include "dmxinterface.h"

typedef void *PVOID;
typedef PVOID	FT_HANDLE;

class FTD2XXInterface : public DMXInterface
{
public:
    FTD2XXInterface(const QString& serial, const QString& name, const QString& vendor,
                    quint16 VID, quint16 PID, quint32 id);

    /** Destructor */
    virtual ~FTD2XXInterface();

    static QList<DMXInterface *> interfaces(QList<DMXInterface *> discoveredList);

    /** @reimpl */
    bool readLabel(uchar label, int &intParam, QString &strParam);

    /************************************************************************
     * DMX/Serial Interface Methods
     ************************************************************************/
public:
    /** @reimpl */
    DMXInterface::Type type();

    /** @reimpl */
    QString typeString();

    /** @reimpl */
    bool open();

    /** @reimpl */
    bool openByPID(const int FTDIPID);

    /** @reimpl */
    bool close();

    /** @reimpl */
    bool isOpen() const;

    /** @reimpl */
    bool reset();

    /** @reimpl */
    bool setLineProperties();

    /** @reimpl */
    bool setBaudRate();

    /** @reimpl */
    bool setFlowControl();

    /** @reimpl */
    bool setLowLatency(bool lowLatency);

    /** @reimpl */
    bool clearRts();

    /** @reimpl */
    bool purgeBuffers();

    /** @reimpl */
    bool setBreak(bool on);

    /** @reimpl */
    bool write(const QByteArray& data);

    /** @reimpl */
    QByteArray read(int size, uchar* buffer = NULL);

    /** @reimpl */
    uchar readByte(bool* ok = NULL);

private:
    FT_HANDLE m_handle;
};

#endif
