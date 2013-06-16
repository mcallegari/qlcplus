/*
  Q Light Controller
  channelselection.h

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

#ifndef CHANNELSELECTION_H
#define CHANNELSELECTION_H

#include <QDialog>

#include "ui_addchannelsgroup.h"
#include "qlcinputsource.h"

class ChannelsGroup;
class Doc;

class AddChannelsGroup : public QDialog, public Ui_AddChannelsGroup
{
    Q_OBJECT
    Q_DISABLE_COPY(AddChannelsGroup)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Constructor
     *
     * @param parent The parent widget for the dialog
     * @param doc The QLC engine instance
     */
    AddChannelsGroup(QWidget* parent, Doc* doc, ChannelsGroup *group);
    ~AddChannelsGroup();

    /** @reimp */
    void accept();

private:
    Doc* m_doc;
    ChannelsGroup* m_chansGroup;

protected:
    QLCInputSource m_inputSource;

    int m_checkedChannels;
    bool m_isUpdating;

    void updateInputSource();

protected slots:
    void slotItemChecked(QTreeWidgetItem *item, int col);
    void slotAutoDetectInputToggled(bool checked);
    void slotInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseInputClicked();
};

#endif
