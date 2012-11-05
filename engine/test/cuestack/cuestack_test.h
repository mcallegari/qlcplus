/*
  Q Light Controller - Unit test
  cuestack_test.h

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

#ifndef CUESTACK_TEST_H
#define CUESTACK_TEST_H

#include <QObject>

class Doc;
class CueStack_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void initial();
    void name();
    void speeds();
    void appendCue();
    void insertCue();
    void replaceCue();
    void currentIndex();
    void removeCue();
    void removeCues();

    void load();
    void save();
    void flash();
    void startStop();
    void preRun();
    void intensity();
    void nextPrevious();
    void insertStartValue();
    void switchCue();
    void postRun();
    void write();

private:
    Doc* m_doc;
};

#endif
