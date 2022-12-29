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

#include <QColor>
#include <QObject>
#include <QVariant>

class QXmlStreamReader;
class QXmlStreamWriter;
class SceneValue;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCPalette   QString("Palette")
#define KXMLQLCPaletteID QString("ID")

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
    Q_PROPERTY(int intValue1 READ intValue1 CONSTANT)
    Q_PROPERTY(int intValue2 READ intValue2 CONSTANT)
    Q_PROPERTY(QString strValue1 READ strValue1 CONSTANT)
    Q_PROPERTY(QColor rgbValue READ rgbValue CONSTANT)
    Q_PROPERTY(QColor wauvValue READ wauvValue CONSTANT)
    Q_PROPERTY(FanningType fanningType READ fanningType WRITE setFanningType NOTIFY fanningTypeChanged)
    Q_PROPERTY(FanningLayout fanningLayout READ fanningLayout WRITE setFanningLayout NOTIFY fanningLayoutChanged)
    Q_PROPERTY(int fanningAmount READ fanningAmount WRITE setFanningAmount NOTIFY fanningAmountChanged)
    Q_PROPERTY(QVariant fanningValue READ fanningValue WRITE setFanningValue NOTIFY fanningValueChanged)

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
        Pan       = 1 << 2,
        Tilt      = 1 << 3,
        PanTilt   = 1 << 4,
        Shutter   = 1 << 5,
        Gobo      = 1 << 6
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(PaletteType)
#endif

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    QLCPalette(QLCPalette::PaletteType type, QObject *parent = 0);
    QLCPalette *createCopy();

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

    /** Get the palette type */
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
    int intValue1() const;
    int intValue2() const;
    QString strValue1() const;
    QColor rgbValue() const;
    QColor wauvValue() const;

    void setValue(QVariant val);
    void setValue(QVariant val1, QVariant val2);
    QVariantList values() const;
    void setValues(QVariantList values);
    void resetValues();

    QList<SceneValue> valuesFromFixtures(Doc *doc, QList<quint32>fixtures);
    QList<SceneValue> valuesFromFixtureGroups(Doc *doc, QList<quint32>groups);

protected:
    /** This method returns a normalized factor between 0.0 and 1.0
     *  which will then be multiplied by a value to obtain the final
     *  DMX value.
     *  It considers the fanning algorithm and amount and with
     *  the provided progress it can calculate the X-axis value. */
    qreal valueFactor(qreal progress);

signals:
    void nameChanged();

private:
    quint32 m_id;
    PaletteType m_type;
    QString m_name;
    QVariantList m_values;

    /************************************************************************
     * Fanning
     ************************************************************************/
public:
    enum FanningType
    {
        Flat,
        Linear,
        Sine,
        Square,
        Saw
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(FanningType)
#endif

    enum FanningLayout
    {
        XAscending,
        XDescending,
        XCentered,
        YAscending,
        YDescending,
        YCentered,
        ZAscending,
        ZDescending,
        ZCentered
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(FanningLayout)
#endif

    /** Get/Set the fanning type */
    FanningType fanningType() const;
    void setFanningType(QLCPalette::FanningType type);

    /** Helper methods to convert fanning type <-> string */
    static QString fanningTypeToString(QLCPalette::FanningType type);
    static FanningType stringToFanningType(const QString& str);

    /** Get/Set the fanning layout */
    FanningLayout fanningLayout() const;
    void setFanningLayout(QLCPalette::FanningLayout layout);

    /** Helper methods to convert fanning layout <-> string */
    static QString fanningLayoutToString(QLCPalette::FanningLayout layout);
    static FanningLayout stringToFanningLayout(const QString& str);

    /** Get/Set the amount of fanning applied to this palette */
    int fanningAmount() const;
    void setFanningAmount(int amount);

    /** Get/Set the fanning value */
    QVariant fanningValue() const;
    void setFanningValue(QVariant value);

signals:
    void fanningTypeChanged();
    void fanningLayoutChanged();
    void fanningAmountChanged();
    void fanningValueChanged();

private:
    FanningType m_fanningType;
    FanningLayout m_fanningLayout;
    int m_fanningAmount;
    QVariant m_fanningValue;

    /************************************************************************
     * Color helpers
     ************************************************************************/
public:
    /** Helper method to pack two QColor into a Qt-like style string
     *  formatted like this: "#rrggbbwwaauv" */
    static QString colorToString(QColor rgb, QColor wauv);

    /** Helper method to convert a string created with colorToString
     *  back to 2 separate QColor */
    static bool stringToColor(QString str, QColor &rgb, QColor &wauv);

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
