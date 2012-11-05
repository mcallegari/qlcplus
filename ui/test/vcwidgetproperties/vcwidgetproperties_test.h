/*
  Q Light Controller - Unit test
  vcwidgetproperties_test.h

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

#ifndef VCWIDGETPROPERTIES_TEST_H
#define VCWIDGETPROPERTIES_TEST_H

#include <QObject>

class VCWidgetProperties_Test : public QObject
{
    Q_OBJECT

private slots:
    void stateAndVisibility();
    void xy();
    void wh();
    void load();
    void loadWrongRoot();
    void save();
    void copy();
};

#endif
