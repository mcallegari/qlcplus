/*
  Q Light Controller Plus
  remapwidget.h

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

#ifndef REMAPWIDGET_H
#define REMAPWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

struct RemapInfo;

class RemapWidget : public QWidget
{
    Q_OBJECT
public:
    /** Constructor */
    RemapWidget(QTreeWidget *src, QTreeWidget *target, QWidget *parent = 0);

    /** Destructor */
    ~RemapWidget();

    void setRemapList(QList<RemapInfo>list);

protected:
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent* e);

private:
    QTreeWidget *m_sourceTree;
    QTreeWidget *m_targetTree;
    QList<RemapInfo> m_list;
};

#endif
