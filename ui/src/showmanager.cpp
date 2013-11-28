/*
  Q Light Controller
  showmanager.cpp

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
#include "audioeditor.h"
#include "showmanager.h"
#include "sceneeditor.h"
#include "sceneitems.h"
#include "qlcmacros.h"
#include "chaser.h"

#define SETTINGS_HSPLITTER "showmanager/hsplitter"
#define SETTINGS_VSPLITTER "showmanager/vsplitter"

ShowManager* ShowManager::s_instance = NULL;

ShowManager::ShowManager(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_show(NULL)
    , m_scene(NULL)
    , m_scene_editor(NULL)
    , m_current_editor(NULL)
    , m_selectedShowIndex(0)
    , m_splitter(NULL)
    , m_vsplitter(NULL)
    , m_showview(NULL)
    , m_toolbar(NULL)
    , m_showsCombo(NULL)
    , m_addShowAction(NULL)
    , m_addTrackAction(NULL)
    , m_addSequenceAction(NULL)
    , m_addAudioAction(NULL)
    , m_copyAction(NULL)
    , m_pasteAction(NULL)
    , m_deleteAction(NULL)
    , m_colorAction(NULL)
    , m_snapGridAction(NULL)
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
    connect(m_showview, SIGNAL(timeChanged(quint32)),
            this, SLOT(slotUpdateTime(quint32)));
    connect(m_showview, SIGNAL(trackClicked(Track*)),
            this, SLOT(slotTrackClicked(Track*)));
    connect(m_showview, SIGNAL(trackMoved(Track*,int)),
            this, SLOT(slotTrackMoved(Track*,int)));

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

    QSettings settings;
    QVariant var = settings.value(SETTINGS_HSPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());
    else
        m_splitter->setSizes(QList <int> () << int(this->width() / 2) << int(this->width() / 2));

    QVariant var2 = settings.value(SETTINGS_VSPLITTER);
    if (var2.isValid() == true)
        m_vsplitter->restoreState(var2.toByteArray());
    else
        m_vsplitter->setSizes(QList <int> () << int(this->width() / 2) << int(this->width() / 2));
}

ShowManager::~ShowManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_HSPLITTER, m_splitter->saveState());
    settings.setValue(SETTINGS_VSPLITTER, m_vsplitter->saveState());

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
    m_copyAction = new QAction(QIcon(":/editcopy.png"),
                                tr("&Copy"), this);
    m_copyAction->setShortcut(QKeySequence("CTRL+C"));
    connect(m_copyAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCopy()));
    m_copyAction->setEnabled(false);

    m_pasteAction = new QAction(QIcon(":/editpaste.png"),
                               tr("&Paste"), this);
    connect(m_pasteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPaste()));
    m_pasteAction->setEnabled(false);

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

    m_snapGridAction = new QAction(QIcon(":/grid.png"),
                                   tr("Snap to &Grid"), this);
    m_snapGridAction->setShortcut(QKeySequence("CTRL+G"));
    m_snapGridAction->setCheckable(true);
    connect(m_snapGridAction, SIGNAL(triggered(bool)),
           this, SLOT(slotToggleSnapToGrid(bool)));

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
    m_showsCombo->setFixedWidth(250);
    m_showsCombo->setMaxVisibleItems(30);
    connect(m_showsCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotShowsComboChanged(int)));
    m_toolbar->addWidget(m_showsCombo);
    m_toolbar->addSeparator();

    m_toolbar->addAction(m_addTrackAction);
    m_toolbar->addAction(m_addSequenceAction);
    m_toolbar->addAction(m_addAudioAction);

    m_toolbar->addSeparator();
    m_toolbar->addAction(m_copyAction);
    m_toolbar->addAction(m_pasteAction);
    m_toolbar->addAction(m_deleteAction);
    m_toolbar->addSeparator();

    m_toolbar->addAction(m_colorAction);
    m_toolbar->addAction(m_snapGridAction);
    m_toolbar->addSeparator();

    // Time label and playback buttons
    m_timeLabel = new QLabel("00:00:00.00");
    m_timeLabel->setFixedWidth(150);
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
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
    m_timeDivisionCombo->addItem(tr("Time"), SceneHeaderItem::Time);
    m_timeDivisionCombo->addItem("BPM 4/4", SceneHeaderItem::BPM_4_4);
    m_timeDivisionCombo->addItem("BPM 3/4", SceneHeaderItem::BPM_3_4);
    m_timeDivisionCombo->addItem("BPM 2/4", SceneHeaderItem::BPM_2_4);
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

    // protect poor Show Manager from drawing all the shows
    disconnect(m_showsCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotShowsComboChanged(int)));

    m_showsCombo->clear();
    foreach (Function* f, m_doc->functionsByType(Function::Show))
    {
        m_showsCombo->addItem(f->name(), QVariant(f->id()));
        if (m_show != NULL && m_show->id() != f->id())
            newIndex++;
    }
    if (m_showsCombo->count() > 0)
    {
        m_addTrackAction->setEnabled(true);
    }
    else
    {
        m_addTrackAction->setEnabled(false);
        m_addSequenceAction->setEnabled(false);
        m_addAudioAction->setEnabled(false);
    }

    if (m_show == NULL || m_show->getTracksCount() == 0)
    {
        m_deleteAction->setEnabled(false);
        m_pasteAction->setEnabled(false);
    }
    else
    {
        if (m_doc->clipboard()->hasFunction())
            m_pasteAction->setEnabled(true);
        m_deleteAction->setEnabled(true);
    }

    connect(m_showsCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotShowsComboChanged(int)));

    if (m_selectedShowIndex < 0 || m_selectedShowIndex >= m_showsCombo->count())
        m_selectedShowIndex = 0;

    m_showsCombo->setCurrentIndex(m_selectedShowIndex);
    updateMultiTrackView();
}

void ShowManager::slotShowsComboChanged(int idx)
{
    qDebug() << Q_FUNC_INFO << "Idx: " << idx;
    if (m_selectedShowIndex != idx)
    {
        m_selectedShowIndex = idx;
        hideRightEditor();
        updateMultiTrackView();
    }
}

void ShowManager::showSceneEditor(Scene *scene)
{
    if (m_scene_editor != NULL)
    {
        emit functionManagerActive(false);
        m_splitter->widget(1)->layout()->removeWidget(m_scene_editor);
        m_splitter->widget(1)->hide();
        m_scene_editor->deleteLater();
        m_scene_editor = NULL;
    }

    if (scene == NULL)
        return;

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

void ShowManager::hideRightEditor()
{
    if (m_current_editor != NULL)
    {
        m_vsplitter->widget(1)->layout()->removeWidget(m_current_editor);
        m_vsplitter->widget(1)->hide();
        m_current_editor->deleteLater();
        m_current_editor = NULL;
    }
}

void ShowManager::showRightEditor(Chaser *chaser)
{
    hideRightEditor();

    if (chaser == NULL)
        return;

    if (this->isVisible())
    {
        m_current_editor = new ChaserEditor(m_vsplitter->widget(1), chaser, m_doc);
        if (m_current_editor != NULL)
        {
            m_vsplitter->widget(1)->layout()->addWidget(m_current_editor);
            /** Signal from chaser editor to scene editor. When a step is clicked apply values immediately */
            connect(m_current_editor, SIGNAL(applyValues(QList<SceneValue>&)),
                    m_scene_editor, SLOT(slotSetSceneValues(QList <SceneValue>&)));
            /** Signal from scene editor to chaser editor. When a fixture value is changed, update the selected chaser step */
            connect(m_scene_editor, SIGNAL(fixtureValueChanged(SceneValue)),
                    m_current_editor, SLOT(slotUpdateCurrentStep(SceneValue)));
            connect(m_current_editor, SIGNAL(stepSelectionChanged(int)),
                    this, SLOT(slotStepSelectionChanged(int)));

            m_vsplitter->widget(1)->show();
            m_current_editor->show();
        }
    }
}

