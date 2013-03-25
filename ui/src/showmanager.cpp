/*
  Q Light Controller
  showmanager.cpp

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

#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <QComboBox>
#include <QSplitter>
#include <QToolBar>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>
#include <QUrl>

#include "multitrackview.h"
#include "sceneselection.h"
#include "chasereditor.h"
#include "showmanager.h"
#include "sceneeditor.h"
#include "sceneitems.h"
#include "chaser.h"

ShowManager* ShowManager::s_instance = NULL;

ShowManager::ShowManager(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_show(NULL)
    , m_scene(NULL)
    , m_scene_editor(NULL)
    , m_sequence_editor(NULL)
    , m_splitter(NULL)
    , m_vsplitter(NULL)
    , m_showview(NULL)
    , m_toolbar(NULL)
    , m_showsCombo(NULL)
    , m_addShowAction(NULL)
    , m_addTrackAction(NULL)
    , m_addSequenceAction(NULL)
    , m_addAudioAction(NULL)
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
    connect(m_showview, SIGNAL(audioMoved(AudioItem *)),
            this, SLOT(slotAudioMoved(AudioItem*)));
    //connect(m_showview, SIGNAL(timeChanged(quint32)),
    //        this, SLOT(slotUpdateTime(quint32)));
    connect(m_showview, SIGNAL(trackClicked(Track*)),
            this, SLOT(slotTrackClicked(Track*)));

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
    m_vsplitter->widget(1)->hide();

    // add container for scene editor
    QWidget* container = new QWidget(this);
    m_splitter->addWidget(container);
    container->setLayout(new QVBoxLayout);
    container->layout()->setContentsMargins(0, 0, 0, 0);
    m_splitter->widget(1)->hide();
    
    //connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged()));
    connect(m_doc, SIGNAL(clearing()), this, SLOT(slotDocClearing()));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
}

ShowManager::~ShowManager()
{
    ShowManager::s_instance = NULL;
}

ShowManager* ShowManager::instance()
{
    return s_instance;
}

void ShowManager::initActions()
{
    /* Manage actions */
    m_addShowAction = new QAction(QIcon(":/show.png"),
                                   tr("New s&how"), this);
    m_addShowAction->setShortcut(QKeySequence("CTRL+H"));
    connect(m_addShowAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddShow()));

    m_addTrackAction = new QAction(QIcon(":/scene.png"),
                                   tr("New &track"), this);
    m_addTrackAction->setShortcut(QKeySequence("CTRL+T"));
    connect(m_addTrackAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddTrack()));

    m_addSequenceAction = new QAction(QIcon(":/sequence.png"),
                                    tr("New s&equence"), this);
    m_addSequenceAction->setShortcut(QKeySequence("CTRL+E"));
    connect(m_addSequenceAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddSequence()));

    m_addAudioAction = new QAction(QIcon(":/audio.png"),
                                    tr("New &audio"), this);
    m_addAudioAction->setShortcut(QKeySequence("CTRL+A"));
    connect(m_addAudioAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddAudio()));
    /* Edit actions */
    m_cloneAction = new QAction(QIcon(":/editcopy.png"),
                                tr("&Clone"), this);
    m_cloneAction->setShortcut(QKeySequence("CTRL+C"));
    connect(m_cloneAction, SIGNAL(triggered(bool)),
            this, SLOT(slotClone()));
    m_cloneAction->setEnabled(false);

    m_deleteAction = new QAction(QIcon(":/editdelete.png"),
                                 tr("&Delete"), this);
    m_deleteAction->setShortcut(QKeySequence("Delete"));
    connect(m_deleteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDelete()));
    m_deleteAction->setEnabled(false);

    m_colorAction = new QAction(QIcon(":/color.png"),
                                tr("Change Co&lor"), this);
    m_colorAction->setShortcut(QKeySequence("CTRL+L"));
    connect(m_colorAction, SIGNAL(triggered(bool)),
           this, SLOT(slotChangeColor()));
    m_colorAction->setEnabled(false);

    m_stopAction = new QAction(QIcon(":/player_stop.png"),
                                 tr("St&op"), this);
    m_stopAction->setShortcut(QKeySequence("CTRL+O"));
    connect(m_stopAction, SIGNAL(triggered(bool)),
            this, SLOT(slotStopPlayback()));

    m_playAction = new QAction(QIcon(":/player_play.png"),
                                 tr("&Play"), this);
    m_playAction->setShortcut(QKeySequence("SPACE"));
    connect(m_playAction, SIGNAL(triggered(bool)),
            this, SLOT(slotStartPlayback()));
}

