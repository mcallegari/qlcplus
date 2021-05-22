/*
  Q Light Controller
  sceneeditor.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QTreeWidgetItem>
#include <QColorDialog>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QComboBox>
#include <QSettings>
#include <QLineEdit>
#include <QToolBar>
#include <QLayout>
#include <qmath.h>
#include <QLabel>
#include <QDebug>
#include <QSettings>
#include <QCheckBox>

#include "inputselectionwidget.h"
#include "genericdmxsource.h"
#include "fixtureselection.h"
#include "speeddialwidget.h"
#include "functionmanager.h"
#include "fixtureconsole.h"
#include "groupsconsole.h"
#include "qlcfixturedef.h"
#include "channelsgroup.h"
#include "qlcclipboard.h"
#include "positiontool.h"
#include "sceneeditor.h"
#include "mastertimer.h"
#include "qlcchannel.h"
#include "chaserstep.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#define KColumnName         0
#define KColumnManufacturer 1
#define KColumnModel        2
#define KColumnID           3

#define KTabGeneral         0

#define SETTINGS_CHASER "sceneeditor/chaser"

#define UI_STATE_TAB_INDEX "tabIndex"
#define UI_STATE_TAB_MODE  "tabMode"
#define UI_STATE_SHOW_DIAL "showDial"
#define UI_STATE_TABBED_FIXTURES  0
#define UI_STATE_ALL_FIXTURES     1

SceneEditor::SceneEditor(QWidget* parent, Scene* scene, Doc* doc, bool applyValues)
    : QWidget(parent)
    , m_doc(doc)
    , m_scene(scene)
    , m_source(NULL)
    , m_initFinished(false)
    , m_speedDials(NULL)
    , m_channelGroupsTab(-1)
    , m_currentTab(KTabGeneral)
    , m_fixtureFirstTabIndex(1)
    , m_copyFromSelection(false)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(scene != NULL);

    setupUi(this);

    init(applyValues);

    // start hw_edit
    QSettings settings;
    // read settings
    hw_enable = settings.value("sceneEdit/hardwareEnable",false).toBool();
    channelBanks.append(settings.value("sceneEdit/channelBank1",QLCInputSource::invalidChannel).toUInt());
    channelBanks.append(settings.value("sceneEdit/channelBank2",QLCInputSource::invalidChannel).toUInt());
    channelBanks.append(settings.value("sceneEdit/channelBank3",QLCInputSource::invalidChannel).toUInt());
    channelBanks.append(settings.value("sceneEdit/channelBank4",QLCInputSource::invalidChannel).toUInt());
    channelBanks.append(settings.value("sceneEdit/channelBank5",QLCInputSource::invalidChannel).toUInt());
    channelBanks.append(settings.value("sceneEdit/channelBank6",QLCInputSource::invalidChannel).toUInt());
    channelBanks.append(settings.value("sceneEdit/channelBank7",QLCInputSource::invalidChannel).toUInt());
    channelBanks.append(settings.value("sceneEdit/channelBank8",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton1",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton2",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton3",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton4",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton5",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton6",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton7",QLCInputSource::invalidChannel).toUInt());
    armButton.append(settings.value("sceneEdit/armButton8",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader1",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader2",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader3",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader4",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader5",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader6",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader7",QLCInputSource::invalidChannel).toUInt());
    faders.append(settings.value("sceneEdit/fader8",QLCInputSource::invalidChannel).toUInt());
    tabs.append(settings.value("sceneEdit/tabPrev",QLCInputSource::invalidChannel).toUInt());
    tabs.append(settings.value("sceneEdit/tabNext",QLCInputSource::invalidChannel).toUInt());
    if(channelBanks.count() != 8 || armButton.count() != 8 || faders.count() != 8 || tabs.count() != 2)
      hw_enable = false;
    m_offset = 0;
    // connect hw to virtual faders
    if(hw_enable)
      connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32, quint32, uchar, const QString&)),
              this, SLOT(onInput(quint32, quint32, uchar, const QString&)));
    // end hw_edit

    // Start new (==empty) scenes from the first tab and ones with something in them
    // on the first fixture page.
    if (m_tab->count() == 0)
        slotTabChanged(KTabGeneral);
    else
    {
        QVariant tabIndex = scene->uiStateValue(UI_STATE_TAB_INDEX);
        if (tabIndex.isValid())
            m_tab->setCurrentIndex(tabIndex.toInt());
        else
            m_tab->setCurrentIndex(0);
    }

    QVariant showDial = scene->uiStateValue(UI_STATE_SHOW_DIAL);
    if (showDial.isNull() == false && showDial.toBool() == true)
        m_speedDialAction->setChecked(true);

    connect(m_doc, SIGNAL(fixtureRemoved(quint32)), this, SLOT(slotFixtureRemoved(quint32)));

    m_initFinished = true;

    // Set focus to the editor
    m_nameEdit->setFocus();
}

SceneEditor::~SceneEditor()
{
    qDebug() << Q_FUNC_INFO;

    delete m_source;

    QSettings settings;
    quint32 id = m_chaserCombo->itemData(m_chaserCombo->currentIndex()).toUInt();
    settings.setValue(SETTINGS_CHASER, id);
}

void SceneEditor::slotFunctionManagerActive(bool active)
{
    qDebug() << Q_FUNC_INFO;

    if (active == true)
    {
        if (m_speedDialAction->isChecked() && m_speedDials == NULL)
            createSpeedDials();
    }
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void SceneEditor::slotSetSceneValues(QList <SceneValue>&sceneValues)
{
    QListIterator <SceneValue> it(sceneValues);

    while (it.hasNext() == true)
    {
        SceneValue sv(it.next());

        Fixture *fixture = m_doc->fixture(sv.fxi);
        Q_ASSERT(fixture != NULL);

        FixtureConsole *fc = fixtureConsole(fixture);
        if (fc != NULL)
        {
            fc->blockSignals(true);
            fc->setSceneValue(sv);
            fc->blockSignals(false);
        }
    }
}

void SceneEditor::slotFixtureRemoved(quint32 id)
{
    removeFixtureTab(id);
    removeFixtureItem(id);

    QListIterator <SceneValue> it(m_scene->values());

    while (it.hasNext() == true)
    {
        SceneValue sv(it.next());
        if (sv.fxi == id)
            m_scene->unsetValue(id, sv.channel);
    }
    m_scene->removeFixture(id);
}

void SceneEditor::init(bool applyValues)
{
    QVariant tabMode = m_scene->uiStateValue(UI_STATE_TAB_MODE);

    this->layout()->setContentsMargins(8, 3, 8, 3);

    /* Actions */
    m_enableCurrentAction = new QAction(QIcon(":/check.png"),
                                        tr("Enable all channels in current fixture"), this);
    m_disableCurrentAction = new QAction(QIcon(":/uncheck.png"),
                                         tr("Disable all channels in current fixture"), this);
    m_copyAction = new QAction(QIcon(":/editcopy.png"),
                               tr("Copy current values to clipboard"), this);
    m_pasteAction = new QAction(QIcon(":/editpaste.png"),
                                tr("Paste clipboard values to current fixture"), this);
    m_copyToAllAction = new QAction(QIcon(":/editcopyall.png"),
                                    tr("Copy current values to all fixtures"), this);
    m_colorToolAction = new QAction(QIcon(":/color.png"),
                                    tr("Color tool for CMY/RGB-capable fixtures"), this);
    m_positionToolAction = new QAction(QIcon(":/xypad.png"),
                                    tr("Position tool for moving heads/scanners"), this);
    m_tabViewAction = new QAction(QIcon(":/tabview.png"),
                                    tr("Switch between tab view and all channels view"), this);
    m_blindAction = new QAction(QIcon(":/blind.png"),
                                tr("Toggle blind mode"), this);
    m_speedDialAction = new QAction(QIcon(":/speed.png"),
                                    tr("Show/Hide speed dial window"), this);
    m_recordAction = new QAction(QIcon(":/record.png"),
                                 tr("Clone this scene and append as a new step to the selected chaser"), this);

    m_nextTabAction = new QAction(QIcon(":/forward.png"), tr("Go to next fixture tab"), this);
    m_nextTabAction->setShortcut(QKeySequence("Alt+Right"));
    connect(m_nextTabAction, SIGNAL(triggered(bool)),
            this, SLOT(slotGoToNextTab()));
    m_prevTabAction = new QAction(QIcon(":/back.png"), tr("Go to previous fixture tab"), this);
    m_prevTabAction->setShortcut(QKeySequence("Alt+Left"));
    connect(m_prevTabAction, SIGNAL(triggered(bool)),
            this, SLOT(slotGoToPreviousTab()));

    // Speed Dial initial state
    m_speedDialAction->setCheckable(true);

    // Blind initial state
    m_blindAction->setCheckable(true);

    m_tabViewAction->setCheckable(true);
    if (tabMode.isNull() || tabMode.toInt() == UI_STATE_TABBED_FIXTURES)
        m_tabViewAction->setChecked(true);

    m_hardwareAction = new QAction(QIcon(":/configure.png"),tr("Configure editing hardware"), this);
    connect(m_hardwareAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHardwareAction()));

    // Chaser combo init
    quint32 selectId = Function::invalidId();
    QSettings settings;
    QVariant var = settings.value(SETTINGS_CHASER);
    if (var.isValid() == true)
        selectId = var.toUInt();
    m_chaserCombo = new QComboBox(this);
    m_chaserCombo->setMaximumWidth(250);
    m_chaserCombo->addItem(tr("None"), Function::invalidId());
    slotChaserComboActivated(0);
    foreach (Function *function, m_doc->functionsByType(Function::ChaserType))
    {
        m_chaserCombo->addItem(function->name(), function->id());
        if (function->id() == selectId)
        {
            int index = m_chaserCombo->count() - 1;
            m_chaserCombo->setCurrentIndex(index);
            slotChaserComboActivated(index);
        }
    }
    QLabel *nameLabel = new QLabel(tr("Scene name:"));
    m_nameEdit = new QLineEdit();

    // Connections
    connect(m_enableCurrentAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEnableCurrent()));
    connect(m_disableCurrentAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDisableCurrent()));
    connect(m_copyAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCopy()));
    connect(m_pasteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPaste()));
    connect(m_copyToAllAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCopyToAll()));
    connect(m_colorToolAction, SIGNAL(triggered(bool)),
            this, SLOT(slotColorTool()));
    connect(m_positionToolAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPositionTool()));
    connect(m_speedDialAction, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialToggle(bool)));
    connect(m_tabViewAction, SIGNAL(toggled(bool)),
            this, SLOT(slotViewModeChanged(bool)));
    connect(m_blindAction, SIGNAL(toggled(bool)),
            this, SLOT(slotBlindToggled(bool)));
    connect(m_recordAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRecord()));
    connect(m_chaserCombo, SIGNAL(activated(int)),
            this, SLOT(slotChaserComboActivated(int)));
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));

    /* Toolbar */
    QToolBar* toolBar = new QToolBar(this);
    layout()->setMenuBar(toolBar);
    toolBar->addAction(m_enableCurrentAction);
    toolBar->addAction(m_disableCurrentAction);
    toolBar->addSeparator();
    toolBar->addAction(m_prevTabAction);
    toolBar->addAction(m_nextTabAction);
    toolBar->addSeparator();
    toolBar->addAction(m_copyAction);
    toolBar->addAction(m_pasteAction);
    toolBar->addAction(m_copyToAllAction);
    toolBar->addSeparator();
    toolBar->addAction(m_colorToolAction);
    toolBar->addAction(m_positionToolAction);
    toolBar->addSeparator();
    toolBar->addAction(m_speedDialAction);
    toolBar->addAction(m_tabViewAction);
    toolBar->addSeparator();
    toolBar->addAction(m_blindAction);
    toolBar->addSeparator();
    toolBar->addAction(m_hardwareAction);
    toolBar->addSeparator();
    toolBar->addAction(m_recordAction);
    toolBar->addWidget(m_chaserCombo);
    toolBar->addSeparator();
    toolBar->addWidget(nameLabel);
    toolBar->addWidget(m_nameEdit);

    /* Tab widget */
    connect(m_tab, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));

    /* Add & remove buttons */
    connect(m_addFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotAddFixtureClicked()));
    connect(m_removeFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveFixtureClicked()));

    m_nameEdit->setText(m_scene->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));

    // Channels groups tab
    QList<quint32> chGrpIds = m_scene->channelGroups();
    QListIterator <ChannelsGroup*> scg(m_doc->channelsGroups());
    while (scg.hasNext() == true)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_channelGroupsTree);
        ChannelsGroup *grp = scg.next();
        item->setText(KColumnName, grp->name());
        item->setData(KColumnName, Qt::UserRole, grp->id());

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (chGrpIds.contains(grp->id()))
            item->setCheckState(KColumnName, Qt::Checked);
        else
            item->setCheckState(KColumnName, Qt::Unchecked);
    }
    connect(m_channelGroupsTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotChannelGroupsChanged(QTreeWidgetItem*,int)));
    updateChannelsGroupsTab();

    // Apply any mode related change
    slotModeChanged(m_doc->mode());

    // Fixtures & tabs
    // Fill the fixtures list from the Scene fixtures
    foreach (quint32 fixtureID, m_scene->fixtures())
    {
        if (fixtureItem(fixtureID) == NULL)
        {
            Fixture* fixture = m_doc->fixture(fixtureID);
            if (fixture == NULL)
                continue;
            addFixtureItem(fixture);
        }
    }

    // Complete the fixtures list from the Scene values
    // (This should be useless)
    QListIterator <SceneValue> it(m_scene->values());
    while (it.hasNext() == true)
    {
        SceneValue scv(it.next());

        if (fixtureItem(scv.fxi) == NULL)
        {
            qWarning() << Q_FUNC_INFO
                << "Fixture" << scv.fxi << "was not in the scene fixture list!";
            Fixture* fixture = m_doc->fixture(scv.fxi);
            if (fixture == NULL)
                continue;

            addFixtureItem(fixture);
        }
    }

    // Create the actual tab view
    if (tabMode.isNull() || tabMode.toInt() == UI_STATE_TABBED_FIXTURES)
        slotViewModeChanged(true, applyValues);
    else
        slotViewModeChanged(false, applyValues);
}