void ShowManager::showRightEditor(Audio *audio)
{
    hideRightEditor();

    if (audio == NULL)
        return;

    if (this->isVisible())
    {
        m_current_editor = new AudioEditor(m_vsplitter->widget(1), audio, m_doc);
        if (m_current_editor != NULL)
        {
            m_vsplitter->widget(1)->layout()->addWidget(m_current_editor);
            m_vsplitter->widget(1)->show();
            m_current_editor->show();
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
        // modify the new selected Show index
        m_selectedShowIndex = m_showsCombo->count();
        updateShowsCombo();
        m_copyAction->setEnabled(false);
        if (m_doc->clipboard()->hasFunction())
            m_pasteAction->setEnabled(true);
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
            else
            {
                delete m_scene;
                return;
            }
        }
        else
        {
            m_scene = qobject_cast<Scene*>(m_doc->function(ss.getSelectedID()));
        }
        if (m_scene == NULL)
            return;

        Track* newTrack = new Track(m_scene->id());
        newTrack->setName(m_scene->name());
        m_show->addTrack(newTrack);
        showSceneEditor(m_scene);
        m_showview->addTrack(newTrack);

        // When adding an existing Scene
        if (ss.getSelectedID() != Scene::invalidId())
        {
            bool childrenFound = false;
            // See if the Scene has children Sequences
            foreach (Function *f, m_doc->functionsByType(Function::Chaser))
            {
                Chaser *chs = qobject_cast<Chaser*>(f);
                if (chs->isSequence())
                {
                    if (chs->getBoundSceneID() == ss.getSelectedID())
                    {
                        newTrack->addFunctionID(chs->id());
                        m_showview->addSequence(chs);
                        childrenFound = true;
                    }
                }
            }

            // If an existing Scene does not have sequunce children,
            // then create a default 10 seconds Sequence
            if (childrenFound == false)
            {
                Function* f = new Chaser(m_doc);
                if (m_doc->addFunction(f) == true)
                {
                    Chaser *chaser = qobject_cast<Chaser*> (f);
                    chaser->enableSequenceMode(m_scene->id());
                    chaser->setRunOrder(Function::SingleShot);
                    chaser->setDurationMode(Chaser::PerStep);
                    m_scene->setChildrenFlag(true);
                    f->setName(QString("%1 %2").arg(tr("New Sequence")).arg(f->id()));
                    newTrack->addFunctionID(chaser->id());
                    m_showview->addSequence(chaser);
                    ChaserStep step(m_scene->id(), m_scene->fadeInSpeed(), 10000, m_scene->fadeOutSpeed());
                    step.note = QString();
                    step.values.append(m_scene->values());
                    chaser->addStep(step);
                }
            }
        }

        m_addSequenceAction->setEnabled(true);
        m_addAudioAction->setEnabled(true);
        m_showview->activateTrack(newTrack);
        m_deleteAction->setEnabled(true);
        m_showview->updateViewSize();
    }
}

