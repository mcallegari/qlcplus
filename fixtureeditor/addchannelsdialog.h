/*
  Q Light Controller Plus - Fixture Definition Editor
  addchannelsdialog.h

  Copyright (C) Massimo Callegari

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

#ifndef ADDCHANNELSDIALOG_H
#define ADDCHANNELSDIALOG_H

#include <QDialog>

#include "ui_addchannelsdialog.h"

class QLCChannel;

class AddChannelsDialog : public QDialog, public Ui_AddChannelsDialog
{
    Q_OBJECT
    
public:
    explicit AddChannelsDialog(QList<QLCChannel*> allList, QList<QLCChannel*> modeList, QWidget *parent = 0);
    ~AddChannelsDialog();

    QList<QLCChannel*> getModeChannelsList();
    
private:
    void fillChannelsTrees(QList<QLCChannel*> allList, QList<QLCChannel*> modeList);

private slots:
    void slotAddChannel();
    void slotRemoveChannel();

private:
    QList<QLCChannel*> m_channelsList;

};

#endif // ADDCHANNELSDIALOG_H
