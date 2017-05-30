/*
  Q Light Controller Plus
  functionspeeds.h

  Copyright (C) 2016 Massimo Callegari
                     David Garyga

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

#ifndef FUNCTIONSPEEDS_H
#define FUNCTIONSPEEDS_H

#include <QString>
#include "speed.h"

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCFunctionSpeeds "Speed"
#define KXMLQLCFunctionSpeedsType "Type"
#define KXMLQLCFunctionSpeedsFadeIn "FadeIn"
#define KXMLQLCFunctionSpeedsHold "Hold"
#define KXMLQLCFunctionSpeedsFadeOut "FadeOut"
#define KXMLQLCFunctionSpeedsDuration "Duration"

class QXmlStreamReader;
class QXmlStreamWriter;

// FunctionSpeeds groups the components of a function's speeds:
// fadeIn, hold, fadeOut, and duration(which is fadeIn+hold)
class FunctionSpeeds
{
    Q_GADGET

private:
    Speed m_fadeIn;
    Speed m_hold;
    Speed m_fadeOut;

public:
    enum SpeedComponentType
    {
        FadeIn = 0,
        Hold,
        FadeOut,
        Duration
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(SpeedComponentType)
#endif

public:
    explicit FunctionSpeeds(
            quint32 fadeIn = Speed::originalValue(),
            quint32 hold = Speed::originalValue(),
            quint32 fadeOut = Speed::originalValue());

    /**
     * Set the speed tempo type of this FunctionSpeeds.
     * When switching from a type to another, the current fade in, hold, fade out
     * and duration times will be converted to the new type.
     *
     * @param type the speed tempo type
     */
    void setTempoType(Speed::TempoType tempoType, float beatTime);

    /** Get the FunctionSpeeds current speed tempo type */
    Speed::TempoType tempoType() const;

    /**
     * Get/Set the value of the desired speed component.
     * The unit is the current Type of the FunctionSpeeds
     */
    quint32 fadeIn() const;
    void setFadeIn(quint32 fadeIn);
    quint32 hold() const;
    void setHold(quint32 hold);
    quint32 fadeOut() const;
    void setFadeOut(quint32 fadeOut);
    quint32 duration() const;
    void setDuration(quint32 duration);

    /**
     * Get/Set the value of the desired speed component in ms.
     * beatTime in ms.
     */
    quint32 msFadeIn(float beatTime) const;
    void setMsFadeIn(quint32 fadeIn, float beatTime);
    quint32 msHold(float beatTime) const;
    void setMsHold(quint32 hold, float beatTime);
    quint32 msFadeOut(float beatTime) const;
    void setMsFadeOut(quint32 fadeOut, float beatTime);
    quint32 msDuration(float beatTime) const;
    void setMsDuration(quint32 duration, float beatTime);

    /**
     * Get/Set the value of the desired speed component in beats(*1000).
     * beatTime in ms.
     */
    quint32 beatsFadeIn(float beatTime) const;
    void setBeatsFadeIn(quint32 fadeIn, float beatTime);
    quint32 beatsHold(float beatTime) const;
    void setBeatsHold(quint32 hold, float beatTime);
    quint32 beatsFadeOut(float beatTime) const;
    void setBeatsFadeOut(quint32 fadeOut, float beatTime);
    quint32 beatsDuration(float beatTime) const;
    void setBeatsDuration(quint32 duration, float beatTime);

    /** Load the contents of a function speeds node */
    bool loadXML(QXmlStreamReader &speedRoot, QString const& nodeName = KXMLQLCFunctionSpeeds);

    /** Save function speeds values in $doc */
    bool saveXML(QXmlStreamWriter *doc, QString const& nodeName = KXMLQLCFunctionSpeeds) const;
};

/** @} */

#endif
