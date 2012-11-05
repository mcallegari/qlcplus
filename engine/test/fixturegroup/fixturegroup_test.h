/*
  Q Light Controller - Unit test
  fixturegroup_test.h

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

#ifndef FIXTUREGROUP_TEST_H
#define FIXTUREGROUP_TEST_H

#include <QObject>

class Doc;
class FixtureGroup_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void groupHead();
    void id();
    void name();
    void size();
    void assignFixtureNoSize();
    void assignFixture4x2();
    void resignFixture();
    void resignHead();
    void fixtureRemoved();
    void swap();
    void copy();
    void loadWrongID();
    void loadWrongHeadAttributes();
    void load();
    void save();

private:
    Doc* m_doc;
};

#endif
