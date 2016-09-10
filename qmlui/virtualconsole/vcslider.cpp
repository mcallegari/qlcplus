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
    , m_sliderMode(Playback)
    , m_value(0)
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
        setCaption(tr("Slider %1").arg(id));
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
    case Level:
        return QString("Level");
        break;

    case Playback:
        return QString("Playback");
        break;

    case Submaster:
        return QString("Submaster");
        break;

    default:
        return QString("Unknown");
        break;
    }
}

VCSlider::SliderMode VCSlider::stringToSliderMode(const QString& mode)
{
    if (mode == QString("Level"))
        return Level;
    else  if (mode == QString("Playback"))
       return Playback;
    else //if (mode == QString("Submaster"))
        return Submaster;
}

VCSlider::SliderMode VCSlider::sliderMode() const
{
    return m_sliderMode;
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
            loadXMLInput(root, INPUT_SLIDER_CONTROL_ID);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown slider tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}
