/*
  Q Light Controller Plus
  e131packetizer.h

  Copyright (c) Massimo Callegari

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

#include <QHostAddress>
#include <QByteArray>
#include <QString>
#include <QHash>

#ifndef E131PACKETIZER_H
#define E131PACKETIZER_H

#define E131_PRIORITY_DEFAULT 100

class E131Packetizer
{
    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    E131Packetizer(QString MACaddr);
    ~E131Packetizer();

public:
    /*********************************************************************
     * Sender functions
     *********************************************************************/

    /** Prepare an E1.31 DMX packet */
    void setupE131Dmx(QByteArray& data, const int& universe, const int& priority, const QByteArray &values);

    /*********************************************************************
     * Receiver functions
     *********************************************************************/

    /** Verify the validity of an E1.31 packet and store the opCode in 'code' */
    bool checkPacket(QByteArray& data);

    bool fillDMXdata(QByteArray& data, QByteArray& dmx, quint32 &universe);

private:
    QByteArray m_commonHeader;
    QHash<int, uchar> m_sequence;
};

#endif
