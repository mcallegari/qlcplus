/*
  Q Light Controller Plus
  qlcpalette.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtMath>
#include <QDebug>

#include "monitorproperties.h"
#include "qlcpalette.h"
#include "qlcchannel.h"
#include "scenevalue.h"
#include "doc.h"

#define KXMLQLCPaletteType      "Type"
#define KXMLQLCPaletteName      "Name"
#define KXMLQLCPaletteValue     "Value"
#define KXMLQLCPaletteFanning   "Fan"
#define KXMLQLCPaletteFanLayout "Layout"
#define KXMLQLCPaletteFanAmount "Amount"
#define KXMLQLCPaletteFanValue  "FanValue"

QLCPalette::QLCPalette(QLCPalette::PaletteType type, QObject *parent)
    : QObject(parent)
    , m_id(QLCPalette::invalidId())
    , m_type(type)
    , m_fanningType(Flat)
    , m_fanningLayout(XAscending)
    , m_fanningAmount(100)
{
}

QLCPalette *QLCPalette::createCopy()
{
    QLCPalette *copy = new QLCPalette(type());
    copy->setValues(this->values());
    copy->setName(this->name());
    copy->setFanningType(this->fanningType());
    copy->setFanningLayout(this->fanningLayout());
    copy->setFanningAmount(this->fanningAmount());
    copy->setFanningValue(this->fanningValue());

    return copy;
}

QLCPalette::~QLCPalette()
{

}

/************************************************************************
 * Properties
 ************************************************************************/

quint32 QLCPalette::id() const
{
    return m_id;
}

void QLCPalette::setID(quint32 id)
{
    m_id = id;
}

quint32 QLCPalette::invalidId()
{
    return UINT_MAX;
}

QLCPalette::PaletteType QLCPalette::type() const
{
    return m_type;
}

QString QLCPalette::typeToString(QLCPalette::PaletteType type)
{
    switch (type)
    {
        case Dimmer:    return "Dimmer";
        case Color:     return "Color";
        case Pan:       return "Pan";
        case Tilt:      return "Tilt";
        case PanTilt:   return "PanTilt";
        case Shutter:   return "Shutter";
        case Gobo:      return "Gobo";
        case Undefined: return "";
    }

    return "";
}

QLCPalette::PaletteType QLCPalette::stringToType(const QString &str)
{
    if (str == "Dimmer")
        return Dimmer;
    else if (str == "Color")
        return Color;
    else if (str == "Pan")
        return Pan;
    else if (str == "Tilt")
        return Tilt;
    else if (str == "PanTilt")
        return PanTilt;
    else if (str == "Shutter")
        return Shutter;
    else if (str == "Gobo")
        return Gobo;

    return Undefined;
}

QString QLCPalette::iconResource(bool svg) const
{
    QString prefix = svg ? "qrc" : "";
    QString ext = svg ? "svg" : "png";

    switch(type())
    {
        case Dimmer: return QString("%1:/intensity.%2").arg(prefix).arg(ext);
        case Color: return QString("%1:/color.%2").arg(prefix).arg(ext);
        case Pan: return QString("%1:/pan.%2").arg(prefix).arg(ext);
        case Tilt: return QString("%1:/tilt.%2").arg(prefix).arg(ext);
        case PanTilt: return QString("%1:/position.%2").arg(prefix).arg(ext);
        case Shutter: return QString("%1:/shutter.%2").arg(prefix).arg(ext);
        case Gobo: return QString("%1:/gobo.%2").arg(prefix).arg(ext);
        default: return "";
    }
}

QString QLCPalette::name() const
{
    return m_name;
}

void QLCPalette::setName(const QString &name)
{
    if (name == m_name)
        return;

    m_name = QString(name);
    emit nameChanged();
}

QVariant QLCPalette::value() const
{
    if (m_values.isEmpty())
        return QVariant();

    return m_values.first();
}

int QLCPalette::intValue1() const
{
    if (m_values.isEmpty())
        return -1;

    return m_values.at(0).toInt();
}

int QLCPalette::intValue2() const
{
    if (m_values.count() < 2)
        return -1;

    return m_values.at(1).toInt();
}

