/*
  Q Light Controller
  groupsconsole.h

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

#ifndef GROUPSCONSOLE_H
#define GROUPSCONSOLE_H

#include <QWidget>

#include "consolechannel.h"
#include "scene.h"

class QSlider;
class QSpinBox;

class GroupsConsole : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(GroupsConsole)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    GroupsConsole(QWidget* parent, Doc* doc, QList <quint32> ids, QList<uchar> levels);
    ~GroupsConsole();

    QList<ConsoleChannel*>groups();
    
private:
    Doc* m_doc;
    /** List of active ChannelsGroup IDs */
    QList <quint32> m_ids;
    QList <uchar> m_levels;
    QList<ConsoleChannel*> m_groups;

    /** init the widget view with groups sliders */
    void init();

signals:
    /** Emitted when the value of a channels group object changes (continuous) */
    void groupValueChanged(quint32 group, uchar value);
};

#endif
