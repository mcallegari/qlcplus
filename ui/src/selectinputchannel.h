/*
  Q Light Controller
  selectinputchannel.h

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

#ifndef SELECTINPUTCHANNEL_H
#define SELECTINPUTCHANNEL_H

#include <QDialog>

#include "ui_selectinputchannel.h"

class QTreeWidgetItem;
class QLCInputProfile;
class QLCInputChannel;
class InputPatch;
class InputMap;

class SelectInputChannel : public QDialog, public Ui_SelectInputChannel
{
    Q_OBJECT
    Q_DISABLE_COPY(SelectInputChannel)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    SelectInputChannel(QWidget* parent, InputMap* inputMap);
    ~SelectInputChannel();

protected slots:
    void accept();

private:
    InputMap* m_inputMap;

    /********************************************************************
     * Selection
     ********************************************************************/
public:
    /** Get the selected universe */
    quint32 universe() const;

    /** Get the selected channel within the selected universe */
    quint32 channel() const;

protected:
    quint32 m_universe;
    quint32 m_channel;

    /********************************************************************
     * Tree widget
     ********************************************************************/
protected:
    /** Fill the tree with available input universes & channels */
    void fillTree();

    /** Update the contents of a universe item */
    void updateUniverseItem(QTreeWidgetItem* item,
                            quint32 uni,
                            InputPatch* patch);

    /** Update the contents of a channel item */
    void updateChannelItem(QTreeWidgetItem* item,
                           quint32 universe,
                           const QLCInputChannel* channel,
                           const QLCInputProfile* profile);

protected slots:
    /** Receives changed information for items with manual input enabled */
    void slotItemChanged(QTreeWidgetItem* item, int column);

    /** Receives item double clicks */
    void slotItemDoubleClicked(QTreeWidgetItem* item, int column);
};

#endif
