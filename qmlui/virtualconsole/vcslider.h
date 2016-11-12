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

    Q_PROPERTY(ValueDisplayStyle valueDisplayStyle READ valueDisplayStyle WRITE setValueDisplayStyle NOTIFY valueDisplayStyleChanged)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance NOTIFY invertedAppearanceChanged)
    Q_PROPERTY(SliderMode sliderMode READ sliderMode WRITE setSliderMode NOTIFY sliderModeChanged)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(quint32 playbackFunction READ playbackFunction WRITE setPlaybackFunction NOTIFY playbackFunctionChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCSlider(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCSlider();

    /** @reimp */
    void setID(quint32 id);

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /*********************************************************************
     * Display style
     *********************************************************************/
public:
    enum ValueDisplayStyle
    {
        DMXValue,
        PercentageValue
    };
    Q_ENUM(ValueDisplayStyle)

    /** Helper methods for ValueDisplayStyle <--> QString conversion */
    static QString valueDisplayStyleToString(ValueDisplayStyle style);
    static ValueDisplayStyle stringToValueDisplayStyle(QString style);

    /** Get/Set the Slider value display style */
    ValueDisplayStyle valueDisplayStyle() const;
    void setValueDisplayStyle(ValueDisplayStyle style);

    /** Get/Set the Slider inverted appearance mode */
    bool invertedAppearance() const;
    void setInvertedAppearance(bool inverted);

signals:
    void valueDisplayStyleChanged(ValueDisplayStyle valueDisplayStyle);
    void invertedAppearanceChanged(bool inverted);

protected:
    ValueDisplayStyle m_valueDisplayStyle;
    bool m_invertedAppearance;

    /*********************************************************************
     * Slider Mode
     *********************************************************************/
public:
    enum SliderMode { Level, Playback, Submaster, GrandMaster };
    Q_ENUM(SliderMode)

public:
    /** Helper methods for SliderMode <--> QString conversion */
    static QString sliderModeToString(SliderMode mode);
    static SliderMode stringToSliderMode(const QString& mode);

    /** Get/Set the current slider mode */
    SliderMode sliderMode() const;
    void setSliderMode(SliderMode mode);

signals:
    void sliderModeChanged(SliderMode mode);

protected:
    SliderMode m_sliderMode;

    /*********************************************************************
     * Slider value
     *********************************************************************/
public:
    int value() const;
    void setValue(int value);

signals:
    void valueChanged(int value);

protected:
    int m_value;

    /*********************************************************************
     * Playback
     *********************************************************************/
public:
    quint32 playbackFunction() const;
    void setPlaybackFunction(quint32 playbackFunction);

private:
    FunctionParent functionParent() const;

signals:
    void playbackFunctionChanged(quint32 playbackFunction);

protected:
    quint32 m_playbackFunction;

    /*********************************************************************
     * External input
     *********************************************************************/
public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    //bool saveXML(QXmlStreamWriter *doc);
};

#endif
