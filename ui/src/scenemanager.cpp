/*
  Q Light Controller
  scenemanager.cpp

  Copyright (C) Massimo Callegari

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

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <QComboBox>
#include <QSplitter>
#include <QToolBar>
#include <QLabel>
#include <QDebug>

#include "multitrackview.h"
#include "scenemanager.h"
#include "chasereditor.h"
#include "scenerunner.h"
#include "sceneeditor.h"
#include "sceneitems.h"
#include "chaser.h"

SceneManager* SceneManager::s_instance = NULL;

SceneManager::SceneManager(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_scene(NULL)
    , m_scene_editor(NULL)
    , m_sequence_editor(NULL)
    , m_splitter(NULL)
    , m_vsplitter(NULL)
    , m_showview(NULL)
    , m_runner(NULL)
    , is_playing(false)
    , m_toolbar(NULL)
    , m_scenesCombo(NULL)
    , m_addSceneAction(NULL)
    , m_addSequenceAction(NULL)
    , m_cloneAction(NULL)
    , m_deleteAction(NULL)
    , m_stopAction(NULL)
    , m_playAction(NULL)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(doc != NULL);

    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    initActions();
    initToolbar();

    m_splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(m_splitter);
    //initMultiTrackView();
    m_showview = new MultiTrackView();
    // add container for multitrack+chaser view
    QWidget* gcontainer = new QWidget(this);
    m_splitter->addWidget(gcontainer);
    gcontainer->setLayout(new QVBoxLayout);
    gcontainer->layout()->setContentsMargins(0, 0, 0, 0);
    //m_showview = new QGraphicsView(m_showscene);
    m_showview->setRenderHint(QPainter::Antialiasing);
    m_showview->setAcceptDrops(true);
    m_showview->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_showview->setBackgroundBrush(QBrush(QColor(88, 88, 88, 255), Qt::SolidPattern));
    connect(m_showview, SIGNAL(viewClicked ( QMouseEvent * )),
            this, SLOT(slotViewClicked( QMouseEvent * )));
    connect(m_showview, SIGNAL(sequenceMoved(SequenceItem *)),
            this, SLOT(slotSequenceMoved(SequenceItem*)));
    connect(m_showview, SIGNAL(timeChanged(quint32)),
            this, SLOT(slotUpdateTime(quint32)));

    // split the multitrack view into two (left: tracks, right: chaser editor)
    m_vsplitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->widget(0)->layout()->addWidget(m_vsplitter);
    QWidget* mcontainer = new QWidget(this);
    mcontainer->setLayout(new QHBoxLayout);
    mcontainer->layout()->setContentsMargins(0, 0, 0, 0);
    m_vsplitter->addWidget(mcontainer);
    m_vsplitter->widget(0)->layout()->addWidget(m_showview);

    // add container for chaser editor
    QWidget* ccontainer = new QWidget(this);
    m_vsplitter->addWidget(ccontainer);
    ccontainer->setLayout(new QVBoxLayout);
    ccontainer->layout()->setContentsMargins(0, 0, 0, 0);

    // add container for scene editor
    QWidget* container = new QWidget(this);
    m_splitter->addWidget(container);
    container->setLayout(new QVBoxLayout);
    container->layout()->setContentsMargins(0, 0, 0, 0);
    
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged()));
    connect(m_doc, SIGNAL(clearing()), this, SLOT(slotDocClearing()));
}

SceneManager::~SceneManager()
{
    SceneManager::s_instance = NULL;
}

SceneManager* SceneManager::instance()
{
    return s_instance;
}

void SceneManager::initActions()
{
    /* Manage actions */
    m_addSceneAction = new QAction(QIcon(":/scene.png"),
                                   tr("New &scene"), this);
    m_addSceneAction->setShortcut(QKeySequence("CTRL+S"));
    connect(m_addSceneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddScene()));

    m_addSequenceAction = new QAction(QIcon(":/sequence.png"),
                                    tr("New s&equence"), this);
    m_addSequenceAction->setShortcut(QKeySequence("CTRL+E"));
    connect(m_addSequenceAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSequence()));

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

    m_stopAction = new QAction(QIcon(":/design.png"),  /* @todo re-used icon...to be changed */
                                 tr("S&top"), this);
    m_stopAction->setShortcut(QKeySequence("CTRL+T"));
    connect(m_stopAction, SIGNAL(triggered(bool)),
            this, SLOT(slotStopPlayback()));

    m_playAction = new QAction(QIcon(":/operate.png"), /* @todo re-used icon...to be changed */
                                 tr("&Play"), this);
    m_playAction->setShortcut(QKeySequence("SPACE"));
    connect(m_playAction, SIGNAL(triggered(bool)),
            this, SLOT(slotStartPlayback()));
}

