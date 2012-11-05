/*
  Q Light Controller
  vcxypadproperties.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#ifndef VCXYPADPROPERTIES_H
#define VCXYPADPROPERTIES_H

#include <QDialog>

#include "ui_vcxypadproperties.h"
#include "qlcinputsource.h"
#include "vcxypadfixture.h"

class InputMap;
class VCXYPad;
class Doc;

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
    QList <quint32> selectedFixtureIDs() const;
    QList <VCXYPadFixture> selectedFixtures() const;
    QTreeWidgetItem* fixtureItem(const VCXYPadFixture& fxi);

    void updateFixtureItem(QTreeWidgetItem* item, const VCXYPadFixture& fxi);
    void removeFixtureItem(quint32 fxi_id);

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
    QLCInputSource m_panInputSource;
    QLCInputSource m_tiltInputSource;

    /********************************************************************
     * OK/Cancel
     ********************************************************************/
public slots:
    void accept();
};

#endif
