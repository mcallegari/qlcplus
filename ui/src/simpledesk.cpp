/*
  Q Light Controller
  simpledesk.cpp

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

#include <QDomDocument>
#include <QInputDialog>
#include <QDomElement>
#include <QToolButton>
#include <QHeaderView>
#include <QPushButton>
#include <QScrollArea>
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
#include "fixtureconsole.h"
#include "playbackslider.h"
#include "consolechannel.h"
#include "cuestackmodel.h"
#include "groupsconsole.h"
#include "simpledesk.h"
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
#define DEFAULT_PAGE_CHANNELS   32
#define DEFAULT_PAGE_PLAYBACKS  15

SimpleDesk* SimpleDesk::s_instance = NULL;

QString ssEven =  "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #C3D1C9, stop: 1 #AFBBB4); "
                 " border: 1px solid gray; border-radius: 4px; }";
QString ssOdd = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D5E0, stop: 1 #A7A6AF); "
                 " border: 1px solid gray; border-radius: 4px; }";
QString ssNone = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D6D2D0, stop: 1 #AFACAB); "
                 " border: 1px solid gray; border-radius: 4px; }";
QString ssOverride = "QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FF2D2D, stop: 1 #FF5050); "
                     " border: 1px solid gray; border-radius: 4px; }";

/*****************************************************************************
 * Initialization
 *****************************************************************************/

SimpleDesk::SimpleDesk(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_engine(new SimpleDeskEngine(doc))
    , m_doc(doc)
    , m_docChanged(false)
    , m_chGroupsArea(NULL)
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
    for (quint32 i = 0; i < m_doc->inputOutputMap()->universes(); i++)
        m_universesPage.append(1);

    initEngine();
    initView();
    initUniverseSliders();
    initUniversePager();
    initPlaybackSliders();
    initCueStack();

    slotSelectPlayback(0);

    connect(m_doc, SIGNAL(fixtureAdded(quint32)),
            this, SLOT(slotDocChanged()));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotDocChanged()));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)),
            this, SLOT(slotDocChanged()));
    connect(m_doc, SIGNAL(channelsGroupAdded(quint32)),
            this, SLOT(slotDocChanged()));
    connect(m_doc, SIGNAL(channelsGroupRemoved(quint32)),
            this, SLOT(slotDocChanged()));

    connect(m_doc->inputOutputMap(), SIGNAL(universeAdded(quint32)),
            this, SLOT(slotDocChanged()));
    connect(m_doc->inputOutputMap(), SIGNAL(universeRemoved(quint32)),
            this, SLOT(slotDocChanged()));

    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
            this, SLOT(slotUniversesWritten(int, const QByteArray&)));
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

    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    m_splitter = new QSplitter(this);
    layout()->addWidget(m_splitter);

    initTopSide();
    initBottomSide();

    QSettings settings;
    m_splitter->restoreState(settings.value(SETTINGS_SPLITTER).toByteArray());
    m_splitter->setOrientation(Qt::Vertical);
}

