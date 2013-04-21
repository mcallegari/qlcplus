/*
  Q Light Controller
  functionmanager.cpp

  Copyright (C) Heikki Junnila

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

#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSplitter>
#include <QSettings>
#include <QToolBar>
#include <QMenuBar>
#include <QPixmap>
#include <QDebug>
#include <QMenu>
#include <QList>
#include <QIcon>

#include "collectioneditor.h"
#include "functionmanager.h"
#include "rgbmatrixeditor.h"
#include "functionwizard.h"
#include "chasereditor.h"
#include "scripteditor.h"
#include "sceneeditor.h"
#include "showeditor.h"
#include "chaserstep.h"
#include "collection.h"
#include "efxeditor.h"
#include "rgbmatrix.h"
#include "function.h"
#include "apputil.h"
#include "chaser.h"
#include "script.h"
#include "scene.h"
#include "show.h"
#include "doc.h"
#include "efx.h"

#define PROP_ID Qt::UserRole
#define COL_NAME 0

#define SETTINGS_SPLITTER "functionmanager/splitter"

FunctionManager* FunctionManager::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionManager::FunctionManager(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_hsplitter(NULL)
    , m_vsplitter(NULL)
    , m_tree(NULL)
    , m_toolbar(NULL)
    , m_addSceneAction(NULL)
    , m_addChaserAction(NULL)
    , m_addCollectionAction(NULL)
    , m_addEFXAction(NULL)
    , m_addRGBMatrixAction(NULL)
    , m_addScriptAction(NULL)
    , m_wizardAction(NULL)
    , m_cloneAction(NULL)
    , m_deleteAction(NULL)
    , m_selectAllAction(NULL)
    , m_editor(NULL)
    , m_scene_editor(NULL)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(doc != NULL);

    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    initActions();
    initToolbar();
    initSplitterView();
    updateActionStatus();

    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged()));
    updateTree();

    m_tree->sortItems(COL_NAME, Qt::AscendingOrder);

    connect(m_doc, SIGNAL(clearing()), this, SLOT(slotDocClearing()));
    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SLOT(slotFunctionAdded(quint32)));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_hsplitter->restoreState(var.toByteArray());
    else
        m_hsplitter->setSizes(QList <int> () << int(this->width() / 2) << int(this->width() / 2));
}

FunctionManager::~FunctionManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_hsplitter->saveState());

    FunctionManager::s_instance = NULL;
}

FunctionManager* FunctionManager::instance()
{
    return s_instance;
}

void FunctionManager::slotModeChanged()
{
    updateActionStatus();
}

void FunctionManager::slotDocClearing()
{
    deleteCurrentEditor();
    m_tree->clear();
}

void FunctionManager::slotDocLoaded()
{
    // Once the doc is completely loaded, update all the steps of Chasers acting like sequences
    foreach (Function *f, m_doc->functionsByType(Function::Chaser))
    {
        Chaser *chaser = qobject_cast<Chaser *>(f);
        if (chaser->isSequence() && chaser->getBoundSceneID() != Scene::invalidId())
        {
            Function *f = m_doc->function(chaser->getBoundSceneID());
            Scene *s = qobject_cast<Scene*>(f);
            int i = 0;
            foreach(ChaserStep step, chaser->steps())
            {
                if (step.values.count() > 0)
                {
                    // Since I saved only the non-zero values in the XML files, at the first chance I need
                    // to fix the values against the bound scene, and restore all the zero values previously there
                    //qDebug() << Q_FUNC_INFO << "Scene values: " << s->values().count() << ", step values: " <<  step.values.count();
                    if (s->values().count() != step.values.count())
                    {
                        int j = 0;
                        // 1- copy the list
                        QList <SceneValue> tmpList = step.values;
                        // 2- clear it
                        step.values.clear();
                        // 3- fix it
                        QListIterator <SceneValue> it(s->values());
                        while (it.hasNext() == true)
                        {
                            SceneValue scv(it.next());
                            scv.value = 0;
                            if (j < tmpList.count() && tmpList.at(j) == scv)
                                step.values.append(tmpList.at(j++));
                            else
                                step.values.append(scv);
                        }
                        chaser->replaceStep(step, i);
                        //qDebug() << "************ STEP FIXED *********** total values: " << step.values.count();
                        i++;
                    }
                }
            }
        }
    }
}

void FunctionManager::slotFunctionChanged(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function == NULL)
        return;

    QTreeWidgetItem* item = functionItem(function);
    if (item != NULL)
        updateFunctionItem(item, function);
}

void FunctionManager::slotFunctionAdded(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function == NULL)
        return;

    QTreeWidgetItem* item = new QTreeWidgetItem(parentItem(function));
    updateFunctionItem(item, function);
}

void FunctionManager::showEvent(QShowEvent* ev)
{
    qDebug() << Q_FUNC_INFO;
    emit functionManagerActive(true);
    QWidget::showEvent(ev);
    updateTree();
}

void FunctionManager::hideEvent(QHideEvent* ev)
{
    qDebug() << Q_FUNC_INFO;
    emit functionManagerActive(false);
    QWidget::hideEvent(ev);
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FunctionManager::initActions()
{
    /* Manage actions */
    m_addSceneAction = new QAction(QIcon(":/scene.png"),
                                   tr("New &scene"), this);
    m_addSceneAction->setShortcut(QKeySequence("CTRL+S"));
    connect(m_addSceneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddScene()));

    m_addChaserAction = new QAction(QIcon(":/chaser.png"),
                                    tr("New c&haser"), this);
    m_addChaserAction->setShortcut(QKeySequence("CTRL+H"));
    connect(m_addChaserAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddChaser()));

    m_addSequenceAction = new QAction(QIcon(":/sequence.png"),
                                    tr("New se&quence"), this);
    m_addSequenceAction->setShortcut(QKeySequence("CTRL+Q"));
    connect(m_addSequenceAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSequence()));
    m_addSequenceAction->setEnabled(false);

    m_addCollectionAction = new QAction(QIcon(":/collection.png"),
                                        tr("New c&ollection"), this);
    m_addCollectionAction->setShortcut(QKeySequence("CTRL+O"));
    connect(m_addCollectionAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddCollection()));

    m_addEFXAction = new QAction(QIcon(":/efx.png"),
                                 tr("New E&FX"), this);
    m_addEFXAction->setShortcut(QKeySequence("CTRL+F"));
    connect(m_addEFXAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddEFX()));

    m_addRGBMatrixAction = new QAction(QIcon(":/rgbmatrix.png"),
                                 tr("New &RGB Matrix"), this);
    m_addRGBMatrixAction->setShortcut(QKeySequence("CTRL+R"));
    connect(m_addRGBMatrixAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddRGBMatrix()));

    m_addScriptAction = new QAction(QIcon(":/script.png"),
                                 tr("New scrip&t"), this);
    m_addScriptAction->setShortcut(QKeySequence("CTRL+T"));
    connect(m_addScriptAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddScript()));

    m_wizardAction = new QAction(QIcon(":/wizard.png"),
                                 tr("Function Wizard"), this);
    m_wizardAction->setShortcut(QKeySequence("CTRL+A"));
    connect(m_wizardAction, SIGNAL(triggered(bool)),
            this, SLOT(slotWizard()));

    /* Edit actions */
    m_cloneAction = new QAction(QIcon(":/editcopy.png"),
                                tr("&Clone"), this);
    m_cloneAction->setShortcut(QKeySequence("CTRL+C"));
    connect(m_cloneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotClone()));

    m_deleteAction = new QAction(QIcon(":/editdelete.png"),
                                 tr("&Delete"), this);
    m_deleteAction->setShortcut(QKeySequence("Delete"));
    connect(m_deleteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDelete()));

    m_selectAllAction = new QAction(QIcon(":/selectall.png"),
                                    tr("Select &all"), this);
    m_selectAllAction->setShortcut(QKeySequence("CTRL+A"));
    connect(m_selectAllAction, SIGNAL(triggered(bool)),
            this, SLOT(slotSelectAll()));
}

