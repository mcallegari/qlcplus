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

#include "tardis.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcclock.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vccuelist.h"
#include "vcsoloframe.h"
#include "simplecrypt.h"
#include "vcanimation.h"
#include "vcaudiotrigger.h"
#include "vcxypad.h"
#include "vcspeeddial.h"
#include "virtualconsole.h"

#define INPUT_NEXT_PAGE_ID      0
#define INPUT_PREVIOUS_PAGE_ID  1
#define INPUT_ENABLE_ID         2
#define INPUT_COLLAPSE_ID       3
#define INPUT_SHORTCUT_BASE_ID  20

static const quint64 encKey = 0x5131632B5067334B; // this is "Q1c+Pg3K"

VCFrame::VCFrame(Doc *doc, VirtualConsole *vc, QObject *parent)
    : VCWidget(doc, parent)
    , m_vc(vc)
    , m_showHeader(true)
    , m_showEnable(true)
    , m_isCollapsed(false)
    , m_multiPageMode(false)
    , m_currentPage(0)
    , m_pagesLoop(false)
    , m_PIN(0)
    , m_validatedPIN(false)
{
    setType(VCWidget::FrameWidget);
    setTotalPagesNumber(1);

    registerExternalControl(INPUT_NEXT_PAGE_ID, tr("Next Page"), true);
    registerExternalControl(INPUT_PREVIOUS_PAGE_ID, tr("Previous Page"), true);
    registerExternalControl(INPUT_ENABLE_ID, tr("Enable"), true);
    registerExternalControl(INPUT_COLLAPSE_ID, tr("Collapse"), true);
}

VCFrame::~VCFrame()
{
    deleteChildren();

    if (m_item)
        delete m_item;
}

QString VCFrame::defaultCaption()
{
    return tr("Frame %1").arg(id() + 1);
}

void VCFrame::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    setDefaultFontSize(pixelDensity * 3.5);
}

void VCFrame::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCFrameItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("frameObj", QVariant::fromValue(this));

    if (m_pagesMap.count() > 0)
    {
        QString chName = QString("frameDropArea%1").arg(id());
        QQuickItem *childrenArea = qobject_cast<QQuickItem*>(m_item->findChild<QObject *>(chName));

        foreach (VCWidget *child, m_pagesMap.keys())
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

VCWidget *VCFrame::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCFrame *frame = new VCFrame(m_doc, m_vc, parent);
    QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);

    if (frame->copyFrom(this) == false)
    {
        delete frame;
        frame = nullptr;
    }

    return frame;
}

