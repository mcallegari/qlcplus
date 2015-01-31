/*
  Q Light Controller
  vcslider.h

  Copyright (c) Heikki Junnila

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

#include <QMutex>
#include <QList>

#include "clickandgoslider.h"
#include "clickandgowidget.h"
#include "knobwidget.h"
#include "dmxsource.h"
#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QToolButton;
class QHBoxLayout;
class QLabel;

class VCSliderProperties;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCSlider "Slider"
#define KXMLQLCVCSliderMode "SliderMode"
#define KXMLQLCVCSliderWidgetStyle "WidgetStyle"

#define KXMLQLCVCSliderValueDisplayStyle "ValueDisplayStyle"
#define KXMLQLCVCSliderValueDisplayStyleExact "Exact"
#define KXMLQLCVCSliderValueDisplayStylePercentage "Percentage"

#define KXMLQLCVCSliderClickAndGoType "ClickAndGoType"

#define KXMLQLCVCSliderInvertedAppearance "InvertedAppearance"

#define KXMLQLCVCSliderBus "Bus"
#define KXMLQLCVCSliderBusLowLimit "LowLimit"
#define KXMLQLCVCSliderBusHighLimit "HighLimit"

#define KXMLQLCVCSliderLevel "Level"
#define KXMLQLCVCSliderLevelLowLimit "LowLimit"
#define KXMLQLCVCSliderLevelHighLimit "HighLimit"
#define KXMLQLCVCSliderLevelValue "Value"
#define KXMLQLCVCSliderLevelMonitor "Monitor"

#define KXMLQLCVCSliderChannel "Channel"
#define KXMLQLCVCSliderChannelFixture "Fixture"

#define KXMLQLCVCSliderPlayback "Playback"
#define KXMLQLCVCSliderPlaybackFunction "Function"

class VCSlider : public VCWidget, public DMXSource
{
    Q_OBJECT
    Q_DISABLE_COPY(VCSlider)

    friend class VCSliderProperties;

public:
    static const QSize defaultSize;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Normal constructor */
    VCSlider(QWidget* parent, Doc* doc);

    /** Destructor */
    ~VCSlider();

    /*********************************************************************
     * ID
     *********************************************************************/
public:
    /** @reimp */
    void setID(quint32 id);

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Create a copy of this widget into the given parent */
    VCWidget* createCopy(VCWidget* parent);

protected:
    /** Copy the contents for this widget from another widget */
    bool copyFrom(const VCWidget* widget);

    /*********************************************************************
     * GUI
     *********************************************************************/
public:
    void setCaption(const QString& text);

    /** @reimp */
    void enableWidgetUI(bool enable);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    /** Edit this widget's properties */
    void editProperties();

    /*********************************************************************
     * QLC+ Mode
     *********************************************************************/
public slots:
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Slider Mode
     *********************************************************************/
public:
    enum SliderMode
    {
        Level,
        Playback,
        Submaster
    };

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
     * Value display style
     *********************************************************************/
public:
    enum ValueDisplayStyle
    {
        ExactValue,
        PercentageValue
    };

    static QString valueDisplayStyleToString(ValueDisplayStyle style);
    static ValueDisplayStyle stringToValueDisplayStyle(QString style);

    void setValueDisplayStyle(ValueDisplayStyle style);
    ValueDisplayStyle valueDisplayStyle() const;

protected:
    ValueDisplayStyle m_valueDisplayStyle;

    /*********************************************************************
     * Inverted appearance
     *********************************************************************/
public:
    bool invertedAppearance() const;
    void setInvertedAppearance(bool invert);

    /*************************************************************************
     * Class LevelChannel
     *************************************************************************/
public:
    /**
     * This class is used to store one (fixture, channel) pair that is used by
     * VCSlider to control the level of individual fixture channels.
     */
    class LevelChannel
    {
    public:
        /** Construct a new LevelChannel with the given fixture & channel */
        LevelChannel(quint32 fid, quint32 ch);
        /** Copy constructor */
        LevelChannel(const LevelChannel& lc);
        /** Comparison operator */
        bool operator==(const LevelChannel& lc) const;
        /** Sorting operator */
        bool operator<(const LevelChannel& lc) const;
        /** Save the contents of a LevelChannel instance to an XML document */
        void saveXML(QDomDocument* doc, QDomElement* root) const;

    public:
        /** The associated fixture ID */
        quint32 fixture;
        /** The associated channel within the fixture */
        quint32 channel;
    };

    /*********************************************************************
     * Level channels
     *********************************************************************/
public:
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

    /**
     * Get the list of channels that this slider controls
     *
     */
    QList <VCSlider::LevelChannel> levelChannels();

    /**
     * Set low limit for levels set thru the slider
     *
     * @param value Low limit
     */
    void setLevelLowLimit(uchar value);

    /**
     * Get low limit for levels set thru the slider
     *
     */
    uchar levelLowLimit() const;

    /**
     * Set high limit for levels set thru the slider
     *
     * @param value High limit
     */
    void setLevelHighLimit(uchar value);

    /**
     * Get high limit for levels set thru the slider
     */
    uchar levelHighLimit() const;

    /**
     * Enable/disable the channels monitor when in Level mode
     */
    void setChannelsMonitorEnabled(bool enable);

    /**
     * Return the current status of the channels monitor
     */
    bool channelsMonitorEnabled();

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

