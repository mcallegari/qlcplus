/*
  Q Light Controller Plus
  oscpacketizer.h

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
#include <QPair>

#ifndef OSCPACKETIZER_H
#define OSCPACKETIZER_H

class OSCPacketizer
{
    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    OSCPacketizer();
    ~OSCPacketizer();

    enum TagType { Integer = 0x01, Float = 0x02, Time = 0x03, String = 0x04, Blob = 0x05 };

public:
    /*********************************************************************
     * Sender functions
     *********************************************************************/

    /** Prepare an OSC DMX packet
 */
    /**
     * Prepare an OSC DMX message using a OSC path like
     * /$universe/dmx/$channel
     * All values are transmitted as float (OSC 'f')
     *
     * @param data the message composed by this function to be sent on the network
     * @param universe the universe used to compose the OSC message path
     * @param channel the DMX channel used to compose the OSC message path
     * @param value the value to be transmitted (as float)
     */
    void setupOSCDmx(QByteArray& data, quint32 universe, quint32 channel, uchar value);

    /**
     * Prepare an generic OSC message using the specified $path.
     * Values are appended to the message as specified by their $types.
     * Note that $types and $values must have the same length
     *
     * @param data the message composed by this function to be sent on the network
     * @param path the OSC path to be used in the message
     * @param types the list of types of the following $values
     * @param values the actual values to be appended to the message
     */
    void setupOSCGeneric(QByteArray& data, QString &path, QString types, QByteArray &values);

    /*********************************************************************
     * Receiver functions
     *********************************************************************/
private:
    /**
     * Extract an OSC message received from a buffer.
     * As a result, it returns the extracted OSC path and values
     * (empty if invalid)
     *
     * @param data the buffer containing the OSC message
     * @param path the OSC path extracted from the buffer
     * @param values the array of values extracted from the buffer
     * @return true on successful parsing, otherwise false
     */
    bool parseMessage(QByteArray const& data, QString& path, QByteArray& values);
public:
    /**
     * Parse a OSC packet received from the network.
     *
     * @param data the payload of a UDP packet received from the network
     * @return a list of couples of OSC path/values
     */
    QList<QPair<QString, QByteArray> > parsePacket(QByteArray const& data);

};

#endif