bool VCFrame::copyFrom(const VCWidget *widget)
{
    const VCFrame *frame = qobject_cast<const VCFrame*> (widget);
    if (frame == nullptr)
        return false;

    setShowHeader(frame->showHeader());
    setShowEnable(frame->showEnable());

    setMultiPageMode(frame->multiPageMode());
    setTotalPagesNumber(frame->totalPagesNumber());
    setPagesLoop(frame->pagesLoop());

    QListIterator <VCWidget*> it(widget->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget *child = it.next();
        VCWidget *childCopy = nullptr;

        /* findChildren() is recursive, so the list contains all
           possible child widgets below this frame. Each frame must
           save only its direct children to preserve hierarchy, so
           save only such widgets that have this widget as their
           direct parent. */
        if (child->parent() == widget)
        {
            childCopy = child->createCopy(this);
            m_vc->addWidgetToMap(childCopy);

            qDebug() << "Child copy in parent:" << childCopy->caption() << ", page:" << childCopy->page();
        }

        if (childCopy != nullptr)
        {
            addWidgetToPageMap(childCopy);
            checkSubmasterConnection(childCopy);
        }
    }

    if (multiPageMode())
        setPage(frame->currentPage());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
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
        foreach (VCWidget *widget, m_pagesMap.keys())
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
    VCWidget::WidgetType type = stringToType(wType);

    qDebug() << "[VCFrame] adding widget of type:" << wType << pos;

    if (m_vc->snapping())
    {
        pos.setX(qRound((qreal)pos.x() / m_vc->snappingSize()) * m_vc->snappingSize());
        pos.setY(qRound((qreal)pos.y() / m_vc->snappingSize()) * m_vc->snappingSize());
    }

    switch (type)
    {
        case FrameWidget:
        {
            VCFrame *frame = new VCFrame(m_doc, m_vc, this);
            QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(frame);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, frame->id()));
            frame->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 50, m_vc->pixelDensity() * 50));
            setupWidget(frame, currentPage());
            frame->render(m_vc->view(), parent);
        }
        break;
        case SoloFrameWidget:
        {
            VCSoloFrame *soloframe = new VCSoloFrame(m_doc, m_vc, this);
            QQmlEngine::setObjectOwnership(soloframe, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(soloframe);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, soloframe->id()));
            soloframe->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 50, m_vc->pixelDensity() * 50));
            setupWidget(soloframe, currentPage());
            soloframe->render(m_vc->view(), parent);
        }
        break;
        case ButtonWidget:
        {
            VCButton *button = new VCButton(m_doc, this);
            QQmlEngine::setObjectOwnership(button, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(button);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, button->id()));
            button->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 17, m_vc->pixelDensity() * 17));
            setupWidget(button, currentPage());
            button->render(m_vc->view(), parent);
        }
        break;
        case LabelWidget:
        {
            VCLabel *label = new VCLabel(m_doc, this);
            QQmlEngine::setObjectOwnership(label, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(label);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, label->id()));
            label->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 25, m_vc->pixelDensity() * 8));
            setupWidget(label, currentPage());
            label->render(m_vc->view(), parent);
        }
        break;
        case SliderWidget:
        {
            VCSlider *slider = new VCSlider(m_doc, this);
            QQmlEngine::setObjectOwnership(slider, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(slider);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, slider->id()));
            if (wType == "Knob")
            {
                slider->setWidgetStyle(VCSlider::WKnob);
                slider->setCaption(slider->defaultCaption());
                slider->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 15, m_vc->pixelDensity() * 22));
            }
            else
                slider->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 15, m_vc->pixelDensity() * 40));
            setupWidget(slider, currentPage());
            slider->render(m_vc->view(), parent);
        }
        break;
        case AnimationWidget:
        {
            VCAnimation *animation = new VCAnimation(m_doc, this);
            QQmlEngine::setObjectOwnership(animation, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(animation);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, animation->id()));
            animation->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 60, m_vc->pixelDensity() * 45));
            setupWidget(animation, currentPage());
            animation->render(m_vc->view(), parent);
        }
        break;
        case AudioTriggersWidget:
        {
            VCAudioTrigger *audioTrigger = new VCAudioTrigger(m_doc, this);
            QQmlEngine::setObjectOwnership(audioTrigger, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(audioTrigger);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, audioTrigger->id()));
            audioTrigger->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 25, m_vc->pixelDensity() * 8));
            setupWidget(audioTrigger, currentPage());
            audioTrigger->render(m_vc->view(), parent);
        }
        break;
        case XYPadWidget:
        {
            VCXYPad *xyPad = new VCXYPad(m_doc, this);
            QQmlEngine::setObjectOwnership(xyPad, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(xyPad);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, xyPad->id()));
            xyPad->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 25, m_vc->pixelDensity() * 8));
            setupWidget(xyPad, currentPage());
            xyPad->render(m_vc->view(), parent);
        }
        break;
        case SpeedWidget:
        {
            VCSpeedDial *speed = new VCSpeedDial(m_doc, this);
            QQmlEngine::setObjectOwnership(speed, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(speed);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, speed->id()));
            speed->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 80, m_vc->pixelDensity() * 50));
            setupWidget(speed, currentPage());
            speed->render(m_vc->view(), parent);
        }
        break;
        case ClockWidget:
        {
            VCClock *clock = new VCClock(m_doc, this);
            QQmlEngine::setObjectOwnership(clock, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(clock);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, clock->id()));
            clock->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 25, m_vc->pixelDensity() * 8));
            setupWidget(clock, currentPage());
            clock->render(m_vc->view(), parent);
        }
        break;
        case CueListWidget:
        {
            VCCueList *cuelist = new VCCueList(m_doc, this);
            QQmlEngine::setObjectOwnership(cuelist, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(cuelist);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, cuelist->id()));
            cuelist->setGeometry(QRect(pos.x(), pos.y(), m_vc->pixelDensity() * 80, m_vc->pixelDensity() * 50));
            setupWidget(cuelist, currentPage());
            cuelist->render(m_vc->view(), parent);
        }
        break;
        default:
        break;
    }
}

