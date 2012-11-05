/*
  Q Light Controller
  outputmanager.h

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

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include <QWidget>

class QTreeWidgetItem;
class QTreeWidget;
class OutputPatch;
class OutputMap;
class QSplitter;

class OutputManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    OutputManager(QWidget* parent, OutputMap* outputMap);
    virtual ~OutputManager();

    /** Get the singleton instance */
    static OutputManager* instance();

private:
    static OutputManager* s_instance;
    OutputMap* m_outputMap;

    /*********************************************************************
     * Editor
     *********************************************************************/
public slots:
    /** Update the output mapping tree */
    void updateTree();

private slots:
    /** Displays the editor */
    void slotCurrentItemChanged();

    /** Updates the current item */
    void slotMappingChanged();

private:
    /** Update the contents of an OutputPatch to an item */
    void updateItem(QTreeWidgetItem* item, quint32 universe);

    /** Get the current Output Patch Editor (if any) */
    QWidget* currentEditor() const;

private:
    QSplitter* m_splitter;
    QTreeWidget* m_tree;
};

#endif
