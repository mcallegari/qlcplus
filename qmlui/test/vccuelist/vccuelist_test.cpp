/*
  Q Light Controller Plus - Test Unit
  vccuelist_test.cpp

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

#include <QtTest>

#include "vccuelist_test.h"
#include "qlcinputsource.h"
#include "chaserstep.h"
#include "vctestenv.h"
#include "vccuelist.h"
#include "listmodel.h"
#include "chaser.h"
#include "scene.h"

/* External control IDs, as defined in vccuelist.cpp */
#define INPUT_NEXT_STEP_ID          0
#define INPUT_PREVIOUS_STEP_ID      1

void VCCueList_Test::init()
{
    m_env = new VCTestEnv();
}

void VCCueList_Test::cleanup()
{
    delete m_env;
}

Chaser *VCCueList_Test::createChaser(QString name, int steps)
{
    Doc *doc = m_env->doc();

    Chaser *chaser = new Chaser(doc);
    chaser->setName(name);
    doc->addFunction(chaser);

    for (int i = 0; i < steps; i++)
    {
        Scene *scene = new Scene(doc);
        doc->addFunction(scene);
        chaser->addStep(ChaserStep(scene->id()));
    }

    return chaser;
}

static int stepsCount(const VCCueList &cueList)
{
    ListModel *model = qvariant_cast<ListModel *>(cueList.stepsList());
    return model == nullptr ? -1 : model->rowCount();
}

void VCCueList_Test::initial()
{
    VCCueList cueList(m_env->doc());

    QCOMPARE(cueList.type(), int(VCWidget::CueListWidget));
    QCOMPARE(cueList.chaserID(), Function::invalidId());
    QVERIFY(cueList.chaser() == nullptr);
    QCOMPARE(stepsCount(cueList), 0);
    QCOMPARE(cueList.nextPrevBehavior(), VCCueList::DefaultRunFirst);
    QCOMPARE(cueList.playbackLayout(), VCCueList::PlayPauseStop);
    QCOMPARE(cueList.sideFaderMode(), VCCueList::None);
    QCOMPARE(cueList.sideFaderLevel(), 100);
    QCOMPARE(cueList.primaryTop(), true);
    QCOMPARE(cueList.nextStepIndex(), -1);
    QCOMPARE(cueList.playbackStatus(), VCCueList::Stopped);
    QCOMPARE(cueList.playbackIndex(), -1);
    QVERIFY(cueList.columnsWidths().isEmpty());
    QCOMPARE(cueList.externalControlsCount(), 5);
    QCOMPARE(cueList.propertiesResource(), QString("qrc:/VCCueListProperties.qml"));

    cueList.setID(0);
    QCOMPARE(cueList.caption(), QString("Cue List 1"));
}

void VCCueList_Test::faderModeStrings()
{
    VCCueList cueList(m_env->doc());

    QCOMPARE(cueList.faderModeToString(VCCueList::None), QString("None"));
    QCOMPARE(cueList.faderModeToString(VCCueList::Crossfade), QString("Crossfade"));
    QCOMPARE(cueList.faderModeToString(VCCueList::Steps), QString("Steps"));

    QCOMPARE(cueList.stringToFaderMode("None"), VCCueList::None);
    QCOMPARE(cueList.stringToFaderMode("Crossfade"), VCCueList::Crossfade);
    QCOMPARE(cueList.stringToFaderMode("Steps"), VCCueList::Steps);
    QCOMPARE(cueList.stringToFaderMode("Foo"), VCCueList::None);
}

