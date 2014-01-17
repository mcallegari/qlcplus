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

#include <QListWidgetItem>
#include <QListWidget>
#include <QHeaderView>
#include <QStringList>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QLineEdit>
#include <QToolBar>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QLabel>
#include <QIcon>

#include "inputoutputpatcheditor.h"
#include "universeitemwidget.h"
#include "inputoutputmanager.h"
#include "inputoutputmap.h"
#include "outputpatch.h"
#include "inputpatch.h"
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
    , m_toolbar(NULL)
    , m_addUniverseAction(NULL)
    , m_deleteUniverseAction(NULL)
    , m_uniNameEdit(NULL)
    , m_editor(NULL)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(doc != NULL);
    
    m_ioMap = doc->inputOutputMap();

    /* Create a new layout for this widget */
    new QVBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(m_splitter);

    m_addUniverseAction = new QAction(QIcon(":/edit_add.png"),
                                   tr("Add U&niverse"), this);
    m_addUniverseAction->setShortcut(QKeySequence("CTRL+N"));
    connect(m_addUniverseAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAddUniverse()));

    m_deleteUniverseAction = new QAction(QIcon(":/edit_remove.png"),
                                   tr("&Delete Universe"), this);
    m_deleteUniverseAction->setShortcut(QKeySequence("CTRL+D"));
    connect(m_deleteUniverseAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDeleteUniverse()));

    QWidget* ucontainer = new QWidget(this);
    m_splitter->addWidget(ucontainer);
    ucontainer->setLayout(new QVBoxLayout);
    ucontainer->layout()->setContentsMargins(0, 0, 0, 0);

    // Add a toolbar to the dock area
    m_toolbar = new QToolBar("Input Output Manager", this);
    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(32, 32));
    m_toolbar->addAction(m_addUniverseAction);
    m_toolbar->addAction(m_deleteUniverseAction);
    m_toolbar->addSeparator();

    QLabel *uniLabel = new QLabel(tr("Universe name:"));
    m_uniNameEdit = new QLineEdit(this);
    QFont font = QApplication::font();
    //font.setBold(true);
    font.setPixelSize(18);
    uniLabel->setFont(font);
    m_uniNameEdit->setFont(font);
    m_toolbar->addWidget(uniLabel);
    m_toolbar->addWidget(m_uniNameEdit);

    m_splitter->widget(0)->layout()->addWidget(m_toolbar);

    connect(m_uniNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotUniverseNameChanged(QString)));

    /* Universes list */
    m_list = new QListWidget(this);
    m_list->setItemDelegate(new UniverseItemWidget(m_list));
    m_splitter->widget(0)->layout()->addWidget(m_list);

    QWidget* gcontainer = new QWidget(this);
    m_splitter->addWidget(gcontainer);
    gcontainer->setLayout(new QVBoxLayout);
    gcontainer->layout()->setContentsMargins(0, 0, 0, 0);

    connect(m_list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(slotCurrentItemChanged()));

    /* Timer that clears the input data icon after a while */
    m_icon = QIcon(":/input.png");
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));

    /* Listen to input map's input data signals */
    connect(m_ioMap, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));

    /* Listen to plugin configuration changes */
    connect(m_ioMap, SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(updateList()));

    updateList();
    m_list->setCurrentItem(m_list->item(0));

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

void InputOutputManager::updateList()
{
    m_list->clear();
    for (quint32 uni = 0; uni < m_ioMap->universes(); uni++)
        updateItem(new QListWidgetItem(m_list), uni);
    m_list->setCurrentItem(m_list->item(0));
    m_uniNameEdit->setText(m_list->item(0)->data(Qt::DisplayRole).toString());

    if (m_ioMap->universes() == 4)
        m_deleteUniverseAction->setEnabled(false);
    else
        m_deleteUniverseAction->setEnabled(true);
}

