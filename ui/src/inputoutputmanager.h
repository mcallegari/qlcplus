/*
  Q Light Controller
  inputoutputmanager.h

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

#ifndef INPUTOUTPUTMANAGER_H
#define INPUTOUTPUTMANAGER_H

#include <QWidget>
#include <QIcon>

class InputOutputPatchEditor;
class QTreeWidgetItem;
class QTreeWidget;
class QSplitter;
class QTimer;
class QIcon;

class InputPatch;
class InputMap;
class OutputPatch;
class OutputMap;
class Doc;

class InputOutputManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(InputOutputManager)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    InputOutputManager(QWidget* parent, Doc* doc);
    virtual ~InputOutputManager();

    /** Get the singleton instance */
    static InputOutputManager* instance();

private:
    static InputOutputManager* s_instance;
    InputMap* m_inputMap;
    OutputMap* m_outputMap;

    /*************************************************************************
     * Tree widget
     *************************************************************************/
public slots:
    /** Update the input mapping tree */
    void updateTree();

private:
    /** Update the contents of the input universe to the item */
    void updateItem(QTreeWidgetItem* item, quint32 universe);

private slots:
    /** Listens to input data and displays a small icon to indicate a
        working connection between a plugin and an input device. */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

    /** Hides the small icon after a while ^^ */
    void slotTimerTimeout();

    /** Displays an editor for the currently selected universe */
    void slotCurrentItemChanged();

    /** Updates the current item */
    void slotMappingChanged();

private:
    QSplitter* m_splitter;
    QTreeWidget* m_tree;
    QIcon m_icon;
    QTimer* m_timer;
    InputOutputPatchEditor *m_editor;
};

#endif
