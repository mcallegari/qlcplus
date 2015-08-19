/*
  Q Light Controller Plus
  virtualconsole.cpp

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

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QtXml>

#include "virtualconsole.h"
#include "vcwidget.h"
#include "vcbutton.h"
#include "vcframe.h"
#include "doc.h"

#define KXMLQLCVCProperties "Properties"
#define KXMLQLCVCPropertiesSize "Size"
#define KXMLQLCVCPropertiesSizeWidth "Width"
#define KXMLQLCVCPropertiesSizeHeight "Height"

#define KXMLQLCVCPropertiesGrandMaster "GrandMaster"
#define KXMLQLCVCPropertiesGrandMasterVisible "Visible"
#define KXMLQLCVCPropertiesGrandMasterChannelMode "ChannelMode"
#define KXMLQLCVCPropertiesGrandMasterValueMode "ValueMode"
#define KXMLQLCVCPropertiesGrandMasterSliderMode "SliderMode"

#define KXMLQLCVCPropertiesInput "Input"
#define KXMLQLCVCPropertiesInputUniverse "Universe"
#define KXMLQLCVCPropertiesInputChannel "Channel"

#define VC_PAGES_NUMBER 4

VirtualConsole::VirtualConsole(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_latestWidgetId(0)
    , m_resizeMode(false)
{
    Q_ASSERT(m_doc != NULL);

    for (int i = 0; i < VC_PAGES_NUMBER; i++)
    {
        VCFrame *page = new VCFrame(m_doc, this, this);
        QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
        page->setGeometry(QRect(0, 0, 1920, 1080));
        m_pages.append(page);
    }

    qmlRegisterType<VCWidget>("com.qlcplus.classes", 1, 0, "VCWidget");
    qmlRegisterType<VCFrame>("com.qlcplus.classes", 1, 0, "VCFrame");
    qmlRegisterType<VCButton>("com.qlcplus.classes", 1, 0, "VCButton");
}

QQuickView *VirtualConsole::view()
{
    return m_view;
}

void VirtualConsole::renderPage(QQuickItem *parent, int page)
{
    if (parent == NULL)
        return;

    if (page < 0 || page >= m_pages.count())
        return;

    QRect pageRect = m_pages.at(page)->geometry();
    parent->setProperty("contentWidth", pageRect.width());
    parent->setProperty("contentHeight", pageRect.height());

    qDebug() << "[VC] renderPage. Parent:" << parent << "rect:" << pageRect;

    m_pages.at(page)->render(m_view, parent);
}

/*********************************************************************
 * Contents
 *********************************************************************/

VCFrame *VirtualConsole::page(int page) const
{
    if (page < 0 || page >= m_pages.count())
        return NULL;

    return m_pages.at(page);
}

void VirtualConsole::resetContents()
{

}

quint32 VirtualConsole::newWidgetId()
{
    /* This results in an endless loop if there are UINT_MAX-1 widgets. That,
       however, seems a bit unlikely. */
    while (m_widgetsMap.contains(m_latestWidgetId) ||
           m_latestWidgetId == VCWidget::invalidId())
    {
        m_latestWidgetId++;
    }

    return m_latestWidgetId;
}

void VirtualConsole::addWidgetToMap(VCWidget* widget)
{
    // Valid ID ?
    if (widget->id() != VCWidget::invalidId())
    {

        // Maybe we don't know this widget yet
        if (!m_widgetsMap.contains(widget->id()))
        {
            m_widgetsMap.insert(widget->id(), widget);
            return;
        }

        // Maybe we already know this widget
        if (m_widgetsMap[widget->id()] == widget)
        {
            qDebug() << Q_FUNC_INFO << "widget" << widget->id() << "already in map";
            return;
        }

        // This widget id conflicts with another one we have to change it.
        qDebug() << Q_FUNC_INFO << "widget id" << widget->id() << "conflicts, creating a new ID";
    }

    quint32 wid = newWidgetId();
    Q_ASSERT(!m_widgetsMap.contains(wid));
    qDebug() << Q_FUNC_INFO << "id=" << wid;
    widget->setID(wid);
    m_widgetsMap.insert(wid, widget);
}

VCWidget *VirtualConsole::widget(quint32 id)
{
    if (id == VCWidget::invalidId())
        return NULL;

    return m_widgetsMap.value(id, NULL);
}

bool VirtualConsole::resizeMode() const
{
    return m_resizeMode;
}

void VirtualConsole::setResizeMode(bool resizeMode)
{
    if (m_resizeMode == resizeMode)
        return;

    m_resizeMode = resizeMode;
    emit resizeModeChanged(resizeMode);
}

