/*
  Q Light Controller
  simpledesk.cpp

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

#include <QDomDocument>
#include <QInputDialog>
#include <QDomElement>
#include <QToolButton>
#include <QHeaderView>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QGroupBox>
#include <QTreeView>
#include <QComboBox>
#include <QSpinBox>
#include <QLayout>
#include <QLabel>
#include <QDebug>

#include "grandmasterslider.h"
#include "simpledeskengine.h"
#include "speeddialwidget.h"
#include "playbackslider.h"
#include "cuestackmodel.h"
#include "simpledesk.h"
#include "consolechannel.h"
#include "qlcmacros.h"
#include "cuestack.h"
#include "cue.h"
#include "doc.h"

#define PROP_ADDRESS    "address"
#define PROP_PLAYBACK   "playback"

#define SETTINGS_SPLITTER       "simpledesk/splitter"
#define SETTINGS_PAGE_CHANNELS  "simpledesk/channelsperpage"
#define SETTINGS_PAGE_PLAYBACKS "simpledesk/playbacksperpage"
#define SETTINGS_CHANNEL_NAMES  "simpledesk/showchannelnames"
#define DEFAULT_PAGE_CHANNELS   16
#define DEFAULT_PAGE_PLAYBACKS  12

SimpleDesk* SimpleDesk::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

SimpleDesk::SimpleDesk(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_engine(new SimpleDeskEngine(doc))
    , m_doc(doc)
    , m_currentUniverse(0)
    , m_channelsPerPage(DEFAULT_PAGE_CHANNELS)
    , m_selectedPlayback(UINT_MAX)
    , m_playbacksPerPage(DEFAULT_PAGE_PLAYBACKS)
    , m_speedDials(NULL)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(doc != NULL);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_PAGE_CHANNELS);
    if (var.isValid() == true)
        m_channelsPerPage = var.toUInt();

    var = settings.value(SETTINGS_PAGE_PLAYBACKS);
    if (var.isValid() == true)
        m_playbacksPerPage = var.toUInt();

    // default all the universes pages to 1
    for (quint32 i = 0; i < m_doc->outputMap()->universes(); i++)
        m_universesPage.append(1);

    initEngine();
    initView();
    initUniverseSliders();
    initUniversePager();
    initPlaybackSliders();
    initCueStack();

    slotSelectPlayback(0);

    connect(m_doc->outputMap(), SIGNAL(universesWritten(const QByteArray&)),
            this, SLOT(slotUniversesWritten(const QByteArray&)));
}

SimpleDesk::~SimpleDesk()
{
    qDebug() << Q_FUNC_INFO;

    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());

    Q_ASSERT(m_engine != NULL);
    delete m_engine;
    m_engine = NULL;

    s_instance = NULL;
}

SimpleDesk* SimpleDesk::instance()
{
    return s_instance;
}

void SimpleDesk::clearContents()
{
    qDebug() << Q_FUNC_INFO;
    CueStackModel* model = qobject_cast<CueStackModel*> (m_cueStackView->model());
    Q_ASSERT(model != NULL);
    model->setCueStack(NULL);

    resetUniverseSliders();
    resetPlaybackSliders();
    m_engine->clearContents();
    slotSelectPlayback(0);
}

void SimpleDesk::initEngine()
{
    qDebug() << Q_FUNC_INFO;
    connect(m_engine, SIGNAL(cueStackStarted(uint)), this, SLOT(slotCueStackStarted(uint)));
    connect(m_engine, SIGNAL(cueStackStopped(uint)), this, SLOT(slotCueStackStopped(uint)));
}

void SimpleDesk::initView()
{
    qDebug() << Q_FUNC_INFO;

    new QHBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(m_splitter);

    initLeftSide();
    initRightSide();

    QSettings settings;
    m_splitter->restoreState(settings.value(SETTINGS_SPLITTER).toByteArray());
}

void SimpleDesk::initLeftSide()
{
    qDebug() << Q_FUNC_INFO;

    QWidget* leftSide = new QWidget(this);
    QVBoxLayout* lay = new QVBoxLayout(leftSide);
    lay->setContentsMargins(1, 1, 1, 1);
    m_splitter->addWidget(leftSide);

    QHBoxLayout* uniLay = new QHBoxLayout;
    QLabel *label = new QLabel(tr("Universe"));
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    uniLay->addWidget(label);
    m_universesCombo = new QComboBox(this);
    m_universesCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    uniLay->addWidget(m_universesCombo);
    lay->addLayout(uniLay);

    initUniversesCombo();
    connect(m_universesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniversesComboChanged(int)));

    m_universeGroup = new QGroupBox(this);
    //m_universeGroup->setTitle(tr("Universe"));
    QHBoxLayout* grpLay = new QHBoxLayout(m_universeGroup);
    grpLay->setContentsMargins(0, 6, 0, 0);
    grpLay->setSpacing(1);
    lay->addWidget(m_universeGroup);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    m_universePageUpButton = new QToolButton(this);
    m_universePageUpButton->setIcon(QIcon(":/forward.png"));
    m_universePageUpButton->setIconSize(QSize(32, 32));
    m_universePageUpButton->setToolTip(tr("Next page"));
    vbox->addWidget(m_universePageUpButton);

    m_universePageSpin = new QSpinBox(this);
    m_universePageSpin->setMaximumSize(QSize(40, 40));
    m_universePageSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_universePageSpin->setWrapping(true);
    m_universePageUpButton->setToolTip(tr("Current page"));
    vbox->addWidget(m_universePageSpin);

    m_universePageDownButton = new QToolButton(this);
    m_universePageDownButton->setIcon(QIcon(":/back.png"));
    m_universePageDownButton->setIconSize(QSize(32, 32));
    m_universePageUpButton->setToolTip(tr("Previous page"));
    vbox->addWidget(m_universePageDownButton);

    m_universeResetButton = new QToolButton(this);
    m_universeResetButton->setIcon(QIcon(":/fileclose.png"));
    m_universeResetButton->setIconSize(QSize(32, 32));
    m_universeResetButton->setToolTip(tr("Reset universe"));
    vbox->addWidget(m_universeResetButton);

    m_grandMasterSlider = new GrandMasterSlider(this, m_doc->outputMap(), m_doc->inputMap());
    vbox->addWidget(m_grandMasterSlider);

    grpLay->addLayout(vbox);

    m_playbackGroup = new QGroupBox(this);
    m_playbackGroup->setTitle(tr("Playback"));
    grpLay = new QHBoxLayout(m_playbackGroup);
    grpLay->setContentsMargins(0, 6, 0, 0);
    grpLay->setSpacing(1);
    lay->addWidget(m_playbackGroup);
}

void SimpleDesk::initRightSide()
{
    qDebug() << Q_FUNC_INFO;

    QWidget* rightSide = new QWidget(this);
    QVBoxLayout* lay = new QVBoxLayout(rightSide);
    lay->setContentsMargins(1, 1, 1, 1);
    m_splitter->addWidget(rightSide);

    m_cueStackGroup = new QGroupBox(this);
    m_cueStackGroup->setTitle(tr("Cue Stack"));
    QVBoxLayout* grpLay = new QVBoxLayout(m_cueStackGroup);
    grpLay->setContentsMargins(0, 6, 0, 0);
    lay->addWidget(m_cueStackGroup);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    m_previousCueButton = new QToolButton(this);
    m_previousCueButton->setIcon(QIcon(":/goprevious.png"));
    m_previousCueButton->setIconSize(QSize(32, 32));
    m_previousCueButton->setToolTip(tr("Previous cue"));
    hbox->addWidget(m_previousCueButton);

    m_stopCueStackButton = new QToolButton(this);
    m_stopCueStackButton->setIcon(QIcon(":/stop.png"));
    m_stopCueStackButton->setIconSize(QSize(32, 32));
    m_stopCueStackButton->setToolTip(tr("Stop cue stack"));
    hbox->addWidget(m_stopCueStackButton);

    m_nextCueButton = new QToolButton(this);
    m_nextCueButton->setIcon(QIcon(":/gonext.png"));
    m_nextCueButton->setIconSize(QSize(32, 32));
    m_nextCueButton->setToolTip(tr("Next cue"));
    hbox->addWidget(m_nextCueButton);

    hbox->addStretch();

    m_cloneCueStackButton = new QToolButton(this);
    m_cloneCueStackButton->setIcon(QIcon(":/editcopy.png"));
    m_cloneCueStackButton->setIconSize(QSize(32, 32));
    m_cloneCueStackButton->setToolTip(tr("Clone cue stack"));
    hbox->addWidget(m_cloneCueStackButton);

    m_editCueStackButton = new QToolButton(this);
    m_editCueStackButton->setIcon(QIcon(":/edit.png"));
    m_editCueStackButton->setIconSize(QSize(32, 32));
    m_editCueStackButton->setToolTip(tr("Edit cue stack"));
    m_editCueStackButton->setCheckable(true);
    hbox->addWidget(m_editCueStackButton);

    m_recordCueButton = new QToolButton(this);
    m_recordCueButton->setIcon(QIcon(":/record.png"));
    m_recordCueButton->setIconSize(QSize(32, 32));
    m_recordCueButton->setToolTip(tr("Record cue"));
    hbox->addWidget(m_recordCueButton);

    grpLay->addLayout(hbox);

    m_cueStackView = new QTreeView(this);
    m_cueStackView->setAllColumnsShowFocus(true);
    m_cueStackView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_cueStackView->setDragEnabled(true);
    m_cueStackView->setDragDropMode(QAbstractItemView::InternalMove);
    m_cueStackGroup->layout()->addWidget(m_cueStackView);
}

/****************************************************************************
 * Universe controls
 ****************************************************************************/

