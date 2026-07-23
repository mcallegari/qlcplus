/*
  Q Light Controller Plus - Unit test
  fixtureremapper_test.h

  Copyright (c) Massimo Callegari

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

#ifndef FIXTUREREMAPPER_TEST_H
#define FIXTUREREMAPPER_TEST_H

#include <QObject>

class Doc;
class Fixture;
class QLCFixtureDef;
class QLCFixtureMode;

class FixtureRemapper_Test final : public QObject
{
    Q_OBJECT

private:
    /** Build a 3-channel fixture (Dimmer, Pan, Tilt) with a named def+mode. */
    Fixture *buildMovingHead(Doc *doc, quint32 address, const QString &defName,
                             const QString &modeName);

    /** Build a generic dimmer fixture. */
    Fixture *buildDimmer(Doc *doc, quint32 address, quint32 channels);

    /** Build an RGB LED fixture (R, G, B intensity channels). */
    Fixture *buildRGB(Doc *doc, quint32 address, const QString &defName);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    /** addChannelRemap populates source/target lists. */
    void testAddChannelRemap();

    /** reset clears both lists. */
    void testReset();

    /** remapSceneValues static helper maps values correctly. */
    void testRemapSceneValues();

    /** autoConnectFixtures: same def+mode → 1:1 channel mapping. */
    void testAutoConnectOneToOne();

    /** autoConnectFixtures: different def/mode → semantic channel matching. */
    void testAutoConnectSemantic();

    /** autoConnectFixtures: two generic dimmers → 1:1 mapping. */
    void testAutoConnectGenericDimmer();

    /** applyRemap updates Scene function values. */
    void testApplyRemapScene();

    /** applyRemap updates Sequence step values. */
    void testApplyRemapSequence();

    /** applyRemap updates FixtureGroup heads. */
    void testApplyRemapFixtureGroup();

    /** applyRemap updates ChannelsGroup channels. */
    void testApplyRemapChannelsGroup();

    /** applyRemap updates MonitorProperties fixture entries. */
    void testApplyRemapMonitor();

private:
    Doc *m_doc;
};

#endif // FIXTUREREMAPPER_TEST_H