void SceneEditor::setSceneValue(const SceneValue& scv)
{
    FixtureConsole* fc;
    Fixture* fixture;

    fixture = m_doc->fixture(scv.fxi);
    Q_ASSERT(fixture != NULL);

    fc = fixtureConsole(fixture);
    if (fc != NULL)
        fc->setSceneValue(scv);
}


void SceneEditor::setBlindModeEnabled(bool active)
{
    m_blindAction->setChecked(active);
}

/*****************************************************************************
 * Common
 *****************************************************************************/

void SceneEditor::slotTabChanged(int tab)
{
    m_currentTab = tab;
    QLCClipboard *clipboard = m_doc->clipboard();

    m_scene->setUiStateValue(UI_STATE_TAB_INDEX, tab);

    if (tab == KTabGeneral)
    {
        m_enableCurrentAction->setEnabled(false);
        m_disableCurrentAction->setEnabled(false);

        m_copyAction->setEnabled(false);
        m_pasteAction->setEnabled(false);
        m_copyToAllAction->setEnabled(false);
        m_colorToolAction->setEnabled(false);
    }
    else
    {
        m_enableCurrentAction->setEnabled(true);
        m_disableCurrentAction->setEnabled(true);

        m_copyAction->setEnabled(true);
        if (clipboard->hasSceneValues())
            m_pasteAction->setEnabled(true);
        else
            m_pasteAction->setEnabled(false);

        if (m_tabViewAction->isChecked())
            m_copyToAllAction->setEnabled(true);
        else
            m_copyToAllAction->setEnabled(false);
        m_colorToolAction->setEnabled(isColorToolAvailable());
        m_positionToolAction->setEnabled(isPositionToolAvailable());
    }
}

void SceneEditor::slotEnableCurrent()
{
    if (m_tabViewAction->isChecked())
    {
        /* QObject cast fails unless the widget is a FixtureConsole */
        FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
        if (fc != NULL)
            fc->setChecked(true);
    }
    else
    {
        foreach(FixtureConsole *fc, m_consoleList.values())
        {
            if (fc == NULL)
                continue;
            fc->setChecked(true);
        }
    }
}

void SceneEditor::slotDisableCurrent()
{
    if (m_tabViewAction->isChecked())
    {
        /* QObject cast fails unless the widget is a FixtureConsole */
        FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
        if (fc != NULL)
            fc->setChecked(false);
    }
    else
    {
        foreach(FixtureConsole *fc, m_consoleList.values())
        {
            if (fc == NULL)
                continue;
            fc->setChecked(false);
        }
    }
}

void SceneEditor::slotCopy()
{
    QList <SceneValue> copyList;
    QLCClipboard *clipboard = m_doc->clipboard();

    /* QObject cast fails unless the widget is a FixtureConsole */
    if (m_tabViewAction->isChecked())
    {
        FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
        if (fc != NULL)
        {
            copyList = fc->values();
            if (fc->hasSelections())
                m_copyFromSelection = true;
            else
                m_copyFromSelection = false;
            clipboard->copyContent(m_scene->id(), copyList);
        }
    }
    else
    {
        bool oneHasSelection = false;
        QList <SceneValue> selectedOnlyList;
        foreach(FixtureConsole *fc, m_consoleList.values())
        {
            if (fc == NULL)
                continue;
            copyList.append(fc->values());
            if (fc->hasSelections())
            {
                oneHasSelection = true;
                selectedOnlyList.append(fc->values());
            }
        }
        m_copyFromSelection = oneHasSelection;
        if (m_copyFromSelection == true)
            clipboard->copyContent(m_scene->id(), selectedOnlyList);
        else
            clipboard->copyContent(m_scene->id(), copyList);
    }
    if (copyList.count() > 0)
        m_pasteAction->setEnabled(true);
}

void SceneEditor::slotPaste()
{
    QLCClipboard *clipboard = m_doc->clipboard();

    if (clipboard->hasSceneValues() == false)
        return;

    if (m_tabViewAction->isChecked())
    {
        FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
        if (fc != NULL)
            fc->setValues(clipboard->getSceneValues(), m_copyFromSelection);
    }
    else
    {
        foreach(FixtureConsole *fc, m_consoleList.values())
        {
            if (fc == NULL)
                continue;
            quint32 fxi = fc->fixture();
            QList<SceneValue>thisFixtureVals;
            foreach(SceneValue val, clipboard->getSceneValues())
            {
                if (val.fxi == fxi)
                    thisFixtureVals.append(val);
            }
            fc->setValues(thisFixtureVals, m_copyFromSelection);
        }
    }
}

void SceneEditor::slotCopyToAll()
{
    slotCopy();

    QLCClipboard *clipboard = m_doc->clipboard();

    if (clipboard->hasSceneValues())
    {
        for (int i = m_fixtureFirstTabIndex; i < m_tab->count(); i++)
        {
            FixtureConsole* fc = fixtureConsoleTab(i);
            if (fc != NULL)
                fc->setValues(clipboard->getSceneValues(), m_copyFromSelection);
        }
    }

    //m_copy.clear();
    m_pasteAction->setEnabled(false);
}

