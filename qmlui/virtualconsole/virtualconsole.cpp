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
#include "qlcinputchannel.h"
#include "inputpatch.h"
#include "treemodel.h"
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcanimation.h"
#include "vcaudiotrigger.h"
#include "vcxypad.h"
#include "vcspeeddial.h"
#include "vcclock.h"
#include "vcpage.h"
#include "tardis.h"
#include "doc.h"
#include "app.h"

#define KXMLQLCVCProperties             QString("Properties")
#define KXMLQLCVCPropertiesSize         QString("Size")
#define KXMLQLCVCPropertiesSizeWidth    QString("Width")
#define KXMLQLCVCPropertiesSizeHeight   QString("Height")

#define KXMLQLCVCPropertiesGrandMaster              QString("GrandMaster")
#define KXMLQLCVCPropertiesGrandMasterVisible       QString("Visible")
#define KXMLQLCVCPropertiesGrandMasterChannelMode   QString("ChannelMode")
#define KXMLQLCVCPropertiesGrandMasterValueMode     QString("ValueMode")
#define KXMLQLCVCPropertiesGrandMasterSliderMode    QString("SliderMode")

#define KXMLQLCVCPropertiesInput            QString("Input")
#define KXMLQLCVCPropertiesInputUniverse    QString("Universe")
#define KXMLQLCVCPropertiesInputChannel     QString("Channel")

#define DEFAULT_VC_PAGES_NUMBER 4

VirtualConsole::VirtualConsole(QQuickView *view, Doc *doc,
                               ContextManager *ctxManager, QObject *parent)
    : PreviewContext(view, doc, "VC", parent)
    , m_editMode(false)
    , m_snapping(true)
    , m_loadStatus(Cleared)
    , m_contextManager(ctxManager)
    , m_selectedPage(0)
    , m_latestWidgetId(0)
    , m_inputDetectionEnabled(false)
    , m_autoDetectionWidget(nullptr)
    , m_autoDetectionSource(nullptr)
    , m_autoDetectionKey(QKeySequence())
    , m_autoDetectionKeyId(UINT_MAX)
    , m_inputChannelsTree(nullptr)
{
    Q_ASSERT(doc != nullptr);

    setContextResource("qrc:/VirtualConsole.qml");
    setContextTitle(tr("Virtual Console"));

    for (int i = 0; i < DEFAULT_VC_PAGES_NUMBER; i++)
    {
        VCPage *page = new VCPage(view, m_doc, this, i, this);
        QQmlEngine::setObjectOwnership(page, QQmlEngine::CppOwnership);
        addWidgetToMap(page);
        m_contextManager->registerContext(page->previewContext());
        m_pages.append(page);
    }

    view->rootContext()->setContextProperty("virtualConsole", this);
    qmlRegisterUncreatableType<GrandMaster>("org.qlcplus.classes", 1, 0, "GrandMaster", "Can't create a GrandMaster!");
    qmlRegisterUncreatableType<QLCInputChannel>("org.qlcplus.classes", 1, 0, "QLCInputChannel", "Can't create a QLCInputChannel!");

    qmlRegisterType<VCWidget>("org.qlcplus.classes", 1, 0, "VCWidget");
    qmlRegisterType<VCFrame>("org.qlcplus.classes", 1, 0, "VCFrame");
    qmlRegisterType<VCPage>("org.qlcplus.classes", 1, 0, "VCPage");
    qmlRegisterType<VCButton>("org.qlcplus.classes", 1, 0, "VCButton");
    qmlRegisterType<VCLabel>("org.qlcplus.classes", 1, 0, "VCLabel");
    qmlRegisterType<VCSlider>("org.qlcplus.classes", 1, 0, "VCSlider");
    qmlRegisterType<VCAnimation>("org.qlcplus.classes", 1, 0, "VCAnimation");
    qmlRegisterType<VCAudioTrigger>("org.qlcplus.classes", 1, 0, "VCAudioTrigger");
    qmlRegisterType<VCXYPad>("org.qlcplus.classes", 1, 0, "VCXYPad");
    qmlRegisterType<VCSpeedDial>("org.qlcplus.classes", 1, 0, "VCSpeedDial");
    qmlRegisterType<VCClock>("org.qlcplus.classes", 1, 0, "VCClock");
    qmlRegisterType<VCClockSchedule>("org.qlcplus.classes", 1, 0, "VCClockSchedule");
    qmlRegisterType<VCCueList>("org.qlcplus.classes", 1, 0, "VCCueList");

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
    resetWidgetSelection();

    foreach (VCPage *page, m_pages)
    {
        page->deleteChildren();
        page->resetInputSourcesMap();
    }

    m_widgetsMap.clear();
    m_latestWidgetId = 0;
    m_selectedPage = 0;
    m_loadStatus = Cleared;
}

