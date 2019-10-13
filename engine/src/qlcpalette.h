/*
  Q Light Controller Plus
  qlcpalette.h

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

#ifndef QLCPALETTE_H
#define QLCPALETTE_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;
class SceneValue;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCPalette "Palette"
#define KXMLQLCPaletteID "ID"

/**
 * QLCPalette represents a QLC+ Palette, which is the definition
 * of a capability such as color, position, dimmer, etc
 * that can be applied to an arbitrary group of fixtures.
 */
class QLCPalette : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(int type READ type CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    /**
     * The Palette type. This is fundamental
     * to properly handle values
     */
    enum PaletteType
    {
        Undefined = 0,
        Dimmer    = 1 << 0,
        Color     = 1 << 1,
        Position  = 1 << 2,
        Shutter   = 1 << 3,
        Gobo      = 1 << 4
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(PaletteType)
#endif

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    QLCPalette(QLCPalette::PaletteType type, QObject *parent = 0);

    virtual ~QLCPalette();

    /************************************************************************
     * Properties
     ************************************************************************/
public:
    /** Get/Set the palette ID */
    quint32 id() const;
    void setID(quint32 id);

    /** Get an invalid palette id */
    static quint32 invalidId();

    /** Get/Set the palette type */
    PaletteType type() const;

    /** Helper methods to convert palette type <-> string */
    static QString typeToString(QLCPalette::PaletteType type);
    static PaletteType stringToType(const QString& str);

    Q_INVOKABLE QString iconResource(bool svg = false) const;

    /** Get/Set the name of this palette */
    QString name() const;
    void setName(const QString& name);

    /** Get/Set the value(s) for this Palette.
     *  Some types like Position will store 2 values */
    QVariant value() const;
    void setValue(QVariant val);
    void setValue(QVariant val1, QVariant val2);

    QList<SceneValue> valuesFromFixtures(Doc *doc, QList<quint32>fixtures);
    QList<SceneValue> valuesFromFixtureGroups(Doc *doc, QList<quint32>groups);

signals:
    void nameChanged();

private:
    quint32 m_id;
    PaletteType m_type;
    QString m_name;
    QVariantList m_values;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Helper method to allocate and add a Palette to a Doc */
    static bool loader(QXmlStreamReader &xmlDoc, Doc *doc);

    /** Load a Palette from the given QXmlStreamReader */
    bool loadXML(QXmlStreamReader &doc);

    /** Save a Palette to the given XML tag in the given document */
    bool saveXML(QXmlStreamWriter *doc);
};

/** @} */

#endif
