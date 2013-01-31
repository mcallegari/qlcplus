/*
  Q Light Controller
  virtualconsole.cpp

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

#include <QDesktopWidget>
#include <QApplication>
#include <QInputDialog>
#include <QColorDialog>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QScrollArea>
#include <QKeyEvent>
#include <QMenuBar>
#include <QToolBar>
#include <QString>
#include <QDebug>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "vcpropertieseditor.h"
#include "addvcbuttonmatrix.h"
#include "addvcslidermatrix.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "vcspeeddial.h"
#include "vcsoloframe.h"
#include "mastertimer.h"
#include "vcdockarea.h"
#include "outputmap.h"
#include "vccuelist.h"
#include "inputmap.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcxypad.h"
#include "doc.h"

#define SETTINGS_VC_SIZE "virtualconsole/size"

VirtualConsole* VirtualConsole::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

VirtualConsole::VirtualConsole(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)

    , m_editAction(EditNone)
    , m_toolbar(NULL)

    , m_addActionGroup(NULL)
    , m_editActionGroup(NULL)
    , m_bgActionGroup(NULL)
    , m_fgActionGroup(NULL)
    , m_fontActionGroup(NULL)
    , m_frameActionGroup(NULL)
    , m_stackingActionGroup(NULL)

    , m_addButtonAction(NULL)
    , m_addButtonMatrixAction(NULL)
    , m_addSliderAction(NULL)
    , m_addSliderMatrixAction(NULL)
    , m_addSpeedDialAction(NULL)
    , m_addXYPadAction(NULL)
    , m_addCueListAction(NULL)
    , m_addFrameAction(NULL)
    , m_addSoloFrameAction(NULL)
    , m_addLabelAction(NULL)

    , m_toolsSettingsAction(NULL)

    , m_editCutAction(NULL)
    , m_editCopyAction(NULL)
    , m_editPasteAction(NULL)
    , m_editDeleteAction(NULL)
    , m_editPropertiesAction(NULL)
    , m_editRenameAction(NULL)

    , m_bgColorAction(NULL)
    , m_bgImageAction(NULL)
    , m_bgDefaultAction(NULL)

    , m_fgColorAction(NULL)
    , m_fgDefaultAction(NULL)

    , m_fontAction(NULL)
    , m_resetFontAction(NULL)

    , m_frameSunkenAction(NULL)
    , m_frameRaisedAction(NULL)
    , m_frameNoneAction(NULL)

    , m_stackingRaiseAction(NULL)
    , m_stackingLowerAction(NULL)

    , m_customMenu(NULL)
    , m_editMenu(NULL)
    , m_addMenu(NULL)

    , m_dockArea(NULL)
    , m_contentsLayout(NULL)
    , m_scrollArea(NULL)
    , m_contents(NULL)

    , m_tapModifierDown(false)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(doc != NULL);

    /* Main layout */
    new QHBoxLayout(this);
    layout()->setMargin(1);
    layout()->setSpacing(1);

    initActions();
    initDockArea();
    m_contentsLayout = new QVBoxLayout;
    layout()->addItem(m_contentsLayout);
    initMenuBar();
    initContents();

    // Propagate mode changes to all widgets
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));

    // Use the initial mode
    slotModeChanged(m_doc->mode());

    // Nothing is selected
    updateActions();
}

VirtualConsole::~VirtualConsole()
{
    s_instance = NULL;
}

