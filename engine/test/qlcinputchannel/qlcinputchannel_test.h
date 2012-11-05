/*
  Q Light Controller - Unit tests
  qlcinputchannel_test.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#ifndef QLCINPUTCHANNEL_TEST_H
#define QLCINPUTCHANNEL_TEST_H

#include <QObject>

class QLCInputChannel_Test : public QObject
{
    Q_OBJECT

private slots:
    void types();
    void type();
    void typeToString();
    void stringToType();
    void name();
    void copy();
    void load();
    void loadWrongType();
    void save();
};

#endif