void ShowManager::slotAddSequence()
{
    // Overlapping check
    if (checkOverlapping(m_showview->getTimeFromCursor(), 1000) == true)
    {
        QMessageBox::warning(this, tr("Overlapping error"), tr("Overlapping not allowed. Operation cancelled."));
        return;
    }

    Function* f = new Chaser(m_doc);
    Chaser *chaser = qobject_cast<Chaser*> (f);
    chaser->enableSequenceMode(m_scene->id());

    if (m_doc->addFunction(f) == true)
    {
        chaser->setRunOrder(Function::SingleShot);
        m_scene->setChildrenFlag(true);
        f->setName(QString("%1 %2").arg(tr("New Sequence")).arg(f->id()));
        showRightEditor(chaser);
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
        delete f;
        return;
    }
    // Overlapping check
    if (checkOverlapping(m_showview->getTimeFromCursor(), audio->getDuration()) == true)
    {
        QMessageBox::warning(this, tr("Overlapping error"), tr("Overlapping not allowed. Operation cancelled."));
        delete f;
        return;
    }
    if (m_doc->addFunction(f) == true)
    {
        Track *track = m_show->getTrackFromSceneID(m_scene->id());
        track->addFunctionID(audio->id());
        m_showview->addAudio(audio);
    }

}

void ShowManager::slotCopy()
{
    quint32 fid = Function::invalidId();
    AudioItem *audItem = NULL;

    SequenceItem *seqItem = m_showview->getSelectedSequence();
    if (seqItem != NULL)
    {
        fid = seqItem->getChaser()->id();
    }
    else
    {
        audItem = m_showview->getSelectedAudio();
        if (audItem != NULL)
            fid = audItem->getAudio()->id();

        if (audItem == NULL)
        {
            fid = m_scene->id();
        }
    }

    if (fid != Function::invalidId())
    {
        Function* function = m_doc->function(fid);
        Q_ASSERT(function != NULL);

        m_doc->clipboard()->copyContent(m_show->id(), function);
        m_pasteAction->setEnabled(true);
    }
}

