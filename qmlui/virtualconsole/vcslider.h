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
#include "dmxsource.h"
#include "grandmaster.h"

#define KXMLQLCVCSlider QString("Slider")

#define KXMLQLCVCSliderMode         QString("SliderMode")
#define KXMLQLCVCSliderWidgetStyle  QString("WidgetStyle")

#define KXMLQLCVCSliderValueDisplayStyle            QString("ValueDisplayStyle")
#define KXMLQLCVCSliderValueDisplayStyleExact       QString("Exact")
#define KXMLQLCVCSliderValueDisplayStylePercentage  QString("Percentage")

#define KXMLQLCVCSliderClickAndGoType QString("ClickAndGoType")

#define KXMLQLCVCSliderInvertedAppearance QString("InvertedAppearance")

#define KXMLQLCVCSliderLevel            QString("Level")
#define KXMLQLCVCSliderLevelLowLimit    QString("LowLimit")
#define KXMLQLCVCSliderLevelHighLimit   QString("HighLimit")
#define KXMLQLCVCSliderLevelValue       QString("Value")
#define KXMLQLCVCSliderLevelMonitor     QString("Monitor")
#define KXMLQLCVCSliderOverrideReset    QString("Reset")
#define KXMLQLCVCSliderFunctionFlash    QString("Flash")

#define KXMLQLCVCSliderChannel          QString("Channel")
#define KXMLQLCVCSliderChannelFixture   QString("Fixture")

#define KXMLQLCVCSliderPlayback             QString("Playback") // LEGACY
#define KXMLQLCVCSliderAdjust               QString("Adjust")
#define KXMLQLCVCSliderAdjustAttribute      QString("Attribute")
#define KXMLQLCVCSliderControlledFunction   QString("Function")

class FunctionParent;
class GenericFader;

class VCSlider : public VCWidget, public DMXSource
{
    Q_OBJECT

    Q_PROPERTY(QVariant channelsList READ channelsList CONSTANT)