/*********************************************************************
 * Drag & Drop
 *********************************************************************/

void VirtualConsole::setDropTarget(QQuickItem *target, bool enable)
{
    if (enable == true)
    {
        resetDropTargets(false);
        m_dropTargets << target;
        target->setProperty("dropActive", true);
    }
    else
    {
        for (int i = 0; i < m_dropTargets.count(); i++)
        {
            if (m_dropTargets.at(i) == target)
            {
                m_dropTargets.at(i)->setProperty("dropActive", false);
                if (i > 0)
                    m_dropTargets.at(i - 1)->setProperty("dropActive", true);
                m_dropTargets.removeLast();
                return;
            }
        }
    }
}

void VirtualConsole::resetDropTargets(bool deleteTargets)
{
    foreach(QQuickItem *item, m_dropTargets)
        item->setProperty("dropActive", false);

    if (deleteTargets)
        m_dropTargets.clear();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VirtualConsole::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCVirtualConsole)
    {
        qWarning() << Q_FUNC_INFO << "Virtual Console node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCFrame)
        {
            /* Contents */
            m_pages.at(0)->loadXML(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCProperties)
        {
            loadPropertiesXML(tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Virtual Console tag"
                       << tag.tagName();
        }

        /* Next node */
        node = node.nextSibling();
    }

    return true;
}

bool VirtualConsole::loadPropertiesXML(const QDomElement &root)
{
    if (root.tagName() != KXMLQLCVCProperties)
    {
        qWarning() << Q_FUNC_INFO << "Virtual Console properties node not found";
        return false;
    }

    QString str;
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        /** This is a legacy property, converted into
         *  VCFrame "WindowState" tag */
        if (tag.tagName() == KXMLQLCVCPropertiesSize)
        {
            QSize sz;

            /* Width */
            str = tag.attribute(KXMLQLCVCPropertiesSizeWidth);
            if (str.isEmpty() == false)
                sz.setWidth(str.toInt());

            /* Height */
            str = tag.attribute(KXMLQLCVCPropertiesSizeHeight);
            if (str.isEmpty() == false)
                sz.setHeight(str.toInt());

            /* Set size if both are valid */
            if (sz.isValid() == true)
                m_pages.at(0)->setGeometry(QRect(0, 0, sz.width(), sz.height()));
        }
#if 0
        else if (tag.tagName() == KXMLQLCVCPropertiesGrandMaster)
        {
            quint32 universe = InputOutputMap::invalidUniverse();
            quint32 channel = QLCChannel::invalid();

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterChannelMode);
            setGrandMasterChannelMode(GrandMaster::stringToChannelMode(str));

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterValueMode);
            setGrandMasterValueMode(GrandMaster::stringToValueMode(str));

            if (tag.hasAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode))
            {
                str = tag.attribute(KXMLQLCVCPropertiesGrandMasterSliderMode);
                setGrandMasterSliderMode(GrandMaster::stringToSliderMode(str));
            }

            /* External input */
            if (loadXMLInput(tag.firstChild().toElement(), &universe, &channel) == true)
                setGrandMasterInputSource(universe, channel);
        }
#endif
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Virtual Console property tag:"
                       << tag.tagName();
        }

        /* Next node */
        node = node.nextSibling();
    }

    return true;
}

void VirtualConsole::postLoad()
{
    /** apply GM values */
    m_doc->inputOutputMap()->setGrandMasterValue(255);
    //m_doc->inputOutputMap()->setGrandMasterValueMode(m_properties.grandMasterValueMode());
    //m_doc->inputOutputMap()->setGrandMasterChannelMode(m_properties.grandMasterChannelMode());

    /** Go through widgets, check IDs and register widgets to the map
      * This code is the same as the one in addWidgetToMap()
      * We have to repeat it to limit conflicts if
      * one widget was not saved with a valid ID,
      * as addWidgetToMap ensures the widget WILL be added */

    QList<VCWidget *> invalidWidgetsList;
    QList<VCWidget *> widgetsList;
    for (int i = 0; i < VC_PAGES_NUMBER; i++)
        widgetsList.append(m_pages.at(i)->children());

    foreach (VCWidget *widget, widgetsList)
    {
        quint32 wid = widget->id();
        if (wid != VCWidget::invalidId())
        {
            if (!m_widgetsMap.contains(wid))
                m_widgetsMap.insert(wid, widget);
            else if (m_widgetsMap[wid] != widget)
                invalidWidgetsList.append(widget);
        }
        else
            invalidWidgetsList.append(widget);
    }
    foreach (VCWidget *widget, invalidWidgetsList)
        addWidgetToMap(widget);
}
