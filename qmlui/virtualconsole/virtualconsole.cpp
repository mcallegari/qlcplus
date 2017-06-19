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
#include "contextmanager.h"
#include "vcwidget.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcclock.h"
#include "vcpage.h"
#include "doc.h"
#include "app.h"

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

#define DEFAULT_VC_PAGES_NUMBER 4

VirtualConsole::VirtualConsole(QQuickView *view, Doc *doc,
                               ContextManager *ctxManager, QObject *parent)
    : PreviewContext(view, doc, "VC", parent)
    , m_editMode(false)
    , m_contextManager(ctxManager)
    , m_selectedPage(0)
    , m_latestWidgetId(0)
    , m_inputDetectionEnabled(false)
    , m_autoDetectionWidget(NULL)
    , m_autoDetectionSource(NULL)
    , m_autoDetectionKey(QKeySequence())
    , m_autoDetectionKeyId(UINT_MAX)
{
    Q_ASSERT(doc != NULL);

    setContextResource("qrc:/VirtualConsole.qml");
    setContextTitle(tr("Virtual Console"));

    for (int i = 0; i < DEFAULT_VC_PAGES_NUMBER; i++)
    {
        VCPage *page = new VCPage(view, m_doc, this, i, this);
        QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
        m_contextManager->registerContext(page->previewContext());
        m_pages.append(page);
    }

    qmlRegisterUncreatableType<GrandMaster>("org.qlcplus.classes", 1, 0, "GrandMaster", "Can't create a GrandMaster !");

    qmlRegisterType<VCWidget>("org.qlcplus.classes", 1, 0, "VCWidget");
    qmlRegisterType<VCFrame>("org.qlcplus.classes", 1, 0, "VCFrame");
    qmlRegisterType<VCPage>("org.qlcplus.classes", 1, 0, "VCPage");
    qmlRegisterType<VCButton>("org.qlcplus.classes", 1, 0, "VCButton");
    qmlRegisterType<VCLabel>("org.qlcplus.classes", 1, 0, "VCLabel");
    qmlRegisterType<VCSlider>("org.qlcplus.classes", 1, 0, "VCSlider");
    qmlRegisterType<VCClock>("org.qlcplus.classes", 1, 0, "VCClock");
    qmlRegisterType<VCClockSchedule>("org.qlcplus.classes", 1, 0, "VCClockSchedule");

    connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar,QString)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
}

qreal VirtualConsole::pixelDensity() const
{
    App *app = qobject_cast<App *>(m_view);
    return app->pixelDensity();
}

void VirtualConsole::resetContents()
{
    foreach (VCPage *page, m_pages)
    {
        page->deleteChildren();
        page->resetInputSourcesMap();
    }

    m_widgetsMap.clear();
    m_latestWidgetId = 0;
    m_selectedPage = 0;
}

bool VirtualConsole::editMode() const
{
    return m_editMode;
}

void VirtualConsole::setEditMode(bool editMode)
{
    if (m_editMode == editMode)
        return;

    m_editMode = editMode;
    emit editModeChanged(editMode);
}

/*********************************************************************
 * Pages
 *********************************************************************/

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

VCPage *VirtualConsole::page(int page) const
{
    if (page < 0 || page >= m_pages.count())
        return NULL;

    return m_pages.at(page);
}

QQuickItem *VirtualConsole::currentPageItem() const
{
    QString currPage = QString("vcPage%1").arg(m_selectedPage);
    QQuickItem *pageItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>(currPage));
    return pageItem;
}

int VirtualConsole::pagesCount() const
{
    return m_pages.count();
}

void VirtualConsole::addPage(int index)
{
    VCPage *page = new VCPage(m_view, m_doc, this, index, this);
    QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
    m_contextManager->registerContext(page->previewContext());
    m_pages.insert(index, page);

    emit pagesCountChanged();

    if (index == m_selectedPage)
    {
        m_selectedPage++;
        emit selectedPageChanged(m_selectedPage);
    }
}

void VirtualConsole::deletePage(int index)
{
    if (index < 0 || index >= m_pages.count())
        return;

    m_pages.at(index)->deleteChildren();
    VCPage *page = m_pages.takeAt(index);
    m_contextManager->unregisterContext(page->previewContext()->name());
    delete page;

    m_itemsMap.clear();

    emit pagesCountChanged();

    if (index > 0)
    {
        m_selectedPage--;
        emit selectedPageChanged(m_selectedPage);
    }
}

