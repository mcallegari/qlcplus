/*
  Q Light Controller Plus
  avolitesd4parser.h

  Copyright (C) Rui Barreiros
                Heikki Junnila
                Massimo Callegari

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

#ifndef AVOLITESD4PARSER_H
#define AVOLITESD4PARSER_H

#include <QString>
#include <QList>
#include <QMap>

#include "qlcchannel.h"
#include "qlcfixturedef.h"

class QXmlStreamReader;
class QLCFixtureMode;

/** @addtogroup engine Engine
 * @{
 */

#define KExtAvolitesFixture ".d4" // Avolites Diamond 4 format

class AvolitesD4Parser
{
public:
    /** Create a new Avolites D4 Parser object */
    AvolitesD4Parser();
    ~AvolitesD4Parser();

    /**
     * Load a D4 file from the given path and fill a QLC Fixture Definition,
     * given in $fixtureDef.
     *
     * @param path The file path to load from
     * @param fixtureDef The fixture definition object to fill, must not be NULL.
     *
     * @return true if successful, otherwise false (see lastError() for a possible cause)
     */
    bool loadXML(const QString& path, QLCFixtureDef *fixtureDef);

    /**
     * Get the last error encountered while loading/parsing a file.
     *
     * @return An error description
     */
    QString lastError() const;

private:
    QLCChannel::Group getGroup(QString ID, QString name, QString group);
    QLCChannel::PrimaryColour getColour(QString ID, QString name, QString group);

private:
    enum Attributes
    {
        SPECIAL,
        INTENSITY,
        PANTILT,
        COLOUR,
        GOBO,
        BEAM,
        EFFECT
    };

    typedef QMap <QString,Attributes> StringToEnumMap;
    typedef QMap <QString,QLCChannel*> ChannelsMap;

private:
    /** Check if the given XML element contains an avolites function */
    //bool isFunction(const QDomElement& elem) const;

    /** Check if the given XML element contains a 16bit function */
    bool is16Bit(QString dmx) const;

    QLCCapability *getCapability(QString dmx, QString name, bool isFine = false);

    /** Parse all channels from $elem into $fixtureDef */
    bool parseChannel(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef);

    /** Parse a Function tag defining a channel capability */
    bool parseFunction(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef,
                       QLCChannel *channel, QString ID, QString group);

    /** Parse the capabilities from one channel contained in $elem into $chan (must exist) */
    bool parseAttribute(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef);

    /** Parse a mode contained under $elem into $fixtureDef */
    bool parseMode(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef);

    /** Compare global vs. mode physical to detect override */
    bool comparePhysical(const QLCPhysical &globalPhy, const QLCPhysical &modePhy) const;

    /** Parse the fixture's/mode's physical properties from $elem into $mode */
    void parsePhysical(QXmlStreamReader *doc, QLCFixtureDef *fixtureDef, QLCFixtureMode *mode);

    /** Parse a mode Include tag */
    void parseInclude(QXmlStreamReader *doc, QLCFixtureMode *mode);

    /** Convert string $attr into an Attributes enum */
    Attributes stringToAttributeEnum(const QString& attr);

    /** Attempt to guess the fixture type from the channels/capabilities in $def */
    QLCFixtureDef::FixtureType guessType(QLCFixtureDef *def) const;

private:
    QString m_lastError;
    ChannelsMap m_channels;

    static StringToEnumMap s_attributesMap;
};

/** @} */

#endif // AVOLITESD4PARSER_H
