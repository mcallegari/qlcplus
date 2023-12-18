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
#include <QCheckBox>
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
    , m_uniPassthroughCheck(NULL)
    , m_editor(NULL)
    , m_editorUniverse(UINT_MAX)
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

    m_uniPassthroughCheck = new QCheckBox(tr("Passthrough"), this);
    m_uniPassthroughCheck->setLayoutDirection(Qt::RightToLeft);
    m_uniPassthroughCheck->setFont(font);
    m_toolbar->addWidget(m_uniPassthroughCheck);

    m_splitter->widget(0)->layout()->addWidget(m_toolbar);

    connect(m_uniNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotUniverseNameChanged(QString)));

    connect(m_uniPassthroughCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotPassthroughChanged(bool)));

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
    connect(m_ioMap, SIGNAL(pluginConfigurationChanged(const QString&, bool)),
            this, SLOT(updateList()));

    connect(m_ioMap, SIGNAL(universeAdded(quint32)),
            this, SLOT(slotUniverseAdded(quint32)));

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
    m_list->blockSignals(true);
    m_list->clear();
    for (quint32 uni = 0; uni < m_ioMap->universesCount(); uni++)
        updateItem(new QListWidgetItem(m_list), uni);
    m_list->blockSignals(false);

    if (m_ioMap->universesCount() == 0)
    {
        if (m_editor != NULL)
        {
            m_splitter->widget(1)->layout()->removeWidget(m_editor);
            m_editor->deleteLater();
            m_editor = NULL;
        }
        m_deleteUniverseAction->setEnabled(false);
        m_uniNameEdit->setText("");
        m_uniNameEdit->setEnabled(false);
    }
    else
    {
        m_list->setCurrentItem(m_list->item(0));
        m_uniNameEdit->setEnabled(true);
        m_uniNameEdit->setText(m_ioMap->getUniverseNameByIndex(0));
        m_uniPassthroughCheck->setChecked(m_ioMap->getUniversePassthrough(0));
    }
}

void InputOutputManager::updateItem(QListWidgetItem* item, quint32 universe)
{
    Q_ASSERT(item != NULL);

    InputPatch* ip = m_ioMap->inputPatch(universe);
    OutputPatch* op = m_ioMap->outputPatch(universe);
    OutputPatch* fp = m_ioMap->feedbackPatch(universe);

    QString uniName = m_ioMap->getUniverseNameByIndex(universe);
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
        if (m_ioMap->universesCount() == 0)
            return;

        m_list->setCurrentItem(m_list->item(0));
        item = m_list->currentItem();
    }
    if (item == NULL)
        return;

    quint32 universe = item->data(Qt::UserRole).toInt();
    if (m_editorUniverse == universe)
        return;

    if ((universe + 1) != m_ioMap->universesCount())
        m_deleteUniverseAction->setEnabled(false);
    else
        m_deleteUniverseAction->setEnabled(true);

    if (m_editor != NULL)
    {
        m_splitter->widget(1)->layout()->removeWidget(m_editor);
        m_editor->deleteLater();
        m_editor = NULL;
    }


    m_editor = new InputOutputPatchEditor(this, universe, m_ioMap, m_doc);
    m_editorUniverse = universe;
    m_splitter->widget(1)->layout()->addWidget(m_editor);
    connect(m_editor, SIGNAL(mappingChanged()), this, SLOT(slotMappingChanged()));
    connect(m_editor, SIGNAL(audioInputDeviceChanged()), this, SLOT(slotAudioInputChanged()));
    m_editor->show();
    int uniIdx = m_list->currentRow();
    m_uniNameEdit->setText(m_ioMap->getUniverseNameByIndex(uniIdx));
    m_uniPassthroughCheck->setChecked(m_ioMap->getUniversePassthrough(uniIdx));
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
    m_ioMap->startUniverses();
    m_doc->setModified();
}

void InputOutputManager::slotDeleteUniverse()
{
    int uniIdx = m_list->currentRow();

    Q_ASSERT((uniIdx + 1) == (int)(m_ioMap->universesCount()));

    // Check if the universe is patched
    if (m_ioMap->isUniversePatched(uniIdx) == true)
    {
        // Ask for user's confirmation
        if (QMessageBox::question(
                    this, tr("Delete Universe"),
                    tr("The universe you are trying to delete is patched. Are you sure you want to delete it?"),
                    QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }
    }

    // Check if there are fixtures using this universe
    quint32 uniID = m_ioMap->getUniverseID(uniIdx);
    if (uniID == m_ioMap->invalidUniverse())
        return;

    foreach (Fixture *fx, m_doc->fixtures())
    {
        if (fx->universe() == uniID)
        {
            // Ask for user's confirmation
            if (QMessageBox::question(
                        this, tr("Delete Universe"),
                        tr("There are some fixtures using the universe you are trying to delete. Are you sure you want to delete it?"),
                        QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
            {
                return;
            }
            break;
        }
    }

    m_ioMap->removeUniverse(uniIdx);
    m_doc->setModified();
    updateList();
}

void InputOutputManager::slotUniverseNameChanged(QString name)
{
    QListWidgetItem *currItem = m_list->currentItem();
    if (currItem == NULL)
        return;

    int uniIdx = m_list->currentRow();
    if (name.isEmpty())
        name = tr("Universe %1").arg(uniIdx + 1);
    m_ioMap->setUniverseName(uniIdx, name);
    currItem->setData(Qt::DisplayRole, name);
}

void InputOutputManager::slotUniverseAdded(quint32 universe)
{
    QListWidgetItem *item = new QListWidgetItem(m_list);
    updateItem(item, universe);
}

void InputOutputManager::slotPassthroughChanged(bool checked)
{
    QListWidgetItem *currItem = m_list->currentItem();
    if (currItem == NULL)
        return;

    int uniIdx = m_list->currentRow();
    m_ioMap->setUniversePassthrough(uniIdx, checked);
    m_doc->inputOutputMap()->saveDefaults();
}

void InputOutputManager::showEvent(QShowEvent *ev)
{
    Q_UNUSED(ev);
    // force the recreation of the selected universe editor
    m_editorUniverse = UINT_MAX;
    updateList();
}

