/*
  Q Light Controller - Fixture Editor
  edithead.h

  Copyright (C) Heikki Junnila

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

#ifndef EDITHEAD_H
#define EDITHEAD_H

#include <QDialog>

#include "ui_edithead.h"
#include "qlcfixturehead.h"

class QTreeWidgetItem;
class QLCFixtureMode;

class EditHead : public QDialog, public Ui_EditHead
{
    Q_OBJECT

public:
    EditHead(QWidget* parent, const QLCFixtureHead& head, const QLCFixtureMode* mode);
    ~EditHead();

    QLCFixtureHead head() const;

private:
    void fillChannelTree(const QLCFixtureMode* mode);

private slots:
    void slotItemChanged(QTreeWidgetItem* item, int column);

private:
    QLCFixtureHead m_head;
};

#endif
