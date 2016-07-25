/*
  Q Light Controller Plus
  functiontimings.h

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

#ifndef FUNCTIONTIMINGS_H
#define FUNCTIONTIMINGS_H

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCFunctionLegacySpeed "Speed"
#define KXMLQLCFunctionLegacySpeedFadeIn "FadeIn"
#define KXMLQLCFunctionLegacySpeedHold "Hold"
#define KXMLQLCFunctionLegacySpeedFadeOut "FadeOut"
#define KXMLQLCFunctionLegacySpeedDuration "Duration"

#define KXMLQLCFunctionTimings "Timings"
#define KXMLQLCFunctionTimingsFadeIn KXMLQLCFunctionLegacySpeedFadeIn
#define KXMLQLCFunctionTimingsHold KXMLQLCFunctionLegacySpeedHold
#define KXMLQLCFunctionTimingsFadeOut KXMLQLCFunctionLegacySpeedDuration

struct FunctionTimings
{
    quint32 fadeIn;
    quint32 hold;
    quint32 fadeOut;

    FunctionTimings(quint32 fadeIn = defaultValue(),
            quint32 hold = defaultValue(),
            quint32 fadeOut = defaultValue());

    quint32 duration() const;
    void setDuration(quint32 duration);

    static quint32 defaultValue();
    static quint32 infiniteValue();

    /** Pretty-print the given timing into a QString */
    static QString valueToString(quint32 ms);

    /** Returns value in msec of a string created by speedToString */
    static quint32 stringToValue(QString string);

    /** Safe operations */
    static quint32 normalize(quint32 value);
    static quint32 add(quint32 left, quint32 right);
    static quint32 substract(quint32 left, quint32 right);

    /** Load the contents of a timings node */
    bool loadXML(QXmlStreamReader &speedRoot);

    /** Load the contents of a speed node */
    bool loadXMLLegacy(QXmlStreamReader &speedRoot);

    /** Save timings values in $doc */
    bool saveXML(QXmlStreamWriter *doc) const;
};

/** @} */

#endif
