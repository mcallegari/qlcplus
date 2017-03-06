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
#include "treemodel.h"

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

    Q_PROPERTY(QVariant channelsList READ channelsList CONSTANT)

    Q_PROPERTY(SliderWidgetStyle widgetStyle READ widgetStyle WRITE setWidgetStyle NOTIFY widgetStyleChanged)
    Q_PROPERTY(ValueDisplayStyle valueDisplayStyle READ valueDisplayStyle WRITE setValueDisplayStyle NOTIFY valueDisplayStyleChanged)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance NOTIFY invertedAppearanceChanged)
    Q_PROPERTY(SliderMode sliderMode READ sliderMode WRITE setSliderMode NOTIFY sliderModeChanged)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(quint32 playbackFunction READ playbackFunction WRITE setPlaybackFunction NOTIFY playbackFunctionChanged)

    Q_PROPERTY(int levelLowLimit READ levelLowLimit WRITE setLevelLowLimit NOTIFY levelLowLimitChanged)
    Q_PROPERTY(int levelHighLimit READ levelHighLimit WRITE setLevelHighLimit NOTIFY levelHighLimitChanged)

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

    QVariant channelsList();

protected:
    /** Reference to a tree model representing Groups/Fitures/Channels
      * This is created only when the UI requests it to confgure the Level mode */
    TreeModel *m_channelsTree;

    /*********************************************************************
     * Widget style
     *********************************************************************/
public:
    enum SliderWidgetStyle
    {
        WSlider,
        WKnob
    };
    Q_ENUM(SliderWidgetStyle)

    /** Helper methods for SliderWidgetStyle <--> QString conversion */
    QString widgetStyleToString(SliderWidgetStyle style);
    SliderWidgetStyle stringToWidgetStyle(QString style);

    /** Get/Set the Slider value display style */
    SliderWidgetStyle widgetStyle() const;
    void setWidgetStyle(SliderWidgetStyle mode);

signals:
    void widgetStyleChanged(SliderWidgetStyle widgetStyle);

protected:
    SliderWidgetStyle m_widgetMode;

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
     * Level mode
     *********************************************************************/
public:
    /** Set/Get the lower limit for levels set through the slider */
    void setLevelLowLimit(uchar value);
    uchar levelLowLimit() const;

    /** Set/Get high limit for levels set through the slider */
    void setLevelHighLimit(uchar value);
    uchar levelHighLimit() const;

    /**
     * Add a channel from a fixture into the slider's list of
     * level channels.
     *
     * @param fixture Fixture ID
     * @param channel A channel from the fixture
     */
    void addLevelChannel(quint32 fixture, quint32 channel);

    /**
     * Remove a fixture & channel from the slider's list of
     * level channels.
     *
     * @param fixture Fixture ID
     * @param channel A channel from the fixture
     */
    void removeLevelChannel(quint32 fixture, quint32 channel);

    /**
     * Clear the list of level channels
     *
     */
    void clearLevelChannels();

    /** Get the list of channels that this slider controls */
    QList <SceneValue> levelChannels();

signals:
    void levelLowLimitChanged();
    void levelHighLimitChanged();

protected:
    /**
     * Set the level to all channels that have been assigned to
     * the slider.
     *
     * @param value DMX value
     */
    void setLevelValue(uchar value);

    /**
     * Get the current "level" mode value
     */
    uchar levelValue() const;

protected:
    QList <SceneValue> m_levelChannels;
    uchar m_levelLowLimit;
    uchar m_levelHighLimit;

    QMutex m_levelValueMutex;
    uchar m_levelValue;
    bool m_levelValueChanged;

    bool m_monitorEnabled;
    uchar m_monitorValue;

    /*********************************************************************
     * Playback mode
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
    bool loadXMLLevel(QXmlStreamReader &level_root);
    bool loadXMLPlayback(QXmlStreamReader &pb_root);

    //bool saveXML(QXmlStreamWriter *doc);
};

#endif
