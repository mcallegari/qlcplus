/*
  Q Light Controller Plus
  vcslider.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QQmlEngine>
#include <qmath.h>

#include "treemodelitem.h"
#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
#include "fixtureutils.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "qlcmacros.h"
#include "vcslider.h"
#include "function.h"
#include "tardis.h"
#include "doc.h"
#include "app.h"

#define INPUT_SLIDER_CONTROL_ID     0
#define INPUT_SLIDER_RESET_ID       1
#define INPUT_SLIDER_FLASH_ID       2

VCSlider::VCSlider(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_channelsTree(nullptr)
    , m_widgetMode(WSlider)
    , m_valueDisplayStyle(DMXValue)
    , m_invertedAppearance(false)
    , m_sliderMode(Adjust)
    , m_value(0)
    , m_rangeLowLimit(0)
    , m_rangeHighLimit(UCHAR_MAX)
    , m_levelValueChanged(false)
    , m_monitorEnabled(false)
    , m_monitorValue(0)
    , m_isOverriding(false)
    , m_fixtureTree(nullptr)
    , m_searchFilter(QString())
    , m_clickAndGoType(CnGNone)
    , m_cngPrimaryColor(QColor())
    , m_cngSecondaryColor(QColor())
    , m_controlledFunctionId(Function::invalidId())
    , m_adjustChangeCounter(0)
    , m_controlledAttributeIndex(Function::invalidAttributeId())
    , m_controlledAttributeId(Function::invalidAttributeId())
    , m_attributeMinValue(0)
    , m_attributeMaxValue(UCHAR_MAX)
    , m_adjustFlashEnabled(false)
{
    setType(VCWidget::SliderWidget);
    setSliderMode(Adjust);

    registerExternalControl(INPUT_SLIDER_CONTROL_ID, tr("Slider Control"), false);
    registerExternalControl(INPUT_SLIDER_RESET_ID, tr("Reset Control"), false);
    registerExternalControl(INPUT_SLIDER_FLASH_ID, tr("Flash Control"), true);
}

VCSlider::~VCSlider()
{
    /* When application exits these are already NULL and unregistration
       is no longer necessary. But a normal deletion of a VCSlider in
       design mode must unregister the slider. */
    m_doc->masterTimer()->unregisterDMXSource(this);

    // request to delete all the active faders
    removeActiveFaders();

    if (m_item)
        delete m_item;
}

QString VCSlider::defaultCaption()
{
    if (widgetStyle() == WSlider)
        return tr("Slider %1").arg(id() + 1);
    else
        return tr("Knob %1").arg(id() + 1);
}

void VCSlider::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    setDefaultFontSize(pixelDensity * 3.5);
    setBackgroundColor(QColor("#444"));
}

void VCSlider::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCSliderItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("sliderObj", QVariant::fromValue(this));
}

QString VCSlider::propertiesResource() const
{
    return QString("qrc:/VCSliderProperties.qml");
}

VCWidget* VCSlider::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != nullptr);

    VCSlider* slider = new VCSlider(m_doc, parent);
    if (slider->copyFrom(this) == false)
    {
        delete slider;
        slider = nullptr;
    }

    return slider;
}