void ShowManager::slotPaste()
{
    if (m_doc->clipboard()->hasFunction() == false)
        return;

    // Get the Function copy and add it to Doc
    Function* clipboardCopy = m_doc->clipboard()->getFunction();
    quint32 copyDuration = 0;
    if (clipboardCopy->type() == Function::Chaser)
        copyDuration = (qobject_cast<Chaser*>(clipboardCopy))->getDuration();
    else if (clipboardCopy->type() == Function::Audio)
        copyDuration = (qobject_cast<Audio*>(clipboardCopy))->getDuration();

    // Overlapping check
    if (checkOverlapping(m_showview->getTimeFromCursor(), copyDuration) == true)
    {
        QMessageBox::warning(this, tr("Paste error"), tr("Overlapping paste not allowed. Operation cancelled."));
        return;
    }
    //qDebug() << "Check overlap... cursor time:" << cursorTime << "msec";

    if (clipboardCopy != NULL)
    {
        // copy the function again, to allow multiple copies of the same function
        Function* newCopy = clipboardCopy->createCopy(m_doc, false);
        if (newCopy == NULL)
            return;

        if (clipboardCopy->type() == Function::Chaser)
        {
            Chaser *chaser = qobject_cast<Chaser*>(newCopy);
            // Verify the Chaser copy steps against the current Scene
            foreach(ChaserStep cs, chaser->steps())
            {
                foreach(SceneValue scv, cs.values)
                {
                    if (m_scene->checkValue(scv) == false)
                    {
                        QMessageBox::warning(this, tr("Paste error"), tr("Trying to paste on an incompatible Scene. Operation cancelled."));
                        return;
                    }
                }
            }

            // Invalidate start time so the sequence will be pasted at the cursor position
            chaser->setStartTime(UINT_MAX);
            // Reset the Scene ID to bind to the correct Scene
            chaser->enableSequenceMode(m_scene->id());
            if (m_doc->addFunction(newCopy) == false)
            {
                delete newCopy;
                return;
            }
            Track *track = m_show->getTrackFromSceneID(m_scene->id());
            track->addFunctionID(chaser->id());
            m_showview->addSequence(chaser);
        }
        else if (clipboardCopy->type() == Function::Audio)
        {
            if (m_doc->addFunction(newCopy) == false)
            {
                delete newCopy;
                return;
            }
            Audio *audio = qobject_cast<Audio*>(newCopy);
            // Invalidate start time so the sequence will be pasted at the cursor position
            audio->setStartTime(UINT_MAX);

            Track *track = m_show->getTrackFromSceneID(m_scene->id());
            track->addFunctionID(audio->id());
            m_showview->addAudio(audio);
        }
        else if (clipboardCopy->type() == Function::Scene)
        {
            if (m_doc->addFunction(newCopy) == false)
                return;
            m_scene = qobject_cast<Scene*>(newCopy);
            Track* newTrack = new Track(m_scene->id());
            newTrack->setName(m_scene->name());
            m_show->addTrack(newTrack);
            showSceneEditor(m_scene);
            m_showview->addTrack(newTrack);
            m_addSequenceAction->setEnabled(true);
            m_addAudioAction->setEnabled(true);
            m_showview->activateTrack(newTrack);
            m_deleteAction->setEnabled(true);
            m_showview->updateViewSize();
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
            {
                currTrack->removeFunctionID(deleteID);
                hideRightEditor();
            }
        }
        else
        {
            m_show->removeTrack(deleteID);
            m_doc->setModified();
            updateMultiTrackView();
        }

        //m_doc->deleteFunction(deleteID);
    }
}

void ShowManager::slotStopPlayback()
{
    if (m_show != NULL && m_show->isRunning())
    {
        m_show->stop();
        return;
    }
    m_showview->rewindCursor();
    m_timeLabel->setText("00:00:00.00");
}

void ShowManager::slotStartPlayback()
{
    if (m_showsCombo->count() == 0 || m_show == NULL)
        return;
    m_show->start(m_doc->masterTimer(), false, m_showview->getTimeFromCursor());
}

void ShowManager::slotShowStopped()
{
    slotUpdateTime(m_showview->getTimeFromCursor());
}

void ShowManager::slotTimeDivisionTypeChanged(int idx)
{
    QVariant var = m_timeDivisionCombo->itemData(idx);
    if (var.isValid())
    {
        m_showview->setHeaderType((SceneHeaderItem::TimeDivision)var.toInt());
        if (idx > 0)
            m_bpmField->setEnabled(true);
        else
            m_bpmField->setEnabled(false);
        if (m_show != NULL)
            m_show->setTimeDivision(SceneHeaderItem::tempoToString((SceneHeaderItem::TimeDivision)var.toInt()), m_bpmField->value());
    }
}