void ShowManager::initToolbar()
{
    // Add a toolbar to the dock area
    m_toolbar = new QToolBar("Show Manager", this);
    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    layout()->addWidget(m_toolbar);
    m_toolbar->addAction(m_addShowAction);
    m_showsCombo = new QComboBox();
    m_showsCombo->setFixedWidth(200);
    connect(m_showsCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotShowsComboChanged(int)));
    m_toolbar->addWidget(m_showsCombo);
    m_toolbar->addSeparator();

    m_toolbar->addAction(m_addTrackAction);
    m_toolbar->addAction(m_addSequenceAction);
    m_toolbar->addAction(m_addAudioAction);

    m_toolbar->addSeparator();
    m_toolbar->addAction(m_cloneAction);
    m_toolbar->addAction(m_deleteAction);
    m_toolbar->addSeparator();

    m_toolbar->addAction(m_colorAction);
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

    /* Create an empty widget between help items to flush them to the right */
    QWidget* widget = new QWidget(this);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolbar->addWidget(widget);

    /* Add time division elements */
    QLabel* timeLabel = new QLabel(tr("Time division:"));
    m_toolbar->addWidget(timeLabel);

    m_timeDivisionCombo = new QComboBox();
    m_timeDivisionCombo->setFixedWidth(100);
    m_timeDivisionCombo->addItem(tr("Time"));
    m_timeDivisionCombo->addItem("BPM 4/4");
    m_timeDivisionCombo->addItem("BPM 3/4");
    m_timeDivisionCombo->addItem("BPM 2/4");
    m_toolbar->addWidget(m_timeDivisionCombo);
    connect(m_timeDivisionCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTimeDivisionTypeChanged(int)));

    m_bpmField = new QSpinBox();
    m_bpmField->setFixedWidth(70);
    m_bpmField->setMinimum(10);
    m_bpmField->setMaximum(240);
    m_bpmField->setValue(120);
    m_bpmField->setEnabled(false);
    m_toolbar->addWidget(m_bpmField);
    connect(m_bpmField, SIGNAL(valueChanged(int)),
            this, SLOT(slotBPMValueChanged(int)));
}

/*********************************************************************
 * Shows combo
 *********************************************************************/
void ShowManager::updateShowsCombo()
{
    int newIndex = 0;
    m_showsCombo->clear();
    foreach (Function* function, m_doc->functions())
    {
        if (function->type() == Function::Show)
        {
            m_showsCombo->addItem(function->name(), QVariant(function->id()));
            if (m_show != NULL && m_show->id() != function->id())
                newIndex++;
        }
    }
    if (m_showsCombo->count() > 0)
    {
        m_showsCombo->setCurrentIndex(newIndex);
        m_addTrackAction->setEnabled(true);
    }
    else
    {
        m_addTrackAction->setEnabled(false);
        m_addSequenceAction->setEnabled(false);
        m_addAudioAction->setEnabled(false);
    }
    if (m_show == NULL || m_show->getTracksCount() == 0)
        m_deleteAction->setEnabled(false);
    else
        m_deleteAction->setEnabled(true);
}

void ShowManager::slotShowsComboChanged(int idx)
{
    qDebug() << Q_FUNC_INFO << "Idx: " << idx;
    updateMultiTrackView();
}

void ShowManager::showSceneEditor(Scene *scene)
{
    if (m_scene_editor != NULL)
    {
        emit functionManagerActive(false);
        m_splitter->widget(1)->layout()->removeWidget(m_scene_editor);
        m_scene_editor->deleteLater();
        m_scene_editor = NULL;
    }

    if (this->isVisible())
    {
        m_scene_editor = new SceneEditor(m_splitter->widget(1), scene, m_doc, false);
        if (m_scene_editor != NULL)
        {
            m_splitter->widget(1)->layout()->addWidget(m_scene_editor);
            m_splitter->widget(1)->show();
            //m_scene_editor->show();
            connect(this, SIGNAL(functionManagerActive(bool)),
                    m_scene_editor, SLOT(slotFunctionManagerActive(bool)));
        }
    }
}

