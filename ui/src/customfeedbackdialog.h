/*
  Q Light Controller Plus
  customfeedbackdialog.h

  Copyright (c) Massimo Callegari

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

#ifndef CUSTOMFEEDBACKDIALOG_H
#define CUSTOMFEEDBACKDIALOG_H

#include <QDialog>

#include "ui_customfeedbackdialog.h"

class Doc;
class QLCInputSource;
class QLCInputProfile;

class CustomFeedbackDialog : public QDialog, public Ui_CustomFeedbackDialog
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    explicit CustomFeedbackDialog(Doc *doc, QSharedPointer<QLCInputSource> const& source, QWidget *parent = nullptr);
    ~CustomFeedbackDialog();

    enum SelectedFeedback { None, LowerValue, UpperValue, MonitoringValue };

    void setMonitoringVisibility(bool visible);

    /** @reimp */
    void accept();

protected slots:
    void slotLowerColorButtonClicked();
    void slotUpperColorButtonClicked();
    void slotMonitorColorButtonClicked();
    void slotColorSelected(QTreeWidgetItem *item);

private:
    Doc *m_doc;
    QLCInputProfile *m_profile;
    QSharedPointer<QLCInputSource> m_inputSource;
    SelectedFeedback m_selectedFeedback;
};

#endif // CUSTOMFEEDBACKDIALOG_H
