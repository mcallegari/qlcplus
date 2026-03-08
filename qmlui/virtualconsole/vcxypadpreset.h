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

#include <QSharedPointer>
#include <QKeySequence>

#include "qlcinputsource.h"
#include "grouphead.h"

class QXmlStreamReader;
class QXmlStreamWriter;

#define KXMLQLCVCXYPadPreset           QStringLiteral("Preset")
#define KXMLQLCVCXYPadPresetID         QStringLiteral("ID")
#define KXMLQLCVCXYPadPresetType       QStringLiteral("Type")
#define KXMLQLCVCXYPadPresetName       QStringLiteral("Name")

#define KXMLQLCVCXYPadPresetFuncID      QStringLiteral("FuncID")
#define KXMLQLCVCXYPadPresetXPos        QStringLiteral("X")
#define KXMLQLCVCXYPadPresetYPos        QStringLiteral("Y")
#define KXMLQLCVCXYPadPresetFixture     QStringLiteral("Fixture")
#define KXMLQLCVCXYPadPresetFixtureID   QStringLiteral("ID")
#define KXMLQLCVCXYPadPresetFixtureHead QStringLiteral("Head")

class VCXYPadPreset final
{
public:
    explicit VCXYPadPreset(quint8 id);
    explicit VCXYPadPreset(const VCXYPadPreset& preset);
    ~VCXYPadPreset();

    enum PresetType
    {
        Position = 0,
        EFX,
        Scene,
        FixtureGroup
    };

    VCXYPadPreset& operator=(const VCXYPadPreset& preset);
    bool operator<(const VCXYPadPreset& right) const;
    static bool compare(const VCXYPadPreset* left, const VCXYPadPreset* right);

    static QString typeToString(PresetType type);
    static PresetType stringToType(const QString& str);

    QString color() const;

    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

public:
    quint8 m_id;
    PresetType m_type;
    QString m_name;
    QPointF m_dmxPos;
    quint32 m_funcID;
    QList<GroupHead> m_fxGroup;
    QSharedPointer<QLCInputSource> m_inputSource;
    QKeySequence m_keySequence;
};

#endif // VCXYPADPRESET_H