bool VCSlider::copyFrom(const VCWidget *widget)
{
    const VCSlider *slider = qobject_cast<const VCSlider*> (widget);
    if (slider == nullptr)
        return false;

    /* Copy widget style */
    setWidgetStyle(slider->widgetStyle());

    /* Copy level stuff */
    setRangeLowLimit(slider->rangeLowLimit());
    setRangeHighLimit(slider->rangeHighLimit());
    for (SceneValue scv : slider->levelChannels())
        addLevelChannel(scv.fxi, scv.channel);

    /* Copy playback stuff */
    setControlledFunction(slider->controlledFunction());

    /* Copy slider appearance */
    setValueDisplayStyle(slider->valueDisplayStyle());
    setInvertedAppearance(slider->invertedAppearance());

    /* Copy Click & Go feature */
    setClickAndGoType(slider->clickAndGoType());

    /* Copy mode & current value */
    setSliderMode(slider->sliderMode());
    setValue(slider->value(), false, false);

    /* Copy monitor mode */
    setMonitorEnabled(slider->monitorEnabled());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

QVariant VCSlider::channelsList()
{
    return QVariant::fromValue(m_channelsTree);
}

/*****************************************************************************
 * Slider Mode
 *****************************************************************************/

QString VCSlider::sliderModeToString(SliderMode mode)
{
    switch (mode)
    {
        case Level: return QString("Level");
        case Submaster: return QString("Submaster");
        case GrandMaster: return QString("GrandMaster");
        case Adjust: return QString("Adjust");
        default: return QString("Unknown");
    }
}

VCSlider::SliderMode VCSlider::stringToSliderMode(const QString& mode)
{
    if (mode == QString("Level"))
        return Level;
    else if (mode == QString("Submaster"))
        return Submaster;
    else if (mode == QString("GrandMaster"))
        return GrandMaster;
    else
        return Adjust;
}

VCSlider::SliderMode VCSlider::sliderMode() const
{
    return m_sliderMode;
}

void VCSlider::setSliderMode(SliderMode mode)
{
    Q_ASSERT(mode >= Level && mode <= GrandMaster);

    if (mode != m_sliderMode)
        Tardis::instance()->enqueueAction(Tardis::VCSliderSetMode, id(), m_sliderMode, mode);

    m_sliderMode = mode;

    emit sliderModeChanged(mode);

    setRangeLowLimit(0);
    setRangeHighLimit(UCHAR_MAX);
    setControlledAttribute(Function::Intensity);
    m_controlledAttributeId = Function::invalidAttributeId();

    switch (mode)
    {
        case Level:
        case Adjust:
            setValue(0);
            m_doc->masterTimer()->registerDMXSource(this);
        break;
        case Submaster:
            setValue(UCHAR_MAX);
        break;
        case GrandMaster:
            setValueDisplayStyle(PercentageValue);
            setValue(UCHAR_MAX);
        break;
    }

    if (mode == Submaster || mode == GrandMaster)
    {
        m_doc->masterTimer()->unregisterDMXSource(this);

        // request to delete all the active faders
        removeActiveFaders();
    }
}

/*********************************************************************
 * Widget style
 *********************************************************************/

QString VCSlider::widgetStyleToString(VCSlider::SliderWidgetStyle style)
{
    if (style == VCSlider::WSlider)
        return QString("Slider");
    else if (style == VCSlider::WKnob)
        return QString("Knob");

    return QString();
}

VCSlider::SliderWidgetStyle VCSlider::stringToWidgetStyle(QString style)
{
    if (style == "Slider")
        return VCSlider::WSlider;
    else if (style == "Knob")
        return VCSlider::WKnob;

    return VCSlider::WSlider;
}

VCSlider::SliderWidgetStyle VCSlider::widgetStyle() const
{
    return m_widgetMode;
}

void VCSlider::setWidgetStyle(SliderWidgetStyle mode)
{
    if (mode == m_widgetMode)
        return;

    m_widgetMode = mode;
    emit widgetStyleChanged(mode);
}

/*****************************************************************************
 * Display style
 *****************************************************************************/

QString VCSlider::valueDisplayStyleToString(VCSlider::ValueDisplayStyle style)
{
    switch (style)
    {
        case DMXValue: return KXMLQLCVCSliderValueDisplayStyleExact;
        case PercentageValue: return KXMLQLCVCSliderValueDisplayStylePercentage;
        default: return QString("Unknown");
    };
}

VCSlider::ValueDisplayStyle VCSlider::stringToValueDisplayStyle(QString style)
{
    if (style == KXMLQLCVCSliderValueDisplayStyleExact)
        return DMXValue;
    else if (style == KXMLQLCVCSliderValueDisplayStylePercentage)
        return PercentageValue;
    else
        return DMXValue;
}

VCSlider::ValueDisplayStyle VCSlider::valueDisplayStyle() const
{
    return m_valueDisplayStyle;
}

void VCSlider::setValueDisplayStyle(VCSlider::ValueDisplayStyle style)
{
    if (m_valueDisplayStyle == style)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCSliderSetDisplayStyle, id(), m_valueDisplayStyle, style);

    m_valueDisplayStyle = style;
    emit valueDisplayStyleChanged(style);
}

bool VCSlider::invertedAppearance() const
{
    return m_invertedAppearance;
}

void VCSlider::setInvertedAppearance(bool inverted)
{
    if (m_invertedAppearance == inverted)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCSliderSetInverted, id(), m_invertedAppearance, inverted);
    m_invertedAppearance = inverted;
    emit invertedAppearanceChanged(inverted);
}

/*********************************************************************
 * Slider value
 *********************************************************************/

int VCSlider::value() const
{
    return m_value;
}

qreal VCSlider::sliderValueToAttributeValue(int value)
{
    qreal fraction;

    if (m_controlledAttributeIndex == Function::Intensity)
        fraction = SCALE(qreal(value), m_rangeLowLimit, m_rangeHighLimit, 0, 1.0);
    else
        fraction = SCALE(qreal(value), m_rangeLowLimit, m_rangeHighLimit, m_attributeMinValue, m_attributeMaxValue);

    return fraction;
}

qreal VCSlider::attributeValueToSliderValue(qreal value)
{
    qreal fraction;

    if (m_controlledAttributeIndex == Function::Intensity)
        fraction = SCALE(value, 0.0, 1.0, m_rangeLowLimit, m_rangeHighLimit);
    else
        fraction = SCALE(value, m_attributeMinValue, m_attributeMaxValue, m_rangeLowLimit, m_rangeHighLimit);

    return fraction;
}

void VCSlider::setValue(int value, bool setDMX, bool updateFeedback)
{
    if (m_value == value)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCSliderSetValue, id(), m_value, value);

    m_value = value;

    switch(sliderMode())
    {
        case Level:
            if (m_monitorEnabled == true && m_isOverriding == false && setDMX)
            {
                m_isOverriding = true;
                emit isOverridingChanged();
            }

            if (clickAndGoType() == CnGPreset)
                updateClickAndGoResource();
        break;
        case Submaster:
            emit submasterValueChanged(SCALE(qreal(m_value), qreal(0),
                    qreal(UCHAR_MAX), qreal(0), qreal(1.0)) * intensity());
        break;
        case GrandMaster:
            m_doc->inputOutputMap()->setGrandMasterValue(value);
        break;
        case Adjust:
            m_adjustChangeCounter++;
        break;
    }

    emit valueChanged(value);

    if (setDMX)
        m_levelValueChanged = true;

    if (updateFeedback)
        this->updateFeedback();
}

void VCSlider::setRangeLowLimit(qreal value)
{
    if (value == m_rangeLowLimit)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCSliderSetLowLimit, id(), m_rangeLowLimit, value);
    m_rangeLowLimit = value;
    emit rangeLowLimitChanged();
}