QString QLCPalette::strValue1() const
{
    if (m_values.isEmpty())
        return QString();

    return m_values.at(0).toString();
}

QColor QLCPalette::rgbValue() const
{
    if (m_values.isEmpty())
        return QColor();

    QColor rgb, wauv;
    stringToColor(m_values.at(0).toString(), rgb, wauv);

    return rgb;
}

QColor QLCPalette::wauvValue() const
{
    if (m_values.isEmpty())
        return QColor();

    QColor rgb, wauv;
    stringToColor(m_values.at(0).toString(), rgb, wauv);

    return wauv;
}

void QLCPalette::setValue(QVariant val)
{
    m_values.clear();
    m_values.append(val);
}

void QLCPalette::setValue(QVariant val1, QVariant val2)
{
    m_values.clear();
    m_values.append(val1);
    m_values.append(val2);
}

QVariantList QLCPalette::values() const
{
    return m_values;
}

void QLCPalette::setValues(QVariantList values)
{
    m_values = values;
}

void QLCPalette::resetValues()
{
    m_values.clear();
}

QList<SceneValue> QLCPalette::valuesFromFixtures(Doc *doc, QList<quint32> fixtures)
{
    QList<SceneValue> list;

    int fxCount = fixtures.count();
    // normalized progress in [ 0.0, 1.0 ] range
    qreal progress = 0.0;
    int intFanValue = fanningValue().toInt();
    FanningType fType = fanningType();
    FanningLayout fLayout = fanningLayout();
    MonitorProperties *mProps = doc->monitorProperties();

    // sort the fixtures list based on selected layout
    std::sort(fixtures.begin(), fixtures.end(),
        [fLayout, mProps](quint32 a, quint32 b) {
            QVector3D posA = mProps->fixturePosition(a, 0, 0);
            QVector3D posB = mProps->fixturePosition(b, 0, 0);

            switch(fLayout)
            {
                case XAscending: return posA.x() < posB.x();
                case XDescending: return posB.x() < posA.x();
                case YAscending: return posA.y() < posB.y();
                case YDescending: return posB.y() < posA.y();
                case ZAscending: return posA.z() < posB.z();
                case ZDescending: return posB.z() < posA.z();
                default: return false;
            }
        });

    foreach (quint32 id, fixtures)
    {
        Fixture *fixture = doc->fixture(id);
        if (fixture == NULL)
            continue;

        qreal factor = valueFactor(progress);

        switch(type())
        {
            case Dimmer:
            {
                int dValue = value().toInt();
                quint32 masterIntensityChannel = fixture->type() == QLCFixtureDef::Dimmer ?
                            0 : fixture->masterIntensityChannel();

                if (fType != Flat)
                    dValue = int((qreal(intFanValue - dValue) * factor) + dValue);

                if (masterIntensityChannel != QLCChannel::invalid())
                    list << SceneValue(id, masterIntensityChannel, uchar(dValue));

                for (int i = 0; i < fixture->heads(); i++)
                {
                    quint32 headDimmerChannel = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, i);
                    if (headDimmerChannel != QLCChannel::invalid())
                        list << SceneValue(id, headDimmerChannel, uchar(dValue));
                }
            }
            break;
            case Color:
            {
                QColor startColor = value().value<QColor>();
                QColor col = startColor;

                if (fType != Flat)
                {
                    QColor endColor = fanningValue().value<QColor>();
                    qreal rDelta = endColor.red() - startColor.red();
                    qreal gDelta = endColor.green() - startColor.green();
                    qreal bDelta = endColor.blue() - startColor.blue();
                    col.setRed(startColor.red() + qRound(rDelta * factor));
                    col.setGreen(startColor.green() + qRound(gDelta * factor));
                    col.setBlue(startColor.blue() + qRound(bDelta * factor));
                }

                for (int i = 0; i < fixture->heads(); i++)
                {
                    QVector<quint32> rgbCh = fixture->rgbChannels(i);
                    if (rgbCh.size() == 3)
                    {
                        list << SceneValue(id, rgbCh.at(0), uchar(col.red()));
                        list << SceneValue(id, rgbCh.at(1), uchar(col.green()));
                        list << SceneValue(id, rgbCh.at(2), uchar(col.blue()));
                    }
                    QVector<quint32> cmyCh = fixture->cmyChannels(i);
                    if (cmyCh.size() == 3)
                    {
                        list << SceneValue(id, cmyCh.at(0), uchar(col.cyan()));
                        list << SceneValue(id, cmyCh.at(1), uchar(col.magenta()));
                        list << SceneValue(id, cmyCh.at(2), uchar(col.yellow()));
                    }
                }
            }
            break;
            case Pan:
            {
                int degrees = value().toInt();

                if (fType != Flat)
                    degrees = int((qreal(degrees) + qreal(intFanValue) * factor));

                list << fixture->positionToValues(QLCChannel::Pan, degrees);
            }
            break;
            case Tilt:
            {
                int degrees = m_values.count() == 2 ? m_values.at(1).toInt() : value().toInt();

                if (fType != Flat)
                    degrees = int((qreal(degrees) + qreal(intFanValue) * factor));

                list << fixture->positionToValues(QLCChannel::Tilt, degrees);
            }
            break;
            case PanTilt:
            {
                if (m_values.count() == 2)
                {
                    int panDegrees = m_values.at(0).toInt();
                    int tiltDegrees = m_values.at(1).toInt();

                    if (fType != Flat)
                    {
                        panDegrees = int((qreal(panDegrees) + qreal(intFanValue) * factor));
                        tiltDegrees = int((qreal(tiltDegrees) + qreal(intFanValue) * factor));
                    }

                    list << fixture->positionToValues(QLCChannel::Pan, panDegrees);
                    list << fixture->positionToValues(QLCChannel::Tilt, tiltDegrees);
                }
            }
            break;
            case Shutter:
            {
                quint32 shCh = fixture->channelNumber(QLCChannel::Shutter, QLCChannel::MSB);
                if (shCh != QLCChannel::invalid())
                    list << SceneValue(id, shCh, uchar(value().toUInt()));
            }
            break;
            case Gobo:
            {
                quint32 goboCh = fixture->channelNumber(QLCChannel::Gobo, QLCChannel::MSB);
                if (goboCh != QLCChannel::invalid())
                    list << SceneValue(id, goboCh, uchar(value().toUInt()));
            }
            break;
            case Undefined:
            break;
        }

        progress += (1.0 / qreal(fxCount - 1));
    }

    return list;
}

