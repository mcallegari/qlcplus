/*
  Q Light Controller Plus
  vcframe.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include <QStyleOptionFrameV2>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMapIterator>
#include <QMetaObject>
#include <QMessageBox>
#include <QSettings>
#include <QPainter>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QPoint>
#include <QSize>
#include <QMenu>
#include <QFont>
#include <QList>

#include "vcpropertieseditor.h"
#include "vcframeproperties.h"
#include "vcaudiotriggers.h"
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vcspeeddial.h"
#include "inputpatch.h"
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcmatrix.h"
#include "qlcfile.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "vcclock.h"
#include "apputil.h"
#include "doc.h"

const QSize VCFrame::defaultSize(QSize(200, 200));

const quint8 VCFrame::nextPageInputSourceId = 0;
const quint8 VCFrame::previousPageInputSourceId = 1;
const quint8 VCFrame::enableInputSourceId = 2;

VCFrame::VCFrame(QWidget* parent, Doc* doc, bool canCollapse)
    : VCWidget(parent, doc)
    , m_hbox(NULL)
    , m_collapseButton(NULL)
    , m_enableButton(NULL)
    , m_label(NULL)
    , m_collapsed(false)
    , m_showHeader(true)
    , m_showEnableButton(true)
    , m_multiPageMode(false)
    , m_currentPage(0)
    , m_totalPagesNumber(1)
    , m_nextPageBtn(NULL)
    , m_prevPageBtn(NULL)
    , m_pageLabel(NULL)
    , m_pagesLoop(false)
{
    /* Set the class name "VCFrame" as the object name as well */
    setObjectName(VCFrame::staticMetaObject.className());
    setFrameStyle(KVCFrameStyleSunken);
    setAllowChildren(true);
    setType(VCWidget::FrameWidget);

    if (canCollapse == true)
        createHeader();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_FRAME_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(defaultSize);
    m_width = this->width();
    m_height = this->height();
}

VCFrame::~VCFrame()
{
}

bool VCFrame::isBottomFrame()
{
    return (parentWidget() != NULL && qobject_cast<VCFrame*>(parentWidget()) == NULL);
}

void VCFrame::setDisableState(bool disable)
{
    if (m_enableButton)
    {
        m_enableButton->blockSignals(true);
        m_enableButton->setChecked(!disable);
        m_enableButton->blockSignals(false);
    }

    foreach( VCWidget* widget, this->findChildren<VCWidget*>())
        widget->setDisableState(disable);
    m_disableState = disable;
    updateFeedback();
    //VCWidget::setDisableState(disable);
}

void VCFrame::setLiveEdit(bool liveEdit)
{
    if (m_doc->mode() == Doc::Design)
        return;

    m_liveEdit = liveEdit;

    if (!m_disableState)
        enableWidgetUI(!m_liveEdit);

    updateSubmasterValue();

    unsetCursor();
    update();
}

void VCFrame::setCaption(const QString& text)
{
    if (m_label != NULL)
        m_label->setText(text);

    VCWidget::setCaption(text);
}

void VCFrame::setFont(const QFont &font)
{
    if (m_label != NULL)
    {
        m_label->setFont(font);
        m_hasCustomFont = true;
        m_doc->setModified();
    }
}

QFont VCFrame::font() const
{
    if (m_label != NULL)
        return m_label->font();
    else
        return VCWidget::font();
}

void VCFrame::setForegroundColor(const QColor &color)
{
    if (m_label != NULL)
    {
        m_label->setStyleSheet("QLabel { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #666666, stop: 1 #000000); "
                               "color: " + color.name() + "; border-radius: 3px; padding: 3px; margin-left: 2px; }");
        m_hasCustomForegroundColor = true;
        m_doc->setModified();
    }
}

QColor VCFrame::foregroundColor() const
{
    if (m_label != NULL)
        return m_label->palette().color(m_label->foregroundRole());
    else
        return VCWidget::foregroundColor();
}

void VCFrame::setHeaderVisible(bool enable)
{
    m_showHeader = enable;

    if (m_hbox == NULL)
        createHeader();

    if (enable == false)
    {
        m_collapseButton->hide();
        m_label->hide();
        m_enableButton->hide();
    }
    else
    {
        m_collapseButton->show();
        m_label->show();
        if (m_showEnableButton)
            m_enableButton->show();
    }
}

bool VCFrame::isHeaderVisible() const
{
    return m_showHeader;
}

