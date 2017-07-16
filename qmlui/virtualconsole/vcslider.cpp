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
#include "qlcmacros.h"
#include "vcslider.h"
#include "function.h"
#include "doc.h"
#include "app.h"

#define INPUT_SLIDER_CONTROL_ID     0
#define INPUT_SLIDER_RESET_ID       1

VCSlider::VCSlider(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_channelsTree(NULL)
    , m_widgetMode(WSlider)
    , m_valueDisplayStyle(DMXValue)
    , m_invertedAppearance(false)
    , m_sliderMode(Adjust)
    , m_value(0)
    , m_levelLowLimit(0)
    , m_levelHighLimit(UCHAR_MAX)
    , m_levelValueChanged(false)
    , m_monitorEnabled(false)
    , m_monitorValue(0)
    , m_isOverriding(false)
    , m_fixtureTree(NULL)
    , m_searchFilter(QString())
    , m_controlledFunction(Function::invalidId())
    , m_controlledAttribute(Function::Intensity)
{
    setType(VCWidget::SliderWidget);
    setBackgroundColor(QColor("#444"));

    registerExternalControl(INPUT_SLIDER_CONTROL_ID, tr("Slider Control"), false);
    registerExternalControl(INPUT_SLIDER_RESET_ID, tr("Reset Control"), false);
}

VCSlider::~VCSlider()
{
    /* When application exits these are already NULL and unregistration
       is no longer necessary. But a normal deletion of a VCSlider in
       design mode must unregister the slider. */
    m_doc->masterTimer()->unregisterDMXSource(this);
}

void VCSlider::setID(quint32 id)
{
    VCWidget::setID(id);

    if (caption().isEmpty())
        setCaption(defaultCaption());
}

QString VCSlider::defaultCaption()
{
    if (widgetStyle() == WSlider)
        return tr("Slider %1").arg(id() + 1);
    else
        return tr("Knob %1").arg(id() + 1);
}

void VCSlider::render(QQuickView *view, QQuickItem *parent)
{
    if (view == NULL || parent == NULL)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCSliderItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(component->create());

    item->setParentItem(parent);
    item->setProperty("sliderObj", QVariant::fromValue(this));
}

QString VCSlider::propertiesResource() const
{
    return QString("qrc:/VCSliderProperties.qml");
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
        case Adjust: return QString("Attribute");
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

void VCSlider::setSliderMode(SliderMode mode, bool init)
{
    Q_ASSERT(mode >= Level && mode <= GrandMaster);

    if (init == false && m_sliderMode == mode)
        return;

    m_sliderMode = mode;

    emit sliderModeChanged(mode);

    setLevelLowLimit(0);
    setLevelHighLimit(UCHAR_MAX);

    switch(mode)
    {
        case Level:
        case Adjust:
            setValue(0);
            m_doc->masterTimer()->registerDMXSource(this);
        break;
        case Submaster:
            setValue(UCHAR_MAX);
            m_doc->masterTimer()->unregisterDMXSource(this);
        break;
        case GrandMaster:
            setValueDisplayStyle(PercentageValue);
            setValue(UCHAR_MAX);
            m_doc->masterTimer()->unregisterDMXSource(this);
        break;
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

void VCSlider::setValue(int value, bool setDMX, bool updateFeedback)
{
    if (m_value == value)
        return;

    m_value = value;

    switch(sliderMode())
    {
        case Level:
            if (m_monitorEnabled == true && m_isOverriding == false && setDMX)
            {
                m_priority = DMXSource::Override;
                m_doc->masterTimer()->requestNewPriority(this);
                m_isOverriding = true;
                emit isOverridingChanged();
            }
        break;
        case Submaster:
        break;
        case GrandMaster:
            m_doc->inputOutputMap()->setGrandMasterValue(value);
        break;
        case Adjust:
        break;
    }

    emit valueChanged(value);

    if (setDMX)
        m_levelValueChanged = true;

    Q_UNUSED(updateFeedback)
    /* TODO
    if (updateFeedback)
    {
        int fbv = 0;
        if (invertedAppearance() == true)
            fbv = levelHighLimit() - m_value;
        else
            fbv = m_value;
        fbv = (int)SCALE(float(fbv), float(levelLowLimit()),
                         float(levelHighLimit()), float(0), float(UCHAR_MAX));
        sendFeedback(fbv);
    }
    */
}

/*********************************************************************
 * Level mode
 *********************************************************************/
void VCSlider::setLevelLowLimit(uchar value)
{
    if (value == m_levelLowLimit)
        return;

    m_levelLowLimit = value;
    emit levelLowLimitChanged();
}

uchar VCSlider::levelLowLimit() const
{
    return m_levelLowLimit;
}

void VCSlider::setLevelHighLimit(uchar value)
{
    if (value == m_levelLowLimit)
        return;

    m_levelHighLimit = value;
    emit levelHighLimitChanged();
}

uchar VCSlider::levelHighLimit() const
{
    return m_levelHighLimit;
}

void VCSlider::setMonitorEnabled(bool enable)
{
    if (enable == m_monitorEnabled)
        return;

    m_monitorEnabled = enable;

    m_priority = DMXSource::Override;
    m_doc->masterTimer()->requestNewPriority(this);

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
    if (enable == m_isOverriding)
        return;

    if (enable == false && m_monitorEnabled)
        setValue(m_monitorValue, false, false);

    m_isOverriding = enable;
    emit isOverridingChanged();
}

void VCSlider::addLevelChannel(quint32 fixture, quint32 channel)
{
    SceneValue lch(fixture, channel);

    if (m_levelChannels.contains(lch) == false)
    {
        m_levelChannels.append(lch);
        qSort(m_levelChannels.begin(), m_levelChannels.end());
    }
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

QList<SceneValue> VCSlider::levelChannels()
{
    return m_levelChannels;
}

QVariant VCSlider::groupsTreeModel()
{
    qDebug() << "Requesting tree model from slider" << m_levelChannels.count();
    if (m_fixtureTree == NULL)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);
        m_fixtureTree->setCheckable(true);

        FixtureManager::updateFixtureTree(m_doc, m_fixtureTree, m_searchFilter, m_levelChannels);

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
        FixtureManager::updateFixtureTree(m_doc, m_fixtureTree, m_searchFilter, m_levelChannels);
        emit groupsTreeModelChanged();
    }

    emit searchFilterChanged();
}

void VCSlider::slotTreeDataChanged(TreeModelItem *item, int role, const QVariant &value)
{
    qDebug() << "Slider tree data changed" << value.toInt();
    qDebug() << "Item data:" << item->data();

    if (role == TreeModel::IsCheckedRole)
    {
        QVariantList itemData = item->data();
        // itemData must be "classRef" << "type" << "id" << "subid" << "chIdx";
        if (itemData.count() != 5)
            return;

        //QString type = itemData.at(1).toString();
        quint32 fixtureID = itemData.at(2).toUInt();
        quint32 chIndex = itemData.at(4).toUInt();

        if (value.toInt() == 0)
            removeLevelChannel(fixtureID, chIndex);
        else
            addLevelChannel(fixtureID, chIndex);
    }
}

/*********************************************************************
 * Attribute mode
 *********************************************************************/

quint32 VCSlider::controlledFunction() const
{
    return m_controlledFunction;
}

void VCSlider::setControlledFunction(quint32 fid)
{
    bool running = false;

    if (m_controlledFunction == fid)
        return;

    Function* current = m_doc->function(m_controlledFunction);
    if (current != NULL)
    {
        /* Get rid of old function connections */
/*
        disconnect(current, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        disconnect(current, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        disconnect(current, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));
*/
        if(current->isRunning())
        {
            running = true;
            current->stop(functionParent());
        }
    }

    Function* function = m_doc->function(fid);
    if (function != NULL)
    {
        /* Connect to the new function */
/*
        connect(function, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        connect(function, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        connect(function, SIGNAL(flashing(quint32,bool)),
                this, SLOT(slotFunctionFlashing(quint32,bool)));
*/
        m_controlledFunction = fid;
        m_controlledAttribute = Function::Intensity;

        if ((isEditing() && caption().isEmpty()) || caption() == defaultCaption())
            setCaption(function->name());

        if(running)
            function->start(m_doc->masterTimer(), functionParent());

        emit controlledFunctionChanged(fid);
        emit availableAttributesChanged();
    }
    else
    {
        /* No function attachment */
        m_controlledFunction = Function::invalidId();
        emit controlledFunctionChanged(-1);
    }
    setDocModified();
}

int VCSlider::controlledAttribute() const
{
    return m_controlledAttribute;
}

void VCSlider::setControlledAttribute(int attr)
{
    if (m_controlledAttribute == attr)
        return;

    Function* function = m_doc->function(m_controlledFunction);
    if (function == NULL || attr >= function->attributes().count())
        return;

    m_controlledAttribute = attr;
    emit controlledAttributeChanged(attr);

    int newValue = int(floor((qreal(m_levelHighLimit) * function->getAttributeValue(attr)) + 0.5));
    setValue(newValue, false, true);
}

QStringList VCSlider::availableAttributes() const
{
    QStringList list;

    Function* function = m_doc->function(m_controlledFunction);
    if (function == NULL)
        return list;

    for (Attribute attr : function->attributes())
        list << attr.name;

    return list;
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
/*
    int r = 0, g = 0, b = 0, c = 0, m = 0, y = 0;

    if (m_cngType == ClickAndGoWidget::RGB)
    {
        float f = 0;
        if (m_slider)
            f = SCALE(float(m_levelValue), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(200));

        if ((uchar)f != 0)
        {
            QColor modColor = m_cngRGBvalue.lighter((uchar)f);
            r = modColor.red();
            g = modColor.green();
            b = modColor.blue();
        }
    }
    else if (m_cngType == ClickAndGoWidget::CMY)
    {
        float f = 0;
        if (m_slider)
            f = SCALE(float(m_levelValue), float(m_slider->minimum()),
                      float(m_slider->maximum()), float(0), float(200));
        if ((uchar)f != 0)
        {
            QColor modColor = m_cngRGBvalue.lighter((uchar)f);
            c = modColor.cyan();
            m = modColor.magenta();
            y = modColor.yellow();
        }
    }
*/
    if (m_monitorEnabled == true && m_levelValueChanged == false)
    {
        bool mixedDMXlevels = false;
        int monitorSliderValue = -1;

        for (SceneValue scv : m_levelChannels)
        {
            Fixture* fxi = m_doc->fixture(scv.fxi);
            if (fxi != NULL)
            {
                const QLCChannel* qlcch = fxi->channel(scv.channel);
                if (qlcch == NULL)
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

    for (SceneValue scv : m_levelChannels)
    {
        Fixture* fxi = m_doc->fixture(scv.fxi);
        if (fxi != NULL)
        {
            const QLCChannel* qlcch = fxi->channel(scv.channel);
            if (qlcch == NULL)
                continue;

            quint32 dmx_ch = fxi->address() + scv.channel;
            int uni = fxi->universe();

            // Dirty channel group check: is the channel HTP or LTP ?
            QLCChannel::Group group = qlcch->group();
            if (fxi->forcedLTPChannels().contains(scv.channel))
                group = QLCChannel::Effect;
            if (fxi->forcedHTPChannels().contains(scv.channel))
                group = QLCChannel::Intensity;

            if (group != QLCChannel::Intensity &&
                m_levelValueChanged == false)
            {
                /* Value has not changed and this is not an intensity channel.
                   LTP in effect. */
                continue;
            }
/*
            if (qlcch->group() == QLCChannel::Intensity)
            {
                if (m_cngType == ClickAndGoWidget::RGB)
                {
                    if (qlcch->colour() == QLCChannel::Red)
                        modLevel = (uchar)r;
                    else if (qlcch->colour() == QLCChannel::Green)
                        modLevel = (uchar)g;
                    else if (qlcch->colour() == QLCChannel::Blue)
                        modLevel = (uchar)b;
                }
                else if (m_cngType == ClickAndGoWidget::CMY)
                {
                    if (qlcch->colour() == QLCChannel::Cyan)
                        modLevel = (uchar)c;
                    else if (qlcch->colour() == QLCChannel::Magenta)
                        modLevel = (uchar)m;
                    else if (qlcch->colour() == QLCChannel::Yellow)
                        modLevel = (uchar)y;
                }
            }
*/
            if (uni < universes.count())
                universes[uni]->write(dmx_ch, modLevel * intensity(), m_isOverriding ? true : false);
        }
    }
    m_levelValueChanged = false;
}

void VCSlider::writeDMXAdjust(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(ua);

    QMutexLocker locker(&m_levelValueMutex);

    //if (m_playbackChangeCounter == 0)
    //    return;

    Function* function = m_doc->function(m_controlledFunction);
    if (function == NULL)
        return;

    uchar value = m_value;
    qreal pIntensity = qreal(value) / qreal(UCHAR_MAX);

    if (m_controlledAttribute == Function::Intensity)
    {
        if (value == 0)
        {
            // Make sure we ignore the fade out time
            function->adjustAttribute(0, Function::Intensity);
            if (function->stopped() == false)
                function->stop(functionParent());
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
            //emit functionStarting(m_playbackFunction, pIntensity);  // TODO
            function->adjustAttribute(pIntensity * intensity(), Function::Intensity);
        }
    }
    else
    {
        function->adjustAttribute(pIntensity * intensity(), m_controlledAttribute);
    }
    //m_playbackChangeCounter--;
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCSlider::slotInputValueChanged(quint8 id, uchar value)
{
    if (id != INPUT_SLIDER_CONTROL_ID)
        return;

    setValue(value, true, false);
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
            setSliderMode(stringToSliderMode(root.readElementText()), true);

            str = mAttrs.value(KXMLQLCVCSliderValueDisplayStyle).toString();
            setValueDisplayStyle(stringToValueDisplayStyle(str));
/*
            if (mAttrs.hasAttribute(KXMLQLCVCSliderClickAndGoType))
            {
                str = mAttrs.value(KXMLQLCVCSliderClickAndGoType).toString();
                setClickAndGoType(ClickAndGoWidget::stringToClickAndGoType(str));
            }
*/
            if (mAttrs.hasAttribute(KXMLQLCVCSliderLevelMonitor))
            {
                if (mAttrs.value(KXMLQLCVCSliderLevelMonitor).toString() == "false")
                    setMonitorEnabled(false);
                else
                    setMonitorEnabled(true);
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

    return true;
}


bool VCSlider::loadXMLLevel(QXmlStreamReader &level_root)
{
    QString str;

    if (level_root.name() != KXMLQLCVCSliderLevel)
    {
        qWarning() << Q_FUNC_INFO << "Slider level node not found";
        return false;
    }

    QXmlStreamAttributes attrs = level_root.attributes();

    /* Level low limit */
    str = attrs.value(KXMLQLCVCSliderLevelLowLimit).toString();
    setLevelLowLimit(str.toInt());

    /* Level high limit */
    str = attrs.value(KXMLQLCVCSliderLevelHighLimit).toString();
    setLevelHighLimit(str.toInt());

    /* Level value */
    str = attrs.value(KXMLQLCVCSliderLevelValue).toString();
    setValue(str.toInt());

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
                str = level_root.attributes().value(KXMLQLCVCSliderChannelFixture).toString();
                addLevelChannel(
                    static_cast<quint32>(str.toInt()),
                    static_cast<quint32> (level_root.readElementText().toInt()));
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Unknown slider level tag:" << level_root.name().toString();
                level_root.skipCurrentElement();
            }
        } while (level_root.readNextStartElement());
    }

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
        if (pb_root.name() == KXMLQLCVCSliderPlaybackFunction)
        {
            /* Function */
            setControlledFunction(pb_root.readElementText().toUInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider playback tag:" << pb_root.name().toString();
            pb_root.skipCurrentElement();
        }
    }

    return true;
}
