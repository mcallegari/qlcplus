/*
  Q Light Controller Plus
  fixtureremap.h

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

#ifndef FIXTUREREMAP_H
#define FIXTUREREMAP_H

#include <QDialog>
#include <QList>

#include "ui_fixtureremap.h"

class Doc;
class VCWidget;
class RemapWidget;
class SceneValue;

/** @addtogroup ui_fixtures
 * @{
 */

struct RemapInfo
{
    QTreeWidgetItem *source;
    QTreeWidgetItem *target;
};

class FixtureRemap : public QDialog, public Ui_FixtureRemap
{
    Q_OBJECT
    Q_DISABLE_COPY(FixtureRemap)

public:
    explicit FixtureRemap(Doc* doc, QWidget *parent = 0);
    ~FixtureRemap();

private:
    Doc* m_doc;
    Doc* m_targetDoc;
    RemapWidget *remapWidget;
    QList <RemapInfo> m_remapList;

protected:
    QTreeWidgetItem *getUniverseItem(Doc *doc, quint32 universe, QTreeWidget *tree);

    void fillFixturesTree(Doc *doc, QTreeWidget *tree);
    void updateTargetFixturesTree();
    QString createImportDialog();
    void connectFixtures(QTreeWidgetItem *sourceItem, QTreeWidgetItem *targetItem);

    QList<SceneValue> remapSceneValues(QList<SceneValue> funcList,
                                       QList<SceneValue> &srcList,
                                       QList<SceneValue> &tgtList);

protected slots:
    void slotImportFixtures();
    void slotAddTargetFixture();
    void slotRemoveTargetFixture();
    void slotCloneSourceFixture();
    void slotAddRemap();
    void slotRemoveRemap();
    void slotUpdateConnections();
    void slotSourceSelectionChanged();

    /** Callback for OK button clicks */
    void accept();

};

/** @} */

#endif // FIXTUREREMAP_H