void VCFrame::setEnableButtonVisible(bool show)
{
    if (show && m_showHeader) m_enableButton->show();
    else m_enableButton->hide();

    m_showEnableButton = show;
}

bool VCFrame::isEnableButtonVisible() const
{
    return m_showEnableButton;
}

bool VCFrame::isCollapsed() const
{
    return m_collapsed;
}

QSize VCFrame::originalSize() const
{
    return QSize(m_width, m_height);
}

void VCFrame::slotCollapseButtonToggled(bool toggle)
{
    if (toggle == true)
    {
        m_width = this->width();
        m_height = this->height();
        if (m_multiPageMode == true)
        {
            if (m_prevPageBtn) m_prevPageBtn->hide();
            if (m_nextPageBtn) m_nextPageBtn->hide();
            if (m_pageLabel) m_pageLabel->hide();
        }
        resize(QSize(200, 40));
        m_collapsed = true;
    }
    else
    {
        resize(QSize(m_width, m_height));
        if (m_multiPageMode == true)
        {
            if (m_prevPageBtn) m_prevPageBtn->show();
            if (m_nextPageBtn) m_nextPageBtn->show();
            if (m_pageLabel) m_pageLabel->show();
        }
        m_collapsed = false;
    }
    m_doc->setModified();
}

void VCFrame::slotEnableButtonClicked(bool checked)
{
    setDisableState(!checked);
}

