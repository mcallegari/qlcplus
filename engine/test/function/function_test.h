/*
  Q Light Controller - Unit test
  function_test.h

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
    void speedToString();
    void stringToSpeed();
    void speedOperations();
    void tempo();
    void attributes();
    void blendMode();
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