void VCFrame::addWidget(QQuickItem *parent, VCWidget *widget, QPoint pos)
{
    if (m_vc->snapping())
    {
        pos.setX(qRound((qreal)pos.x() / m_vc->snappingSize()) * m_vc->snappingSize());
        pos.setY(qRound((qreal)pos.y() / m_vc->snappingSize()) * m_vc->snappingSize());
    }

    QQmlEngine::setObjectOwnership(widget, QQmlEngine::CppOwnership);
    m_vc->addWidgetToMap(widget);
    Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                      Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, widget->id()));
    widget->setGeometry(QRect(pos.x(), pos.y(), widget->geometry().width(), widget->geometry().height()));
    addWidgetToPageMap(widget);
    checkSubmasterConnection(widget);
    widget->render(m_vc->view(), parent);
}

void VCFrame::addWidgetMatrix(QQuickItem *parent, QString matrixType, QPoint pos, QSize matrixSize, QSize widgetSize, bool soloFrame)
{
    VCFrame *frame;
    int totalWidth = (matrixSize.width() * widgetSize.width()) + (m_vc->pixelDensity() * 2);
    int totalHeight = (matrixSize.height() * widgetSize.height()) + (m_vc->pixelDensity() * 2);
    int yPos = m_vc->pixelDensity();

    qDebug() << "Matrix size" << matrixSize << "widget size" << widgetSize;
    qDebug() << "Frame total width" << totalWidth << ", height" << totalHeight;

    if (soloFrame)
    {
        VCSoloFrame *solo = new VCSoloFrame(m_doc, m_vc, this);
        frame = qobject_cast<VCFrame *>(solo);
    }
    else
    {
        frame = new VCFrame(m_doc, m_vc, this);
    }

    if (m_vc->snapping())
    {
        pos.setX(qRound((qreal)pos.x() / m_vc->snappingSize()) * m_vc->snappingSize());
        pos.setY(qRound((qreal)pos.y() / m_vc->snappingSize()) * m_vc->snappingSize());
    }

    QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);
    m_vc->addWidgetToMap(frame);
    Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                      Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, frame->id()));
    frame->setGeometry(QRect(pos.x(), pos.y(), totalWidth, totalHeight));
    frame->setShowHeader(false);
    setupWidget(frame, currentPage());

    for (int row = 0; row < matrixSize.height(); row++)
    {
        int xPos = m_vc->pixelDensity();

        for (int col = 0; col < matrixSize.width(); col++)
        {
            frame->addWidget(nullptr, matrixType == "buttonmatrix" ? typeToString(ButtonWidget) : typeToString(SliderWidget), QPoint(xPos, yPos));
            xPos += widgetSize.width();
        }
        yPos += widgetSize.height();
    }

    frame->render(m_vc->view(), parent);
}

void VCFrame::addWidgetsFromClipboard(QQuickItem *parent, QVariantList idsList, QPoint pos)
{
    QPoint currPos = pos;

    for (QVariant wID : idsList)
    {
        VCWidget *widget = m_vc->widget(wID.toUInt());
        if (widget == nullptr)
            continue;

        VCWidget *copy = widget->createCopy(this);
        addWidget(parent, copy, currPos);

        currPos.setX(currPos.x() + copy->geometry().width());
        if (currPos.x() >= geometry().width())
        {
            currPos.setX(pos.x());
            currPos.setY(currPos.y() + copy->geometry().height());
        }
    }
}

