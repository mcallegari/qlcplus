/*
  Q Light Controller
  vinceusbdmx512tx.h

  Copyright (C) Jérôme Lebleu

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

#ifndef VINCEUSBDMX512TX_H
#define VINCEUSBDMX512TX_H

#include "vinceusbdmx512.h"

class QByteArray;
class VinceUSBDMX512TX : public VinceUSBDMX512
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    VinceUSBDMX512TX(const QString& serial, const QString& name, const QString& vendor,
                     QLCFTDI *ftdi = NULL, quint32 id = 0);
    ~VinceUSBDMX512TX();

    /** @reimp */
    Type type() const;

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

private:
    QByteArray m_universe;
};

#endif