bool VirtualConsole::setPagePIN(int index, QString currentPIN, QString newPIN)
{
    bool ok = false;
    int iPIN;

    Q_UNUSED(iPIN)

    if (index < 0 || index >= m_pages.count())
        return false;

    /* A PIN must be either empty or 4 digits */
    if (newPIN.length() != 0 && newPIN.length() != 4)
        return false;

    /* If the current PIN is set, check if
     * the entered PIN is numeric */
    if (m_pages.at(index)->PIN() != 0)
    {
        iPIN = currentPIN.toInt(&ok);
        if (ok == false)
            return false;
    }

    /* Check if the current PIN matches with the page PIN */
    if (m_pages.at(index)->PIN() != 0 &&
        m_pages.at(index)->PIN() != currentPIN.toInt())
        return false;

    /* At last, set the new PIN for the page */
    if (newPIN.isEmpty())
        m_pages.at(index)->setPIN(0);
    else
    {
        /* If the new PIN is numeric */
        iPIN = newPIN.toInt(&ok);
        if (ok == false)
            return false;

        m_pages.at(index)->setPIN(newPIN.toInt());
    }

    return true;
}

bool VirtualConsole::validatePagePIN(int index, QString PIN, bool remember)
{
    if (index < 0 || index >= m_pages.count())
        return false;

    if (m_pages.at(index)->PIN() != PIN.toInt())
        return false;

    if(remember)
        m_pages.at(index)->validatePIN();

    return true;
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

void VirtualConsole::setPageInteraction(bool enable)
{
    QQuickItem *page = currentPageItem();
    if (page != NULL)
        page->setProperty("interactive", enable);
}

/*********************************************************************
 * Widgets
 *********************************************************************/

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

void VirtualConsole::removeWidgetFromMap(VCWidget *widget)
{
    if (widget == NULL)
        return;

    m_widgetsMap.remove(widget->id());
}

VCWidget *VirtualConsole::widget(quint32 id)
{
    if (id == VCWidget::invalidId())
        return NULL;

    return m_widgetsMap.value(id, NULL);
}

void VirtualConsole::setWidgetSelection(quint32 wID, QQuickItem *item, bool enable, bool multi)
{
    VCWidget *vcWidget = NULL;

    if (multi == false)
    {
        // disable any previously selected widget
        QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
        while(it.hasNext())
        {
            it.next();
            QQuickItem *uiWidget = it.value();
            vcWidget = m_widgetsMap[it.key()];
            uiWidget->setProperty("isSelected", false);
            vcWidget->setIsEditing(false);
        }
        m_itemsMap.clear();
    }

    if (enable)
    {
        m_itemsMap[wID] = item;

        vcWidget = m_widgetsMap[wID];

        if (vcWidget != NULL)
            vcWidget->setIsEditing(true);

        if (selectedWidgetsCount() == 1)
            emit selectedWidgetChanged();
    }
    else
    {
        m_itemsMap.remove(wID);

        vcWidget = m_widgetsMap[wID];

        if (vcWidget != NULL)
            vcWidget->setIsEditing(false);
    }

    emit selectedWidgetsCountChanged();
}

void VirtualConsole::resetWidgetSelection()
{
    foreach(QQuickItem *widget, m_itemsMap.values())
        widget->setProperty("isSelected", false);
    m_itemsMap.clear();

    emit selectedWidgetChanged();
    emit selectedWidgetsCountChanged();
}

QStringList VirtualConsole::selectedWidgetNames()
{
    QStringList names;
    if (m_itemsMap.isEmpty() == false)
    {
        foreach(quint32 wID, m_itemsMap.keys())
        {
            VCWidget *vcWidget = m_widgetsMap[wID];
            if (vcWidget != NULL)
                names << vcWidget->caption();
        }
    }

    return names;
}

int VirtualConsole::selectedWidgetsCount() const
{
    return m_itemsMap.keys().count();
}

QVariantList VirtualConsole::selectedWidgetIDs()
{
    QVariantList ids;
    if (m_itemsMap.isEmpty() == false)
    {
        foreach(quint32 wID, m_itemsMap.keys())
            ids << wID;
    }

    return ids;
}

void VirtualConsole::moveWidget(VCWidget *widget, VCFrame *targetFrame, QPoint pos)
{
    // reset all the drop targets, otherwise two overlapping
    // frames can get the same drop event
    resetDropTargets(true);

    VCFrame *sourceFrame = qobject_cast<VCFrame*>(widget->parent());

    if (sourceFrame != targetFrame)
    {
        sourceFrame->removeWidgetFromPageMap(widget);
        widget->setPage(targetFrame->currentPage());
        targetFrame->addWidgetToPageMap(widget);

        widget->setParent(targetFrame);
    }

    QRect wRect = widget->geometry();
    wRect.moveTopLeft(pos);
    widget->setGeometry(wRect);

    qDebug() << "New widget geometry:" << widget->geometry();
}

void VirtualConsole::setWidgetsAlignment(VCWidget *refWidget, int alignment)
{
    if (refWidget == NULL)
        return;

    QRect refGeom = refWidget->geometry();

    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        QRect wGeom = widget->geometry();

        switch(alignment)
        {
            case Qt::AlignTop:
                widget->setGeometry(QRect(wGeom.x(), refGeom.y(), wGeom.width(), wGeom.height()));
            break;
            case Qt::AlignLeft:
                widget->setGeometry(QRect(refGeom.x(), wGeom.y(), wGeom.width(), wGeom.height()));
            break;
            case Qt::AlignRight:
            {
                // TODO: for now, let's do an ingnorant alignment, without considering
                // that widgets can be nested into VC frames...
                int right = refGeom.x() + refGeom.width();
                widget->setGeometry(QRect(right - wGeom.width(), wGeom.y(), wGeom.width(), wGeom.height()));
            }
            break;
            case Qt::AlignBottom:
            {
                // TODO: for now, let's do an ingnorant alignment, without considering
                // that widgets can be nested into VC frames...
                int bottom = refGeom.y() + refGeom.height();
                widget->setGeometry(QRect(wGeom.x(), bottom - wGeom.height(), wGeom.width(), wGeom.height()));
            }
            break;
        }
    }
}

