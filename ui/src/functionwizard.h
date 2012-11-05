/*
  Q Light Controller
  functionwizard.h

  Copyright (C) Heikki Junnila

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

