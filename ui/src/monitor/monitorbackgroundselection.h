/*
  Q Light Controller Plus
  monitorbackgroundselection.h

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

#ifndef MONITORBACKGROUNDSELECTION_H
#define MONITORBACKGROUNDSELECTION_H

#include <QDialog>

#include "ui_monitorbackgroundselection.h"

class MonitorProperties;
class Doc;

/** \addtogroup ui_mon DMX Monitor
 * @{
 */

class MonitorBackgroundSelection : public QDialog, public Ui_MonitorBackgroundSelection
{
    Q_OBJECT

public:
    explicit MonitorBackgroundSelection(QWidget *parent, Doc* doc);
    ~MonitorBackgroundSelection();

    /** @reimp */
    void accept();

protected:
    void updateCustomTree();

protected slots:
    void slotNoBackgroundChecked(bool checked);
    void slotCommonBackgroundChecked(bool checked);
    void slotCustomBackgroundChecked(bool checked);

    void slotSelectCommonBackground();
    void slotAddCustomBackground();
    void slotRemoveCustomBackground();

private:
    Doc *m_doc;
    MonitorProperties *m_props;
    QString m_commonBackgroundImage;
    QMap <quint32, QString> m_customBackgroundImages;
    QString m_lastUsedPath;
};

/** @} */

#endif // MONITORBACKGROUNDSELECTION_H
