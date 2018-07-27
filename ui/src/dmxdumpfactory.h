/*
  Q Light Controller Plus
  dmxdumpfactory.h

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

#ifndef DMXDUMPFACTORY_H
#define DMXDUMPFACTORY_H

#include <QDialog>

#include "ui_dmxdumpfactory.h"

class DmxDumpFactoryProperties;
class FixtureTreeWidget;
class VCWidget;
class Doc;

/** @addtogroup ui UI
 * @{
 */

class DmxDumpFactory : public QDialog, public Ui_DmxDumpFactory
{
    Q_OBJECT
    Q_DISABLE_COPY(DmxDumpFactory)

public:
    DmxDumpFactory(Doc* doc, DmxDumpFactoryProperties* props, QWidget *parent = 0);
    ~DmxDumpFactory();

protected slots:
    void slotUpdateChasersTree();
    void slotUpdateButtons();
    void slotUpdateSliders();
    void slotSelectSceneButtonClicked();

protected:
    QList<VCWidget *> getChildren(VCWidget *obj, int type);
    void updateWidgetsTree(int type);

private:
    Doc* m_doc;
    FixtureTreeWidget *m_fixturesTree;
    DmxDumpFactoryProperties* m_properties;
    quint32 m_selectedSceneID;

protected slots:
    void slotDumpModeChanged(bool mode);
    void slotDumpNonZeroChanged(bool active);

    /** Callback for OK button clicks */
    void accept();

};

/** @} */

#endif // DMXDUMPFACTORY_H