void SimpleDesk::initUniversesCombo()
{
    disconnect(m_universesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniversesComboChanged(int)));
    m_universesCombo->clear();
    m_universesCombo->addItems(m_doc->outputMap()->universeNames());
    connect(m_universesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniversesComboChanged(int)));
}

void SimpleDesk::initUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    quint32 start = (m_universePageSpin->value() - 1) * m_channelsPerPage;
    for (quint32 i = 0; i < m_channelsPerPage; i++)
    {
        ConsoleChannel* slider = NULL;
        const Fixture* fxi = m_doc->fixture(m_doc->fixtureForAddress(start + i));
        if (fxi == NULL)
            slider = new ConsoleChannel(this, m_doc, Fixture::invalidId(), i, false);
        else
        {
            uint ch = (start + i) - fxi->universeAddress();
            slider = new ConsoleChannel(this, m_doc, fxi->id(), ch, false);
        }
        m_universeGroup->layout()->addWidget(slider);
        m_universeSliders << slider;
        connect(slider, SIGNAL(valueChanged(quint32,quint32,uchar)),
                this, SLOT(slotUniverseSliderValueChanged(quint32,quint32,uchar)));
    }

    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotUpdateUniverseSliders()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotUpdateUniverseSliders()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)), this, SLOT(slotUpdateUniverseSliders()));
}

