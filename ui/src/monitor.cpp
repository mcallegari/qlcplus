/*
  Q Light Controller
  monitor.cpp

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

#include <QApplication>
#include <QActionGroup>
#include <QFontDialog>
#include <QScrollArea>
#include <QSpacerItem>
#include <QByteArray>
#include <QToolBar>
#include <QAction>
#include <QFont>
#include <QIcon>
#include <QtXml>

#include "monitorfixture.h"
#include "monitorlayout.h"
#include "outputmap.h"
#include "monitor.h"
#include "apputil.h"
#include "doc.h"

#include "qlcfile.h"

#define SETTINGS_GEOMETRY "monitor/geometry"
#define SETTINGS_FONT "monitor/font"
#define SETTINGS_VALUESTYLE "monitor/valuestyle"
#define SETTINGS_CHANNELSTYLE "monitor/channelstyle"

Monitor* Monitor::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Monitor::Monitor(QWidget* parent, Doc* doc, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    /* Master layout for toolbar and scroll area */
    new QVBoxLayout(this);

    /* Scroll area that contains the monitor widget */
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    layout()->addWidget(m_scrollArea);

    /* Monitor widget that contains all MonitorFixtures */
    m_monitorWidget = new QWidget(m_scrollArea);
    m_monitorWidget->setBackgroundRole(QPalette::Dark);
    m_monitorLayout = new MonitorLayout(m_monitorWidget);
    m_monitorLayout->setSpacing(1);
    m_monitorLayout->setMargin(1);

    /* Load global settings */
    loadSettings();

    /* Create toolbar */
    initToolBar();

    /* Create a bunch of MonitorFixtures for each fixture */
    foreach(Fixture* fxi, m_doc->fixtures())
    {
        Q_ASSERT(fxi != NULL);
        createMonitorFixture(fxi);
    }

    /* Show the master container widgets */
    m_scrollArea->setWidget(m_monitorWidget);
    m_monitorWidget->show();
    m_scrollArea->show();

    /* Listen to fixture additions and changes from Doc */
    connect(m_doc, SIGNAL(fixtureAdded(quint32)),
            this, SLOT(slotFixtureAdded(quint32)));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)),
            this, SLOT(slotFixtureChanged(quint32)));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));

    connect(m_doc->outputMap(), SIGNAL(universesWritten(const QByteArray&)),
            this, SLOT(slotUniversesWritten(const QByteArray&)));
}

Monitor::~Monitor()
{
    disconnect(m_doc->outputMap(), SIGNAL(universesWritten(const QByteArray&)),
               this, SLOT(slotUniversesWritten(const QByteArray&)));

    while (m_monitorFixtures.isEmpty() == false)
        delete m_monitorFixtures.takeFirst();

    saveSettings();

    /* Reset the singleton instance */
    Monitor::s_instance = NULL;
}

Monitor* Monitor::instance()
{
    return s_instance;
}

void Monitor::loadSettings()
{
    QSettings settings;
    QVariant var;

    // Load font
    var = settings.value(SETTINGS_FONT);
    if (var.isValid() == true)
    {
        QFont fn;
        fn.fromString(var.toString());
        if (fn != QApplication::font())
            m_monitorWidget->setFont(fn);
    }

    // Load channel style
    var = settings.value(SETTINGS_CHANNELSTYLE);
    if (var.isValid() == true)
        m_channelStyle = ChannelStyle(var.toInt());
    else
        m_channelStyle = DMXChannels;

    // Load value style
    var = settings.value(SETTINGS_VALUESTYLE);
    if (var.isValid() == true)
        m_valueStyle = ValueStyle(var.toInt());
    else
        m_valueStyle = DMXValues;
}

void Monitor::saveSettings()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    settings.setValue(SETTINGS_FONT, m_monitorWidget->font().toString());
    settings.setValue(SETTINGS_VALUESTYLE, valueStyle());
    settings.setValue(SETTINGS_CHANNELSTYLE, channelStyle());
}

void Monitor::createAndShow(QWidget* parent, Doc* doc)
{
    QWidget* window = NULL;

    /* Must not create more than one instance */
    if (s_instance == NULL)
    {
        /* Create a separate window for OSX */
        s_instance = new Monitor(parent, doc, Qt::Window);
        window = s_instance;

        /* Set some common properties for the window and show it */
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->setWindowIcon(QIcon(":/monitor.png"));
        window->setWindowTitle(tr("Fixture Monitor"));
        window->setContextMenuPolicy(Qt::CustomContextMenu);

        QSettings settings;
        QVariant var = settings.value(SETTINGS_GEOMETRY);
        if (var.isValid() == true)
            window->restoreGeometry(var.toByteArray());
        AppUtil::ensureWidgetIsVisible(window);
    }
    else
    {
        window = s_instance;
    }

    window->show();
    window->raise();
}

/****************************************************************************
 * Channel & Value styles
 ****************************************************************************/

Monitor::ValueStyle Monitor::valueStyle() const
{
    return m_valueStyle;
}

