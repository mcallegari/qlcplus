/*
  Q Light Controller Plus
  vcframe.cpp

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
#include <QDebug>

#include "vcframe.h"
#include "vclabel.h"
#include "vcclock.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcsoloframe.h"
#include "virtualconsole.h"

#define INPUT_NEXT_PAGE_ID      0
#define INPUT_PREVIOUS_PAGE_ID  1
#define INPUT_ENABLE_ID         2
#define INPUT_COLLAPSE_ID       3

VCFrame::VCFrame(Doc *doc, VirtualConsole *vc, QObject *parent)
    : VCWidget(doc, parent)
    , m_vc(vc)
    , m_hasSoloParent(false)
    , m_showHeader(true)
    , m_showEnable(true)
    , m_isCollapsed(false)
    , m_multiPageMode(false)
    , m_currentPage(0)
    , m_totalPagesNumber(1)
    , m_pagesLoop(false)
{
    setType(VCWidget::FrameWidget);

    registerExternalControl(INPUT_NEXT_PAGE_ID, tr("Next Page"), true);
    registerExternalControl(INPUT_PREVIOUS_PAGE_ID, tr("Previous Page"), true);
    registerExternalControl(INPUT_ENABLE_ID, tr("Enable"), true);
    registerExternalControl(INPUT_COLLAPSE_ID, tr("Collapse"), true);
}

VCFrame::~VCFrame()
{
    deleteChildren();
}

QString VCFrame::defaultCaption()
{
    return tr("Frame %1").arg(id());
}

void VCFrame::render(QQuickView *view, QQuickItem *parent)
{
    if (view == NULL || parent == NULL)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCFrameItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(component->create());

    item->setParentItem(parent);
    item->setProperty("frameObj", QVariant::fromValue(this));

    if (m_pagesMap.count() > 0)
    {
        QString chName = QString("frameDropArea%1").arg(id());
        QQuickItem *childrenArea = qobject_cast<QQuickItem*>(item->findChild<QObject *>(chName));

        foreach(VCWidget *child, m_pagesMap.keys())
            child->render(view, childrenArea);
    }
}

QString VCFrame::propertiesResource() const
{
    /** If this frame is a top level frame, then it means
     *  it is a VC page, so return a specific properties resource */
    if (parent() == m_vc)
        return QString("qrc:/VCPageProperties.qml");

    return QString("qrc:/VCFrameProperties.qml");
}

void VCFrame::setHasSoloParent(bool hasSoloParent)
{
    m_hasSoloParent = hasSoloParent;
}

bool VCFrame::hasSoloParent() const
{
    return m_hasSoloParent;
}

bool VCFrame::hasChildren()
{
    return !m_pagesMap.isEmpty();
}

QList<VCWidget *> VCFrame::children(bool recursive)
{
    QList<VCWidget *> widgetsList;

    if (recursive == false)
        return m_pagesMap.keys();
    else
    {
        foreach(VCWidget *widget, m_pagesMap.keys())
        {
            widgetsList.append(widget);
            if (widget->type() == FrameWidget || widget->type() == SoloFrameWidget)
            {
                VCFrame *frame = qobject_cast<VCFrame *>(widget);
                widgetsList.append(frame->children(true));
            }
        }
    }

    return widgetsList;
}