void SimpleDesk::initUniversePager()
{
    qDebug() << Q_FUNC_INFO;
    m_universePageSpin->setRange(1, int(512 / m_channelsPerPage));
    m_universePageSpin->setValue(1);
    slotUniversePageChanged(1);

    connect(m_universePageUpButton, SIGNAL(clicked()), this, SLOT(slotUniversePageUpClicked()));
    connect(m_universePageDownButton, SIGNAL(clicked()), this, SLOT(slotUniversePageDownClicked()));
    connect(m_universePageSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUniversePageChanged(int)));
    connect(m_universeResetButton, SIGNAL(clicked()), this, SLOT(slotUniverseResetClicked()));
}

void SimpleDesk::resetUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    QListIterator <ConsoleChannel*> it(m_universeSliders);
    while (it.hasNext() == true)
        it.next()->setValue(0);
}

void SimpleDesk::slotUniversesComboChanged(int index)
{
    m_currentUniverse = index;
    int page = m_universesPage.at(index);
    m_universePageSpin->setValue(page);
    slotUniversePageChanged(page);
}

void SimpleDesk::slotUniversePageUpClicked()
{
    qDebug() << Q_FUNC_INFO;
    m_universePageSpin->setValue(m_universePageSpin->value() + 1);
}

void SimpleDesk::slotUniversePageDownClicked()
{
    qDebug() << Q_FUNC_INFO;
    m_universePageSpin->setValue(m_universePageSpin->value() - 1);
}