void SceneEditor::slotColorTool()
{
    QColor color = slotColorSelectorChanged(QColor());

    QColorDialog dialog(color);
    connect(&dialog, SIGNAL(currentColorChanged(const QColor&)),
            this, SLOT(slotColorSelectorChanged(const QColor&)));

    int result = dialog.exec();
    if (result == QDialog::Rejected)
    {
        slotColorSelectorChanged(color); // reset color to what it previously was
    }
}

void SceneEditor::slotPositionTool()
{
    FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
    if (fc != NULL)
    {
        QList<SceneValue> origValues = fc->values();

        Fixture* fxi = m_doc->fixture(fc->fixture());
        QPointF pos;
        QRectF range;
        bool panFound = false;
        bool tiltFound = false;

        Q_ASSERT(fxi != NULL);

        for (int i = 0; i < fxi->heads(); ++i )
        {
             if (!range.isValid())
                 range = fxi->degreesRange(i);

             quint32 panMsbChannel = fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, i);
             quint32 panLsbChannel = fxi->channelNumber(QLCChannel::Pan, QLCChannel::LSB, i);
             quint32 tiltMsbChannel = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, i);
             quint32 tiltLsbChannel = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::LSB, i);

             if (panMsbChannel != QLCChannel::invalid())
             {
                 if (!panFound )
                 {
                     qDebug() << "panFound" << i;
                     panFound = true;
                     qreal v = qreal(fc->value(panMsbChannel));
                     if (panLsbChannel != QLCChannel::invalid())
                     {
                        v += qreal(fc->value(panLsbChannel)) / qreal(256);
                     }

                     pos.setX(v);
                 }
             }

             if (tiltMsbChannel != QLCChannel::invalid())
             {
                 if (!tiltFound )
                 {
                     tiltFound = true;
                     qDebug() << "tiltFound" << i;
                     qreal v = qreal(fc->value(tiltMsbChannel));
                     if (tiltLsbChannel != QLCChannel::invalid())
                     {
                        v += qreal(fc->value(tiltLsbChannel)) / qreal(256);
                     }

                     pos.setY(v);
                 }
             }
        }

        PositionTool dialog(pos, range);
        connect(&dialog, SIGNAL(currentPositionChanged(const QPointF&)),
            this, SLOT(slotPositionSelectorChanged(const QPointF&)));

        int result = dialog.exec();
        if (result == QDialog::Rejected)
        {
            fc->setValues(origValues, false); // reset position to what it previously was
        }
    }
}

QColor SceneEditor::slotColorSelectorChanged(const QColor& color)
{
    QColor returnColor = QColor();

    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
    if (fc != NULL)
    {
        Fixture* fxi = m_doc->fixture(fc->fixture());
        Q_ASSERT(fxi != NULL);

        QSet <quint32> cyan    = fxi->channels(QLCChannel::Intensity, QLCChannel::Cyan);
        QSet <quint32> magenta = fxi->channels(QLCChannel::Intensity, QLCChannel::Magenta);
        QSet <quint32> yellow  = fxi->channels(QLCChannel::Intensity, QLCChannel::Yellow);
        QSet <quint32> red     = fxi->channels(QLCChannel::Intensity, QLCChannel::Red);
        QSet <quint32> green   = fxi->channels(QLCChannel::Intensity, QLCChannel::Green);
        QSet <quint32> blue    = fxi->channels(QLCChannel::Intensity, QLCChannel::Blue);

        if (!cyan.isEmpty() && !magenta.isEmpty() && !yellow.isEmpty())
        {
            returnColor.setCmyk(fc->value(*cyan.begin()),
                                fc->value(*magenta.begin()),
                                fc->value(*yellow.begin()),
                                0);
            if (color.isValid() == true)
            {
                foreach (quint32 ch, cyan)
                {
                    fc->setChecked(true, ch);
                    fc->setValue(ch, color.cyan());
                }

                foreach (quint32 ch, magenta)
                {
                    fc->setChecked(true, ch);
                    fc->setValue(ch, color.magenta());
                }

                foreach (quint32 ch, yellow)
                {
                    fc->setChecked(true, ch);
                    fc->setValue(ch, color.yellow());
                }
            }
        }
        else if (!red.isEmpty() && !green.isEmpty() && !blue.isEmpty())
        {
            returnColor.setRgb(fc->value(*red.begin()),
                               fc->value(*green.begin()),
                               fc->value(*blue.begin()),
                               0);

            if (color.isValid() == true)
            {
                foreach (quint32 ch, red)
                {
                    fc->setChecked(true, ch);
                    fc->setValue(ch, color.red());
                }

                foreach (quint32 ch, green)
                {
                    fc->setChecked(true, ch);
                    fc->setValue(ch, color.green());
                }

                foreach (quint32 ch, blue)
                {
                    fc->setChecked(true, ch);
                    fc->setValue(ch, color.blue());
                }
            }
        }
        return returnColor;
    }

    /* QObject cast fails unless the widget is a GroupsConsole */
    GroupsConsole* gc = groupConsoleTab(m_currentTab);
    if (gc != NULL)
    {
        foreach(ConsoleChannel *cc, gc->groups())
        {
            Fixture* fxi = m_doc->fixture(cc->fixture());
            Q_ASSERT(fxi != NULL);
            const QLCChannel *ch = fxi->channel(cc->channelIndex());
            if (ch->group() == QLCChannel::Intensity)
            {
                if (ch->colour() == QLCChannel::Red)
                    cc->setValue(color.red());
                else if (ch->colour() == QLCChannel::Green)
                    cc->setValue(color.green());
                else if (ch->colour() == QLCChannel::Blue)
                    cc->setValue(color.blue());
                else if (ch->colour() == QLCChannel::Magenta)
                    cc->setValue(color.magenta());
                else if (ch->colour() == QLCChannel::Yellow)
                    cc->setValue(color.yellow());
                else if (ch->colour() == QLCChannel::Cyan)
                    cc->setValue(color.cyan());
            }
        }
    }

    return returnColor;
}

void SceneEditor::slotPositionSelectorChanged(const QPointF& position)
{
    qreal x = position.x();
    qreal y = position.y();

    uchar panMsbNew = x;
    uchar panLsbNew = (x - floor(x)) * 256;
    uchar tiltMsbNew = y;
    uchar tiltLsbNew = (y - floor(y)) * 256;

    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
    if (fc != NULL)
    {
        Fixture* fxi = m_doc->fixture(fc->fixture());
        Q_ASSERT(fxi != NULL);

        for (int i = 0; i < fxi->heads(); ++i )
        {
             quint32 panMsbChannel = fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, i);
             quint32 panLsbChannel = fxi->channelNumber(QLCChannel::Pan, QLCChannel::LSB, i);
             quint32 tiltMsbChannel = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, i);
             quint32 tiltLsbChannel = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::LSB, i);

             if (panMsbChannel != QLCChannel::invalid())
             {
                 fc->setChecked(true, panMsbChannel);
                 fc->setValue(panMsbChannel, panMsbNew);

                 if (panLsbChannel != QLCChannel::invalid())
                 {
                     fc->setChecked(true, panLsbChannel);
                     fc->setValue(panLsbChannel, panLsbNew);
                 }
             }

             if (tiltMsbChannel != QLCChannel::invalid())
             {
                 fc->setChecked(true, tiltMsbChannel);
                 fc->setValue(tiltMsbChannel, tiltMsbNew);

                 if (tiltLsbChannel != QLCChannel::invalid())
                 {
                     fc->setChecked(true, tiltLsbChannel);
                     fc->setValue(tiltLsbChannel, tiltLsbNew);
                 }
             }
        }
    }

    /* QObject cast fails unless the widget is a GroupsConsole */
    GroupsConsole* gc = groupConsoleTab(m_currentTab);
    if (gc != NULL)
    {
        foreach(ConsoleChannel *cc, gc->groups())
        {
            Fixture* fxi = m_doc->fixture(cc->fixture());
            Q_ASSERT(fxi != NULL);
            const QLCChannel *ch = fxi->channel(cc->channelIndex());
            if (ch->group() == QLCChannel::Pan)
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    cc->setValue(panMsbNew);
                else
                    cc->setValue(panLsbNew);
            }
            else if(ch->group() == QLCChannel::Tilt)
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    cc->setValue(tiltMsbNew);
                else
                    cc->setValue(tiltLsbNew);
            }
        }
    }
}

void SceneEditor::slotSpeedDialToggle(bool state)
{
    if (state == true)
    {
        createSpeedDials();
    }
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }

    m_scene->setUiStateValue(UI_STATE_SHOW_DIAL, state);
}

void SceneEditor::slotBlindToggled(bool state)
{
    if (m_doc->mode() == Doc::Operate)
    {
        delete m_source;
        m_source = NULL;

        if (m_scene != NULL && !m_scene->isRunning())
        {
            m_source = new GenericDMXSource(m_doc);
            foreach(SceneValue scv, m_scene->values())
                m_source->set(scv.fxi, scv.channel, scv.value);
        }
    }
    else
    {
        if (m_source == NULL)
            m_source = new GenericDMXSource(m_doc);
    }

    if (m_source != NULL)
        m_source->setOutputEnabled(!state);
}

void SceneEditor::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        m_blindAction->setChecked(true);
        slotBlindToggled(true);
    }
    else
    {
        m_blindAction->setChecked(false);
        slotBlindToggled(false);
    }

}

