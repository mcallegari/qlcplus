/*
  Q Light Controller Plus
  stageprofi.h

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

#ifndef STAGEPROFI_H
#define STAGEPROFI_H

#include "dmxusbwidget.h"

class Stageprofi final : public QThread, public DMXUSBWidget
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    Stageprofi(DMXInterface *interface, quint32 outputLine);
    virtual ~Stageprofi();

    /** @reimp */
    DMXUSBWidget::Type type() const override;

    /************************************************************************
     * Widget functions
     ************************************************************************/
public:
    /** @reimp */
    bool open(quint32 line = 0, bool input = false) override;

    /** @reimp */
    bool close(quint32 line = 0, bool input = false) override;

    /** @reimp */
    QString uniqueName(ushort line = 0, bool input = false) const override;

    /** @reimp */
    QString additionalInfo() const override;

    /** @reimp */
    bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged) override;

protected:
    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run() override;

private:
    bool checkReply();
    bool sendChannelValue(int channel, uchar value);

private:
    bool m_running;
};

#endif
