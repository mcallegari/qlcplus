/*
  Q Light Controller Plus
  functionstreewidget.h

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


#ifndef FUNCTIONSTREEWIDGET_H
#define FUNCTIONSTREEWIDGET_H

#include <QTreeWidget>

class Function;
class Doc;

class FunctionsTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    FunctionsTreeWidget(Doc* doc, QWidget *parent = 0);
    
    /** Update all functions to function tree */
    void updateTree();

    void clearTree();

    void functionChanged(quint32 fid);

    void functionAdded(quint32 fid);

    /** Return a suitable parent item for the $function's type */
    QTreeWidgetItem* parentItem(const Function* function);

    /** Get the ID of the function represented by $item. */
    quint32 itemFunctionId(const QTreeWidgetItem* item) const;

    /** Get the item that represents the given function. */
    QTreeWidgetItem* functionItem(const Function* function);

private:
    /** Update $item's contents from the given $function */
    void updateFunctionItem(QTreeWidgetItem* item, const Function* function);

    /** Get an icon that represents the given function's type */
    QIcon functionIcon(const Function* function) const;

private:
    Doc* m_doc;

    /*********************************************************************
     * Tree folders
     *********************************************************************/
public:
    void addFolder();

    void deleteFolder(QString name);

private:
    QTreeWidgetItem *folderItem(QString name);

private:
    QHash <QString, QTreeWidgetItem *> m_foldersMap;

    /*********************************************************************
     * Drag & Drop events
     *********************************************************************/
protected:
    void mousePressEvent(QMouseEvent *event);

    void dropEvent(QDropEvent *event);

private:
    QTreeWidgetItem *m_draggedItem;
};

#endif // FUNCTIONSTREEWIDGET_H