void SimpleDesk::initTopSide()
{
    QWidget* topSide = new QWidget(this);
    QVBoxLayout* lay = new QVBoxLayout(topSide);
    lay->setContentsMargins(1, 1, 1, 1);
    m_splitter->addWidget(topSide);

    QHBoxLayout* uniLay = new QHBoxLayout;
    uniLay->setContentsMargins(1, 1, 1, 1);

    m_viewModeButton = new QToolButton(this);
    m_viewModeButton->setIcon(QIcon(":/tabview.png"));
    m_viewModeButton->setIconSize(QSize(24, 24));
    m_viewModeButton->setMinimumSize(QSize(36, 36));
    m_viewModeButton->setMaximumSize(QSize(36, 36));
    m_viewModeButton->setToolTip(tr("View mode"));
    m_viewModeButton->setCheckable(true);
    uniLay->addWidget(m_viewModeButton);

    m_universePageDownButton = new QToolButton(this);
    m_universePageDownButton->setIcon(QIcon(":/back.png"));
    m_universePageDownButton->setIconSize(QSize(24, 24));
    m_universePageDownButton->setMinimumSize(QSize(36, 36));
    m_universePageDownButton->setMaximumSize(QSize(36, 36));
    m_universePageDownButton->setToolTip(tr("Previous page"));
    uniLay->addWidget(m_universePageDownButton);

    m_universePageSpin = new QSpinBox(this);
    m_universePageSpin->setMaximumSize(QSize(40, 34));
    m_universePageSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_universePageSpin->setAlignment(Qt::AlignCenter);
    m_universePageSpin->setWrapping(true);
    m_universePageSpin->setToolTip(tr("Current page"));
    uniLay->addWidget(m_universePageSpin);

    m_universePageUpButton = new QToolButton(this);
    m_universePageUpButton->setIcon(QIcon(":/forward.png"));
    m_universePageUpButton->setIconSize(QSize(24, 24));
    m_universePageUpButton->setMinimumSize(QSize(36, 36));
    m_universePageUpButton->setMaximumSize(QSize(36, 36));
    m_universePageUpButton->setToolTip(tr("Next page"));
    uniLay->addWidget(m_universePageUpButton);

    m_universeResetButton = new QToolButton(this);
    m_universeResetButton->setIcon(QIcon(":/fileclose.png"));
    m_universeResetButton->setIconSize(QSize(24, 24));
    m_universeResetButton->setMinimumSize(QSize(36, 36));
    m_universeResetButton->setMaximumSize(QSize(36, 36));
    m_universeResetButton->setToolTip(tr("Reset universe"));
    uniLay->addWidget(m_universeResetButton);

    uniLay->addSpacing(50);

    QLabel *label = new QLabel(tr("Universe"));
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    uniLay->addWidget(label);

    m_universesCombo = new QComboBox(this);
    //m_universesCombo->setFixedWidth(200);
    m_universesCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    uniLay->addWidget(m_universesCombo);
    lay->addLayout(uniLay);

    initUniversesCombo();
    connect(m_universesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniversesComboChanged(int)));

    m_universeGroup = new QGroupBox(this);
    //m_universeGroup->setTitle(tr("Universe"));
    QHBoxLayout* grpLay = new QHBoxLayout(m_universeGroup);
    grpLay->setContentsMargins(1, 1, 1, 1);
    grpLay->setSpacing(1);
    lay->addWidget(m_universeGroup);

    QVBoxLayout* vbox = new QVBoxLayout;
    m_grandMasterSlider = new GrandMasterSlider(this, m_doc->inputOutputMap());
    vbox->addWidget(m_grandMasterSlider);

    grpLay->addLayout(vbox);
}

void SimpleDesk::initBottomSide()
{
    m_tabs = new QTabWidget(this);
    m_splitter->addWidget(m_tabs);

    QWidget* cueStackWidget = new QWidget(this);
    QHBoxLayout* lay = new QHBoxLayout(cueStackWidget);
    lay->setContentsMargins(1, 1, 1, 1);
    m_tabs->addTab(cueStackWidget, tr("Cue Stack"));

    m_playbackGroup = new QGroupBox(this);
    m_playbackGroup->setTitle(tr("Playback"));
    QHBoxLayout *grpLay = new QHBoxLayout(m_playbackGroup);
    grpLay->setContentsMargins(0, 6, 0, 0);
    grpLay->setSpacing(1);
    lay->addWidget(m_playbackGroup);

    m_cueStackGroup = new QGroupBox(this);
    m_cueStackGroup->setTitle(tr("Cue Stack"));
    QVBoxLayout *grpLay2 = new QVBoxLayout(m_cueStackGroup);
    grpLay2->setContentsMargins(0, 6, 0, 0);
    lay->addWidget(m_cueStackGroup);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    m_previousCueButton = new QToolButton(this);
    m_previousCueButton->setIcon(QIcon(":/back.png"));
    m_previousCueButton->setIconSize(QSize(32, 32));
    m_previousCueButton->setToolTip(tr("Previous cue"));
    hbox->addWidget(m_previousCueButton);

    m_stopCueStackButton = new QToolButton(this);
    m_stopCueStackButton->setIcon(QIcon(":/player_stop.png"));
    m_stopCueStackButton->setIconSize(QSize(32, 32));
    m_stopCueStackButton->setToolTip(tr("Stop cue stack"));
    hbox->addWidget(m_stopCueStackButton);

    m_nextCueButton = new QToolButton(this);
    m_nextCueButton->setIcon(QIcon(":/forward.png"));
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

    grpLay2->addLayout(hbox);

    m_cueStackView = new QTreeView(this);
    m_cueStackView->setAllColumnsShowFocus(true);
    m_cueStackView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_cueStackView->setDragEnabled(true);
    m_cueStackView->setDragDropMode(QAbstractItemView::InternalMove);
    m_cueStackGroup->layout()->addWidget(m_cueStackView);

    initChannelGroupsView();
}