qreal VCSlider::rangeLowLimit() const
{
    return m_rangeLowLimit;
}

void VCSlider::setRangeHighLimit(qreal value)
{
    if (value == m_rangeLowLimit)
        return;

    Tardis::instance()->enqueueAction(Tardis::VCSliderSetHighLimit, id(), m_rangeHighLimit, value);
    m_rangeHighLimit = value;
    emit rangeHighLimitChanged();
}

qreal VCSlider::rangeHighLimit() const
{
    return m_rangeHighLimit;
}

/*********************************************************************
 * Level mode
 *********************************************************************/

void VCSlider::setMonitorEnabled(bool enable)
{
    if (enable == m_monitorEnabled)
        return;

    m_monitorEnabled = enable;

    emit monitorEnabledChanged();
}

bool VCSlider::monitorEnabled() const
{
    return m_monitorEnabled;
}

int VCSlider::monitorValue() const
{
    return m_monitorValue;
}

bool VCSlider::isOverriding() const
{
    return m_isOverriding;
}

void VCSlider::setIsOverriding(bool enable)
{
    if (enable == false && m_monitorEnabled)
    {
        if (m_isOverriding)
        {
            removeActiveFaders();
            setValue(m_monitorValue, false, false);
        }
        else
        {
            // reset once more
            //setValue(0, true, true);
        }
    }

    m_isOverriding = enable;
    emit isOverridingChanged();
}

void VCSlider::addLevelChannel(quint32 fixture, quint32 channel)
{
    SceneValue lch(fixture, channel);

    if (m_levelChannels.contains(lch) == false)
        m_levelChannels.append(lch);
}

void VCSlider::removeLevelChannel(quint32 fixture, quint32 channel)
{
    SceneValue lch(fixture, channel);
    m_levelChannels.removeAll(lch);
}

void VCSlider::clearLevelChannels()
{
    m_levelChannels.clear();
}

QList<SceneValue> VCSlider::levelChannels() const
{
    return m_levelChannels;
}

QVariant VCSlider::groupsTreeModel()
{
    qDebug() << "Requesting tree model from slider" << m_levelChannels.count();
    if (m_fixtureTree == nullptr)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);

        FixtureManager::updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter,
                                         FixtureManager::ShowCheckBoxes | FixtureManager::ShowGroups | FixtureManager::ShowChannels,
                                         m_levelChannels);

        connect(m_fixtureTree, SIGNAL(roleChanged(TreeModelItem*,int,const QVariant&)),
                this, SLOT(slotTreeDataChanged(TreeModelItem*,int,const QVariant&)));
    }

    return QVariant::fromValue(m_fixtureTree);
}

QString VCSlider::searchFilter() const
{
    return m_searchFilter;
}

void VCSlider::setSearchFilter(QString searchFilter)
{
    if (m_searchFilter == searchFilter)
        return;

    int currLen = m_searchFilter.length();

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
    {
        FixtureManager::updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter,
                                         FixtureManager::ShowCheckBoxes | FixtureManager::ShowGroups | FixtureManager::ShowChannels,
                                         m_levelChannels);
        emit groupsTreeModelChanged();
    }

    emit searchFilterChanged();
}

void VCSlider::removeActiveFaders()
{
    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
    m_fadersMap.clear();
}

void VCSlider::slotTreeDataChanged(TreeModelItem *item, int role, const QVariant &value)
{
    qDebug() << "Slider tree data changed" << value.toInt();
    qDebug() << "Item data:" << item->data();

    if (role != TreeModel::IsCheckedRole)
        return;

    QVariantList itemData = item->data();
    // itemData must be "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
    if (itemData.count() != 6)
        return;

    //QString type = itemData.at(1).toString();
    quint32 itemID = itemData.at(2).toUInt();
    quint32 chIndex = itemData.at(4).toUInt();
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);

    if (value.toInt() == 0)
    {
        removeLevelChannel(fixtureID, chIndex);
    }
    else
    {
        addLevelChannel(fixtureID, chIndex);
        std::sort(m_levelChannels.begin(), m_levelChannels.end());
   }

    if (clickAndGoType() == CnGPreset)
    {
        updateClickAndGoResource();
        emit clickAndGoPresetsListChanged();
    }
}

/*********************************************************************
 * Click & Go
 *********************************************************************/

VCSlider::ClickAndGoType VCSlider::clickAndGoType() const
{
    return m_clickAndGoType;
}

void VCSlider::setClickAndGoType(VCSlider::ClickAndGoType clickAndGoType)
{
    if (m_clickAndGoType == clickAndGoType)
        return;

    m_clickAndGoType = clickAndGoType;
    emit clickAndGoTypeChanged(m_clickAndGoType);
}

QString VCSlider::clickAndGoTypeToString(VCSlider::ClickAndGoType type)
{
    switch (type)
    {
        default:
        case CnGNone: return "None"; break;
        case CnGColors: return "Colors"; break;
        case CnGPreset: return "Preset"; break;
    }
}

VCSlider::ClickAndGoType VCSlider::stringToClickAndGoType(QString str)
{
    if (str == "Colors" || str == "RGB" || str == "CMY")
        return CnGColors;
    else if (str == "Preset")
        return CnGPreset;

    return CnGNone;
}

QColor VCSlider::cngPrimaryColor() const
{
    return m_cngPrimaryColor;
}

QColor VCSlider::cngSecondaryColor() const
{
    return m_cngSecondaryColor;
}