bool VirtualConsole::editMode() const
{
    return m_editMode;
}

void VirtualConsole::setEditMode(bool editMode)
{
    if (m_editMode == editMode)
        return;

    if (editMode == false)
    {
        m_clipboardIDList.clear();
        emit clipboardItemsCountChanged();

        resetWidgetSelection();
    }

    m_editMode = editMode;
    emit editModeChanged(editMode);
}

bool VirtualConsole::snapping() const
{
    return m_snapping;
}

void VirtualConsole::setSnapping(bool enable)
{
    if (m_snapping == enable)
        return;

    m_snapping = enable;
    emit snappingChanged(enable);
}

qreal VirtualConsole::snappingSize()
{
    return pixelDensity() * 3;
}

VirtualConsole::LoadStatus VirtualConsole::loadStatus() const
{
    return m_loadStatus;
}

QVariantList VirtualConsole::usageList(quint32 fid)
{
    QVariantList list;

    QHash<quint32, VCWidget *>::const_iterator i = m_widgetsMap.constBegin();
    while (i != m_widgetsMap.constEnd())
    {
        VCWidget *widget = i.value();
        bool found = false;

        if (widget == nullptr)
        {
            ++i;
            continue;
        }
        switch (widget->type())
        {
            case VCWidget::ButtonWidget:
            {
                VCButton *button = qobject_cast<VCButton *>(widget);
                if (button->functionID() == fid)
                    found = true;
            }
            break;
            case VCWidget::SliderWidget:
            {
                VCSlider *slider = qobject_cast<VCSlider *>(widget);
                if (slider->controlledFunction() == fid)
                    found = true;
            }
            break;
            case VCWidget::CueListWidget:
            {
                VCCueList *cuelist = qobject_cast<VCCueList *>(widget);
                if (cuelist->chaserID() == fid)
                    found = true;
            }
            break;
            case VCWidget::ClockWidget:
            {
                VCClock *clock = qobject_cast<VCClock *>(widget);
                for (VCClockSchedule *schedule : clock->schedules())
                {
                    if (schedule->functionID() == fid)
                    {
                        found = true;
                        // a single match will do
                        break;
                    }
                }
            }
            break;
            default:
            break;
        }

        if (found)
        {
            QVariantMap wMap;
            wMap.insert("classRef", QVariant::fromValue(widget));
            wMap.insert("label", QString("%1").arg(widget->caption()));
            list.append(wMap);
        }

        ++i;
    }

    if (list.isEmpty())
    {
        QVariantMap noneMap;
        noneMap.insert("label", tr("<None>"));
        list.append(noneMap);
    }

    return list;
}

/*********************************************************************
 * Pages
 *********************************************************************/

void VirtualConsole::renderPage(QQuickItem *parent, QQuickItem *contentItem, int page)
{
    if (parent == nullptr)
        return;

    if (page < 0 || page >= m_pages.count())
        return;

    QRectF pageRect = m_pages.at(page)->geometry();
    parent->setProperty("contentWidth", pageRect.width());
    parent->setProperty("contentHeight", pageRect.height());

    qDebug() << "[VC] renderPage. Parent:" << parent << "contents rect:" << pageRect;

    m_pages.at(page)->render(m_view, contentItem);
}

void VirtualConsole::enableFlicking(bool enable)
{
    QQuickItem *currPage = currentPageItem();
    if (currPage == nullptr)
        return;

    currPage->setProperty("interactive", enable);
}