void ShowManager::slotBPMValueChanged(int value)
{
    m_showview->setBPMValue(value);
    QVariant var = m_timeDivisionCombo->itemData(m_timeDivisionCombo->currentIndex());
    if (var.isValid() && m_show != NULL)
        m_show->setTimeDivision(SceneHeaderItem::tempoToString((SceneHeaderItem::TimeDivision)var.toInt()), m_bpmField->value());
}

void ShowManager::slotViewClicked(QMouseEvent *event)
{
    Q_UNUSED(event)
    //qDebug() << Q_FUNC_INFO << "View clicked at pos: " << event->pos().x() << event->pos().y();
    hideRightEditor();
    m_colorAction->setEnabled(false);
    if (m_show != NULL && m_show->getTracksCount() == 0)
        m_deleteAction->setEnabled(false);
}

void ShowManager::slotSequenceMoved(SequenceItem *item)
{
    qDebug() << Q_FUNC_INFO << "Sequence moved...";
    Chaser *chaser = item->getChaser();
    if (chaser == NULL)
        return;
    /* Check if scene has changed */
    quint32 newSceneID = chaser->getBoundSceneID();
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
    showRightEditor(chaser);
    m_copyAction->setEnabled(true);
    m_deleteAction->setEnabled(true);
    m_colorAction->setEnabled(true);
    m_doc->setModified();
}

void ShowManager::slotAudioMoved(AudioItem *item)
{
    qDebug() << Q_FUNC_INFO << "Audio moved.........";
    Audio *audio = item->getAudio();
    if (audio == NULL)
        return;
    // reverse lookup of Track from an Audio item
    foreach(Track *track, m_show->tracks())
    {
        foreach(quint32 fid, track->functionsID())
        {
            if (fid == audio->id())
            {
                m_showview->activateTrack(track);
                Function *f = m_doc->function(track->getSceneID());
                if (f == NULL)
                    return;
                m_scene = qobject_cast<Scene*>(f);
                showSceneEditor(NULL);
            }
        }
    }

    showRightEditor(audio);
    m_copyAction->setEnabled(true);
    m_deleteAction->setEnabled(true);
    m_colorAction->setEnabled(true);
    m_doc->setModified();
}

void ShowManager::slotupdateTimeAndCursor(quint32 msec_time)
{
    //qDebug() << Q_FUNC_INFO << "time: " << msec_time;
    slotUpdateTime(msec_time);
    m_showview->moveCursor(msec_time);
}

void ShowManager::slotUpdateTime(quint32 msec_time)
{
    uint h, m, s;

    h = msec_time / MS_PER_HOUR;
    msec_time -= (h * MS_PER_HOUR);

    m = msec_time / MS_PER_MINUTE;
    msec_time -= (m * MS_PER_MINUTE);

    s = msec_time / MS_PER_SECOND;
    msec_time -= (s * MS_PER_SECOND);

    QString str;
    if (m_show && m_show->isRunning())
    {
        str = QString("%1:%2:%3.%4").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'))
              .arg(s, 2, 10, QChar('0')).arg(msec_time / 100, 1, 10, QChar('0'));
    }
    else
        str = QString("%1:%2:%3.%4").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'))
              .arg(s, 2, 10, QChar('0')).arg(msec_time / 10, 2, 10, QChar('0'));

    m_timeLabel->setText(str);
}

void ShowManager::slotTrackClicked(Track *track)
{
    Function *f = m_doc->function(track->getSceneID());
    if (f == NULL)
        return;
    m_scene = qobject_cast<Scene*>(f);
    showSceneEditor(m_scene);
    m_deleteAction->setEnabled(true);
    m_copyAction->setEnabled(true);
}

