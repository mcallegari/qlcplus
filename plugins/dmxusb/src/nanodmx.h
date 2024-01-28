/*
  Q Light Controller Plus
  nanodmx.h

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

#ifndef NANODMX_H
#define NANODMX_H

#include <QFile>

#include "dmxusbwidget.h"

class NanoDMX : public QThread, public DMXUSBWidget
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    NanoDMX(DMXInterface *interface, quint32 outputLine);
    virtual ~NanoDMX();

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
    bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged);

protected:
    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run();

private:
    bool checkReply();
    bool sendChannelValue(int channel, uchar value);
    QString getDeviceName();

private:
    /** File handle for /dev/ttyACMx */
    QFile m_file;
    bool m_running;
};

#endif