void VirtualConsole::setWidgetsCaption(QString caption)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setCaption(caption);
    }
}

void VirtualConsole::setWidgetsForegroundColor(QColor color)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setForegroundColor(color);
    }
}

void VirtualConsole::setWidgetsBackgroundColor(QColor color)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setBackgroundColor(color);
    }
}

void VirtualConsole::setWidgetsBackgroundImage(QString path)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setBackgroundImage(path);
    }
}

void VirtualConsole::setWidgetsFont(QFont font)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setFont(font);
    }
}

void VirtualConsole::deleteVCWidgets(QVariantList IDList)
{
    foreach(QVariant id, IDList)
    {
        quint32 wID = id.toUInt();
        VCWidget *w = widget(wID);
        if (w == NULL)
            continue;

        /* 1- remove the widget from its parent frame page map */
        VCFrame *parentFrame = qobject_cast<VCFrame *>(w->parent());
        if (parentFrame != NULL)
            parentFrame->removeWidgetFromPageMap(w);

        /* 2- remove the widget from the global VC widgets map */
        m_widgetsMap.remove(wID);

        /* 3- if the widget is a frame, delete also all its children */
        if (w->type() == VCWidget::FrameWidget || w->type() == VCWidget::SoloFrameWidget)
        {
            VCFrame *frame = qobject_cast<VCFrame *>(w);
            foreach (VCWidget* child, frame->children(true))
                m_widgetsMap.remove(child->id());
        }

        /* 4- perform the actual widget deletion */
        delete w;

        /* 5- if the widget was selected, delete the on-screen Quick item */
        if (m_itemsMap.contains(wID))
        {
            QQuickItem *qItem = m_itemsMap[wID];
            emit selectedWidgetChanged();
            delete qItem;
        }
    }
    m_itemsMap.clear();
}

VCWidget *VirtualConsole::selectedWidget() const
{
    if (m_itemsMap.isEmpty())
        return qobject_cast<VCWidget *>(m_pages.at(m_selectedPage));

    return m_widgetsMap[m_itemsMap.firstKey()];
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



/*********************************************************************
 * External input
 *********************************************************************/

bool VirtualConsole::createAndDetectInputSource(VCWidget *widget)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == NULL)
        return false;

    /** Create an empty input source and add it to the requested widget */
    QSharedPointer<QLCInputSource> source = QSharedPointer<QLCInputSource>(new QLCInputSource());
    widget->addInputSource(source);

    enableInputSourceAutoDetection(widget, source->id(), QLCInputSource::invalidUniverse, QLCInputSource::invalidChannel);

    return true;
}

bool VirtualConsole::createAndDetectInputKey(VCWidget *widget)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == NULL)
        return false;

    widget->addKeySequence(QKeySequence());
    enableKeyAutoDetection(widget, 0, "");

    return true;
}

