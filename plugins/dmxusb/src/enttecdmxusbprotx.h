/*
  Q Light Controller
  enttecdmxusbprotx.h

  Copyright (C) Heikki Junnila

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

#ifndef ENTTECDMXUSBPROTX_H
#define ENTTECDMXUSBPROTX_H

#include "enttecdmxusbpro.h"

class QByteArray;
class EnttecDMXUSBProTX : public EnttecDMXUSBPro
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    EnttecDMXUSBProTX(const QString& serial, const QString& name, const QString& vendor,
                      int port = 0, QLCFTDI *ftdi = NULL, quint32 id = 0);
    ~EnttecDMXUSBProTX();

    /** @reimp */
    Type type() const;

private:
    bool configurePort(int port);

private:
    int m_port;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    QString uniqueName() const;

    /****************************************************************************
     * Name & Serial
     ****************************************************************************/
public:
    /** @reimp */
    QString additionalInfo() const;

    /************************************************************************
     * Write universe
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(const QByteArray& universe);
};

#endif