void SimpleDesk::slotUniversePageChanged(int page)
{
    qDebug() << Q_FUNC_INFO;
    quint32 start = (page - 1) * m_channelsPerPage;
    QString ssEven =  "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #C3D1C9, stop: 1 #AFBBB4); "
                     " border: 1px solid gray; border-radius: 4px; }";
    QString ssOdd = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D2D0, stop: 1 #AFACAB); "
                     " border: 1px solid gray; border-radius: 4px; }";
    QString ssNone = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D5E0, stop: 1 #A7A6AF); "
                     " border: 1px solid gray; border-radius: 4px; }";

    /* now, calculate the absolute address including current universe (0 - 2048) */
    quint32 absoluteAddr = start | (m_currentUniverse << 9);

    /* Set the new page for this universe */
    m_universesPage.replace(m_currentUniverse, page);

    //qDebug() << "[slotUniversePageChanged] start: " << start << ", absoluteAddr: " << absoluteAddr;

    for (quint32 i = 0; i < m_channelsPerPage; i++)
    {
        ConsoleChannel* slider = m_universeSliders[i];
        if (slider != NULL)
        {
            m_universeGroup->layout()->removeWidget(slider);
            disconnect(slider, SIGNAL(valueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotUniverseSliderValueChanged(quint32,quint32,uchar)));
            delete slider;
        }
        const Fixture* fx = m_doc->fixture(m_doc->fixtureForAddress(absoluteAddr + i));
        if (fx == NULL)
        {
            slider = new ConsoleChannel(this, m_doc, Fixture::invalidId(), start + i, false);
            slider->setStyleSheet(ssNone);
        }
        else
        {
            uint ch = (absoluteAddr + i) - fx->universeAddress();
            slider = new ConsoleChannel(this, m_doc, fx->id(), ch, false);
            if (fx->id() % 2 == 0)
                slider->setStyleSheet(ssOdd);
            else
                slider->setStyleSheet(ssEven);
        }

        if ((start + i) < 512)
        {
            slider->setEnabled(true);
            slider->setProperty(PROP_ADDRESS, absoluteAddr + i);
            slider->setLabel(QString::number(start + i + 1));
            //qDebug() << "Set slider value[" << (absoluteAddr + i) << "] = " << m_engine->value(absoluteAddr + i);
            slider->setValue(m_engine->value(absoluteAddr + i), false);
            connect(slider, SIGNAL(valueChanged(quint32,quint32,uchar)),
                    this, SLOT(slotUniverseSliderValueChanged(quint32,quint32,uchar)));
        }
        else
        {
            slider->setEnabled(false);
            slider->setProperty(PROP_ADDRESS, QVariant());
            slider->setValue(0);
            slider->setLabel("---");
            slider->setPalette(this->palette());
        }

        m_universeGroup->layout()->addWidget(slider);
        m_universeSliders[i] = slider;
    }
}

void SimpleDesk::slotUniverseResetClicked()
{
    qDebug() << Q_FUNC_INFO;
    resetUniverseSliders();
    m_engine->resetUniverse(m_currentUniverse);
    slotUniversePageChanged(1);
}

void SimpleDesk::slotUniverseSliderValueChanged(quint32,quint32,uchar value)
{
    QVariant var(sender()->property(PROP_ADDRESS));
    if (var.isValid() == true) // Not true with disabled sliders
    {
        m_engine->setValue(var.toUInt(), value);

        if (m_editCueStackButton->isChecked() == true)
            replaceCurrentCue();
    }
}

void SimpleDesk::slotUniversesWritten(const QByteArray& ua)
{
    quint32 start = (m_universePageSpin->value() - 1) * m_channelsPerPage;
    // add the universe bits to retrieve the absolute address (0 - 2048)
    start = start | (m_currentUniverse << 9);

    for (quint32 i = 0; i < (quint32)ua.length(); i++)
    {
        //m_engine->setValue(i, ua.at(i));
        // update current page sliders
        if (i >= start && i < start + (quint32)m_channelsPerPage)
        {
            const Fixture* fx = m_doc->fixture(m_doc->fixtureForAddress(i));
            if (fx != NULL)
            {
                ConsoleChannel *cc = m_universeSliders[i - start];
                cc->setValue(ua.at(i), false);
            }
        }
    }
}