QList<SceneValue> QLCPalette::valuesFromFixtureGroups(Doc *doc, QList<quint32> groups)
{
    QList<quint32> fixturesList;

    foreach (quint32 id, groups)
    {
        FixtureGroup *group = doc->fixtureGroup(id);
        if (group == NULL)
            continue;

        fixturesList.append(group->fixtureList());
    }

    return valuesFromFixtures(doc, fixturesList);
}

qreal QLCPalette::valueFactor(qreal progress)
{
    qreal factor = 1.0;
    qreal normalizedAmount = qreal(m_fanningAmount) / 100.0;

    switch (m_fanningType)
    {
        case Flat:
            // nothing to do. Factor is always 1.0
        break;
        case Linear:
        {
            if (normalizedAmount < 1.0)
            {
                if (progress > normalizedAmount)
                    factor = 1.0;
                else
                    factor = progress * normalizedAmount;
            }
            else if (normalizedAmount > 1.0)
            {
                factor = progress / normalizedAmount;
            }
            else
            {
                factor = progress;
            }
        }
        break;
        case Sine:
        {
            qreal degrees = (progress * 360.0) + 270.0;
            factor = (qSin(normalizedAmount * qDegreesToRadians(degrees)) + 1.0) / 2.0;
        }
        break;
        case Square:
        {
            factor = qSin(normalizedAmount * qDegreesToRadians(progress * 360.0)) < 0 ? 1 : 0;
        }
        break;
        case Saw:
        break;
    }

    return factor;
}

/************************************************************************
 * Fanning
 ************************************************************************/

