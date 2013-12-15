/*
  Q Light Controller
  avolitesd4parser.h

  Copyright (C) Rui Barreiros
                Heikki Junnila

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

#include <QDomDocument>
#include <QString>
#include <QList>
#include <QMap>

#include "qlcchannel.h"

class QLCFixtureMode;
class QLCFixtureDef;
class QDomElement;

#define KExtAvolitesFixture ".d4" // Avolites Diamond 4 format

class AvolitesD4Parser
{
public:
    /** Create a new Avolites D4 Parser object */
    AvolitesD4Parser();
    ~AvolitesD4Parser();

    /**
     * Load a D4 file from the given path
     *
     * @param path The file path to load from
     * @return true if successful, otherwise false (see lastError() for a possible cause)
     */
    bool loadXML(const QString& path);

    /**
     * After loading a file with loadXML(), convert the loaded D4 file
     * into a QLC Fixture Definition, given in $fixtureDef.
     *
     * @param fixtureDef The fixture definition object to fill, must not be NULL.
     * @return true if successful, otherwise false (see lastError() for possible cause)
     */
    bool fillFixtureDef(QLCFixtureDef *fixtureDef);

    /**
     * Get the last error encountered while loading/parsing a file.
     *
     * @return An error description
     */
    QString lastError() const;

private:
    QString fixtureName() const;
    QString fixtureShortName() const;
    QString fixtureCompany() const;
    QString copyright() const;

    QLCChannel::Group getGroupFromXML(const QDomElement& elem);
    QLCChannel::PrimaryColour getColourFromXML(const QDomElement& elem);

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
    bool isFunction(const QDomElement& elem) const;

    /** Check if the given XML element contains a 16bit function */
    bool is16Bit(const QDomElement& elem) const;

    /** Parse all channels from $elem into $fixtureDef */
    bool parseChannels(const QDomElement& elem, QLCFixtureDef* fixtureDef);

    /** Parse the capabilities from one channel contained in $elem into $chan (must exist) */
    bool parseCapabilities(const QDomElement& elem, QLCChannel* chan, bool isFine = false);

    /** Parse all modes contained under $elem into $fixtureDef */
    bool parseModes(const QDomElement& elem, QLCFixtureDef* fixtureDef);

    /** Parse the fixture's/mode's physical properties from $elem into $mode */
    void parsePhysical(const QDomElement& elem, QLCFixtureMode* mode);

    /** Convert string $attr into an Attributes enum */
    Attributes stringToAttributeEnum(const QString& attr);

    /** Attempt to guess the fixture type from the channels/capabilities in $def */
    QString guessType(QLCFixtureDef *def) const;

private:
    QString m_lastError;
    QDomDocument m_documentRoot;
    ChannelsMap m_channels;

    static StringToEnumMap s_attributesMap;
};

#endif // AVOLITESD4PARSER_H