void SimpleDesk::slotUpdateUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    slotUniversePageChanged(m_universePageSpin->value());
}

/****************************************************************************
 * Playback Sliders
 ****************************************************************************/

void SimpleDesk::initPlaybackSliders()
{
    qDebug() << Q_FUNC_INFO;
    for (uint i = 0; i < m_playbacksPerPage; i++)
    {
        PlaybackSlider* slider = new PlaybackSlider(m_playbackGroup);
        m_playbackGroup->layout()->addWidget(slider);
        slider->setLabel(QString::number(i + 1));
        slider->setProperty(PROP_PLAYBACK, uint(i));
        m_playbackSliders << slider;
        connect(slider, SIGNAL(selected()), this, SLOT(slotPlaybackSelected()));
        connect(slider, SIGNAL(started()), this, SLOT(slotPlaybackStarted()));
        connect(slider, SIGNAL(stopped()), this, SLOT(slotPlaybackStopped()));
        connect(slider, SIGNAL(flashing(bool)), this, SLOT(slotPlaybackFlashing(bool)));
        connect(slider, SIGNAL(valueChanged(uchar)), this, SLOT(slotPlaybackValueChanged(uchar)));
    }
}

void SimpleDesk::resetPlaybackSliders()
{
    qDebug() << Q_FUNC_INFO;
    QListIterator <PlaybackSlider*> it(m_playbackSliders);
    while (it.hasNext() == true)
        it.next()->setValue(0);
}

void SimpleDesk::slotPlaybackSelected()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(sender() != NULL);
    uint pb = sender()->property(PROP_PLAYBACK).toUInt();
    if (m_selectedPlayback == pb)
        return;

    slotSelectPlayback(pb);
}

void SimpleDesk::slotSelectPlayback(uint pb)
{
    qDebug() << Q_FUNC_INFO;

    if (m_selectedPlayback != UINT_MAX)
        m_playbackSliders[m_selectedPlayback]->setSelected(false);

    if (pb != UINT_MAX)
        m_playbackSliders[pb]->setSelected(true);
    m_selectedPlayback = pb;

    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    CueStackModel* model = qobject_cast<CueStackModel*> (m_cueStackView->model());
    Q_ASSERT(model != NULL);
    model->setCueStack(cueStack);

    m_cueStackGroup->setTitle(tr("Cue Stack - Playback %1").arg(m_selectedPlayback + 1));

    updateCueStackButtons();
}

void SimpleDesk::slotPlaybackStarted()
{
    qDebug() << Q_FUNC_INFO;
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    if (cueStack->isRunning() == false)
        cueStack->nextCue();
}

void SimpleDesk::slotPlaybackStopped()
{
    qDebug() << Q_FUNC_INFO;
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    if (cueStack->isRunning() == true)
        cueStack->stop();
}

void SimpleDesk::slotPlaybackFlashing(bool enabled)
{
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    cueStack->setFlashing(enabled);
}

void SimpleDesk::slotPlaybackValueChanged(uchar value)
{
    int pb = sender()->property(PROP_PLAYBACK).toUInt();
    CueStack* cueStack = m_engine->cueStack(pb);
    Q_ASSERT(cueStack != NULL);

    cueStack->adjustIntensity(qreal(value) / qreal(UCHAR_MAX));
}

/****************************************************************************
 * Cue Stack controls
 ****************************************************************************/

void SimpleDesk::initCueStack()
{
    qDebug() << Q_FUNC_INFO;
    CueStackModel* model = new CueStackModel(this);
    m_cueStackView->setModel(model);
    m_cueStackView->header()->setResizeMode(QHeaderView::ResizeToContents);

    connect(m_previousCueButton, SIGNAL(clicked()), this, SLOT(slotPreviousCueClicked()));
    connect(m_nextCueButton, SIGNAL(clicked()), this, SLOT(slotNextCueClicked()));
    connect(m_stopCueStackButton, SIGNAL(clicked()), this, SLOT(slotStopCueStackClicked()));
    connect(m_cloneCueStackButton, SIGNAL(clicked()), this, SLOT(slotCloneCueStackClicked()));
    connect(m_editCueStackButton, SIGNAL(clicked()), this, SLOT(slotEditCueStackClicked()));
    connect(m_recordCueButton, SIGNAL(clicked()), this, SLOT(slotRecordCueClicked()));

    connect(m_cueStackView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this, SLOT(slotCueStackSelectionChanged()));
}