void ShowManager::showSequenceEditor(Chaser *chaser)
{
    if (m_sequence_editor != NULL)
    {
        m_vsplitter->widget(1)->layout()->removeWidget(m_sequence_editor);
        m_sequence_editor->deleteLater();
        m_sequence_editor = NULL;
    }

    if (chaser == NULL)
        return;

    if (this->isVisible())
    {
        m_sequence_editor = new ChaserEditor(m_vsplitter->widget(1), chaser, m_doc);
        if (m_sequence_editor != NULL)
        {
            m_vsplitter->widget(1)->layout()->addWidget(m_sequence_editor);
            /** Signal from chaser editor to scene editor. When a step is clicked apply values immediately */
            connect(m_sequence_editor, SIGNAL(applyValues(QList<SceneValue>&)),
                    m_scene_editor, SLOT(slotSetSceneValues(QList <SceneValue>&)));
            /** Signal from scene editor to chaser editor. When a fixture value is changed, update the selected chaser step */
            connect(m_scene_editor, SIGNAL(fixtureValueChanged(SceneValue)),
                    m_sequence_editor, SLOT(slotUpdateCurrentStep(SceneValue)));
            m_vsplitter->widget(1)->show();
            m_sequence_editor->show();
        }
    }
}

void ShowManager::slotAddShow()
{
    m_show = new Show(m_doc);
    Function *f = qobject_cast<Function*>(m_show);
    if (m_doc->addFunction(f) == true)
    {
        f->setName(QString("%1 %2").arg(tr("New Show")).arg(f->id()));
        bool ok;
        QString text = QInputDialog::getText(this, tr("Show name setup"),
                                             tr("Show name:"), QLineEdit::Normal,
                                             f->name(), &ok);
        if (ok && !text.isEmpty())
            m_show->setName(text);
        updateShowsCombo();
    }
}

void ShowManager::slotAddTrack()
{
    if (m_show == NULL)
        return;

    QList <quint32> disabledIDs;
    if (m_show->tracks().count() > 0)
    {
        /** Add Scene IDs already assigned in this Show */
        foreach (Track *track, m_show->tracks())
            disabledIDs.append(track->getSceneID());
    }

    SceneSelection ss(this, m_doc);
    ss.setDisabledScenes(disabledIDs);
    if (ss.exec() == QDialog::Accepted)
    {
        /* Do we have to create a new Scene ? */
        if (ss.getSelectedID() == Scene::invalidId())
        {
            m_scene = new Scene(m_doc);
            Function *f = qobject_cast<Function*>(m_scene);
            if (m_doc->addFunction(f) == true)
                f->setName(QString("%1 %2").arg(tr("New Scene")).arg(f->id()));
        }
        else
        {
            m_scene = qobject_cast<Scene*>(m_doc->function(ss.getSelectedID()));
        }
        Track* newTrack = new Track(m_scene->id());
        newTrack->setName(m_scene->name());
        m_show->addTrack(newTrack);
        showSceneEditor(m_scene);
        m_showview->addTrack(newTrack);
        m_addSequenceAction->setEnabled(true);
        m_addAudioAction->setEnabled(true);
        m_showview->activateTrack(newTrack);
        m_deleteAction->setEnabled(true);
    }
}

void ShowManager::slotAddSequence()
{
    Function* f = new Chaser(m_doc);
    Chaser *chaser = qobject_cast<Chaser*> (f);
    quint32 sceneID = m_scene->id();
    chaser->enableSequenceMode(sceneID);
    chaser->setRunOrder(Function::SingleShot);
    Scene *boundScene = qobject_cast<Scene*>(m_doc->function(sceneID));
    boundScene->setChildrenFlag(true);
    if (m_doc->addFunction(f) == true)
    {
        f->setName(QString("%1 %2").arg(tr("New Sequence")).arg(f->id()));
        showSequenceEditor(chaser);
        Track *track = m_show->getTrackFromSceneID(m_scene->id());
        track->addFunctionID(chaser->id());
        m_showview->addSequence(chaser);
    }
}