VCPage *VirtualConsole::page(int page) const
{
    if (page < 0 || page >= m_pages.count())
        return nullptr;

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

    if (m_pages.count() == 1)
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

    VCPage *page = m_pages.at(index);

    /* If the current PIN is set, check if
     * the entered PIN is numeric */
    if (page->PIN() != 0)
    {
        iPIN = currentPIN.toInt(&ok);
        if (ok == false)
            return false;
    }

    /* Check if the current PIN matches with the page PIN */
    if (page->PIN() != 0 &&
        page->PIN() != currentPIN.toInt())
        return false;

    /* At last, set the new PIN for the page */
    if (newPIN.isEmpty())
        page->setPIN(0);
    else
    {
        /* If the new PIN is numeric */
        iPIN = newPIN.toInt(&ok);
        if (ok == false)
            return false;

        page->setPIN(newPIN.toInt());
    }

    return true;
}

bool VirtualConsole::validatePagePIN(int index, QString PIN, bool remember)
{
    if (index < 0 || index >= m_pages.count())
        return false;

    if (m_pages.at(index)->PIN() != PIN.toInt())
        return false;

    if (remember)
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
    if (m_editMode)
        emit selectedWidgetChanged();
}

void VirtualConsole::setPageInteraction(bool enable)
{
    QQuickItem *page = currentPageItem();
    if (page != nullptr)
        page->setProperty("interactive", enable);
}

void VirtualConsole::setPageScale(qreal factor)
{
    m_pages.at(m_selectedPage)->setPageScale(factor);
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
    if (widget == nullptr)
        return;

    m_widgetsMap.remove(widget->id());
}

VCWidget *VirtualConsole::widget(quint32 id)
{
    if (id == VCWidget::invalidId())
        return nullptr;

    return m_widgetsMap.value(id, nullptr);
}

void VirtualConsole::setWidgetSelection(quint32 wID, QQuickItem *item, bool enable, bool multi)
{
    VCWidget *vcWidget = nullptr;

    if (multi == false)
    {
        // disable any previously selected widget
        QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
        while (it.hasNext())
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
        if (item != nullptr)
            m_itemsMap[wID] = item;

        vcWidget = m_widgetsMap[wID];

        if (vcWidget != nullptr)
            vcWidget->setIsEditing(true);

        if (selectedWidgetsCount() == 1)
            emit selectedWidgetChanged();
    }
    else
    {
        m_itemsMap.remove(wID);

        vcWidget = m_widgetsMap[wID];

        if (vcWidget != nullptr)
            vcWidget->setIsEditing(false);
    }

    emit selectedWidgetsCountChanged();
}

void VirtualConsole::resetWidgetSelection()
{
    foreach (QQuickItem *widget, m_itemsMap.values())
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
        foreach (quint32 wID, m_itemsMap.keys())
        {
            VCWidget *vcWidget = m_widgetsMap[wID];
            if (vcWidget != nullptr)
            {
                if (vcWidget->caption().isEmpty())
                    names << vcWidget->typeToString(vcWidget->type());
                else
                    names << vcWidget->caption();
            }
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
        foreach (quint32 wID, m_itemsMap.keys())
            ids << wID;
    }

    return ids;
}

void VirtualConsole::moveWidget(VCWidget *widget, VCFrame *targetFrame, QPoint pos)
{
    VCFrame *sourceFrame = qobject_cast<VCFrame*>(widget->parent());

    if (sourceFrame != targetFrame)
    {
        sourceFrame->removeWidgetFromPageMap(widget);
        widget->setPage(targetFrame->currentPage());
        targetFrame->addWidgetToPageMap(widget);

        widget->setParent(targetFrame);
    }

    if (snapping())
    {
        pos.setX(qRound((qreal)pos.x() / snappingSize()) * snappingSize());
        pos.setY(qRound((qreal)pos.y() / snappingSize()) * snappingSize());
    }

    QRectF wRect = widget->geometry();
    wRect.moveTopLeft(pos);
    widget->setGeometry(wRect);

    qDebug() << "New widget geometry:" << widget->geometry();
}

void VirtualConsole::setWidgetsAlignment(VCWidget *refWidget, int alignment)
{
    if (refWidget == nullptr)
        return;

    QRectF refGeom = refWidget->geometry();

    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        QRectF wGeom = widget->geometry();

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
    while (it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setCaption(caption);
    }
}

void VirtualConsole::setWidgetsForegroundColor(QColor color)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setForegroundColor(color);
    }
}

void VirtualConsole::setWidgetsBackgroundColor(QColor color)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setBackgroundColor(color);
    }
}

void VirtualConsole::setWidgetsBackgroundImage(QString path)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setBackgroundImage(path);
    }
}

