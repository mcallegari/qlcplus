/*
  Q Light Controller
  inputmanager.cpp

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

#include "inputpatcheditor.h"
#include "inputmanager.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "apputil.h"
#include "doc.h"

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnInput    2
#define KColumnProfile  3
#define KColumnInputNum 4

#define SETTINGS_SPLITTER "inputmanager/splitter"

InputManager* InputManager::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputManager::InputManager(QWidget* parent, InputMap* inputMap)
    : QWidget(parent)
    , m_inputMap(inputMap)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    Q_ASSERT(inputMap != NULL);

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
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QStringList columns;
    columns << tr("Universe") << tr("Plugin") << tr("Input") << tr("Profile");
    m_tree->setHeaderLabels(columns);

    connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotCurrentItemChanged()));

    /* Timer that clears the input data icon after a while */
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
    slotCurrentItemChanged();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());
}

InputManager::~InputManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());

    s_instance = NULL;
}

InputManager* InputManager::instance()
{
    return s_instance;
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void InputManager::updateTree()
{
    m_tree->clear();
    for (quint32 uni = 0; uni < m_inputMap->universes(); uni++)
        updateItem(new QTreeWidgetItem(m_tree), uni);
    m_tree->setCurrentItem(m_tree->topLevelItem(0));
}

void InputManager::updateItem(QTreeWidgetItem* item, quint32 universe)
{
    Q_ASSERT(item != NULL);

    InputPatch* ip = m_inputMap->patch(universe);
    Q_ASSERT(ip != NULL);

    item->setText(KColumnUniverse, QString::number(universe + 1));
    item->setText(KColumnPlugin, ip->pluginName());
    item->setText(KColumnInput, ip->inputName());
    item->setText(KColumnProfile, ip->profileName());
    item->setText(KColumnInputNum, QString::number(ip->input() + 1));
}

QWidget* InputManager::currentEditor() const
{
    Q_ASSERT(m_splitter != NULL);
    if (m_splitter->count() < 2)
        return NULL;
    else
        return m_splitter->widget(1);
}

void InputManager::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);

    QTreeWidgetItem* item = m_tree->topLevelItem(universe);
    if (item == NULL)
        return;

    /* Show an icon on a universe row that received input data */
    QIcon icon(":/input.png");
    item->setIcon(KColumnUniverse, icon);

    /* Restart the timer */
    m_timer->start(250);
}

void InputManager::slotTimerTimeout()
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        (*it)->setIcon(KColumnUniverse, QIcon());
        ++it;
    }
}

void InputManager::slotCurrentItemChanged()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
    {
        m_tree->setCurrentItem(m_tree->topLevelItem(0));
        item = m_tree->currentItem();
    }
    Q_ASSERT(item != NULL);

    if (currentEditor() != NULL)
        delete currentEditor();

    quint32 universe = item->text(KColumnUniverse).toInt() - 1;
    QWidget* editor = new InputPatchEditor(this, universe, m_inputMap);
    m_splitter->addWidget(editor);
    connect(editor, SIGNAL(mappingChanged()), this, SLOT(slotMappingChanged()));
    editor->show();
}

void InputManager::slotMappingChanged()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        uint universe = item->text(KColumnUniverse).toUInt() - 1;
        updateItem(item, universe);
    }
}
