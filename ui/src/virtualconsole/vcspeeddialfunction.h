/*
  Q Light Controller Plus
  vcspeeddialfunction.h

  Copyright (C) 2014 David Garyga

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

#ifndef VCSPEEDDIALFUNCTION_H
#define VCSPEEDDIALFUNCTION_H

#include "function.h"

class QXmlStreamReader;
class QXmlStreamWriter;

/** @addtogroup ui_vc_props
 * @{
 */

class VCSpeedDialFunction
{
    /************************************************************************
     * Speed Multiplier Definition
     ************************************************************************/
public:
    enum SpeedMultiplier
    {
        None = 0,
        Zero,
        OneSixteenth,
        OneEighth,
        OneFourth,
        Half,
        One,
        Two,
        Four,
        Eight,
        Sixteen
    };

    /**
     * Get the ordered list of existing speed multiplier names
     */
    static const QStringList &speedMultiplierNames();
    /**
     * Get the ordered list of existing speed multiplier values, times 1000
     * (value for multiplier 1/4 will be 250)
     */
    static const QVector <quint32> &speedMultiplierValuesTimes1000();

    /************************************************************************
     * Initialization
     ***********************************************************************/
public:
    /** Construct a new VCSpeedDialFunction with the given attributes */
    VCSpeedDialFunction(quint32 aFid = Function::invalidId(),
            SpeedMultiplier aFadeIn = None, SpeedMultiplier aFadeOut = None, SpeedMultiplier aDuration = One);

    /************************************************************************
     * Load & Save
     ***********************************************************************/
public:
    /** Load SpeedDialFunction with default values for multipliers */
    bool loadXML(QXmlStreamReader &root, SpeedMultiplier aFadeIn = None,
                 SpeedMultiplier aFadeOut = None, SpeedMultiplier aDuration = One);

    /** Save SpeedDialFunction contents to $doc, under $root */
    bool saveXML(QXmlStreamWriter *doc) const;

public:
    quint32 functionId;
    SpeedMultiplier fadeInMultiplier;
    SpeedMultiplier fadeOutMultiplier;
    SpeedMultiplier durationMultiplier;
};

/** @} */

#endif