void SimpleDesk::slotDocChanged()
{
    m_docChanged = true;
}

int SimpleDesk::getSlidersNumber()
{
    return m_channelsPerPage;
}

int SimpleDesk::getCurrentUniverseIndex()
{
    return m_currentUniverse;
}

int SimpleDesk::getCurrentPage()
{
    return m_universePageSpin->value();
}

uchar SimpleDesk::getAbsoluteChannelValue(uint address)
{
    if (m_engine->hasChannel(address))
        return m_engine->value(address);
    else
        return 0;
}

void SimpleDesk::setAbsoluteChannelValue(uint address, uchar value)
{
    m_engine->setValue(address, value);
}

void SimpleDesk::resetUniverse()
{
    // force a engine reset
    m_engine->resetUniverse(m_currentUniverse);
    // simulate a user click on the reset button
    // to avoid messing up with multithread calls
    m_universeResetButton->click();
}

/****************************************************************************
 * Universe controls
 ****************************************************************************/

void SimpleDesk::initUniversesCombo()
{
    disconnect(m_universesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniversesComboChanged(int)));
    int currIdx = m_universesCombo->currentIndex();
    m_universesCombo->clear();
    m_universesCombo->addItems(m_doc->inputOutputMap()->universeNames());
    if (currIdx != -1)
        m_universesCombo->setCurrentIndex(currIdx);
    if (m_universesPage.length() < m_universesCombo->count())
    {
        for (int i = 0; i < m_universesCombo->count() - m_universesPage.length(); i++)
            m_universesPage.append(1);
    }
    connect(m_universesCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniversesComboChanged(int)));
}

void SimpleDesk::initUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    quint32 start = m_universesPage.at(m_currentUniverse) * m_channelsPerPage;
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
}

void SimpleDesk::initUniversePager()
{
    qDebug() << Q_FUNC_INFO;
    m_universePageSpin->setRange(1, int(512 / m_channelsPerPage));
    m_universePageSpin->setValue(1);
    slotUniversePageChanged(1);

    connect(m_viewModeButton, SIGNAL(clicked(bool)), this, SLOT(slotViewModeClicked(bool)));
    connect(m_universePageUpButton, SIGNAL(clicked()), this, SLOT(slotUniversePageUpClicked()));
    connect(m_universePageDownButton, SIGNAL(clicked()), this, SLOT(slotUniversePageDownClicked()));
    connect(m_universePageSpin, SIGNAL(valueChanged(int)), this, SLOT(slotUniversePageChanged(int)));
    connect(m_universeResetButton, SIGNAL(clicked()), this, SLOT(slotUniverseResetClicked()));
}

void SimpleDesk::resetUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    foreach (ConsoleChannel *channel, m_universeSliders)
    {
        if (channel != NULL)
            channel->setValue(0);
    }
}

void SimpleDesk::initSliderView(bool fullMode)
{
    m_consoleList.clear();

    if (fullMode == true)
    {
        scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);

        QGroupBox* grpBox = new QGroupBox(scrollArea);
        grpBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        QHBoxLayout* fixturesLayout = new QHBoxLayout(grpBox);
        grpBox->setLayout(fixturesLayout);
        fixturesLayout->setSpacing(2);
        fixturesLayout->setContentsMargins(2, 2, 2, 2);

        int c = 0;
        foreach(Fixture *fixture, m_doc->fixtures())
        {
            if (fixture->universe() != (quint32)m_universesCombo->currentIndex())
                continue;
            FixtureConsole* console = NULL;
            if (c%2 == 0)
                console = new FixtureConsole(scrollArea, m_doc, FixtureConsole::GroupOdd, false);
            else
                console = new FixtureConsole(scrollArea, m_doc, FixtureConsole::GroupEven, false);
            console->setFixture(fixture->id());
            quint32 absoluteAddr = fixture->universeAddress();
            for (quint32 i = 0; i < fixture->channels(); i++)
            {
                if (m_engine->hasChannel(absoluteAddr + i))
                {
                    SceneValue scv(fixture->id(), i, m_engine->value(absoluteAddr + i));
                    console->setSceneValue(scv);
                    console->setChannelStylesheet(i, ssOverride);
                }
            }
            fixturesLayout->addWidget(console);
            connect(console, SIGNAL(valueChanged(quint32,quint32,uchar)),
                    this, SLOT(slotUniverseSliderValueChanged(quint32,quint32,uchar)));
            c++;
            m_consoleList[fixture->id()] = console;
        }
        fixturesLayout->addStretch(1);
        scrollArea->setWidget(grpBox);

        m_universeGroup->layout()->addWidget(scrollArea);
    }
    else
    {
        int page = m_universesPage.at(m_universesCombo->currentIndex());
        slotUniversePageChanged(page);
    }
}