VirtualConsole* VirtualConsole::instance()
{
    return s_instance;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

VCProperties VirtualConsole::properties() const
{
    return m_properties;
}

/*****************************************************************************
 * Selected widget
 *****************************************************************************/

void VirtualConsole::setEditAction(VirtualConsole::EditAction action)
{
    m_editAction = action;
}

VirtualConsole::EditAction VirtualConsole::editAction() const
{
    return m_editAction;
}

const QList <VCWidget*> VirtualConsole::selectedWidgets() const
{
    return m_selectedWidgets;
}

void VirtualConsole::setWidgetSelected(VCWidget* widget, bool select)
{
    Q_ASSERT(widget != NULL);

    if (select == false)
    {
        m_selectedWidgets.removeAll(widget);
        widget->update();
    }
    else if (select == true && m_selectedWidgets.indexOf(widget) == -1)
    {
        m_selectedWidgets.append(widget);
        widget->update();
    }

    /* Change the custom menu to the latest-selected widget's menu */
    updateCustomMenu();

    /* Enable or disable actions */
    updateActions();
}

bool VirtualConsole::isWidgetSelected(VCWidget* widget) const
{
    if (widget == NULL || m_selectedWidgets.indexOf(widget) == -1)
        return false;
    else
        return true;
}

void VirtualConsole::clearWidgetSelection()
{
    /* Get a copy of selected widget list */
    QList <VCWidget*> widgets(m_selectedWidgets);

    /* Clear the list so isWidgetSelected() returns false for all widgets */
    m_selectedWidgets.clear();

    /* Update all widgets to clear the selection frame around them */
    QListIterator <VCWidget*> it(widgets);
    while (it.hasNext() == true)
        it.next()->update();

    /* Change the custom menu to the latest-selected widget's menu */
    updateCustomMenu();

    /* Enable or disable actions */
    updateActions();
}

void VirtualConsole::reselectWidgets()
{
    QList <VCWidget*> widgets(m_selectedWidgets);
    clearWidgetSelection();
    foreach (VCWidget* w, widgets)
        setWidgetSelected(w, true);
}

/*****************************************************************************
 * Actions, menu- and toolbar
 *****************************************************************************/

QMenu* VirtualConsole::customMenu() const
{
    return m_customMenu;
}

QMenu* VirtualConsole::editMenu() const
{
    return m_editMenu;
}

QMenu* VirtualConsole::addMenu() const
{
    return m_addMenu;
}

void VirtualConsole::initActions()
{
    /* Add menu actions */
    m_addButtonAction = new QAction(QIcon(":/button.png"), tr("New Button"), this);
    connect(m_addButtonAction, SIGNAL(triggered(bool)), this, SLOT(slotAddButton()));

    m_addButtonMatrixAction = new QAction(QIcon(":/buttonmatrix.png"), tr("New Button Matrix"), this);
    connect(m_addButtonMatrixAction, SIGNAL(triggered(bool)), this, SLOT(slotAddButtonMatrix()));

    m_addSliderAction = new QAction(QIcon(":/slider.png"), tr("New Slider"), this);
    connect(m_addSliderAction, SIGNAL(triggered(bool)), this, SLOT(slotAddSlider()));

    m_addSliderMatrixAction = new QAction(QIcon(":/slidermatrix.png"), tr("New Slider Matrix"), this);
    connect(m_addSliderMatrixAction, SIGNAL(triggered(bool)), this, SLOT(slotAddSliderMatrix()));

    m_addSpeedDialAction = new QAction(QIcon(":/knob.png"), tr("New Speed Dial"), this);
    connect(m_addSpeedDialAction, SIGNAL(triggered(bool)), this, SLOT(slotAddSpeedDial()));

    m_addXYPadAction = new QAction(QIcon(":/xypad.png"), tr("New XY pad"), this);
    connect(m_addXYPadAction, SIGNAL(triggered(bool)), this, SLOT(slotAddXYPad()));

    m_addCueListAction = new QAction(QIcon(":/cuelist.png"), tr("New Cue list"), this);
    connect(m_addCueListAction, SIGNAL(triggered(bool)), this, SLOT(slotAddCueList()));

    m_addFrameAction = new QAction(QIcon(":/frame.png"), tr("New Frame"), this);
    connect(m_addFrameAction, SIGNAL(triggered(bool)), this, SLOT(slotAddFrame()));

    m_addSoloFrameAction = new QAction(QIcon(":/soloframe.png"), tr("New Solo frame"), this);
    connect(m_addSoloFrameAction, SIGNAL(triggered(bool)), this, SLOT(slotAddSoloFrame()));

    m_addLabelAction = new QAction(QIcon(":/label.png"), tr("New Label"), this);
    connect(m_addLabelAction, SIGNAL(triggered(bool)), this, SLOT(slotAddLabel()));

    /* Put add actions under the same group */
    m_addActionGroup = new QActionGroup(this);
    m_addActionGroup->setExclusive(false);
    m_addActionGroup->addAction(m_addButtonAction);
    m_addActionGroup->addAction(m_addButtonMatrixAction);
    m_addActionGroup->addAction(m_addSliderAction);
    m_addActionGroup->addAction(m_addSliderMatrixAction);
    m_addActionGroup->addAction(m_addSpeedDialAction);
    m_addActionGroup->addAction(m_addXYPadAction);
    m_addActionGroup->addAction(m_addCueListAction);
    m_addActionGroup->addAction(m_addFrameAction);
    m_addActionGroup->addAction(m_addSoloFrameAction);
    m_addActionGroup->addAction(m_addLabelAction);

    /* Tools menu actions */
    m_toolsSettingsAction = new QAction(QIcon(":/configure.png"), tr("Virtual Console Settings"), this);
    connect(m_toolsSettingsAction, SIGNAL(triggered(bool)), this, SLOT(slotToolsSettings()));
    // Prevent this action from ending up to the application menu on OSX
    // and crashing the app after VC window is closed.
    m_toolsSettingsAction->setMenuRole(QAction::NoRole);

    /* Edit menu actions */
    m_editCutAction = new QAction(QIcon(":/editcut.png"), tr("Cut"), this);
    connect(m_editCutAction, SIGNAL(triggered(bool)), this, SLOT(slotEditCut()));

    m_editCopyAction = new QAction(QIcon(":/editcopy.png"), tr("Copy"), this);
    connect(m_editCopyAction, SIGNAL(triggered(bool)), this, SLOT(slotEditCopy()));

    m_editPasteAction = new QAction(QIcon(":/editpaste.png"), tr("Paste"), this);
    m_editPasteAction->setEnabled(false);
    connect(m_editPasteAction, SIGNAL(triggered(bool)), this, SLOT(slotEditPaste()));

    m_editDeleteAction = new QAction(QIcon(":/editdelete.png"), tr("Delete"), this);
    connect(m_editDeleteAction, SIGNAL(triggered(bool)), this, SLOT(slotEditDelete()));

    m_editPropertiesAction = new QAction(QIcon(":/edit.png"), tr("Widget Properties"), this);
    connect(m_editPropertiesAction, SIGNAL(triggered(bool)), this, SLOT(slotEditProperties()));

    m_editRenameAction = new QAction(QIcon(":/editclear.png"), tr("Rename Widget"), this);
    connect(m_editRenameAction, SIGNAL(triggered(bool)), this, SLOT(slotEditRename()));

    /* Put edit actions under the same group */
    m_editActionGroup = new QActionGroup(this);
    m_editActionGroup->setExclusive(false);
    m_editActionGroup->addAction(m_editCutAction);
    m_editActionGroup->addAction(m_editCopyAction);
    m_editActionGroup->addAction(m_editPasteAction);
    m_editActionGroup->addAction(m_editDeleteAction);
    m_editActionGroup->addAction(m_editPropertiesAction);
    m_editActionGroup->addAction(m_editRenameAction);

    /* Background menu actions */
    m_bgColorAction = new QAction(QIcon(":/color.png"), tr("Background Color"), this);
    connect(m_bgColorAction, SIGNAL(triggered(bool)), this, SLOT(slotBackgroundColor()));

    m_bgImageAction = new QAction(QIcon(":/image.png"), tr("Background Image"), this);
    connect(m_bgImageAction, SIGNAL(triggered(bool)), this, SLOT(slotBackgroundImage()));

    m_bgDefaultAction = new QAction(QIcon(":/undo.png"), tr("Default"), this);
    connect(m_bgDefaultAction, SIGNAL(triggered(bool)), this, SLOT(slotBackgroundNone()));

    /* Put BG actions under the same group */
    m_bgActionGroup = new QActionGroup(this);
    m_bgActionGroup->setExclusive(false);
    m_bgActionGroup->addAction(m_bgColorAction);
    m_bgActionGroup->addAction(m_bgImageAction);
    m_bgActionGroup->addAction(m_bgDefaultAction);

    /* Foreground menu actions */
    m_fgColorAction = new QAction(QIcon(":/fontcolor.png"), tr("Font Colour"), this);
    connect(m_fgColorAction, SIGNAL(triggered(bool)), this, SLOT(slotForegroundColor()));

    m_fgDefaultAction = new QAction(QIcon(":/undo.png"), tr("Default"), this);
    connect(m_fgDefaultAction, SIGNAL(triggered(bool)), this, SLOT(slotForegroundNone()));

    /* Put FG actions under the same group */
    m_fgActionGroup = new QActionGroup(this);
    m_fgActionGroup->setExclusive(false);
    m_fgActionGroup->addAction(m_fgColorAction);
    m_fgActionGroup->addAction(m_fgDefaultAction);

    /* Font menu actions */
    m_fontAction = new QAction(QIcon(":/fonts.png"), tr("Font"), this);
    connect(m_fontAction, SIGNAL(triggered(bool)), this, SLOT(slotFont()));

    m_resetFontAction = new QAction(QIcon(":/undo.png"), tr("Default"), this);
    connect(m_resetFontAction, SIGNAL(triggered(bool)), this, SLOT(slotResetFont()));

    /* Put font actions under the same group */
    m_fontActionGroup = new QActionGroup(this);
    m_fontActionGroup->setExclusive(false);
    m_fontActionGroup->addAction(m_fontAction);
    m_fontActionGroup->addAction(m_resetFontAction);

    /* Frame menu actions */
    m_frameSunkenAction = new QAction(QIcon(":/framesunken.png"), tr("Sunken"), this);
    connect(m_frameSunkenAction, SIGNAL(triggered(bool)), this, SLOT(slotFrameSunken()));

    m_frameRaisedAction = new QAction(QIcon(":/frameraised.png"), tr("Raised"), this);
    connect(m_frameRaisedAction, SIGNAL(triggered(bool)), this, SLOT(slotFrameRaised()));

    m_frameNoneAction = new QAction(QIcon(":/framenone.png"), tr("None"), this);
    connect(m_frameNoneAction, SIGNAL(triggered(bool)), this, SLOT(slotFrameNone()));

    /* Put frame actions under the same group */
    m_frameActionGroup = new QActionGroup(this);
    m_frameActionGroup->setExclusive(false);
    m_frameActionGroup->addAction(m_frameRaisedAction);
    m_frameActionGroup->addAction(m_frameSunkenAction);
    m_frameActionGroup->addAction(m_frameNoneAction);

    /* Stacking menu actions */
    m_stackingRaiseAction = new QAction(QIcon(":/up.png"), tr("Bring to front"), this);
    connect(m_stackingRaiseAction, SIGNAL(triggered(bool)), this, SLOT(slotStackingRaise()));

    m_stackingLowerAction = new QAction(QIcon(":/down.png"), tr("Send to back"), this);
    connect(m_stackingLowerAction, SIGNAL(triggered(bool)), this, SLOT(slotStackingLower()));

    /* Put stacking actions under the same group */
    m_stackingActionGroup = new QActionGroup(this);
    m_stackingActionGroup->setExclusive(false);
    m_stackingActionGroup->addAction(m_stackingRaiseAction);
    m_stackingActionGroup->addAction(m_stackingLowerAction);
}

void VirtualConsole::initMenuBar()
{
    /* Add menu */
    m_addMenu = new QMenu(this);
    m_addMenu->setTitle(tr("&Add"));
    m_addMenu->addAction(m_addButtonAction);
    m_addMenu->addAction(m_addButtonMatrixAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addSliderAction);
    m_addMenu->addAction(m_addSliderMatrixAction);
    m_addMenu->addAction(m_addSpeedDialAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addXYPadAction);
    m_addMenu->addAction(m_addCueListAction);
    m_addMenu->addSeparator();
    m_addMenu->addAction(m_addFrameAction);
    m_addMenu->addAction(m_addSoloFrameAction);
    m_addMenu->addAction(m_addLabelAction);

    /* Edit menu */
    m_editMenu = new QMenu(this);
    m_editMenu->setTitle(tr("&Edit"));
    m_editMenu->addAction(m_editCutAction);
    m_editMenu->addAction(m_editCopyAction);
    m_editMenu->addAction(m_editPasteAction);
    m_editMenu->addAction(m_editDeleteAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_editPropertiesAction);
    m_editMenu->addAction(m_editRenameAction);
    m_editMenu->addSeparator();

    /* Background Menu */
    QMenu* bgMenu = new QMenu(m_editMenu);
    bgMenu->setTitle(tr("&Background"));
    m_editMenu->addMenu(bgMenu);
    bgMenu->addAction(m_bgColorAction);
    bgMenu->addAction(m_bgImageAction);
    bgMenu->addAction(m_bgDefaultAction);

    /* Foreground menu */
    QMenu* fgMenu = new QMenu(m_editMenu);
    fgMenu->setTitle(tr("&Foreground"));
    m_editMenu->addMenu(fgMenu);
    fgMenu->addAction(m_fgColorAction);
    fgMenu->addAction(m_fgDefaultAction);

    /* Font menu */
    QMenu* fontMenu = new QMenu(m_editMenu);
    fontMenu->setTitle(tr("F&ont"));
    m_editMenu->addMenu(fontMenu);
    fontMenu->addAction(m_fontAction);
    fontMenu->addAction(m_resetFontAction);

    /* Frame menu */
    QMenu* frameMenu = new QMenu(m_editMenu);
    frameMenu->setTitle(tr("F&rame"));
    m_editMenu->addMenu(frameMenu);
    frameMenu->addAction(m_frameSunkenAction);
    frameMenu->addAction(m_frameRaisedAction);
    frameMenu->addAction(m_frameNoneAction);

    /* Stacking order menu */
    QMenu* stackMenu = new QMenu(m_editMenu);
    stackMenu->setTitle(tr("Stacking &order"));
    m_editMenu->addMenu(stackMenu);
    stackMenu->addAction(m_stackingRaiseAction);
    stackMenu->addAction(m_stackingLowerAction);

    /* Add a separator that separates the common edit items from a custom
       widget menu that gets appended to the edit menu when a selected
       widget provides one. */
    m_editMenu->addSeparator();

    /* Toolbar */
    m_toolbar = new QToolBar(this);
    m_contentsLayout->addWidget(m_toolbar);

    m_toolbar->addAction(m_addButtonAction);
    m_toolbar->addAction(m_addButtonMatrixAction);
    m_toolbar->addAction(m_addSliderAction);
    m_toolbar->addAction(m_addSliderMatrixAction);
    m_toolbar->addAction(m_addSpeedDialAction);
    m_toolbar->addAction(m_addXYPadAction);
    m_toolbar->addAction(m_addCueListAction);
    m_toolbar->addAction(m_addFrameAction);
    m_toolbar->addAction(m_addSoloFrameAction);
    m_toolbar->addAction(m_addLabelAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_editCutAction);
    m_toolbar->addAction(m_editCopyAction);
    m_toolbar->addAction(m_editPasteAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_editDeleteAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_editPropertiesAction);
    m_toolbar->addAction(m_editRenameAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_stackingRaiseAction);
    m_toolbar->addAction(m_stackingLowerAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_bgColorAction);
    m_toolbar->addAction(m_bgImageAction);
    m_toolbar->addAction(m_fgColorAction);
    m_toolbar->addAction(m_fontAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_toolsSettingsAction);
}

void VirtualConsole::updateCustomMenu()
{
    /* Get rid of the custom menu, but delete it later because this might
       be called from the very menu that is being deleted. */
    if (m_customMenu != NULL)
    {
        delete m_customMenu;
        m_customMenu = NULL;
    }

    if (m_selectedWidgets.size() > 0)
    {
        /* Change the custom menu to the last selected widget's menu */
        VCWidget* latestWidget = m_selectedWidgets.last();
        m_customMenu = latestWidget->customMenu(m_editMenu);
        if (m_customMenu != NULL)
            m_editMenu->addMenu(m_customMenu);
    }
    else
    {
        /* Change the custom menu to the bottom frame's menu */
        Q_ASSERT(contents() != NULL);
        m_customMenu = contents()->customMenu(m_editMenu);
        if (m_customMenu != NULL)
            m_editMenu->addMenu(m_customMenu);
    }
}

void VirtualConsole::updateActions()
{
    /* When selected widgets is empty, all actions go to main draw area. */
    if (m_selectedWidgets.isEmpty() == true)
    {
        /* Enable widget additions to draw area */
        m_addActionGroup->setEnabled(true);

        /* Disable edit actions that can't be allowed for draw area */
        m_editCutAction->setEnabled(false);
        m_editCopyAction->setEnabled(false);
        m_editDeleteAction->setEnabled(false);
        m_editRenameAction->setEnabled(false);
        m_editPropertiesAction->setEnabled(false);

        /* All the rest are disabled for draw area, except BG & font */
        m_frameActionGroup->setEnabled(false);
        m_stackingActionGroup->setEnabled(false);

        /* Enable paste to draw area if there's something to paste */
        if (m_clipboard.isEmpty() == true)
            m_editPasteAction->setEnabled(false);
        else
            m_editPasteAction->setEnabled(true);
    }
    else
    {
        /* Enable edit actions for other widgets */
        m_editCutAction->setEnabled(true);
        m_editCopyAction->setEnabled(true);
        m_editDeleteAction->setEnabled(true);
        m_editRenameAction->setEnabled(true);
        m_editPropertiesAction->setEnabled(true);

        /* Enable all common properties */
        m_bgActionGroup->setEnabled(true);
        m_fgActionGroup->setEnabled(true);
        m_fontActionGroup->setEnabled(true);
        m_frameActionGroup->setEnabled(true);
        m_stackingActionGroup->setEnabled(true);

        /* Check, whether the last selected widget can hold children */
        if (m_selectedWidgets.last()->allowChildren() == true)
        {
            /* Enable paste for widgets that can hold children */
            if (m_clipboard.isEmpty() == true)
                m_editPasteAction->setEnabled(false);
            else
                m_editPasteAction->setEnabled(true);

            /* Enable also new additions */
            m_addActionGroup->setEnabled(true);
        }
        else
        {
            /* No pasted children possible */
            m_editPasteAction->setEnabled(false);
        }
    }
}

/*****************************************************************************
 * Add menu callbacks
 *****************************************************************************/

VCWidget* VirtualConsole::closestParent() const
{
    /* If nothing is selected, return the bottom-most contents frame */
    if (m_selectedWidgets.isEmpty() == true)
        return contents();

    /* Find the next VCWidget in the hierarchy that accepts children */
    VCWidget* widget = m_selectedWidgets.last();
    while (widget != NULL)
    {
        if (widget->allowChildren() == true)
            return widget;
        else
            widget = qobject_cast<VCWidget*> (widget->parentWidget());
    }

    return NULL;
}

void VirtualConsole::slotAddButton()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCButton* button = new VCButton(parent, m_doc);
    Q_ASSERT(button != NULL);
    button->show();
    button->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(button, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddButtonMatrix()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    AddVCButtonMatrix abm(this, m_doc);
    if (abm.exec() == QDialog::Rejected)
        return;

    int h = abm.horizontalCount();
    int v = abm.verticalCount();
    int sz = abm.buttonSize();

    VCFrame* frame = NULL;
    if (abm.frameStyle() == AddVCButtonMatrix::NormalFrame)
        frame = new VCFrame(parent, m_doc);
    else
        frame = new VCSoloFrame(parent, m_doc);
    Q_ASSERT(frame != NULL);

    // Resize the parent frame to fit the buttons nicely and toggle resizing off
    frame->resize(QSize((h * sz) + 20, (v * sz) + 20));
    frame->setAllowResize(false);

    for (int y = 0; y < v; y++)
    {
        for (int x = 0; x < h; x++)
        {
            VCButton* button = new VCButton(frame, m_doc);
            Q_ASSERT(button != NULL);
            button->move(QPoint(10 + (x * sz), 10 + (y * sz)));
            button->resize(QSize(sz, sz));
            button->show();

            int index = (y * h) + x;
            if (index < abm.functions().size())
            {
                quint32 fid = abm.functions().at(index);
                Function* function = m_doc->function(fid);
                if (function != NULL)
                {
                    button->setFunction(fid);
                    button->setCaption(function->name());
                }
            }
        }
    }

    // Show the frame after adding buttons to prevent flickering
    frame->show();
    frame->move(parent->lastClickPoint());
    frame->setAllowChildren(false); // Don't allow more children
    clearWidgetSelection();
    setWidgetSelected(frame, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddSlider()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCSlider* slider = new VCSlider(parent, m_doc);
    Q_ASSERT(slider != NULL);
    slider->show();
    slider->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(slider, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddSliderMatrix()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    AddVCSliderMatrix avsm(this);
    if (avsm.exec() == QDialog::Rejected)
        return;

    int width = VCSlider::defaultSize.width();
    int height = avsm.height();
    int count = avsm.amount();

    VCFrame* frame = new VCFrame(parent, m_doc);
    Q_ASSERT(frame != NULL);

    // Resize the parent frame to fit the sliders nicely
    frame->resize(QSize((count * width) + 20, height + 20));
    frame->setAllowResize(false);

    for (int i = 0; i < count; i++)
    {
        VCSlider* slider = new VCSlider(frame, m_doc);
        Q_ASSERT(slider != NULL);
        slider->move(QPoint(10 + (width * i), 10));
        slider->resize(QSize(width, height));
        slider->show();
    }

    // Show the frame after adding buttons to prevent flickering
    frame->show();
    frame->move(parent->lastClickPoint());
    frame->setAllowChildren(false); // Don't allow more children
    clearWidgetSelection();
    setWidgetSelected(frame, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddSpeedDial()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCSpeedDial* dial = new VCSpeedDial(parent, m_doc);
    Q_ASSERT(dial != NULL);
    dial->show();
    dial->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(dial, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddXYPad()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCXYPad* xypad = new VCXYPad(parent, m_doc);
    Q_ASSERT(xypad != NULL);
    xypad->show();
    xypad->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(xypad, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddCueList()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCCueList* cuelist = new VCCueList(parent, m_doc);
    Q_ASSERT(cuelist != NULL);
    cuelist->show();
    cuelist->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(cuelist, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddFrame()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCFrame* frame = new VCFrame(parent, m_doc, true);
    Q_ASSERT(frame != NULL);
    frame->show();
    frame->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(frame, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddSoloFrame()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCSoloFrame* soloframe = new VCSoloFrame(parent, m_doc, true);
    Q_ASSERT(soloframe != NULL);
    soloframe->show();
    soloframe->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(soloframe, true);
    m_doc->setModified();
}

void VirtualConsole::slotAddLabel()
{
    VCWidget* parent(closestParent());
    if (parent == NULL)
        return;

    VCLabel* label = new VCLabel(parent, m_doc);
    Q_ASSERT(label != NULL);
    label->show();
    label->move(parent->lastClickPoint());
    clearWidgetSelection();
    setWidgetSelected(label, true);
    m_doc->setModified();
}

/*****************************************************************************
 * Tools menu callbacks
 *****************************************************************************/

void VirtualConsole::slotToolsSettings()
{
    VCPropertiesEditor vcpe(this, m_properties, m_doc->inputMap());
    if (vcpe.exec() == QDialog::Accepted)
    {
        m_properties = vcpe.properties();
        contents()->resize(m_properties.size());
        m_doc->outputMap()->setGrandMasterChannelMode(m_properties.grandMasterChannelMode());
        m_doc->outputMap()->setGrandMasterValueMode(m_properties.grandMasterValueMode());
        m_doc->setModified();
    }
}

/*****************************************************************************
 * Edit menu callbacks
 *****************************************************************************/

void VirtualConsole::slotEditCut()
{
    /* No need to delete widgets in clipboard because they are actually just
       MOVED to another parent during Paste when m_editAction == EditCut.
       Cutting the widgets does nothing to them unless Paste is invoked. */

    /* Make the edit action valid only if there's something to cut */
    if (m_selectedWidgets.size() == 0)
    {
        m_editAction = EditNone;
        m_clipboard.clear();
        m_editPasteAction->setEnabled(false);
    }
    else
    {
        m_editAction = EditCut;
        m_clipboard = m_selectedWidgets;
        m_editPasteAction->setEnabled(true);
    }

    updateActions();
}

void VirtualConsole::slotEditCopy()
{
    /* Make the edit action valid only if there's something to copy */
    if (m_selectedWidgets.size() == 0)
    {
        m_editAction = EditNone;
        m_clipboard.clear();
        m_editPasteAction->setEnabled(false);
    }
    else
    {
        m_editAction = EditCopy;
        m_clipboard = m_selectedWidgets;
        m_editPasteAction->setEnabled(true);
    }
}

void VirtualConsole::slotEditPaste()
{
    if (m_clipboard.size() == 0)
    {
        /* Invalidate the edit action if there's nothing to paste */
        m_editAction = EditNone;
        m_editPasteAction->setEnabled(false);
        return;
    }

    VCWidget* parent;
    VCWidget* widget;
    QRect bounds;

    Q_ASSERT(contents() != NULL);

    /* Select the parent that gets the cut clipboard contents */
    parent = closestParent();

    /* Get the bounding rect for all selected widgets */
    QListIterator <VCWidget*> it(m_clipboard);
    while (it.hasNext() == true)
    {
        widget = it.next();
        Q_ASSERT(widget != NULL);
        bounds = bounds.united(widget->geometry());
    }

    /* Get the upcoming parent's last mouse click point */
    QPoint cp(parent->lastClickPoint());

    if (m_editAction == EditCut)
    {
        it.toFront();
        while (it.hasNext() == true)
        {
            widget = it.next();
            Q_ASSERT(widget != NULL);
            if (widget == parent)
                continue;

            /* Get widget's relative pos to the bounding rect */
            QPoint p(widget->x() - bounds.x() + cp.x(),
                     widget->y() - bounds.y() + cp.y());

            /* Reparent and move to the correct place */
            widget->setParent(parent);
            widget->move(p);
            widget->show();
        }

        /* Clear clipboard after pasting stuff that was CUT */
        m_clipboard.clear();
        m_editPasteAction->setEnabled(false);
    }
    else if (m_editAction == EditCopy)
    {
        it.toFront();
        while (it.hasNext() == true)
        {
            widget = it.next();
            Q_ASSERT(widget != NULL);
            if (widget == parent)
                continue;

            /* Get widget's relative pos to the bounding rect */
            QPoint p(widget->x() - bounds.x() + cp.x(),
                     widget->y() - bounds.y() + cp.y());

            /* Create a copy and move to correct place */
            VCWidget* copy = widget->createCopy(parent);
            Q_ASSERT(copy != NULL);
            copy->move(p);
            copy->show();
        }
    }

    updateActions();
}

void VirtualConsole::slotEditDelete()
{
    QString msg(tr("Do you wish to delete the selected widgets?"));
    QString title(tr("Delete widgets"));
    int result = QMessageBox::question(this, title, msg,
                                       QMessageBox::Yes,
                                       QMessageBox::No);
    if (result == QMessageBox::Yes)
    {
        while (m_selectedWidgets.isEmpty() == false)
        {
            /* Consume the selected list until it is empty and
               delete each widget. */
            VCWidget* widget = m_selectedWidgets.takeFirst();
            widget->deleteLater();

            /* Remove the widget from clipboard as well so that
               deleted widgets won't be pasted anymore anywhere */
            m_clipboard.removeAll(widget);
            m_editPasteAction->setEnabled(false);
        }

        updateActions();
    }
}

void VirtualConsole::slotEditProperties()
{
    VCWidget* widget;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        widget = contents();
    else
        widget = m_selectedWidgets.last();

    if (widget != NULL)
        widget->editProperties();
}

void VirtualConsole::slotEditRename()
{
    if (m_selectedWidgets.isEmpty() == true)
        return;

    bool ok = false;
    QString text(m_selectedWidgets.last()->caption());
    text = QInputDialog::getText(this, tr("Rename widgets"), tr("Caption:"),
                                 QLineEdit::Normal, text, &ok);
    if (ok == true)
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->setCaption(text);
    }
}

/*****************************************************************************
 * Background menu callbacks
 *****************************************************************************/

void VirtualConsole::slotBackgroundColor()
{
    QColor color;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        color = contents()->backgroundColor();
    else
        color = m_selectedWidgets.last()->backgroundColor();

    color = QColorDialog::getColor(color);
    if (color.isValid() == true)
    {
        if (m_selectedWidgets.isEmpty() == true)
        {
            contents()->setBackgroundColor(color);
        }
        else
        {
            VCWidget* widget;
            foreach(widget, m_selectedWidgets)
                widget->setBackgroundColor(color);
        }
    }
}

void VirtualConsole::slotBackgroundImage()
{
    QString path;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        path = contents()->backgroundImage();
    else
        path = m_selectedWidgets.last()->backgroundImage();

    path = QFileDialog::getOpenFileName(this,
                                        tr("Select background image"),
                                        path,
                                        "Images (*.png *.xpm *.jpg *.gif)");
    if (path.isEmpty() == false)
    {
        if (m_selectedWidgets.isEmpty() == true)
        {
            contents()->setBackgroundImage(path);
        }
        else
        {
            VCWidget* widget;
            foreach(widget, m_selectedWidgets)
                widget->setBackgroundImage(path);
        }
    }
}

void VirtualConsole::slotBackgroundNone()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
    {
        contents()->resetBackgroundColor();
    }
    else
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->resetBackgroundColor();
    }
}

/*****************************************************************************
 * Foreground menu callbacks
 *****************************************************************************/

void VirtualConsole::slotForegroundColor()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    QColor color(m_selectedWidgets.last()->foregroundColor());
    color = QColorDialog::getColor(color);
    if (color.isValid() == true)
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->setForegroundColor(color);
    }
}

void VirtualConsole::slotForegroundNone()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->resetForegroundColor();
}

/*****************************************************************************
 * Font menu callbacks
 *****************************************************************************/

void VirtualConsole::slotFont()
{
    bool ok = false;
    QFont font;

    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        font = contents()->font();
    else
        font = m_selectedWidgets.last()->font();

    /* This crashes with Qt 4.6.x on OSX. Upgrade to 4.7.x. */
    font = QFontDialog::getFont(&ok, font);
    if (ok == true)
    {
        if (m_selectedWidgets.isEmpty() == true)
        {
            contents()->setFont(font);
        }
        else
        {
            VCWidget* widget;
            foreach(widget, m_selectedWidgets)
                widget->setFont(font);
        }
    }
}

void VirtualConsole::slotResetFont()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
    {
        contents()->resetFont();
    }
    else
    {
        VCWidget* widget;
        foreach(widget, m_selectedWidgets)
            widget->resetFont();
    }
}

/*****************************************************************************
 * Stacking menu callbacks
 *****************************************************************************/

void VirtualConsole::slotStackingRaise()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->raise();
}

