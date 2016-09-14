/*
  Q Light Controller Plus
  cue.h

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#ifndef CUE_H
#define CUE_H

#include <QString>
#include <QHash>

#include "scenevalue.h"

class QXmlStreamReader;
class QXmlStreamWriter;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCCue "Cue"
#define KXMLQLCCueName "Name"
#define KXMLQLCCueValue "Value"
#define KXMLQLCCueValueChannel "Channel"

#define KXMLQLCCueSpeed         "Speed"
#define KXMLQLCCueSpeedFadeIn   "FadeIn"
#define KXMLQLCCueSpeedFadeOut  "FadeOut"
#define KXMLQLCCueSpeedDuration "Duration"

class Cue
{
public:
    Cue(const QString& name = QString());
    Cue(const QHash <uint,uchar> values);
    Cue(const Cue& cue);
    ~Cue();

    /************************************************************************
     * Name
     ************************************************************************/
public:
    void setName(const QString& str);
    QString name() const;

private:
    QString m_name;

    /************************************************************************
     * Values
     ************************************************************************/
public:
    void setValue(uint channel, uchar value);
    void unsetValue(uint channel);
    uchar value(uint channel) const;

    QHash <uint,uchar> values() const;

private:
    QHash <uint,uchar> m_values;

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    void setFadeIn(uint ms);
    uint fadeIn() const;

    void setFadeOut(uint ms);
    uint fadeOut() const;

    void setDuration(uint ms);
    uint duration() const;

private:
    uint m_fadeIn;
    uint m_fadeOut;
    uint m_duration;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

private:
    bool loadXMLSpeed(QXmlStreamReader &speedRoot);
    bool saveXMLSpeed(QXmlStreamWriter *doc) const;
};

/** @} */

#endif
