/*
  Q Light Controller
  vcxypadproperties.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#ifndef VCXYPADPROPERTIES_H
#define VCXYPADPROPERTIES_H

#include <QDialog>

#include "ui_vcxypadproperties.h"
#include "qlcinputsource.h"
#include "vcxypadfixture.h"

class InputMap;
class VCXYPad;
class Doc;

/** @addtogroup ui_vc_props
 * @{
 */

class VCXYPadProperties : public QDialog, public Ui_VCXYPadProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCXYPadProperties)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    VCXYPadProperties(VCXYPad* xypad, Doc* doc);
    ~VCXYPadProperties();

private:
    VCXYPad* m_xypad;
    Doc* m_doc;

    /********************************************************************
     * Fixtures page
     ********************************************************************/
private:
    void fillTree();
    QList <VCXYPadFixture> selectedFixtures() const;
    QTreeWidgetItem* fixtureItem(const VCXYPadFixture& fxi);

    void updateFixtureItem(QTreeWidgetItem* item, const VCXYPadFixture& fxi);
    void removeFixtureItem(GroupHead const & head);

private slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotEditClicked();
    void slotSelectionChanged(QTreeWidgetItem* item);

    /********************************************************************
     * Fixtures page
     ********************************************************************/
private slots:
    void slotPanAutoDetectToggled(bool toggled);
    void slotPanChooseClicked();
    void slotPanInputValueChanged(quint32 uni, quint32 ch);

    void slotTiltAutoDetectToggled(bool toggled);
    void slotTiltChooseClicked();
    void slotTiltInputValueChanged(quint32 uni, quint32 ch);

private:
    void updatePanInputSource();
    void updateTiltInputSource();

private:
    QLCInputSource *m_panInputSource;
    QLCInputSource *m_tiltInputSource;

    /********************************************************************
     * OK/Cancel
     ********************************************************************/
public slots:
    void accept();
};

/** @} */

#endif