void VirtualConsole::slotStackingLower()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->lower();
}

/*****************************************************************************
 * Frame menu callbacks
 *****************************************************************************/

void VirtualConsole::slotFrameSunken()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->setFrameStyle(KVCFrameStyleSunken);
}

void VirtualConsole::slotFrameRaised()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->setFrameStyle(KVCFrameStyleRaised);
}

void VirtualConsole::slotFrameNone()
{
    Q_ASSERT(contents() != NULL);

    if (m_selectedWidgets.isEmpty() == true)
        return;

    VCWidget* widget;
    foreach(widget, m_selectedWidgets)
        widget->setFrameStyle(KVCFrameStyleNone);
}

/*****************************************************************************
 * Dock area
 *****************************************************************************/

VCDockArea* VirtualConsole::dockArea() const
{
    return m_dockArea;
}

void VirtualConsole::initDockArea()
{
    if (m_dockArea != NULL)
        delete m_dockArea;

    m_dockArea = new VCDockArea(this, m_doc->outputMap(), m_doc->inputMap());
    m_dockArea->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

    // Add the dock area into the master horizontal layout
    layout()->addWidget(m_dockArea);

    /* Show the dock area by default */
    m_dockArea->show();
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

VCFrame* VirtualConsole::contents() const
{
    return m_contents;
}

void VirtualConsole::resetContents()
{
    if (m_contents != NULL)
        delete m_contents;

    Q_ASSERT(m_scrollArea != NULL);
    m_contents = new VCFrame(m_scrollArea, m_doc);
    m_contents->setFrameStyle(0);

    // Get virtual console size from properties
    QSize size(m_properties.size());
    contents()->resize(size);
    contents()->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    m_scrollArea->setWidget(contents());

    /* Disconnect old key handlers to prevent duplicates */
    disconnect(this, SIGNAL(keyPressed(const QKeySequence&)),
               contents(), SLOT(slotKeyPressed(const QKeySequence&)));
    disconnect(this, SIGNAL(keyReleased(const QKeySequence&)),
               contents(), SLOT(slotKeyReleased(const QKeySequence&)));

    /* Connect new key handlers */
    connect(this, SIGNAL(keyPressed(const QKeySequence&)),
            contents(), SLOT(slotKeyPressed(const QKeySequence&)));
    connect(this, SIGNAL(keyReleased(const QKeySequence&)),
            contents(), SLOT(slotKeyReleased(const QKeySequence&)));

    /* Make the contents area take up all available space */
    contents()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_clipboard.clear();
    m_selectedWidgets.clear();

    /* Update actions' enabled status */
    updateActions();

    /* Reset all properties but size */
    m_properties.setTapModifier(Qt::ControlModifier);
    m_properties.setGrandMasterChannelMode(UniverseArray::GMIntensity);
    m_properties.setGrandMasterValueMode(UniverseArray::GMReduce);
    m_properties.setGrandMasterInputSource(InputMap::invalidUniverse(), InputMap::invalidChannel());
}

void VirtualConsole::initContents()
{
    Q_ASSERT(layout() != NULL);

    m_scrollArea = new QScrollArea(this);
    m_contentsLayout->addWidget(m_scrollArea);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setWidgetResizable(false);

    resetContents();
}

/*****************************************************************************
 * Key press handler
 *****************************************************************************/

bool VirtualConsole::isTapModifierDown() const
{
    return m_tapModifierDown;
}

void VirtualConsole::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat() == true)
    {
        event->ignore();
        return;
    }

    if ((event->modifiers() & Qt::ControlModifier) != 0)
        m_tapModifierDown = true;

    QKeySequence seq(event->key() | (event->modifiers() & ~Qt::ControlModifier));
    emit keyPressed(seq);

    event->accept();
}