QVariantList VCSlider::clickAndGoPresetsList()
{
    QVariantList prList;

    if (sliderMode() != Level || m_levelChannels.isEmpty())
        return prList;

    /* Find the first valid channel and return it to QML */
    for (SceneValue &scv : m_levelChannels)
    {
        Fixture *fixture = m_doc->fixture(scv.fxi);
        if (fixture == nullptr)
            continue;

        const QLCFixtureDef *def = fixture->fixtureDef();
        const QLCFixtureMode *mode = fixture->fixtureMode();

        if (def == nullptr || mode == nullptr)
            continue;

        const QLCChannel *ch = fixture->channel(scv.channel);

        if (ch == nullptr)
            continue;

        QVariantMap prMap;
        prMap.insert("name", QString("%1 - %2")
                            .arg(def->model())
                            .arg(ch->name()));
        prMap.insert("fixtureID", scv.fxi);
        prMap.insert("channelIdx", scv.channel);
        prList.append(prMap);

        break;
    }

    return prList;
}

QString VCSlider::cngPresetResource() const
{
    return m_cngResource;
}

void VCSlider::setClickAndGoColors(QColor rgb, QColor wauv)
{
    m_cngPrimaryColor = rgb;
    m_cngSecondaryColor = wauv;

    setValue(128, true, true);

    emit cngPrimaryColorChanged(rgb);
    emit cngSecondaryColorChanged(wauv);
}

void VCSlider::setClickAndGoPresetValue(int value)
{
    setValue(value, true, true);
}

void VCSlider::updateClickAndGoResource()
{
    /* Find the first valid channel and retrieve the capability
     * resource from the current slider value */
    for (SceneValue scv : m_levelChannels)
    {
        Fixture *fixture = m_doc->fixture(scv.fxi);
        if (fixture == nullptr)
            continue;

        const QLCChannel *ch = fixture->channel(scv.channel);
        if (ch == nullptr)
            return;

        for (QLCCapability *cap : ch->capabilities())
        {
            if (m_value < cap->min() || m_value > cap->max())
                continue;

            if (cap->presetType() == QLCCapability::Picture)
            {
                m_cngResource = cap->resource(0).toString();
            }
            else if (cap->presetType() == QLCCapability::SingleColor)
            {
                m_cngResource = QString();
                m_cngPrimaryColor = cap->resource(0).value<QColor>();
                m_cngSecondaryColor = QColor();
                emit cngPrimaryColorChanged(m_cngPrimaryColor);
                emit cngSecondaryColorChanged(QColor());
            }
            else if (cap->presetType() == QLCCapability::DoubleColor)
            {
                m_cngResource = QString();
                m_cngPrimaryColor = cap->resource(0).value<QColor>();
                m_cngSecondaryColor = cap->resource(1).value<QColor>();
                emit cngPrimaryColorChanged(m_cngPrimaryColor);
                emit cngSecondaryColorChanged(m_cngSecondaryColor);
            }
            else
            {
                m_cngResource = QString();
            }

            emit cngPresetResourceChanged();
            return;
        }
    }
}

/*********************************************************************
 * Attribute mode
 *********************************************************************/

quint32 VCSlider::controlledFunction() const
{
    return m_controlledFunctionId;
}

void VCSlider::setControlledFunction(quint32 fid)
{
    bool running = false;

    if (m_controlledFunctionId == fid)
        return;

    Function *current = m_doc->function(m_controlledFunctionId);
    if (current != nullptr)
    {
        /* Get rid of old function connections */
        disconnect(current, SIGNAL(stopped(quint32)),
                this, SLOT(slotControlledFunctionStopped(quint32)));
        disconnect(current, SIGNAL(attributeChanged(int,qreal)),
                this, SLOT(slotControlledFunctionAttributeChanged(int,qreal)));

        if (current->isRunning())
        {
            running = true;
            current->stop(functionParent());
        }
    }

    Function *function = m_doc->function(fid);
    if (function != nullptr)
    {
        /* Connect to the new function */
        connect(function, SIGNAL(stopped(quint32)),
                this, SLOT(slotControlledFunctionStopped(quint32)));
        connect(function, SIGNAL(attributeChanged(int,qreal)),
                this, SLOT(slotControlledFunctionAttributeChanged(int,qreal)));

        m_controlledFunctionId = fid;
        m_controlledAttributeIndex = Function::Intensity;

        if ((isEditing() && caption().isEmpty()) || caption() == defaultCaption())
            setCaption(function->name());

        if (running)
            function->start(m_doc->masterTimer(), functionParent());

        emit controlledFunctionChanged(fid);
        emit availableAttributesChanged();
    }
    else
    {
        /* No function attachment */
        m_controlledFunctionId = Function::invalidId();
        emit controlledFunctionChanged(-1);
    }

    Tardis::instance()->enqueueAction(Tardis::VCSliderSetFunctionID, id(),
                                      current ? current->id() : Function::invalidId(),
                                      function ? function->id() : Function::invalidId());
}

void VCSlider::adjustIntensity(qreal val)
{
    VCWidget::adjustIntensity(val);

    if (sliderMode() == Adjust)
    {
        Function *function = m_doc->function(m_controlledFunctionId);
        if (function == nullptr)
            return;

        qreal fraction = sliderValueToAttributeValue(m_value);
        adjustFunctionAttribute(function, fraction * intensity());
    }
    else if (sliderMode() == Level)
    {
        // force channel levels refresh
        m_levelValueChanged = true;
    }
}

