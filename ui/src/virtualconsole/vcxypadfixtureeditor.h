/*
  Q Light Controller
  vcxypadfixtureeditor.h

  Copyright (c) Heikki Junnila

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

#ifndef VCXYPADFIXTUREEDITOR
#define VCXYPADFIXTUREEDITOR

#include <QDialog>

#include "ui_vcxypadfixtureeditor.h"
#include "vcxypadfixture.h"

/** @addtogroup ui_vc_props
 * @{
 */

class VCXYPadFixtureEditor : public QDialog, public Ui_VCXYPadFixtureEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(VCXYPadFixtureEditor)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    VCXYPadFixtureEditor(QWidget* parent, QList <VCXYPadFixture> fixtures);
    ~VCXYPadFixtureEditor();

protected slots:
    void accept();

    void slotXMinChanged(int value);
    void slotXMaxChanged(int value);
    void slotYMinChanged(int value);
    void slotYMaxChanged(int value);

    /********************************************************************
     * Fixtures
     ********************************************************************/
public:
    QList <VCXYPadFixture> fixtures() const;

protected:
    QList <VCXYPadFixture> m_fixtures;
    int m_maxXVal, m_maxYVal;
};

/** @} */

#endif