void SceneEditor::slotViewModeChanged(bool tabbed, bool applyValues)
{
    m_tab->blockSignals(true);
    for (int i = m_tab->count() - 1; i >= m_fixtureFirstTabIndex; i--)
    {
        QScrollArea* area = qobject_cast<QScrollArea*> (m_tab->widget(i));
        Q_ASSERT(area != NULL);
        m_tab->removeTab(i);
        delete area; // Deletes also FixtureConsole
    }
    m_consoleList.clear();
    m_tab->blockSignals(false);

    // all fixtures view mode
    if (tabbed == false)
    {
        QListIterator <Fixture*> it(selectedFixtures());
        if (it.hasNext() == true)
        {
            QScrollArea* scrollArea = new QScrollArea(m_tab);

            scrollArea->setWidgetResizable(true);
            int tIdx = m_tab->addTab(scrollArea, tr("All fixtures"));
            m_tab->setTabToolTip(tIdx, tr("All fixtures"));

            QGroupBox* grpBox = new QGroupBox(scrollArea);
            grpBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
            QHBoxLayout* fixturesLayout = new QHBoxLayout(grpBox);
            grpBox->setLayout(fixturesLayout);
            fixturesLayout->setSpacing(2);
            fixturesLayout->setContentsMargins(0, 2, 2, 2);

            int c = 0;
            while (it.hasNext() == true)
            {
                Fixture* fixture = it.next();
                Q_ASSERT(fixture != NULL);
                FixtureConsole* console = NULL;
                if (c%2 == 0)
                    console = new FixtureConsole(scrollArea, m_doc, FixtureConsole::GroupOdd);
                else
                    console = new FixtureConsole(scrollArea, m_doc, FixtureConsole::GroupEven);
                console->setFixture(fixture->id());
                console->setChecked(false);
                m_consoleList[fixture->id()] = console;

                connect(console, SIGNAL(valueChanged(quint32,quint32,uchar)),
                        this, SLOT(slotValueChanged(quint32,quint32,uchar)));
                connect(console, SIGNAL(checked(quint32,quint32,bool)),
                        this, SLOT(slotChecked(quint32,quint32,bool)));

                QListIterator <SceneValue> it(m_scene->values());
                while (it.hasNext() == true)
                {
                    SceneValue scv(it.next());
                    if (applyValues == false)
                        scv.value = 0;
                    if (scv.fxi == fixture->id())
                        console->setSceneValue(scv);
                }

                fixturesLayout->addWidget(console);
                c++;
            }
            fixturesLayout->addStretch(1);
            scrollArea->setWidget(grpBox);
        }
    }
    // tabbed fixtures view mode
    else
    {
        QListIterator <Fixture*> it(selectedFixtures());
        while (it.hasNext() == true)
        {
            Fixture* fixture = it.next();
            Q_ASSERT(fixture != NULL);

            addFixtureTab(fixture);

            QListIterator <SceneValue> it(m_scene->values());
            while (it.hasNext() == true)
            {
                SceneValue scv(it.next());
                if (applyValues == false)
                    scv.value = 0;
                if (scv.fxi == fixture->id())
                    setSceneValue(scv);
            }
        }
    }

    m_scene->setUiStateValue(UI_STATE_TAB_MODE, tabbed ? UI_STATE_TABBED_FIXTURES : UI_STATE_ALL_FIXTURES);

    if (m_tab->count() == 0)
    {
        slotTabChanged(KTabGeneral);
    }
    else
    {
        QVariant tabIndex = m_scene->uiStateValue(UI_STATE_TAB_INDEX);
        int prevTabIdx = tabIndex.isValid() ? tabIndex.toInt() : 0;
        if (prevTabIdx > m_tab->count())
            m_tab->setCurrentIndex(m_fixtureFirstTabIndex);
        else
            m_tab->setCurrentIndex(prevTabIdx);
    }

    m_scene->setUiStateValue(UI_STATE_TAB_INDEX, m_tab->currentIndex());
}

void SceneEditor::slotRecord()
{
    Chaser* chaser = selectedChaser();
    if (chaser == NULL)
        return;

    QString name = chaser->name() + QString(" - %1").arg(chaser->steps().size() + 1);
    Scene* clone = new Scene(m_doc);
    clone->copyFrom(m_scene);
    clone->setName(name);
    m_doc->addFunction(clone);
    chaser->addStep(ChaserStep(clone->id()));

    // Switch to the cloned scene
    FunctionManager::instance()->selectFunction(clone->id());
}

void SceneEditor::slotChaserComboActivated(int index)
{
    quint32 id = m_chaserCombo->itemData(index).toUInt();
    if (id == Function::invalidId())
        m_recordAction->setEnabled(false);
    else
        m_recordAction->setEnabled(true);
}

bool SceneEditor::isColorToolAvailable()
{
    Fixture* fxi = NULL;
    quint32 cyan = QLCChannel::invalid(), magenta = QLCChannel::invalid(), yellow = QLCChannel::invalid();
    quint32 red = QLCChannel::invalid(), green = QLCChannel::invalid(), blue = QLCChannel::invalid();

    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
    if (fc != NULL)
    {
        fxi = m_doc->fixture(fc->fixture());
        Q_ASSERT(fxi != NULL);

        cyan = fxi->channel(QLCChannel::Intensity, QLCChannel::Cyan);
        magenta = fxi->channel(QLCChannel::Intensity, QLCChannel::Magenta);
        yellow = fxi->channel(QLCChannel::Intensity, QLCChannel::Yellow);
        red = fxi->channel(QLCChannel::Intensity, QLCChannel::Red);
        green = fxi->channel(QLCChannel::Intensity, QLCChannel::Green);
        blue = fxi->channel(QLCChannel::Intensity, QLCChannel::Blue);
    }

    GroupsConsole* gc = groupConsoleTab(m_currentTab);
    if (gc != NULL)
    {
        cyan = magenta = yellow = red = green = blue = QLCChannel::invalid();
        foreach(ConsoleChannel *cc, gc->groups())
        {
            fxi = m_doc->fixture(cc->fixture());
            Q_ASSERT(fxi != NULL);
            const QLCChannel *ch = fxi->channel(cc->channelIndex());
            if (ch->group() == QLCChannel::Intensity)
            {
                if (ch->colour() == QLCChannel::Red)
                    red = 1;
                else if (ch->colour() == QLCChannel::Green)
                    green = 1;
                else if (ch->colour() == QLCChannel::Blue)
                    blue = 1;
                else if (ch->colour() == QLCChannel::Magenta)
                    magenta = 1;
                else if (ch->colour() == QLCChannel::Yellow)
                    yellow = 1;
                else if (ch->colour() == QLCChannel::Cyan)
                    cyan = 1;
            }
        }
    }

    if (cyan != QLCChannel::invalid() && magenta != QLCChannel::invalid() &&
        yellow != QLCChannel::invalid())
    {
        return true;
    }
    else if (red != QLCChannel::invalid() && green != QLCChannel::invalid() &&
             blue != QLCChannel::invalid())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SceneEditor::isPositionToolAvailable()
{
    Fixture* fxi = NULL;

    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = fixtureConsoleTab(m_currentTab);
    if (fc != NULL)
    {
        fxi = m_doc->fixture(fc->fixture());
        Q_ASSERT(fxi != NULL);

        for (int i = 0; i < fxi->heads(); ++i)
        {
            if (fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, i) != QLCChannel::invalid())
                return true;
            if (fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, i) != QLCChannel::invalid())
                return true;
        }
    }

    GroupsConsole* gc = groupConsoleTab(m_currentTab);
    if (gc != NULL)
    {
        foreach(ConsoleChannel *cc, gc->groups())
        {
            fxi = m_doc->fixture(cc->fixture());
            Q_ASSERT(fxi != NULL);
            const QLCChannel *ch = fxi->channel(cc->channelIndex());
            if (ch->group() == QLCChannel::Pan || ch->group() == QLCChannel::Tilt)
                return true;
        }
    }

    return false;
}

void SceneEditor::createSpeedDials()
{
    if (m_speedDials != NULL)
        return;

    m_speedDials = new SpeedDialWidget(this);
    m_speedDials->setAttribute(Qt::WA_DeleteOnClose);
    m_speedDials->setWindowTitle(m_scene->name());
    m_speedDials->setFadeInSpeed(m_scene->fadeInSpeed());
    m_speedDials->setFadeOutSpeed(m_scene->fadeOutSpeed());
    m_speedDials->setDurationEnabled(false);
    m_speedDials->setDurationVisible(false);
    connect(m_speedDials, SIGNAL(fadeInChanged(int)), this, SLOT(slotFadeInChanged(int)));
    connect(m_speedDials, SIGNAL(fadeOutChanged(int)), this, SLOT(slotFadeOutChanged(int)));
    connect(m_speedDials, SIGNAL(destroyed(QObject*)), this, SLOT(slotDialDestroyed(QObject*)));
    m_speedDials->show();
}

void SceneEditor::slotDialDestroyed(QObject *)
{
    m_speedDialAction->setChecked(false);
}

Chaser* SceneEditor::selectedChaser() const
{
    QVariant var = m_chaserCombo->itemData(m_chaserCombo->currentIndex());
    if (var.isValid() == false)
        return NULL;
    else
        return qobject_cast<Chaser*> (m_doc->function(var.toUInt()));
}

/*****************************************************************************
 * General page
 *****************************************************************************/

QTreeWidgetItem* SceneEditor::fixtureItem(quint32 fxi_id)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QTreeWidgetItem* item = *it;
        if (item->text(KColumnID).toUInt() == fxi_id)
            return item;
        ++it;
    }

    return NULL;
}

QList <Fixture*> SceneEditor::selectedFixtures() const
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    QList <Fixture*> list;

    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item;
        quint32 fxi_id;
        Fixture* fixture;

        item = it.next();
        fxi_id = item->text(KColumnID).toInt();
        fixture = m_doc->fixture(fxi_id);
        Q_ASSERT(fixture != NULL);

        list.append(fixture);
    }

    return list;
}