void ShowManager::slotTrackMoved(Track *track, int direction)
{
    if (m_show != NULL)
        m_show->moveTrack(track, direction);
    updateMultiTrackView();
    m_doc->setModified();
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

void ShowManager::slotToggleSnapToGrid(bool enable)
{
    m_showview->setSnapToGrid(enable);
}

void ShowManager::slotChangeSize(int width, int height)
{
    if (m_showview != NULL)
        m_showview->setViewSize(width, height);
}

void ShowManager::slotStepSelectionChanged(int index)
{
    SequenceItem *seqItem = m_showview->getSelectedSequence();
    if (seqItem != NULL)
    {
        seqItem->setSelectedStep(index);
    }
}

void ShowManager::slotDocClearing()
{
    m_showsCombo->clear();

    if (m_showview != NULL)
        m_showview->resetView();

    if (m_current_editor != NULL)
    {
        m_vsplitter->widget(1)->layout()->removeWidget(m_current_editor);
        m_current_editor->deleteLater();
        m_current_editor = NULL;
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
    m_copyAction->setEnabled(false);
    m_deleteAction->setEnabled(false);
    m_colorAction->setEnabled(false);
    m_timeLabel->setText("00:00:00.00");
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
        {
            int idx = m_show->getAttributeIndex(trk->name());
            m_show->renameAttribute(idx, function->name());
            trk->setName(function->name());
        }
    }
}

void ShowManager::slotFunctionRemoved(quint32 id)
{
    /* Here we handle only the cases where 'id' */
    /* is a Scene (Track) or a Chaser (Sequence) */
    foreach (Function *function, m_doc->functionsByType(Function::Show))
    {
        Show *show = qobject_cast<Show*>(function);
        foreach(Track *track, show->tracks())
        {
            // if 'id' is a track, remove all the functions associated to it
            if (track->id() == id)
            {
                foreach (quint32 fid, track->functionsID())
                {
                    track->removeFunctionID(fid);
                }
            }
            // if 'id' is a sequence or an audio item then remove it
            else
            {
                track->removeFunctionID(id);
            }
        }
    }

    if (m_show != NULL && m_show->id() == id)
        m_show = NULL;
    if (m_scene != NULL && m_scene->id() == id)
        m_scene = NULL;

    if (isVisible())
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

    // disconnect BPM field and update the view manually, to
    // prevent m_show time division override
    disconnect(m_bpmField, SIGNAL(valueChanged(int)), this, SLOT(slotBPMValueChanged(int)));

    m_bpmField->setValue(m_show->getTimeDivisionBPM());
    m_showview->setBPMValue(m_show->getTimeDivisionBPM());
    int tIdx = m_timeDivisionCombo->findData(QVariant(SceneHeaderItem::stringToTempo(m_show->getTimeDivisionType())));
    m_timeDivisionCombo->setCurrentIndex(tIdx);

    connect(m_bpmField, SIGNAL(valueChanged(int)), this, SLOT(slotBPMValueChanged(int)));
    connect(m_show, SIGNAL(timeChanged(quint32)), this, SLOT(slotupdateTimeAndCursor(quint32)));
    connect(m_show, SIGNAL(showFinished()), this, SLOT(slotStopPlayback()));
    connect(m_show, SIGNAL(stopped(quint32)), this, SLOT(slotShowStopped()));

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
        m_copyAction->setEnabled(true);
        m_addSequenceAction->setEnabled(true);
        m_addAudioAction->setEnabled(true);
    }
    else
    {
        m_addSequenceAction->setEnabled(false);
        m_addAudioAction->setEnabled(false);
        m_scene = NULL;
        showSceneEditor(NULL);
    }
    if (m_doc->clipboard()->hasFunction())
        m_pasteAction->setEnabled(true);
    m_showview->updateViewSize();
}

bool ShowManager::checkOverlapping(quint32 startTime, quint32 duration)
{
    if (m_scene == NULL)
        return false;

    Track *track = m_show->getTrackFromSceneID(m_scene->id());

    foreach(quint32 fid, track->functionsID())
    {
        Function *func = m_doc->function(fid);
        if (func != NULL)
        {
            if (func->type() == Function::Chaser)
            {
                Chaser *chaser = qobject_cast<Chaser*>(func);
                quint32 chsST = chaser->getStartTime();
                //qDebug() << "Chaser ID:" << chaser->id() << ", start time:" << chsST << ", duration:" << chaser->getDuration();
                if ((startTime >= chsST && startTime <= chsST + chaser->getDuration()) ||
                    (chsST >= startTime && chsST <= startTime + duration))
                {
                    return true;
                }
            }
            else if (func->type() == Function::Audio)
            {
                Audio *audio = qobject_cast<Audio*>(func);
                quint32 audST = audio->getStartTime();
                if ((startTime >= audST && startTime <= audST + audio->getDuration()) ||
                    (audST >= startTime && audST <= startTime + duration))
                {
                    return true;
                }
            }
        }
    }

    return false;
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
    
    if (m_current_editor != NULL)
    {
        m_vsplitter->widget(1)->layout()->removeWidget(m_current_editor);
        m_vsplitter->widget(1)->hide();
        m_current_editor->deleteLater();
        m_current_editor = NULL;
    }

    if (m_scene_editor != NULL)
    {
        m_splitter->widget(1)->layout()->removeWidget(m_scene_editor);
        m_splitter->widget(1)->hide();
        m_scene_editor->deleteLater();
        m_scene_editor = NULL;
    }
}
