/*
  Q Light Controller
  functionmanager.cpp

  Copyright (C) Heikki Junnila
  Copyright (C) Massimo Callegari

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

#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFileDialog>
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

#include "functionstreewidget.h"
#include "functionselection.h"
#include "collectioneditor.h"
#include "functionmanager.h"
#include "rgbmatrixeditor.h"
#include "functionwizard.h"
#include "chasereditor.h"
#include "scripteditor.h"
#include "sceneeditor.h"
#include "audioeditor.h"
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
#include "audio.h"
#include "show.h"
#include "doc.h"
#include "efx.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include "videoeditor.h"
#include "video.h"
#endif

#define COL_NAME 0
#define COL_PATH 1

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
    , m_addAudioAction(NULL)
    , m_addVideoAction(NULL)
    , m_autostartAction(NULL)
    , m_wizardAction(NULL)
    , m_addFolderAction(NULL)
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
    m_tree->updateTree();

    connect(m_doc, SIGNAL(clearing()), this, SLOT(slotDocClearing()));
    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
    connect(m_doc, SIGNAL(functionNameChanged(quint32)), this, SLOT(slotFunctionNameChanged(quint32)));
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
    deleteCurrentEditor(false); // Synchronous delete
    m_tree->clearTree();
}

void FunctionManager::slotDocLoaded()
{
    // Once the doc is completely loaded, update all the steps of Chasers acting like sequences
    foreach (Function *f, m_doc->functionsByType(Function::Chaser))
    {
        Chaser *chaser = qobject_cast<Chaser *>(f);
        if (chaser->isSequence() && chaser->getBoundSceneID() != Scene::invalidId())
        {
            Function *sceneFunc = m_doc->function(chaser->getBoundSceneID());
            if (sceneFunc == NULL || sceneFunc->type() != Function::Scene)
                continue;

            Scene *scene = qobject_cast<Scene*>(sceneFunc);
            scene->setChildrenFlag(true);
            int i = 0;
            int sceneValuesCount = scene->values().count();
            foreach(ChaserStep step, chaser->steps())
            {
                // Since I saved only the non-zero values in the XML files, at the first chance I need
                // to fix the values against the bound scene, and restore all the zero values previously there
                //qDebug() << Q_FUNC_INFO << "Scene values: " << scene->values().count() << ", step values: " <<  step.values.count();
                if (sceneValuesCount != step.values.count())
                {
                    int j = 0;
                    // 1- copy the list
                    QList <SceneValue> tmpList = step.values;
                    // 2- clear it
                    step.values.clear();
                    // 3- fix it
                    QListIterator <SceneValue> it(scene->values());
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
                }
                i++;
            }
        }
    }
}

void FunctionManager::slotFunctionNameChanged(quint32 id)
{
    m_tree->functionNameChanged(id);
}

void FunctionManager::slotFunctionAdded(quint32 id)
{
    m_tree->addFunction(id);
}

void FunctionManager::showEvent(QShowEvent* ev)
{
    qDebug() << Q_FUNC_INFO;
    emit functionManagerActive(true);
    QWidget::showEvent(ev);
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

    m_addAudioAction = new QAction(QIcon(":/audio.png"),
                                   tr("New au&dio"), this);
    m_addAudioAction->setShortcut(QKeySequence("CTRL+D"));
    connect(m_addAudioAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddAudio()));

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    m_addVideoAction = new QAction(QIcon(":/video.png"),
                                   tr("New vid&eo"), this);
    m_addVideoAction->setShortcut(QKeySequence("CTRL+E"));
    connect(m_addVideoAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddVideo()));
#endif
    m_addFolderAction = new QAction(QIcon(":/folder.png"),
                                   tr("New fo&lder"), this);
    m_addFolderAction->setShortcut(QKeySequence("CTRL+L"));
    connect(m_addFolderAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddFolder()));

    m_autostartAction = new QAction(QIcon(":/autostart.png"),
                                    tr("Select Startup Function"), this);
    connect(m_autostartAction, SIGNAL(triggered(bool)),
            this, SLOT(slotSelectAutostartFunction()));

    m_wizardAction = new QAction(QIcon(":/wizard.png"),
                                 tr("Function &Wizard"), this);
    m_wizardAction->setShortcut(QKeySequence("CTRL+W"));
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
    m_toolbar->addAction(m_addAudioAction);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    m_toolbar->addAction(m_addVideoAction);
#endif
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_addFolderAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_autostartAction);
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
        QTreeWidgetItem* item = m_tree->functionItem(f);
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
        QTreeWidgetItem* item = m_tree->functionItem(f);
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
        Scene *boundScene = qobject_cast<Scene*>(m_doc->function(m_tree->itemFunctionId(selection.first())));
        boundScene->setChildrenFlag(true);
        chs->enableSequenceMode(m_tree->itemFunctionId(selection.first()));
        chs->setRunOrder(Function::SingleShot);
    }

    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = m_tree->functionItem(f);
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
        QTreeWidgetItem* item = m_tree->functionItem(f);
        Q_ASSERT(item != NULL);
        f->setName(QString("%1 %2").arg(tr("New Collection")).arg(f->id()));
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddEFX()
{
    Function* f = new EFX(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = m_tree->functionItem(f);
        Q_ASSERT(item != NULL);
        f->setName(QString("%1 %2").arg(tr("New EFX")).arg(f->id()));
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddRGBMatrix()
{
    Function* f = new RGBMatrix(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = m_tree->functionItem(f);
        Q_ASSERT(item != NULL);
        f->setName(QString("%1 %2").arg(tr("New RGB Matrix")).arg(f->id()));
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddScript()
{
    Function* f = new Script(m_doc);
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = m_tree->functionItem(f);
        Q_ASSERT(item != NULL);
        f->setName(QString("%1 %2").arg(tr("New Script")).arg(f->id()));
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddAudio()
{
    QString fn;

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Audio File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    /* Append file filters to the dialog */
    QStringList extList = Audio::getCapabilities();

    QStringList filters;
    qDebug() << Q_FUNC_INFO << "Extensions: " << extList.join(" ");
    filters << tr("Audio Files (%1)").arg(extList.join(" "));