void VCFrame::addWidget(QQuickItem *parent, QString wType, QPoint pos)
{
    qDebug() << "[VCFrame] adding widget of type:" << wType << pos;

    // reset all the drop targets, otherwise two overlapping
    // frames can get the same drop event
    m_vc->resetDropTargets(true);

    VCWidget::WidgetType type = stringToType(wType);

    switch (type)
    {
        case FrameWidget:
        {
            VCFrame *frame = new VCFrame(m_doc, m_vc, this);
            QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);
            frame->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 50, m_vc->pixelDensity() * 50));
            setupWidget(frame);
            m_vc->addWidgetToMap(frame);
            frame->render(m_vc->view(), parent);
        }
        break;
        case SoloFrameWidget:
        {
            VCSoloFrame *soloframe = new VCSoloFrame(m_doc, m_vc, this);
            QQmlEngine::setObjectOwnership(soloframe, QQmlEngine::CppOwnership);
            soloframe->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 50, m_vc->pixelDensity() * 50));
            setupWidget(soloframe);
            m_vc->addWidgetToMap(soloframe);
            soloframe->render(m_vc->view(), parent);
        }
        break;
        case ButtonWidget:
        {
            VCButton *button = new VCButton(m_doc, this);
            QQmlEngine::setObjectOwnership(button, QQmlEngine::CppOwnership);
            button->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 17, m_vc->pixelDensity() * 17));
            setupWidget(button);
            m_vc->addWidgetToMap(button);
            button->render(m_vc->view(), parent);
        }
        break;
        case LabelWidget:
        {
            VCLabel *label = new VCLabel(m_doc, this);
            QQmlEngine::setObjectOwnership(label, QQmlEngine::CppOwnership);
            label->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 25, m_vc->pixelDensity() * 8));
            setupWidget(label);
            m_vc->addWidgetToMap(label);
            label->render(m_vc->view(), parent);
        }
        break;
        case SliderWidget:
        {
            VCSlider *slider = new VCSlider(m_doc, this);
            QQmlEngine::setObjectOwnership(slider, QQmlEngine::CppOwnership);
            slider->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 10, m_vc->pixelDensity() * 35));
            setupWidget(slider);
            //slider->setDefaultFontSize(m_vc->pixelDensity() * 3.5);
            m_vc->addWidgetToMap(slider);
            slider->render(m_vc->view(), parent);
        }
        break;
        case ClockWidget:
        {
            VCClock *clock = new VCClock(m_doc, this);
            QQmlEngine::setObjectOwnership(clock, QQmlEngine::CppOwnership);
            clock->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 25, m_vc->pixelDensity() * 8));
            setupWidget(clock);
            clock->setDefaultFontSize(m_vc->pixelDensity() * 5.5);
            m_vc->addWidgetToMap(clock);
            clock->render(m_vc->view(), parent);
        }
        break;
        default:
        break;
    }
}

void VCFrame::addFunctions(QQuickItem *parent, QVariantList idsList, QPoint pos, int keyModifiers)
{
    // reset all the drop targets, otherwise two overlapping
    // frames can get the same drop event
    m_vc->resetDropTargets(true);

    //qDebug() << "modifiers:" << QString::number(keyModifiers, 16);

    QPoint currPos = pos;

    for (QVariant vID : idsList) // C++11
    {
        quint32 funcID = vID.toUInt();
        Function *func = m_doc->function(funcID);

        if (func == NULL)
            continue;

        if (keyModifiers & Qt::ShiftModifier)
        {
            VCSlider *slider = new VCSlider(m_doc, this);
            QQmlEngine::setObjectOwnership(slider, QQmlEngine::CppOwnership);
            slider->setGeometry(QRect(currPos.x(), currPos.y(), m_vc->pixelDensity() * 10, m_vc->pixelDensity() * 35));
            slider->setCaption(func->name());
            slider->setPlaybackFunction(funcID);
            setupWidget(slider);
            m_vc->addWidgetToMap(slider);
            slider->render(m_vc->view(), parent);

            currPos.setX(currPos.x() + slider->geometry().width());
            if (currPos.x() >= geometry().width())
            {
                currPos.setX(pos.x());
                currPos.setY(currPos.y() + slider->geometry().height());
            }
        }
        else
        {
            VCButton *button = new VCButton(m_doc, this);
            QQmlEngine::setObjectOwnership(button, QQmlEngine::CppOwnership);
            button->setGeometry(QRect(currPos.x(), currPos.y(), m_vc->pixelDensity() * 17, m_vc->pixelDensity() * 17));
            button->setCaption(func->name());
            button->setFunctionID(funcID);
            setupWidget(button);
            m_vc->addWidgetToMap(button);
            button->render(m_vc->view(), parent);

            currPos.setX(currPos.x() + button->geometry().width());
            if (currPos.x() >= geometry().width())
            {
                currPos.setX(pos.x());
                currPos.setY(currPos.y() + button->geometry().height());
            }
        }
    }
}