void ShowManager::slotAddAudio()
{
    QString fn;

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Audio File"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    //dialog.selectFile(fileName());

    /* Append file filters to the dialog */
    QStringList extList;
#ifdef QT_PHONON_LIB
    QStringList systemCaps = Audio::getCapabilities();
    if (systemCaps.contains("audio/ogg") || systemCaps.contains("audio/x-ogg"))
        extList << "*.ogg";
    if (systemCaps.contains("audio/x-m4a"))
        extList << "*.m4a";
    if (systemCaps.contains("audio/flac") || systemCaps.contains("audio/x-flac"))
        extList << "*.flac";
    if (systemCaps.contains("audio/x-ms-wma"))
        extList << "*.wma";
    if (systemCaps.contains("audio/wav") || systemCaps.contains("audio/x-wav"))
        extList << "*.wav";
    if (systemCaps.contains("audio/mp3") || systemCaps.contains("audio/x-mp3") ||
        systemCaps.contains("audio/mpeg3") || systemCaps.contains("audio/x-mpeg3"))
        extList << "*.mp3";
#else
    extList = Audio::getCapabilities();
#endif
    QStringList filters;
    qDebug() << Q_FUNC_INFO << "Extensions: " << extList.join(" ");
    filters << tr("Audio Files (%1)").arg(extList.join(" "));
#ifdef WIN32
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
        Track *track = m_show->getTrackFromSceneID(m_scene->id());
        track->addFunctionID(audio->id());
        m_showview->addAudio(audio);
    }

}

void ShowManager::slotClone()
{
    quint32 fid = Function::invalidId();
    AudioItem *audItem = NULL;

    SequenceItem *seqItem = m_showview->getSelectedSequence();
    if (seqItem != NULL)
        fid = seqItem->getChaser()->id();
    else
    {
        audItem = m_showview->getSelectedAudio();
        if (audItem != NULL)
            fid = audItem->getAudio()->id();
    }

    if (fid != Function::invalidId())
    {
        Function* function = m_doc->function(fid);
        Q_ASSERT(function != NULL);

        /* Attempt to create a copy of the function to Doc */
        Function* copy = function->createCopy(m_doc);
        if (copy != NULL)
        {
            copy->setName(tr("Copy of %1").arg(function->name()));
            Track *track = m_show->getTrackFromSceneID(m_scene->id());
            if (seqItem != NULL)
            {
                Chaser *chaser = qobject_cast<Chaser*>(copy);
                // Invalidate start time so the sequence will be cloned at the cursor position
                chaser->setStartTime(UINT_MAX);
                track->addFunctionID(chaser->id());
                m_showview->addSequence(chaser);
            }

            if (audItem != NULL)
            {
                Audio *audio = qobject_cast<Audio*>(copy);
                // Invalidate start time so the sequence will be cloned at the cursor position
                audio->setStartTime(UINT_MAX);
                track->addFunctionID(audio->id());
                m_showview->addAudio(audio);
            }
        }
    }
}

void ShowManager::slotDelete()
{
    // find out if we're deleting a sequence/audio or a track
    bool isTrack = true;
    if (m_showview->getSelectedSequence() != NULL ||
        m_showview->getSelectedAudio() != NULL)
            isTrack = false;
    // get the ID of the function to delete (invalidId if nothing was selected)
    quint32 deleteID = m_showview->deleteSelectedFunction();
    if (deleteID != Function::invalidId())
    {
        if (isTrack == false)
        {
            Track *currTrack = m_show->getTrackFromSceneID(m_scene->id());
            if (currTrack != NULL)
                currTrack->removeFunctionID(deleteID);
        }

        m_doc->deleteFunction(deleteID);
/*
        if (m_sequence_editor != NULL)
        {
            m_vsplitter->widget(1)->layout()->removeWidget(m_sequence_editor);
            m_sequence_editor->deleteLater();
            m_sequence_editor = NULL;
        }

        else
        {
            foreach(Track *track, m_show->tracks())
            {
                m_scene = qobject_cast<Scene*>(m_doc->function(track->getSceneID()));
                if (m_scene == NULL)
                {
                    qDebug() << Q_FUNC_INFO << "Invalid scene !";
                    continue;
                }
                Track *firstTrack = m_show->getTrackFromSceneID(m_scene->id());
                m_showview->activateTrack(firstTrack);
                showSceneEditor(m_scene);
                m_deleteAction->setEnabled(true);
                break;
            }
        }
*/
    }
}

void ShowManager::slotStopPlayback()
{
    if (m_show != NULL && m_show->isRunning())
        m_show->stop();
    m_showview->rewindCursor();
    m_timeLabel->setText("00:00:00.000");
}

void ShowManager::slotStartPlayback()
{
    if (m_showsCombo->count() == 0 || m_show == NULL)
        return;
    m_show->start(m_doc->masterTimer());
}

void ShowManager::slotTimeDivisionTypeChanged(int idx)
{
    m_showview->setHeaderType(idx);
    if (idx > 0)
        m_bpmField->setEnabled(true);
    else
        m_bpmField->setEnabled(false);
}

void ShowManager::slotBPMValueChanged(int value)
{
    m_showview->setBPMValue(value);
}

