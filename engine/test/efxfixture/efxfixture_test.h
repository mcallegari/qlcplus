/*
  Q Light Controller - Unit test
  efxfixture_test.h

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

#ifndef EFXFIXTURE_TEST_H
#define EFXFIXTURE_TEST_H

#include <QObject>

class Doc;
class EFXFixture_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void initial();
    void copyFrom();
    void publicProperties();

    void loadSuccess();
    void loadWrongRoot();
    void loadWrongDirection();
    void loadExtraTag();
    void save();

    void serialNumber();
    void isValid();
    void reset();

    void setPoint8bit();
    void setPoint16bit();
    void startOffset();
    void nextStepLoop();
    void nextStepLoopZeroDuration();
    void nextStepSingleShot();

    void start();
    void stop();

private:
    Doc* m_doc;
};

#endif
