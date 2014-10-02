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
#include <QComboBox>
#include <QSplitter>
#include <QToolBar>
#include <QSpinBox>
#include <QAction>
#include <QLabel>
#include <QFont>
#include <QIcon>
#include <QtXml>

#include "monitorfixturepropertieseditor.h"
#include "monitorbackgroundselection.h"
#include "monitorgraphicsview.h"
#include "fixtureselection.h"
#include "monitorfixture.h"
#include "monitorlayout.h"
#include "universe.h"
#include "monitor.h"
#include "apputil.h"
#include "doc.h"

#include "qlcfile.h"

#define SETTINGS_GEOMETRY "monitor/geometry"
#define SETTINGS_VSPLITTER "monitor/vsplitter"

Monitor* Monitor::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Monitor::Monitor(QWidget* parent, Doc* doc, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_doc(doc)
    , m_props(NULL)
    , m_toolBar(NULL)
    , m_scrollArea(NULL)
    , m_monitorWidget(NULL)
    , m_monitorLayout(NULL)
    , m_currentUniverse(Universe::invalid())
    , m_splitter(NULL)
    , m_graphicsView(NULL)
    , m_fixtureItemEditor(NULL)
    , m_gridWSpin(NULL)
    , m_gridHSpin(NULL)
    , m_unitsCombo(NULL)
{
    Q_ASSERT(doc != NULL);

    m_props = m_doc->monitorProperties();

    /* Master layout for toolbar and scroll area */
    new QVBoxLayout(this);

    initView();

    /* Listen to fixture additions and changes from Doc */
    connect(m_doc, SIGNAL(fixtureAdded(quint32)),
            this, SLOT(slotFixtureAdded(quint32)));
    connect(m_doc, SIGNAL(fixtureChanged(quint32)),
            this, SLOT(slotFixtureChanged(quint32)));
    connect(m_doc, SIGNAL(fixtureRemoved(quint32)),
            this, SLOT(slotFixtureRemoved(quint32)));
    connect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)),
            this, SLOT(slotFunctionStarted(quint32)));

    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
            this, SLOT(slotUniversesWritten(int, const QByteArray&)));
}

void Monitor::slotFunctionStarted(quint32 id)
{
    if (m_props->displayMode() == MonitorProperties::Graphics)
    {
        QString bgImage = m_props->customBackground(id);
        if (m_graphicsView != NULL && bgImage.isEmpty() == false)
            m_graphicsView->setBackgroundImage(bgImage);
    }
}

Monitor::~Monitor()
{
    disconnect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
               this, SLOT(slotUniversesWritten(int, const QByteArray&)));

    if (m_props->displayMode() == MonitorProperties::DMX)
    {
        while (m_monitorFixtures.isEmpty() == false)
            delete m_monitorFixtures.takeFirst();
    }

    saveSettings();

    /* Reset the singleton instance */
    Monitor::s_instance = NULL;
}

void Monitor::initView()
{
    if (m_props->displayMode() == MonitorProperties::DMX)
    {
        initDMXToolbar();
        initDMXView();
    }
    else
    {
        initGraphicsToolbar();
        initGraphicsView();
    }
}

void Monitor::initDMXView()
{
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
    m_monitorWidget->setFont(m_props->font());

    /* Create a bunch of MonitorFixtures for each fixture */
    foreach(Fixture* fxi, m_doc->fixtures())
    {
        Q_ASSERT(fxi != NULL);
        if (m_currentUniverse == Universe::invalid() ||
            m_currentUniverse == fxi->universe())
                createMonitorFixture(fxi);
    }

    for (quint32 i = 0; i < m_doc->inputOutputMap()->universes(); i++)
    {
        quint32 uniID = m_doc->inputOutputMap()->getUniverseID(i);
        if (m_currentUniverse == Universe::invalid() || uniID == m_currentUniverse)
            m_doc->inputOutputMap()->setUniverseMonitor(i, true);
        else
            m_doc->inputOutputMap()->setUniverseMonitor(i, false);
    }

    /* Show the master container widgets */
    m_scrollArea->setWidget(m_monitorWidget);
    m_monitorWidget->show();
    m_scrollArea->show();
}

