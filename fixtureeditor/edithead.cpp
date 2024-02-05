/*
  Q Light Controller - Fixture Editor
  edithead.cpp

  Copyright (C) Heikki Junnila

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
#include <QDebug>
#include <QAction>
#include <QSettings>

#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcchannel.h"
#include "edithead.h"

#define SETTINGS_GEOMETRY "edithead/geometry"

EditHead::EditHead(QWidget* parent, const QLCFixtureHead& head, const QLCFixtureMode* mode)
    : QDialog(parent)
    , m_head(head)
{
    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    fillChannelTree(mode);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

EditHead::~EditHead()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

QLCFixtureHead EditHead::head() const
{
    return m_head;
}

void EditHead::fillChannelTree(const QLCFixtureMode* mode)
{
    Q_ASSERT(mode != NULL);

    for (quint32 i = 0; i < quint32(mode->channels().size()); i++)
    {
        const QLCChannel* ch = mode->channels().at(i);
        Q_ASSERT(ch != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(0, QString::number(i + 1));
        item->setText(1, ch->name());

        if (ch->group() == QLCChannel::Intensity && ch->colour() == QLCChannel::NoColour)
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        else if (m_head.channels().contains(i) == false && mode->headForChannel(i) != -1)
            item->setFlags(Qt::NoItemFlags);
        else
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        if (m_head.channels().contains(i) == true)
            item->setCheckState(0, Qt::Checked);
        else
            item->setCheckState(0, Qt::Unchecked);
    }
}

void EditHead::slotItemChanged(QTreeWidgetItem* item, int column)
{
    if (column != 0)
        return;

    int index = m_tree->indexOfTopLevelItem(item);

    if (item->checkState(0) == Qt::Checked)
        m_head.addChannel(index);
    else
        m_head.removeChannel(index);
}
