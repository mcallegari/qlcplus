/*
  Q Light Controller
  inputoutputmanager.cpp

  Copyright (c) Massimo Callegari

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
#include <QTreeWidget>
#include <QHeaderView>
#include <QStringList>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QIcon>

#include "inputoutputpatcheditor.h"
#include "inputoutputmanager.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "apputil.h"
#include "doc.h"

#define KColumnUniverse     0
#define KColumnInput        1
#define KColumnOutput       2
#define KColumnFeedback     3
#define KColumnProfile      4
#define KColumnInputNum     5
#define KColumnOutputNum    6

#define SETTINGS_SPLITTER "inputmanager/splitter"

InputOutputManager* InputOutputManager::s_instance = NULL;

InputOutputManager::InputOutputManager(QWidget* parent, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_editor(NULL)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(doc != NULL);
    
    m_inputMap = doc->inputMap();
    m_outputMap = doc->outputMap();

    /* Create a new layout for this widget */
    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(m_splitter);

    /* Tree */
    m_tree = new QTreeWidget(this);
    m_splitter->addWidget(m_tree);
    m_tree->setRootIsDecorated(false);
    m_tree->setItemsExpandable(false);
    m_tree->setSortingEnabled(false);
    m_tree->setAllColumnsShowFocus(true);

    QWidget* gcontainer = new QWidget(this);
    m_splitter->addWidget(gcontainer);
    gcontainer->setLayout(new QVBoxLayout);
    gcontainer->layout()->setContentsMargins(0, 0, 0, 0);

    QStringList columns;
    columns << tr("Universe") << tr("Input") << tr("Output") << tr("Feedback") << tr("Profile");
    m_tree->setHeaderLabels(columns);

    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotCurrentItemChanged()));

    /* Timer that clears the input data icon after a while */
    m_icon = QIcon(":/input.png");
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));

    /* Listen to input map's input data signals */
    connect(m_inputMap, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));

    /* Listen to plugin configuration changes */
    connect(m_inputMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(updateTree()));

    updateTree();
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
    //slotCurrentItemChanged();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());
}

InputOutputManager::~InputOutputManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());

    s_instance = NULL;
}

InputOutputManager* InputOutputManager::instance()
{
    return s_instance;
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void InputOutputManager::updateTree()
{
    m_tree->clear();
    for (quint32 uni = 0; uni < m_inputMap->universes(); uni++)
        updateItem(new QTreeWidgetItem(m_tree), uni);
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
}

void InputOutputManager::updateItem(QTreeWidgetItem* item, quint32 universe)
{
    Q_ASSERT(item != NULL);

    InputPatch* ip = m_inputMap->patch(universe);
    OutputPatch* op = m_outputMap->patch(universe);
    OutputPatch* fp = m_outputMap->feedbackPatch(universe);
    Q_ASSERT(ip != NULL);

    item->setText(KColumnUniverse, QString::number(universe + 1));
    item->setText(KColumnInput, QString("[%1] %2").arg(ip->pluginName()).arg(ip->inputName()));
    item->setText(KColumnOutput, QString("[%1] %2").arg(op->pluginName()).arg(op->outputName()));
    item->setText(KColumnFeedback, QString("[%1] %2").arg(fp->pluginName()).arg(fp->outputName()));
    item->setText(KColumnProfile, ip->profileName());
    item->setText(KColumnInputNum, QString::number(ip->input() + 1));
    item->setText(KColumnOutputNum, QString::number(op->output() + 1));

    m_tree->resizeColumnToContents(KColumnUniverse);
    m_tree->resizeColumnToContents(KColumnInput);
    m_tree->resizeColumnToContents(KColumnOutput);
    m_tree->resizeColumnToContents(KColumnProfile);
}

void InputOutputManager::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);

    QTreeWidgetItem* item = m_tree->topLevelItem(universe);
    if (item == NULL)
        return;

    /* Show an icon on a universe row that received input data */
    item->setIcon(KColumnUniverse, m_icon);

    /* Restart the timer */
    m_timer->start(250);
}

void InputOutputManager::slotTimerTimeout()
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        (*it)->setIcon(KColumnUniverse, QIcon());
        ++it;
    }
}

void InputOutputManager::slotCurrentItemChanged()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
    {
        m_tree->setCurrentItem(m_tree->topLevelItem(0));
        item = m_tree->currentItem();
    }
    //Q_ASSERT(item != NULL);
    if (item == NULL)
        return;

    if (m_editor != NULL)
    {
        m_splitter->widget(1)->layout()->removeWidget(m_editor);
        m_editor->deleteLater();
        m_editor = NULL;
        //delete currentEditor();
    }

    quint32 universe = item->text(KColumnUniverse).toInt() - 1;
    m_editor = new InputOutputPatchEditor(this, universe, m_inputMap, m_outputMap);
    m_splitter->widget(1)->layout()->addWidget(m_editor);
    connect(m_editor, SIGNAL(mappingChanged()), this, SLOT(slotMappingChanged()));
    connect(m_editor, SIGNAL(audioInputDeviceChanged()), this, SLOT(slotAudioInputChanged()));
    m_editor->show();
}

void InputOutputManager::slotMappingChanged()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        uint universe = item->text(KColumnUniverse).toUInt() - 1;
        updateItem(item, universe);
        m_doc->outputMap()->saveDefaults();
        m_doc->inputMap()->saveDefaults();
    }
}

void InputOutputManager::slotAudioInputChanged()
{
    m_doc->destroyAudioCapture();
}



