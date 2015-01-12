/*
  Q Light Controller Plus
  vcmatrixcontrol.h

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

#ifndef VCMATRIXCONTROL_H
#define VCMATRIXCONTROL_H

#include <QKeySequence>
#include <QColor>

#include "qlcinputsource.h"

class QDomDocument;
class QDomElement;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCMatrixControl         "Control"
#define KXMLQLCVCMatrixControlID       "ID"
#define KXMLQLCVCMatrixControlType     "Type"
#define KXMLQLCVCMatrixControlColor    "Color"
#define KXMLQLCVCMatrixControlResource "Resource"
#define KXMLQLCVCMatrixControlProperty "Property"
#define KXMLQLCVCMatrixControlPropertyName "Name"

#define KXMLQLCVCMatrixControlInput         "Input"
#define KXMLQLCVCMatrixControlInputUniverse "Universe"
#define KXMLQLCVCMatrixControlInputChannel  "Channel"

#define KXMLQLCVCMatrixControlKey "Key"

class VCMatrixControl
{

public:
    explicit VCMatrixControl(quint8 id);
    explicit VCMatrixControl(VCMatrixControl const& vcmc);

    /** Destructor */
    ~VCMatrixControl();

    enum ControlType
    {
        StartColor = 0,
        EndColor,
        Animation,
        Image,
        Text,
        ResetEndColor,
        StartColorKnob,
        EndColorKnob
    };

    enum WidgetType
    {
        Button,
        Knob
    };

    WidgetType widgetType() const;

    /** This is for Control Knobs:
     *  extract the value for this knob from and RGB color
     */
    quint8 rgbToValue(QRgb color) const;

    /** This is for Control Knobs:
     *  get the rgb value for this value of the knob
     */
    QRgb valueToRgb(quint8 value) const;
protected:
    static QString typeToString(ControlType type);
    static ControlType stringToType(QString str);

public:
    bool operator<(VCMatrixControl const& right) const;
    static bool compare(VCMatrixControl const* left, VCMatrixControl const* right);
    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(const QDomElement& root);

    /** Save properties and contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* mtx_root);

public:
    /** Preset unique ID
     *  Note that id = 0 is reserved for the main Matrix slider
     */
    quint8 m_id;

    /** The control type */
    ControlType m_type;

    /** The preset color if m_type == StartColor or EndColor */
    QColor m_color;

    /** Resource can be:
     *  - the name of an animation preset
     *  - the absolute path of a picture
     *  - the matrix text
     */
    QString m_resource;

    /** A map holding the requested script properties */
    QHash<QString, QString> m_properties;

    QLCInputSource *m_inputSource;
    QKeySequence m_keySequence;
};

/** @} */

#endif // VCMATRIXCONTROL_H