void SimpleDesk::updateCueStackButtons()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    if (cueStack == NULL)
        return;

    m_stopCueStackButton->setEnabled(cueStack->isRunning());
    m_nextCueButton->setEnabled(cueStack->cues().size() > 0);
    m_previousCueButton->setEnabled(cueStack->cues().size() > 0);
}

void SimpleDesk::replaceCurrentCue()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_selectedPlayback < uint(m_playbackSliders.size()));

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    QItemSelectionModel* selectionModel = m_cueStackView->selectionModel();
    if (selectionModel->hasSelection() == true)
    {
        // Replace current cue values
        QModelIndex index = m_cueStackView->currentIndex();
        QString name = cueStack->cues().at(index.row()).name();
        Cue cue = m_engine->cue();
        cue.setName(name);
        cueStack->replaceCue(index.row(), cue);
    }
}

void SimpleDesk::updateSpeedDials()
{
    qDebug() << Q_FUNC_INFO;

    if (m_speedDials == NULL)
        return;

    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    if (selected.size() == 0)
    {
        m_speedDials->setEnabled(false);

        m_speedDials->setWindowTitle(tr("No selection"));
        m_speedDials->setFadeInSpeed(0);
        m_speedDials->setFadeOutSpeed(0);
        m_speedDials->setDuration(0);

        m_speedDials->setOptionalTextTitle(QString());
        m_speedDials->setOptionalText(QString());
    }
    else if (selected.size() == 1)
    {
        m_speedDials->setEnabled(true);

        QModelIndex index = selected.first();
        Q_ASSERT(index.row() >= 0 && index.row() < cueStack->cues().size());
        Cue cue = cueStack->cues()[index.row()];
        m_speedDials->setWindowTitle(cue.name());
        m_speedDials->setFadeInSpeed(cue.fadeInSpeed());
        m_speedDials->setFadeOutSpeed(cue.fadeOutSpeed());
        m_speedDials->setDuration(cue.duration());

        m_speedDials->setOptionalTextTitle(tr("Cue name"));
        m_speedDials->setOptionalText(cue.name());
    }
    else
    {
        m_speedDials->setEnabled(true);

        m_speedDials->setWindowTitle(tr("Multiple Cues"));
        m_speedDials->setFadeInSpeed(0);
        m_speedDials->setFadeOutSpeed(0);
        m_speedDials->setDuration(0);

        m_speedDials->setOptionalTextTitle(QString());
        m_speedDials->setOptionalText(QString());
    }
}

CueStack* SimpleDesk::currentCueStack() const
{
    Q_ASSERT(m_engine != NULL);
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    return cueStack;
}

int SimpleDesk::currentCueIndex() const
{
    Q_ASSERT(m_cueStackView != NULL);
    return m_cueStackView->currentIndex().row();
}

void SimpleDesk::slotCueStackStarted(uint stack)
{
    qDebug() << Q_FUNC_INFO;
    if (stack != m_selectedPlayback)
        return;

    PlaybackSlider* slider = m_playbackSliders[m_selectedPlayback];
    Q_ASSERT(slider != NULL);
    if (slider->value() == 0)
        slider->setValue(UCHAR_MAX);
    updateCueStackButtons();
}

void SimpleDesk::slotCueStackStopped(uint stack)
{
    qDebug() << Q_FUNC_INFO;
    if (stack != m_selectedPlayback)
        return;

    PlaybackSlider* slider = m_playbackSliders[m_selectedPlayback];
    Q_ASSERT(slider != NULL);
    if (slider->value() != 0)
        slider->setValue(0);
    updateCueStackButtons();
}

