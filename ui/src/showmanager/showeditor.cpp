/*
  Q Light Controller
  showeditor.cpp

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
#include <QLineEdit>
#include <QLabel>
#include <QDebug>

#include "chaserstep.h"
#include "showeditor.h"
#include "chaser.h"
#include "audio.h"
#include "track.h"
#include "scene.h"
#include "show.h"
#include "doc.h"

#define NAME_COL  0
#define STEPS_COL 1
#define TIME_COL  2
#define DUR_COL   3

#define PROP_ID Qt::UserRole

ShowEditor::ShowEditor(QWidget* parent, Show* show, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_show(show)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(show != NULL);

    setupUi(this);
    m_tree->setRootIsDecorated(true);
    m_tree->setSortingEnabled(false);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    m_tree->header()->setResizeMode(QHeaderView::Interactive);
#else
    m_tree->header()->setSectionResizeMode(QHeaderView::Interactive);
#endif

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(m_remove, SIGNAL(clicked()), this, SLOT(slotRemove()));

    // for now just disabled the add/remove buttons
    m_add->setVisible(false);
    m_remove->setVisible(false);

    m_nameEdit->setText(m_show->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    updateFunctionList();

    // Set focus to the editor
    m_nameEdit->setFocus();
}

ShowEditor::~ShowEditor()
{
}

void ShowEditor::slotNameEdited(const QString& text)
{
    m_show->setName(text);
}

void ShowEditor::slotAdd()
{

}

void ShowEditor::slotRemove()
{

}

void ShowEditor::updateFunctionList()
{
    quint32 totalDuration = 0;

    m_tree->clear();

    if (m_show == NULL)
    {
        qDebug() << Q_FUNC_INFO << "Invalid show !";
        return;
    }

    QTreeWidgetItem* masterItem = new QTreeWidgetItem(m_tree);
    masterItem->setText(NAME_COL, m_show->name());
    masterItem->setData(NAME_COL, PROP_ID, m_show->id());
    masterItem->setIcon(NAME_COL, QIcon(":/show.png"));

    foreach(Track *track, m_show->tracks())
    {
        Scene *scene = qobject_cast<Scene*>(m_doc->function(track->getSceneID()));
        if (scene == NULL)
        {
            qDebug() << Q_FUNC_INFO << "Invalid scene !";
            continue;
        }
        QTreeWidgetItem* sceneItem = new QTreeWidgetItem(masterItem);
        sceneItem->setText(NAME_COL, scene->name());
        sceneItem->setData(NAME_COL, PROP_ID, scene->id());
        sceneItem->setIcon(NAME_COL, QIcon(":/scene.png"));

        foreach(quint32 id, track->functionsID())
        {
            Function *func = m_doc->function(id);
            if (func == NULL)
                continue;

            if (func->type() == Function::Chaser)
            {
                Chaser *chaser = qobject_cast<Chaser*>(m_doc->function(id));
                QTreeWidgetItem *chItem = new QTreeWidgetItem(sceneItem);
                chItem->setText(NAME_COL, chaser->name());
                chItem->setData(NAME_COL, PROP_ID, chaser->id());
                chItem->setIcon(NAME_COL, QIcon(":/sequence.png"));
                chItem->setText(STEPS_COL, QString("%1").arg(chaser->steps().count()));
                chItem->setText(TIME_COL, Function::speedToString(chaser->getStartTime()));
                quint32 seq_duration = chaser->getDuration();
                chItem->setText(DUR_COL, Function::speedToString(seq_duration));
                if (chaser->getStartTime() + seq_duration > totalDuration)
                    totalDuration = chaser->getStartTime() + seq_duration;
            }
            else if (func->type() == Function::Audio)
            {
                Audio *audio = qobject_cast<Audio*>(m_doc->function(id));
                QTreeWidgetItem *chItem = new QTreeWidgetItem(sceneItem);
                chItem->setText(NAME_COL, audio->name());
                chItem->setData(NAME_COL, PROP_ID, audio->id());
                chItem->setIcon(NAME_COL, QIcon(":/audio.png"));
                chItem->setText(TIME_COL, Function::speedToString(audio->getStartTime()));
                chItem->setText(DUR_COL, Function::speedToString(audio->getDuration()));
                if (audio->getStartTime() + audio->getDuration() > totalDuration)
                    totalDuration = audio->getStartTime() + audio->getDuration();
            }
        }
    }

    masterItem->setText(DUR_COL, Function::speedToString(totalDuration));

    m_tree->expandAll();
    m_tree->resizeColumnToContents(NAME_COL);
    m_tree->resizeColumnToContents(TIME_COL);
    m_tree->resizeColumnToContents(DUR_COL);
}