void VCFrame::addFunctions(QQuickItem *parent, QVariantList idsList, QPoint pos, int keyModifiers)
{
    //qDebug() << "modifiers:" << QString::number(keyModifiers, 16);

    if (m_vc->snapping())
    {
        pos.setX(qRound((qreal)pos.x() / m_vc->snappingSize()) * m_vc->snappingSize());
        pos.setY(qRound((qreal)pos.y() / m_vc->snappingSize()) * m_vc->snappingSize());
    }

    QPoint currPos = pos;

    for (QVariant vID : idsList) // C++11
    {
        quint32 funcID = vID.toUInt();
        Function *func = m_doc->function(funcID);

        if (func == nullptr)
            continue;

        if (keyModifiers & Qt::ShiftModifier)
        {
            VCSlider *slider = new VCSlider(m_doc, this);
            QQmlEngine::setObjectOwnership(slider, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(slider);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, slider->id()));
            slider->setGeometry(QRect(currPos.x(), currPos.y(), m_vc->pixelDensity() * 15, m_vc->pixelDensity() * 40));
            slider->setCaption(func->name());
            slider->setControlledFunction(funcID);
            setupWidget(slider, currentPage());

            slider->render(m_vc->view(), parent);

            currPos.setX(currPos.x() + slider->geometry().width());
            if (currPos.x() >= geometry().width())
            {
                currPos.setX(pos.x());
                currPos.setY(currPos.y() + slider->geometry().height());
            }
        }
        else if (keyModifiers & Qt::ControlModifier)
        {
            Function *f = m_doc->function(funcID);
            if (f->type() != Function::ChaserType)
                return;

            VCCueList *cuelist = new VCCueList(m_doc, this);
            QQmlEngine::setObjectOwnership(cuelist, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(cuelist);

            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, cuelist->id()));
            cuelist->setGeometry(QRect(currPos.x(), currPos.y(), m_vc->pixelDensity() * 80, m_vc->pixelDensity() * 50));
            cuelist->setCaption(func->name());
            cuelist->setChaserID(funcID);
            setupWidget(cuelist, currentPage());

            cuelist->render(m_vc->view(), parent);

            currPos.setX(currPos.x() + cuelist->geometry().width());
            if (currPos.x() >= geometry().width())
            {
                currPos.setX(pos.x());
                currPos.setY(currPos.y() + cuelist->geometry().height());
            }
        }
        else
        {
            VCButton *button = new VCButton(m_doc, this);
            QQmlEngine::setObjectOwnership(button, QQmlEngine::CppOwnership);
            m_vc->addWidgetToMap(button);
            Tardis::instance()->enqueueAction(Tardis::VCWidgetCreate, this->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::VCWidgetCreate, button->id()));
            button->setGeometry(QRect(currPos.x(), currPos.y(), m_vc->pixelDensity() * 17, m_vc->pixelDensity() * 17));
            button->setCaption(func->name());
            button->setFunctionID(funcID);
            setupWidget(button, currentPage());

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
        if (widget->type() == FrameWidget || widget->type() == SoloFrameWidget)
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

void VCFrame::setupWidget(VCWidget *widget, int page)
{
    if (m_vc->loadStatus() != VirtualConsole::Loading)
        widget->setupLookAndFeel(m_vc->pixelDensity(), page);

    addWidgetToPageMap(widget);
    checkSubmasterConnection(widget);
}

void VCFrame::checkSubmasterConnection(VCWidget *widget)
{
    if (widget->type() == VCWidget::SliderWidget)
    {
        VCSlider *slider = qobject_cast<VCSlider *>(widget);

        // always connect a slider in case it emits a submaster signal
        connect(slider, SIGNAL(submasterValueChanged(qreal)),
                this, SLOT(slotSubmasterValueChanged(qreal)));
    }
}

void VCFrame::addWidgetToPageMap(VCWidget *widget)
{
    m_pagesMap.insert(widget, widget->page());

    // if we're a normal Frame and we have a Solo Frame parent
    // then passthrough the widget functionStarting signal.
    // If we're not into a Solo Frame parent, then don't even connect
    // the signal, so each widget will know to immediately start the Function
    if (xmlTagName() == KXMLQLCVCFrame && hasSoloParent() == true)
    {
        qDebug() << "------ FRAME ----- connect";
        connect(widget, &VCWidget::functionStarting, this, &VCWidget::functionStarting);
    }

    // otherwise, if we're a Solo Frame, connect the widget
    // functionStarting signal to a slot to handle the event
    if (xmlTagName() == KXMLQLCVCSoloFrame)
    {
        qDebug() << "------ SOLO FRAME ----- connect";
        connect(widget, &VCWidget::functionStarting, this, &VCFrame::slotFunctionStarting);
    }
}