void VCFrame::deleteChildren()
{
    if (m_pagesMap.isEmpty())
        return;

    QMapIterator <VCWidget*, int> it(m_pagesMap);
    while (it.hasNext() == true)
    {
        it.next();
        VCWidget *widget = it.key();
        if(widget->type() == FrameWidget || widget->type() == SoloFrameWidget)
        {
            VCFrame *frame = qobject_cast<VCFrame*>(widget);
            frame->deleteChildren();
        }
        /* Remove the widget from the frame pages map */
        m_pagesMap.remove(widget);
        /* Remove it also from the global VC widgets map */
        m_vc->removeWidgetFromMap(widget);
        delete widget;
    }
}

void VCFrame::setupWidget(VCWidget *widget)
{
    widget->setDefaultFontSize(m_vc->pixelDensity() * 2.7);

    addWidgetToPageMap(widget);

    // if we're a normal Frame and we have a Solo Frame parent
    // then passthrough the widget functionStarting signal.
    // If we're not into a Solo Frame parent, then don't even connect
    // the signal, so each widget will know to immediately start the Function
    if (xmlTagName() == KXMLQLCVCFrame && m_hasSoloParent == true)
    {
        connect(widget, SIGNAL(functionStarting(VCWidget*,quint32,qreal)),
                this, SIGNAL(functionStarting(VCWidget*,quint32,qreal)));
    }

    // otherwise, if we're a Solo Frame, connect the widget
    // functionStarting signal to a slot to handle the event
    if (xmlTagName() == KXMLQLCVCSoloFrame)
    {
        connect(widget, SIGNAL(functionStarting(VCWidget*,quint32,qreal)),
                this, SLOT(slotFunctionStarting(VCWidget*,quint32,qreal)));
    }
}

void VCFrame::addWidgetToPageMap(VCWidget *widget)
{
    m_pagesMap.insert(widget, widget->page());
}

void VCFrame::removeWidgetFromPageMap(VCWidget *widget)
{
    m_pagesMap.remove(widget);
}


/*********************************************************************
 * Disable state
 *********************************************************************/

void VCFrame::setDisabled(bool disable)
{
    for (VCWidget *widget : children(true)) // C++11
        widget->setDisabled(disable);

    VCWidget::setDisabled(disable);
}
/*********************************************************************
 * Header
 *********************************************************************/

bool VCFrame::showHeader() const
{
    return m_showHeader;
}

void VCFrame::setShowHeader(bool showHeader)
{
    if (m_showHeader == showHeader)
        return;

    m_showHeader = showHeader;
    emit showHeaderChanged(showHeader);
}

/*********************************************************************
 * Enable button
 *********************************************************************/

bool VCFrame::showEnable() const
{
    return m_showEnable;
}

void VCFrame::setShowEnable(bool showEnable)
{
    if (m_showEnable == showEnable)
        return;

    m_showEnable = showEnable;
    emit showEnableChanged(showEnable);
}

/*********************************************************************
 * Collapsed state
 *********************************************************************/

bool VCFrame::isCollapsed() const
{
    return m_isCollapsed;
}

void VCFrame::setCollapsed(bool isCollapsed)
{
    if (m_isCollapsed == isCollapsed)
        return;

    m_isCollapsed = isCollapsed;
    emit collapsedChanged(isCollapsed);
    setDocModified();
}

/*********************************************************************
 * Multi page mode
 *********************************************************************/

