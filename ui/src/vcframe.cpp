/*
  Q Light Controller
  vcframe.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QStyleOptionFrameV2>
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
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vcspeeddial.h"
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "qlcfile.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "apputil.h"
#include "doc.h"

const QSize VCFrame::defaultSize(QSize(200, 200));

VCFrame::VCFrame(QWidget* parent, Doc* doc, bool canCollapse) : VCWidget(parent, doc)
    , m_hbox(NULL)
    , m_button(NULL)
    , m_label(NULL)
    , m_collapsed(false)
    , m_showHeader(true)
{
    /* Set the class name "VCFrame" as the object name as well */
    setObjectName(VCFrame::staticMetaObject.className());
    setFrameStyle(KVCFrameStyleSunken);
    setAllowChildren(true);
    setType(VCWidget::FrameWidget);

    if (canCollapse == true)
    {
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
                               "color: " + txtColor + "; border-radius: 3px; padding: 3px; margin-left: 2px }");

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
                               "color: " + color.name() + "; border-radius: 3px; padding: 3px; margin-left: 2px }");
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

void VCFrame::setShowHeader(bool enable)
{
    m_showHeader = enable;

    if (m_button == NULL)
        return;

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
        resize(QSize(200, 40));
        m_collapsed = true;
    }
    else
    {
        resize(QSize(m_width, m_height));
        m_collapsed = false;
    }
    m_doc->setModified();
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

bool VCFrame::copyFrom(VCWidget* widget)
{
    VCFrame* frame = qobject_cast<VCFrame*> (widget);
    if (frame == NULL)
        return false;

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

    VCFrameProperties prop(NULL, this);
    if (prop.exec() == QDialog::Accepted)
    {
        setAllowChildren(prop.allowChildren());
        setAllowResize(prop.allowResize());
        setCaption(prop.frameName());
        m_showHeader = prop.showHeader();
        setShowHeader(prop.showHeader());
        VirtualConsole* vc = VirtualConsole::instance();
        if (vc != NULL)
            vc->reselectWidgets();
    }
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

    /* Caption */
    setCaption(root->attribute(KXMLQLCVCCaption));

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
                setShowHeader(true);
            else
                setShowHeader(false);
        }
        else if (tag.tagName() == KXMLQLCVCFrame)
        {
            /* Create a new frame into its parent */
            VCFrame* frame = new VCFrame(this, m_doc, true);
            if (frame->loadXML(&tag) == false)
                delete frame;
            else
                frame->show();
        }
        else if (tag.tagName() == KXMLQLCVCLabel)
        {
            /* Create a new label into its parent */
            VCLabel* label = new VCLabel(this, m_doc);
            if (label->loadXML(&tag) == false)
                delete label;
            else
                label->show();
        }
        else if (tag.tagName() == KXMLQLCVCButton)
        {
            /* Create a new button into its parent */
            VCButton* button = new VCButton(this, m_doc);
            if (button->loadXML(&tag) == false)
                delete button;
            else
                button->show();
        }
        else if (tag.tagName() == KXMLQLCVCXYPad)
        {
            /* Create a new xy pad into its parent */
            VCXYPad* xypad = new VCXYPad(this, m_doc);
            if (xypad->loadXML(&tag) == false)
                delete xypad;
            else
                xypad->show();
        }
        else if (tag.tagName() == KXMLQLCVCSlider)
        {
            /* Create a new slider into its parent */
            VCSlider* slider = new VCSlider(this, m_doc);
            if (slider->loadXML(&tag) == false)
                delete slider;
            else
                slider->show();
        }
        else if (tag.tagName() == KXMLQLCVCSoloFrame)
        {
            /* Create a new frame into its parent */
            VCSoloFrame* soloframe = new VCSoloFrame(this, m_doc, true);
            if (soloframe->loadXML(&tag) == false)
                delete soloframe;
            else
                soloframe->show();
        }
        else if (tag.tagName() == KXMLQLCVCCueList)
        {
            /* Create a new cuelist into its parent */
            VCCueList* cuelist = new VCCueList(this, m_doc);
            if (cuelist->loadXML(&tag) == false)
                delete cuelist;
            else
                cuelist->show();
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDial)
        {
            /* Create a new speed dial into its parent */
            VCSpeedDial* dial = new VCSpeedDial(this, m_doc);
            if (dial->loadXML(&tag) == false)
                delete dial;
            else
                dial->show();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown frame tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

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

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

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