void SimpleDesk::initChannelGroupsView()
{
    if (m_chGroupsArea != NULL)
    {
        delete m_chGroupsArea;
        m_chGroupsArea = NULL;
    }

    if (m_doc->channelsGroups().count() > 0)
    {
        m_chGroupsArea = new QScrollArea();
        QList<quint32> chGrpIDs;
        foreach(ChannelsGroup *grp, m_doc->channelsGroups())
            chGrpIDs.append(grp->id());
        GroupsConsole* console = new GroupsConsole(m_chGroupsArea, m_doc, chGrpIDs, QList<uchar>());
        m_chGroupsArea->setWidget(console);
        m_chGroupsArea->setWidgetResizable(true);
        m_tabs->addTab(m_chGroupsArea, tr("Channel groups"));
        connect(console, SIGNAL(groupValueChanged(quint32,uchar)),
                this, SLOT(slotGroupValueChanged(quint32,uchar)));
    }
}

void SimpleDesk::slotUniversesComboChanged(int index)
{
    m_currentUniverse = index;
    if (m_viewModeButton->isChecked() == true)
    {
        m_universeGroup->layout()->removeWidget(scrollArea);
        delete scrollArea;
        initSliderView(true);
    }
    else
    {
        int page = m_universesPage.at(index);
        slotUniversePageChanged(m_universesPage.at(index));
        m_universePageSpin->setValue(page);
    }
}

void SimpleDesk::slotViewModeClicked(bool toggle)
{
    if (toggle == true)
    {
        for (quint32 i = 0; i < m_channelsPerPage; i++)
        {
            ConsoleChannel* slider = m_universeSliders[i];
            if (slider != NULL)
            {
                m_universeGroup->layout()->removeWidget(slider);
                disconnect(slider, SIGNAL(valueChanged(quint32,quint32,uchar)),
                       this, SLOT(slotUniverseSliderValueChanged(quint32,quint32,uchar)));
                delete slider;
                m_universeSliders[i] = NULL;
            }
        }
        initSliderView(true);
    }
    else
    {
        m_universeGroup->layout()->removeWidget(scrollArea);
        delete scrollArea;
        initSliderView(false);
    }
    m_universePageUpButton->setEnabled(!toggle);
    m_universePageDownButton->setEnabled(!toggle);
    m_universePageSpin->setEnabled(!toggle);
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
            m_universeSliders[i] = NULL;
        }
        const Fixture* fx = m_doc->fixture(m_doc->fixtureForAddress(absoluteAddr + i));
        if (fx == NULL)
        {
            slider = new ConsoleChannel(this, m_doc, Fixture::invalidId(), start + i, false);
            if (m_engine->hasChannel((m_currentUniverse << 9) + (start + i)))
                slider->setStyleSheet(ssOverride);
            else
                slider->setStyleSheet(ssNone);
        }
        else
        {
            uint ch = (absoluteAddr + i) - fx->universeAddress();
            slider = new ConsoleChannel(this, m_doc, fx->id(), ch, false);
            if (m_engine->hasChannel(absoluteAddr + i))
            {
                slider->setStyleSheet(ssOverride);
            }
            else
            {
                if (fx->id() % 2 == 0)
                    slider->setStyleSheet(ssOdd);
                else
                    slider->setStyleSheet(ssEven);
            }
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
    m_engine->resetUniverse(m_currentUniverse);
    m_universePageSpin->setValue(1);
    if (m_viewModeButton->isChecked() == false)
        slotUniversePageChanged(1);
    else
    {
        QHashIterator <quint32,FixtureConsole*> it(m_consoleList);

        while (it.hasNext() == true)
        {
            it.next();
            FixtureConsole *fc = it.value();
            Q_ASSERT(fc != NULL);
            fc->resetChannelsStylesheet();
        }
    }
}