void ShowManager::slotViewClicked(QMouseEvent *event)
{
    qDebug() << Q_FUNC_INFO << "View clicked at pos: " << event->pos().x() << event->pos().y();
    if (m_sequence_editor != NULL)
    {
        m_vsplitter->widget(1)->layout()->removeWidget(m_sequence_editor);
        m_sequence_editor->deleteLater();
        m_sequence_editor = NULL;
    }
    m_vsplitter->widget(1)->hide();
    m_cloneAction->setEnabled(false);
    if (m_show != NULL && m_show->getTracksCount() == 0)
        m_deleteAction->setEnabled(false);
}

void ShowManager::slotSequenceMoved(SequenceItem *item)
{
    qDebug() << Q_FUNC_INFO << "Sequence moved.........";
    Chaser *chaser = item->getChaser();
    if (chaser == NULL)
        return;
    /* Check if scene has changed */
    quint32 newSceneID = chaser->getBoundedSceneID();
    if (newSceneID != m_scene->id())
    {
        Function *f = m_doc->function(newSceneID);
        if (f == NULL)
            return;
        m_scene = qobject_cast<Scene*>(f);
        showSceneEditor(m_scene);
        /* activate the new track */
        Track *track = m_show->getTrackFromSceneID(newSceneID);
        m_showview->activateTrack(track);
    }
    showSequenceEditor(chaser);
    m_cloneAction->setEnabled(true);
    m_deleteAction->setEnabled(true);
    m_colorAction->setEnabled(true);
}

void ShowManager::slotAudioMoved(AudioItem *item)
{
    qDebug() << Q_FUNC_INFO << "Audio moved.........";
    Audio *audio = item->getAudio();
    if (audio == NULL)
        return;
    showSequenceEditor(NULL);
    m_cloneAction->setEnabled(true);
    m_deleteAction->setEnabled(true);
    m_colorAction->setEnabled(true);
}

void ShowManager::slotupdateTimeAndCursor(quint32 msec_time)
{
    //qDebug() << Q_FUNC_INFO << "time: " << msec_time;
    slotUpdateTime(msec_time);
    m_showview->moveCursor(msec_time);
}

void ShowManager::slotUpdateTime(quint32 msec_time)
{
    QTime tmpTime = QTime(0, 0, 0, 0).addMSecs(msec_time);

    m_timeLabel->setText(tmpTime.toString("hh:mm:ss.zzz"));
}

void ShowManager::slotTrackClicked(Track *track)
{
    Function *f = m_doc->function(track->getSceneID());
    if (f == NULL)
        return;
    m_scene = qobject_cast<Scene*>(f);
    showSceneEditor(m_scene);
}

void ShowManager::slotChangeColor()
{
    SequenceItem *seqItem = m_showview->getSelectedSequence();
    if (seqItem != NULL)
    {
        QColor color = seqItem->getChaser()->getColor();

        color = QColorDialog::getColor(color);
        seqItem->getChaser()->setColor(color);
        seqItem->setColor(color);
        return;
    }
    AudioItem *audItem = m_showview->getSelectedAudio();
    if (audItem != NULL)
    {
        QColor color = audItem->getAudio()->getColor();

    color = QColorDialog::getColor(color);
        audItem->getAudio()->setColor(color);
        audItem->setColor(color);
        return;
    }
}

void ShowManager::slotChangeSize(int width, int height)
{
    if (m_showview != NULL)
        m_showview->setViewSize(width, height);
}

void ShowManager::slotDocClearing()
{
    m_showsCombo->clear();

    if (m_showview != NULL)
        m_showview->resetView();

    if (m_sequence_editor != NULL)
    {
        m_vsplitter->widget(1)->layout()->removeWidget(m_sequence_editor);
        m_sequence_editor->deleteLater();
        m_sequence_editor = NULL;
    }
    m_vsplitter->widget(1)->hide();

    if (m_scene_editor != NULL)
    {
        emit functionManagerActive(false);
        m_splitter->widget(1)->layout()->removeWidget(m_scene_editor);
        m_scene_editor->deleteLater();
        m_scene_editor = NULL;
    }
    m_splitter->widget(1)->hide();

    m_addTrackAction->setEnabled(false);
    m_addSequenceAction->setEnabled(false);
    m_addAudioAction->setEnabled(false);
    m_deleteAction->setEnabled(false);
}

void ShowManager::slotDocLoaded()
{
    updateShowsCombo();
}

