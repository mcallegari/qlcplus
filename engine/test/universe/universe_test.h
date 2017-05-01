/*
  Q Light Controller - Unit test
  universe_test.h

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

#ifndef UNIVERSE_TEST_H
#define UNIVERSE_TEST_H

#include <QObject>

class GrandMaster;
class Universe;

class Universe_Test : public QObject
{
    Q_OBJECT

private slots:

    void init();
    void cleanup();

    void initial();
    void channelCapabilities();
    void blendModes();
    void grandMasterIntensityReduce();
    void grandMasterIntensityLimit();
    void grandMasterAllChannelsReduce();
    void grandMasterAllChannelsLimit();
    void applyGM();
    void write();
    void writeRelative();
    void reset();

    void loadEmpty();
    void loadPassthroughTrue();
    void loadPassthrough1();
    void loadPassthroughFalse();
    void loadWrong();
    void saveEmpty();
    void savePasthroughTrue();

    void setGMValueEfficiency();
    void writeEfficiency();
    void hasChangedEfficiency();
    void hasNotChangedEfficiency();
    void zeroIntensityChannelsEfficiency();
    void zeroIntensityChannelsEfficiency2();

private:

    GrandMaster *m_gm;
    Universe *m_uni;
};

#endif
