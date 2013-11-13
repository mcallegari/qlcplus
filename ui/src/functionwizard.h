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

class QLCChannel;
class Fixture;
class Scene;
class Doc;

class FunctionWizard : public QDialog, public Ui_FunctionWizard
{
    Q_OBJECT

public:
    FunctionWizard(QWidget* parent, Doc* doc);
    ~FunctionWizard();

protected slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void accept();

private:
    Doc* m_doc;

    /********************************************************************
     * Fixtures
     ********************************************************************/
protected:
    /** Add a fixture to the tree widget */
    void addFixture(quint32 fxi_id);

    /** Get a list of currently selected fixtures */
    QList <Fixture*> fixtures() const;

    /** Get a list of currently selected fixture ids */
    QList <quint32> fixtureIds() const;
};

#endif

