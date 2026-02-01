/*
  Q Light Controller Plus
  vcspeeddialpreset.h

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

#ifndef VCSPEEDDIALPRESET_H
#define VCSPEEDDIALPRESET_H

#include <QSharedPointer>
#include <QKeySequence>

#include "qlcinputsource.h"

class QXmlStreamReader;
class QXmlStreamWriter;

#define KXMLQLCVCSpeedDialPreset         QStringLiteral("Preset")
#define KXMLQLCVCSpeedDialPresetID       QStringLiteral("ID")
#define KXMLQLCVCSpeedDialPresetName     QStringLiteral("Name")
#define KXMLQLCVCSpeedDialPresetValue    QStringLiteral("Value")

class VCSpeedDialPreset final
{
public:
    explicit VCSpeedDialPreset(quint8 id);
    explicit VCSpeedDialPreset(VCSpeedDialPreset const& preset);
    ~VCSpeedDialPreset();

    VCSpeedDialPreset& operator=(const VCSpeedDialPreset& preset);
    bool operator<(VCSpeedDialPreset const& right) const;
    static bool compare(VCSpeedDialPreset const* left, VCSpeedDialPreset const* right);

    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);

public:
    quint8 m_id;
    QString m_name;
    int m_value;
    QSharedPointer<QLCInputSource> m_inputSource;
    QKeySequence m_keySequence;
};

#endif // VCSPEEDDIALPRESET_H
