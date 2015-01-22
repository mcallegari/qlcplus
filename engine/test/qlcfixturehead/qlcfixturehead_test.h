/*
  Q Light Controller - Unit tests
  qlcfixturehead_test.h

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

#ifndef QLCFIXTUREHEAD_TEST_H
#define QLCFIXTUREHEAD_TEST_H

#include <QObject>

class QLCFixtureDef;
class QLCChannel;

class QLCFixtureHead_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void channels();
    void load();
    void save();
    void cacheChannelsRgbMaster();
    void cacheChannelsCmyMaster();
    void cacheChannelsPanTilt();
    void cacheChannelsColor();
    void doublePanTilt();
    void dimmerHead();

    void cleanupTestCase();

private:
    QLCFixtureDef* m_fixtureDef;
    QLCChannel* m_ch1;
    QLCChannel* m_ch2;
    QLCChannel* m_ch3;
    QLCChannel* m_ch4;
};

#endif
