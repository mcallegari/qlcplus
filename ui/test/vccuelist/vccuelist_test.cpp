/*
  Q Light Controller
  vccuelist_test.cpp

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

#include <QTreeWidgetItem>
#include <QDomDocument>
#include <QDomElement>
#include <QTreeWidget>
#include <QtTest>

#define protected public
#define private public
#include "virtualconsole.h"
#include "genericfader.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "vccuelist.h"
#include "vcwidget.h"
#include "vcframe.h"

#include "qlcfixturedefcache.h"
#include "qlcinputsource.h"
#include "vccuelist_test.h"
#include "chaserstep.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"
#undef private
#undef protected

static const QKeySequence keySequenceA(Qt::Key_A);
static const QKeySequence keySequenceB(Qt::Key_B);
static const QKeySequence keySequenceC(Qt::Key_C);
static const QKeySequence keySequenceD(Qt::Key_D);

static Chaser* createChaser(Doc* doc)
{
    Fixture* fxi = new Fixture(doc);
    fxi->setChannels(1);
    doc->addFixture(fxi);

    Scene* s1 = new Scene(doc);
    s1->setName("First");
    s1->setValue(fxi->id(), 0, 255);
    doc->addFunction(s1);

    Scene* s2 = new Scene(doc);
    s2->setName("Second");
    s2->setValue(fxi->id(), 0, 127);
    doc->addFunction(s2);

    Scene* s3 = new Scene(doc);
    s3->setName("Third");
    s3->setValue(fxi->id(), 0, 64);
    doc->addFunction(s3);

    Scene* s4 = new Scene(doc);
    s4->setName("Fourth");
    s4->setValue(fxi->id(), 0, 32);
    doc->addFunction(s4);

    Chaser* c = new Chaser(doc);
    c->addStep(ChaserStep(s1->id()));
    c->addStep(ChaserStep(s2->id()));
    c->addStep(ChaserStep(s3->id()));
    c->addStep(ChaserStep(s4->id()));
    doc->addFunction(c);

    return c;
}

void VCCueList_Test::initTestCase()
{
    m_doc = NULL;
}

void VCCueList_Test::init()
{
    m_doc = new Doc(this);
    new VirtualConsole(NULL, m_doc);
}

void VCCueList_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_doc;
}

void VCCueList_Test::initial()
{
    QWidget w;

    VCCueList cl(&w, m_doc);
    QCOMPARE(cl.objectName(), QString("VCCueList"));
    QCOMPARE(cl.frameStyle(), QFrame::Panel | QFrame::Sunken);
    QCOMPARE(cl.caption(), tr("Cue list"));
    QCOMPARE(cl.size(), QSize(300, 220));
    // QVERIFY(cl.m_runner == NULL);
    QVERIFY(cl.m_tree != NULL);
    QCOMPARE(cl.m_tree->isEnabled(), false);
    QCOMPARE(cl.m_tree->topLevelItemCount(), 0);
    QCOMPARE(cl.m_tree->selectionMode(), QAbstractItemView::SingleSelection);
    QCOMPARE(cl.m_tree->rootIsDecorated(), false);
    QCOMPARE(cl.chaserID(), Function::invalidId());

    QCOMPARE(cl.m_nextLatestValue, quint32(0));
    QCOMPARE(cl.m_previousLatestValue, quint32(0));
    QCOMPARE(cl.m_playbackLatestValue, quint32(0));

    QCOMPARE(cl.m_nextKeySequence, QKeySequence());
    QCOMPARE(cl.m_previousKeySequence, QKeySequence());
    QCOMPARE(cl.m_playbackKeySequence, QKeySequence());

    QVERIFY(cl.inputSource(VCCueList::nextInputSourceId) == NULL);
    QVERIFY(cl.inputSource(VCCueList::previousInputSourceId) == NULL);
    QVERIFY(cl.inputSource(VCCueList::playbackInputSourceId) == NULL);
}

void VCCueList_Test::chaser()
{
    QWidget w;
    VCCueList cl(&w, m_doc);
    Chaser* c = createChaser(m_doc);

    // Try to put a non-chaser as the chaser
    cl.setChaser(c->steps().first().fid);
    QCOMPARE(cl.chaserID(), Function::invalidId());
    QCOMPARE(cl.m_tree->topLevelItemCount(), 0);

    // Put a real chaser as the chaser
    cl.setChaser(c->id());
    QCOMPARE(cl.chaserID(), c->id());
    QCOMPARE(cl.m_tree->topLevelItemCount(), 4);
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(0), QString("1"));
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(0), QString("2"));
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(0), QString("3"));
    QCOMPARE(cl.m_tree->topLevelItem(3)->text(0), QString("4"));
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(1), QString("First"));
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(1), QString("Second"));
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(1), QString("Third"));
    QCOMPARE(cl.m_tree->topLevelItem(3)->text(1), QString("Fourth"));
}

void VCCueList_Test::functionRemoved()
{
    QWidget w;
    VCCueList cl(&w, m_doc);
    Chaser* c = createChaser(m_doc);
    cl.setChaser(c->id());

    // Chaser members are removed from list
    m_doc->deleteFunction(c->steps().first().fid);
    QCOMPARE(cl.m_tree->topLevelItemCount(), 4);
    // deferred changes arrive afetr 100ms
    QTest::qWait(150);
    QCOMPARE(cl.m_tree->topLevelItemCount(), 3);
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(0), QString("1"));
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(0), QString("2"));
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(0), QString("3"));
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(1), QString("Second"));
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(1), QString("Third"));
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(1), QString("Fourth"));

    // Chaser is removed completely
    m_doc->deleteFunction(c->id());
    QCOMPARE(cl.m_tree->topLevelItemCount(), 0);
    QCOMPARE(cl.chaserID(), Function::invalidId());
}

void VCCueList_Test::functionChanged()
{
}

void VCCueList_Test::keySequences()
{
    QWidget w;

    VCCueList cl(&w, m_doc);
    cl.setNextKeySequence(QKeySequence(keySequenceB));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(keySequenceB));
    QCOMPARE(cl.previousKeySequence(), QKeySequence());
    QCOMPARE(cl.playbackKeySequence(), QKeySequence());

    cl.setPreviousKeySequence(QKeySequence(keySequenceA));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(keySequenceB));
    QCOMPARE(cl.previousKeySequence(), QKeySequence(keySequenceA));
    QCOMPARE(cl.playbackKeySequence(), QKeySequence());

    cl.setPlaybackKeySequence(QKeySequence(keySequenceD));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(keySequenceB));
    QCOMPARE(cl.previousKeySequence(), QKeySequence(keySequenceA));
    QCOMPARE(cl.playbackKeySequence(), QKeySequence(keySequenceD));
}

void VCCueList_Test::copy()
{
    // Input sources are tested by VCWidget tests. No point testing here.

    QWidget w;
    VCFrame parent(&w, m_doc);
    VCCueList cl(&parent, m_doc);
    Chaser* c = createChaser(m_doc);

    cl.setChaser(c->id());
    cl.setCaption("Wheeee");
    cl.setNextKeySequence(QKeySequence(keySequenceB));
    cl.setPreviousKeySequence(QKeySequence(keySequenceA));
    cl.setPlaybackKeySequence(QKeySequence(keySequenceC));

    VCCueList* cl2 = qobject_cast<VCCueList*> (cl.createCopy(&parent));
    QVERIFY(cl2 != NULL);
    QCOMPARE(cl2->caption(), QString("Wheeee"));
    QCOMPARE(cl2->nextKeySequence(), QKeySequence(keySequenceB));
    QCOMPARE(cl2->previousKeySequence(), QKeySequence(keySequenceA));
    QCOMPARE(cl2->playbackKeySequence(), QKeySequence(keySequenceC));
    QCOMPARE(cl2->chaserID(), c->id());
    QCOMPARE(cl2->m_tree->topLevelItemCount(), 4);

    VCCueList cl3(&parent, m_doc);
    cl3.copyFrom(NULL);
    QCOMPARE(cl3.m_tree->topLevelItemCount(), 0);
    QCOMPARE(cl3.caption(), tr("Cue list"));

    cl.copyFrom(&cl3);
    QCOMPARE(cl.caption(), cl3.caption());
    QCOMPARE(cl.chaserID(), Function::invalidId());
    QCOMPARE(cl.m_tree->topLevelItemCount(), 0);
    QCOMPARE(cl.nextKeySequence(), QKeySequence());
    QCOMPARE(cl.previousKeySequence(), QKeySequence());
    QCOMPARE(cl.playbackKeySequence(), QKeySequence());

    delete cl2;
}

void VCCueList_Test::modeChange()
{
    QWidget w;
    VCFrame parent(&w, m_doc);
    VCCueList cl(&parent, m_doc);
    Chaser* c = createChaser(m_doc);
    cl.setChaser(c->id());

    m_doc->setMode(Doc::Operate);
    // QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 1);
    // QCOMPARE(m_doc->masterTimer()->m_dmxSourceList[0], &cl);
    // QVERIFY(cl.m_runner == NULL);
    QVERIFY(cl.m_tree->isEnabled() == true);

    // cl.createRunner();

    m_doc->setMode(Doc::Design);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
    // QVERIFY(cl.m_runner == NULL);
    QVERIFY(cl.m_tree->isEnabled() == false);
}

void VCCueList_Test::loadXML()
{
    QWidget w;

    Scene* s1 = new Scene(m_doc);
    s1->setName("The first");
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    s2->setName("Another one");
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    s3->setName("The third one");
    m_doc->addFunction(s3);

    Chaser* c4 = new Chaser(m_doc);
    c4->setName("The defiant one");
    m_doc->addFunction(c4);

    QDomDocument xmldoc;

    QDomElement root = xmldoc.createElement("CueList");
    root.setAttribute("Caption", "Test list");
    xmldoc.appendChild(root);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("Width", "42");
    wstate.setAttribute("Height", "69");
    wstate.setAttribute("X", "3");
    wstate.setAttribute("Y", "4");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement appearance = xmldoc.createElement("Appearance");
    QFont f(w.font());
    f.setPointSize(f.pointSize() + 3);
    QDomElement font = xmldoc.createElement("Font");
    QDomText fontText = xmldoc.createTextNode(f.toString());
    font.appendChild(fontText);
    appearance.appendChild(font);
    root.appendChild(appearance);

    QDomElement next = xmldoc.createElement("Next");
    QDomElement nextInput = xmldoc.createElement("Input");
    nextInput.setAttribute("Universe", "0");
    nextInput.setAttribute("Channel", "1");
    next.appendChild(nextInput);
    QDomElement nextKey = xmldoc.createElement("Key");
    QDomText nextKeyText = xmldoc.createTextNode(QKeySequence(keySequenceD).toString());
    nextKey.appendChild(nextKeyText);
    next.appendChild(nextKey);
    QDomElement nextFoo = xmldoc.createElement("Foo");
    next.appendChild(nextFoo);
    root.appendChild(next);

    QDomElement previous = xmldoc.createElement("Previous");
    QDomElement previousInput = xmldoc.createElement("Input");
    previousInput.setAttribute("Universe", "2");
    previousInput.setAttribute("Channel", "3");
    previous.appendChild(previousInput);
    QDomElement previousKey = xmldoc.createElement("Key");
    QDomText previousKeyText = xmldoc.createTextNode(QKeySequence(keySequenceC).toString());
    previousKey.appendChild(previousKeyText);
    previous.appendChild(previousKey);
    QDomElement previousFoo = xmldoc.createElement("Foo");
    previous.appendChild(previousFoo);
    root.appendChild(previous);

    QDomElement playback = xmldoc.createElement("Playback");
    QDomElement playbackInput = xmldoc.createElement("Input");
    playbackInput.setAttribute("Universe", "4");
    playbackInput.setAttribute("Channel", "5");
    playback.appendChild(playbackInput);
    QDomElement playbackKey = xmldoc.createElement("Key");
    QDomText playbackKeyText = xmldoc.createTextNode(QKeySequence(keySequenceA).toString());
    playbackKey.appendChild(playbackKeyText);
    playback.appendChild(playbackKey);
    QDomElement playbackFoo = xmldoc.createElement("Foo");
    playback.appendChild(playbackFoo);
    root.appendChild(playback);

    QDomElement f1 = xmldoc.createElement("Function");
    QDomText f1Text = xmldoc.createTextNode(QString::number(s1->id()));
    f1.appendChild(f1Text);
    root.appendChild(f1);

    QDomElement f2 = xmldoc.createElement("Function");
    QDomText f2Text = xmldoc.createTextNode(QString::number(s2->id()));
    f2.appendChild(f2Text);
    root.appendChild(f2);

    QDomElement f3 = xmldoc.createElement("Function");
    QDomText f3Text = xmldoc.createTextNode(QString::number(s3->id()));
    f3.appendChild(f3Text);
    root.appendChild(f3);

    QDomElement f4 = xmldoc.createElement("Function");
    QDomText f4Text = xmldoc.createTextNode(QString::number(c4->id()));
    f4.appendChild(f4Text);
    root.appendChild(f4);

    QDomElement f5 = xmldoc.createElement("Function");
    QDomText f5Text = xmldoc.createTextNode(QString::number(INT_MAX - 1));
    f5.appendChild(f5Text);
    root.appendChild(f5);

    // Make sure that nonexistent (id:31337) functions don't appear in the list
    QDomElement f6 = xmldoc.createElement("Function");
    QDomText f6Text = xmldoc.createTextNode(QString::number(31337));
    f6.appendChild(f6Text);
    root.appendChild(f6);

    QDomElement foo = xmldoc.createElement("Foobar");
    root.appendChild(foo);

    VCCueList cl(&w, m_doc);
    QVERIFY(cl.loadXML(&root) == true);
    QCOMPARE(cl.m_tree->topLevelItemCount(), 4);
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(0).toInt(), 1);
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(0).toInt(), 2);
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(0).toInt(), 3);
    QCOMPARE(cl.m_tree->topLevelItem(3)->text(0).toInt(), 4);
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(1), s3->name());
    QCOMPARE(cl.m_tree->topLevelItem(3)->text(1), c4->name());
    QLCInputSource *ni = cl.inputSource(VCCueList::nextInputSourceId);
    QCOMPARE(ni->universe(), quint32(0));
    QCOMPARE(ni->channel(), quint32(1));
    QCOMPARE(cl.nextKeySequence(), QKeySequence(keySequenceD));
    QLCInputSource *pi = cl.inputSource(VCCueList::previousInputSourceId);
    QCOMPARE(pi->universe(), quint32(2));
    QCOMPARE(pi->channel(), quint32(3));
    QCOMPARE(cl.previousKeySequence(), QKeySequence(keySequenceC));
    QLCInputSource *pli = cl.inputSource(VCCueList::playbackInputSourceId);
    QCOMPARE(pli->universe(), quint32(4));
    QCOMPARE(pli->channel(), quint32(5));
    QCOMPARE(cl.playbackKeySequence(), QKeySequence(keySequenceA));

    QCOMPARE(cl.pos(), QPoint(3, 4));
    QCOMPARE(cl.size(), QSize(42, 69));
    QCOMPARE(cl.font(), f);

    cl.postLoad();
    QCOMPARE(cl.m_tree->topLevelItemCount(), 4);
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(0).toInt(), 1);
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(0).toInt(), 2);
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(0).toInt(), 3);
    QCOMPARE(cl.m_tree->topLevelItem(0)->text(1), s1->name());
    QCOMPARE(cl.m_tree->topLevelItem(1)->text(1), s2->name());
    QCOMPARE(cl.m_tree->topLevelItem(2)->text(1), s3->name());
    QCOMPARE(cl.m_tree->topLevelItem(3)->text(1), c4->name());

    root.setTagName("CueLits");
    QVERIFY(cl.loadXML(&root) == false);
}

void VCCueList_Test::saveXML()
{
    QWidget w;
    VCCueList cl(&w, m_doc);
    Chaser* c = createChaser(m_doc);
    cl.setChaser(c->id());

    cl.setCaption("Testing");
    cl.setInputSource(new QLCInputSource(0, 1), VCCueList::nextInputSourceId);
    cl.setInputSource(new QLCInputSource(1, 2), VCCueList::previousInputSourceId);
    cl.setInputSource(new QLCInputSource(2, 3), VCCueList::playbackInputSourceId);
    cl.setNextKeySequence(QKeySequence(keySequenceB));
    cl.setPreviousKeySequence(QKeySequence(keySequenceA));
    cl.setPlaybackKeySequence(QKeySequence(keySequenceC));

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);

    int chaser = 0, next = 0, nextKey = 0, nextInput = 0, previous = 0, previousKey = 0,
        previousInput = 0, playback = 0, playbackKey = 0, playbackInput = 0, wstate = 0, appearance = 0;

    QVERIFY(cl.saveXML(&xmldoc, &root) == true);
    QDomElement clroot = root.firstChild().toElement();
    QCOMPARE(clroot.tagName(), QString("CueList"));
    QCOMPARE(clroot.attribute("Caption"), QString("Testing"));

    QDomNode node = clroot.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Chaser")
        {
            QCOMPARE(tag.text().toUInt(), c->id());
            chaser++;
        }
        else if (tag.tagName() == "Function")
        {
            QFAIL("Function node should not be written anymore!");
        }
        else if (tag.tagName() == "Next")
        {
            next++;
            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "Key")
                {
                    nextKey++;
                    QCOMPARE(subtag.text(), QKeySequence(keySequenceB).toString());
                }
                else if (subtag.tagName() == "Input")
                {
                    nextInput++;
                    // Handled by VCWidget tests, just check that the node is there
                }
                else
                {
                    QFAIL(QString("Unexpected tag: %1").arg(subtag.tagName()).toUtf8().constData());
                }

                subnode = subnode.nextSibling();
            }
        }
        else if (tag.tagName() == "Previous")
        {
            previous++;
            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "Key")
                {
                    previousKey++;
                    QCOMPARE(subtag.text(), QKeySequence(keySequenceA).toString());
                }
                else if (subtag.tagName() == "Input")
                {
                    previousInput++;
                    // Handled by VCWidget tests, just check that the node is there
                }
                else
                {
                    QFAIL(QString("Unexpected tag: %1").arg(subtag.tagName()).toUtf8().constData());
                }

                subnode = subnode.nextSibling();
            }
        }
        else if (tag.tagName() == "Playback")
        {
            playback++;
            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "Key")
                {
                    playbackKey++;
                    QCOMPARE(subtag.text(), QKeySequence(keySequenceC).toString());
                }
                else if (subtag.tagName() == "Input")
                {
                    playbackInput++;
                    // Handled by VCWidget tests, just check that the node is there
                }
                else
                {
                    QFAIL(QString("Unexpected tag: %1").arg(subtag.tagName()).toUtf8().constData());
                }

                subnode = subnode.nextSibling();
            }
        }
        else if (tag.tagName() == "WindowState")
        {
            // Handled by VCWidget tests, just check that the node is there
            wstate++;
        }
        else if (tag.tagName() == "Appearance")
        {
            // Handled by VCWidget tests, just check that the node is there
            appearance++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(chaser, 1);
    QCOMPARE(next, 1);
    QCOMPARE(nextKey, 1);
    QCOMPARE(nextInput, 1);
    QCOMPARE(previous, 1);
    QCOMPARE(previousKey, 1);
    QCOMPARE(previousInput, 1);
    QCOMPARE(playback, 1);
    QCOMPARE(playbackKey, 1);
    QCOMPARE(playbackInput, 1);
    QCOMPARE(wstate, 1);
    QCOMPARE(appearance, 1);
}

void VCCueList_Test::nextPrevious()
{
    QWidget w;

    VCCueList cl(&w, m_doc);
    Chaser* c = createChaser(m_doc);
    c->setDuration(Function::infiniteSpeed());
    cl.setChaser(c->id());
    QCOMPARE(c->steps().size(), 4);
    Scene* s1 = qobject_cast<Scene*> (m_doc->function(c->steps()[0].fid));
    Scene* s2 = qobject_cast<Scene*> (m_doc->function(c->steps()[1].fid));
    Scene* s3 = qobject_cast<Scene*> (m_doc->function(c->steps()[2].fid));
    Scene* s4 = qobject_cast<Scene*> (m_doc->function(c->steps()[3].fid));
    Q_ASSERT(s1 && s2 && s3 && s4);

    // Not in operate mode, check for crashes
    cl.slotNextCue();
    cl.slotPreviousCue();
    cl.slotItemActivated(cl.m_tree->topLevelItem(2));
    // QVERIFY(cl.m_runner == NULL);

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    // Create runner with a next action -> first item should be activated
    cl.slotNextCue();
    // QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);

    cl.slotNextCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s2);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s2);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s2);

    cl.slotNextCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);

    cl.slotPreviousCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s2);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s2);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s2);

    cl.slotPreviousCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);

    // Wrap around to the last cue
    cl.slotPreviousCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s4);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s4);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s4);

    // Wrap around to the next cue
    cl.slotNextCue();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
}

void VCCueList_Test::manualActivation()
{
    QWidget w;
    VCCueList cl(&w, m_doc);
    Chaser* c = createChaser(m_doc);
    c->setDuration(Function::infiniteSpeed());
    cl.setChaser(c->id());
    Scene* s1 = qobject_cast<Scene*> (m_doc->function(c->steps()[0].fid));
    //Scene* s2 = qobject_cast<Scene*> (m_doc->function(c->steps()[1].fid));
    Scene* s3 = qobject_cast<Scene*> (m_doc->function(c->steps()[2].fid));
    //Scene* s4 = qobject_cast<Scene*> (m_doc->function(c->steps()[3].fid));
    Q_ASSERT(s1 /*&& s2*/ && s3/* && s4*/);

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    // QVERIFY(cl.m_runner == NULL);
    cl.slotItemActivated(cl.m_tree->topLevelItem(2));
    // QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);

    // Same item
    cl.slotItemActivated(cl.m_tree->topLevelItem(2));
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s3);

    // Another item
    cl.slotItemActivated(cl.m_tree->topLevelItem(0));
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);

    // Crash check
    cl.slotItemActivated(NULL);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
    timer->timerTick();
    QCOMPARE(timer->runningFunctions(), 2);
    QCOMPARE(timer->m_functionList[1], s1);
}

