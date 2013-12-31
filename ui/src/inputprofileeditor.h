/*
  Q Light Controller
  inputprofileeditor.h

  Copyright (C) Heikki Junnila

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

#ifndef INPUTPROFILEEDITOR_H
#define INPUTPROFILEEDITOR_H

#include <QDialog>

#include "ui_inputprofileeditor.h"

class QLCInputChannel;
class QLCInputProfile;
class InputOutputMap;
class QTimer;

class InputProfileEditor : public QDialog, public Ui_InputProfileEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(InputProfileEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    InputProfileEditor(QWidget* parent, QLCInputProfile* profile, InputOutputMap* ioMap);
    virtual ~InputProfileEditor();

protected:
    void fillTree();
    void updateChannelItem(QTreeWidgetItem* item, QLCInputChannel* ch);

private:
    InputOutputMap* m_ioMap;

    /************************************************************************
     * OK & Cancel
     ************************************************************************/
public slots:
    void reject();
    void accept();

    /************************************************************************
     * Editing
     ************************************************************************/
protected slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotEditClicked();
    void slotWizardClicked(bool checked);

    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key = 0);
    void slotTimerTimeout();

protected:
    bool m_wizardActive;
    QTreeWidgetItem* m_latestItem;
    QTimer* m_timer;

    /************************************************************************
     * Profile
     ************************************************************************/
public:
    const QLCInputProfile* profile() const;

protected:
    QLCInputProfile* m_profile;
};

#endif