void VCCueList_Test::properties()
{
    VCCueList cueList(m_env->doc());

    QSignalSpy npSpy(&cueList, &VCCueList::nextPrevBehaviorChanged);
    cueList.setNextPrevBehavior(VCCueList::RunNext);
    QCOMPARE(cueList.nextPrevBehavior(), VCCueList::RunNext);
    QCOMPARE(npSpy.count(), 1);
    cueList.setNextPrevBehavior(VCCueList::RunNext);
    QCOMPARE(npSpy.count(), 1);

    QSignalSpy layoutSpy(&cueList, &VCCueList::playbackLayoutChanged);
    cueList.setPlaybackLayout(VCCueList::PlayStopPause);
    QCOMPARE(cueList.playbackLayout(), VCCueList::PlayStopPause);
    QCOMPARE(layoutSpy.count(), 1);
    cueList.setPlaybackLayout(VCCueList::PlayStopPause);
    QCOMPARE(layoutSpy.count(), 1);

    /* The side fader level depends on the fader mode */
    QSignalSpy modeSpy(&cueList, &VCCueList::sideFaderModeChanged);
    cueList.setSideFaderMode(VCCueList::Steps);
    QCOMPARE(cueList.sideFaderMode(), VCCueList::Steps);
    QCOMPARE(cueList.sideFaderLevel(), 255);
    QCOMPARE(modeSpy.count(), 1);

    cueList.setSideFaderMode(VCCueList::Crossfade);
    QCOMPARE(cueList.sideFaderLevel(), 100);
    QCOMPARE(modeSpy.count(), 2);

    QSignalSpy levelSpy(&cueList, &VCCueList::sideFaderLevelChanged);
    cueList.setSideFaderLevel(40);
    QCOMPARE(cueList.sideFaderLevel(), 40);
    QCOMPARE(levelSpy.count(), 1);
    cueList.setSideFaderLevel(40);
    QCOMPARE(levelSpy.count(), 1);

    QSignalSpy widthSpy(&cueList, &VCCueList::columnsWidthsChanged);
    QVariantList widths;
    widths << 10.0 << 20.0;
    cueList.setColumnsWidths(widths);
    QCOMPARE(cueList.columnsWidths(), widths);
    QCOMPARE(widthSpy.count(), 1);
    cueList.setColumnsWidths(widths);
    QCOMPARE(widthSpy.count(), 1);
}

void VCCueList_Test::chaserAttachment()
{
    Chaser *chaser = createChaser("MyChaser", 3);
    Chaser *chaser2 = createChaser("Other", 2);

    VCCueList cueList(m_env->doc());
    cueList.setID(0);

    QSignalSpy spy(&cueList, &VCCueList::chaserIDChanged);
    QSignalSpy stepsSpy(&cueList, &VCCueList::stepsListChanged);
    cueList.setChaserID(chaser->id());
    QCOMPARE(cueList.chaserID(), chaser->id());
    QVERIFY(cueList.chaser() == chaser);
    QCOMPARE(cueList.caption(), QString("MyChaser"));
    QCOMPARE(stepsCount(cueList), 3);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(stepsSpy.count(), 1);

    cueList.setChaserID(chaser->id());
    QCOMPARE(spy.count(), 1);

    /* The steps list follows the chaser */
    cueList.setChaserID(chaser2->id());
    QCOMPARE(stepsCount(cueList), 2);

    cueList.setChaserID(12345);
    QCOMPARE(cueList.chaserID(), Function::invalidId());
    QVERIFY(cueList.chaser() == nullptr);
    QCOMPARE(stepsCount(cueList), 0);
}

void VCCueList_Test::chaserRemoved()
{
    Chaser *chaser = createChaser("C", 2);
    quint32 chaserID = chaser->id();

    VCCueList cueList(m_env->doc());
    cueList.setChaserID(chaserID);
    QCOMPARE(stepsCount(cueList), 2);

    QSignalSpy spy(&cueList, &VCCueList::chaserIDChanged);
    QVERIFY(m_env->doc()->deleteFunction(chaserID));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(cueList.chaserID(), Function::invalidId());
    QCOMPARE(stepsCount(cueList), 0);
}