QLCPalette::FanningType QLCPalette::fanningType() const
{
    return m_fanningType;
}

void QLCPalette::setFanningType(QLCPalette::FanningType type)
{
    if (type == m_fanningType)
        return;

    m_fanningType = type;

    emit fanningTypeChanged();
}

QString QLCPalette::fanningTypeToString(QLCPalette::FanningType type)
{
    switch (type)
    {
        case Flat:      return "Flat";
        case Linear:    return "Linear";
        case Sine:      return "Sine";
        case Square:    return "Square";
        case Saw:       return "Saw";
    }

    return "";
}

QLCPalette::FanningType QLCPalette::stringToFanningType(const QString &str)
{
    if (str == "Flat")
        return Flat;
    else if (str == "Linear")
        return Linear;
    else if (str == "Sine")
        return Sine;
    else if (str == "Square")
        return Square;
    else if (str == "Saw")
        return Saw;

    return Flat;
}

QLCPalette::FanningLayout QLCPalette::fanningLayout() const
{
    return m_fanningLayout;
}

void QLCPalette::setFanningLayout(QLCPalette::FanningLayout layout)
{
    if (layout == m_fanningLayout)
        return;

    m_fanningLayout = layout;

    emit fanningLayoutChanged();
}

QString QLCPalette::fanningLayoutToString(QLCPalette::FanningLayout layout)
{
    switch (layout)
    {
        case XAscending:    return "XAscending";
        case XDescending:   return "XDescending";
        case XCentered:     return "XCentered";
        case YAscending:    return "YAscending";
        case YDescending:   return "YDescending";
        case YCentered:     return "YCentered";
        case ZAscending:    return "ZAscending";
        case ZDescending:   return "ZDescending";
        case ZCentered:     return "ZCentered";
    }

    return "";
}

QLCPalette::FanningLayout QLCPalette::stringToFanningLayout(const QString &str)
{
    if (str == "XAscending")
        return XAscending;
    else if (str == "XDescending")
        return XDescending;
    else if (str == "XCentered")
        return XCentered;
    else     if (str == "YAscending")
        return YAscending;
    else if (str == "YDescending")
        return YDescending;
    else if (str == "YCentered")
        return YCentered;
    else if (str == "ZAscending")
        return ZAscending;
    else if (str == "ZDescending")
        return ZDescending;
    else if (str == "ZCentered")
        return ZCentered;

    return XAscending;
}

int QLCPalette::fanningAmount() const
{
    return m_fanningAmount;
}

void QLCPalette::setFanningAmount(int amount)
{
    if (amount == m_fanningAmount)
        return;

    m_fanningAmount = amount;

    emit fanningAmountChanged();
}

QVariant QLCPalette::fanningValue() const
{
    return m_fanningValue;
}

void QLCPalette::setFanningValue(QVariant value)
{
    if (value == m_fanningValue)
        return;

    m_fanningValue = value;

    emit fanningValueChanged();
}

/************************************************************************
 * Color helpers
 ************************************************************************/

QString QLCPalette::colorToString(QColor rgb, QColor wauv)
{
    QString final = rgb.name();
    final.append(wauv.name().right(6));
    return final;
}

bool QLCPalette::stringToColor(QString str, QColor &rgb, QColor &wauv)
{
    // string must be like #rrggbb or #rrggbbwwaauv
    if (str.length() != 7 && str.length() != 13)
        return false;

    rgb = QColor(str.left(7));

    if (str.length() == 13)
        wauv = QColor("#" + str.right(6));
    else
        wauv = QColor();

    return true;
}

/************************************************************************
 * Load & Save
 ************************************************************************/

bool QLCPalette::loader(QXmlStreamReader &xmlDoc, Doc *doc)
{
    QLCPalette *palette = new QLCPalette(Dimmer, doc);
    Q_ASSERT(palette != NULL);

    if (palette->loadXML(xmlDoc) == true)
    {
        doc->addPalette(palette, palette->id());
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "QLCPalette" << palette->name() << "cannot be loaded.";
        delete palette;
        return false;
    }

    return true;
}

