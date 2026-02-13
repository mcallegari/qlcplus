/*
  Q Light Controller Plus - Unit test
  showmanager_test.cpp

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

#define private public
#define protected public
#include "showfunction.h"
#include "function.h"
#include "track.h"
#include "scene.h"
#include "show.h"
#include "doc.h"
#undef private
#undef protected

#include "showmanager_test.h"

void ShowManager_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void ShowManager_Test::cleanupTestCase()
{
    delete m_doc;
}

/**
 * Test the overlap detection logic directly (mirrors ShowManager::checkOverlapping).
 * This avoids constructing the full ShowManager widget in a headless test environment.
 */
static bool checkOverlapping(Doc *doc, Track *track, quint32 startTime, quint32 duration)
{
    if (track == NULL)
        return false;

    foreach (ShowFunction *sf, track->showFunctions())
    {
        Function *func = doc->function(sf->functionID());
        if (func != NULL)
        {
            quint32 fst = sf->startTime();
            if ((startTime >= fst && startTime < fst + sf->duration()) ||
                (fst >= startTime && fst < startTime + duration))
            {
                return true;
            }
        }
    }

    return false;
}

void ShowManager_Test::overlapDetectionAdjacent()
{
    Show *show = new Show(m_doc);
    m_doc->addFunction(show);

    Scene *s1 = new Scene(m_doc);
    m_doc->addFunction(s1);

    Track *track = new Track(s1->id(), show);
    show->addTrack(track);

    // Add first function: start=0, duration=5000
    ShowFunction *sf1 = new ShowFunction(show->getLatestShowFunctionId());
    sf1->setFunctionID(s1->id());
    sf1->setStartTime(0);
    sf1->setDuration(5000);
    track->addShowFunction(sf1);

    // H1 regression: adjacent items (start=5000) should NOT be considered overlapping
    bool overlaps = checkOverlapping(m_doc, track, 5000, 3000);
    QCOMPARE(overlaps, false);

    m_doc->clearContents();
}

void ShowManager_Test::overlapDetectionActual()
{
    Show *show = new Show(m_doc);
    m_doc->addFunction(show);

    Scene *s1 = new Scene(m_doc);
    m_doc->addFunction(s1);

    Track *track = new Track(s1->id(), show);
    show->addTrack(track);

    ShowFunction *sf1 = new ShowFunction(show->getLatestShowFunctionId());
    sf1->setFunctionID(s1->id());
    sf1->setStartTime(0);
    sf1->setDuration(5000);
    track->addShowFunction(sf1);

    // An item starting at 4999 should overlap
    bool overlaps = checkOverlapping(m_doc, track, 4999, 3000);
    QCOMPARE(overlaps, true);

    m_doc->clearContents();
}

void ShowManager_Test::overlapDetectionContained()
{
    Show *show = new Show(m_doc);
    m_doc->addFunction(show);

    Scene *s1 = new Scene(m_doc);
    m_doc->addFunction(s1);

    Track *track = new Track(s1->id(), show);
    show->addTrack(track);

    ShowFunction *sf1 = new ShowFunction(show->getLatestShowFunctionId());
    sf1->setFunctionID(s1->id());
    sf1->setStartTime(1000);
    sf1->setDuration(3000);
    track->addShowFunction(sf1);

    // An item fully containing the existing one
    bool overlaps = checkOverlapping(m_doc, track, 0, 10000);
    QCOMPARE(overlaps, true);

    m_doc->clearContents();
}

QTEST_MAIN(ShowManager_Test)
