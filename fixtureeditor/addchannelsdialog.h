/*
  Q Light Controller Plus - Fixture Definition Editor
  addchannelsdialog.h

  Copyright (C) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef ADDCHANNELSDIALOG_H
#define ADDCHANNELSDIALOG_H

#include <QDialog>

#include "ui_addchannelsdialog.h"

class QLCChannel;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

class AddChannelsDialog : public QDialog, public Ui_AddChannelsDialog
{
    Q_OBJECT

public:
    explicit AddChannelsDialog(QList<QLCChannel*> allList, QVector<QLCChannel*> modeList, QWidget *parent = 0);
    ~AddChannelsDialog();

    QList<QLCChannel*> getModeChannelsList();

private:
    void fillChannelsTrees(QList<QLCChannel*> allList, QVector<QLCChannel*> modeList);

private slots:
    void slotAddChannel();
    void slotRemoveChannel();

private:
    QList<QLCChannel*> m_channelsList;

};

/** @} */

#endif // ADDCHANNELSDIALOG_H