void SceneManager::initToolbar()
{
    // Add a toolbar to the dock area
    m_toolbar = new QToolBar("Show Manager", this);
    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    layout()->addWidget(m_toolbar);
    m_toolbar->addAction(m_addSceneAction);
    m_scenesCombo = new QComboBox();
    m_scenesCombo->setFixedWidth(150);
    connect(m_scenesCombo, SIGNAL(currentIndexChanged(int)),this, SLOT(slotSceneComboChanged(int)));
    m_toolbar->addWidget(m_scenesCombo);
    m_toolbar->addSeparator();

    m_toolbar->addAction(m_addSequenceAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_cloneAction);
    m_toolbar->addAction(m_deleteAction);
    m_toolbar->addSeparator();

    // Time label and playback buttons
    m_timeLabel = new QLabel("00:00:00:000");
    m_timeLabel->setFixedWidth(150);
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    QFont timeFont = QApplication::font();
    timeFont.setBold(true);
    timeFont.setPixelSize(20);
    m_timeLabel->setFont(timeFont);
    m_toolbar->addWidget(m_timeLabel);
    m_toolbar->addSeparator();

    m_toolbar->addAction(m_stopAction);
    m_toolbar->addAction(m_playAction);
}

void SceneManager::updateScenesCombo()
{
    int newIndex = 0;
    m_scenesCombo->clear();
    foreach (Function* function, m_doc->functions())
    {
        if (function->type() == Function::Scene)
        {
            m_scenesCombo->addItem(function->name(), QVariant(function->id()));
            if (m_scene != NULL && m_scene->id() != function->id())
                newIndex++;
        }
    }
    if (m_scenesCombo->count() > 0)
    {
        m_scenesCombo->setCurrentIndex(newIndex);
        m_addSequenceAction->setEnabled(true);
    }
    else
        m_addSequenceAction->setEnabled(false);
}

void SceneManager::slotSceneComboChanged(int idx)
{
    qDebug() << Q_FUNC_INFO << "Idx: " << idx;
    updateMultiTrackView();
}

void SceneManager::slotAddScene()
{
    m_scene = new Scene(m_doc);
    Function *f = qobject_cast<Function*>(m_scene);
    if (m_doc->addFunction(f) == true)
    {
        f->setName(QString("%1 %2").arg(tr("New Scene")).arg(f->id()));
        if (m_scene_editor != NULL)
            m_scene_editor->deleteLater();
        m_scene_editor = NULL;
        m_scene_editor = new SceneEditor(m_splitter->widget(1), m_scene, m_doc, false);
        connect(this, SIGNAL(functionManagerActive(bool)),
                    m_scene_editor, SLOT(slotFunctionManagerActive(bool)));

        if (m_scene_editor != NULL)
            m_splitter->widget(1)->layout()->addWidget(m_scene_editor);

        updateScenesCombo();
    }
}

void SceneManager::slotAddSequence()
{
    Function* f = new Chaser(m_doc);
    Chaser *chaser = qobject_cast<Chaser*> (f);
    chaser->enableSequenceMode(m_scenesCombo->itemData(m_scenesCombo->currentIndex()).toUInt());
    chaser->setRunOrder(Function::SingleShot);
    if (m_doc->addFunction(f) == true)
    {
        f->setName(QString("%1 %2").arg(tr("New Sequence")).arg(f->id()));
        if (m_sequence_editor != NULL)
            m_sequence_editor->deleteLater();
        m_sequence_editor = NULL;
        m_sequence_editor = new ChaserEditor(m_vsplitter->widget(1), chaser, m_doc);
        m_vsplitter->widget(1)->layout()->addWidget(m_sequence_editor);

        m_showview->addSequence(chaser);
    }
}

void SceneManager::slotClone()
{
}

void SceneManager::slotDelete()
{
    m_showview->deleteSelectedSequence();
}