void SimpleDesk::slotCueStackSelectionChanged()
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());

    updateCueStackButtons();

    // Destroy the existing delete icon
    if (m_cueDeleteIconIndex.isValid() == true)
        m_cueStackView->setIndexWidget(m_cueDeleteIconIndex, NULL);
    m_cueDeleteIconIndex = QModelIndex();

    if (m_editCueStackButton->isChecked() == true)
    {
        CueStack* cueStack = currentCueStack();
        if (selected.size() == 0)
        {
            resetUniverseSliders();
            m_universeGroup->setEnabled(false);
        }
        else if (selected.size() == 1)
        {
            m_universeGroup->setEnabled(true);
            QModelIndex index = selected.first();
            if (index.row() >= 0 && index.row() < cueStack->cues().size())
            {
                Cue cue = cueStack->cues()[index.row()];
                m_engine->setCue(cue);
                slotUniversePageChanged(m_universePageSpin->value());
            }
        }
        else
        {
            m_universeGroup->setEnabled(false);
            resetUniverseSliders();
        }

        // Put a delete button on the first selected item
        if (selected.size() > 0)
        {
            QModelIndex index = selected.first();
            if (index.row() >= 0 && index.row() < cueStack->cues().size())
            {
                QPushButton* btn = new QPushButton(m_cueStackView);
                btn->setToolTip(tr("Delete cue"));
                btn->setFlat(true);
                btn->setFixedSize(m_cueStackView->sizeHintForIndex(index));
                btn->setIcon(QIcon(":/delete.png"));
                m_cueStackView->setIndexWidget(index, btn);
                m_cueDeleteIconIndex = index;
                connect(btn, SIGNAL(clicked()), this, SLOT(slotDeleteCueClicked()));
            }
        }
    }
    else
    {
        m_universeGroup->setEnabled(true);
    }

    updateSpeedDials();
}

void SimpleDesk::slotPreviousCueClicked()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->previousCue();
}

void SimpleDesk::slotNextCueClicked()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->nextCue();
}

void SimpleDesk::slotStopCueStackClicked()
{
    qDebug() << Q_FUNC_INFO;
    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);
    cueStack->stop();
}

void SimpleDesk::slotCloneCueStackClicked()
{
    qDebug() << Q_FUNC_INFO;

    QStringList items;
    for (uint i = 0; i < m_playbacksPerPage; i++)
    {
        if (i != m_selectedPlayback)
            items << QString::number(i + 1);
    }

    bool ok = false;
    QString text = QInputDialog::getItem(this, tr("Clone Cue Stack"), tr("Clone To Playback#"),
                                         items, 0, false, &ok);
    if (ok == false)
        return;

    uint pb = text.toUInt() - 1;
    CueStack* cs = m_engine->cueStack(m_selectedPlayback);
    CueStack* clone = m_engine->cueStack(pb);
    Q_ASSERT(cs != NULL);
    Q_ASSERT(clone != NULL);

    while (clone->cues().size() > 0)
        clone->removeCue(0);

    QListIterator <Cue> it(cs->cues());
    while (it.hasNext() == true)
        clone->appendCue(it.next());

    slotSelectPlayback(pb);
}

void SimpleDesk::slotEditCueStackClicked()
{
    qDebug() << Q_FUNC_INFO;

    slotCueStackSelectionChanged();
    if (m_editCueStackButton->isChecked() == true)
    {
        if (m_speedDials == NULL)
        {
            m_speedDials = new SpeedDialWidget(this);
            m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
            connect(m_speedDials, SIGNAL(fadeInChanged(int)),
                    this, SLOT(slotFadeInDialChanged(int)));
            connect(m_speedDials, SIGNAL(fadeOutChanged(int)),
                    this, SLOT(slotFadeOutDialChanged(int)));
            connect(m_speedDials, SIGNAL(durationChanged(int)),
                    this, SLOT(slotDurationDialChanged(int)));
            connect(m_speedDials, SIGNAL(optionalTextEdited(const QString&)),
                    this, SLOT(slotCueNameEdited(const QString&)));
        }

        m_speedDials->raise();
        m_speedDials->show();
        updateSpeedDials();
    }
    else
    {
        resetUniverseSliders();

        if (m_speedDials != NULL)
            delete m_speedDials;
        m_speedDials = NULL;
    }
}

