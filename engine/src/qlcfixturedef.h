/*
  Q Light Controller
  qlcfixturedef.h

  Copyright (c) Heikki Junnila

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


/** @addtogroup engine Engine
 * @{
 */

// Fixture document type
#define KXMLQLCFixtureDefDocument "FixtureDefinition"

// Fixture definition XML tags
#define KXMLQLCFixtureDef "FixtureDefinition"
#define KXMLQLCFixtureDefManufacturer "Manufacturer"
#define KXMLQLCFixtureDefModel "Model"
#define KXMLQLCFixtureDefType "Type"

// Fixture instance XML tags
#define KXMLQLCFixtureName "Name"
#define KXMLQLCFixtureID "ID"
#define KXMLQLCFixtureUniverse "Universe"
#define KXMLQLCFixtureAddress "Address"

class QDomDocument;
class QDomElement;
class QLCChannel;
class QLCFixtureMode;
class QLCFixtureDef;

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

    /** Set the fixture's type string */
    void setType(const QString& type);

    /** Get the fixture's type string */
    QString type();

    /** Set the definition's author */
    void setAuthor(const QString& author);

    /** Get the definition's author */
    QString author();

private:
    void checkLoaded();

protected:
    bool m_isLoaded;
    QString m_defFileAbsolutePath;
    QString m_manufacturer;
    QString m_model;
    QString m_type;
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
     * Load & Save
     *********************************************************************/
public:
    /** Save the fixture into an XML file */
    QFile::FileError saveXML(const QString& fileName);

    /** Load this fixture's contents from the given file */
    QFile::FileError loadXML(const QString& fileName);

protected:
    /** Load fixture contents from an XML document */
    bool loadXML(const QDomDocument& doc);

    /** Load <Creator> information */
    bool loadCreator(const QDomElement& creator);
};

/** @} */

#endif