void SceneManager::slotStopPlayback()
{
    if (m_runner != NULL)
    {
        m_runner->stop();
        delete m_runner;
        m_runner = NULL;
    }
    m_showview->rewindCursor();
    m_timeLabel->setText("00:00:00.000");
}

void SceneManager::slotStartPlayback()
{
    if (m_scenesCombo->count() == 0)
        return;
    if (m_runner != NULL)
    {
        m_runner->stop();
        delete m_runner;
    }

    m_runner = new SceneRunner(m_doc, m_scenesCombo->itemData(m_scenesCombo->currentIndex()).toUInt());
    connect(m_runner, SIGNAL(timeChanged(quint32)), this, SLOT(slotupdateTimeAndCursor(quint32)));
    m_runner->start();
}

void SceneManager::slotViewClicked(QMouseEvent *event)
{
    qDebug() << Q_FUNC_INFO << "View clicked at pos: " << event->pos().x() << event->pos().y();
    if (m_sequence_editor != NULL)
        m_sequence_editor->deleteLater();
    m_sequence_editor = NULL;
    m_vsplitter->widget(1)->hide();
}

void SceneManager::slotSequenceMoved(SequenceItem *item)
{
    qDebug() << Q_FUNC_INFO << "Sequence moved.........";
    if (m_sequence_editor != NULL)
        m_sequence_editor->deleteLater();
    m_sequence_editor = NULL;
    m_vsplitter->widget(1)->hide();

    m_sequence_editor = new ChaserEditor(m_vsplitter->widget(1), qobject_cast<Chaser*> (item->getChaser()), m_doc);
    m_vsplitter->widget(1)->layout()->addWidget(m_sequence_editor);
    m_vsplitter->widget(1)->show();
    m_sequence_editor->show();
}

void SceneManager::slotupdateTimeAndCursor(quint32 msec_time)
{
    qDebug() << Q_FUNC_INFO << "time: " << msec_time;
    slotUpdateTime(msec_time);
    m_showview->moveCursor(msec_time);
}

void SceneManager::slotUpdateTime(quint32 msec_time)
{
    QTime tmpTime = QTime(0, 0, 0, 0).addMSecs(msec_time);

    m_timeLabel->setText(tmpTime.toString("hh:mm:ss.zzz"));
}

void SceneManager::updateMultiTrackView()
{
    m_showview->resetView();
    /* first of all get the ID of the selected scene */
    int idx = m_scenesCombo->currentIndex();
    if (idx == -1)
        return;
    quint32 sceneID = m_scenesCombo->itemData(idx).toUInt();

    /* create a SequenceItem for each Chaser that is a sequence and is bound to the scene ID */
    QListIterator <Function*> it(m_doc->functions());
    while (it.hasNext() == true)
    {
        Function *f = it.next();
        if (f->type() == Function::Chaser)
        {
            Chaser *chaser = qobject_cast<Chaser*> (f);
            if (chaser->isSequence() && chaser->getBoundedSceneID() == sceneID)
            {
                m_showview->addSequence(chaser);
            }
        }
    }
    if (m_scene_editor != NULL)
        m_scene_editor->deleteLater();
    m_scene_editor = NULL;
    m_scene = qobject_cast<Scene*>(m_doc->function(sceneID));
    m_scene_editor = new SceneEditor(m_splitter->widget(1), m_scene, m_doc, false);
    connect(this, SIGNAL(functionManagerActive(bool)),
                m_scene_editor, SLOT(slotFunctionManagerActive(bool)));

    if (m_scene_editor != NULL)
        m_splitter->widget(1)->layout()->addWidget(m_scene_editor);
}

void SceneManager::showEvent(QShowEvent* ev)
{
    qDebug() << Q_FUNC_INFO;
    emit functionManagerActive(true);
    QWidget::showEvent(ev);
    m_showview->show();
    m_showview->horizontalScrollBar()->setSliderPosition(0);
    updateScenesCombo();
}

void SceneManager::hideEvent(QHideEvent* ev)
{
    qDebug() << Q_FUNC_INFO;
    emit functionManagerActive(false);
    QWidget::hideEvent(ev);
    
    if (m_scene_editor != NULL)
        m_scene_editor->deleteLater();
    m_scene_editor = NULL;
}