void VCCueList_Test::keyboardNextPrevious()
{
    QWidget w;
    VCCueList cl(&w, m_doc);
    Chaser* c = createChaser(m_doc);
    c->setDuration(Function::infiniteSpeed());
    cl.setChaser(c->id());

    cl.setNextKeySequence(QKeySequence(keySequenceB));
    cl.setPreviousKeySequence(QKeySequence(keySequenceA));
    cl.setPlaybackKeySequence(QKeySequence(keySequenceD));

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    // Next keyboard key
    cl.slotKeyPressed(QKeySequence(keySequenceB));
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 0);
    QCOMPARE(cl.m_tree->indexOfTopLevelItem(cl.m_tree->currentItem()), 0);

    // Next keyboard key
    cl.slotKeyPressed(QKeySequence(keySequenceB));
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 1);
    QCOMPARE(cl.m_tree->indexOfTopLevelItem(cl.m_tree->currentItem()), 1);

    // Unrecognized keyboard key
    cl.slotKeyPressed(QKeySequence(QKeySequence::SelectAll));
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 1);
    QCOMPARE(cl.m_tree->indexOfTopLevelItem(cl.m_tree->currentItem()), 1);

    // Previous keyboard key
    cl.slotKeyPressed(QKeySequence(keySequenceA));
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 0);
    QCOMPARE(cl.m_tree->indexOfTopLevelItem(cl.m_tree->currentItem()), 0);

    // Previous keyboard key
    cl.slotKeyPressed(QKeySequence(keySequenceA));
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 3);
    QCOMPARE(cl.m_tree->indexOfTopLevelItem(cl.m_tree->currentItem()), 3);

    // Next keyboard key
    cl.slotKeyPressed(QKeySequence(keySequenceB));
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 0);
    QCOMPARE(cl.m_tree->indexOfTopLevelItem(cl.m_tree->currentItem()), 0);

    // Playback
    cl.slotKeyPressed(QKeySequence(keySequenceD));
    timer->timerTick();
    //QVERIFY(cl.m_runner == NULL);
    QCOMPARE(cl.m_tree->indexOfTopLevelItem(cl.m_tree->currentItem()), 0);
}

