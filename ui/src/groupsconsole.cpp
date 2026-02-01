/*
  Q Light Controller
  groupsconsole.cpp

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

#include <QHBoxLayout>
#include <QToolButton>
#include <QSpinBox>
#include <QSlider>
#include <QDebug>
#include <QIcon>
#include <QList>

#include "consolechannel.h"
#include "clickandgoslider.h"
#include "groupsconsole.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

GroupsConsole::GroupsConsole(QWidget* parent, Doc* doc, QList <quint32> ids, QList<uchar> levels)
    : QWidget(parent)
    , m_doc(doc)
    , m_ids(ids)
    , m_levels(levels)
{
    Q_ASSERT(doc != NULL);
    new QHBoxLayout(this);

    init();
}

GroupsConsole::~GroupsConsole()
{
}

QList<ConsoleChannel *> GroupsConsole::groups()
{
    return m_groups;
}

void GroupsConsole::init()
{
    int idx = 0;
    foreach (quint32 id, m_ids)
    {
        ChannelsGroup *grp = m_doc->channelsGroup(id);
        if (grp != NULL && grp->getChannels().count() > 0)
        {
            SceneValue scv = grp->getChannels().at(0);

            ConsoleChannel* cc = new ConsoleChannel(this, m_doc, scv.fxi, scv.channel, false);
            cc->setLabel(grp->name());
            cc->setChannelsGroup(id);
            cc->setChannelStyleSheet(CNG_DEFAULT_STYLE);
            if (idx < m_levels.count())
                cc->setValue(m_levels.at(idx));
            layout()->addWidget(cc);
            m_groups.append(cc);

            connect(cc, SIGNAL(groupValueChanged(quint32, uchar)),
                    this, SIGNAL(groupValueChanged(quint32, uchar)));
            idx++;
        }
    }
    /* Make a spacer item eat excess space to justify channels left */
    layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
}