signals:
    void monitorDMXValueChanged(int value);

protected slots:
    /** Removes all level channels related to removed fixture */
    void slotFixtureRemoved(quint32 fxi_id);

    /** Slot called when the DMX levels of the controlled channels
     *  has changed */
    void slotMonitorDMXValueChanged(int value);

protected:
    QList <VCSlider::LevelChannel> m_levelChannels;
    uchar m_levelLowLimit;
    uchar m_levelHighLimit;

    QMutex m_levelValueMutex;
    bool m_levelValueChanged;
    uchar m_levelValue;

    bool m_monitorEnabled;
    uchar m_monitorValue;

    /*********************************************************************
     * Playback
     *********************************************************************/
public:
    /**
     * Set the function used as the slider's playback function (when in
     * playback mode).
     *
     * @param fid The ID of the function
     */
    void setPlaybackFunction(quint32 fid);

    /**
     * Get the function used as the slider's playback function (when in
     * playback mode).
     *
     * @return The ID of the function
     */
    quint32 playbackFunction() const;

    /**
     * Set the level of the currently selected playback function.
     *
     * @param level The current playback function's level.
     */
    void setPlaybackValue(uchar value);

    /**
     * Get the level of the currently selected playback function.
     *
     * @return The current playback function level.
     */
    uchar playbackValue() const;

    /** @reimp */
    virtual void notifyFunctionStarting(quint32 fid);

protected slots:
    void slotPlaybackFunctionRunning(quint32 fid);
    void slotPlaybackFunctionStopped(quint32 fid);
    void slotPlaybackFunctionIntensityChanged(int attrIndex, qreal fraction);

protected:
    quint32 m_playbackFunction;
    uchar m_playbackValue;
    bool m_playbackValueChanged;
    QMutex m_playbackValueMutex;

    /*********************************************************************
     * Submaster
     *********************************************************************/
protected:
    qreal m_submasterValue;

signals:
    void submasterValueChanged(qreal value);

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes);

protected:
    /** writeDMX for Level mode */
    void writeDMXLevel(MasterTimer* timer, QList<Universe*> universes);

    /** writeDMX for Playback mode */
    void writeDMXPlayback(MasterTimer* timer, QList<Universe*> universes);

    /*********************************************************************
     * Top label
     *********************************************************************/
public:
    /**
     * Set the text for the top label
     */
    void setTopLabelText(int value);

    /**
     * Get the text in the top label
     */
    QString topLabelText();

protected:
    QLabel* m_topLabel;

    /*********************************************************************
     * Slider / Knob
     *********************************************************************/
public:
    enum SliderWidgetStyle
    {
        WSlider,
        WKnob
    };

public:
    void setSliderValue(uchar value, bool noScale = false);

    int sliderValue() const;

    void setWidgetStyle(SliderWidgetStyle mode);

    SliderWidgetStyle widgetStyle() const;

    QString widgetStyleToString(SliderWidgetStyle style);

    SliderWidgetStyle stringToWidgetStyle(QString style);

    void updateFeedback();

private slots:
    void slotSliderMoved(int value);

protected:
    QHBoxLayout* m_hbox;
    QAbstractSlider* m_slider; //!< either QClickAndGoSlider or KnobWidget
    bool m_externalMovement;
    SliderWidgetStyle m_widgetMode;

    /*********************************************************************
     * Bottom label
     *********************************************************************/
public:
    /**
     * Set the text for the bottom label
     */
    void setBottomLabelText(const QString& text);

    /**
     * Get the text in the top label
     */
    QString bottomLabelText();

protected:
    QLabel* m_bottomLabel;

    /*********************************************************************
     * Click & Go Button
     *********************************************************************/
public:

    /**
     * Set the Click & Go type. Fundamental to decide
     * the popup behaviour
     */
    void setClickAndGoType(ClickAndGoWidget::ClickAndGo type);

    /**
     * Returns the Click & Go type
     */
    ClickAndGoWidget::ClickAndGo clickAndGoType() const;

    /**
     * Create or update the Click And Go widget (if applicable)
     */
    void setupClickAndGoWidget();

    /**
     * Returns the Click & Go widget. Used by
     * configuration dialog to setup the widget
     */
    ClickAndGoWidget* getClickAndGoWidget();

protected:
    void setClickAndGoWidgetFromLevel(uchar level);

private slots:
    void slotClickAndGoLevelChanged(uchar level);
    void slotClickAndGoColorChanged(QRgb color);
    void slotClickAndGoLevelAndPresetChanged(uchar level, QImage img);

protected:
    ClickAndGoWidget::ClickAndGo m_cngType;
    QHBoxLayout *m_cngBox;
    QToolButton *m_cngButton;
    QMenu *m_menu;
    ClickAndGoWidget *m_cngWidget;
    QColor m_cngRGBvalue;

    /*********************************************************************
     * External input
     *********************************************************************/
protected slots:
    /** Called when an external input device produces input data */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /*********************************************************************
     * Intensity
     *********************************************************************/
public:
    /** @reimp */
    void adjustIntensity(qreal val);

signals:
    void valueChanged(QString val);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement* root);
    bool loadXMLLevel(const QDomElement* level_root);
    bool loadXMLPlayback(const QDomElement* pb_root);

    bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

/** @} */

#endif
