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

#include <QWidget>
#include <QtXml>

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

bool VCWidgetProperties::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCWidgetProperties)
    {
        qWarning() << Q_FUNC_INFO << "Widget Properties node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCWidgetX)
            m_x = tag.text().toInt();
        else if (tag.tagName() == KXMLQLCWidgetY)
            m_y = tag.text().toInt();
        else if (tag.tagName() == KXMLQLCWidgetWidth)
            m_width = tag.text().toInt();
        else if (tag.tagName() == KXMLQLCWidgetHeight)
            m_height = tag.text().toInt();
        else if (tag.tagName() == KXMLQLCWidgetState)
            m_state = Qt::WindowState(tag.text().toInt());
        else if (tag.tagName() == KXMLQLCWidgetVisible)
            m_visible = bool(tag.text().toInt());
        else
            qWarning() << Q_FUNC_INFO << "Unknown widget tag:" << tag.tagName();

        node = node.nextSibling();
    }

    return true;
}

bool VCWidgetProperties::saveXML(QDomDocument* doc, QDomElement* root)
{
    QDomElement prop_root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(root != NULL);

    /* Widget Properties entry */
    prop_root = doc->createElement(KXMLQLCWidgetProperties);
    root->appendChild(prop_root);

    /* X */
    tag = doc->createElement(KXMLQLCWidgetX);
    prop_root.appendChild(tag);
    text = doc->createTextNode(QString("%1").arg(m_x));
    tag.appendChild(text);

    /* Y */
    tag = doc->createElement(KXMLQLCWidgetY);
    prop_root.appendChild(tag);
    text = doc->createTextNode(QString("%1").arg(m_y));
    tag.appendChild(text);

    /* W */
    tag = doc->createElement(KXMLQLCWidgetWidth);
    prop_root.appendChild(tag);
    text = doc->createTextNode(QString("%1").arg(m_width));
    tag.appendChild(text);

    /* H */
    tag = doc->createElement(KXMLQLCWidgetHeight);
    prop_root.appendChild(tag);
    text = doc->createTextNode(QString("%1").arg(m_height));
    tag.appendChild(text);

    /* Window state */
    tag = doc->createElement(KXMLQLCWidgetState);
    prop_root.appendChild(tag);
    text = doc->createTextNode(QString("%1").arg(m_state));
    tag.appendChild(text);

    /* Visible state */
    tag = doc->createElement(KXMLQLCWidgetVisible);
    prop_root.appendChild(tag);
    text = doc->createTextNode(QString("%1").arg(m_visible));
    tag.appendChild(text);

    return true;
}