    Q_PROPERTY(SliderWidgetStyle widgetStyle READ widgetStyle WRITE setWidgetStyle NOTIFY widgetStyleChanged)
    Q_PROPERTY(ValueDisplayStyle valueDisplayStyle READ valueDisplayStyle WRITE setValueDisplayStyle NOTIFY valueDisplayStyleChanged)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance NOTIFY invertedAppearanceChanged)
    Q_PROPERTY(SliderMode sliderMode READ sliderMode WRITE setSliderMode NOTIFY sliderModeChanged)

    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(qreal rangeLowLimit READ rangeLowLimit WRITE setRangeLowLimit NOTIFY rangeLowLimitChanged)
    Q_PROPERTY(qreal rangeHighLimit READ rangeHighLimit WRITE setRangeHighLimit NOTIFY rangeHighLimitChanged)

    Q_PROPERTY(bool monitorEnabled READ monitorEnabled WRITE setMonitorEnabled NOTIFY monitorEnabledChanged)
    Q_PROPERTY(int monitorValue READ monitorValue NOTIFY monitorValueChanged)
    Q_PROPERTY(bool isOverriding READ isOverriding WRITE setIsOverriding NOTIFY isOverridingChanged)

    Q_PROPERTY(bool adjustFlashEnabled READ adjustFlashEnabled WRITE setAdjustFlashEnabled NOTIFY adjustFlashEnabledChanged)

    Q_PROPERTY(quint32 controlledFunction READ controlledFunction WRITE setControlledFunction NOTIFY controlledFunctionChanged)
    Q_PROPERTY(int controlledAttribute READ controlledAttribute WRITE setControlledAttribute NOTIFY controlledAttributeChanged)
    Q_PROPERTY(QStringList availableAttributes READ availableAttributes NOTIFY availableAttributesChanged)
    Q_PROPERTY(qreal attributeMinValue READ attributeMinValue NOTIFY attributeMinValueChanged)
    Q_PROPERTY(qreal attributeMaxValue READ attributeMaxValue NOTIFY attributeMaxValueChanged)

    Q_PROPERTY(GrandMaster::ValueMode grandMasterValueMode READ grandMasterValueMode WRITE setGrandMasterValueMode NOTIFY grandMasterValueModeChanged)
    Q_PROPERTY(GrandMaster::ChannelMode grandMasterChannelMode READ grandMasterChannelMode WRITE setGrandMasterChannelMode NOTIFY grandMasterChannelModeChanged)

    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)

    Q_PROPERTY(ClickAndGoType clickAndGoType READ clickAndGoType WRITE setClickAndGoType NOTIFY clickAndGoTypeChanged)
    Q_PROPERTY(QColor cngPrimaryColor READ cngPrimaryColor NOTIFY cngPrimaryColorChanged)
    Q_PROPERTY(QColor cngSecondaryColor READ cngSecondaryColor NOTIFY cngSecondaryColorChanged)
    Q_PROPERTY(QVariantList clickAndGoPresetsList READ clickAndGoPresetsList NOTIFY clickAndGoPresetsListChanged)
    Q_PROPERTY(QString cngPresetResource READ cngPresetResource NOTIFY cngPresetResourceChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCSlider(Doc* doc = nullptr, QObject *parent = nullptr);
    virtual ~VCSlider();

    /** @reimp */
    QString defaultCaption();

    /** @reimp */
    void setupLookAndFeel(qreal pixelDensity, int page);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /** @reimp */
    VCWidget *createCopy(VCWidget *parent);

protected:
    /** @reimp */
    bool copyFrom(const VCWidget* widget);

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
    enum SliderMode { Level, Adjust, Submaster, GrandMaster };
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
    void setValue(int value, bool setDMX = true, bool updateFeedback = true);

    /** Set/Get the lower limit for the slider values */
    void setRangeLowLimit(qreal value);
    qreal rangeLowLimit() const;

    /** Set/Get the higher limit for the slider values */
    void setRangeHighLimit(qreal value);
    qreal rangeHighLimit() const;

protected:

    qreal sliderValueToAttributeValue(int value);
    qreal attributeValueToSliderValue(qreal value);

signals:
    void valueChanged(int value);
    void rangeLowLimitChanged();
    void rangeHighLimitChanged();

protected:
    int m_value;
    qreal m_rangeLowLimit;
    qreal m_rangeHighLimit;

    /*********************************************************************
     * Level mode
     *********************************************************************/
public:
    /** Get/Set the channels monitor status when in Level mode */
    void setMonitorEnabled(bool enable);
    bool monitorEnabled() const;

    /** Get the current monitor value when in Level mode */
    int monitorValue() const;

    /** Get/Set if the slider is overriding a monitoring level */
    bool isOverriding() const;
    void setIsOverriding(bool enable);

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

    /** Clear the list of level channels */
    void clearLevelChannels();

    /** Get the list of channels that this slider controls */
    QList <SceneValue> levelChannels() const;

    /** Returns the data model to display a tree of FixtureGroups/Fixtures */
    QVariant groupsTreeModel();

    QVariant channelsList();

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString searchFilter() const;
    void setSearchFilter(QString searchFilter);

private:
    void removeActiveFaders();

protected slots:
    void slotTreeDataChanged(TreeModelItem *item, int role, const QVariant &value);

signals:
    void monitorEnabledChanged();
    void monitorValueChanged();
    void isOverridingChanged();
    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();
    /** Notify the listeners that the search filter has changed */
    void searchFilterChanged();

protected:
    QList <SceneValue> m_levelChannels;

    QMutex m_levelValueMutex;
    bool m_levelValueChanged;

    bool m_monitorEnabled;
    uchar m_monitorValue;
    bool m_isOverriding;

    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** A string to filter the displayed tree items */
    QString m_searchFilter;

    /*********************************************************************
     * Click & Go
     *********************************************************************/
public:
    enum ClickAndGoType
    {
        CnGNone,
        CnGColors,
        CnGPreset
    };
    Q_ENUM(ClickAndGoType)

    /** Get/Set the current Click & Go type */
    ClickAndGoType clickAndGoType() const;
    void setClickAndGoType(ClickAndGoType clickAndGoType);

    /** Returns a human readable string of a Click And Go type */
    static QString clickAndGoTypeToString(ClickAndGoType type);

    /** Returns a Click And Go type from the given string */
    static ClickAndGoType stringToClickAndGoType(QString str);

    QColor cngPrimaryColor() const;
    QColor cngSecondaryColor() const;
    QVariantList clickAndGoPresetsList();
    QString cngPresetResource() const;

    Q_INVOKABLE void setClickAndGoColors(QColor rgb, QColor wauv);
    Q_INVOKABLE void setClickAndGoPresetValue(int value);

protected:
    void updateClickAndGoResource();

signals:
    void clickAndGoTypeChanged(ClickAndGoType clickAndGoType);
    void cngPrimaryColorChanged(QColor value);
    void cngSecondaryColorChanged(QColor value);
    void clickAndGoPresetsListChanged();
    void cngPresetResourceChanged();

protected:
    ClickAndGoType m_clickAndGoType;

    /** RGB and WAUV colors when in CnGColors type */
    QColor m_cngPrimaryColor;
    QColor m_cngSecondaryColor;
    QString m_cngResource;

    /*********************************************************************
     * Adjust mode
     *********************************************************************/
public:
    /** Get/Set the ID of the Function that will be controlled by this Slider */
    quint32 controlledFunction() const;
    void setControlledFunction(quint32 fid);

    /** Get/Set the attribute index that will be controlled by this Slider */
    int controlledAttribute() const;
    void setControlledAttribute(int attributeIndex);

    void adjustFunctionAttribute(Function *f, qreal value);

    /** Get/Set the status of the flash button enablement */
    bool adjustFlashEnabled() const;
    void setAdjustFlashEnabled(bool enable);

    Q_INVOKABLE void flashFunction(bool on);

    /** Get the list of the available attributes for the Function to control */
    QStringList availableAttributes() const;

    /** Return the min/max values for the attribute to control */
    qreal attributeMinValue() const;
    qreal attributeMaxValue() const;

    /** @reimp */
    void adjustIntensity(qreal val);

private:
    FunctionParent functionParent() const;

signals:
    void controlledFunctionChanged(quint32 fid);
    void controlledAttributeChanged(int attr);
    void adjustFlashEnabledChanged(bool enable);
    void availableAttributesChanged();
    void attributeMinValueChanged();
    void attributeMaxValueChanged();

protected slots:
    void slotControlledFunctionAttributeChanged(int attrIndex, qreal fraction);
    void slotControlledFunctionStopped(quint32 fid);

protected:
    quint32 m_controlledFunctionId;
    int m_adjustChangeCounter;
    int m_controlledAttributeIndex;
    int m_controlledAttributeId;
    qreal m_attributeMinValue;
    qreal m_attributeMaxValue;

    bool m_adjustFlashEnabled;
    qreal m_adjustFlashPreviousValue;

    /*********************************************************************
     * Submaster
     *********************************************************************/
signals:
    void submasterValueChanged(qreal value);

    /*********************************************************************
     * Grand Master mode
     *********************************************************************/
public:

    GrandMaster::ValueMode grandMasterValueMode() const;
    void setGrandMasterValueMode(GrandMaster::ValueMode mode);

    GrandMaster::ChannelMode grandMasterChannelMode() const;
    void setGrandMasterChannelMode(GrandMaster::ChannelMode mode);

signals:
    void grandMasterValueModeChanged(GrandMaster::ValueMode mode);
    void grandMasterChannelModeChanged(GrandMaster::ChannelMode mode);

    /*********************************************************************
     * DMXSource
     *********************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, QList<Universe*> universes);

protected:
    /** writeDMX for Level mode */
    void writeDMXLevel(MasterTimer* timer, QList<Universe*> universes);

    /** writeDMX for Adjust mode */
    void writeDMXAdjust(MasterTimer* timer, QList<Universe*> universes);

private:
    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** @reimp */
    void updateFeedback();

public slots:
    /** @reimp */
    void slotInputValueChanged(quint8 id, uchar value);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);
    bool loadXMLLevel(QXmlStreamReader &level_root);
    bool loadXMLAdjust(QXmlStreamReader &adj_root);
    bool loadXMLLegacyPlayback(QXmlStreamReader &pb_root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