void VCFrame::removeWidgetFromPageMap(VCWidget *widget)
{
    m_pagesMap.remove(widget);

    // disconnect function start event. See addWidgetToPageMap
    if (xmlTagName() == KXMLQLCVCFrame && hasSoloParent() == true)
    {
        qDebug() << "------ FRAME --//--- disconnect";
        disconnect(widget, &VCWidget::functionStarting, this, &VCWidget::functionStarting);
    }

    if (xmlTagName() == KXMLQLCVCSoloFrame)
    {
        qDebug() << "------ SOLO FRAME --//--- disconnect";
        disconnect(widget, &VCWidget::functionStarting, this, &VCFrame::slotFunctionStarting);
    }
}


/*********************************************************************
 * Disable state
 *********************************************************************/

void VCFrame::setDisabled(bool disable)
{
    for (VCWidget *widget : children(true)) // C++11
        widget->setDisabled(disable);

    VCWidget::setDisabled(disable);

    updateFeedback();
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
    if (m_totalPagesNumber == num)
        return;

    if (num < m_totalPagesNumber)
    {
        for (int i = m_totalPagesNumber - 1; i > num; i--)
        {
            m_pageLabels.remove(i);
            unregisterExternalControl(INPUT_SHORTCUT_BASE_ID + i);
        }
    }
    else
    {
        for (int i = m_totalPagesNumber; i < num; i++)
        {
            QString name = tr("Page %1").arg(i + 1);
            m_pageLabels.insert(i, name);
            registerExternalControl(INPUT_SHORTCUT_BASE_ID + i, name, true);
        }
    }

    m_totalPagesNumber = num;
    setDocModified();
    emit totalPagesNumberChanged(num);
    emit pageLabelsChanged();
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
        }
        else
        {
            widget->setDisabled(true);
            widget->setVisible(false);
        }
    }

    updateFeedback();

    setDocModified();
    emit currentPageChanged(m_currentPage);
}

void VCFrame::setPagesLoop(bool pagesLoop)
{
    if (m_pagesLoop == pagesLoop)
        return;

    m_pagesLoop = pagesLoop;
    setDocModified();
    emit pagesLoopChanged(pagesLoop);
}

bool VCFrame::pagesLoop() const
{
    return m_pagesLoop;
}

QStringList VCFrame::pageLabels()
{
    return m_pageLabels.values();
}

void VCFrame::setShortcutName(int pageIndex, QString name)
{
    m_pageLabels[pageIndex] = name;
    setDocModified();

    emit pageLabelsChanged();
}

void VCFrame::gotoPreviousPage()
{
    if (m_pagesLoop && m_currentPage == 0)
        setCurrentPage(m_totalPagesNumber - 1);
    else
        setCurrentPage(m_currentPage - 1);

    sendFeedback(m_currentPage, INPUT_PREVIOUS_PAGE_ID);
}

void VCFrame::gotoNextPage()
{
    if (m_pagesLoop && m_currentPage == m_totalPagesNumber - 1)
        setCurrentPage(0);
    else
        setCurrentPage(m_currentPage + 1);

    sendFeedback(m_currentPage, INPUT_NEXT_PAGE_ID);
}

void VCFrame::gotoPage(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= m_totalPagesNumber)
        return;

    setCurrentPage(pageIndex);
}

void VCFrame::cloneFirstPage()
{
    if (m_totalPagesNumber == 1)
        return;

    for (int pg = 1; pg < totalPagesNumber(); pg++)
    {
        QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
        while (it.hasNext() == true)
        {
            VCWidget* child = it.next();
            if (child->page() == 0 && child->parent() == this)
            {
                VCWidget *newWidget = child->createCopy(this);
                m_vc->addWidgetToMap(newWidget);
                newWidget->setPage(pg);
                setupWidget(newWidget, pg);
                newWidget->render(m_vc->view(), m_item);
            }
        }
    }
}

/*********************************************************************
 * PIN
 *********************************************************************/

int VCFrame::PIN() const
{
    return m_PIN;
}

