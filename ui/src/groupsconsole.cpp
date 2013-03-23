/*
  Q Light Controller
  groupsconsole.cpp

  Copyright (c) Massimo Callegari

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

#include <QHBoxLayout>
#include <QToolButton>
#include <QSpinBox>
#include <QSlider>
#include <QDebug>
#include <QIcon>
#include <QList>

#include "consolechannel.h"
#include "groupsconsole.h"
#include "fixture.h"
#include "apputil.h"
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
    foreach(quint32 id, m_ids)
    {
        ChannelsGroup *grp = m_doc->channelsGroup(id);
        SceneValue scv = grp->getChannels().at(0);

        ConsoleChannel* cc = new ConsoleChannel(this, m_doc, scv.fxi, scv.channel);
        cc->setLabel(grp->name());
        cc->setChannelsGroup(id);
        if (idx < m_levels.count())
            cc->setValue(m_levels.at(idx));
        layout()->addWidget(cc);
        m_groups.append(cc);

        connect(cc, SIGNAL(groupValueChanged(quint32, uchar)),
                this, SIGNAL(groupValueChanged(quint32, uchar)));
        idx++;
    }
    /* Make a spacer item eat excess space to justify channels left */
    layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
}