void InputOutputManager::updateItem(QListWidgetItem* item, quint32 universe)
{
    Q_ASSERT(item != NULL);

    InputPatch* ip = m_ioMap->inputPatch(universe);
    OutputPatch* op = m_ioMap->outputPatch(universe);
    OutputPatch* fp = m_ioMap->feedbackPatch(universe);

    QString uniName = m_ioMap->getUniverseName(universe);
    if (uniName.isEmpty())
    {
        QString defUniName = tr("Universe %1").arg(universe + 1);
        m_ioMap->setUniverseName(universe, defUniName);
        item->setData(Qt::DisplayRole, defUniName);
    }
    else
        item->setData(Qt::DisplayRole, uniName);
    item->setSizeHint(QSize(m_list->width(), 50));
    item->setData(Qt::UserRole, universe);
    if (ip != NULL)
    {
        item->setData(Qt::UserRole + 1, ip->inputName());
        item->setData(Qt::UserRole + 2, ip->profileName());
    }
    else
    {
        item->setData(Qt::UserRole + 1, KInputNone);
        item->setData(Qt::UserRole + 2, KInputNone);
    }
    if (op != NULL)
        item->setData(Qt::UserRole + 3, op->outputName());
    else
        item->setData(Qt::UserRole + 3, KOutputNone);
    if (fp != NULL)
        item->setData(Qt::UserRole + 4, fp->outputName());
    else
        item->setData(Qt::UserRole + 4, KOutputNone);
}

void InputOutputManager::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);

    // If the manager is not visible, don't even waste CPU
    if (isVisible() == false)
        return;

    QListWidgetItem *item = m_list->item(universe);
    if (item == NULL)
        return;

    /* Show an icon on a universe row that received input data */
    item->setData(Qt::DecorationRole, m_icon);

    /* Restart the timer */
    m_timer->start(300);
}

void InputOutputManager::slotTimerTimeout()
{
    for (int i = 0; i < m_list->count(); i++)
    {
        QListWidgetItem *item = m_list->item(i);
        item->setData(Qt::DecorationRole, QIcon());
    }
}

void InputOutputManager::slotCurrentItemChanged()
{
    QListWidgetItem* item = m_list->currentItem();
    if (item == NULL)
    {
        m_list->setCurrentItem(m_list->item(0));
        item = m_list->currentItem();
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

    quint32 universe = item->data(Qt::UserRole).toInt();
    m_editor = new InputOutputPatchEditor(this, universe, m_ioMap);
    m_splitter->widget(1)->layout()->addWidget(m_editor);
    connect(m_editor, SIGNAL(mappingChanged()), this, SLOT(slotMappingChanged()));
    connect(m_editor, SIGNAL(audioInputDeviceChanged()), this, SLOT(slotAudioInputChanged()));
    m_editor->show();
    m_uniNameEdit->setText(item->data(Qt::DisplayRole).toString());
}

void InputOutputManager::slotMappingChanged()
{
    QListWidgetItem* item = m_list->currentItem();
    if (item != NULL)
    {
        uint universe = item->data(Qt::UserRole).toInt();
        updateItem(item, universe);
        m_doc->inputOutputMap()->saveDefaults();
    }
}

void InputOutputManager::slotAudioInputChanged()
{
    m_doc->destroyAudioCapture();
}

void InputOutputManager::slotAddUniverse()
{
    m_ioMap->addUniverse();
    updateList();
}

void InputOutputManager::slotDeleteUniverse()
{
    m_ioMap->removeUniverse();
    updateList();
}

void InputOutputManager::slotUniverseNameChanged(QString name)
{
    QListWidgetItem *currItem = m_list->currentItem();
    int uniIdx = m_list->currentRow();
    if (name.isEmpty())
        name = tr("Universe %1").arg(uniIdx + 1);
    m_ioMap->setUniverseName(uniIdx, name);
    currItem->setData(Qt::DisplayRole, name);
}