void VCFrame::createHeader()
{
    if (m_hbox != NULL)
        return;

    QVBoxLayout *vbox = new QVBoxLayout(this);
    /* Main HBox */
    m_hbox = new QHBoxLayout();
    m_hbox->setGeometry(QRect(0, 0, 200, 40));

    layout()->setSpacing(2);
    layout()->setContentsMargins(4, 4, 4, 4);
    layout()->addItem(m_hbox);
    vbox->addStretch();

    m_collapseButton = new QToolButton(this);
    m_collapseButton->setStyle(AppUtil::saneStyle());
    m_collapseButton->setIconSize(QSize(32, 32));
    m_collapseButton->setMinimumSize(QSize(32, 32));
    m_collapseButton->setMaximumSize(QSize(32, 32));
    m_collapseButton->setIcon(QIcon(":/expand.png"));
    m_collapseButton->setCheckable(true);
    QString cBtnSS = "QToolButton { background-color: #E0DFDF; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    cBtnSS += "QToolButton:pressed { background-color: #919090; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    m_collapseButton->setStyleSheet(cBtnSS);

    m_hbox->addWidget(m_collapseButton);
    connect(m_collapseButton, SIGNAL(toggled(bool)), this, SLOT(slotCollapseButtonToggled(bool)));

    m_label = new QLabel(this);
    m_label->setText(this->caption());
    QString txtColor = "white";
    if (m_hasCustomForegroundColor)
        txtColor = this->foregroundColor().name();
    m_label->setStyleSheet("QLabel { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #666666, stop: 1 #000000); "
                           "color: " + txtColor + "; border-radius: 3px; padding: 3px; margin-left: 2px; margin-right: 2px; }");

    if (m_hasCustomFont)
        m_label->setFont(font());
    else
    {
        QFont m_font = QApplication::font();
        m_font.setBold(true);
        m_font.setPixelSize(12);
        m_label->setFont(m_font);
    }
    m_hbox->addWidget(m_label);

    m_enableButton = new QToolButton(this);
    m_enableButton->setStyle(AppUtil::saneStyle());
    m_enableButton->setIconSize(QSize(32, 32));
    m_enableButton->setMinimumSize(QSize(32, 32));
    m_enableButton->setMaximumSize(QSize(32, 32));
    m_enableButton->setIcon(QIcon(":/check.png"));
    m_enableButton->setCheckable(true);
    QString eBtnSS = "QToolButton { background-color: #E0DFDF; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    eBtnSS += "QToolButton:checked { background-color: #D7DE75; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    m_enableButton->setStyleSheet(eBtnSS);
    m_enableButton->setEnabled(true);
    m_enableButton->setChecked(true);
    if (!m_showEnableButton)
        m_enableButton->hide();

    m_hbox->addWidget(m_enableButton);
    connect(m_enableButton, SIGNAL(clicked(bool)), this, SLOT(slotEnableButtonClicked(bool)));
}

/*********************************************************************
 * Pages
 *********************************************************************/

void VCFrame::setMultipageMode(bool enable)
{
    if (enable == true)
    {
        if (m_prevPageBtn != NULL && m_nextPageBtn != NULL && m_pageLabel != NULL)
            return;

        QString btnSS = "QToolButton { background-color: #E0DFDF; border: 1px solid gray; border-radius: 3px; padding: 3px; margin-left: 2px; }";
        btnSS += "QToolButton:pressed { background-color: #919090; border: 1px solid gray; border-radius: 3px; padding: 3px; margin-left: 2px; }";

        m_prevPageBtn = new QToolButton(this);
        m_prevPageBtn->setStyle(AppUtil::saneStyle());
        m_prevPageBtn->setIconSize(QSize(32, 32));
        m_prevPageBtn->setMinimumSize(QSize(32, 32));
        m_prevPageBtn->setMaximumSize(QSize(32, 32));
        m_prevPageBtn->setIcon(QIcon(":/back.png"));
        m_prevPageBtn->setStyleSheet(btnSS);
        m_hbox->addWidget(m_prevPageBtn);

        m_pageLabel = new QLabel(this);
        m_pageLabel->setMaximumWidth(100);
        m_pageLabel->setAlignment(Qt::AlignCenter);
        m_pageLabel->setText(tr("Page: %1").arg(m_currentPage + 1));
        m_pageLabel->setStyleSheet("QLabel { background-color: #000000; font-size: 15px; font-weight: bold;"
                                   "color: red; border-radius: 3px; padding: 3px; margin-left: 2px; }");
        m_hbox->addWidget(m_pageLabel);

        m_nextPageBtn = new QToolButton(this);
        m_nextPageBtn->setStyle(AppUtil::saneStyle());
        m_nextPageBtn->setIconSize(QSize(32, 32));
        m_nextPageBtn->setMinimumSize(QSize(32, 32));
        m_nextPageBtn->setMaximumSize(QSize(32, 32));
        m_nextPageBtn->setIcon(QIcon(":/forward.png"));
        m_nextPageBtn->setStyleSheet(btnSS);
        m_hbox->addWidget(m_nextPageBtn);

        connect (m_prevPageBtn, SIGNAL(clicked()), this, SLOT(slotPreviousPage()));
        connect (m_nextPageBtn, SIGNAL(clicked()), this, SLOT(slotNextPage()));

        m_prevPageBtn->show();
        m_pageLabel->show();
        m_nextPageBtn->show();

        if (m_pagesMap.isEmpty())
        {
            QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
            while (it.hasNext() == true)
            {
                VCWidget* child = it.next();
                addWidgetToPageMap(child);
            }
        }
    }
    else
    {
        if (m_prevPageBtn == NULL && m_nextPageBtn == NULL && m_pageLabel == NULL)
            return;
        m_hbox->removeWidget(m_prevPageBtn);
        m_hbox->removeWidget(m_pageLabel);
        m_hbox->removeWidget(m_nextPageBtn);
        delete m_prevPageBtn;
        delete m_pageLabel;
        delete m_nextPageBtn;
        m_prevPageBtn = NULL;
        m_pageLabel = NULL;
        m_nextPageBtn = NULL;
    }

    m_multiPageMode = enable;
}

bool VCFrame::multipageMode() const
{
    return m_multiPageMode;
}

void VCFrame::setTotalPagesNumber(int num)
{
    m_totalPagesNumber = num;
}

int VCFrame::totalPagesNumber()
{
    return m_totalPagesNumber;
}

int VCFrame::currentPage()
{
    if (m_multiPageMode == false)
        return 0;
    return m_currentPage;
}

void VCFrame::setPagesLoop(bool pagesLoop)
{
    m_pagesLoop = pagesLoop;
}

bool VCFrame::pagesLoop() const
{
    return m_pagesLoop;
}

void VCFrame::addWidgetToPageMap(VCWidget *widget)
{
    m_pagesMap.insert(widget, widget->page());
}

void VCFrame::removeWidgetFromPageMap(VCWidget *widget)
{
    m_pagesMap.remove(widget);
}

void VCFrame::slotPreviousPage()
{
    if (m_pagesLoop && m_currentPage == 0)
        slotSetPage(m_totalPagesNumber - 1);
    else
        slotSetPage(m_currentPage - 1);
    sendFeedback(m_currentPage, previousPageInputSourceId);
}

void VCFrame::slotNextPage()
{
    if (m_pagesLoop && m_currentPage == m_totalPagesNumber - 1)
        slotSetPage(0);
    else
        slotSetPage(m_currentPage + 1);

    sendFeedback(m_currentPage, nextPageInputSourceId);
}

void VCFrame::slotSetPage(int pageNum)
{
    if (m_pageLabel)
    {
        if (pageNum >= 0 && pageNum < m_totalPagesNumber)
            m_currentPage = pageNum;

        m_pageLabel->setText(tr("Page: %1").arg(m_currentPage + 1));

        QMapIterator <VCWidget*, int> it(m_pagesMap);
        while (it.hasNext() == true)
        {
            it.next();
            int page = it.value();
            VCWidget *widget = it.key();
            if (page == m_currentPage)
            {
                widget->setEnabled(true);
                widget->show();
                widget->updateFeedback();
            }
            else
            {
                widget->setEnabled(false);
                widget->hide();
            }
        }
        m_doc->setModified();
        emit pageChanged(m_currentPage);
    }
}

void VCFrame::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        if (isDisabled())
            slotEnableButtonClicked(false);
        updateSubmasterValue();
        updateFeedback();
    }

    VCWidget::slotModeChanged(mode);
}

