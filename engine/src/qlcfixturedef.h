/*
  Q Light Controller Plus
  qlcfixturedef.h

  Copyright (c) Heikki Junnila
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

#ifndef QLCFIXTUREDEF_H
#define QLCFIXTUREDEF_H

#include <QString>
#include <QList>
#include <QFile>

#include "qlcphysical.h"

/** @addtogroup engine Engine
 * @{
 */

// Fixture document type
#define KXMLQLCFixtureDefDocument QString("FixtureDefinition")

// Fixture definition XML tags
#define KXMLQLCFixtureDef               QString("FixtureDefinition")
#define KXMLQLCFixtureDefManufacturer   QString("Manufacturer")
#define KXMLQLCFixtureDefModel          QString("Model")
#define KXMLQLCFixtureDefType           QString("Type")

// Fixture instance XML tags
#define KXMLQLCFixtureName      QString("Name")
#define KXMLQLCFixtureID        QString("ID")
#define KXMLQLCFixtureUniverse  QString("Universe")
#define KXMLQLCFixtureAddress   QString("Address")

class QXmlStreamReader;
class QLCFixtureMode;
class QLCFixtureDef;
class QLCChannel;

/**
 * QLCFixtureDef represents exactly one fixture, identified by its manufacturer
 * and model names. Each fixture definition has also a type that describes
 * roughly the fixture's purpose (moving head, scanner, flower etc).
 *
 * A QLCFixtureDef houses all of its QLCChannel entries in a non-ordered pool.
 * Each QLCFixtureMode picks their channels from this channel pool and arranges
 * them in such an order that represents each mode (channel and physical
 * configuration) of the fixture.
 *
 * The same channel instance cannot exist multiple times in a QLCFixtureDef,
 * but it is still possible to create two channel instances with the same name
 * and apparent content. The same rules apply to QLCFixtureModes within a
 * QLCFixtureDef.
 *
 * QLCFixtureDef owns the channel instances and deletes them when it is deleted
 * itself. QLCFixtureModes do not delete their channels because they might be
 * shared between multiple modes.
 */
class QLCFixtureDef
{
public:
    /** Default constructor */
    QLCFixtureDef();

    /** Copy constructor */
    QLCFixtureDef(const QLCFixtureDef* fixtureDef);

    /** Destructor */
    ~QLCFixtureDef();

    /** Assignment operator */
    QLCFixtureDef& operator=(const QLCFixtureDef& fixtureDef);

    /*********************************************************************
     * Fixture information
     *********************************************************************/
public:
    /** Keep this ordered alphabetically */
    enum FixtureType
    {
        ColorChanger = 0,
        Dimmer,
        Effect,
        Fan,
        Flower,
        Hazer,
        Laser,
        LEDBarBeams,
        LEDBarPixels,
        MovingHead,
        Other,
        Scanner,
        Smoke,
        Strobe
    };

    /** Get the temporary definition file absolute path */
    QString definitionSourceFile() const;

    /** Set the temporary definition file absolute path */
    void setDefinitionSourceFile(const QString& absPath);

    /** Get the fixture's name string (=="manufacturer model") */
    QString name() const;

    /** Set the fixture's manufacturer string */
    void setManufacturer(const QString& mfg);

    /** Set the fixture's manufacturer string */
    QString manufacturer() const;

    /** Set the fixture's model string */
    void setModel(const QString& model);

    /** Get the fixture's model string */
    QString model() const;

    /** Set the fixture's type */
    void setType(const FixtureType type);

    /** Get the fixture's type */
    FixtureType type();

    /** Convert a fixture type to string */
    static QString typeToString(FixtureType type);

    /** Convert string into a fixture type */
    static FixtureType stringToType(const QString &type);

    /** Set the definition's author */
    void setAuthor(const QString& author);

    /** Get the definition's author */
    QString author();

    /** Check if the full definition has been loaded */
    void checkLoaded(QString mapPath);
    void setLoaded(bool loaded);

    /** Get/Set if the definition is user-made */
    bool isUser() const;
    void setIsUser(bool flag);

protected:
    bool m_isLoaded;
    bool m_isUser;
    QString m_fileAbsolutePath;
    QString m_manufacturer;
    QString m_model;
    FixtureType m_type;
    QString m_author;

    /*********************************************************************
     * Channels
     *********************************************************************/
public:
    /** Add a new channel to this fixture */
    bool addChannel(QLCChannel* channel);

    /** Remove a certain channel from this fixture */
    bool removeChannel(QLCChannel* channel);

    /** Search for a channel by its name */
    QLCChannel* channel(const QString& name);

    /**
     * Get all channels in this fixture. Changes to the list won't end
     * up into the fixture definition. This list does not represent the actual
     * channel order for the fixture; use QLCFixtureMode for that.
     *
     * @return An arbitrarily-ordered list of possible channels in a fixture
     */
    QList <QLCChannel*> channels() const;

protected:
    /** Available channels */
    QList <QLCChannel*> m_channels;

    /*********************************************************************
     * Modes
     *********************************************************************/
public:
    /** Add a new mode to this fixture */
    bool addMode(QLCFixtureMode* mode);

    /** Remove a certain mode from this fixture */
    bool removeMode(QLCFixtureMode* mode);

    /** Get a certain mode by its name */
    QLCFixtureMode* mode(const QString& name);

    /** Get all modes in this fixture. Changes to the list won't end
        up into the fixture definition. */
    QList <QLCFixtureMode*> modes();

protected:
    /** Modes (i.e. ordered collections of channels) */
    QList <QLCFixtureMode*> m_modes;


    /*********************************************************************
     * Physical
     *********************************************************************/
public:
    /** Get/Set the global physical information */
    QLCPhysical physical() const;
    void setPhysical(const QLCPhysical& physical);

protected:
    QLCPhysical m_physical;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Save the fixture into an XML file */
    QFile::FileError saveXML(const QString& fileName);

    /** Load this fixture's contents from the given file */
    QFile::FileError loadXML(const QString& fileName);

protected:
    /** Load fixture contents from an XML document */
    bool loadXML(QXmlStreamReader &doc);

    /** Load <Creator> information */
    bool loadCreator(QXmlStreamReader &doc);
};

/** @} */

#endif