void SimpleDesk::slotUniverseSliderValueChanged(quint32 fid, quint32 chan, uchar value)
{
    QVariant var(sender()->property(PROP_ADDRESS));
    if (var.isValid() == true) // Not true with disabled sliders
    {
        quint32 chanAbsAddr = var.toUInt();
        if (m_viewModeButton->isChecked() == false &&
            m_engine->hasChannel(chanAbsAddr) == false)
        {
            quint32 chanAddr = (chanAbsAddr & 0x01FF) - ((m_universePageSpin->value() - 1) * m_channelsPerPage);
            if (chanAddr < (quint32)m_universeSliders.count())
            {
                ConsoleChannel *chan = m_universeSliders.at(chanAddr);
                chan->setStyleSheet(ssOverride);
            }
        }
        m_engine->setValue(chanAbsAddr, value);

        if (m_editCueStackButton->isChecked() == true)
            replaceCurrentCue();
    }
    else // calculate the absolute address from the given parameters
    {
        Fixture *fixture = m_doc->fixture(fid);
        if (fixture != NULL)
        {
            quint32 chanAbsAddr = fixture->universeAddress() + chan;
            if (m_viewModeButton->isChecked() == true &&
                m_engine->hasChannel(chanAbsAddr) == false)
            {
                FixtureConsole *fc = m_consoleList[fid];
                if (fc != NULL)
                    fc->setChannelStylesheet(chan, ssOverride);
            }
            m_engine->setValue(chanAbsAddr, value);

            if (m_editCueStackButton->isChecked() == true)
                replaceCurrentCue();
        }
    }
}

void SimpleDesk::slotUniversesWritten(int idx, const QByteArray& ua)
{
    // If Simple Desk is not visible, don't even waste CPU
    if (isVisible() == false)
        return;

    if (idx != m_currentUniverse)
        return;

    if (m_viewModeButton->isChecked() == false)
    {
        quint32 start = (m_universePageSpin->value() - 1) * m_channelsPerPage;

        // update current page sliders
        for (quint32 i = start; i < start + (quint32)m_channelsPerPage; i++)
        {
            if (i >= (quint32)ua.length())
                break;

            quint32 absAddr = i + (idx << 9);
            ConsoleChannel *cc = m_universeSliders[i - start];
            if (cc == NULL)
                continue;

            if (m_engine->hasChannel(absAddr) == true)
            {
                if (cc->value() != m_engine->value(absAddr))
                {
                    cc->blockSignals(true);
                    cc->setValue(m_engine->value(absAddr), false);
                    cc->setStyleSheet(ssOverride);
                    cc->blockSignals(false);
                }
                continue;
            }

            cc->blockSignals(true);
            cc->setValue(ua.at(i), false);
            cc->blockSignals(false);
        }
    }
    else
    {
        foreach(FixtureConsole *fc, m_consoleList.values())
        {
            quint32 fxi = fc->fixture();
            Fixture *fixture = m_doc->fixture(fxi);
            if (fixture != NULL)
            {
                quint32 startAddr = fixture->address();
                for (quint32 c = 0; c < fixture->channels(); c++)
                {
                    if (startAddr + c >= (quint32)ua.length())
                        break;

                    if (m_engine->hasChannel((startAddr + c) + (idx << 9)) == true)
                        continue;

                    fc->blockSignals(true);
                    fc->setValue(c, ua.at(startAddr + c), false);
                    fc->blockSignals(false);
                }
            }
        }
    }
}