void VCSlider::slotControlledFunctionAttributeChanged(int attrIndex, qreal fraction)
{
    if (attrIndex != m_controlledAttributeIndex || m_adjustChangeCounter)
        return;

    qreal newValue = qRound(attributeValueToSliderValue(fraction / intensity()));

    qDebug() << "Function attribute" << m_controlledAttributeIndex << "changed" << fraction << "->" << newValue;

    setValue(newValue, false, true);
}

void VCSlider::slotControlledFunctionStopped(quint32 fid)
{
    if (fid == controlledFunction())
    {
        if (m_controlledAttributeIndex == Function::Intensity)
            setValue(0, false, true);

        Function *function = m_doc->function(fid);
        function->releaseAttributeOverride(m_controlledAttributeId);
        m_controlledAttributeId = Function::invalidAttributeId();
    }
}

int VCSlider::controlledAttribute() const
{
    return m_controlledAttributeIndex;
}

void VCSlider::setControlledAttribute(int attributeIndex)
{
    if (m_controlledAttributeIndex == attributeIndex)
        return;

    Function *function = m_doc->function(m_controlledFunctionId);
    if (function == nullptr || attributeIndex >= function->attributes().count())
        return;

    function->releaseAttributeOverride(m_controlledAttributeId);
    m_controlledAttributeId = Function::invalidAttributeId();

    Tardis::instance()->enqueueAction(Tardis::VCSliderSetControlledAttribute, id(), m_controlledAttributeIndex, attributeIndex);

    m_controlledAttributeIndex = attributeIndex;
    qreal newValue = 0;

    // normalize intensity to 0-255 since Slider / Spin boxes step is an integer
    if (m_controlledAttributeIndex == Function::Intensity)
    {
        m_attributeMinValue = 0;
        m_attributeMaxValue = 255;
    }
    else
    {
        m_attributeMinValue = function->attributes().at(m_controlledAttributeIndex).m_min;
        m_attributeMaxValue = function->attributes().at(m_controlledAttributeIndex).m_max;
        newValue = function->getAttributeValue(m_controlledAttributeIndex);
    }

    setRangeLowLimit(m_attributeMinValue);
    setRangeHighLimit(m_attributeMaxValue);

    emit controlledAttributeChanged(attributeIndex);
    emit attributeMinValueChanged();
    emit attributeMaxValueChanged();

    setValue(newValue, false, true);
}

void VCSlider::adjustFunctionAttribute(Function *f, qreal value)
{
    if (f == nullptr)
        return;

    if (m_controlledAttributeId == Function::invalidAttributeId())
        m_controlledAttributeId = f->requestAttributeOverride(m_controlledAttributeIndex, value);
    else
        f->adjustAttribute(value, m_controlledAttributeId);
}

bool VCSlider::adjustFlashEnabled() const
{
    return m_adjustFlashEnabled;
}

void VCSlider::setAdjustFlashEnabled(bool enable)
{
    if (enable == m_adjustFlashEnabled)
        return;

    m_adjustFlashEnabled = enable;
    emit adjustFlashEnabledChanged(enable);
}

void VCSlider::flashFunction(bool on)
{
    Function *function = m_doc->function(m_controlledFunctionId);
    if (function == nullptr)
        return;

    if (on)
    {
        if (m_controlledAttributeId == Function::invalidAttributeId())
            m_adjustFlashPreviousValue = 0;
        else
            m_adjustFlashPreviousValue = function->getAttributeValue(m_controlledAttributeIndex);
    }

    adjustFunctionAttribute(function, on ? 1.0 : m_adjustFlashPreviousValue);
}

QStringList VCSlider::availableAttributes() const
{
    QStringList list;

    Function *function = m_doc->function(m_controlledFunctionId);
    if (function == nullptr)
        return list;

    for (Attribute &attr : function->attributes())
        list << attr.m_name;

    return list;
}

qreal VCSlider::attributeMinValue() const
{
    return m_attributeMinValue;
}

qreal VCSlider::attributeMaxValue() const
{
    return m_attributeMaxValue;
}

FunctionParent VCSlider::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*********************************************************************
 * Grand Master mode
 *********************************************************************/

GrandMaster::ValueMode VCSlider::grandMasterValueMode() const
{
    return m_doc->inputOutputMap()->grandMasterValueMode();
}

void VCSlider::setGrandMasterValueMode(GrandMaster::ValueMode mode)
{
    if (mode == m_doc->inputOutputMap()->grandMasterValueMode())
        return;

    m_doc->inputOutputMap()->setGrandMasterValueMode(mode);
    emit grandMasterValueModeChanged(mode);
}

GrandMaster::ChannelMode VCSlider::grandMasterChannelMode() const
{
    return m_doc->inputOutputMap()->grandMasterChannelMode();
}

void VCSlider::setGrandMasterChannelMode(GrandMaster::ChannelMode mode)
{
    if (mode == m_doc->inputOutputMap()->grandMasterChannelMode())
        return;

    m_doc->inputOutputMap()->setGrandMasterChannelMode(mode);
    emit grandMasterChannelModeChanged(mode);
}

/*********************************************************************
 * DMXSource
 *********************************************************************/

void VCSlider::writeDMX(MasterTimer* timer, QList<Universe*> universes)
{
    if (sliderMode() == Level)
        writeDMXLevel(timer, universes);
    else if (sliderMode() == Adjust)
        writeDMXAdjust(timer, universes);
}

