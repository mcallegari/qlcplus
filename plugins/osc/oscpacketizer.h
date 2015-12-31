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

    enum TagType { Integer = 0x01, Float = 0x02, String = 0x03, Blob = 0x04 };

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

    /**
     * Parse an OSC message received from the network.
     * As a result, it returns the extracted OSC path and values
     * (empty if invalid)
     *
     * @param data the UDP datagram received from the network
     * @param path the OSC path extracted from the packet
     * @param values the array of values extracted from the packet
     * @return true on successful parsing, otherwise false
     */
    bool parseMessage(QByteArray& data, QString &path, QByteArray& values);
};

#endif