void VirtualConsole::setWidgetsFont(QFont font)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        widget->setFont(font);
    }
}

void VirtualConsole::deleteVCWidgets(QVariantList IDList)
{
    foreach (QVariant id, IDList)
    {
        quint32 wID = id.toUInt();
        VCWidget *w = widget(wID);
        if (w == nullptr)
            continue;

        /* 1- remove the widget from its parent frame page map */
        VCFrame *parentFrame = qobject_cast<VCFrame *>(w->parent());
        if (parentFrame != nullptr)
            parentFrame->removeWidgetFromPageMap(w);

        /* 2- if the widget is a frame, delete also all its children */
        if (w->type() == VCWidget::FrameWidget || w->type() == VCWidget::SoloFrameWidget)
        {
            VCFrame *frame = qobject_cast<VCFrame *>(w);
            for (VCWidget *child : frame->children(true))
            {
                Tardis::instance()->enqueueAction(Tardis::VCWidgetDelete, w->id(),
                                                  Tardis::instance()->actionToByteArray(Tardis::VCWidgetDelete, child->id()),
                                                  QVariant());
                m_widgetsMap.remove(child->id());
            }
        }

        /* 3- remove the widget from the global VC widgets map */
        VCFrame *parent = qobject_cast<VCFrame *>(w->parent());
        Tardis::instance()->enqueueAction(Tardis::VCWidgetDelete, parent->id(),
                                          Tardis::instance()->actionToByteArray(Tardis::VCWidgetDelete, w->id()),
                                          QVariant());
        m_widgetsMap.remove(wID);

        /* 4- perform the actual widget deletion */
        delete w;
    }
    m_itemsMap.clear();
}

VCWidget *VirtualConsole::selectedWidget() const
{
    if (m_itemsMap.isEmpty())
        return qobject_cast<VCWidget *>(m_pages.at(m_selectedPage));

    return m_widgetsMap[m_itemsMap.firstKey()];
}

void VirtualConsole::requestAddMatrixPopup(VCFrame *frame, QQuickItem *parent, QString widgetType, QPoint pos)
{
    QQuickItem *vcItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("virtualConsole"));
    if (vcItem == nullptr)
        return;

    QMetaObject::invokeMethod(vcItem, "requestMatrixPopup",
            Q_ARG(QVariant, QVariant::fromValue(frame)),
            Q_ARG(QVariant, QVariant::fromValue(parent)),
            Q_ARG(QVariant, widgetType),
            Q_ARG(QVariant, pos));
}

QString VirtualConsole::widgetIcon(int type)
{
    switch (type)
    {
        case VCWidget::ButtonWidget: return "qrc:/button.svg";
        case VCWidget::SliderWidget: return "qrc:/slider.svg";
        case VCWidget::XYPadWidget: return "qrc:/xypad.svg";
        case VCWidget::FrameWidget: return "qrc:/frame.svg";
        case VCWidget::SoloFrameWidget: return "qrc:/soloframe.svg";
        case VCWidget::SpeedWidget: return "qrc:/speed.svg";
        case VCWidget::CueListWidget: return "qrc:/cuelist.svg";
        case VCWidget::LabelWidget: return "qrc:/label.svg";
        case VCWidget::AudioTriggersWidget: return "qrc:/audiotriggers.svg";
        case VCWidget::AnimationWidget: return "qrc:/animation.svg";
        case VCWidget::ClockWidget: return "qrc:/clock.svg";
        default:
            qDebug() << "Unhandled widget type" << type << ". FIXME";
        break;
    }

    return "";
}


/*********************************************************************
 * Clipboard
 *********************************************************************/

void VirtualConsole::copyToClipboard()
{
    m_clipboardIDList.clear();
    for (quint32 wID : m_itemsMap.keys())
        m_clipboardIDList.append(wID);

    emit clipboardItemsCountChanged();
}

