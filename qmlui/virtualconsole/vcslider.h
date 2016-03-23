/*
  Q Light Controller Plus
  vcslider.h

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

#ifndef VCSLIDER_H
#define VCSLIDER_H

#include "vcwidget.h"

#define KXMLQLCVCSlider "Slider"

#define KXMLQLCVCSliderMode "SliderMode"
#define KXMLQLCVCSliderWidgetStyle "WidgetStyle"

#define KXMLQLCVCSliderValueDisplayStyle "ValueDisplayStyle"
#define KXMLQLCVCSliderValueDisplayStyleExact "Exact"
#define KXMLQLCVCSliderValueDisplayStylePercentage "Percentage"

#define KXMLQLCVCSliderClickAndGoType "ClickAndGoType"

#define KXMLQLCVCSliderInvertedAppearance "InvertedAppearance"

#define KXMLQLCVCSliderLevel "Level"
#define KXMLQLCVCSliderLevelLowLimit "LowLimit"
#define KXMLQLCVCSliderLevelHighLimit "HighLimit"
#define KXMLQLCVCSliderLevelValue "Value"
#define KXMLQLCVCSliderLevelMonitor "Monitor"

#define KXMLQLCVCSliderChannel "Channel"
#define KXMLQLCVCSliderChannelFixture "Fixture"

#define KXMLQLCVCSliderPlayback "Playback"
#define KXMLQLCVCSliderPlaybackFunction "Function"

class FunctionParent;

class VCSlider : public VCWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/

public:
    VCSlider(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCSlider();

    /** @reimp */
    void setID(quint32 id);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /*********************************************************************
     * Slider Mode
     *********************************************************************/
public:
    enum SliderMode { Level, Playback, Submaster };
    Q_ENUMS(SliderMode)

public:
    /**
     * Convert a SliderMode enum to a string that can be saved into
     * an XML file.
     *
     * @param mode The mode to convert
     * @return A string
     */
    static QString sliderModeToString(SliderMode mode);

    /**
     * Convert a string into a SliderMode enum.
     *
     * @param mode The string to convert
     * @return SliderMode
     */
    static SliderMode stringToSliderMode(const QString& mode);

    /**
     * Get the slider's current SliderMode
     */
    SliderMode sliderMode() const;

    /**
     * Change the slider's current SliderMode
     */
    void setSliderMode(SliderMode mode);

protected:
    SliderMode m_sliderMode;

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(QXmlStreamReader &root);
    //bool saveXML(QXmlStreamWriter *doc);
};

#endif