#if defined(WIN32) || defined(Q_OS_WIN)
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return;

    fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return;

    Function* f = new Audio(m_doc);
    Audio *audio = qobject_cast<Audio*> (f);
    if (audio->setSourceFileName(fn) == false)
    {
        QMessageBox::warning(this, tr("Unsupported audio file"), tr("This audio file cannot be played with QLC+. Sorry."));
        return;
    }
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = m_tree->functionItem(f);
        Q_ASSERT(item != NULL);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
}

void FunctionManager::slotAddVideo()
{
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    QString fn;

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Video File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    /* Append file filters to the dialog */
    QStringList extList = Video::getCapabilities();

    QStringList filters;
    qDebug() << Q_FUNC_INFO << "Extensions: " << extList.join(" ");
    filters << tr("Video Files (%1)").arg(extList.join(" "));
#if defined(WIN32) || defined(Q_OS_WIN)
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return;

    fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return;

    Function* f = new Video(m_doc);
    Video *video = qobject_cast<Video*> (f);
    if (video->setSourceUrl(fn) == false)
    {
        QMessageBox::warning(this, tr("Unsupported video file"), tr("This video file cannot be played with QLC+. Sorry."));
        return;
    }
    if (m_doc->addFunction(f) == true)
    {
        QTreeWidgetItem* item = m_tree->functionItem(f);
        Q_ASSERT(item != NULL);
        m_tree->scrollToItem(item);
        m_tree->setCurrentItem(item);
    }
#endif
}

void FunctionManager::slotAddFolder()
{
    m_tree->addFolder();
    m_doc->setModified();
}

void FunctionManager::slotSelectAutostartFunction()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.showNone(true);

    if (fs.exec() == QDialog::Accepted && fs.selection().size() > 0)
    {
        quint32 startID = fs.selection().first();
        m_doc->setStartupFunction(startID);
        m_doc->setModified();
    }
}

void FunctionManager::slotWizard()
{
    FunctionWizard fw(this, m_doc);
    if (fw.exec() == QDialog::Accepted)
        m_tree->updateTree();
}

void FunctionManager::slotClone()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        copyFunction(m_tree->itemFunctionId(it.next()));
}

