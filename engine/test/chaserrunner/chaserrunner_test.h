/*
  Q Light Controller - Unit test
  chaserrunner_test.h

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

#ifndef CHASERRUNNER_TEST_H
#define CHASERRUNNER_TEST_H

#include <QObject>
#include "qlcfixturedefcache.h"

class Chaser;
class Scene;
class Doc;

class ChaserRunner_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void initial();
    void nextPrevious();
    void currentFadeIn();
    void currentFadeOut();
    void currentDuration();
/*
    void roundCheckSingleShotForward();
    void roundCheckSingleShotBackward();
    void roundCheckLoopForward();
    void roundCheckLoopBackward();
    void roundCheckPingPongForward();
    void roundCheckPingPongBackward();
*/
    void writeNoSteps();
    void writeForwardLoopZero();
    void writeBackwardLoopZero();
    void writeForwardSingleShotZero();
    void writeBackwardSingleShotZero();
    void writeForwardPingPongZero();
    void writeBackwardPingPongZero();

    void writeForwardLoopFive();
    void writeBackwardLoopFive();
    void writeForwardSingleShotFive();
    void writeBackwardSingleShotFive();
    void writeForwardPingPongFive();
    void writeBackwardPingPongFive();
    void writeNoAutoStep();

    void adjustIntensity();

private:
    Doc* m_doc;
    Scene* m_scene1;
    Scene* m_scene2;
    Scene* m_scene3;
    Chaser* m_chaser;
};

#endif