void VCFrame::setPIN(int newPIN)
{
    if (newPIN == m_PIN)
        return;

    m_PIN = newPIN;
    setDocModified();
    emit PINChanged(newPIN);
}

void VCFrame::validatePIN()
{
    m_validatedPIN = true;
}

bool VCFrame::requirePIN() const
{
    if (m_PIN == 0 || m_validatedPIN == true)
        return false;

    return true;
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
        qDebug() << "[VCFrame] ERROR ! This should never happen!";
}

/*********************************************************************
 * Submasters
 *********************************************************************/

void VCFrame::slotSubmasterValueChanged(qreal value)
{
    qDebug() << Q_FUNC_INFO << "val:" << value;
    VCSlider *submaster = qobject_cast<VCSlider *>(sender());
    QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        if (child->parent() == this && child != submaster)
            child->adjustIntensity(value);
    }
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCFrame::updateFeedback()
{
    if (isDisabled())
    {
        // temporarily revert the disabled state otherwise
        // this feedback will never go through
        m_isDisabled = false;
        sendFeedback(0, INPUT_ENABLE_ID, VCWidget::LowerValue);
        m_isDisabled = true;
    }
    else
    {
        sendFeedback(UCHAR_MAX, INPUT_ENABLE_ID, VCWidget::UpperValue);
    }

    QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        if (child->parent() == this && child->page() == currentPage())
            child->updateFeedback();
    }

    for (int &pIdx : m_pageLabels.keys())
    {
        if (pIdx == m_currentPage)
            sendFeedback(UCHAR_MAX, INPUT_SHORTCUT_BASE_ID + pIdx, VCWidget::UpperValue);
        else
            sendFeedback(0, INPUT_SHORTCUT_BASE_ID + pIdx, VCWidget::LowerValue);
    }
}

