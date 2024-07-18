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
#include "grouphead.h"

class QXmlStreamReader;
class QXmlStreamWriter;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCXYPadPreset         QString("Preset")
#define KXMLQLCVCXYPadPresetID       QString("ID")
#define KXMLQLCVCXYPadPresetType     QString("Type")
#define KXMLQLCVCXYPadPresetName     QString("Name")

#define KXMLQLCVCXYPadPresetFuncID      QString("FuncID")
#define KXMLQLCVCXYPadPresetXPos        QString("X")
#define KXMLQLCVCXYPadPresetYPos        QString("Y")
#define KXMLQLCVCXYPadPresetFixture     QString("Fixture")
#define KXMLQLCVCXYPadPresetFixtureID   QString("ID")
#define KXMLQLCVCXYPadPresetFixtureHead QString("Head")

class VCXYPadPreset
{
public:
    explicit VCXYPadPreset(quint8 id);
    explicit VCXYPadPreset(VCXYPadPreset const& other);

    /** Destructor */
    ~VCXYPadPreset();

    enum PresetType
    {
        Position = 0,
        EFX,
        Scene,
        FixtureGroup
    };

    QString getColor() const;

    void setFunctionID(quint32 id);
    quint32 functionID() const;

    void setPosition(QPointF pos);
    QPointF position() const;

    void setFixtureGroup(QList<GroupHead>heads);
    QList<GroupHead> fixtureGroup() const;

public:
    VCXYPadPreset& operator=(const VCXYPadPreset& vcpp);
    bool operator<(VCXYPadPreset const& right) const;
    static bool compare(VCXYPadPreset const* left, VCXYPadPreset const* right);

    static QString typeToString(PresetType type);
    static PresetType stringToType(QString str);

public:
    /** The preset unique ID */
    quint8 m_id;

    /** The preset type */
    PresetType m_type;

    /** The preset name */
    QString m_name;

    /** Position in DMX coordinates 0.0..(256.0 - 1/256)
      * when the preset type is Position */
    QPointF m_dmxPos;

    /** ID of the Function controlled by this preset
     *  when type is EFX or Scene */
    quint32 m_funcID;

    /** A list of heads activated by this preset
     *  when type is FixtureGroup */
    QList<GroupHead> m_fxGroup;

    QSharedPointer<QLCInputSource> m_inputSource;
    QKeySequence m_keySequence;

    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(QXmlStreamReader &root);

    /** Save properties and contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc);
};

/** @} */

#endif // VCXYPADPRESET_H