void SimpleDesk::slotUpdateUniverseSliders()
{
    qDebug() << Q_FUNC_INFO;
    if (m_viewModeButton->isChecked() == true)
    {
        m_universeGroup->layout()->removeWidget(scrollArea);
        delete scrollArea;
        initSliderView(true);
    }
    else
    {
        slotUniversePageChanged(m_universePageSpin->value());
    }
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

void SimpleDesk::slotGroupValueChanged(quint32 groupID, uchar value)
{
    ChannelsGroup *group = m_doc->channelsGroup(groupID);
    if (group == NULL)
        return;
    quint32 start = (m_universePageSpin->value() - 1) * m_channelsPerPage;

    foreach (SceneValue scv, group->getChannels())
    {
        Fixture *fixture = m_doc->fixture(scv.fxi);
        if (fixture == NULL)
            continue;
        quint32 absAddr = fixture->universeAddress() + scv.channel;
        m_engine->setValue(absAddr, value);

        // Update sliders on screen
        if (m_viewModeButton->isChecked() == false)
        {
            if (fixture->universe() == (quint32)m_currentUniverse &&
                absAddr >= start &&
                absAddr < start + m_channelsPerPage)
            {
                ConsoleChannel *cc = m_universeSliders[absAddr - start];
                if (cc != NULL)
                {
                    cc->blockSignals(true);
                    cc->setValue(value, false);
                    cc->blockSignals(false);
                }
            }
        }
        else
        {
            FixtureConsole *fc = m_consoleList[fixture->id()];
            if(fc != NULL)
            {
                fc->blockSignals(true);
                fc->setValue(scv.channel, value, false);
                fc->blockSignals(false);
            }
        }
    }
}

/****************************************************************************
 * Cue Stack controls
 ****************************************************************************/

void SimpleDesk::initCueStack()
{
    qDebug() << Q_FUNC_INFO;
    CueStackModel* model = new CueStackModel(this);
    m_cueStackView->setModel(model);

    connect(m_previousCueButton, SIGNAL(clicked()), this, SLOT(slotPreviousCueClicked()));
    connect(m_nextCueButton, SIGNAL(clicked()), this, SLOT(slotNextCueClicked()));
    connect(m_stopCueStackButton, SIGNAL(clicked()), this, SLOT(slotStopCueStackClicked()));
    connect(m_cloneCueStackButton, SIGNAL(clicked()), this, SLOT(slotCloneCueStackClicked()));
    connect(m_editCueStackButton, SIGNAL(toggled(bool)), this, SLOT(slotEditCueStackClicked(bool)));
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

void SimpleDesk::createSpeedDials()
{
    if (m_speedDials != NULL)
        return;

    m_speedDials = new SpeedDialWidget(this);
    m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_speedDials, SIGNAL(fadeInChanged(int)),
            this, SLOT(slotFadeInDialChanged(int)));
    connect(m_speedDials, SIGNAL(fadeOutChanged(int)),
            this, SLOT(slotFadeOutDialChanged(int)));
    connect(m_speedDials, SIGNAL(holdChanged(int)),
            this, SLOT(slotHoldDialChanged(int)));
    connect(m_speedDials, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotDialDestroyed(QObject*)));
    connect(m_speedDials, SIGNAL(optionalTextEdited(const QString&)),
            this, SLOT(slotCueNameEdited(const QString&)));
    m_speedDials->raise();
    m_speedDials->show();
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
        if ((int)cue.duration() < 0)
            m_speedDials->setDuration(cue.duration());
        else
            m_speedDials->setDuration(cue.duration() - cue.fadeInSpeed() - cue.fadeOutSpeed());

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

void SimpleDesk::slotDialDestroyed(QObject *)
{
    if (m_speedDials != NULL)
        m_speedDials->deleteLater();
    m_speedDials = NULL;
    m_editCueStackButton->setChecked(false);
}

void SimpleDesk::slotEditCueStackClicked(bool state)
{
    qDebug() << Q_FUNC_INFO;

    slotCueStackSelectionChanged();

    if (state == true)
    {
        createSpeedDials();
        updateSpeedDials();
    }
    else
    {
        resetUniverseSliders();
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
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

void SimpleDesk::slotHoldDialChanged(int ms)
{
    Q_ASSERT(m_cueStackView != NULL);
    Q_ASSERT(m_cueStackView->selectionModel() != NULL);
    QModelIndexList selected(m_cueStackView->selectionModel()->selectedRows());
    CueStack* cueStack = currentCueStack();
    foreach (QModelIndex index, selected)
    {
        if (ms < 0)
            cueStack->setDuration(ms, index.row());
        else
            cueStack->setDuration(cueStack->fadeInSpeed() + ms + cueStack->fadeOutSpeed(), index.row());
    }
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
    if (m_docChanged == true)
    {
        if (m_editCueStackButton->isChecked() == true)
            slotEditCueStackClicked(true);
        initUniversesCombo();
        slotUpdateUniverseSliders();
        initChannelGroupsView();
        m_docChanged = false;
    }
    QWidget::showEvent(ev);
}

void SimpleDesk::hideEvent(QHideEvent* ev)
{
    if (m_speedDials != NULL)
        m_speedDials->deleteLater();
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