void VCCueList_Test::copy()
{
    Chaser *chaser = createChaser("C", 2);

    VCWidget parent(m_env->doc());

    VCCueList cueList(m_env->doc());
    cueList.setCaption("Copied");
    cueList.setChaserID(chaser->id());
    cueList.setPlaybackLayout(VCCueList::PlayStopPause);
    cueList.setNextPrevBehavior(VCCueList::Select);
    cueList.setColumnsWidths(QVariantList() << 1.0 << 2.0);
    cueList.setSideFaderMode(VCCueList::Crossfade);

    VCCueList *copy = qobject_cast<VCCueList *>(cueList.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->chaserID(), chaser->id());
    QCOMPARE(stepsCount(*copy), 2);
    QCOMPARE(copy->playbackLayout(), VCCueList::PlayStopPause);
    QCOMPARE(copy->nextPrevBehavior(), VCCueList::Select);
    QCOMPARE(copy->columnsWidths(), QVariantList() << 1.0 << 2.0);

    QEXPECT_FAIL("", "copyFrom() doesn't copy the side fader mode", Continue);
    QCOMPARE(copy->sideFaderMode(), VCCueList::Crossfade);
}

void VCCueList_Test::saveLoad()
{
    Chaser *chaser = createChaser("C", 3);

    VCCueList cueList(m_env->doc());
    cueList.setID(11);
    cueList.setCaption("CL");
    cueList.setChaserID(chaser->id());
    cueList.setPlaybackLayout(VCCueList::PlayStopPause);
    cueList.setNextPrevBehavior(VCCueList::Select);
    cueList.setSideFaderMode(VCCueList::Steps);
    cueList.setColumnsWidths(QVariantList() << 100.5 << 50.0 << 30.25);

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 1));
    src->setID(INPUT_NEXT_STEP_ID);
    cueList.addInputSource(src);
    cueList.addKeySequence(QKeySequence("N"), INPUT_PREVIOUS_STEP_ID);

    QByteArray data = vcSaveXML(&cueList);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("CueList"));

    VCCueList cueList2(m_env->doc());
    QVERIFY(cueList2.loadXML(xml.reader()));
    QCOMPARE(cueList2.id(), quint32(11));
    QCOMPARE(cueList2.caption(), QString("CL"));
    QCOMPARE(cueList2.chaserID(), chaser->id());
    QCOMPARE(stepsCount(cueList2), 3);
    QCOMPARE(cueList2.playbackLayout(), VCCueList::PlayStopPause);
    QCOMPARE(cueList2.nextPrevBehavior(), VCCueList::Select);
    QCOMPARE(cueList2.sideFaderMode(), VCCueList::Steps);

    QVariantList widths = cueList2.columnsWidths();
    QCOMPARE(widths.count(), 3);
    QCOMPARE(widths.at(0).toDouble(), 100.5);
    QCOMPARE(widths.at(1).toDouble(), 50.0);
    QCOMPARE(widths.at(2).toDouble(), 30.25);

    QCOMPARE(cueList2.inputSources().count(), 1);
    QCOMPARE(cueList2.inputSources().first()->id(), quint32(INPUT_NEXT_STEP_ID));
    QCOMPARE(cueList2.inputSources().first()->channel(), quint32(1));
    QCOMPARE(cueList2.keySequenceMap().value(QKeySequence("N")), quint32(INPUT_PREVIOUS_STEP_ID));

    /* Default values are not saved */
    VCCueList plain(m_env->doc());
    QByteArray plainData = vcSaveXML(&plain);
    QVERIFY(plainData.contains("<PlaybackLayout>") == false);
    QVERIFY(plainData.contains("<NextPrevBehavior>") == false);
    QVERIFY(plainData.contains("<SlidersMode>") == false);
    QVERIFY(plainData.contains("<ColumnsWidths>") == false);

    /* Invalid layout and behavior values fall back to the defaults */
    VCXMLReader badXml(QByteArray("<CueList><PlaybackLayout>7</PlaybackLayout>"
                                  "<NextPrevBehavior>9</NextPrevBehavior></CueList>"));
    VCCueList cueList3(m_env->doc());
    QVERIFY(cueList3.loadXML(badXml.reader()));
    QCOMPARE(cueList3.playbackLayout(), VCCueList::PlayPauseStop);
    QCOMPARE(cueList3.nextPrevBehavior(), VCCueList::DefaultRunFirst);
}

QTEST_MAIN(VCCueList_Test)
