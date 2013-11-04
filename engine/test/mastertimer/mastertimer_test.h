/*
  Q Light Controller
  mastertimer_test.h

  Copyright (C) Heikki Junnila

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

#ifndef MASTERTIMER_TEST_H
#define MASTERTIMER_TEST_H

#include <QObject>

class Doc;
class MasterTimer_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void initial();
    void startStop();
    void startStopFunction();
    void registerUnregisterDMXSource();
    void interval();
    void functionInitiatedStop();
    void runMultipleFunctions();
    void stopAllFunctions();
    void stop();
    void restart();

private:
    Doc* m_doc;
};

#endif
