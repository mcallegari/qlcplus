/*
  Q Light Controller
  outputpatcheditor.h

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

#ifndef OUTPUTPATCHEDITOR_H
#define OUTPUTPATCHEDITOR_H

#include <QWidget>

#include "ui_outputpatcheditor.h"

class QStringList;
class OutputPatch;
class OutputMap;

class OutputPatchEditor : public QWidget, public Ui_OutputPatchEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputPatchEditor)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Create a new output patch editor for the given universe.
     *
     * @param parent The owning parent widget
     * @param universe The universe whose settings to edit
     * @param outputMap The output map object that handles DMX output
     */
    OutputPatchEditor(QWidget* parent, quint32 universe, OutputMap* outputMap);
    ~OutputPatchEditor();

signals:
    /** Tells that the mapping settings have changed */
    void mappingChanged();

private:
    QTreeWidgetItem* currentlyMappedItem() const;
    void fillTree();
    void fillPluginItem(const QString& pluginName, QTreeWidgetItem* pitem);
    QTreeWidgetItem* pluginItem(const QString& pluginName);
    void updateOutputInfo();
    OutputPatch* patch() const;

private slots:
    void slotCurrentItemChanged(QTreeWidgetItem* item);
    void slotItemChanged(QTreeWidgetItem* item);
    void slotConfigureClicked();
    void slotPluginConfigurationChanged(const QString& pluginName);

private:
    OutputMap* m_outputMap;
    int m_universe;
};

#endif /* OUTPUTPATCHEDITOR_H */
