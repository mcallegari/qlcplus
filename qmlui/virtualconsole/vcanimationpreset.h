/*
  Q Light Controller Plus
  vcanimationpreset.h

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

#ifndef VCANIMATIONPRESET_H
#define VCANIMATIONPRESET_H

#include <QColor>
#include <QMap>

class QXmlStreamReader;
class QXmlStreamWriter;

/** XML tags shared with the legacy VCMatrix widget so presets created in
 *  one editor remain readable by the other. */
#define KXMLQLCVCAnimationPreset              QStringLiteral("Control")
#define KXMLQLCVCAnimationPresetID            QStringLiteral("ID")
#define KXMLQLCVCAnimationPresetType          QStringLiteral("Type")
#define KXMLQLCVCAnimationPresetColor         QStringLiteral("Color")
#define KXMLQLCVCAnimationPresetResource      QStringLiteral("Resource")
#define KXMLQLCVCAnimationPresetProperty      QStringLiteral("Property")
#define KXMLQLCVCAnimationPresetPropertyName  QStringLiteral("Name")

/** A single preset entry of a VCAnimation widget.
 *
 *  This is the qmlui counterpart of the legacy VCMatrixControl. It is
 *  intentionally kept compatible with the VCMatrix child-element XML so
 *  that projects can be shared between the two virtual consoles.
 */
class VCAnimationPreset final
{
public:
    explicit VCAnimationPreset(quint8 id);
    VCAnimationPreset(const VCAnimationPreset &other);
    ~VCAnimationPreset();

    VCAnimationPreset &operator=(const VCAnimationPreset &other);

    enum ControlType
    {
        Color1 = 0,
        Color2,
        Color3,
        Color4,
        Color5,
        Color1Knob,
        Color2Knob,
        Color3Knob,
        Color4Knob,
        Color5Knob,
        Color1Reset,
        Color2Reset,
        Color3Reset,
        Color4Reset,
        Color5Reset,
        Animation,
        Text
    };

    enum WidgetType
    {
        Button,
        Knob
    };

    /** Returns whether this preset is rendered as a button or a knob */
    WidgetType widgetType() const;

    /** Returns the color slot index (0..4) this preset affects, or -1 */
    int colorIndex() const;

    /** This is for Control Knobs:
     *  extract the value for this knob from an RGB color */
    quint8 rgbToValue(QRgb color) const;

    /** This is for Control Knobs:
     *  get the rgb value for this value of the knob */
    QRgb valueToRgb(quint8 value) const;

    static QString typeToString(ControlType type);
    static ControlType stringToType(const QString &str);

    bool operator<(const VCAnimationPreset &right) const;
    static bool compare(const VCAnimationPreset *left, const VCAnimationPreset *right);

    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

public:
    /** Preset unique ID. Note that id = 0 is reserved for the main fader */
    quint8 m_id;

    /** The preset type */
    ControlType m_type;

    /** The preset color if m_type is a Color or Color Knob */
    QColor m_color;

    /** Resource can be the name of an algorithm preset or the matrix text */
    QString m_resource;

    /** A map holding the requested script properties */
    QMap<QString, QString> m_properties;
};

#endif // VCANIMATIONPRESET_H