void FunctionManager::slotDelete()
{
    bool isFolder = false;
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    if (it.hasNext() == false)
        return;

    QString msg;
    QTreeWidgetItem *firstItem = m_tree->selectedItems().first();

    if (firstItem->childCount() > 0 || firstItem->text(COL_PATH).isEmpty() == false)
        isFolder = true;

    if (isFolder == true)
        msg = tr("Do you want to DELETE folder:") + QString("\n");
    else
        msg = tr("Do you want to DELETE functions:") + QString("\n");

    // Append functions' names to the message
    while (it.hasNext() == true)
    {
        QTreeWidgetItem *item = it.next();
        msg.append(item->text(COL_NAME));
        if (it.hasNext())
            msg.append(", ");

        if (item->childCount() > 0)
        {
            msg.append("\n" + tr("(This will also DELETE: "));
            for(int i = 0; i < item->childCount(); i++)
            {
                QTreeWidgetItem *child = item->child(i);
                if (i > 0) msg.append(", ");
                msg.append(child->text(COL_NAME));
            }
            msg.append(")");
        }
    }

    // Ask for user's confirmation
    if (QMessageBox::question(this, tr("Delete Functions"), msg,
                              QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::Yes)
    {
        if (isFolder)
        {
            m_tree->deleteFolder(m_tree->selectedItems().first());
            m_doc->setModified();
        }
        else
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
    bool validSelection = false;
    m_cloneAction->setEnabled(false);

    if (m_tree->selectedItems().isEmpty() == false)
    {
        QTreeWidgetItem *firstItem = m_tree->selectedItems().first();
        quint32 fid = m_tree->itemFunctionId(firstItem);
        if (fid != Function::invalidId())
        {
            m_cloneAction->setEnabled(true);
            validSelection = true;
        }

        // check if this is a folder
        if (m_tree->selectedItems().count() == 1 && m_tree->indexOfTopLevelItem(firstItem) < 0)
            validSelection = true;

        m_addFolderAction->setEnabled(true);
    }
    else
        m_addFolderAction->setEnabled(false);

    if (validSelection == true)
    {
        /* At least one function has been selected, so
           editing is possible. */
        m_selectAllAction->setEnabled(true);
        if (m_doc->mode() == Doc::Operate)
            m_deleteAction->setEnabled(false);
        else
            m_deleteAction->setEnabled(true);
    }
    else
    {
        /* No functions selected */
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
    m_tree = new FunctionsTreeWidget(m_doc, this);
    Q_ASSERT(m_hsplitter != NULL);
    m_hsplitter->addWidget(m_tree);

    QStringList labels;
    labels << tr("Function"); // << "Path";
    m_tree->setHeaderLabels(labels);
    m_tree->setRootIsDecorated(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setSortingEnabled(true);
    m_tree->sortByColumn(COL_NAME, Qt::AscendingOrder);
    m_tree->setDragEnabled(true);
    m_tree->setAcceptDrops(true);
    m_tree->setDragDropMode(QAbstractItemView::InternalMove);

    // Catch selection changes
    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotTreeSelectionChanged()));

    // Catch right-mouse clicks
    connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotTreeContextMenuRequested()));
}

void FunctionManager::selectFunction(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function == NULL)
        return;
    QTreeWidgetItem* item = m_tree->functionItem(function);
    if (item != NULL)
        m_tree->setCurrentItem(item);
}

void FunctionManager::deleteSelectedFunctions()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        bool isSequence = false;
        QTreeWidgetItem* item(it.next());
        quint32 fid = m_tree->itemFunctionId(item);
        Function *func = m_doc->function(fid);
        if (func == NULL)
            continue;

        if (func->type() == Function::Chaser && qobject_cast<const Chaser*>(func)->isSequence() == true)
            isSequence = true;

        // Stop running tests before deleting function
        if (m_editor != NULL)
        {
            if (func->type() == Function::RGBMatrix)
                static_cast<RGBMatrixEditor*>(m_editor)->stopTest();
            else if (func->type() == Function::EFX)
                static_cast<EFXEditor*>(m_editor)->stopTest();
            else if (func->type() == Function::Chaser)
                static_cast<ChaserEditor*>(m_editor)->stopTest();
        }

        m_doc->deleteFunction(fid);

        QTreeWidgetItem* parent = item->parent();
        delete item;
        if (parent != NULL && parent->childCount() == 0 && isSequence == false)
        {
            if (m_tree->indexOfTopLevelItem(parent) >= 0)
                m_tree->deleteFolder(parent);
        }
    }
}

void FunctionManager::slotTreeSelectionChanged()
{
    updateActionStatus();

    QList <QTreeWidgetItem*> selection(m_tree->selectedItems());
    if (selection.size() == 1)
    {
        Function* function = m_doc->function(m_tree->itemFunctionId(selection.first()));
        if (function != NULL)
            editFunction(function);
        else
            deleteCurrentEditor();
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
    menu.addAction(m_addAudioAction);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    menu.addAction(m_addVideoAction);
#endif
    menu.addSeparator();
    menu.addAction(m_addFolderAction);
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
        copy->setName(copy->name() + tr(" (Copy)"));
        QTreeWidgetItem* item = m_tree->functionItem(copy);
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
    else if (function->type() == Function::Audio)
    {
        m_editor = new AudioEditor(m_hsplitter->widget(1), qobject_cast<Audio*> (function), m_doc);
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    else if (function->type() == Function::Video)
    {
        m_editor = new VideoEditor(m_hsplitter->widget(1), qobject_cast<Video*> (function), m_doc);
    }
#endif
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

void FunctionManager::deleteCurrentEditor(bool async)
{
    if (async)
    {
        if (m_editor) m_editor->deleteLater();
        if (m_scene_editor) m_scene_editor->deleteLater();
    }
    else
    {
        delete m_editor;
        delete m_scene_editor;
    }

    m_editor = NULL;
    m_scene_editor = NULL;

    m_hsplitter->widget(1)->hide();
    m_vsplitter->widget(1)->hide();
}