bool VCFrame::multiPageMode() const
{
    return m_multiPageMode;
}

void VCFrame::setMultiPageMode(bool multiPageMode)
{
    if (m_multiPageMode == multiPageMode)
        return;

    m_multiPageMode = multiPageMode;
    emit multiPageModeChanged(multiPageMode);
}

void VCFrame::setTotalPagesNumber(int num)
{
    m_totalPagesNumber = num;
}

int VCFrame::totalPagesNumber() const
{
    return m_totalPagesNumber;
}

int VCFrame::currentPage() const
{
    if (m_multiPageMode == false)
        return 0;
    return m_currentPage;
}

void VCFrame::setCurrentPage(int pageNum)
{
    if (pageNum < 0 || pageNum >= m_totalPagesNumber)
        return;

    m_currentPage = pageNum;

    QMapIterator <VCWidget*, int> it(m_pagesMap);
    while (it.hasNext() == true)
    {
        it.next();
        int page = it.value();
        VCWidget *widget = it.key();
        if (page == m_currentPage)
        {
            widget->setDisabled(false);
            widget->setVisible(true);
            //widget->updateFeedback();
        }
        else
        {
            widget->setDisabled(true);
            widget->setVisible(false);
        }
    }
    setDocModified();
    emit currentPageChanged(m_currentPage);
}

void VCFrame::setPagesLoop(bool pagesLoop)
{
    m_pagesLoop = pagesLoop;
}

bool VCFrame::pagesLoop() const
{
    return m_pagesLoop;
}

void VCFrame::gotoPreviousPage()
{
    if (m_pagesLoop && m_currentPage == 0)
        setCurrentPage(m_totalPagesNumber - 1);
    else
        setCurrentPage(m_currentPage - 1);

    //sendFeedback(m_currentPage, previousPageInputSourceId);
}

void VCFrame::gotoNextPage()
{
    if (m_pagesLoop && m_currentPage == m_totalPagesNumber - 1)
        setCurrentPage(0);
    else
        setCurrentPage(m_currentPage + 1);

    //sendFeedback(m_currentPage, nextPageInputSourceId);
}

/*********************************************************************
 * Widget Function
 *********************************************************************/

void VCFrame::slotFunctionStarting(VCWidget *widget, quint32 fid, qreal fIntensity)
{
    Q_UNUSED(widget)
    Q_UNUSED(fid)
    Q_UNUSED(fIntensity)

    if (xmlTagName() == KXMLQLCVCFrame)
        qDebug() << "[VCFrame] ERROR ! This should never happen !";
}

