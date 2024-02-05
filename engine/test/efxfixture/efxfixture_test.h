/*
  Q Light Controller - Unit test
  efxfixture_test.h

  Copyright (c) Heikki Junnila

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
    void startOffset();

    void setPoint8bit();
    void setPoint16bit();
    void setPointPanOnly();
    void setPointLedBar();

    void nextStepLoop();
    void nextStepLoopZeroDuration();
    void nextStepSingleShot();

private:
    Doc* m_doc;

    int m_fixture8bit;
    int m_fixture8bitAddress;
    int m_fixture16bit;
    int m_fixture16bitAddress;
    int m_fixturePanOnly;
    int m_fixturePanOnlyAddress;
    int m_fixtureLedBar;
    int m_fixtureLedBarAddress;
};

#endif
