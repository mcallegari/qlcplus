/*
  Q Light Controller
  vcframe.cpp

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

#include <QStyleOptionFrameV2>
#include <QMapIterator>
#include <QMetaObject>
#include <QMessageBox>
#include <QPainter>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QPoint>
#include <QSize>
#include <QMenu>
#include <QFont>
#include <QList>
#include <QtXml>

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

VCFrame::VCFrame(QWidget* parent, Doc* doc, bool canCollapse) : VCWidget(parent, doc)
    , m_hbox(NULL)
    , m_button(NULL)
    , m_label(NULL)
    , m_collapsed(false)
    , m_showHeader(true)
    , m_multiPageMode(false)
    , m_currentPage(0)
    , m_totalPagesNumber(1)
    , m_nextPageBtn(NULL)
    , m_prevPageBtn(NULL)
    , m_pageLabel(NULL)
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
        m_button->hide();
        m_label->hide();
    }
    else
    {
        m_button->show();
        m_label->show();
    }
}

bool VCFrame::isHeaderVisible() const
{
    return m_showHeader;
}

bool VCFrame::isCollapsed()
{
    return m_collapsed;
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

    m_button = new QToolButton(this);
    m_button->setStyle(AppUtil::saneStyle());
    m_button->setIconSize(QSize(32, 32));
    m_button->setMinimumSize(QSize(32, 32));
    m_button->setMaximumSize(QSize(32, 32));
    m_button->setIcon(QIcon(":/expand.png"));
    m_button->setCheckable(true);
    QString btnSS = "QToolButton { background-color: #E0DFDF; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    btnSS += "QToolButton:pressed { background-color: #919090; border: 1px solid gray; border-radius: 3px; padding: 3px; } ";
    m_button->setStyleSheet(btnSS);

    m_hbox->addWidget(m_button);
    connect(m_button, SIGNAL(toggled(bool)), this, SLOT(slotCollapseButtonToggled(bool)));

    m_label = new QLabel(this);
    m_label->setText(this->caption());
    QString txtColor = "white";
    if (m_hasCustomForegroundColor)
        txtColor = this->foregroundColor().name();
    m_label->setStyleSheet("QLabel { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #666666, stop: 1 #000000); "
                           "color: " + txtColor + "; border-radius: 3px; padding: 3px; margin-left: 2px; }");

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
                m_pagesMap.insert(child, child->page());
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
    return m_currentPage;
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
    slotSetPage(m_currentPage - 1);
}

void VCFrame::slotNextPage()
{
    slotSetPage(m_currentPage + 1);
}

void VCFrame::slotSetPage(int pageNum)
{
    if (m_pageLabel)
    {
        if (pageNum >= 0 && pageNum < m_totalPagesNumber)
            m_currentPage = pageNum;

        // let's say this is a piece of code to "feedback" the
        // InputPatch and keep it in sync with VCFrame
        QLCInputSource src = inputSource(nextPageInputSourceId);
        if (src.universe() != QLCInputSource::invalidUniverse)
        {
            InputPatch *patch = m_doc->inputMap()->patch(src.universe());
            if (patch != NULL)
                patch->setPage(m_currentPage);
        }

        // invalid page ? exits here
        if (pageNum != m_currentPage)
            return;

        m_pageLabel->setText(tr("Page: %1").arg(m_currentPage + 1));

        QMapIterator <VCWidget*, int> it(m_pagesMap);
        while (it.hasNext() == true)
        {
            it.next();
            int page = it.value();
            VCWidget *widget = it.key();
            if (page == m_currentPage)
            {
                widget->show();
                widget->updateFeedback();
            }
            else
                widget->hide();
        }
        m_doc->setModified();
    }
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
    if (m_previousPageKeySequence == keySequence)
        slotSetPage(m_currentPage - 1);
    else if (m_nextPageKeySequence == keySequence)
        slotSetPage(m_currentPage + 1);
}

void VCFrame::updateFeedback()
{
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
    Q_UNUSED(value)

    //if (m_doc->mode() == Doc::Design)
    //    return;

    QLCInputSource src(universe, channel);

    if (src == inputSource(previousPageInputSourceId))
        slotSetPage(value);
    else if (src == inputSource(nextPageInputSourceId))
        slotSetPage(value);
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

    QListIterator <VCWidget*> it(widget->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();

        /* findChildren() is recursive, so the list contains all
           possible child widgets below this frame. Each frame must
           save only its direct children to preserve hierarchy, so
           save only such widgets that have this widget as their
           direct parent. */
        if (child->parentWidget() == widget)
            child->createCopy(this);
    }

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCFrame::editProperties()
{
    if (isBottomFrame() == true)
        return;

    VCFrameProperties prop(NULL, this, m_doc);
    if (prop.exec() == QDialog::Accepted)
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
                        newWidget->setID(VirtualConsole::instance()->newWidgetId());
                        newWidget->setPage(pg);
                        newWidget->remapInputSources(pg);
                        newWidget->show();
                        /**
                         *  Remap input sources to the new page, otherwise
                         *  all the cloned widgets would respond to the
                         *  same controls
                         */
                        foreach( VCWidget* widget, newWidget->findChildren<VCWidget*>())
                        {
                            widget->setPage(pg);
                            widget->remapInputSources(pg);
                        }

                        m_pagesMap.insert(newWidget, pg);
                    }
                }
            }
            slotSetPage(0);
        }
        VirtualConsole* vc = VirtualConsole::instance();
        if (vc != NULL)
            vc->reselectWidgets();
    }
}

