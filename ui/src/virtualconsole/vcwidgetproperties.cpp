/*
  Q Light Controller
  vcwidgetproperties.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QWidget>
#include <QDebug>

#include "vcwidgetproperties.h"

VCWidgetProperties::VCWidgetProperties()
{
    m_state = Qt::WindowNoState;
    m_visible = false;
    m_x = 100;
    m_y = 100;
    m_width = 0;
    m_height = 0;
}

VCWidgetProperties::VCWidgetProperties(const VCWidgetProperties& properties)
{
    m_state = properties.m_state;
    m_visible = properties.m_visible;
    m_x = properties.m_x;
    m_y = properties.m_y;
    m_width = properties.m_width;
    m_height = properties.m_height;
}

VCWidgetProperties::~VCWidgetProperties()
{
}

QFlags <Qt::WindowState> VCWidgetProperties::state() const
{
    return m_state;
}

bool VCWidgetProperties::visible() const
{
    return m_visible;
}

int VCWidgetProperties::x() const
{
    return m_x;
}

int VCWidgetProperties::y() const
{
    return m_y;
}

int VCWidgetProperties::width() const
{
    return m_width;
}

int VCWidgetProperties::height() const
{
    return m_height;
}

void VCWidgetProperties::store(QWidget* widget)
{
    Q_ASSERT(widget != NULL);
    m_state = widget->windowState();
    m_visible = widget->isVisible();
    m_x = widget->x();
    m_y = widget->y();
    m_width = widget->width();
    m_height = widget->height();
}

bool VCWidgetProperties::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCWidgetProperties)
    {
        qWarning() << Q_FUNC_INFO << "Widget Properties node not found";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWidgetX)
            m_x = root.readElementText().toInt();
        else if (root.name() == KXMLQLCWidgetY)
            m_y = root.readElementText().toInt();
        else if (root.name() == KXMLQLCWidgetWidth)
            m_width = root.readElementText().toInt();
        else if (root.name() == KXMLQLCWidgetHeight)
            m_height = root.readElementText().toInt();
        else if (root.name() == KXMLQLCWidgetState)
            m_state = Qt::WindowState(root.readElementText().toInt());
        else if (root.name() == KXMLQLCWidgetVisible)
            m_visible = bool(root.readElementText().toInt());
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown widget tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCWidgetProperties::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Widget Properties entry */
    doc->writeStartElement(KXMLQLCWidgetProperties);

    /* X */
    doc->writeTextElement(KXMLQLCWidgetX, QString::number(m_x));
    /* Y */
    doc->writeTextElement(KXMLQLCWidgetY, QString::number(m_y));
    /* W */
    doc->writeTextElement(KXMLQLCWidgetWidth, QString::number(m_width));
    /* H */
    doc->writeTextElement(KXMLQLCWidgetHeight, QString::number(m_height));
    /* Window state */
    doc->writeTextElement(KXMLQLCWidgetState, QString::number(m_state));
    /* Visible state */
    doc->writeTextElement(KXMLQLCWidgetVisible, QString::number(m_visible));

    /* End the <WidgetProperties> tag */
    doc->writeEndElement();

    return true;
}