void SimpleDesk::slotRecordCueClicked()
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_selectedPlayback < uint(m_playbackSliders.size()));

    CueStack* cueStack = m_engine->cueStack(m_selectedPlayback);
    Q_ASSERT(cueStack != NULL);

    QItemSelectionModel* selModel = m_cueStackView->selectionModel();
    Q_ASSERT(selModel != NULL);
    int index = 0;
    if (selModel->hasSelection() == true)
        index = selModel->currentIndex().row() + 1;
    else
        index = cueStack->cues().size();

    Cue cue = m_engine->cue();
    cue.setName(tr("Cue %1").arg(cueStack->cues().size() + 1));
    cueStack->insertCue(index, cue);

    // Select the newly-created Cue, all columns from 0 to last
    const QAbstractItemModel* itemModel = selModel->model();
    Q_ASSERT(itemModel != NULL);
    int firstCol = 0;
    int lastCol = itemModel->columnCount() - 1;
    QItemSelection sel(itemModel->index(index, firstCol), itemModel->index(index, lastCol));
    selModel->select(sel, QItemSelectionModel::ClearAndSelect);
    selModel->setCurrentIndex(itemModel->index(index, firstCol), QItemSelectionModel::Current);

    updateCueStackButtons();
}

void SimpleDesk::slotDeleteCueClicked()
{
    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());
    QModelIndex current = m_cueStackView->selectionModel()->currentIndex();
    CueStack* cueStack = currentCueStack();
    Q_ASSERT(cueStack != NULL);
    QList <int> indexes;
    foreach (QModelIndex index, selected)
        indexes << index.row();
    cueStack->removeCues(indexes);

    // Select an item ~at the current index
    QAbstractItemModel* model = m_cueStackView->model();
    if (model->hasIndex(current.row(), 0) == true)
    {
        m_cueStackView->setCurrentIndex(current);
    }
    else if (model->rowCount() != 0)
    {
        QModelIndex index = model->index(model->rowCount() - 1, 0);
        m_cueStackView->setCurrentIndex(index);
    }
}

void SimpleDesk::slotFadeInDialChanged(int ms)
{
    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());
    CueStack* cueStack = currentCueStack();
    foreach (QModelIndex index, selected)
        cueStack->setFadeInSpeed(ms, index.row());
}

void SimpleDesk::slotFadeOutDialChanged(int ms)
{
    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());
    CueStack* cueStack = currentCueStack();
    foreach (QModelIndex index, selected)
        cueStack->setFadeOutSpeed(ms, index.row());
}

void SimpleDesk::slotDurationDialChanged(int ms)
{
    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());
    CueStack* cueStack = currentCueStack();
    foreach (QModelIndex index, selected)
        cueStack->setDuration(ms, index.row());
}

void SimpleDesk::slotCueNameEdited(const QString& name)
{
    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());
    CueStack* cueStack = currentCueStack();
    if (selected.size() == 1)
        cueStack->setName(name, selected.first().row());
}

void SimpleDesk::showEvent(QShowEvent* ev)
{
    //m_engine->registerSourceAgain();
    if (m_editCueStackButton->isChecked() == true)
        slotEditCueStackClicked();
    initUniversesCombo();
    QWidget::showEvent(ev);
}

void SimpleDesk::hideEvent(QHideEvent* ev)
{
    if (m_speedDials != NULL)
        delete m_speedDials;
    m_speedDials = NULL;
    QWidget::hideEvent(ev);
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool SimpleDesk::loadXML(const QDomElement& root)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_engine != NULL);

    clearContents();

    if (root.tagName() != KXMLQLCSimpleDesk)
    {
        qWarning() << Q_FUNC_INFO << "Simple Desk node not found";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCSimpleDeskEngine)
        {
            m_engine->loadXML(tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized Simple Desk node:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    slotSelectPlayback(0);

    return true;
}

bool SimpleDesk::saveXML(QDomDocument* doc, QDomElement* wksp_root) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);
    Q_ASSERT(m_engine != NULL);

    QDomElement root = doc->createElement(KXMLQLCSimpleDesk);
    wksp_root->appendChild(root);

    return m_engine->saveXML(doc, &root);
}