void Monitor::initGraphicsView()
{
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(m_splitter);
    QWidget* gcontainer = new QWidget(this);
    m_splitter->addWidget(gcontainer);
    gcontainer->setLayout(new QVBoxLayout);
    gcontainer->layout()->setContentsMargins(0, 0, 0, 0);

    m_graphicsView = new MonitorGraphicsView(m_doc, this);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setAcceptDrops(true);
    m_graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_graphicsView->setBackgroundBrush(QBrush(QColor(11, 11, 11, 255), Qt::SolidPattern));
    m_splitter->widget(0)->layout()->addWidget(m_graphicsView);

    if (m_props->gridUnits() == MonitorProperties::Meters)
        m_graphicsView->setGridMetrics(1000.0);
    else if (m_props->gridUnits() == MonitorProperties::Feet)
        m_graphicsView->setGridMetrics(304.8);
    m_graphicsView->setGridSize(m_props->gridSize());

    if (m_props->commonBackgroundImage().isEmpty() == false)
        m_graphicsView->setBackgroundImage(m_props->commonBackgroundImage());

    foreach (quint32 fid, m_props->fixtureItemsID())
    {
        if (m_doc->fixture(fid) != NULL)
        {
            m_graphicsView->addFixture(fid, m_props->fixturePosition(fid));
            qDebug() << "Gel color:" << m_props->fixtureGelColor(fid);
            m_graphicsView->setFixtureGelColor(fid, m_props->fixtureGelColor(fid));
            m_graphicsView->setFixtureRotation(fid, m_props->fixtureRotation(fid));
        }
    }

    for (quint32 i = 0; i < m_doc->inputOutputMap()->universes(); i++)
    {
        quint32 uniID = m_doc->inputOutputMap()->getUniverseID(i);
        if (m_currentUniverse == Universe::invalid() || uniID == m_currentUniverse)
            m_doc->inputOutputMap()->setUniverseMonitor(i, true);
        else
            m_doc->inputOutputMap()->setUniverseMonitor(i, false);
    }

    m_graphicsView->showFixturesLabels(m_props->labelsVisible());

    connect(m_graphicsView, SIGNAL(fixtureMoved(quint32,QPointF)),
            this, SLOT(slotFixtureMoved(quint32,QPointF)));
    connect(m_graphicsView, SIGNAL(viewClicked(QMouseEvent*)),
            this, SLOT(slotViewCliked()));

    // add container for chaser editor
    QWidget* econtainer = new QWidget(this);
    m_splitter->addWidget(econtainer);
    econtainer->setLayout(new QVBoxLayout);
    econtainer->layout()->setContentsMargins(0, 0, 0, 0);
    m_splitter->widget(1)->hide();

    QSettings settings;
    QVariant var2 = settings.value(SETTINGS_VSPLITTER);
    if (var2.isValid() == true)
        m_splitter->restoreState(var2.toByteArray());
}

Monitor* Monitor::instance()
{
    return s_instance;
}

void Monitor::saveSettings()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    if (m_splitter != NULL)
    {
        QSettings settings;
        settings.setValue(SETTINGS_VSPLITTER, m_splitter->saveState());
    }

    if (m_monitorWidget != NULL)
        m_props->setFont(m_monitorWidget->font());
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
        else
        {
            window->resize(800, 600);
            window->move(50, 50);
        }
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
 * Menu
 ****************************************************************************/

