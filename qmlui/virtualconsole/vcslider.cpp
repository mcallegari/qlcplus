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

#include "treemodelitem.h"
#include "qlcfixturemode.h"
#include "qlcmacros.h"
#include "vcslider.h"
#include "doc.h"

#define INPUT_SLIDER_CONTROL_ID     0

VCSlider::VCSlider(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_channelsTree(NULL)
    , m_widgetMode(WSlider)
    , m_valueDisplayStyle(DMXValue)
    , m_invertedAppearance(false)
    , m_sliderMode(Playback)
    , m_value(0)
    , m_levelLowLimit(0)
    , m_levelHighLimit(UCHAR_MAX)
    , m_levelValue(0)
    , m_levelValueChanged(false)
    , m_monitorEnabled(false)
    , m_monitorValue(0)
    , m_fixtureTree(NULL)
    , m_playbackFunction(Function::invalidId())
{
    setType(VCWidget::SliderWidget);
    setBackgroundColor(QColor("#444"));

    registerExternalControl(INPUT_SLIDER_CONTROL_ID, tr("Slider Control"), false);
}

VCSlider::~VCSlider()
{
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
        return tr("Slider %1").arg(id());
    else
        return tr("Knob %1").arg(id());
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
/*
    if (m_channelsTree == NULL)
    {
        m_channelsTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_channelsTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "uni" << "fxID" << "chIndex";
        m_channelsTree->setColumnNames(treeColumns);
    }
*/
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
        case Playback: return QString("Playback");
        case Submaster: return QString("Submaster");
        case GrandMaster: return QString("GrandMaster");
        case Attribute: return QString("Attribute");
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
    else if (mode == QString("Attribute"))
        return Attribute;
    else
        return Playback;
}

VCSlider::SliderMode VCSlider::sliderMode() const
{
    return m_sliderMode;
}

void VCSlider::setSliderMode(SliderMode mode)
{
    Q_ASSERT(mode >= Level && mode <= GrandMaster);

    if (m_sliderMode == mode)
        return;

    m_sliderMode = mode;
    emit sliderModeChanged(mode);
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

void VCSlider::setValue(int value)
{
    if (m_value == value)
        return;

    m_value = value;
    emit valueChanged(value);
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

void VCSlider::updateFixtureTree(Doc *doc, TreeModel *treeModel)
{
    if (doc == NULL || treeModel == NULL)
        return;

    treeModel->clear();

    QStringList uniNames = doc->inputOutputMap()->universeNames();

    // add Fixture Groups first
    for (FixtureGroup* grp : doc->fixtureGroups()) // C++11
    {
        foreach(quint32 fxID, grp->fixtureList())
        {
            Fixture *fixture = doc->fixture(fxID);
            if (fixture == NULL)
                continue;

            QLCFixtureMode *mode = fixture->fixtureMode();
            if (mode == NULL)
                continue;

            int chIdx = 0;
            QString chPath = QString("%1/%2").arg(grp->name()).arg(fixture->name());
            for (QLCChannel *channel : mode->channels()) // C++11
            {
                bool checked = false;
                QVariantList chParams;
                chParams.append(QVariant::fromValue(NULL)); // classRef
                chParams.append("FCG"); // type
                chParams.append(fixture->id()); // id
                chParams.append(grp->id()); // subid
                chParams.append(chIdx); // chIdx

                if (m_levelChannels.contains(SceneValue(fixture->id(), chIdx)))
                    checked = true;
                treeModel->addItem(channel->name(), chParams, chPath, checked ? TreeModel::Checked : 0);
                chIdx++;
            }

            // when all the channel 'leaves' have been added, set the parent node data
            QVariantList params;
            params.append(QVariant::fromValue(fixture)); // classRef
            params.append("FXG"); // type
            params.append(fixture->id()); // id
            params.append(grp->id()); // subid
            params.append(0); // chIdx
            treeModel->setPathData(chPath, params);
        }
    }

    // add the current universes as groups
    for (Fixture *fixture : doc->fixtures()) // C++11
    {
        if (fixture->universe() >= (quint32)uniNames.count())
            continue;

        QString chPath = QString("%1/%2").arg(uniNames.at(fixture->universe())).arg(fixture->name());
        QLCFixtureMode *mode = fixture->fixtureMode();
        if (mode == NULL)
            continue;

        int chIdx = 0;
        for (QLCChannel *channel : mode->channels()) // C++11
        {
            bool checked = false;
            QVariantList chParams;
            chParams.append(QVariant::fromValue(NULL)); // classRef
            chParams.append("FCU"); // type
            chParams.append(fixture->id()); // id
            chParams.append(fixture->universe()); // subid
            chParams.append(chIdx); // chIdx

            if (m_levelChannels.contains(SceneValue(fixture->id(), chIdx)))
                checked = true;
            treeModel->addItem(channel->name(), chParams, chPath, checked ? TreeModel::Checked : 0);
            chIdx++;
        }

        // when all the channel 'leaves' have been added, set the parent node data
        QVariantList params;
        params.append(QVariant::fromValue(fixture)); // classRef
        params.append("FXU"); // type
        params.append(fixture->id()); // id
        params.append(fixture->universe()); // subid
        params.append(0); // chIdx

        treeModel->setPathData(chPath, params);
    }
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
        updateFixtureTree(m_doc, m_fixtureTree);

        connect(m_fixtureTree, SIGNAL(roleChanged(TreeModelItem*,int,const QVariant&)),
                this, SLOT(slotTreeDataChanged(TreeModelItem*,int,const QVariant&)));
    }

    return QVariant::fromValue(m_fixtureTree);
}

void VCSlider::setLevelValue(uchar value)
{
    m_levelValueMutex.lock();
    m_levelValue = value;
    if (m_monitorEnabled == true)
        m_monitorValue = m_levelValue;
    m_levelValueChanged = true;
    m_levelValueMutex.unlock();
}

uchar VCSlider::levelValue() const
{
    return m_levelValue;
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
 * Playback mode
 *********************************************************************/

quint32 VCSlider::playbackFunction() const
{
    return m_playbackFunction;
}

void VCSlider::setPlaybackFunction(quint32 playbackFunction)
{
    bool running = false;

    if (m_playbackFunction == playbackFunction)
        return;

    Function* current = m_doc->function(m_playbackFunction);
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

    Function* function = m_doc->function(playbackFunction);
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
        m_playbackFunction = playbackFunction;
        if ((isEditing() && caption().isEmpty()) || caption() == defaultCaption())
            setCaption(function->name());

        if(running)
            function->start(m_doc->masterTimer(), functionParent());
        emit playbackFunctionChanged(playbackFunction);
    }
    else
    {
        /* No function attachment */
        m_playbackFunction = Function::invalidId();
        emit playbackFunctionChanged(-1);
    }
    setDocModified();
}

FunctionParent VCSlider::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCSlider::slotInputValueChanged(quint8 id, uchar value)
{
    if (id != INPUT_SLIDER_CONTROL_ID)
        return;

    setValue(value);
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

    SliderMode sliderMode = Playback;
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
            sliderMode = stringToSliderMode(root.readElementText());

            str = mAttrs.value(KXMLQLCVCSliderValueDisplayStyle).toString();
            setValueDisplayStyle(stringToValueDisplayStyle(str));
/*
            if (mAttrs.hasAttribute(KXMLQLCVCSliderClickAndGoType))
            {
                str = mAttrs.value(KXMLQLCVCSliderClickAndGoType).toString();
                setClickAndGoType(ClickAndGoWidget::stringToClickAndGoType(str));
            }

            if (mAttrs.hasAttribute(KXMLQLCVCSliderLevelMonitor))
            {
                if (mAttrs.value(KXMLQLCVCSliderLevelMonitor).toString() == "false")
                    setChannelsMonitorEnabled(false);
                else
                    setChannelsMonitorEnabled(true);
            }
*/
        }
        else if (root.name() == KXMLQLCVCSliderLevel)
        {
            loadXMLLevel(root);
        }
        else if (root.name() == KXMLQLCVCSliderPlayback)
        {
            loadXMLPlayback(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    /* Set the mode last, after everything else has been set */
    setSliderMode(sliderMode);

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
    setLevelValue(str.toInt());

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

bool VCSlider::loadXMLPlayback(QXmlStreamReader &pb_root)
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
            setPlaybackFunction(pb_root.readElementText().toUInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider playback tag:" << pb_root.name().toString();
            pb_root.skipCurrentElement();
        }
    }

    return true;
}