void VCFrame::slotInputValueChanged(quint8 id, uchar value)
{
    if (value != 255)
        return;

    switch(id)
    {
        case INPUT_NEXT_PAGE_ID:
            gotoNextPage();
        break;
        case INPUT_PREVIOUS_PAGE_ID:
            gotoPreviousPage();
        break;
        case INPUT_ENABLE_ID:
            // TODO
        break;
        case INPUT_COLLAPSE_ID:
            setCollapsed(!isCollapsed());
        break;
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

QString VCFrame::xmlTagName() const
{
    return KXMLQLCVCFrame;
}

bool VCFrame::loadXML(QXmlStreamReader &root)
{
    if (root.name() != xmlTagName())
    {
        qWarning() << Q_FUNC_INFO << "Frame node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    int currentPage = 0;

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            /* Frame geometry (visibility is ignored) */
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            /* Frame appearance */
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCFrameShowHeader)
        {
            if (root.readElementText() == KXMLQLCTrue)
                setShowHeader(true);
            else
                setShowHeader(false);
        }
        else if (root.name() == KXMLQLCVCFrameIsCollapsed)
        {
            /* Collapsed */
            if (root.readElementText() == KXMLQLCTrue)
                setCollapsed(true);
        }
        else if (root.name() == KXMLQLCVCFrameShowEnableButton)
        {
            if (root.readElementText() == KXMLQLCTrue)
                setShowEnable(true);
            else
                setShowEnable(false);
        }
        else if (root.name() == KXMLQLCVCFrameMultipage)
        {
            setMultiPageMode(true);
            QXmlStreamAttributes attrs = root.attributes();
            if (attrs.hasAttribute(KXMLQLCVCFramePagesNumber))
                setTotalPagesNumber(attrs.value(KXMLQLCVCFramePagesNumber).toInt());

            if(attrs.hasAttribute(KXMLQLCVCFrameCurrentPage))
                currentPage = attrs.value(KXMLQLCVCFrameCurrentPage).toInt();
            root.skipCurrentElement();
        }

        /** ***************** children widgets *************************** */

        else if (root.name() == KXMLQLCVCFrame)
        {
            /* Create a new frame into its parent */
            VCFrame* frame = new VCFrame(m_doc, m_vc, this);

            // if we're a Solo Frame or we have a Solo Frame parent, set
            // the new frame accordingly
            if (xmlTagName() == KXMLQLCVCSoloFrame || m_hasSoloParent == true)
                frame->setHasSoloParent(true);

            if (frame->loadXML(root) == false)
                delete frame;
            else
            {
                QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);
                setupWidget(frame);
                m_vc->addWidgetToMap(frame);
            }
        }
        else if (root.name() == KXMLQLCVCSoloFrame)
        {
            /* Create a new frame into its parent */
            VCSoloFrame* soloframe = new VCSoloFrame(m_doc, m_vc, this);
            if (soloframe->loadXML(root) == false)
                delete soloframe;
            else
            {
                QQmlEngine::setObjectOwnership(soloframe, QQmlEngine::CppOwnership);
                setupWidget(soloframe);
                m_vc->addWidgetToMap(soloframe);
            }
        }
        else if (root.name() == KXMLQLCVCButton)
        {
            /* Create a new button into its parent */
            VCButton* button = new VCButton(m_doc, this);
            if (button->loadXML(root) == false)
                delete button;
            else
            {
                QQmlEngine::setObjectOwnership(button, QQmlEngine::CppOwnership);
                setupWidget(button);
                m_vc->addWidgetToMap(button);
            }
        }
        else if (root.name() == KXMLQLCVCLabel)
        {
            /* Create a new label into its parent */
            VCLabel* label = new VCLabel(m_doc, this);
            if (label->loadXML(root) == false)
                delete label;
            else
            {
                QQmlEngine::setObjectOwnership(label, QQmlEngine::CppOwnership);
                setupWidget(label);
                m_vc->addWidgetToMap(label);
            }
        }
        else if (root.name() == KXMLQLCVCSlider)
        {
            /* Create a new slider into its parent */
            VCSlider* slider = new VCSlider(m_doc, this);
            if (slider->loadXML(root) == false)
                delete slider;
            else
            {
                QQmlEngine::setObjectOwnership(slider, QQmlEngine::CppOwnership);
                setupWidget(slider);
                m_vc->addWidgetToMap(slider);
            }
        }
        else if (root.name() == KXMLQLCVCClock)
        {
            /* Create a new clock into its parent */
            VCClock* clock = new VCClock(m_doc, this);
            if (clock->loadXML(root) == false)
                delete clock;
            else
            {
                QQmlEngine::setObjectOwnership(clock, QQmlEngine::CppOwnership);
                setupWidget(clock);
                m_vc->addWidgetToMap(clock);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown frame tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    if (multiPageMode() == true)
        setCurrentPage(currentPage);

    return true;
}

bool VCFrame::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC Frame entry */
    doc->writeStartElement(xmlTagName());

    saveXMLCommon(doc);

    /* Save appearance */
    saveXMLAppearance(doc);

    /* Save widget proportions only for child frames */
    saveXMLWindowState(doc);

    /* Allow resize */
    doc->writeTextElement(KXMLQLCVCFrameAllowResize, allowResize() ? KXMLQLCTrue : KXMLQLCFalse);

    /* ShowHeader */
    doc->writeTextElement(KXMLQLCVCFrameShowHeader, showHeader() ? KXMLQLCTrue : KXMLQLCFalse);

    /* ShowEnableButton */
    doc->writeTextElement(KXMLQLCVCFrameShowEnableButton, showEnable() ? KXMLQLCTrue : KXMLQLCFalse);

#if 0 // TODO
    /* Solo frame mixing */
    if (this->type() == SoloFrameWidget)
    {
        if (reinterpret_cast<VCSoloFrame*>(this)->soloframeMixing())
            doc->writeTextElement(KXMLQLCVCSoloFrameMixing, KXMLQLCTrue);
        else
            doc->writeTextElement(KXMLQLCVCSoloFrameMixing, KXMLQLCFalse);
    }
#endif
    /* Collapsed */
    doc->writeTextElement(KXMLQLCVCFrameIsCollapsed, isCollapsed() ? KXMLQLCTrue : KXMLQLCFalse);

    /* Disabled */
    doc->writeTextElement(KXMLQLCVCFrameIsDisabled, isDisabled() ? KXMLQLCTrue : KXMLQLCFalse);

#if 0 // TODO
    /* Enable control */
    QString keySeq = m_enableKeySequence.toString();
    QSharedPointer<QLCInputSource> enableSrc = inputSource(enableInputSourceId);

    if (keySeq.isEmpty() == false || (!enableSrc.isNull() && enableSrc->isValid()))
    {
        doc->writeStartElement(KXMLQLCVCFrameEnableSource);
        if (keySeq.isEmpty() == false)
            doc->writeTextElement(KXMLQLCVCWidgetKey, keySeq);
        saveXMLInput(doc, enableSrc);
        doc->writeEndElement();
    }
#endif
    /* Multipage mode */
    if (multiPageMode() == true)
    {
        doc->writeStartElement(KXMLQLCVCFrameMultipage);
        doc->writeAttribute(KXMLQLCVCFramePagesNumber, QString::number(totalPagesNumber()));
        doc->writeAttribute(KXMLQLCVCFrameCurrentPage, QString::number(currentPage()));
        doc->writeEndElement();
#if 0 // TODO
        /* Next page */
        keySeq = m_nextPageKeySequence.toString();
        QSharedPointer<QLCInputSource> nextSrc = inputSource(nextPageInputSourceId);

        if (keySeq.isEmpty() == false || (!nextSrc.isNull() && nextSrc->isValid()))
        {
            doc->writeStartElement(KXMLQLCVCFrameNext);
            if (keySeq.isEmpty() == false)
                doc->writeTextElement(KXMLQLCVCWidgetKey, keySeq);
            saveXMLInput(doc, nextSrc);
            doc->writeEndElement();
        }

        /* Previous page */
        keySeq = m_previousPageKeySequence.toString();
        QSharedPointer<QLCInputSource> prevSrc = inputSource(previousPageInputSourceId);

        if (keySeq.isEmpty() == false || (!prevSrc.isNull() && prevSrc->isValid()))
        {
            doc->writeStartElement(KXMLQLCVCFramePrevious);
            if (keySeq.isEmpty() == false)
                doc->writeTextElement(KXMLQLCVCWidgetKey, keySeq);
            saveXMLInput(doc, prevSrc);
            doc->writeEndElement();
        }
#endif
        /* Pages Loop */
        doc->writeTextElement(KXMLQLCVCFramePagesLoop, m_pagesLoop ? KXMLQLCTrue : KXMLQLCFalse);
    }

    /* Save children */
    foreach(VCWidget *child, children(false))
        child->saveXML(doc);

    /* End the <Frame> tag */
    doc->writeEndElement();

    return true;
}
