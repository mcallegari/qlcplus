/*
  Q Light Controller
  vcxypadfixtureeditor.h

  Copyright (c) Heikki Junnila

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

#ifndef VCXYPADFIXTUREEDITOR
#define VCXYPADFIXTUREEDITOR

#include <QDialog>

#include "ui_vcxypadfixtureeditor.h"
#include "vcxypadfixture.h"

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
};

#endif