void VCSlider::writeDMXLevel(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    QMutexLocker locker(&m_levelValueMutex);

    uchar modLevel = m_value;

    int r = 0, g = 0, b = 0, c = 0, m = 0, y = 0, w = 0, a = 0, uv = 0;

    if (clickAndGoType() == CnGColors)
    {
        float f = SCALE(float(modLevel), rangeLowLimit(), rangeHighLimit(), 0.0, 200.0);

        if ((uchar)f != 0)
        {
            QColor modColor = m_cngPrimaryColor.lighter((uchar)f);
            r = modColor.red();
            g = modColor.green();
            b = modColor.blue();
            c = modColor.cyan();
            m = modColor.magenta();
            y = modColor.yellow();

            modColor = m_cngSecondaryColor.lighter((uchar)f);
            w = modColor.red();
            a = modColor.green();
            uv = modColor.blue();
        }
    }

    if (m_monitorEnabled == true && m_levelValueChanged == false)
    {
        bool mixedDMXlevels = false;
        int monitorSliderValue = -1;

        for (SceneValue &scv : m_levelChannels)
        {
            Fixture* fxi = m_doc->fixture(scv.fxi);
            if (fxi != nullptr)
            {
                const QLCChannel* qlcch = fxi->channel(scv.channel);
                if (qlcch == nullptr)
                    continue;

                quint32 dmx_ch = fxi->address() + scv.channel;
                int uni = fxi->universe();
                if (uni < universes.count())
                {
                    uchar chValue = universes[uni]->preGMValue(dmx_ch);
                    if (monitorSliderValue == -1)
                    {
                        monitorSliderValue = chValue;
                        //qDebug() << caption() << "Monitor DMX value:" << monitorSliderValue << "level value:" << m_value;
                    }
                    else
                    {
                        if (chValue != (uchar)monitorSliderValue)
                        {
                            mixedDMXlevels = true;
                            // no need to proceed further as mixed values cannot
                            // be represented by one single slider
                            break;
                        }
                    }
                }
            }
        }

        // check if all the DMX channels controlled by this slider
        // have the same value. If so, move the widget slider or knob
        // to the detected position
        if (mixedDMXlevels == false &&
            monitorSliderValue != -1 &&
            monitorSliderValue != m_monitorValue)
        {
            //qDebug() << caption() << "Monitor DMX value:" << monitorSliderValue << "level value:" << m_value;

            m_monitorValue = monitorSliderValue;
            emit monitorValueChanged();

            if (m_isOverriding == false)
            {
                setValue(m_monitorValue, false, true);
                return;
            }
        }
    }

    if (m_levelValueChanged)
    {
        for (SceneValue &scv : m_levelChannels)
        {
            Fixture* fxi = m_doc->fixture(scv.fxi);
            if (fxi == nullptr)
                continue;

            quint32 universe = fxi->universe();

            QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>(nullptr));
            if (fader.isNull())
            {
                fader = universes[universe]->requestFader(m_monitorEnabled ? Universe::Override : Universe::Auto);
                m_fadersMap[universe] = fader;
            }

            FadeChannel *fc = fader->getChannelFader(m_doc, universes[universe], scv.fxi, scv.channel);
            if (fc->universe() == Universe::invalid())
            {
                fader->remove(fc);
                continue;
            }

            int chType = fc->flags();

            // on override, force channel to LTP
            if (m_isOverriding)
                fc->addFlag(FadeChannel::Override);

            // request to autoremove LTP channels when set
            if (! (chType & FadeChannel::Intensity))
                fc->addFlag(FadeChannel::AutoRemove);

            if (chType & FadeChannel::Intensity && clickAndGoType() == CnGColors)
            {
                const QLCChannel *qlcch = fxi->channel(scv.channel);

                if (qlcch != nullptr)
                {
                    switch (qlcch->colour())
                    {
                        case QLCChannel::Red: modLevel = (uchar)r; break;
                        case QLCChannel::Green: modLevel = (uchar)g; break;
                        case QLCChannel::Blue: modLevel = (uchar)b; break;
                        case QLCChannel::Cyan: modLevel = (uchar)c; break;
                        case QLCChannel::Magenta: modLevel = (uchar)m; break;
                        case QLCChannel::Yellow: modLevel = (uchar)y; break;
                        case QLCChannel::White: modLevel = (uchar)w; break;
                        case QLCChannel::Amber: modLevel = (uchar)a; break;
                        case QLCChannel::UV: modLevel = (uchar)uv; break;
                        default: break;
                    }
                }
            }

            fc->setStart(fc->current());
            fc->setTarget(qreal(modLevel) * intensity());
            fc->setReady(false);
            fc->setElapsed(0);
        }
    }
    m_levelValueChanged = false;
}