bool SceneEditor::addFixtureItem(Fixture* fixture)
{
    Q_ASSERT(fixture != NULL);

    // check if the fixture is already there
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *fix = m_tree->topLevelItem(i);
        if (fix != NULL)
        {
            quint32 fxid = fix->text(KColumnID).toUInt();
            if (fxid == fixture->id())
                return false;
        }
    }

    QTreeWidgetItem* item;

    item = new QTreeWidgetItem(m_tree);
    item->setText(KColumnName, fixture->name());
    item->setText(KColumnID, QString("%1").arg(fixture->id()));

    if (fixture->fixtureDef() == NULL)
    {
        item->setText(KColumnManufacturer, tr("Generic"));
        item->setText(KColumnModel, tr("Generic"));
    }
    else
    {
        item->setText(KColumnManufacturer,
                      fixture->fixtureDef()->manufacturer());
        item->setText(KColumnModel, fixture->fixtureDef()->model());
    }

    /* Select newly-added fixtures so that their channels can be
       quickly disabled/enabled */
    item->setSelected(true);

    return true;
}

void SceneEditor::removeFixtureItem(quint32 fixtureID)
{
    QTreeWidgetItem *item;

    item = fixtureItem(fixtureID);
    delete item;
}

void SceneEditor::slotNameEdited(const QString& name)
{
    m_scene->setName(name);
    if (m_speedDials != NULL)
        m_speedDials->setWindowTitle(m_scene->name());
}

void SceneEditor::slotAddFixtureClicked()
{
    /* Put all fixtures already present into a list of fixtures that
       will be disabled in the fixture selection dialog */
    QList <quint32> disabled;
    QTreeWidgetItemIterator twit(m_tree);
    while (*twit != NULL)
    {
        disabled.append((*twit)->text(KColumnID).toInt());
        twit++;
    }

    /* Get a list of new fixtures to add to the scene */
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFixtures(disabled);
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
        {
            Fixture *fixture = m_doc->fixture(it.next());
            Q_ASSERT(fixture != NULL);

            addFixtureItem(fixture);
            addFixtureTab(fixture);

            // Add fixture in scene
            m_scene->addFixture(fixture->id());
        }
    }
}

void SceneEditor::slotRemoveFixtureClicked()
{
    int r = QMessageBox::question(
                this, tr("Remove fixtures"),
                tr("Do you want to remove the selected fixture(s)?"),
                QMessageBox::Yes, QMessageBox::No);

    if (r == QMessageBox::Yes)
    {
        QListIterator <Fixture*> it(selectedFixtures());
        while (it.hasNext() == true)
        {
            Fixture* fixture = it.next();
            Q_ASSERT(fixture != NULL);

            removeFixtureTab(fixture->id());
            removeFixtureItem(fixture->id());

            /* Remove all values associated to the fixture */
            for (quint32 i = 0; i < fixture->channels(); i++)
                m_scene->unsetValue(fixture->id(), i);

            // Remove fixture from scene
            m_scene->removeFixture(fixture->id());
        }
    }
}

void SceneEditor::slotEnableAll()
{
    foreach (FixtureConsole* fc, m_consoleList.values())
    {
        if (fc != NULL)
            fc->setChecked(true);
    }
}

void SceneEditor::slotDisableAll()
{
    foreach (FixtureConsole* fc, m_consoleList.values())
    {
        if (fc != NULL)
            fc->setChecked(false);
    }
}

void SceneEditor::slotFadeInChanged(int ms)
{
    m_scene->setFadeInSpeed(ms);
}

void SceneEditor::slotFadeOutChanged(int ms)
{
    m_scene->setFadeOutSpeed(ms);
}

void SceneEditor::slotEnableAllChannelGroups()
{
    for (int i = 0; i < m_channelGroupsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_channelGroupsTree->topLevelItem(i);
        item->setCheckState(KColumnName, Qt::Checked);
    }
}

void SceneEditor::slotDisableAllChannelGroups()
{
    for (int i = 0; i < m_channelGroupsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_channelGroupsTree->topLevelItem(i);
        item->setCheckState(KColumnName, Qt::Unchecked);
    }
}

void SceneEditor::slotChannelGroupsChanged(QTreeWidgetItem *item, int column)
{
    if (item == NULL)
        return;

    quint32 grpID = item->data(column, Qt::UserRole).toUInt();
    ChannelsGroup *grp = m_doc->channelsGroup(grpID);
    if (grp == NULL)
        return;

    if (item->checkState(column) == Qt::Checked)
    {
        m_scene->addChannelGroup(grpID);
        foreach (SceneValue val, grp->getChannels())
        {
            Fixture *fixture = m_doc->fixture(val.fxi);
            if (fixture != NULL)
            {
                if (addFixtureItem(fixture) == true)
                    addFixtureTab(fixture, val.channel);
                else
                    setTabChannelState(true, fixture, val.channel);
            }
        }
    }
    else
    {
        m_scene->removeChannelGroup(grpID);
        foreach (SceneValue val, grp->getChannels())
        {
            Fixture *fixture = m_doc->fixture(val.fxi);
            if (fixture != NULL)
                setTabChannelState(false, fixture, val.channel);
        }
    }

    qDebug() << Q_FUNC_INFO << "Groups in list: " << m_scene->channelGroups().count();

    updateChannelsGroupsTab();
}

/*********************************************************************
 * Channels groups tabs
 *********************************************************************/
void SceneEditor::updateChannelsGroupsTab()
{
    QScrollArea* scrollArea = NULL;
    QList <quint32> ids = m_scene->channelGroups();

    if (m_channelGroupsTree->topLevelItemCount() == 0)
    {
        m_fixtureFirstTabIndex = 1;
        return;
    }

    /* Get a scroll area for the console */
    if (m_channelGroupsTab != -1)
    {
        scrollArea = qobject_cast<QScrollArea*> (m_tab->widget(m_channelGroupsTab));
        Q_ASSERT(scrollArea != NULL);
        GroupsConsole *tmpGrpConsole = qobject_cast<GroupsConsole*> (scrollArea->widget());
        Q_ASSERT(tmpGrpConsole != NULL);
        delete tmpGrpConsole;
        if (ids.count() == 0)
        {
            m_tab->removeTab(1);
            m_channelGroupsTab = -1;
            m_fixtureFirstTabIndex = 1;
            return;
        }
    }
    else
    {
        if (ids.count() == 0)
            return;
        scrollArea = new QScrollArea(m_tab);
    }

    QList<uchar>levels = m_scene->channelGroupsLevels();
    GroupsConsole* console = new GroupsConsole(scrollArea, m_doc, ids, levels);
    scrollArea->setWidget(console);
    scrollArea->setWidgetResizable(true);
    if (m_channelGroupsTab == -1)
    {
        m_tab->insertTab(1, scrollArea, tr("Channels Groups"));
        m_tab->setTabToolTip(1, tr("Channels Groups"));
    }

    m_channelGroupsTab = 1;
    m_fixtureFirstTabIndex = 2;
    connect(console, SIGNAL(groupValueChanged(quint32,uchar)),
            this, SLOT(slotGroupValueChanged(quint32,uchar)));
}

GroupsConsole *SceneEditor::groupConsoleTab(int tab)
{
    if (tab != m_channelGroupsTab)
        return NULL;

    QScrollArea* area = qobject_cast<QScrollArea*> (m_tab->widget(tab));
    Q_ASSERT(area != NULL);

    return qobject_cast<GroupsConsole*> (area->widget());
}

void SceneEditor::slotGroupValueChanged(quint32 groupID, uchar value)
{
    // Don't modify m_scene contents when doing initialization
    if (m_initFinished == true)
    {
        Q_ASSERT(m_scene != NULL);
        ChannelsGroup *group = m_doc->channelsGroup(groupID);
        if (group == NULL)
            return;
        foreach (SceneValue scv, group->getChannels())
        {
            Fixture *fixture = m_doc->fixture(scv.fxi);
            if (fixture == NULL)
                continue;
            FixtureConsole *fc = fixtureConsole(fixture);
            if (fc == NULL)
                continue;
            fc->setValue(scv.channel, value);
        }
        m_scene->setChannelGroupLevel(groupID, value);
    }
}

/*****************************************************************************
 * Fixture tabs
 *****************************************************************************/

FixtureConsole* SceneEditor::fixtureConsole(Fixture* fixture)
{
    Q_ASSERT(fixture != NULL);

    if (m_consoleList.contains(fixture->id()))
        return m_consoleList[fixture->id()];

    return NULL;
}

void SceneEditor::addFixtureTab(Fixture* fixture, quint32 channel)
{
    Q_ASSERT(fixture != NULL);

    /* Put the console inside a scroll area */
    QScrollArea* scrollArea = new QScrollArea(m_tab);
    scrollArea->setWidgetResizable(true);

    FixtureConsole* console = new FixtureConsole(scrollArea, m_doc);
    console->setFixture(fixture->id());
    m_consoleList[fixture->id()] = console;
    scrollArea->setWidget(console);
    int tIdx = m_tab->addTab(scrollArea, fixture->name());
    m_tab->setTabToolTip(tIdx, fixture->name());

    /* Start off with all channels disabled */
    console->setChecked(false);

    connect(console, SIGNAL(valueChanged(quint32,quint32,uchar)),
            this, SLOT(slotValueChanged(quint32,quint32,uchar)));
    connect(console, SIGNAL(checked(quint32,quint32,bool)),
            this, SLOT(slotChecked(quint32,quint32,bool)));

    if (channel != QLCChannel::invalid())
        console->setChecked(true, channel);
}