void FunctionManager::initToolbar()
{
    // Add a toolbar to the dock area
    m_toolbar = new QToolBar("Function Manager", this);
    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    layout()->addWidget(m_toolbar);
    m_toolbar->addAction(m_addSceneAction);
    m_toolbar->addAction(m_addChaserAction);
    m_toolbar->addAction(m_addSequenceAction);
    m_toolbar->addAction(m_addEFXAction);
    m_toolbar->addAction(m_addCollectionAction);
    m_toolbar->addAction(m_addRGBMatrixAction);
    m_toolbar->addAction(m_addScriptAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_wizardAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_cloneAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_deleteAction);
}

void FunctionManager::slotAddScene()
{
    Function* f = new Scene(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = functionItem(f);
        Q_ASSERT(item != NULL);
        f->setName(QString("%1 %2").arg(tr("New Scene")).arg(f->id()));
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddChaser()
{
    Function* f = new Chaser(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = functionItem(f);
        Q_ASSERT(item != NULL);
        f->setName(QString("%1 %2").arg(tr("New Chaser")).arg(f->id()));
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddSequence()
{
    Function* f = new Chaser(m_doc);
    QList <QTreeWidgetItem*> selection(m_tree->selectedItems());
    if (selection.size() == 1)
    {
        Chaser *chs = qobject_cast<Chaser*>(f);
        Scene *boundScene = qobject_cast<Scene*>(m_doc->function(itemFunctionId(selection.first())));
        boundScene->setChildrenFlag(true);
        chs->enableSequenceMode(itemFunctionId(selection.first()));
        chs->setRunOrder(Function::SingleShot);
    }

    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = functionItem(f);
        Q_ASSERT(item != NULL);
        f->setName(QString("%1 %2").arg(tr("New Sequence")).arg(f->id()));
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddCollection()
{
    Function* f = new Collection(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = functionItem(f);
        Q_ASSERT(item != NULL);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddEFX()
{
    Function* f = new EFX(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = functionItem(f);
        Q_ASSERT(item != NULL);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddRGBMatrix()
{
    Function* f = new RGBMatrix(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = functionItem(f);
        Q_ASSERT(item != NULL);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddScript()
{
    Function* f = new Script(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = functionItem(f);
        Q_ASSERT(item != NULL);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotWizard()
{
    FunctionWizard fw(this, m_doc);
    if (fw.exec() == QDialog::Accepted)
        updateTree();
}

void FunctionManager::slotClone()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        copyFunction(itemFunctionId(it.next()));
}

void FunctionManager::slotDelete()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    if (it.hasNext() == false)
        return;

    QString msg = tr("Do you want to DELETE functions:") + QString("\n");

    // Append functions' names to the message
    while (it.hasNext() == true)
        msg += it.next()->text(COL_NAME) + QString(", ");

    // Ask for user's confirmation
    if (QMessageBox::question(this, tr("Delete Functions"), msg,
                              QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::Yes)
    {
        deleteSelectedFunctions();
        updateActionStatus();
        deleteCurrentEditor();
    }
}

void FunctionManager::slotSelectAll()
{
    /* This has to be done thru an intermediary slot because the tree
       widget hasn't been created when actions are being created and
       so a direct slot collection to m_tree is not possible. */
    m_tree->selectAll();
}

void FunctionManager::updateActionStatus()
{
    if (m_tree->selectedItems().isEmpty() == false)
    {
        /* At least one function has been selected, so
           editing is possible. */
        m_cloneAction->setEnabled(true);
        m_selectAllAction->setEnabled(true);
        if (m_doc->mode() == Doc::Operate)
            m_deleteAction->setEnabled(false);
        else
            m_deleteAction->setEnabled(true);
    }
    else
    {
        /* No functions selected */
        m_cloneAction->setEnabled(false);
        m_selectAllAction->setEnabled(false);
        m_deleteAction->setEnabled(false);
    }
}

/****************************************************************************
 * Function tree
 ****************************************************************************/

void FunctionManager::initSplitterView()
{
    m_vsplitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(m_vsplitter);
    // add container for tree view + right editor
    QWidget* gcontainer = new QWidget(this);
    m_vsplitter->addWidget(gcontainer);
    gcontainer->setLayout(new QVBoxLayout);
    gcontainer->layout()->setContentsMargins(0, 0, 0, 0);

    // add container for scene editor
    QWidget* scontainer = new QWidget(this);
    m_vsplitter->addWidget(scontainer);
    scontainer->setLayout(new QVBoxLayout);
    scontainer->layout()->setContentsMargins(0, 0, 0, 0);
    scontainer->hide();

    m_hsplitter = new QSplitter(Qt::Horizontal, this);
    //layout()->addWidget(m_hsplitter);
    m_vsplitter->widget(0)->layout()->addWidget(m_hsplitter);
    initTree();

    QWidget* container = new QWidget(this);
    m_hsplitter->addWidget(container);
    container->setLayout(new QVBoxLayout);
    container->layout()->setContentsMargins(0, 0, 0, 0);
    container->hide();
}

void FunctionManager::initTree()
{
    m_tree = new QTreeWidget(this);
    Q_ASSERT(m_hsplitter != NULL);
    m_hsplitter->addWidget(m_tree);

    // Add two columns for function and type
    QStringList labels;
    labels << tr("Function");
    m_tree->setHeaderLabels(labels);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    m_tree->setRootIsDecorated(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setSortingEnabled(true);
    m_tree->sortByColumn(COL_NAME, Qt::AscendingOrder);

    // Catch selection changes
    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotTreeSelectionChanged()));

    // Catch right-mouse clicks
    connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotTreeContextMenuRequested()));
}

void FunctionManager::updateTree()
{
    m_tree->clear();
    foreach (Function* function, m_doc->functions())
        updateFunctionItem(new QTreeWidgetItem(parentItem(function)), function);
    /** Set hasChildren flag for scenes with children */
    for (int t = 0; t < m_tree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *topLevelItem = m_tree->topLevelItem(t);
        for (int f = 0; f < topLevelItem->childCount(); f++)
        {
            QTreeWidgetItem *item = topLevelItem->child(f);
            Function *function = m_doc->function(itemFunctionId(item));

            if (function != NULL && function->type() == Function::Scene &&
                item->childCount() > 0)
            {
                Scene *scene = qobject_cast<Scene*>(function);
                scene->setChildrenFlag(true);
                qDebug() << Q_FUNC_INFO << "Scene -" << function->name() << "has children !";
            }
        }
    }
}

void FunctionManager::selectFunction(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function == NULL)
        return;
    QTreeWidgetItem* item = functionItem(function);
    if (item != NULL)
        m_tree->setCurrentItem(item);
}

void FunctionManager::updateFunctionItem(QTreeWidgetItem* item, const Function* function)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(function != NULL);
    item->setText(COL_NAME, function->name());
    item->setIcon(COL_NAME, functionIcon(function));
    item->setData(COL_NAME, PROP_ID, function->id());
}

QTreeWidgetItem* FunctionManager::parentItem(const Function* function)
{
    Q_ASSERT(function != NULL);

    // Search for a parent item for function->type()
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);
        QVariant var = item->data(COL_NAME, Qt::UserRole);
        if (var.isValid() == false)
            continue;
        Function::Type type = (Function::Type) var.toInt();
        if (function->type() == Function::Chaser && qobject_cast<const Chaser*>(function)->isSequence() == true)
        {
            if (type == Function::Scene)
            {
                quint32 sid = qobject_cast<const Chaser*>(function)->getBoundSceneID();
                for (int c = 0; c < item->childCount(); c++)
                {
                    QTreeWidgetItem* child = item->child(c);
                    QVariant var2 = child->data(COL_NAME, Qt::UserRole);

                    if (var2.isValid() == false)
                        continue;
                    quint32 cid = var2.toInt();

                    if (sid == cid)
                        return child;
                }
            }
        }
        else
        {
            if (type == function->type())
                return item;
        }
    }

    // Parent item for the given type doesn't exist yet so create one
    QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
    item->setText(COL_NAME, Function::typeToString(function->type()));
    item->setIcon(COL_NAME, functionIcon(function));
    item->setData(COL_NAME, Qt::UserRole, function->type());
    item->setFlags(Qt::ItemIsEnabled);
    return item;
}

quint32 FunctionManager::itemFunctionId(const QTreeWidgetItem* item) const
{
    if (item == NULL || item->parent() == NULL)
        return Function::invalidId();
    else
        return item->data(COL_NAME, PROP_ID).toUInt();
}

QTreeWidgetItem* FunctionManager::functionItem(const Function* function)
{
    Q_ASSERT(function != NULL);

    QTreeWidgetItem* parent = parentItem(function);
    Q_ASSERT(parent != NULL);

    for (int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem* item = parent->child(i);
        if (itemFunctionId(item) == function->id())
            return item;
    }

    return NULL;
}

QIcon FunctionManager::functionIcon(const Function* function) const
{
    switch (function->type())
    {
    case Function::Scene:
        return QIcon(":/scene.png");
    case Function::Chaser:
        if (qobject_cast<const Chaser*>(function)->isSequence() == true)
            return QIcon(":/sequence.png");
        else
            return QIcon(":/chaser.png");
    case Function::EFX:
        return QIcon(":/efx.png");
    case Function::Collection:
        return QIcon(":/collection.png");
    case Function::RGBMatrix:
        return QIcon(":/rgbmatrix.png");
    case Function::Script:
        return QIcon(":/script.png");
    case Function::Show:
        return QIcon(":/show.png");
    case Function::Audio:
        return QIcon(":/audio.png");
    default:
        return QIcon(":/function.png");
    }
}

void FunctionManager::deleteSelectedFunctions()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        quint32 fid = itemFunctionId(item);
        m_doc->deleteFunction(fid);

        QTreeWidgetItem* parent = item->parent();
        delete item;
        if (parent != NULL && parent->childCount() == 0)
            delete parent;
    }
/*
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        quint32 fid = itemFunctionId(item);

        Function *f = m_doc->function(fid);
        if (f != NULL && f->type() == Function::Scene)
        {
            foreach (Function* function, m_doc->functions())
            {
                // Search for Show tracks associated to Scenes
                if (function->type() == Function::Show)
                {
                    Show *show = qobject_cast<Show*>(function);
                    foreach (Track *track, show->tracks())
                    {
                        if (track->getSceneID() == fid)
                            show->removeTrack(track->id());
                    }
                }
                // Search for Sequences associated to Scenes
                if (function->type() == Function::Chaser)
                {
                    Chaser *chaser = qobject_cast<Chaser*>(function);
                    if (chaser->isSequence() && chaser->getBoundSceneID() == fid)
                    {
                        m_doc->deleteFunction(chaser->id());
                    }
                }
            }
        }

        m_doc->deleteFunction(fid);

        QTreeWidgetItem* parent = item->parent();
        delete item;
        if (parent != NULL && parent->childCount() == 0)
            delete parent;
    }
*/
}

void FunctionManager::slotTreeSelectionChanged()
{
    updateActionStatus();

    QList <QTreeWidgetItem*> selection(m_tree->selectedItems());
    if (selection.size() == 1)
    {
        Function* function = m_doc->function(itemFunctionId(selection.first()));
        if (function != NULL)
            editFunction(function);
    }
    else
    {
        deleteCurrentEditor();
    }
}

void FunctionManager::slotTreeContextMenuRequested()
{
    QMenu menu(this);
    menu.addAction(m_cloneAction);
    menu.addAction(m_selectAllAction);
    menu.addSeparator();
    menu.addAction(m_deleteAction);
    menu.addSeparator();
    menu.addAction(m_addSceneAction);
    menu.addAction(m_addChaserAction);
    menu.addAction(m_addEFXAction);
    menu.addAction(m_addCollectionAction);
    menu.addAction(m_addRGBMatrixAction);
    menu.addAction(m_addScriptAction);
    menu.addSeparator();
    menu.addAction(m_wizardAction);

    updateActionStatus();

    menu.exec(QCursor::pos());
}

/*****************************************************************************
 * Helpers
 *****************************************************************************/

void FunctionManager::copyFunction(quint32 fid)
{
    Function* function = m_doc->function(fid);
    Q_ASSERT(function != NULL);

    /* Attempt to create a copy of the function to Doc */
    Function* copy = function->createCopy(m_doc);
    if (copy != NULL)
    {
        copy->setName(tr("Copy of %1").arg(function->name()));
        QTreeWidgetItem* item = functionItem(copy);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::editFunction(Function* function)
{
    deleteCurrentEditor();

    m_addSequenceAction->setEnabled(false);

    if (function == NULL)
        return;

    // Choose the editor by the selected function's type
    if (function->type() == Function::Scene)
    {
        m_scene_editor = new SceneEditor(m_vsplitter->widget(1), qobject_cast<Scene*> (function), m_doc, true);
        connect(this, SIGNAL(functionManagerActive(bool)),
                m_scene_editor, SLOT(slotFunctionManagerActive(bool)));
        m_addSequenceAction->setEnabled(true);
    }
    else if (function->type() == Function::Chaser)
    {
        Chaser *chaser = qobject_cast<Chaser*> (function);
        m_editor = new ChaserEditor(m_hsplitter->widget(1), chaser, m_doc);
        connect(this, SIGNAL(functionManagerActive(bool)),
                m_editor, SLOT(slotFunctionManagerActive(bool)));
        if (chaser->isSequence() == true)
        {
            Function* sfunc = m_doc->function(chaser->getBoundSceneID());
            m_scene_editor = new SceneEditor(m_vsplitter->widget(1), qobject_cast<Scene*> (sfunc), m_doc, false);
            connect(this, SIGNAL(functionManagerActive(bool)),
                    m_scene_editor, SLOT(slotFunctionManagerActive(bool)));
            /** Signal from chaser editor to scene editor. When a step is clicked apply values immediately */
            connect(m_editor, SIGNAL(applyValues(QList<SceneValue>&)),
                    m_scene_editor, SLOT(slotSetSceneValues(QList <SceneValue>&)));
            /** Signal from scene editor to chaser editor. When a fixture value is changed, update the selected chaser step */
            connect(m_scene_editor, SIGNAL(fixtureValueChanged(SceneValue)),
                    m_editor, SLOT(slotUpdateCurrentStep(SceneValue)));
        }
    }
    else if (function->type() == Function::Collection)
    {
        m_editor = new CollectionEditor(m_hsplitter->widget(1), qobject_cast<Collection*> (function), m_doc);
    }
    else if (function->type() == Function::EFX)
    {
        m_editor = new EFXEditor(m_hsplitter->widget(1), qobject_cast<EFX*> (function), m_doc);
        connect(this, SIGNAL(functionManagerActive(bool)),
                m_editor, SLOT(slotFunctionManagerActive(bool)));
    }
    else if (function->type() == Function::RGBMatrix)
    {
        m_editor = new RGBMatrixEditor(m_hsplitter->widget(1), qobject_cast<RGBMatrix*> (function), m_doc);
        connect(this, SIGNAL(functionManagerActive(bool)),
                m_editor, SLOT(slotFunctionManagerActive(bool)));
    }
    else if (function->type() == Function::Script)
    {
        m_editor = new ScriptEditor(m_hsplitter->widget(1), qobject_cast<Script*> (function), m_doc);
    }
    else if (function->type() == Function::Show)
    {
        m_editor = new ShowEditor(m_hsplitter->widget(1), qobject_cast<Show*> (function), m_doc);
    }
    else
    {
        m_editor = NULL;
        m_scene_editor = NULL;
    }

    // Show the editor
    if (m_editor != NULL)
    {
        m_hsplitter->widget(1)->show();
        m_hsplitter->widget(1)->layout()->addWidget(m_editor);
        m_editor->show();
    }
    if (m_scene_editor != NULL)
    {
        m_vsplitter->widget(1)->show();
        m_vsplitter->widget(1)->layout()->addWidget(m_scene_editor);
        m_scene_editor->show();
    }
}

void FunctionManager::deleteCurrentEditor()
{
    if (m_editor != NULL)
        m_editor->deleteLater();
    if (m_scene_editor != NULL)
        m_scene_editor->deleteLater();

    m_editor = NULL;
    m_scene_editor = NULL;

    m_hsplitter->widget(1)->hide();
    m_vsplitter->widget(1)->hide();
}
