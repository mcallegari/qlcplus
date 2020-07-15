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


#ifndef ACTSONCHANNELDIALOG_H
#define ACTSONCHANNELDIALOG_H

#include <QDialog>

namespace Ui {
class ActsOnChannelDialog;
}

class QLCChannel;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

class ActsOnChannelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ActsOnChannelDialog(QList<QLCChannel*> allList, QVector<QLCChannel*> modeList, QWidget *parent = nullptr);
    ~ActsOnChannelDialog();

    QList<QLCChannel *> getModeChannelsList();

private slots:
    void slotAddChannel();
    void slotRemoveChannel();

private:
    Ui::ActsOnChannelDialog *ui;
    QList<QLCChannel*> m_channelsList;
    void fillChannelsTrees(QList<QLCChannel*> allList, QVector<QLCChannel*> modeList);

};

#endif // ACTSONCHANNELDIALOG_H