void VCFrame::slotInputValueChanged(quint8 id, uchar value)
{
    if (value != UCHAR_MAX)
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
            setDisabled(isDisabled() ? false : true);
        break;
        case INPUT_COLLAPSE_ID:
            setCollapsed(!isCollapsed());
        break;
        default:
            if (id < INPUT_SHORTCUT_BASE_ID || id > INPUT_SHORTCUT_BASE_ID + m_totalPagesNumber)
                break;
            setCurrentPage(id - INPUT_SHORTCUT_BASE_ID);
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

bool VCFrame::loadWidgetXML(QXmlStreamReader &root, bool render)
{
    if (root.name() == KXMLQLCVCFrame)
    {
        /* Create a new frame into its parent */
        VCFrame* frame = new VCFrame(m_doc, m_vc, this);

        if (frame->loadXML(root) == false)
            delete frame;
        else
        {
            QQmlEngine::setObjectOwnership(frame, QQmlEngine::CppOwnership);
            setupWidget(frame, frame->page());
            m_vc->addWidgetToMap(frame);
            if (render && m_item)
                frame->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCSoloFrame)
    {
        /* Create a new frame into its parent */
        VCSoloFrame *soloframe = new VCSoloFrame(m_doc, m_vc, this);
        if (soloframe->loadXML(root) == false)
            delete soloframe;
        else
        {
            QQmlEngine::setObjectOwnership(soloframe, QQmlEngine::CppOwnership);
            setupWidget(soloframe, soloframe->page());
            m_vc->addWidgetToMap(soloframe);
            if (render && m_item)
                soloframe->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCButton)
    {
        /* Create a new button into its parent */
        VCButton *button = new VCButton(m_doc, this);
        if (button->loadXML(root) == false)
            delete button;
        else
        {
            QQmlEngine::setObjectOwnership(button, QQmlEngine::CppOwnership);
            setupWidget(button, button->page());
            m_vc->addWidgetToMap(button);
            if (render && m_item)
                button->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCLabel)
    {
        /* Create a new label into its parent */
        VCLabel *label = new VCLabel(m_doc, this);
        if (label->loadXML(root) == false)
            delete label;
        else
        {
            QQmlEngine::setObjectOwnership(label, QQmlEngine::CppOwnership);
            setupWidget(label, label->page());
            m_vc->addWidgetToMap(label);
            if (render && m_item)
                label->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCSlider)
    {
        /* Create a new slider into its parent */
        VCSlider *slider = new VCSlider(m_doc, this);
        if (slider->loadXML(root) == false)
            delete slider;
        else
        {
            QQmlEngine::setObjectOwnership(slider, QQmlEngine::CppOwnership);
            setupWidget(slider, slider->page());
            m_vc->addWidgetToMap(slider);
            if (render && m_item)
                slider->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCAnimation)
    {
        /* Create a new clock into its parent */
        VCAnimation *animation = new VCAnimation(m_doc, this);
        if (animation->loadXML(root) == false)
            delete animation;
        else
        {
            QQmlEngine::setObjectOwnership(animation, QQmlEngine::CppOwnership);
            setupWidget(animation, animation->page());
            m_vc->addWidgetToMap(animation);
            if (render && m_item)
                animation->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCAudioTriggers)
    {
        /* Create a new clock into its parent */
        VCAudioTrigger *audioTrigger = new VCAudioTrigger(m_doc, this);
        if (audioTrigger->loadXML(root) == false)
            delete audioTrigger;
        else
        {
            QQmlEngine::setObjectOwnership(audioTrigger, QQmlEngine::CppOwnership);
            setupWidget(audioTrigger, audioTrigger->page());
            m_vc->addWidgetToMap(audioTrigger);
            if (render && m_item)
                audioTrigger->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCSpeedDial)
    {
        /* Create a new speedDial into its parent */
        VCSpeedDial *speedDial = new VCSpeedDial(m_doc, this);
        if (speedDial->loadXML(root) == false)
            delete speedDial;
        else
        {
            QQmlEngine::setObjectOwnership(speedDial, QQmlEngine::CppOwnership);
            setupWidget(speedDial, speedDial->page());
            m_vc->addWidgetToMap(speedDial);
            if (render && m_item)
                speedDial->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCXYPad)
    {
        /* Create a new xyPad into its parent */
        VCXYPad *xyPad = new VCXYPad(m_doc, this);
        if (xyPad->loadXML(root) == false)
            delete xyPad;
        else
        {
            QQmlEngine::setObjectOwnership(xyPad, QQmlEngine::CppOwnership);
            setupWidget(xyPad, xyPad->page());
            m_vc->addWidgetToMap(xyPad);
            if (render && m_item)
                xyPad->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCClock)
    {
        /* Create a new clock into its parent */
        VCClock *clock = new VCClock(m_doc, this);
        if (clock->loadXML(root) == false)
            delete clock;
        else
        {
            QQmlEngine::setObjectOwnership(clock, QQmlEngine::CppOwnership);
            setupWidget(clock, clock->page());
            m_vc->addWidgetToMap(clock);
            if (render && m_item)
                clock->render(m_vc->view(), m_item);
        }
    }
    else if (root.name() == KXMLQLCVCCueList)
    {
        /* Create a new cue list into its parent */
        VCCueList *cuelist = new VCCueList(m_doc, this);
        if (cuelist->loadXML(root) == false)
            delete cuelist;
        else
        {
            QQmlEngine::setObjectOwnership(cuelist, QQmlEngine::CppOwnership);
            setupWidget(cuelist, cuelist->page());
            m_vc->addWidgetToMap(cuelist);
            if (render && m_item)
                cuelist->render(m_vc->view(), m_item);
        }
    }
    else
    {
        return false;
    }

    return true;
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
        else if (root.name() == KXMLQLCVCFramePIN)
        {
            SimpleCrypt crypter(encKey);
            QString decPin = crypter.decryptToString(root.readElementText());
            setPIN(decPin.toInt());
        }
        else if (root.name() == KXMLQLCVCFrameMultipage)
        {
            setMultiPageMode(true);
            QXmlStreamAttributes attrs = root.attributes();
            if (attrs.hasAttribute(KXMLQLCVCFramePagesNumber))
                setTotalPagesNumber(attrs.value(KXMLQLCVCFramePagesNumber).toInt());

            if (attrs.hasAttribute(KXMLQLCVCFrameCurrentPage))
                currentPage = attrs.value(KXMLQLCVCFrameCurrentPage).toInt();

            if (attrs.hasAttribute(KXMLQLCVCFramePagesLoop))
                setPagesLoop(true);

            root.skipCurrentElement();
        }
#if 0
        else if (root.name() == KXMLQLCVCSoloFrameMixing && this->type() == SoloFrameWidget)
        {
            if (root.readElementText() == KXMLQLCTrue)
                reinterpret_cast<VCSoloFrame*>(this)->setSoloframeMixing(true);
            else
                reinterpret_cast<VCSoloFrame*>(this)->setSoloframeMixing(false);
        }
#endif
        else if (root.name() == KXMLQLCVCFrameEnableSource)
        {
            loadXMLSources(root, INPUT_ENABLE_ID);
        }
        else if (root.name() == KXMLQLCVCFrameNext)
        {
            loadXMLSources(root, INPUT_NEXT_PAGE_ID);
        }
        else if (root.name() == KXMLQLCVCFramePrevious)
        {
            loadXMLSources(root, INPUT_PREVIOUS_PAGE_ID);
        }
        else if (root.name() == KXMLQLCVCFramePagesLoop) // LEGACY
        {
            if (root.readElementText() == KXMLQLCTrue)
                setPagesLoop(true);
            else
                setPagesLoop(false);
        }
        else if (root.name() == KXMLQLCVCFrameShortcut)
        {
            int page = 0;
            QString name;

            QXmlStreamAttributes attrs = root.attributes();
            if (attrs.hasAttribute(KXMLQLCVCFrameShortcutPage))
                page = attrs.value(KXMLQLCVCFrameShortcutPage).toInt();

            if (attrs.hasAttribute(KXMLQLCVCFrameShortcutName))
                name = attrs.value(KXMLQLCVCFrameShortcutName).toString();
            m_pageLabels.insert(page, name);

            registerExternalControl(INPUT_SHORTCUT_BASE_ID + page, name, true);
            loadXMLSources(root, INPUT_SHORTCUT_BASE_ID + page);

            //root.skipCurrentElement();
        }
        else
        {
            if (loadWidgetXML(root) == false)
            {
                qWarning() << Q_FUNC_INFO << "Unknown frame tag:" << root.name().toString();
                root.skipCurrentElement();
            }
        }
    }

    if (multiPageMode() == true)
        setCurrentPage(currentPage);

    return true;
}

bool VCFrame::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

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

    /* Optional PIN */
    if (PIN() != 0)
    {
        SimpleCrypt crypter(encKey);
        QString encPin = crypter.encryptToString(QString("%1").arg(PIN()));
        doc->writeTextElement(KXMLQLCVCFramePIN, encPin);
    }

    /* Enable control */
    saveXMLInputControl(doc, INPUT_ENABLE_ID, false, KXMLQLCVCFrameEnableSource);

    /* Multipage mode */
    if (multiPageMode() == true)
    {
        doc->writeStartElement(KXMLQLCVCFrameMultipage);
        doc->writeAttribute(KXMLQLCVCFramePagesNumber, QString::number(totalPagesNumber()));
        doc->writeAttribute(KXMLQLCVCFrameCurrentPage, QString::number(currentPage()));
        if (pagesLoop())
            doc->writeAttribute(KXMLQLCVCFramePagesLoop, KXMLQLCTrue);
        doc->writeEndElement();

        saveXMLInputControl(doc, INPUT_NEXT_PAGE_ID, false, KXMLQLCVCFrameNext);
        saveXMLInputControl(doc, INPUT_PREVIOUS_PAGE_ID, false, KXMLQLCVCFramePrevious);

        /* Write shortcuts, if any */
        QMapIterator <int, QString> it(m_pageLabels);
        while (it.hasNext() == true)
        {
            it.next();

            doc->writeStartElement(KXMLQLCVCFrameShortcut);
            doc->writeAttribute(KXMLQLCVCFrameShortcutPage, QString::number(it.key()));
            doc->writeAttribute(KXMLQLCVCFrameShortcutName, it.value());

            saveXMLInputControl(doc, INPUT_SHORTCUT_BASE_ID + it.key());

            doc->writeEndElement();
        }
    }

    /* Save children */
    foreach (VCWidget *child, children(false))
        child->saveXML(doc);

    /* End the <Frame> tag */
    doc->writeEndElement();

    return true;
}