bool VirtualConsole::enableInputSourceAutoDetection(VCWidget *widget, quint32 id, quint32 universe, quint32 channel)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == NULL)
        return false;

    qDebug() << "[enableInputSourceAutoDetection] id:" << id << ",uni:" << universe << ",ch:" << channel;

    /** Save also the reference to the requested widget and source, otherwise
     *  the slotInputValueChanged method will not know where to act */
    m_autoDetectionSource = widget->inputSource(id, universe, channel);

    if (m_autoDetectionSource.isNull())
    {
        qDebug() << "Input source is null. Aborting autodetection.";
        return false;
    }

    m_autoDetectionWidget = widget;

    qDebug() << "Autodetection enabled on widget" << widget->id();

    /** Finally raise the auto detection flag, to
     *  modify the behaviour of slotInputValueChanged */
    m_inputDetectionEnabled = true;

    /** Note that VC pages should update their multi hash map as well,
     *  but since the input source is still invalid, this action is deferred
     *  to when the first input signal comes from an external controller */

    return true;
}

void VirtualConsole::updateInputSourceControlID(VCWidget *widget, quint32 id, quint32 universe, quint32 channel)
{
    if (widget == NULL)
        return;

    qDebug() << "Setting control ID" << id << "to widget" << widget->caption();

    widget->updateInputSourceControlID(universe, channel, id);
}

bool VirtualConsole::enableKeyAutoDetection(VCWidget *widget, quint32 id, QString keyText)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == NULL)
        return false;

    qDebug() << "[enableKeyAutoDetection] id:" << id << ", key:" << keyText;

    m_autoDetectionKey = QKeySequence(keyText);
    m_autoDetectionWidget = widget;
    m_autoDetectionKeyId = id;
    m_inputDetectionEnabled = true;

    return true;
}

void VirtualConsole::updateKeySequenceControlID(VCWidget *widget, quint32 id, QString keyText)
{
    if (widget == NULL)
        return;

    qDebug() << "Setting control ID" << id << "to widget" << widget->caption();

    QKeySequence seq(keyText);

    widget->updateKeySequenceControlID(seq, id);

    /** Update also the key sequence maps in VC pages */
    for(VCPage *page : m_pages) // C++11
        page->updateKeySequenceIDInMap(seq, id, widget, true);
}

void VirtualConsole::disableAutoDetection()
{
    m_inputDetectionEnabled = false;
    m_autoDetectionWidget = NULL;
    m_autoDetectionSource.clear();
    m_autoDetectionKey = QKeySequence();
    m_autoDetectionKeyId = UINT_MAX;
}

void VirtualConsole::deleteInputSource(VCWidget *widget, quint32 id, quint32 universe, quint32 channel)
{
    if (widget == NULL)
        return;

    /** In case an autodetection process is running, stop it */
    disableAutoDetection();

    for(VCPage *page : m_pages) // C++11
        page->unMapInputSource(id, universe, channel, widget, true);

    widget->deleteInputSurce(id, universe, channel);
}

void VirtualConsole::deleteKeySequence(VCWidget *widget, quint32 id, QString keyText)
{
    if (widget == NULL)
        return;

    /** In case an autodetection process is running, stop it */
    disableAutoDetection();

    QKeySequence seq(keyText);

    for(VCPage *page : m_pages) // C++11
        page->unMapKeySequence(seq, id, widget, true);

    widget->deleteKeySequence(seq);
}

void VirtualConsole::handleKeyEvent(QKeyEvent *e, bool pressed)
{
    if (m_inputDetectionEnabled == false)
    {
        /* Ignore the repeating events */
        if (e->isAutoRepeat())
            return;

        for(VCPage *page : m_pages) // C++11
            page->handleKeyEvent(e, pressed);
    }
    else
    {
        Q_ASSERT(m_autoDetectionWidget != NULL);

        if (pressed == false)
            return;

        QKeySequence seq(e->key() | e->modifiers());
        qDebug() << "Got key sequence:" << seq.toString(QKeySequence::NativeText);
        m_autoDetectionWidget->updateKeySequence(m_autoDetectionKey, seq, m_autoDetectionKeyId);

        for(VCPage *page : m_pages) // C++11
            page->mapKeySequence(seq, m_autoDetectionKeyId, m_autoDetectionWidget, true);

        /** At last, disable the autodetection process */
        m_inputDetectionEnabled = false;
        m_autoDetectionWidget = NULL;
        m_autoDetectionKey = QKeySequence();
        m_autoDetectionKeyId = UINT_MAX;
    }
}