void VCCueList_Test::input()
{
    QWidget w;
    VCCueList cl(&w, m_doc);
    Chaser* c = createChaser(m_doc);
    c->setDuration(Function::infiniteSpeed());
    cl.setChaser(c->id());

    cl.setInputSource(new QLCInputSource(0, 1), VCCueList::nextInputSourceId);
    cl.setInputSource(new QLCInputSource(2, 3), VCCueList::previousInputSourceId);
    cl.setInputSource(new QLCInputSource(4, 5), VCCueList::playbackInputSourceId);

    // Switch mode
    m_doc->setMode(Doc::Operate);
    MasterTimer* timer = m_doc->masterTimer();

    // Runner creation thru "next" input
    cl.slotInputValueChanged(5, 3, 255);
    //QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(2, 15, 255);
    //QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(0, 1, 255);
    //QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 0);

    cl.slotInputValueChanged(0, 1, 0);
    //QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 0);

    cl.slotInputValueChanged(0, 1, 255);
    //QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 1);

    // Runner creation thru previous input
    m_doc->setMode(Doc::Design);
    //QVERIFY(cl.m_runner == NULL);
    m_doc->setMode(Doc::Operate);

    cl.slotInputValueChanged(0, 3, 255);
    //QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(2, 1, 255);
    //QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(2, 3, 255);
    //QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 3);

    cl.slotInputValueChanged(2, 3, 0);
    //QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 3);

    cl.slotInputValueChanged(2, 3, 255);
    //QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    //QCOMPARE(cl.m_runner->currentStepIndex(), 2);

    cl.slotInputValueChanged(4, 5, 255);
    //QVERIFY(cl.m_runner != NULL);
    timer->timerTick();
    //QVERIFY(cl.m_runner == NULL);

    cl.slotInputValueChanged(4, 5, 0);
    //QVERIFY(cl.m_runner == NULL);
    timer->timerTick();
    //QVERIFY(cl.m_runner == NULL);
}

QTEST_MAIN(VCCueList_Test)