void SceneEditor::removeFixtureTab(quint32 fixtureID)
{
    /* Start searching from the first fixture tab */
    for (int i = m_fixtureFirstTabIndex; i < m_tab->count(); i++)
    {
        FixtureConsole* fc = fixtureConsoleTab(i);
        if (fc != NULL && fc->fixture() == fixtureID)
        {
            /* First remove the tab because otherwise Qt might
               remove two tabs -- undocumented feature, which
               might be intended or it might not. */
            QScrollArea* area = qobject_cast<QScrollArea*> (m_tab->widget(i));
            Q_ASSERT(area != NULL);
            m_tab->removeTab(i);
            m_consoleList.take(fixtureID);
            delete area; // Deletes also FixtureConsole
            break;
        }
    }
}

FixtureConsole* SceneEditor::fixtureConsoleTab(int tab)
{
    if (tab >= m_tab->count() || tab <= 0)
        return NULL;

    QScrollArea* area = qobject_cast<QScrollArea*> (m_tab->widget(tab));
    Q_ASSERT(area != NULL);

    return qobject_cast<FixtureConsole*> (area->widget());
}

void SceneEditor::setTabChannelState(bool status, Fixture *fixture, quint32 channel)
{
    Q_ASSERT(fixture != NULL);

    if (channel == QLCChannel::invalid())
        return;

    if (m_consoleList.contains(fixture->id()))
        m_consoleList[fixture->id()]->setChecked(status, channel);
}

void SceneEditor::slotValueChanged(quint32 fxi, quint32 channel, uchar value)
{
    // Don't modify m_scene contents when doing initialization
    if (m_initFinished == true)
    {
        Q_ASSERT(m_scene != NULL);

        if (m_doc->mode() == Doc::Operate)
            m_scene->setValue(SceneValue(fxi, channel, value), m_blindAction->isChecked(), false);
        else
            m_scene->setValue(SceneValue(fxi, channel, value), m_blindAction->isChecked(), true);
        emit fixtureValueChanged(SceneValue(fxi, channel, value), true);
    }

    if (m_source != NULL)
        m_source->set(fxi, channel, value);
}

void SceneEditor::slotChecked(quint32 fxi, quint32 channel, bool state)
{
    // Don't modify m_scene contents when doing initialization
    if (m_initFinished == true)
    {
        // When a channel is enabled, its current value is emitted with valueChanged().
        // So, state == true case doesn't need to be handled here.
        Q_ASSERT(m_scene != NULL);
        if (state == false)
        {
            m_scene->unsetValue(fxi, channel);
            if (m_source != NULL)
            {
                m_source->unset(fxi, channel);
                emit fixtureValueChanged(SceneValue(fxi, channel, 0), false);
            }
        }
    }
}

void SceneEditor::slotGoToNextTab()
{
    m_currentTab++;
    if (m_currentTab == m_tab->count())
        m_currentTab = 0;
    m_tab->setCurrentIndex(m_currentTab);
}

void SceneEditor::slotGoToPreviousTab()
{
    if (m_currentTab == 0)
        m_currentTab = m_tab->count() - 1;
    else
        m_currentTab--;
    m_tab->setCurrentIndex(m_currentTab);
}

void
SceneEditor::onInput(quint32 univ, quint32 chan, uchar val, const QString &key)
{
    Q_UNUSED(key);
    static const QString activeChannel = QString(
                                                 "QSlider::groove:vertical { background: #f0f0f0; width: 32px; } "
                                                 "QSlider::handle:vertical { "
                                                 "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #000, stop:0.55 #888, stop:1 #999);"
                                                 "border: 1px solid #5c5c5c;"
                                                 "border-radius: 4px; margin: 0 -1px; height: 20px; }"
                                                 "QSlider::handle:vertical:hover {"
                                                 "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #eee, stop:0.45 #999, stop:0.50 #ff0000, stop:0.55 #999, stop:1 #ccc);"
                                                 "border: 1px solid #000; }"
                                                 "QSlider::add-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #78d, stop: 1 #97CDEC );"
                                                 "border: 1px solid #5288A7; margin: 0 13px; }"
                                                 "QSlider::sub-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #888, stop: 1 #ddd );"
                                                 "border: 1px solid #8E8A86; margin: 0 13px; }"
                                                 "QSlider::handle:vertical:disabled { background: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #444, stop:0.55 #888, stop:1 #999);"
                                                 "border: 1px solid #666; }"
                                                 );
    static const QString inactiveChannel = QString(
                                                   "QSlider::groove:vertical { background: transparent; width: 32px; } "
                                                   "QSlider::handle:vertical { "
                                                   "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #000, stop:0.55 #888, stop:1 #999);"
                                                   "border: 1px solid #5c5c5c;"
                                                   "border-radius: 4px; margin: 0 -1px; height: 20px; }"
                                                   "QSlider::handle:vertical:hover {"
                                                   "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #eee, stop:0.45 #999, stop:0.50 #ff0000, stop:0.55 #999, stop:1 #ccc);"
                                                   "border: 1px solid #000; }"
                                                   "QSlider::add-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #78d, stop: 1 #97CDEC );"
                                                   "border: 1px solid #5288A7; margin: 0 13px; }"
                                                   "QSlider::sub-page:vertical { background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #888, stop: 1 #ddd );"
                                                   "border: 1px solid #8E8A86; margin: 0 13px; }"
                                                   "QSlider::handle:vertical:disabled { background: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ddd, stop:0.45 #888, stop:0.50 #444, stop:0.55 #888, stop:1 #999);"
                                                   "border: 1px solid #666; }"
                                                   );
    if(m_doc->mode() == Doc::Operate) return;
    if(!hw_enable) return;
    if (m_scene->uiStateValue(UI_STATE_TAB_MODE).toInt() != UI_STATE_TABBED_FIXTURES) return;
    if((int)val > 0){
        switch(tabs.indexOf(chan)){
        case 0: // left
            slotGoToPreviousTab();
            m_offset = 0; 
            if(m_currentTab < m_fixtureFirstTabIndex) return;
            for(int i = 0; i < 8; i++)
                m_doc->inputOutputMap()->sendFeedBack(univ, channelBanks.at(i), i == 0 ? 0xFF : 0, 0);
            for(int i = 0; i < 8; i++)
                m_doc->inputOutputMap()->sendFeedBack(univ, armButton.at(i), fixtureConsoleTab(m_currentTab)->isChecked(m_offset + i) ? 0xFF : 0, 0);
            for(unsigned int i = 0; i < 64; i++)
                fixtureConsoleTab(m_currentTab)->setSliderStylesheet(i, (i < m_offset || i > (m_offset + 7)) ? inactiveChannel : activeChannel);
            return;
            break;
        case 1: // right
            slotGoToNextTab();
            m_offset = 0; 
            if(m_currentTab < m_fixtureFirstTabIndex) return;
            for(int i = 0; i < 8; i++)
                m_doc->inputOutputMap()->sendFeedBack(univ, channelBanks.at(i), i == 0 ? 0xFF : 0, 0);
            for(int i = 0; i < 8; i++)
                m_doc->inputOutputMap()->sendFeedBack(univ, armButton.at(i), fixtureConsoleTab(m_currentTab)->isChecked(m_offset + i) ? 0xFF : 0, 0);
            for(unsigned int i = 0; i < 64; i++)
                fixtureConsoleTab(m_currentTab)->setSliderStylesheet(i, (i < m_offset || i > (m_offset + 7)) ? inactiveChannel : activeChannel);
            return;
            break;
        }
    }
    if(m_currentTab < m_fixtureFirstTabIndex) return;
    int index;
    if((int)val > 0 && (index = channelBanks.indexOf(chan)) >= 0){
        m_offset = 8 * index;
        for(int i = 0; i < 8; i++)
            m_doc->inputOutputMap()->sendFeedBack(univ, channelBanks.at(i), channelBanks.at(i) == chan ? 0xFF : 0, 0);
        for(int i = 0; i < 8; i++)
            m_doc->inputOutputMap()->sendFeedBack(univ, armButton.at(i), fixtureConsoleTab(m_currentTab)->isChecked(m_offset + i) ? 0xFF : 0, 0);
        for(unsigned int i = 0; i < 64; i++)
            fixtureConsoleTab(m_currentTab)->setSliderStylesheet(i, (i < m_offset || i > (m_offset + 7)) ? inactiveChannel : activeChannel);
        return;
    }
    if((index = faders.indexOf(chan)) >=0){
        fixtureConsoleTab(m_currentTab)->setValue(index + m_offset, val);
        return;
    }
    if((int)val > 0 && (index = armButton.indexOf(chan)) >= 0){
        bool check = fixtureConsoleTab(m_currentTab)->check(m_offset + index);
        m_doc->inputOutputMap()->sendFeedBack(univ, chan, check ? 0xFF : 0, 0);
        return;
    }
}