/*********************************************************************
 * Web access
 *********************************************************************/

QString VCFrame::getCSS()
{
    QString str = "<style>\n"
            " .vcframe {\n"
            " position: absolute;\n"
            " border-radius: 4px;\n"
            "}\n\n"

            ".vcframeHeader {\n"
            " background: linear-gradient(to bottom, #666666 0%, #000000 100%);\n"
            " background: -ms-linear-gradient(top, #666666 0%, #000000 100%);\n"
            " background: -moz-linear-gradient(top, #666666 0%, #000000 100%);\n"
            " background: -o-linear-gradient(top, #666666 0%, #000000 100%);\n"
            " background: -webkit-gradient(linear, left top, left bottom, color-stop(0, #666666), color-stop(1, #000000));\n"
            " background: -webkit-linear-gradient(top, #666666 0%, #000000 100%);\n"
            " border-radius: 3px;\n"
            " margin: 2px;\n"
            " padding: 0 0 0 3px;\n"
            " height: 32px;\n"
            " font:normal 20px/1.2em sans-serif;\n"
            "}\n"
            "</style>\n";

    return str;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCFrame::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != xmlTagName())
    {
        qWarning() << Q_FUNC_INFO << "Frame node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            /* Frame geometry (visibility is ignored) */
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            /* Frame appearance */
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCFrameAllowChildren)
        {
            /* Allow children */
            if (tag.text() == KXMLQLCTrue)
                setAllowChildren(true);
            else
                setAllowChildren(false);
        }
        else if (tag.tagName() == KXMLQLCVCFrameAllowResize)
        {
            /* Allow resize */
            if (tag.text() == KXMLQLCTrue)
                setAllowResize(true);
            else
                setAllowResize(false);
        }
        else if (tag.tagName() == KXMLQLCVCFrameIsCollapsed)
        {
            /* Collapsed */
            if (tag.text() == KXMLQLCTrue && m_button != NULL)
                m_button->toggle();
        }
        else if (tag.tagName() == KXMLQLCVCFrameShowHeader)
        {
            if (tag.text() == KXMLQLCTrue)
                setHeaderVisible(true);
            else
                setHeaderVisible(false);
        }
        else if (tag.tagName() == KXMLQLCVCFrameMultipage)
        {
            setMultipageMode(true);
            if (tag.hasAttribute(KXMLQLCVCFramePagesNumber))
                setTotalPagesNumber(tag.attribute(KXMLQLCVCFramePagesNumber).toInt());

            if(tag.hasAttribute(KXMLQLCVCFrameCurrentPage))
                slotSetPage(tag.attribute(KXMLQLCVCFrameCurrentPage).toInt());
        }
        else if (tag.tagName() == KXMLQLCVCFrameNext)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0, ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(QLCInputSource(uni, ch), nextPageInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCFrameKey)
                {
                    setNextPageKeySequence(stripKeySequence(QKeySequence(subTag.text())));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown Frame Next tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCFramePrevious)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0, ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(QLCInputSource(uni, ch), previousPageInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCFrameKey)
                {
                    setPreviousPageKeySequence(stripKeySequence(QKeySequence(subTag.text())));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown Frame Previous tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCFrame)
        {
            /* Create a new frame into its parent */
            VCFrame* frame = new VCFrame(this, m_doc, true);
            if (frame->loadXML(&tag) == false)
                delete frame;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(frame);
                frame->show();
            }
        }
        else if (tag.tagName() == KXMLQLCVCLabel)
        {
            /* Create a new label into its parent */
            VCLabel* label = new VCLabel(this, m_doc);
            if (label->loadXML(&tag) == false)
                delete label;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(label);
                label->show();
            }
        }
        else if (tag.tagName() == KXMLQLCVCButton)
        {
            /* Create a new button into its parent */
            VCButton* button = new VCButton(this, m_doc);
            if (button->loadXML(&tag) == false)
                delete button;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(button);
                button->show();
            }
        }
        else if (tag.tagName() == KXMLQLCVCXYPad)
        {
            /* Create a new xy pad into its parent */
            VCXYPad* xypad = new VCXYPad(this, m_doc);
            if (xypad->loadXML(&tag) == false)
                delete xypad;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(xypad);
                xypad->show();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSlider)
        {
            /* Create a new slider into its parent */
            VCSlider* slider = new VCSlider(this, m_doc);
            if (slider->loadXML(&tag) == false)
                delete slider;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(slider);
                slider->show();
                // always connect a slider as it it was a submaster
                // cause this signal is emitted only when a slider is
                // a submaster
                connect(slider, SIGNAL(submasterValueChanged(qreal)),
                        this, SLOT(slotSubmasterValueChanged(qreal)));
            }
        }
        else if (tag.tagName() == KXMLQLCVCSoloFrame)
        {
            /* Create a new frame into its parent */
            VCSoloFrame* soloframe = new VCSoloFrame(this, m_doc, true);
            if (soloframe->loadXML(&tag) == false)
                delete soloframe;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(soloframe);
                soloframe->show();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueList)
        {
            /* Create a new cuelist into its parent */
            VCCueList* cuelist = new VCCueList(this, m_doc);
            if (cuelist->loadXML(&tag) == false)
                delete cuelist;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(cuelist);
                cuelist->show();
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDial)
        {
            /* Create a new speed dial into its parent */
            VCSpeedDial* dial = new VCSpeedDial(this, m_doc);
            if (dial->loadXML(&tag) == false)
                delete dial;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(dial);
                dial->show();
            }
        }
        else if (tag.tagName() == KXMLQLCVCAudioTriggers)
        {
            VCAudioTriggers* triggers = new VCAudioTriggers(this, m_doc);
            if (triggers->loadXML(&tag) == false)
                delete triggers;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(triggers);
                triggers->show();
                connect(triggers, SIGNAL(enableRequest(quint32)),
                        VirtualConsole::instance(), SLOT(slotEnableAudioTriggers(quint32)));
            }
        }
        else if (tag.tagName() == KXMLQLCVCClock)
        {
            /* Create a new label into its parent */
            VCClock* clock = new VCClock(this, m_doc);
            if (clock->loadXML(&tag) == false)
                delete clock;
            else
            {
                if (multipageMode() == true)
                    addWidgetToPageMap(clock);
                clock->show();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown frame tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    if (multipageMode() == true)
        slotSetPage(0);

    return true;
}

bool VCFrame::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC Frame entry */
    root = doc->createElement(xmlTagName());
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Save appearance */
    saveXMLAppearance(doc, &root);

    if (isBottomFrame() == false)
    {
        /* Save widget proportions only for child frames */
        if (isCollapsed())
        {
            resize(QSize(m_width, m_height));
            saveXMLWindowState(doc, &root);
            resize(QSize(200, 40));
        }
        else
            saveXMLWindowState(doc, &root);

        /* Allow children */
        tag = doc->createElement(KXMLQLCVCFrameAllowChildren);
        if (allowChildren() == true)
            text = doc->createTextNode(KXMLQLCTrue);
        else
            text = doc->createTextNode(KXMLQLCFalse);
        tag.appendChild(text);
        root.appendChild(tag);

        /* Allow resize */
        tag = doc->createElement(KXMLQLCVCFrameAllowResize);
        if (allowResize() == true)
            text = doc->createTextNode(KXMLQLCTrue);
        else
            text = doc->createTextNode(KXMLQLCFalse);
        tag.appendChild(text);
        root.appendChild(tag);

        /* ShowHeader */
        tag = doc->createElement(KXMLQLCVCFrameShowHeader);
        if (isHeaderVisible())
            text = doc->createTextNode(KXMLQLCTrue);
        else
            text = doc->createTextNode(KXMLQLCFalse);
        tag.appendChild(text);
        root.appendChild(tag);

        /* Collapsed */
        tag = doc->createElement(KXMLQLCVCFrameIsCollapsed);
        if (isCollapsed())
            text = doc->createTextNode(KXMLQLCTrue);
        else
            text = doc->createTextNode(KXMLQLCFalse);
        tag.appendChild(text);
        root.appendChild(tag);

        /* Multipage mode */
        if (multipageMode() == true)
        {
            QDomElement subtag;

            tag = doc->createElement(KXMLQLCVCFrameMultipage);
            tag.setAttribute(KXMLQLCVCFramePagesNumber, totalPagesNumber());
            tag.setAttribute(KXMLQLCVCFrameCurrentPage, currentPage());
            root.appendChild(tag);

            /* Next page */
            tag = doc->createElement(KXMLQLCVCFrameNext);
            root.appendChild(tag);
            subtag = doc->createElement(KXMLQLCVCFrameKey);
            tag.appendChild(subtag);
            text = doc->createTextNode(m_nextPageKeySequence.toString());
            subtag.appendChild(text);
            saveXMLInput(doc, &tag, inputSource(nextPageInputSourceId));

            /* Previous page */
            tag = doc->createElement(KXMLQLCVCFramePrevious);
            root.appendChild(tag);
            subtag = doc->createElement(KXMLQLCVCFrameKey);
            tag.appendChild(subtag);
            text = doc->createTextNode(m_previousPageKeySequence.toString());
            subtag.appendChild(text);
            saveXMLInput(doc, &tag, inputSource(previousPageInputSourceId));
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
            widget->saveXML(doc, &root);
    }

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
