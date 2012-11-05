/*
  Q Light Controller - Unit test
  function_test.h

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

#ifndef FUNCTION_TEST_H
#define FUNCTION_TEST_H

#include <QObject>

class Function_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void initial();
    void properties();
    void copyFrom();
    void flashUnflash();
    void elapsed();
    void preRunPostRun();
    void stopAndWait();
    void stopAndWaitFail();
    void adjustIntensity();
    void slotFixtureRemoved();
    void invalidId();
    void typeString();
    void typeToString();
    void stringToType();
    void runOrderToString();
    void stringToRunOrder();
    void directionToString();
    void stringToDirection();
    void loaderWrongRoot();
    void loaderWrongID();
    void loaderScene();
    void loaderChaser();
    void loaderCollection();
    void loaderEFX();
    void loaderUnknownType();

    void runOrderXML();
    void directionXML();
    void speedXML();
};

#endif