void VirtualConsole::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat() == true)
    {
        event->ignore();
        return;
    }

    if ((event->modifiers() & Qt::ControlModifier) == 0)
        m_tapModifierDown = false;

    QKeySequence seq(event->key() | event->modifiers());
    emit keyReleased(seq);

    event->accept();
}

/*****************************************************************************
 * Main application mode
 *****************************************************************************/

void VirtualConsole::slotModeChanged(Doc::Mode mode)
{
    QString config;

    if (mode == Doc::Operate)
    {
        // Don't allow editing or adding in operate mode
        m_toolsSettingsAction->setEnabled(false);
        m_editActionGroup->setEnabled(false);
        m_addActionGroup->setEnabled(false);
        m_bgActionGroup->setEnabled(false);
        m_fgActionGroup->setEnabled(false);
        m_fontActionGroup->setEnabled(false);
        m_frameActionGroup->setEnabled(false);
        m_stackingActionGroup->setEnabled(false);

        // Disable action shortcuts in operate mode
        m_addButtonAction->setShortcut(QKeySequence());
        m_addButtonMatrixAction->setShortcut(QKeySequence());
        m_addSliderAction->setShortcut(QKeySequence());
        m_addSliderMatrixAction->setShortcut(QKeySequence());
        m_addSpeedDialAction->setShortcut(QKeySequence());
        m_addXYPadAction->setShortcut(QKeySequence());
        m_addCueListAction->setShortcut(QKeySequence());
        m_addFrameAction->setShortcut(QKeySequence());
        m_addSoloFrameAction->setShortcut(QKeySequence());
        m_addLabelAction->setShortcut(QKeySequence());

        m_editCutAction->setShortcut(QKeySequence());
        m_editCopyAction->setShortcut(QKeySequence());
        m_editPasteAction->setShortcut(QKeySequence());
        m_editDeleteAction->setShortcut(QKeySequence());
        m_editPropertiesAction->setShortcut(QKeySequence());

        m_bgColorAction->setShortcut(QKeySequence());
        m_bgImageAction->setShortcut(QKeySequence());
        m_bgDefaultAction->setShortcut(QKeySequence());
        m_fgColorAction->setShortcut(QKeySequence());
        m_fgDefaultAction->setShortcut(QKeySequence());
        m_fontAction->setShortcut(QKeySequence());
        m_resetFontAction->setShortcut(QKeySequence());
        m_frameSunkenAction->setShortcut(QKeySequence());
        m_frameRaisedAction->setShortcut(QKeySequence());
        m_frameNoneAction->setShortcut(QKeySequence());

        m_stackingRaiseAction->setShortcut(QKeySequence());
        m_stackingLowerAction->setShortcut(QKeySequence());

        // Hide toolbar; there's nothing usable there in operate mode
        m_toolbar->hide();
    }
    else
    {
        // Allow editing and adding in design mode
        m_toolsSettingsAction->setEnabled(true);
        m_editActionGroup->setEnabled(true);
        m_addActionGroup->setEnabled(true);
        m_bgActionGroup->setEnabled(true);
        m_fgActionGroup->setEnabled(true);
        m_fontActionGroup->setEnabled(true);
        m_frameActionGroup->setEnabled(true);
        m_stackingActionGroup->setEnabled(true);

        // Set action shortcuts for design mode
        m_addButtonAction->setShortcut(QKeySequence("CTRL+SHIFT+B"));
        m_addButtonMatrixAction->setShortcut(QKeySequence("CTRL+SHIFT+M"));
        m_addSliderAction->setShortcut(QKeySequence("CTRL+SHIFT+S"));
        m_addSliderMatrixAction->setShortcut(QKeySequence("CTRL+SHIFT+I"));
        m_addSpeedDialAction->setShortcut(QKeySequence("CTRL+SHIFT+D"));
        m_addXYPadAction->setShortcut(QKeySequence("CTRL+SHIFT+X"));
        m_addCueListAction->setShortcut(QKeySequence("CTRL+SHIFT+C"));
        m_addFrameAction->setShortcut(QKeySequence("CTRL+SHIFT+F"));
        m_addSoloFrameAction->setShortcut(QKeySequence("CTRL+SHIFT+O"));
        m_addLabelAction->setShortcut(QKeySequence("CTRL+SHIFT+L"));

        m_editCutAction->setShortcut(QKeySequence("CTRL+X"));
        m_editCopyAction->setShortcut(QKeySequence("CTRL+C"));
        m_editPasteAction->setShortcut(QKeySequence("CTRL+V"));
        m_editDeleteAction->setShortcut(QKeySequence("Delete"));
        m_editPropertiesAction->setShortcut(QKeySequence("CTRL+E"));

        m_bgColorAction->setShortcut(QKeySequence("SHIFT+B"));
        m_bgImageAction->setShortcut(QKeySequence("SHIFT+I"));
        m_bgDefaultAction->setShortcut(QKeySequence("SHIFT+ALT+B"));
        m_fgColorAction->setShortcut(QKeySequence("SHIFT+F"));
        m_fgDefaultAction->setShortcut(QKeySequence("SHIFT+ALT+F"));
        m_fontAction->setShortcut(QKeySequence("SHIFT+O"));
        m_resetFontAction->setShortcut(QKeySequence("SHIFT+ALT+O"));
        m_frameSunkenAction->setShortcut(QKeySequence("SHIFT+S"));
        m_frameRaisedAction->setShortcut(QKeySequence("SHIFT+R"));
        m_frameNoneAction->setShortcut(QKeySequence("SHIFT+ALT+S"));

        m_stackingRaiseAction->setShortcut(QKeySequence("SHIFT+UP"));
        m_stackingLowerAction->setShortcut(QKeySequence("SHIFT+DOWN"));

        // Show toolbar
        m_toolbar->show();
    }
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
        if (tag.tagName() == KXMLQLCVCProperties)
        {
            /* Properties */
            m_properties.loadXML(tag);
        }
        else if (tag.tagName() == KXMLQLCVCFrame)
        {
            /* Contents */
            Q_ASSERT(m_contents != NULL);
            m_contents->loadXML(&tag);
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

bool VirtualConsole::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Virtual Console entry */
    QDomElement vc_root = doc->createElement(KXMLQLCVirtualConsole);
    wksp_root->appendChild(vc_root);

    /* Contents */
    Q_ASSERT(m_contents != NULL);
    m_contents->saveXML(doc, &vc_root);

    /* Properties */
    m_properties.saveXML(doc, &vc_root);

    return true;
}

void VirtualConsole::postLoad()
{
    m_contents->postLoad();

    /* apply GM values
      this should probably be placed in another place, but at the moment m_properties
      is just loaded in VirtualConsole */
    m_doc->outputMap()->setGrandMasterValue(255);
    m_doc->outputMap()->setGrandMasterValueMode(m_properties.grandMasterValueMode());
    m_doc->outputMap()->setGrandMasterChannelMode(m_properties.grandMasterChannelMode());
}