void Monitor::initDMXToolbar()
{
    QActionGroup* group;
    QAction* action;
    m_toolBar = new QToolBar(this);

    /* Menu bar */
    Q_ASSERT(layout() != NULL);
    layout()->setMenuBar(m_toolBar);

    action = m_toolBar->addAction(tr("2D View"));
    m_toolBar->addSeparator();
    action->setData(MonitorProperties::Graphics);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotSwitchMode()));

    /* Font */
    m_toolBar->addAction(QIcon(":/fonts.png"), tr("Font"),
                       this, SLOT(slotChooseFont()));

    m_toolBar->addSeparator();

    /* Channel style */
    group = new QActionGroup(this);
    group->setExclusive(true);

    action = m_toolBar->addAction(tr("DMX Channels"));
    action->setToolTip(tr("Show absolute DMX channel numbers"));
    action->setCheckable(true);
    action->setData(MonitorProperties::DMXChannels);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelStyleTriggered()));
    m_toolBar->addAction(action);
    group->addAction(action);
    if (m_props->channelStyle() == MonitorProperties::DMXChannels)
        action->setChecked(true);

    action = m_toolBar->addAction(tr("Relative Channels"));
    action->setToolTip(tr("Show channel numbers relative to fixture"));
    action->setCheckable(true);
    action->setData(MonitorProperties::RelativeChannels);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelStyleTriggered()));
    m_toolBar->addAction(action);
    group->addAction(action);
    if (m_props->channelStyle() == MonitorProperties::RelativeChannels)
        action->setChecked(true);

    m_toolBar->addSeparator();

    /* Value display style */
    group = new QActionGroup(this);
    group->setExclusive(true);

    action = m_toolBar->addAction(tr("DMX Values"));
    action->setToolTip(tr("Show DMX values 0-255"));
    action->setCheckable(true);
    action->setData(MonitorProperties::DMXValues);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotValueStyleTriggered()));
    m_toolBar->addAction(action);
    group->addAction(action);
    action->setChecked(true);
    if (m_props->valueStyle() == MonitorProperties::DMXValues)
        action->setChecked(true);

    action = m_toolBar->addAction(tr("Percent Values"));
    action->setToolTip(tr("Show percentage values 0-100%"));
    action->setCheckable(true);
    action->setData(MonitorProperties::PercentageValues);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotValueStyleTriggered()));
    m_toolBar->addAction(action);
    group->addAction(action);
    if (m_props->valueStyle() == MonitorProperties::PercentageValues)
        action->setChecked(true);

    /* Universe combo box */
    m_toolBar->addSeparator();

    QLabel *uniLabel = new QLabel(tr("Universe"));
    uniLabel->setMargin(5);
    m_toolBar->addWidget(uniLabel);

    QComboBox *uniCombo = new QComboBox(this);
    uniCombo->addItem(tr("All universes"), Universe::invalid());
    for (quint32 i = 0; i < m_doc->inputOutputMap()->universes(); i++)
    {
        quint32 uniID = m_doc->inputOutputMap()->getUniverseID(i);
        uniCombo->addItem(m_doc->inputOutputMap()->getUniverseNameByIndex(i), uniID);
    }
    connect(uniCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniverseSelected(int)));
    m_toolBar->addWidget(uniCombo);

}

void Monitor::initGraphicsToolbar()
{
    QAction* action;

    m_toolBar = new QToolBar(this);

    /* Menu bar */
    Q_ASSERT(layout() != NULL);
    layout()->setMenuBar(m_toolBar);

    action = m_toolBar->addAction(tr("DMX View"));
    m_toolBar->addSeparator();
    action->setData(MonitorProperties::DMX);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotSwitchMode()));

    QLabel *label = new QLabel(tr("Size"));
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_toolBar->addWidget(label);

    QSize gridSize = m_props->gridSize();

    m_gridWSpin = new QSpinBox();
    m_gridWSpin->setMinimum(1);
    m_gridWSpin->setValue(gridSize.width());
    m_toolBar->addWidget(m_gridWSpin);
    connect(m_gridWSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotGridWidthChanged(int)));

    QLabel *xlabel = new QLabel("x");
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_toolBar->addWidget(xlabel);
    m_gridHSpin = new QSpinBox();
    m_gridHSpin->setMinimum(1);
    m_gridHSpin->setValue(gridSize.height());
    m_toolBar->addWidget(m_gridHSpin);
    connect(m_gridHSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotGridHeightChanged(int)));

    m_unitsCombo = new QComboBox();
    m_unitsCombo->addItem(tr("Meters"), MonitorProperties::Meters);
    m_unitsCombo->addItem(tr("Feet"), MonitorProperties::Feet);
    if (m_props->gridUnits() == MonitorProperties::Feet)
        m_unitsCombo->setCurrentIndex(1);
    m_toolBar->addWidget(m_unitsCombo);
    connect(m_unitsCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotGridUnitsChanged(int)));

    m_toolBar->addSeparator();

    m_toolBar->addAction(QIcon(":/edit_add.png"), tr("Add fixture"),
                       this, SLOT(slotAddFixture()));
    m_toolBar->addAction(QIcon(":/edit_remove.png"), tr("Remove fixture"),
                       this, SLOT(slotRemoveFixture()));

    m_toolBar->addSeparator();

    m_toolBar->addAction(QIcon(":/image.png"), tr("Set a background picture"),
                       this, SLOT(slotSetBackground()));

    action = m_toolBar->addAction(QIcon(":/label.png"), tr("Show/hide labels"));
    action->setCheckable(true);
    action->setChecked(m_props->labelsVisible());
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotShowLabels(bool)));
}

void Monitor::slotChooseFont()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, m_monitorWidget->font(), this);
    if (ok == true)
    {
        m_monitorWidget->setFont(f);
        m_props->setFont(f);
    }
}

