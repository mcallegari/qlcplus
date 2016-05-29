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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "virtualconsole.h"
#include "vcwidget.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcclock.h"
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
    : PreviewContext(view, doc, parent)
    , m_latestWidgetId(0)
    , m_resizeMode(false)
    , m_selectedPage(0)
    , m_selectedWidget(NULL)
{
    Q_ASSERT(doc != NULL);

    for (int i = 0; i < VC_PAGES_NUMBER; i++)
    {
        VCFrame *page = new VCFrame(m_doc, this, this);
        QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
        page->setAllowResize(false);
        page->setShowHeader(false);
        page->setGeometry(QRect(0, 0, 1920, 1080));
        page->setFont(QFont("Roboto Condensed", 16));
        m_pages.append(page);
    }

    qmlRegisterType<VCWidget>("com.qlcplus.classes", 1, 0, "VCWidget");
    qmlRegisterType<VCFrame>("com.qlcplus.classes", 1, 0, "VCFrame");
    qmlRegisterType<VCButton>("com.qlcplus.classes", 1, 0, "VCButton");
    qmlRegisterType<VCLabel>("com.qlcplus.classes", 1, 0, "VCLabel");
    qmlRegisterType<VCSlider>("com.qlcplus.classes", 1, 0, "VCSlider");
    qmlRegisterType<VCClock>("com.qlcplus.classes", 1, 0, "VCClock");
    qmlRegisterType<VCClockSchedule>("com.qlcplus.classes", 1, 0, "VCClockSchedule");
}

void VirtualConsole::renderPage(QQuickItem *parent, QQuickItem *contentItem, int page)
{
    if (parent == NULL)
        return;

    if (page < 0 || page >= m_pages.count())
        return;

    QRect pageRect = m_pages.at(page)->geometry();
    parent->setProperty("contentWidth", pageRect.width());
    parent->setProperty("contentHeight", pageRect.height());

    qDebug() << "[VC] renderPage. Parent:" << parent << "contents rect:" << pageRect;

    m_pages.at(page)->render(m_view, contentItem);
}

void VirtualConsole::setWidgetSelection(quint32 wID, QQuickItem *item, bool enable)
{
    // disable any previously selected widget
    // TODO: handle multiple widgets selection
    resetWidgetSelection();

    if (enable)
    {
        m_itemsMap[wID] = item;
        if (m_selectedWidget != NULL)
            m_selectedWidget->setIsEditing(false);

        m_selectedWidget = m_widgetsMap[wID];

        if (m_selectedWidget != NULL)
            m_selectedWidget->setIsEditing(true);
    }
    else
    {
        if (m_selectedWidget != NULL)
            m_selectedWidget->setIsEditing(false);
        m_selectedWidget = NULL;
    }

    emit selectedWidgetChanged(m_selectedWidget);
}