/*********************************************************************
 * Submasters
 *********************************************************************/

void VCFrame::slotSubmasterValueChanged(qreal value)
{
    qDebug() << Q_FUNC_INFO << "val:" << value;
    VCSlider *submaster = (VCSlider *)sender();
    QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        if (child->parent() == this && child != submaster)
            child->adjustIntensity(value);
    }
}

void VCFrame::updateSubmasterValue()
{
    QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        if (child->parent() == this && child->type() == SliderWidget)
        {
            VCSlider* slider = reinterpret_cast<VCSlider*>(child);
            if (slider->sliderMode() == VCSlider::Submaster)
                slider->emitSubmasterValue();
        }
    }
}

void VCFrame::adjustIntensity(qreal val)
{
    QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        if (child->parent() == this)
            child->adjustIntensity(val);
    }
    VCWidget::adjustIntensity(val);
}

/*****************************************************************************
 * Key Sequences
 *****************************************************************************/

void VCFrame::setEnableKeySequence(const QKeySequence &keySequence)
{
    m_enableKeySequence = QKeySequence(keySequence);
    /* Quite a dirty workaround, but it works without interfering with other widgets */
    disconnect(this, SIGNAL(keyPressed(QKeySequence)), this, SLOT(slotFrameKeyPressed(QKeySequence)));
    connect(this, SIGNAL(keyPressed(QKeySequence)), this, SLOT(slotFrameKeyPressed(QKeySequence)));
}

QKeySequence VCFrame::enableKeySequence() const
{
    return m_enableKeySequence;
}

void VCFrame::setNextPageKeySequence(const QKeySequence& keySequence)
{
    m_nextPageKeySequence = QKeySequence(keySequence);
    /* Quite a dirty workaround, but it works without interfering with other widgets */
    disconnect(this, SIGNAL(keyPressed(QKeySequence)), this, SLOT(slotFrameKeyPressed(QKeySequence)));
    connect(this, SIGNAL(keyPressed(QKeySequence)), this, SLOT(slotFrameKeyPressed(QKeySequence)));
}

QKeySequence VCFrame::nextPageKeySequence() const
{
    return m_nextPageKeySequence;
}

void VCFrame::setPreviousPageKeySequence(const QKeySequence& keySequence)
{
    m_previousPageKeySequence = QKeySequence(keySequence);
    /* Quite a dirty workaround, but it works without interfering with other widgets */
    disconnect(this, SIGNAL(keyPressed(QKeySequence)), this, SLOT(slotFrameKeyPressed(QKeySequence)));
    connect(this, SIGNAL(keyPressed(QKeySequence)), this, SLOT(slotFrameKeyPressed(QKeySequence)));
}

QKeySequence VCFrame::previousPageKeySequence() const
{
    return m_previousPageKeySequence;
}

void VCFrame::slotFrameKeyPressed(const QKeySequence& keySequence)
{
    if (isEnabled() == false)
        return;

    if (m_enableKeySequence == keySequence)
        setDisableState(!isDisabled());
    else if (m_previousPageKeySequence == keySequence)
        slotPreviousPage();
    else if (m_nextPageKeySequence == keySequence)
        slotNextPage();
}