void ShowManager::slotFunctionChanged(quint32 id)
{
    if (this->isVisible() == false)
        return;

    Function* function = m_doc->function(id);
    if (function == NULL)
        return;

    if (function->type() == Function::Scene)
    {
        if (m_show == NULL)
            return;
        Track *trk = m_show->getTrackFromSceneID(id);
        if (trk != NULL)
            trk->setName(function->name());
    }
}

void ShowManager::slotFunctionRemoved(quint32 id)
{
    /** If the deleted function was a Chaser, find and delete all the
     *  associated Sequences */
    foreach (Function* function, m_doc->functions())
    {
        if (function->type() == Function::Chaser)
        {
            Chaser *chaser = qobject_cast<Chaser*>(function);
            if (chaser->isSequence() && chaser->getBoundedSceneID() == id)
            {
                m_doc->deleteFunction(chaser->id());
            }
        }
    }

    if (m_show != NULL && m_show->id() == id)
        m_show = NULL;
    if (m_scene != NULL && m_scene->id() == id)
        m_scene = NULL;

    updateMultiTrackView();
}

void ShowManager::updateMultiTrackView()
{
    m_showview->resetView();
    /* first of all get the ID of the selected scene */
    int idx = m_showsCombo->currentIndex();
    if (idx == -1)
        return;
    quint32 showID = m_showsCombo->itemData(idx).toUInt();

    bool first = true;
    /* create a SequenceItem for each Chaser that is a sequence and is bound to the scene ID */
    m_show = qobject_cast<Show *>(m_doc->function(showID));
    if (m_show == NULL)
    {
        qDebug() << Q_FUNC_INFO << "Invalid show !";
        return;
    }

    connect(m_show, SIGNAL(timeChanged(quint32)), this, SLOT(slotupdateTimeAndCursor(quint32)));
    connect(m_show, SIGNAL(showFinished()), this, SLOT(slotStopPlayback()));

    Scene *firstScene = NULL;

    foreach(Track *track, m_show->tracks())
    {
        m_scene = qobject_cast<Scene*>(m_doc->function(track->getSceneID()));
        if (m_scene == NULL)
        {
            qDebug() << Q_FUNC_INFO << "Invalid scene !";
            continue;
        }
        if (first == true)
        {
            firstScene = m_scene;
            first = false;
        }
        m_showview->addTrack(track);
        m_addSequenceAction->setEnabled(true);
        m_addAudioAction->setEnabled(true);

        foreach(quint32 id, track->functionsID())
        {
            Function *fn = m_doc->function(id);
            if (fn != NULL)
            {
                if (fn->type() == Function::Chaser)
                {
                    Chaser *chaser = qobject_cast<Chaser*>(m_doc->function(id));
                    m_showview->addSequence(chaser);
                }
                else if (fn->type() == Function::Audio)
                {
                    Audio *audio = qobject_cast<Audio*>(m_doc->function(id));
                    //audio->setSourceFileName(audio->getSourceFileName()); // kind of a dirty hack
                    m_showview->addAudio(audio);
                }
            }
        }
    }
    /** Set first track active */
    if (firstScene != NULL)
    {
        Track *firstTrack = m_show->getTrackFromSceneID(firstScene->id());
        m_scene = firstScene;
        m_showview->activateTrack(firstTrack);
        showSceneEditor(m_scene);
    }
}

void ShowManager::showEvent(QShowEvent* ev)
{
    qDebug() << Q_FUNC_INFO;
    emit functionManagerActive(true);
    QWidget::showEvent(ev);
    m_showview->show();
    m_showview->horizontalScrollBar()->setSliderPosition(0);
    m_showview->verticalScrollBar()->setSliderPosition(0);
    updateShowsCombo();
}

void ShowManager::hideEvent(QHideEvent* ev)
{
    qDebug() << Q_FUNC_INFO;
    emit functionManagerActive(false);
    QWidget::hideEvent(ev);
    
    if (m_sequence_editor != NULL)
    {
        m_vsplitter->widget(1)->layout()->removeWidget(m_sequence_editor);
        m_vsplitter->widget(1)->hide();
        m_sequence_editor->deleteLater();
        m_sequence_editor = NULL;
    }

    if (m_scene_editor != NULL)
    {
        m_splitter->widget(1)->layout()->removeWidget(m_scene_editor);
        m_splitter->widget(1)->hide();
        m_scene_editor->deleteLater();
        m_scene_editor = NULL;
    }
}