void Monitor::slotChannelStyleTriggered()
{
    QAction* action = qobject_cast<QAction*> (QObject::sender());
    Q_ASSERT(action != NULL);

    action->setChecked(true);
    m_props->setChannelStyle(MonitorProperties::ChannelStyle(action->data().toInt()));
    emit channelStyleChanged(m_props->channelStyle());
}

void Monitor::slotValueStyleTriggered()
{
    QAction* action = qobject_cast<QAction*> (QObject::sender());
    Q_ASSERT(action != NULL);

    action->setChecked(true);
    m_props->setValueStyle(MonitorProperties::ValueStyle(action->data().toInt()));
    emit valueStyleChanged(m_props->valueStyle());
}

void Monitor::slotSwitchMode()
{
    QAction* action = qobject_cast<QAction*> (QObject::sender());
    Q_ASSERT(action != NULL);

    disconnect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
               this, SLOT(slotUniversesWritten(int, const QByteArray&)));

    if (m_props->displayMode() == MonitorProperties::DMX)
    {
        while (m_monitorFixtures.isEmpty() == false)
            delete m_monitorFixtures.takeFirst();
        layout()->removeWidget(m_scrollArea);
        delete m_monitorWidget;
        m_monitorWidget = NULL;
        delete m_scrollArea;
        m_scrollArea = NULL;
        m_toolBar->deleteLater();
    }
    else
    {
        hideFixtureItemEditor();
        m_toolBar->deleteLater();
        m_graphicsView->deleteLater();
        m_graphicsView = NULL;

        if (m_splitter != NULL)
        {
            QSettings settings;
            settings.setValue(SETTINGS_VSPLITTER, m_splitter->saveState());
            m_splitter->deleteLater();
            m_splitter = NULL;
        }
    }
    m_toolBar = NULL;

    m_props->setDisplayMode(MonitorProperties::DisplayMode(action->data().toInt()));

    initView();

    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
            this, SLOT(slotUniversesWritten(int, const QByteArray&)));
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
    mof->slotChannelStyleChanged(m_props->channelStyle());
    mof->slotValueStyleChanged(m_props->valueStyle());
    mof->show();

    /* Make mof listen to value & channel style changes */
    connect(this, SIGNAL(valueStyleChanged(MonitorProperties::ValueStyle)),
            mof, SLOT(slotValueStyleChanged(MonitorProperties::ValueStyle)));
    connect(this, SIGNAL(channelStyleChanged(MonitorProperties::ChannelStyle)),
            mof, SLOT(slotChannelStyleChanged(MonitorProperties::ChannelStyle)));

    m_monitorLayout->addItem(new MonitorLayoutItem(mof));
    m_monitorFixtures.append(mof);
}

void Monitor::slotFixtureAdded(quint32 fxi_id)
{
    if (m_props->displayMode() == MonitorProperties::DMX)
    {
        Fixture* fxi = m_doc->fixture(fxi_id);
        if (fxi != NULL)
            createMonitorFixture(fxi);
    }
}