bool QLCPalette::loadXML(QXmlStreamReader &doc)
{
    if (doc.name() != KXMLQLCPalette)
    {
        qWarning() << Q_FUNC_INFO << "Palette node not found";
        return false;
    }

    QXmlStreamAttributes attrs = doc.attributes();

    bool ok = false;
    quint32 id = attrs.value(KXMLQLCPaletteID).toString().toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Palette ID:" << attrs.value(KXMLQLCPaletteID).toString();
        return false;
    }

    setID(id);

    if (attrs.hasAttribute(KXMLQLCPaletteType) == false)
    {
        qWarning() << "Palette type not found!";
        return false;
    }

    m_type = stringToType(attrs.value(KXMLQLCPaletteType).toString());

    if (attrs.hasAttribute(KXMLQLCPaletteName))
        setName(attrs.value(KXMLQLCPaletteName).toString());

    if (attrs.hasAttribute(KXMLQLCPaletteValue))
    {
        QString strVal = attrs.value(KXMLQLCPaletteValue).toString();
        switch (m_type)
        {
            case Dimmer:
            case Pan:
            case Tilt:
                setValue(strVal.toInt());
            break;
            case Color:
                setValue(strVal);
            break;
            case PanTilt:
            {
                QStringList posList = strVal.split(",");
                if (posList.count() == 2)
                    setValue(posList.at(0).toInt(), posList.at(1).toInt());
            }
            break;
            case Shutter:   break;
            case Gobo:      break;
            case Undefined: break;
        }
    }

    if (attrs.hasAttribute(KXMLQLCPaletteFanning))
    {
        setFanningType(stringToFanningType(attrs.value(KXMLQLCPaletteFanning).toString()));

        if (attrs.hasAttribute(KXMLQLCPaletteFanLayout))
            setFanningLayout(stringToFanningLayout(attrs.value(KXMLQLCPaletteFanLayout).toString()));

        if (attrs.hasAttribute(KXMLQLCPaletteFanAmount))
            setFanningAmount(attrs.value(KXMLQLCPaletteFanAmount).toInt());

        if (attrs.hasAttribute(KXMLQLCPaletteFanValue))
        {
            QString strVal = attrs.value(KXMLQLCPaletteFanValue).toString();
            switch (m_type)
            {
                case Dimmer:
                case Pan:
                case Tilt:
                case PanTilt:
                    setFanningValue(strVal.toInt());
                break;
                case Color:
                    setFanningValue(strVal);
                break;
                case Shutter:   break;
                case Gobo:      break;
                case Undefined: break;
            }
        }
    }

    return true;
}

bool QLCPalette::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    if (m_values.isEmpty())
    {
        qWarning() << "Unable to save a Palette without value!";
        return false;
    }

    /* write a Palette entry */
    doc->writeStartElement(KXMLQLCPalette);
    doc->writeAttribute(KXMLQLCPaletteID, QString::number(this->id()));
    doc->writeAttribute(KXMLQLCPaletteType, typeToString(m_type));
    doc->writeAttribute(KXMLQLCPaletteName, this->name());

    /* write value */
    switch (m_type)
    {
        case Dimmer:
        case Pan:
        case Tilt:
        case Color:
            doc->writeAttribute(KXMLQLCPaletteValue, value().toString());
        break;
        case PanTilt:
            doc->writeAttribute(KXMLQLCPaletteValue,
                                QString("%1,%2").arg(m_values.at(0).toInt()).arg(m_values.at(1).toInt()));
        break;
        case Shutter:   break;
        case Gobo:      break;
        case Undefined: break;
    }

    /* write fanning */
    if (m_fanningType != Flat)
    {
        doc->writeAttribute(KXMLQLCPaletteFanning, fanningTypeToString(m_fanningType));
        doc->writeAttribute(KXMLQLCPaletteFanLayout, fanningLayoutToString(m_fanningLayout));
        doc->writeAttribute(KXMLQLCPaletteFanAmount, QString::number(m_fanningAmount));
        doc->writeAttribute(KXMLQLCPaletteFanValue, fanningValue().toString());
    }

    doc->writeEndElement();

    return true;
}