void VCFrame::updateFeedback()
{
    QSharedPointer<QLCInputSource> src = inputSource(enableInputSourceId);
    if (!src.isNull() && src->isValid() == true)
    {
        if (m_disableState == false)
            sendFeedback(src->upperValue(), enableInputSourceId);
        else
            sendFeedback(src->lowerValue(), enableInputSourceId);
    }

    QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        child->updateFeedback();
    }
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCFrame::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (isEnabled() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender(), enableInputSourceId) && value)
        setDisableState(!isDisabled());
    else if (checkInputSource(universe, pagedCh, value, sender(), previousPageInputSourceId) && value)
        slotPreviousPage();
    else if (checkInputSource(universe, pagedCh, value, sender(), nextPageInputSourceId) && value)
        slotNextPage();
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCFrame::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCFrame* frame = new VCFrame(parent, m_doc, true);
    if (frame->copyFrom(this) == false)
    {
        delete frame;
        frame = NULL;
    }

    return frame;
}

bool VCFrame::copyFrom(const VCWidget* widget)
{
    const VCFrame* frame = qobject_cast<const VCFrame*> (widget);
    if (frame == NULL)
        return false;

    setHeaderVisible(frame->m_showHeader);
    setEnableButtonVisible(frame->m_showEnableButton);

    setTotalPagesNumber(frame->m_totalPagesNumber);
    setMultipageMode(frame->m_multiPageMode);

    setPagesLoop(frame->m_pagesLoop);

    QListIterator <VCWidget*> it(widget->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        VCWidget* childCopy = NULL;

        /* findChildren() is recursive, so the list contains all
           possible child widgets below this frame. Each frame must
           save only its direct children to preserve hierarchy, so
           save only such widgets that have this widget as their
           direct parent. */
        if (child->parentWidget() == widget)
        {
            childCopy = child->createCopy(this);
            VirtualConsole::instance()->addWidgetInMap(childCopy);

            qDebug() << "Child copy in parent:" << childCopy->caption() << ", page:" << childCopy->page();
        }

        if (childCopy != NULL)
        {
            addWidgetToPageMap(childCopy);

            if (childCopy->type() == VCWidget::SliderWidget)
            {
                VCSlider *slider = (VCSlider*)childCopy;
                // always connect a slider as it it was a submaster
                // cause this signal is emitted only when a slider is
                // a submaster
                connect(slider, SIGNAL(submasterValueChanged(qreal)),
                        this, SLOT(slotSubmasterValueChanged(qreal)));
            }
        }
    }

    if (m_multiPageMode)
        slotSetPage(frame->m_currentPage);

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCFrame::applyProperties(VCFrameProperties const& prop)
{
    if (multipageMode() == true && prop.cloneWidgets() == true && m_pagesMap.isEmpty() == false)
    {
        for (int pg = 1; pg < totalPagesNumber(); pg++)
        {
            QListIterator <VCWidget*> it(this->findChildren<VCWidget*>());
            while (it.hasNext() == true)
            {
                VCWidget* child = it.next();
                if (child->page() == 0 && child->parentWidget() == this)
                {
                    VCWidget *newWidget = child->createCopy(this);
                    VirtualConsole::instance()->addWidgetInMap(newWidget);
                    //qDebug() << "Cloning:" << newWidget->caption() << ", copy page:" << newWidget->page() << ", page to set:" << pg;
                    newWidget->setPage(pg);
                    newWidget->remapInputSources(pg);
                    newWidget->show();

                    bool multiPageFrame = false;
                    if (newWidget->type() == VCWidget::FrameWidget || newWidget->type() == VCWidget::SoloFrameWidget)
                    {
                        VCFrame *fr = qobject_cast<VCFrame *>(newWidget);
                        multiPageFrame = fr->multipageMode();
                    }
                    /** If the cloned widget is again a multipage frame, then there's not much
                     *  that can be done to distinguish nested pages, so we leave the children
                     *  mapping as it is */
                    if (multiPageFrame == false)
                    {
                        /**
                         *  Remap input sources to the new page, otherwise
                         *  all the cloned widgets would respond to the
                         *  same controls
                         */
                        foreach (VCWidget* widget, newWidget->findChildren<VCWidget*>())
                        {
                            //qDebug() << "Child" << widget->caption() << ", page:" << widget->page() << ", new page:" << pg;
                            widget->setPage(pg);
                            widget->remapInputSources(pg);
                        }
                    }

                    addWidgetToPageMap(newWidget);
                }
            }
        }
        slotSetPage(0);
    }
    else if (multipageMode() == false)
    {
        setTotalPagesNumber(1);
        resize(QSize(this->width(), this->height()));

        QMapIterator <VCWidget*, int> it(m_pagesMap);
        while (it.hasNext() == true)
        {
            it.next();
            int page = it.value();
            VCWidget *widget = it.key();
            if (page > 0)
            {
                removeWidgetFromPageMap(widget);
                delete widget;
            }
            else
            {
                widget->setEnabled(true);
                widget->show();
                widget->updateFeedback();
            }
        }
    }
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc != NULL)
        vc->reselectWidgets();
}