void SceneEditor::slotHardwareAction()
{
    QSettings settings;
    QDialog *d = new QDialog(this);
    d->setWindowTitle(tr("Configure hardware editing"));
    QVBoxLayout *mainLayout = new QVBoxLayout();
    d->setLayout(mainLayout);

    QTabWidget *page1 = new QTabWidget();
    page1->setTabPosition(QTabWidget::West);
    QWidget *page1_1 = new QWidget();
    QGridLayout *grid1_1 = new QGridLayout(page1_1);
    page1->addTab(page1_1, tr("1-2"));

    QLabel *lcb1 = new QLabel(tr("Channel Bank 1"));
    grid1_1->addWidget(lcb1, 0, 0);
    InputSelectionWidget *cb1 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb1(new QLCInputSource(0, settings.value("sceneEdit/channelBank1",-1).toUInt()));
    cb1->setKeyInputVisibility(false);
    cb1->setInputSource(pcb1);
    grid1_1->addWidget(cb1, 0, 1);

    QLabel *lcb2 = new QLabel(tr("Channel Bank 2"));
    grid1_1->addWidget(lcb2, 1, 0);
    InputSelectionWidget *cb2 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb2(new QLCInputSource(0, settings.value("sceneEdit/channelBank2",-1).toUInt()));
    cb2->setKeyInputVisibility(false);
    cb2->setInputSource(pcb2);
    grid1_1->addWidget(cb2, 1, 1);

    QWidget *page1_2 = new QWidget();
    QGridLayout *grid1_2 = new QGridLayout(page1_2);
    page1->addTab(page1_2, tr("3-4"));

    QLabel *lcb3 = new QLabel(tr("Channel Bank 3"));
    grid1_2->addWidget(lcb3, 0, 0);
    InputSelectionWidget *cb3 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb3(new QLCInputSource(0, settings.value("sceneEdit/channelBank3",-1).toUInt()));
    cb3->setKeyInputVisibility(false);
    cb3->setInputSource(pcb3);
    grid1_2->addWidget(cb3, 0, 1);

    QLabel *lcb4 = new QLabel(tr("Channel Bank 4"));
    grid1_2->addWidget(lcb4, 1, 0);
    InputSelectionWidget *cb4 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb4(new QLCInputSource(0, settings.value("sceneEdit/channelBank4",-1).toUInt()));
    cb4->setKeyInputVisibility(false);
    cb4->setInputSource(pcb4);
    grid1_2->addWidget(cb4, 1, 1);

    QWidget *page1_3 = new QWidget();
    QGridLayout *grid1_3 = new QGridLayout(page1_3);
    page1->addTab(page1_3, tr("5-6"));

    QLabel *lcb5 = new QLabel(tr("Channel Bank 5"));
    grid1_3->addWidget(lcb5, 0, 0);
    InputSelectionWidget *cb5 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb5(new QLCInputSource(0, settings.value("sceneEdit/channelBank5",-1).toUInt()));
    cb5->setKeyInputVisibility(false);
    cb5->setInputSource(pcb5);
    grid1_3->addWidget(cb5, 0, 1);

    QLabel *lcb6 = new QLabel(tr("Channel Bank 6"));
    grid1_3->addWidget(lcb6, 1, 0);
    InputSelectionWidget *cb6 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb6(new QLCInputSource(0, settings.value("sceneEdit/channelBank6",-1).toUInt()));
    cb6->setKeyInputVisibility(false);
    cb6->setInputSource(pcb6);
    grid1_3->addWidget(cb6, 1, 1);

    QWidget *page1_4 = new QWidget();
    QGridLayout *grid1_4 = new QGridLayout(page1_4);
    page1->addTab(page1_4, tr("7-8"));

    QLabel *lcb7 = new QLabel(tr("Channel Bank 7"));
    grid1_4->addWidget(lcb7, 0, 0);
    InputSelectionWidget *cb7 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb7(new QLCInputSource(0, settings.value("sceneEdit/channelBank7",-1).toUInt()));
    cb7->setKeyInputVisibility(false);
    cb7->setInputSource(pcb7);
    grid1_4->addWidget(cb7, 0, 1);

    QLabel *lcb8 = new QLabel(tr("Channel Bank 8"));
    grid1_4->addWidget(lcb8, 1, 0);
    InputSelectionWidget *cb8 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pcb8(new QLCInputSource(0, settings.value("sceneEdit/channelBank8",-1).toUInt()));
    cb8->setKeyInputVisibility(false);
    cb8->setInputSource(pcb8);
    grid1_4->addWidget(cb8, 1, 1);

    QTabWidget *page2 = new QTabWidget();
    page2->setTabPosition(QTabWidget::West);
    QWidget *page2_1 = new QWidget();
    QGridLayout *grid2_1 = new QGridLayout(page2_1);
    page2->addTab(page2_1, tr("1-2"));
    
    QLabel *lab1 = new QLabel(tr("Arm Button 1"));
    grid2_1->addWidget(lab1, 0, 0);
    InputSelectionWidget *ab1 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab1(new QLCInputSource(0, settings.value("sceneEdit/armButton1",-1).toUInt()));
    ab1->setKeyInputVisibility(false);
    ab1->setInputSource(pab1);
    grid2_1->addWidget(ab1, 0, 1);
    
    QLabel *lab2 = new QLabel(tr("Arm Button 2"));
    grid2_1->addWidget(lab2, 1, 0);
    InputSelectionWidget *ab2 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab2(new QLCInputSource(0, settings.value("sceneEdit/armButton2",-1).toUInt()));
    ab2->setKeyInputVisibility(false);
    ab2->setInputSource(pab1);
    grid2_1->addWidget(ab2, 1, 1);

    QWidget *page2_2 = new QWidget();
    QGridLayout *grid2_2 = new QGridLayout(page2_2);
    page2->addTab(page2_2, tr("3-4"));

    QLabel *lab3 = new QLabel(tr("Arm Button 3"));
    grid2_2->addWidget(lab3, 0, 0);
    InputSelectionWidget *ab3 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab3(new QLCInputSource(0, settings.value("sceneEdit/armButton3",-1).toUInt()));
    ab3->setKeyInputVisibility(false);
    ab3->setInputSource(pab3);
    grid2_2->addWidget(ab3, 0, 1);

    QLabel *lab4 = new QLabel(tr("Arm Button 4"));
    grid2_2->addWidget(lab4, 1, 0);
    InputSelectionWidget *ab4 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab4(new QLCInputSource(0, settings.value("sceneEdit/armButton4",-1).toUInt()));
    ab4->setKeyInputVisibility(false);
    ab4->setInputSource(pab4);
    grid2_2->addWidget(ab4, 1, 1);

    QWidget *page2_3 = new QWidget();
    QGridLayout *grid2_3 = new QGridLayout(page2_3);
    page2->addTab(page2_3, tr("5-6"));

    QLabel *lab5 = new QLabel(tr("Arm Button 5"));
    grid2_3->addWidget(lab5, 0, 0);
    InputSelectionWidget *ab5 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab5(new QLCInputSource(0, settings.value("sceneEdit/armButton5",-1).toUInt()));
    ab5->setKeyInputVisibility(false);
    ab5->setInputSource(pab5);
    grid2_3->addWidget(ab5, 0, 1);

    QLabel *lab6 = new QLabel(tr("Arm Button 6"));
    grid2_3->addWidget(lab6, 1, 0);
    InputSelectionWidget *ab6 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab6(new QLCInputSource(0, settings.value("sceneEdit/armButton6",-1).toUInt()));
    ab6->setKeyInputVisibility(false);
    ab6->setInputSource(pab6);
    grid2_3->addWidget(ab6, 1, 1);

    QWidget *page2_4 = new QWidget();
    QGridLayout *grid2_4 = new QGridLayout(page2_4);
    page2->addTab(page2_4, tr("7-8"));

    QLabel *lab7 = new QLabel(tr("Arm Button 7"));
    grid2_4->addWidget(lab7, 0, 0);
    InputSelectionWidget *ab7 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab7(new QLCInputSource(0, settings.value("sceneEdit/armButton7",-1).toUInt()));
    ab7->setKeyInputVisibility(false);
    ab7->setInputSource(pab7);
    grid2_4->addWidget(ab7, 0, 1);

    QLabel *lab8 = new QLabel(tr("Arm Button 8"));
    grid2_4->addWidget(lab8, 1, 0);
    InputSelectionWidget *ab8 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pab8(new QLCInputSource(0, settings.value("sceneEdit/armButton8",-1).toUInt()));
    ab8->setKeyInputVisibility(false);
    ab8->setInputSource(pab8);
    grid2_4->addWidget(ab8, 1, 1);

    QTabWidget *page3 = new QTabWidget();
    page3->setTabPosition(QTabWidget::West);
    QWidget *page3_1 = new QWidget();
    QGridLayout *grid3_1 = new QGridLayout(page3_1);
    page3->addTab(page3_1, tr("1-2"));

    QLabel *lfd1 = new QLabel(tr("Fader 1"));
    grid3_1->addWidget(lfd1, 0, 0);
    InputSelectionWidget *fd1 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd1(new QLCInputSource(0, settings.value("sceneEdit/fader1",-1).toUInt()));
    fd1->setKeyInputVisibility(false);
    fd1->setInputSource(pfd1);
    grid3_1->addWidget(fd1, 0, 1);

    QLabel *lfd2 = new QLabel(tr("Fader 2"));
    grid3_1->addWidget(lfd2, 1, 0);
    InputSelectionWidget *fd2 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd2(new QLCInputSource(0, settings.value("sceneEdit/fader2",-1).toUInt()));
    fd2->setKeyInputVisibility(false);
    fd2->setInputSource(pfd2);
    grid3_1->addWidget(fd2, 1, 1);

    QWidget *page3_2 = new QWidget();
    QGridLayout *grid3_2 = new QGridLayout(page3_2);
    page3->addTab(page3_2, tr("3-4"));

    QLabel *lfd3 = new QLabel(tr("Fader 3"));
    grid3_2->addWidget(lfd3, 0, 0);
    InputSelectionWidget *fd3 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd3(new QLCInputSource(0, settings.value("sceneEdit/fader3",-1).toUInt()));
    fd3->setKeyInputVisibility(false);
    fd3->setInputSource(pfd3);
    grid3_2->addWidget(fd3, 0, 1);

    QLabel *lfd4 = new QLabel(tr("Fader 4"));
    grid3_2->addWidget(lfd4, 1, 0);
    InputSelectionWidget *fd4 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd4(new QLCInputSource(0, settings.value("sceneEdit/fader4",-1).toUInt()));
    fd4->setKeyInputVisibility(false);
    fd4->setInputSource(pfd4);
    grid3_2->addWidget(fd4, 1, 1);

    QWidget *page3_3 = new QWidget();
    QGridLayout *grid3_3 = new QGridLayout(page3_3);
    page3->addTab(page3_3, tr("5-6"));

    QLabel *lfd5 = new QLabel(tr("Fader 5"));
    grid3_3->addWidget(lfd5, 0, 0);
    InputSelectionWidget *fd5 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd5(new QLCInputSource(0, settings.value("sceneEdit/fader5",-1).toUInt()));
    fd5->setKeyInputVisibility(false);
    fd5->setInputSource(pfd5);
    grid3_3->addWidget(fd5, 0, 1);

    QLabel *lfd6 = new QLabel(tr("Fader 6"));
    grid3_3->addWidget(lfd6, 1, 0);
    InputSelectionWidget *fd6 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd6(new QLCInputSource(0, settings.value("sceneEdit/fader6",-1).toUInt()));
    fd6->setKeyInputVisibility(false);
    fd6->setInputSource(pfd6);
    grid3_3->addWidget(fd6, 1, 1);

    QWidget *page3_4 = new QWidget();
    QGridLayout *grid3_4 = new QGridLayout(page3_4);
    page3->addTab(page3_4, tr("7-8"));

    QLabel *lfd7 = new QLabel(tr("Fader 7"));
    grid3_4->addWidget(lfd7, 0, 0);
    InputSelectionWidget *fd7 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd7(new QLCInputSource(0, settings.value("sceneEdit/fader7",-1).toUInt()));
    fd7->setKeyInputVisibility(false);
    fd7->setInputSource(pfd7);
    grid3_4->addWidget(fd7, 0, 1);

    QLabel *lfd8 = new QLabel(tr("Fader 8"));
    grid3_4->addWidget(lfd8, 1, 0);
    InputSelectionWidget *fd8 = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pfd8(new QLCInputSource(0, settings.value("sceneEdit/fader8",-1).toUInt()));
    fd8->setKeyInputVisibility(false);
    fd8->setInputSource(pfd8);
    grid3_4->addWidget(fd8, 1, 1);

    QWidget *page4 = new QWidget();
    QGridLayout *grid4 = new QGridLayout(page4);

    QLabel *lbLeft = new QLabel(tr("Previous Fixture"));
    grid4->addWidget(lbLeft, 0, 0);
    InputSelectionWidget *iLeft = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pLeft(new QLCInputSource(0, settings.value("sceneEdit/tabPrev",-1).toUInt()));
    iLeft->setKeyInputVisibility(false);
    iLeft->setInputSource(pLeft);
    grid4->addWidget(iLeft, 0, 1);

    QLabel *lbRight = new QLabel(tr("Next Fixture"));
    grid4->addWidget(lbRight, 1, 0);
    InputSelectionWidget *iRight = new InputSelectionWidget(m_doc, d);
    QSharedPointer<QLCInputSource> pRight(new QLCInputSource(0, settings.value("sceneEdit/tabNext",-1).toUInt()));
    iRight->setKeyInputVisibility(false);
    iRight->setInputSource(pRight);
    grid4->addWidget(iRight, 1, 1);
    
    QHBoxLayout *boxLabel = new QHBoxLayout();
    QLabel *lbHw = new QLabel(tr("Enable Hardware"));
    boxLabel->addWidget(lbHw);
    QCheckBox *cHw = new QCheckBox();
    cHw->setCheckState(settings.value("sceneEdit/hardwareEnable",false).toBool() ? Qt::Checked : Qt::Unchecked);
    boxLabel->addWidget(cHw);
    mainLayout->addLayout(boxLabel);

    QHBoxLayout *boxBtn = new QHBoxLayout();
    QPushButton *bSave = new QPushButton(tr("Save"));
    boxBtn->addWidget(bSave);
    connect(bSave, SIGNAL(released()), d, SLOT(accept()));
    QPushButton *bCancel = new QPushButton(tr("Cancel"));
    boxBtn->addWidget(bCancel);
    connect(bCancel, SIGNAL(released()), d, SLOT(reject()));

    QTabWidget *tw = new QTabWidget();
    tw->addTab(page1, tr("Channel Banks"));
    tw->addTab(page2, tr("Arm Buttons"));
    tw->addTab(page3, tr("Faders"));
    tw->addTab(page4, tr("Other"));
    // d->adjustSize();
    mainLayout->addWidget(tw);
    mainLayout->addLayout(boxBtn);
    if(d->exec() == QDialog::Accepted){
        settings.setValue("sceneEdit/channelBank1", cb1->inputSource()->channel());
        settings.setValue("sceneEdit/channelBank2", cb2->inputSource()->channel());
        settings.setValue("sceneEdit/channelBank3", cb3->inputSource()->channel());
        settings.setValue("sceneEdit/channelBank4", cb4->inputSource()->channel());
        settings.setValue("sceneEdit/channelBank5", cb5->inputSource()->channel());
        settings.setValue("sceneEdit/channelBank6", cb6->inputSource()->channel());
        settings.setValue("sceneEdit/channelBank7", cb7->inputSource()->channel());
        settings.setValue("sceneEdit/channelBank8", cb8->inputSource()->channel());
        settings.setValue("sceneEdit/armButton1", ab1->inputSource()->channel());
        settings.setValue("sceneEdit/armButton2", ab2->inputSource()->channel());
        settings.setValue("sceneEdit/armButton3", ab3->inputSource()->channel());
        settings.setValue("sceneEdit/armButton4", ab4->inputSource()->channel());
        settings.setValue("sceneEdit/armButton5", ab5->inputSource()->channel());
        settings.setValue("sceneEdit/armButton6", ab6->inputSource()->channel());
        settings.setValue("sceneEdit/armButton7", ab7->inputSource()->channel());
        settings.setValue("sceneEdit/armButton8", ab8->inputSource()->channel());
        settings.setValue("sceneEdit/fader1", fd1->inputSource()->channel());
        settings.setValue("sceneEdit/fader2", fd2->inputSource()->channel());
        settings.setValue("sceneEdit/fader3", fd3->inputSource()->channel());
        settings.setValue("sceneEdit/fader4", fd4->inputSource()->channel());
        settings.setValue("sceneEdit/fader5", fd5->inputSource()->channel());
        settings.setValue("sceneEdit/fader6", fd6->inputSource()->channel());
        settings.setValue("sceneEdit/fader7", fd7->inputSource()->channel());
        settings.setValue("sceneEdit/fader8", fd8->inputSource()->channel());
        settings.setValue("sceneEdit/tabPrev", iLeft->inputSource()->channel());
        settings.setValue("sceneEdit/tabNext", iRight->inputSource()->channel());
        settings.setValue("sceneEdit/hardwareEnabled", cHw->isChecked());
        hw_enable = settings.value("sceneEdit/hardwareEnable",false).toBool();
        channelBanks.clear();
        channelBanks.append(settings.value("sceneEdit/channelBank1",QLCInputSource::invalidChannel).toUInt());
        channelBanks.append(settings.value("sceneEdit/channelBank2",QLCInputSource::invalidChannel).toUInt());
        channelBanks.append(settings.value("sceneEdit/channelBank3",QLCInputSource::invalidChannel).toUInt());
        channelBanks.append(settings.value("sceneEdit/channelBank4",QLCInputSource::invalidChannel).toUInt());
        channelBanks.append(settings.value("sceneEdit/channelBank5",QLCInputSource::invalidChannel).toUInt());
        channelBanks.append(settings.value("sceneEdit/channelBank6",QLCInputSource::invalidChannel).toUInt());
        channelBanks.append(settings.value("sceneEdit/channelBank7",QLCInputSource::invalidChannel).toUInt());
        channelBanks.append(settings.value("sceneEdit/channelBank8",QLCInputSource::invalidChannel).toUInt());
        armButton.clear();
        armButton.append(settings.value("sceneEdit/armButton1",QLCInputSource::invalidChannel).toUInt());
        armButton.append(settings.value("sceneEdit/armButton2",QLCInputSource::invalidChannel).toUInt());
        armButton.append(settings.value("sceneEdit/armButton3",QLCInputSource::invalidChannel).toUInt());
        armButton.append(settings.value("sceneEdit/armButton4",QLCInputSource::invalidChannel).toUInt());
        armButton.append(settings.value("sceneEdit/armButton5",QLCInputSource::invalidChannel).toUInt());
        armButton.append(settings.value("sceneEdit/armButton6",QLCInputSource::invalidChannel).toUInt());
        armButton.append(settings.value("sceneEdit/armButton7",QLCInputSource::invalidChannel).toUInt());
        armButton.append(settings.value("sceneEdit/armButton8",QLCInputSource::invalidChannel).toUInt());
        faders.clear();
        faders.append(settings.value("sceneEdit/fader1",QLCInputSource::invalidChannel).toUInt());
        faders.append(settings.value("sceneEdit/fader2",QLCInputSource::invalidChannel).toUInt());
        faders.append(settings.value("sceneEdit/fader3",QLCInputSource::invalidChannel).toUInt());
        faders.append(settings.value("sceneEdit/fader4",QLCInputSource::invalidChannel).toUInt());
        faders.append(settings.value("sceneEdit/fader5",QLCInputSource::invalidChannel).toUInt());
        faders.append(settings.value("sceneEdit/fader6",QLCInputSource::invalidChannel).toUInt());
        faders.append(settings.value("sceneEdit/fader7",QLCInputSource::invalidChannel).toUInt());
        faders.append(settings.value("sceneEdit/fader8",QLCInputSource::invalidChannel).toUInt());
        tabs.clear();
        tabs.append(settings.value("sceneEdit/tabPrev",QLCInputSource::invalidChannel).toUInt());
        tabs.append(settings.value("sceneEdit/tabNext",QLCInputSource::invalidChannel).toUInt());
        if(channelBanks.count() != 8 || armButton.count() != 8 || faders.count() != 8 || tabs.count() != 2)
            hw_enable = false;
        settings.sync();
    }
}
