/*
  Q Light Controller
  VCWizard.h

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

#ifndef VCWIZARD_H
#define VCWIZARD_H

#include <QDialog>
#include <QList>

#include "ui_vcwizard.h"

class QLCChannel;
class VCWidget;
class Fixture;
class Scene;
class Doc;

/** @addtogroup ui_functions
 * @{
 */

class VCWizard : public QDialog, public Ui_VCWizard
{
    Q_OBJECT

public:
    VCWizard(QWidget* parent, Doc* doc);
    ~VCWizard();

protected slots:
    void slotNextPageClicked();
    void slotTabClicked();
    void accept();

private:
    void checkTabsAndButtons();

private:
    Doc* m_doc;

    /********************************************************************
     * Fixtures
     ********************************************************************/
protected:
    /** Create or retrieve an existing item to group fixtures of the same type */
    QTreeWidgetItem *getFixtureGroupItem(QString manufacturer, QString model);

    /** Add a fixture to the tree widget */
    void addFixture(quint32 fxi_id);

    /** Get a list of currently selected fixture ids */
    QList <quint32> fixtureIds() const;

protected slots:
    void slotAddClicked();
    void slotRemoveClicked();

    /********************************************************************
     * Widgets
     ********************************************************************/
protected:

    void checkPanTilt(QTreeWidgetItem *grpItem, QTreeWidgetItem *fxGrpItem, qint32* channels);
    void checkRGB(QTreeWidgetItem *grpItem, QTreeWidgetItem *fxGrpItem, qint32* channels);
    void addWidgetItem(QTreeWidgetItem *grpItem, QString name, int type, 
                       QTreeWidgetItem *fxGrpItem, quint32* channels);

    /** Populate the available widgets tree based on the available fixtures */
    void updateAvailableWidgetsTree();

    VCWidget *createWidget(int type, VCWidget *parent, int xpos, int ypos,
                            QTreeWidgetItem *fxGrpItem, QString str);

    QSize recursiveCreateWidget(QTreeWidgetItem *item, VCWidget *parent);

    void addWidgetsToVirtualConsole();
};

/** @} */

#endif
