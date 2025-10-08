/*
  Q Light Controller
  functionwizard.h

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

#ifndef FUNCTIONWIZARD_H
#define FUNCTIONWIZARD_H

#include <QDialog>
#include <QList>

#include "ui_functionwizard.h"
#include "palettegenerator.h"
#include "scenevalue.h"
#include "function.h"

class QLCChannel;
class VCWidget;
class Fixture;
class Scene;
class Doc;

/** @addtogroup ui_functions
 * @{
 */

class FunctionWizard : public QDialog, public Ui_FunctionWizard
{
    Q_OBJECT

public:
    FunctionWizard(QWidget* parent, Doc* doc);
    ~FunctionWizard();

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
    void slotPageCheckboxChanged();

    /********************************************************************
     * Functions
     ********************************************************************/
protected:

    void addFunctionsGroup(QTreeWidgetItem *fxGrpItem, QTreeWidgetItem *grpItem,
                           QString name, PaletteGenerator::PaletteType type);

    /** Populate the available functions tree based on the available fixtures */
    void updateAvailableFunctionsTree();

    /** Create or retrieve an existing item to group functions of the same type */
    QTreeWidgetItem *getFunctionGroupItem(const Function *func);

    /** Populate the result functions tree based on selected preset functions */
    void updateResultFunctionsTree();

protected slots:
    void slotFunctionItemChanged(QTreeWidgetItem* item, int col);

protected:
    QList<PaletteGenerator *> m_paletteList;

    /********************************************************************
     * Widgets
     ********************************************************************/
protected:

    /** Populate the widgets tree based on selected preset functions */
    void updateWidgetsTree();
    void checkPanTilt(QTreeWidgetItem *grpItem, QTreeWidgetItem *fxGrpItem, qint32* channels);
    void checkRGB(QTreeWidgetItem *grpItem, QTreeWidgetItem *fxGrpItem, qint32* channels);
    void addChannelsToTree(QTreeWidgetItem *grpItem, QTreeWidgetItem *fxGrpItem, QList<quint32> channels );
    void addWidgetItem(QTreeWidgetItem *grpItem, QString name, int type, 
                       QTreeWidgetItem *fxGrpItem, quint32 *chan/* , QLCChannel* channel */);

    VCWidget *createWidget(int type, VCWidget *parent, int xpos, int ypos,
                           Function *func = NULL, int pType = 0, QTreeWidgetItem* fxGrpItem = NULL,
                           quint32 chan = 0, qint32 fixtureNr = -1, qint32 headId = -1);

    QSize recursiveCreateWidget(QTreeWidgetItem *item, VCWidget *parent, int type);

    void addWidgetsToVirtualConsole();
};

/** @} */

#endif
