/*
  Q Light Controller Plus
  vcxypadpreset.h

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

#ifndef VCXYPADPRESET_H
#define VCXYPADPRESET_H

#include <QKeySequence>

#include "qlcinputsource.h"

class QDomDocument;
class QDomElement;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCXYPadPreset         "Preset"
#define KXMLQLCVCXYPadPresetID       "ID"
#define KXMLQLCVCXYPadPresetType     "Type"

#define KXMLQLCVCXYPadPresetEFXID    "EFXID"
#define KXMLQLCVCXYPadPresetXPos     "X"
#define KXMLQLCVCXYPadPresetYPos     "Y"

#define KXMLQLCVCXYPadPresetInput         "Input"
#define KXMLQLCVCXYPadPresetInputUniverse "Universe"
#define KXMLQLCVCXYPadPresetInputChannel  "Channel"

#define KXMLQLCVCXYPadPresetKey "Key"

class VCXYPadPreset
{
public:
    explicit VCXYPadPreset(quint8 id);
    explicit VCXYPadPreset(VCXYPadPreset const& vcpp);

    /** Destructor */
    ~VCXYPadPreset();

    enum PresetType
    {
        Position = 0,
        EFX
    };

    void setEFXID(quint32 id);
    quint32 efxID() const;

    void setPosition(QPointF pos);
    QPointF position() const;

public:
    bool operator<(VCXYPadPreset const& right) const;
    static bool compare(VCXYPadPreset const* left, VCXYPadPreset const* right);

protected:
    static QString typeToString(PresetType type);
    static PresetType stringToType(QString str);

public:
    /** The preset unique ID */
    quint8 m_id;

    /** The preset type */
    PresetType m_type;

    /** Position in DMX coordinates 0.0..(256.0 - 1/256)
      * when the preset type is Position */
    QPointF m_dmxPos;

    /** ID of the EFX when the preset type is EFX */
    quint32 m_efxID;

    QSharedPointer<QLCInputSource> m_inputSource;
    QKeySequence m_keySequence;

    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(const QDomElement& root);

    /** Save properties and contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* xypad_root);
};

/** @} */

#endif // VCXYPADPRESET_H