Monitor::ChannelStyle Monitor::channelStyle() const
{
    return m_channelStyle;
}

/****************************************************************************
 * Menu
 ****************************************************************************/

void Monitor::initToolBar()
{
    QActionGroup* group;
    QAction* action;
    QToolBar* toolBar = new QToolBar(this);

    /* Menu bar */
    Q_ASSERT(layout() != NULL);
    layout()->setMenuBar(toolBar);

    /* Font */
    toolBar->addAction(QIcon(":/fonts.png"), tr("Font"),
                       this, SLOT(slotChooseFont()));

    toolBar->addSeparator();

    /* Channel style */
    group = new QActionGroup(this);
    group->setExclusive(true);

    action = toolBar->addAction(tr("DMX Channels"));
    action->setToolTip(tr("Show absolute DMX channel numbers"));
    action->setCheckable(true);
    action->setData(DMXChannels);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    if (channelStyle() == DMXChannels)
        action->setChecked(true);

    action = toolBar->addAction(tr("Relative Channels"));
    action->setToolTip(tr("Show channel numbers relative to fixture"));
    action->setCheckable(true);
    action->setData(RelativeChannels);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    if (channelStyle() == RelativeChannels)
        action->setChecked(true);

    toolBar->addSeparator();

    /* Value display style */
    group = new QActionGroup(this);
    group->setExclusive(true);

    action = toolBar->addAction(tr("DMX Values"));
    action->setToolTip(tr("Show DMX values 0-255"));
    action->setCheckable(true);
    action->setData(DMXValues);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotValueStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    action->setChecked(true);
    if (valueStyle() == DMXValues)
        action->setChecked(true);

    action = toolBar->addAction(tr("Percent Values"));
    action->setToolTip(tr("Show percentage values 0-100%"));
    action->setCheckable(true);
    action->setData(PercentageValues);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotValueStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    if (valueStyle() == PercentageValues)
        action->setChecked(true);
}

void Monitor::slotChooseFont()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, m_monitorWidget->font(), this);
    if (ok == true)
        m_monitorWidget->setFont(f);
}

void Monitor::slotChannelStyleTriggered()
{
    QAction* action = qobject_cast<QAction*> (QObject::sender());
    Q_ASSERT(action != NULL);

    action->setChecked(true);
    m_channelStyle = ChannelStyle(action->data().toInt());
    emit channelStyleChanged(channelStyle());
}

void Monitor::slotValueStyleTriggered()
{
    QAction* action = qobject_cast<QAction*> (QObject::sender());
    Q_ASSERT(action != NULL);

    action->setChecked(true);
    m_valueStyle = ValueStyle(action->data().toInt());
    emit valueStyleChanged(valueStyle());
}

/****************************************************************************
 * Fixture added/removed stuff
 ****************************************************************************/

void Monitor::updateFixtureLabelStyles()
{
    QListIterator <MonitorFixture*> it(m_monitorFixtures);
    while (it.hasNext() == true)
        it.next()->updateLabelStyles();
}

void Monitor::createMonitorFixture(Fixture* fxi)
{
    MonitorFixture* mof = new MonitorFixture(m_monitorWidget, m_doc);
    mof->setFixture(fxi->id());
    mof->slotChannelStyleChanged(channelStyle());
    mof->slotValueStyleChanged(valueStyle());
    mof->show();

    /* Make mof listen to value & channel style changes */
    connect(this, SIGNAL(valueStyleChanged(Monitor::ValueStyle)),
            mof, SLOT(slotValueStyleChanged(Monitor::ValueStyle)));
    connect(this, SIGNAL(channelStyleChanged(Monitor::ChannelStyle)),
            mof, SLOT(slotChannelStyleChanged(Monitor::ChannelStyle)));

    m_monitorLayout->addItem(new MonitorLayoutItem(mof));
    m_monitorFixtures.append(mof);
}

void Monitor::slotFixtureAdded(quint32 fxi_id)
{
    Fixture* fxi = m_doc->fixture(fxi_id);
    if (fxi != NULL)
        createMonitorFixture(fxi);
}

void Monitor::slotFixtureChanged(quint32 fxi_id)
{
    Q_UNUSED(fxi_id);

    QListIterator <MonitorFixture*> it(m_monitorFixtures);
    while (it.hasNext() == true)
    {
        MonitorFixture* mof = it.next();
        if (mof->fixture() == fxi_id)
            mof->setFixture(fxi_id);
    }

    m_monitorLayout->sort();
    m_monitorWidget->updateGeometry();
}

void Monitor::slotFixtureRemoved(quint32 fxi_id)
{
    QMutableListIterator <MonitorFixture*> it(m_monitorFixtures);
    while (it.hasNext() == true)
    {
        MonitorFixture* mof = it.next();
        if (mof->fixture() == fxi_id)
        {
            it.remove();
            delete mof;
        }
    }
}

void Monitor::slotUniversesWritten(const QByteArray& ua)
{
    QListIterator <MonitorFixture*> it(m_monitorFixtures);
    while (it.hasNext() == true)
        it.next()->updateValues(ua);
}
