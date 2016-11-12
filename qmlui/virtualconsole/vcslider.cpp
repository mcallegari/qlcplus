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

#include "qlcmacros.h"
#include "vcslider.h"
#include "doc.h"

#define INPUT_SLIDER_CONTROL_ID     0

VCSlider::VCSlider(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_valueDisplayStyle(DMXValue)
    , m_invertedAppearance(false)
    , m_sliderMode(Playback)
    , m_value(0)
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
    return tr("Slider %1").arg(id());
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

    /* Widget commons */
    loadXMLCommon(root);

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
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}