void VirtualConsole::resetWidgetSelection()
{
    foreach(QQuickItem *widget, m_itemsMap.values())
        widget->setProperty("isSelected", false);
    m_itemsMap.clear();

    m_selectedWidget = NULL;
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
    foreach (VCFrame *page, m_pages)
        page->deleteChildren();

    m_widgetsMap.clear();
    m_latestWidgetId = 0;
    m_selectedPage = 0;
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

int VirtualConsole::selectedPage() const
{
    return m_selectedPage;
}

void VirtualConsole::setSelectedPage(int selectedPage)
{
    if (m_selectedPage == selectedPage)
        return;

    m_selectedPage = selectedPage;
    emit selectedPageChanged(selectedPage);
}

bool VirtualConsole::editMode() const
{
    return m_resizeMode;
}

void VirtualConsole::setEditMode(bool resizeMode)
{
    if (m_resizeMode == resizeMode)
        return;

    m_resizeMode = resizeMode;
    emit editModeChanged(resizeMode);
}

VCWidget *VirtualConsole::selectedWidget() const
{
    return m_selectedWidget;
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

bool VirtualConsole::loadXML(QXmlStreamReader &root)
{
    int currPageIdx = 0;

    if (root.name() != KXMLQLCVirtualConsole)
    {
        qWarning() << Q_FUNC_INFO << "Virtual Console node not found";
        return false;
    }

    while (root.readNextStartElement())
    {
        //qDebug() << "VC tag:" << root.name();
        if (root.name() == KXMLQLCVCFrame)
        {
            if (currPageIdx == m_pages.count())
            {
                VCFrame *page = new VCFrame(m_doc, this, this);
                QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
                page->setAllowResize(false);
                page->setShowHeader(false);
                page->setGeometry(QRect(0, 0, 1920, 1080));
                page->setFont(QFont("Roboto Condensed", 16));
                m_pages.append(page);
            }
            /* Contents */
            m_pages.at(currPageIdx)->loadXML(root);
            currPageIdx++;

        }
        else if (root.name() == KXMLQLCVCProperties)
        {
            loadPropertiesXML(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Virtual Console tag"
                       << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VirtualConsole::loadPropertiesXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCProperties)
    {
        qWarning() << Q_FUNC_INFO << "Virtual Console properties node not found";
        return false;
    }

    QString str;
    while (root.readNextStartElement())
    {
        /** This is a legacy property, converted into
         *  VCFrame "WindowState" tag */
        if (root.name() == KXMLQLCVCPropertiesSize)
        {
            QSize sz;

            /* Width */
            str = root.attributes().value(KXMLQLCVCPropertiesSizeWidth).toString();
            if (str.isEmpty() == false)
                sz.setWidth(str.toInt());

            /* Height */
            str = root.attributes().value(KXMLQLCVCPropertiesSizeHeight).toString();
            if (str.isEmpty() == false)
                sz.setHeight(str.toInt());

            /* Set size if both are valid */
            if (sz.isValid() == true)
                m_pages.at(0)->setGeometry(QRect(0, 0, sz.width(), sz.height()));
            root.skipCurrentElement();
        }
#if 0
        else if (root.name() == KXMLQLCVCPropertiesGrandMaster)
        {
            QXmlStreamAttributes attrs = root.attributes();

            str = attrs.value(KXMLQLCVCPropertiesGrandMasterChannelMode).toString();
            setGrandMasterChannelMode(GrandMaster::stringToChannelMode(str));

            str = attrs.value(KXMLQLCVCPropertiesGrandMasterValueMode).toString();
            setGrandMasterValueMode(GrandMaster::stringToValueMode(str));

            if (attrs.hasAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode))
            {
                str = attrs.value(KXMLQLCVCPropertiesGrandMasterSliderMode).toString();
                setGrandMasterSliderMode(GrandMaster::stringToSliderMode(str));
            }

            QXmlStreamReader::TokenType tType = root.readNext();
            if (tType == QXmlStreamReader::Characters)
                tType = root.readNext();

            // check if there is a Input tag defined
            if (tType == QXmlStreamReader::StartElement)
            {
                if (root.name() == KXMLQLCVCPropertiesInput)
                {
                    quint32 universe = InputOutputMap::invalidUniverse();
                    quint32 channel = QLCChannel::invalid();
                    /* External input */
                    if (loadXMLInput(root, &universe, &channel) == true)
                        setGrandMasterInputSource(universe, channel);
                }
                root.skipCurrentElement();
            }
        }
#endif
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Virtual Console property tag:"
                       << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VirtualConsole::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Virtual Console entry */
    doc->writeStartElement(KXMLQLCVirtualConsole);

    /* Contents */
    for (int i = 0; i < m_pages.count(); i++)
        m_pages.at(i)->saveXML(doc);

    /* Properties */
    //m_properties.saveXML(doc);

    /* End the <VirtualConsole> tag */
    doc->writeEndElement();

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