void VCFrame::editProperties()
{
    if (isBottomFrame() == true)
        return;

    VCFrameProperties prop(NULL, this, m_doc);
    if (prop.exec() == QDialog::Accepted)
    {
        applyProperties(prop);
    }
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCFrame::loadXML(QXmlStreamReader &root)
{
    bool disableState = false;

    if (root.name() != xmlTagName())
    {
        qWarning() << Q_FUNC_INFO << "Frame node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    while (root.readNextStartElement())
    {
        /*
        qDebug() << "VC Frame <" << caption() << "> tag:" << root.name();
        if (root.attributes().hasAttribute("Caption"))
            qDebug() << "Widget caption:" << root.attributes().value("Caption").toString();
        */

        if (root.name() == KXMLQLCWindowState)
        {
            /* Frame geometry (visibility is ignored) */
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
            m_width = w;
            m_height = h;
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            /* Frame appearance */
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCFrameAllowChildren)
        {
            /* Allow children */
            if (root.readElementText() == KXMLQLCTrue)
                setAllowChildren(true);
            else
                setAllowChildren(false);
        }
        else if (root.name() == KXMLQLCVCFrameAllowResize)
        {
            /* Allow resize */
            if (root.readElementText() == KXMLQLCTrue)
                setAllowResize(true);
            else
                setAllowResize(false);
        }
        else if (root.name() == KXMLQLCVCFrameIsCollapsed)
        {
            /* Collapsed */
            if (root.readElementText() == KXMLQLCTrue && m_collapseButton != NULL)
                m_collapseButton->toggle();
        }
        else if (root.name() == KXMLQLCVCFrameIsDisabled)
        {
            /* Enabled */
            if (root.readElementText() == KXMLQLCTrue)
                disableState = true;
        }
        else if (root.name() == KXMLQLCVCFrameShowHeader)
        {
            if (root.readElementText() == KXMLQLCTrue)
                setHeaderVisible(true);
            else
                setHeaderVisible(false);
        }
        else if (root.name() == KXMLQLCVCFrameShowEnableButton)
        {
            if (root.readElementText() == KXMLQLCTrue)
                setEnableButtonVisible(true);
            else
                setEnableButtonVisible(false);
        }
        else if (root.name() == KXMLQLCVCSoloFrameMixing && this->type() == SoloFrameWidget)
        {
            if (root.readElementText() == KXMLQLCTrue)
                reinterpret_cast<VCSoloFrame*>(this)->setSoloframeMixing(true);
            else
                reinterpret_cast<VCSoloFrame*>(this)->setSoloframeMixing(false);
        }
        else if (root.name() == KXMLQLCVCFrameMultipage)
        {
            setMultipageMode(true);
            QXmlStreamAttributes attrs = root.attributes();
            if (attrs.hasAttribute(KXMLQLCVCFramePagesNumber))
                setTotalPagesNumber(attrs.value(KXMLQLCVCFramePagesNumber).toString().toInt());

            if(attrs.hasAttribute(KXMLQLCVCFrameCurrentPage))
                slotSetPage(attrs.value(KXMLQLCVCFrameCurrentPage).toString().toInt());
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCFrameEnableSource)
        {
            QString str = loadXMLSources(root, enableInputSourceId);
            if (str.isEmpty() == false)
                setEnableKeySequence(stripKeySequence(QKeySequence(str)));
        }
        else if (root.name() == KXMLQLCVCFrameNext)
        {
            QString str = loadXMLSources(root, nextPageInputSourceId);
            if (str.isEmpty() == false)
                setNextPageKeySequence(stripKeySequence(QKeySequence(str)));
        }
        else if (root.name() == KXMLQLCVCFramePrevious)
        {
            QString str = loadXMLSources(root, previousPageInputSourceId);
            if (str.isEmpty() == false)
                setPreviousPageKeySequence(stripKeySequence(QKeySequence(str)));
        }
        else if (root.name() == KXMLQLCVCFramePagesLoop)
        {
            if (root.readElementText() == KXMLQLCTrue)
                setPagesLoop(true);
            else
                setPagesLoop(false);
        }
        else if (root.name() == KXMLQLCVCFrame)
        {
            /* Create a new frame into its parent */
            VCFrame* frame = new VCFrame(this, m_doc, true);
            if (frame->loadXML(root) == false)
                delete frame;
            else
            {
                addWidgetToPageMap(frame);
                frame->show();
            }
        }
        else if (root.name() == KXMLQLCVCLabel)
        {
            /* Create a new label into its parent */
            VCLabel* label = new VCLabel(this, m_doc);
            if (label->loadXML(root) == false)
                delete label;
            else
            {
                addWidgetToPageMap(label);
                label->show();
            }
        }
        else if (root.name() == KXMLQLCVCButton)
        {
            /* Create a new button into its parent */
            VCButton* button = new VCButton(this, m_doc);
            if (button->loadXML(root) == false)
                delete button;
            else
            {
                addWidgetToPageMap(button);
                button->show();
            }
        }
        else if (root.name() == KXMLQLCVCXYPad)
        {
            /* Create a new xy pad into its parent */
            VCXYPad* xypad = new VCXYPad(this, m_doc);
            if (xypad->loadXML(root) == false)
                delete xypad;
            else
            {
                addWidgetToPageMap(xypad);
                xypad->show();
            }
        }
        else if (root.name() == KXMLQLCVCSlider)
        {
            /* Create a new slider into its parent */
            VCSlider* slider = new VCSlider(this, m_doc);
            if (slider->loadXML(root) == false)
                delete slider;
            else
            {
                addWidgetToPageMap(slider);
                slider->show();
                // always connect a slider as it it was a submaster
                // cause this signal is emitted only when a slider is
                // a submaster
                connect(slider, SIGNAL(submasterValueChanged(qreal)),
                        this, SLOT(slotSubmasterValueChanged(qreal)));
            }
        }
        else if (root.name() == KXMLQLCVCSoloFrame)
        {
            /* Create a new frame into its parent */
            VCSoloFrame* soloframe = new VCSoloFrame(this, m_doc, true);
            if (soloframe->loadXML(root) == false)
                delete soloframe;
            else
            {
                if (m_doc->mode() == Doc::Operate)
                    soloframe->updateChildrenConnection(true);
                addWidgetToPageMap(soloframe);
                soloframe->show();
            }
        }
        else if (root.name() == KXMLQLCVCCueList)
        {
            /* Create a new cuelist into its parent */
            VCCueList* cuelist = new VCCueList(this, m_doc);
            if (cuelist->loadXML(root) == false)
                delete cuelist;
            else
            {
                addWidgetToPageMap(cuelist);
                cuelist->show();
            }
        }
        else if (root.name() == KXMLQLCVCSpeedDial)
        {
            /* Create a new speed dial into its parent */
            VCSpeedDial* dial = new VCSpeedDial(this, m_doc);
            if (dial->loadXML(root) == false)
                delete dial;
            else
            {
                addWidgetToPageMap(dial);
                dial->show();
            }
        }
        else if (root.name() == KXMLQLCVCAudioTriggers)
        {
            VCAudioTriggers* triggers = new VCAudioTriggers(this, m_doc);
            if (triggers->loadXML(root) == false)
                delete triggers;
            else
            {
                addWidgetToPageMap(triggers);
                triggers->show();
            }
        }
        else if (root.name() == KXMLQLCVCClock)
        {
            /* Create a new VCClock into its parent */
            VCClock* clock = new VCClock(this, m_doc);
            if (clock->loadXML(root) == false)
                delete clock;
            else
            {
                addWidgetToPageMap(clock);
                clock->show();
            }
        }
        else if (root.name() == KXMLQLCVCMatrix)
        {
            /* Create a new VCMatrix into its parent */
            VCMatrix* matrix = new VCMatrix(this, m_doc);
            if (matrix->loadXML(root) == false)
                delete matrix;
            else
            {
                addWidgetToPageMap(matrix);
                matrix->show();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown frame tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    if (multipageMode() == true)
        slotSetPage(0);

    if (disableState == true)
        setDisableState(true);

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

    if (isBottomFrame() == false)
    {
        /* Save widget proportions only for child frames */
        if (isCollapsed())
        {
            resize(QSize(m_width, m_height));
            saveXMLWindowState(doc);
            resize(QSize(200, 40));
        }
        else
            saveXMLWindowState(doc);

        /* Allow children */
        doc->writeTextElement(KXMLQLCVCFrameAllowChildren, allowChildren() ? KXMLQLCTrue : KXMLQLCFalse);

        /* Allow resize */
        doc->writeTextElement(KXMLQLCVCFrameAllowResize, allowResize() ? KXMLQLCTrue : KXMLQLCFalse);

        /* ShowHeader */
        doc->writeTextElement(KXMLQLCVCFrameShowHeader, isHeaderVisible() ? KXMLQLCTrue : KXMLQLCFalse);

        /* ShowEnableButton */
        doc->writeTextElement(KXMLQLCVCFrameShowEnableButton, isEnableButtonVisible() ? KXMLQLCTrue : KXMLQLCFalse);

        /* Solo frame mixing */
        if (this->type() == SoloFrameWidget)
        {
            if (reinterpret_cast<VCSoloFrame*>(this)->soloframeMixing())
                doc->writeTextElement(KXMLQLCVCSoloFrameMixing, KXMLQLCTrue);
            else
                doc->writeTextElement(KXMLQLCVCSoloFrameMixing, KXMLQLCFalse);
        }

        /* Collapsed */
        doc->writeTextElement(KXMLQLCVCFrameIsCollapsed, isCollapsed() ? KXMLQLCTrue : KXMLQLCFalse);

        /* Disabled */
        doc->writeTextElement(KXMLQLCVCFrameIsDisabled, isDisabled() ? KXMLQLCTrue : KXMLQLCFalse);

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

        /* Multipage mode */
        if (multipageMode() == true)
        {
            doc->writeStartElement(KXMLQLCVCFrameMultipage);
            doc->writeAttribute(KXMLQLCVCFramePagesNumber, QString::number(totalPagesNumber()));
            doc->writeAttribute(KXMLQLCVCFrameCurrentPage, QString::number(currentPage()));
            doc->writeEndElement();

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

            /* Pages Loop */
            doc->writeTextElement(KXMLQLCVCFramePagesLoop, m_pagesLoop ? KXMLQLCTrue : KXMLQLCFalse);
        }
    }

    /* Save children */
    QListIterator <VCWidget*> it(findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* widget = it.next();

        /* findChildren() is recursive, so the list contains all
           possible child widgets below this frame. Each frame must
           save only its direct children to preserve hierarchy, so
           save only such widgets that have this widget as their
           direct parent. */
        if (widget->parentWidget() == this)
            widget->saveXML(doc);
    }

    /* End the <Frame> tag */
    doc->writeEndElement();

    return true;
}

void VCFrame::postLoad()
{
    QListIterator <VCWidget*> it(findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* widget = it.next();

        /* findChildren() is recursive, so the list contains all
           possible child widgets below this frame. Each frame must
           save only its direct children to preserve hierarchy, so
           save only such widgets that have this widget as their
           direct parent. */
        if (widget->parentWidget() == this)
            widget->postLoad();
    }
}

QString VCFrame::xmlTagName() const
{
    return KXMLQLCVCFrame;
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCFrame::customMenu(QMenu* parentMenu)
{
    QMenu* menu = NULL;
    VirtualConsole* vc = VirtualConsole::instance();

    if (allowChildren() == true && vc != NULL)
    {
        /* Basically, just returning VC::addMenu() would suffice here, but
           since the returned menu will be deleted when the current widget
           changes, we have to copy the menu's contents into a new menu. */
        menu = new QMenu(parentMenu);
        menu->setTitle(tr("Add"));
        QListIterator <QAction*> it(vc->addMenu()->actions());
        while (it.hasNext() == true)
            menu->addAction(it.next());
    }

    return menu;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCFrame::handleWidgetSelection(QMouseEvent* e)
{
    /* No point coming here if there is no VC */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    /* Don't allow selection of the bottom frame. Selecting it will always
       actually clear the current selection. */
    if (isBottomFrame() == false)
        VCWidget::handleWidgetSelection(e);
    else
        vc->clearWidgetSelection();
}

void VCFrame::mouseMoveEvent(QMouseEvent* e)
{
    if (isBottomFrame() == false)
        VCWidget::mouseMoveEvent(e);
    else
        QWidget::mouseMoveEvent(e);
}