void VirtualConsole::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    qDebug() << "Input signal received. Universe:" << universe << ", channel:" << channel << ", value:" << value;
    if (m_inputDetectionEnabled == false)
    {
        for(VCPage *page : m_pages) // C++11
        {
            // TODO: send only to enabled (visible) pages
            page->inputValueChanged(universe, channel, value);
        }
    }
    else
    {
        /** The widget reference must be not NULL, otherwise
         *  it means something went nuts */
        Q_ASSERT(m_autoDetectionWidget != NULL);

        m_autoDetectionWidget->updateInputSource(m_autoDetectionSource, universe, channel);

        for(VCPage *page : m_pages) // C++11
            page->mapInputSource(m_autoDetectionSource, m_autoDetectionWidget, true);

        /** At last, disable the autodetection process */
        m_inputDetectionEnabled = false;
        m_autoDetectionWidget = NULL;
        m_autoDetectionSource.clear();
    }
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
                VCPage *page = new VCPage(m_view, m_doc, this, currPageIdx, this);
                QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
                m_contextManager->registerContext(page->previewContext());
                m_pages.append(page);
            }
            /* Contents */
            m_pages.at(currPageIdx)->loadXML(root);
            if (m_pages.at(currPageIdx)->caption().isEmpty())
                m_pages.at(currPageIdx)->setCaption(tr("Page %1").arg(currPageIdx + 1));
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

bool VirtualConsole::loadXMLLegacyInput(QXmlStreamReader &root, quint32* uni, quint32* ch) const
{
    if (root.name() != KXMLQLCVCWidgetInput)
    {
        qWarning() << Q_FUNC_INFO << "Input node not found!";
        return false;
    }
    else
    {
        QXmlStreamAttributes attrs = root.attributes();
        *uni = attrs.value(KXMLQLCVCWidgetInputUniverse).toString().toUInt();
        *ch = attrs.value(KXMLQLCVCWidgetInputChannel).toString().toUInt();
        root.skipCurrentElement();
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

    GrandMaster::ChannelMode gmLegacyChannelMode = GrandMaster::Intensity;
    GrandMaster::ValueMode gmLegacyValueMode = GrandMaster::Reduce;
    GrandMaster::SliderMode gmLegacySliderMode = GrandMaster::Normal;
    QSharedPointer<QLCInputSource> gmLegacyInputSource;

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
        else if (root.name() == KXMLQLCVCPropertiesGrandMaster)
        {
            QXmlStreamAttributes attrs = root.attributes();

            str = attrs.value(KXMLQLCVCPropertiesGrandMasterChannelMode).toString();
            gmLegacyChannelMode = GrandMaster::stringToChannelMode(str);

            str = attrs.value(KXMLQLCVCPropertiesGrandMasterValueMode).toString();
            gmLegacyValueMode = GrandMaster::stringToValueMode(str);

            if (attrs.hasAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode))
            {
                str = attrs.value(KXMLQLCVCPropertiesGrandMasterSliderMode).toString();
                gmLegacySliderMode = GrandMaster::stringToSliderMode(str);
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
                    if (loadXMLLegacyInput(root, &universe, &channel) == true)
                        gmLegacyInputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(universe, channel));
                }
                root.skipCurrentElement();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Virtual Console property tag:"
                       << root.name().toString();
            root.skipCurrentElement();
        }
    }

    /* Now check if there's the need to create a GrandMaster
     * as a VC Slider in GrandMaster mode */
    if (gmLegacyChannelMode != GrandMaster::Intensity || gmLegacyValueMode != GrandMaster::Reduce ||
        gmLegacySliderMode != GrandMaster::Normal || gmLegacyInputSource.isNull() == false)
    {
        VCSlider *slider = new VCSlider(m_doc, m_pages.at(0));
        QQmlEngine::setObjectOwnership(slider, QQmlEngine::CppOwnership);
        slider->setGeometry(QRect(0, 0, pixelDensity() * 10, m_pages.at(0)->geometry().height()));
        slider->setDefaultFontSize(pixelDensity() * 3.5);
        slider->setSliderMode(VCSlider::GrandMaster);

        slider->setGrandMasterChannelMode(gmLegacyChannelMode);
        slider->setGrandMasterValueMode(gmLegacyValueMode);

        if (gmLegacySliderMode == GrandMaster::Inverted)
            slider->setInvertedAppearance(true);

        if (gmLegacyInputSource.isNull() == false)
            slider->addInputSource(gmLegacyInputSource);

        m_pages.at(0)->addWidgetToPageMap(slider);
        addWidgetToMap(slider);
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

    foreach (VCPage *page, m_pages)
        widgetsList.append(page->children(true));

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

    /** Now for each page, map the children widgets input
     *  sources, to look it up later when input signals start
     *  to roll */
    foreach (VCPage *page, m_pages)
        page->mapChildrenInputSources();
}