void VirtualConsole::pasteFromClipboard()
{
    VCFrame *frame = nullptr;
    QQuickItem *renderParent = nullptr;
    QPoint currPos(0, 0);

    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();

        VCWidget *widget = m_widgetsMap[it.key()];
        if (widget->type() == VCWidget::FrameWidget ||
            widget->type() == VCWidget::SoloFrameWidget)
        {
            frame = qobject_cast<VCFrame*>(widget);
            renderParent = it.value();
            // y position below the frame header
            currPos = QPoint(0, pixelDensity() * 7);
            break;
        }
    }

    // no selected frame found ? Paste on current page
    if (frame == nullptr)
    {
        frame = qobject_cast<VCFrame*>(m_pages.at(selectedPage()));
        renderParent = currentPageItem();
    }

    for (QVariant wID : m_clipboardIDList)
    {
        VCWidget *cWidget = widget(wID.toUInt());
        if (cWidget == nullptr)
            continue;

        // do not allow pasting an item into itself
        if (cWidget->id() == frame->id())
            continue;

        VCWidget *copy = cWidget->createCopy(frame);
        frame->addWidget(renderParent, copy, currPos);

        currPos.setX(currPos.x() + copy->geometry().width());
        if (currPos.x() >= frame->geometry().width())
        {
            currPos.setX(0);
            currPos.setY(currPos.y() + copy->geometry().height());
        }
    }
}

QVariantList VirtualConsole::clipboardItemsList()
{
    return m_clipboardIDList;
}

int VirtualConsole::clipboardItemsCount() const
{
    return m_clipboardIDList.count();
}

/*********************************************************************
 * External input
 *********************************************************************/

bool VirtualConsole::createAndDetectInputSource(VCWidget *widget)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == nullptr)
        return false;

    /** Create an empty input source and add it to the requested widget */
    QSharedPointer<QLCInputSource> source = QSharedPointer<QLCInputSource>(new QLCInputSource());
    widget->addInputSource(source);

    enableInputSourceAutoDetection(widget, source->id(), QLCInputSource::invalidUniverse, QLCInputSource::invalidChannel);

    return true;
}

void VirtualConsole::createAndAddInputSource(VCWidget *widget, quint32 universe, quint32 channel)
{
    QSharedPointer<QLCInputSource> source = QSharedPointer<QLCInputSource>(new QLCInputSource());
    source->setID(0); // this is a blind guess, but every widget should have a 0 control ID
    source->setUniverse(universe);
    source->setChannel(channel);
    widget->addInputSource(source);
}

bool VirtualConsole::createAndDetectInputKey(VCWidget *widget)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == nullptr)
        return false;

    widget->addKeySequence(QKeySequence());
    enableKeyAutoDetection(widget, 0, "");

    return true;
}

bool VirtualConsole::enableInputSourceAutoDetection(VCWidget *widget, quint32 id, quint32 universe, quint32 channel)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == nullptr)
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

bool VirtualConsole::enableKeyAutoDetection(VCWidget *widget, quint32 id, QString keyText)
{
    /** Do not allow multiple detections at once ! */
    if (m_inputDetectionEnabled == true || widget == nullptr)
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
    if (widget == nullptr)
        return;

    qDebug() << "Setting control ID" << id << "to widget" << widget->caption() << "sequence" << keyText;

    QKeySequence seq(keyText);

    widget->updateKeySequenceControlID(seq, id);

    /** Update also the key sequence maps in VC pages */
    for (VCPage *page : m_pages) // C++11
        page->updateKeySequenceIDInMap(seq, id, widget, true);
}

void VirtualConsole::disableAutoDetection()
{
    m_inputDetectionEnabled = false;
    m_autoDetectionWidget = nullptr;
    m_autoDetectionSource.clear();
    m_autoDetectionKey = QKeySequence();
    m_autoDetectionKeyId = UINT_MAX;
}

void VirtualConsole::deleteInputSource(VCWidget *widget, quint32 id, quint32 universe, quint32 channel)
{
    if (widget == nullptr)
        return;

    /** In case an autodetection process is running, stop it */
    disableAutoDetection();

    for (VCPage *page : m_pages) // C++11
        page->unMapInputSource(id, universe, channel, widget, true);

    widget->deleteInputSurce(id, universe, channel);
}