void VCSlider::writeDMXAdjust(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(ua);

    QMutexLocker locker(&m_levelValueMutex);

    if (m_adjustChangeCounter == 0)
        return;

    Function *function = m_doc->function(m_controlledFunctionId);
    if (function == nullptr)
        return;

    qreal fraction = sliderValueToAttributeValue(m_value);

    qDebug() << "Adjust Function attribute" << m_controlledAttributeIndex << "to" << fraction;

    if (m_controlledAttributeIndex == Function::Intensity)
    {
        if (m_value == 0)
        {
            if (function->stopped() == false)
            {
                function->stop(functionParent());
                m_controlledAttributeId = Function::invalidAttributeId();
                m_adjustChangeCounter--;
                return;
            }
        }
        else
        {
            if (function->stopped() == true)
            {
#if 0 // temporarily revert #699 until a better solution is found
                // Since this function is started by a fader, its fade in time
                // is decided by the fader movement.
                function->start(timer, functionParent(),
                                0, 0, Function::defaultSpeed(), Function::defaultSpeed());
#endif
                function->start(timer, functionParent());
                qDebug() << "Function started";
            }
            emit functionStarting(this, m_controlledFunctionId, fraction);
        }
    }

    adjustFunctionAttribute(function, fraction * intensity());

    m_adjustChangeCounter--;
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCSlider::updateFeedback()
{
    int fbv = invertedAppearance() ? rangeHighLimit() - m_value : m_value;
    fbv = int(SCALE(float(fbv), float(rangeLowLimit()),
                    float(rangeHighLimit()), float(0), float(UCHAR_MAX)));

    sendFeedback(fbv, INPUT_SLIDER_CONTROL_ID);
}

void VCSlider::slotInputValueChanged(quint8 id, uchar value)
{
    int scaledValue = SCALE(float(value), float(0), float(UCHAR_MAX),
            float(rangeLowLimit()),
            float(rangeHighLimit()));

    switch (id)
    {
        case INPUT_SLIDER_CONTROL_ID:
            setValue(scaledValue, true, false);
        break;
        case INPUT_SLIDER_RESET_ID:
            if (value)
                setIsOverriding(false);
        break;
        case INPUT_SLIDER_FLASH_ID:
            flashFunction(value ? true : false);
        break;
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCSlider::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCSlider)
    {
        qWarning() << Q_FUNC_INFO << "Slider node not found";
        return false;
    }

    QString str;
    bool enableMonitoring = false;

    /* Widget commons */
    loadXMLCommon(root);

    QXmlStreamAttributes attrs = root.attributes();

    /* Widget style */
    if (attrs.hasAttribute(KXMLQLCVCSliderWidgetStyle))
        setWidgetStyle(stringToWidgetStyle(attrs.value(KXMLQLCVCSliderWidgetStyle).toString()));

    if (attrs.value(KXMLQLCVCSliderInvertedAppearance).toString() == "false")
        setInvertedAppearance(false);
    else
        setInvertedAppearance(true);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            loadXMLInputSource(root, INPUT_SLIDER_CONTROL_ID);
        }
        else if (root.name() == KXMLQLCVCSliderMode)
        {
            QXmlStreamAttributes mAttrs = root.attributes();
            setSliderMode(stringToSliderMode(root.readElementText()));

            str = mAttrs.value(KXMLQLCVCSliderValueDisplayStyle).toString();
            setValueDisplayStyle(stringToValueDisplayStyle(str));

            if (mAttrs.hasAttribute(KXMLQLCVCSliderClickAndGoType))
            {
                str = mAttrs.value(KXMLQLCVCSliderClickAndGoType).toString();
                setClickAndGoType(stringToClickAndGoType(str));
            }

            if (mAttrs.hasAttribute(KXMLQLCVCSliderLevelMonitor))
            {
                if (mAttrs.value(KXMLQLCVCSliderLevelMonitor).toString() != "false")
                    enableMonitoring = true;
            }
        }
        else if (root.name() == KXMLQLCVCSliderOverrideReset)
        {
            loadXMLSources(root, INPUT_SLIDER_RESET_ID);
        }
        else if (root.name() == KXMLQLCVCSliderLevel)
        {
            loadXMLLevel(root);
        }
        else if (root.name() == KXMLQLCVCSliderAdjust)
        {
            loadXMLAdjust(root);
        }
        else if (root.name() == KXMLQLCVCSliderFunctionFlash)
        {
            setAdjustFlashEnabled(true);
            loadXMLSources(root, INPUT_SLIDER_FLASH_ID);
        }
        else if (root.name() == KXMLQLCVCSliderPlayback) // LEGACY
        {
            loadXMLLegacyPlayback(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    setMonitorEnabled(enableMonitoring);

    if (clickAndGoType() == CnGPreset)
    {
        updateClickAndGoResource();
        emit clickAndGoPresetsListChanged();
    }

    return true;
}


bool VCSlider::loadXMLLevel(QXmlStreamReader &level_root)
{
    int value;

    if (level_root.name() != KXMLQLCVCSliderLevel)
    {
        qWarning() << Q_FUNC_INFO << "Slider level node not found";
        return false;
    }

    QXmlStreamAttributes attrs = level_root.attributes();

    /* Level low limit */
    value = attrs.value(KXMLQLCVCSliderLevelLowLimit).toInt();
    setRangeLowLimit(value);

    /* Level high limit */
    value = attrs.value(KXMLQLCVCSliderLevelHighLimit).toInt();
    setRangeHighLimit(value);

    /* Level value */
    value = attrs.value(KXMLQLCVCSliderLevelValue).toInt();
    setValue(value);

    QXmlStreamReader::TokenType tType = level_root.readNext();

    if (tType == QXmlStreamReader::EndElement)
    {
        level_root.readNext();
        return true;
    }

    if (tType == QXmlStreamReader::Characters)
        tType = level_root.readNext();

    // check if there is a Channel tag defined
    if (tType == QXmlStreamReader::StartElement)
    {
        /* Children */
        do
        {
            if (level_root.name() == KXMLQLCVCSliderChannel)
            {
                /* Fixture & channel */
                value = level_root.attributes().value(KXMLQLCVCSliderChannelFixture).toInt();
                addLevelChannel(
                    static_cast<quint32>(value),
                    static_cast<quint32> (level_root.readElementText().toInt()));
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Unknown slider level tag:" << level_root.name().toString();
                level_root.skipCurrentElement();
            }
        } while (level_root.readNextStartElement());
    }

    if (m_levelChannels.count())
        std::sort(m_levelChannels.begin(), m_levelChannels.end());

    return true;
}

bool VCSlider::loadXMLAdjust(QXmlStreamReader &adj_root)
{
    if (adj_root.name() != KXMLQLCVCSliderAdjust)
    {
        qWarning() << Q_FUNC_INFO << "Slider Adjust node not found";
        return false;
    }

    QXmlStreamAttributes attrs = adj_root.attributes();

    int value;

    /* Controlled Function ID */
    value = attrs.value(KXMLQLCVCSliderControlledFunction).toInt();
    setControlledFunction(value);

    /* Level high limit */
    value = attrs.value(KXMLQLCVCSliderAdjustAttribute).toInt();
    setControlledAttribute(value);

    adj_root.skipCurrentElement();

    return true;
}

bool VCSlider::loadXMLLegacyPlayback(QXmlStreamReader &pb_root)
{
    if (pb_root.name() != KXMLQLCVCSliderPlayback)
    {
        qWarning() << Q_FUNC_INFO << "Slider playback node not found";
        return false;
    }

    /* Children */
    while (pb_root.readNextStartElement())
    {
        if (pb_root.name() == KXMLQLCVCSliderControlledFunction)
        {
            /* Function */
            setControlledFunction(pb_root.readElementText().toUInt());
            setControlledAttribute(Function::Intensity);
        }
        else if (pb_root.name() == KXMLQLCVCSliderFunctionFlash)
        {
            setAdjustFlashEnabled(true);
            loadXMLSources(pb_root, INPUT_SLIDER_FLASH_ID);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider playback tag:" << pb_root.name().toString();
            pb_root.skipCurrentElement();
        }
    }

    return true;
}

bool VCSlider::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC slider entry */
    doc->writeStartElement(KXMLQLCVCSlider);

    saveXMLCommon(doc);

    /* Widget style */
    doc->writeAttribute(KXMLQLCVCSliderWidgetStyle, widgetStyleToString(widgetStyle()));

    /* Inverted appearance */
    if (invertedAppearance() == true)
        doc->writeAttribute(KXMLQLCVCSliderInvertedAppearance, "true");
    else
        doc->writeAttribute(KXMLQLCVCSliderInvertedAppearance, "false");

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Main external control */
    saveXMLInputControl(doc, INPUT_SLIDER_CONTROL_ID);

    /* SliderMode */
    doc->writeStartElement(KXMLQLCVCSliderMode);

    /* Value display style */
    doc->writeAttribute(KXMLQLCVCSliderValueDisplayStyle, valueDisplayStyleToString(valueDisplayStyle()));

    /* Click And Go type */
    if (m_clickAndGoType != CnGNone)
        doc->writeAttribute(KXMLQLCVCSliderClickAndGoType, clickAndGoTypeToString(m_clickAndGoType));

    /* Monitor channels */
    if (sliderMode() == Level && monitorEnabled() == true)
        doc->writeAttribute(KXMLQLCVCSliderLevelMonitor, "true");

    doc->writeCharacters(sliderModeToString(m_sliderMode));

    /* End the <SliderMode> tag */
    doc->writeEndElement();

    /* Override reset external control */
    if (sliderMode() == Level && monitorEnabled() == true)
        saveXMLInputControl(doc, INPUT_SLIDER_RESET_ID, false, KXMLQLCVCSliderOverrideReset);

    /* Level */
    doc->writeStartElement(KXMLQLCVCSliderLevel);
    /* Level low limit */
    doc->writeAttribute(KXMLQLCVCSliderLevelLowLimit, QString::number(rangeLowLimit()));
    /* Level high limit */
    doc->writeAttribute(KXMLQLCVCSliderLevelHighLimit, QString::number(rangeHighLimit()));
    /* Level value */
    if (monitorEnabled())
        doc->writeAttribute(KXMLQLCVCSliderLevelValue, QString::number(0));
    else
        doc->writeAttribute(KXMLQLCVCSliderLevelValue, QString::number(value()));

    /* Level channels */
    for (SceneValue &scv : m_levelChannels)
    {
        doc->writeStartElement(KXMLQLCVCSliderChannel);
        doc->writeAttribute(KXMLQLCVCSliderChannelFixture, QString::number(scv.fxi));
        doc->writeCharacters(QString::number(scv.channel));
        doc->writeEndElement();
    }

    /* End the <Level> tag */
    doc->writeEndElement();

    if (controlledFunction() != Function::invalidId())
    {
        /* Start the <Adjust> tag */
        doc->writeStartElement(KXMLQLCVCSliderAdjust);
        /* Controlled attribute index */
        doc->writeAttribute(KXMLQLCVCSliderAdjustAttribute, QString::number(controlledAttribute()));
        /* Controlled function ID */
        doc->writeAttribute(KXMLQLCVCSliderControlledFunction, QString::number(controlledFunction()));
        /* End the <Adjust> tag */
        doc->writeEndElement();

        if (adjustFlashEnabled())
            saveXMLInputControl(doc, INPUT_SLIDER_FLASH_ID, false, KXMLQLCVCSliderFunctionFlash);
    }

    /* End the <Slider> tag */
    doc->writeEndElement();

    return true;
}