void Monitor::slotFixtureChanged(quint32 fxi_id)
{
    if (m_props->displayMode() == MonitorProperties::DMX)
    {
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
    else if (m_props->displayMode() == MonitorProperties::Graphics)
    {
        if (m_graphicsView != NULL)
            m_graphicsView->updateFixture(fxi_id);
    }
}

void Monitor::slotFixtureRemoved(quint32 fxi_id)
{
    if (m_props->displayMode() == MonitorProperties::DMX)
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
    else if (m_props->displayMode() == MonitorProperties::Graphics)
    {
        if (m_graphicsView != NULL)
            m_graphicsView->removeFixture(fxi_id);
    }
}

void Monitor::slotUniverseSelected(int index)
{
    QComboBox *combo = (QComboBox *)sender();
    m_currentUniverse = combo->itemData(index).toUInt();

    for (quint32 i = 0; i < m_doc->inputOutputMap()->universes(); i++)
    {
        quint32 uniID = m_doc->inputOutputMap()->getUniverseID(i);
        if (m_currentUniverse == Universe::invalid() || uniID == m_currentUniverse)
            m_doc->inputOutputMap()->setUniverseMonitor(i, true);
        else
            m_doc->inputOutputMap()->setUniverseMonitor(i, false);
    }

    if (m_props->displayMode() == MonitorProperties::DMX)
    {
        while (m_monitorFixtures.isEmpty() == false)
            delete m_monitorFixtures.takeFirst();
        layout()->removeWidget(m_scrollArea);
        delete m_monitorWidget;
        m_monitorWidget = NULL;
        delete m_scrollArea;
        m_scrollArea = NULL;

        initDMXView();
    }
}

void Monitor::slotUniversesWritten(int index, const QByteArray& ua)
{
    if (m_props->displayMode() == MonitorProperties::DMX)
    {
        QListIterator <MonitorFixture*> it(m_monitorFixtures);
        while (it.hasNext() == true)
            it.next()->updateValues(index, ua);
    }
    else if (m_props->displayMode() == MonitorProperties::Graphics)
    {
        if (m_graphicsView != NULL)
            m_graphicsView->writeUniverse(index, ua);
    }
}

/********************************************************************
 * Graphics View
 ********************************************************************/

void Monitor::slotGridWidthChanged(int value)
{
    if (m_graphicsView != NULL)
    {
        m_graphicsView->setGridSize(QSize(value, m_gridHSpin->value()));
        m_props->setGridSize(QSize(value, m_gridHSpin->value()));
    }
}

void Monitor::slotGridHeightChanged(int value)
{
    if (m_graphicsView != NULL)
    {
        m_graphicsView->setGridSize(QSize(m_gridWSpin->value(), value));
        m_props->setGridSize(QSize(m_gridWSpin->value(), value));
    }
}

void Monitor::slotGridUnitsChanged(int index)
{
    if (m_graphicsView != NULL)
    {
        MonitorProperties::GridUnits units = MonitorProperties::Meters;

        QVariant var = m_unitsCombo->itemData(index);
        if (var.isValid())
            units = MonitorProperties::GridUnits(var.toInt());

        if (units == MonitorProperties::Meters)
            m_graphicsView->setGridMetrics(1000.0);
        else if (units == MonitorProperties::Feet)
            m_graphicsView->setGridMetrics(304.8);

        m_props->setGridUnits(units);
    }
}

void Monitor::slotAddFixture()
{
    if (m_graphicsView == NULL)
        return;

    QList <quint32> disabled = m_graphicsView->fixturesID();
    /* Get a list of new fixtures to add to the scene */
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFixtures(disabled);
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
        {
            quint32 fid = it.next();
            m_graphicsView->addFixture(fid);
            m_props->setFixturePosition(fid, QPointF(0, 0));
            m_doc->setModified();
        }
    }
}

void Monitor::slotRemoveFixture()
{
    if (m_graphicsView != NULL)
    {
        hideFixtureItemEditor();
        if (m_graphicsView->removeFixture() == true)
            m_doc->setModified();
    }
}

void Monitor::slotSetBackground()
{
    Q_ASSERT(m_graphicsView != NULL);

    MonitorBackgroundSelection mbgs(this, m_doc);

    if (mbgs.exec() == QDialog::Accepted)
    {
        if (m_props->commonBackgroundImage().isEmpty() == false)
            m_graphicsView->setBackgroundImage(m_props->commonBackgroundImage());
        else
            m_graphicsView->setBackgroundImage(QString());

        m_doc->setModified();
    }
}

void Monitor::slotShowLabels(bool visible)
{
    if (m_graphicsView == NULL)
        return;

    m_props->setLabelsVisible(visible);
    m_graphicsView->showFixturesLabels(visible);
}

void Monitor::slotFixtureMoved(quint32 fid, QPointF pos)
{
    showFixtureItemEditor();
    m_props->setFixturePosition(fid, pos);
    m_doc->setModified();
}

void Monitor::slotViewCliked()
{
    hideFixtureItemEditor();
}

void Monitor::hideFixtureItemEditor()
{
    if (m_fixtureItemEditor != NULL)
    {
        m_splitter->widget(1)->layout()->removeWidget(m_fixtureItemEditor);
        m_splitter->widget(1)->hide();
        m_fixtureItemEditor->deleteLater();
        m_fixtureItemEditor = NULL;
    }
}

void Monitor::showFixtureItemEditor()
{
    MonitorFixtureItem *item = m_graphicsView->getSelectedItem();
    hideFixtureItemEditor();

    if (item != NULL)
    {

        m_fixtureItemEditor = new MonitorFixturePropertiesEditor(
                    item, m_graphicsView,
                    m_props, m_splitter->widget(1));
        m_splitter->widget(1)->layout()->addWidget(m_fixtureItemEditor);
        m_splitter->widget(1)->show();
        m_fixtureItemEditor->show();
    }
}