void VirtualConsole::deleteKeySequence(VCWidget *widget, quint32 id, QString keyText)
{
    if (widget == nullptr)
        return;

    /** In case an autodetection process is running, stop it */
    disableAutoDetection();

    QKeySequence seq(keyText);

    for (VCPage *page : m_pages) // C++11
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

        int pageIdx = 0;

        for (VCPage *page : m_pages) // C++11
        {
            if (pageIdx == selectedPage())
                page->handleKeyEvent(e, pressed);

            pageIdx++;
        }
    }
    else
    {
        Q_ASSERT(m_autoDetectionWidget != nullptr);

        /** consider only the key release */
        if (pressed == true)
            return;

        QKeySequence seq(e->key() | e->modifiers());
        qDebug() << "Got key sequence:" << seq.toString(QKeySequence::NativeText);
        m_autoDetectionWidget->updateKeySequence(m_autoDetectionKey, seq, m_autoDetectionKeyId);

        for (VCPage *page : m_pages) // C++11
            page->mapKeySequence(seq, m_autoDetectionKeyId, m_autoDetectionWidget, true);

        /** At last, disable the autodetection process */
        disableAutoDetection();
    }
}

QVariant VirtualConsole::inputChannelsModel()
{
    if (m_inputChannelsTree == nullptr)
    {
        m_inputChannelsTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_inputChannelsTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id";
        m_inputChannelsTree->setColumnNames(treeColumns);
        m_inputChannelsTree->enableSorting(false);
    }
    else
    {
        m_inputChannelsTree->clear();
    }

    for (Universe *universe : m_doc->inputOutputMap()->universes())
    {
        /* Get the patch associated to the current universe */
        InputPatch *patch = universe->inputPatch();
        if (patch == nullptr)
            continue;

        QLCInputProfile *profile = patch->profile();
        if (profile == nullptr)
            continue;

        QString nodePath = QString("%1: %2").arg(universe->name()).arg(profile->name());

        QMapIterator <quint32, QLCInputChannel*> it(profile->channels());
        while (it.hasNext() == true)
        {
            QLCInputChannel *channel = it.next().value();
            int itemID = (universe->id() << 16) | it.key();

            QVariantList chParams;
            chParams.append(QVariant::fromValue(channel)); // classRef
            chParams.append(App::ChannelDragItem); // type
            chParams.append(itemID); // id
            m_inputChannelsTree->addItem(QString("%1: %2").arg(it.key() + 1).arg(channel->name()), chParams, nodePath);
        }

        // add also the Universe node data
        QVariantList uniParams;
        uniParams.append(QVariant()); // classRef
        uniParams.append(App::UniverseDragItem); // type
        uniParams.append(universe->id()); // id

        m_inputChannelsTree->setPathData(nodePath, uniParams);
    }

    return QVariant::fromValue(m_inputChannelsTree);
}

QVariantList VirtualConsole::universeListModel()
{
    QVariantList list;

    for (Universe *universe : m_doc->inputOutputMap()->universes())
    {
        QString name = universe->name();

        /* Get the patch associated to the current universe */
        InputPatch *patch = universe->inputPatch();
        if (patch != nullptr)
        {
            QLCInputProfile *profile = patch->profile();
            if (profile != nullptr)
                name = QString("%1: %2").arg(universe->name()).arg(profile->name());
        }

        QVariantMap uniMap;
        uniMap.insert("mIcon", "");
        uniMap.insert("mLabel", name);
        uniMap.insert("mValue", universe->id());
        list.append(uniMap);
    }

    return list;
}

void VirtualConsole::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    qDebug() << "Input signal received. Universe:" << universe << ", channel:" << channel << ", value:" << value;
    if (m_inputDetectionEnabled == false)
    {
        int pageIdx = 0;

        for (VCPage *page : m_pages) // C++11
        {
            if (pageIdx == selectedPage())
            {
                page->inputValueChanged(universe, channel, value);
                break;
            }

            pageIdx++;
        }
    }
    else
    {
        /** The widget reference must be not NULL, otherwise
         *  it means something went nuts */
        Q_ASSERT(m_autoDetectionWidget != nullptr);

        m_autoDetectionWidget->updateInputSource(m_autoDetectionSource, universe, channel);

        for (VCPage *page : m_pages) // C++11
            page->mapInputSource(m_autoDetectionSource, m_autoDetectionWidget, true);

        /** At last, disable the autodetection process */
        disableAutoDetection();
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

    m_loadStatus = Loading;

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

            m_pages.at(currPageIdx)->buildKeySequenceMap();
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

    // delete the exceeding pages
    while (m_pages.count() - currPageIdx > 0)
        deletePage(m_pages.count() - 1);

    m_loadStatus = Loaded;

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
    Q_ASSERT(doc != nullptr);

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
