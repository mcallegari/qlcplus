/*
  Q Light Controller Plus - Unit test
  genericdmxsource_test.cpp

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

#define protected public
#define private public
#include "genericdmxsource_test.h"
#include "genericdmxsource.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "mastertimer.h"
#include "universe.h"
#include "doc.h"
#include "fixture.h"
#undef private
#undef protected

void GenericDMXSource_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void GenericDMXSource_Test::cleanupTestCase()
{
    delete m_doc;
}

void GenericDMXSource_Test::init()
{
    m_doc->clearContents();
    Fixture *fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    fxi->setAddress(0);
    m_doc->addFixture(fxi);
    m_fxiId = fxi->id();
}

void GenericDMXSource_Test::cleanup()
{
    m_doc->clearContents();
}

void GenericDMXSource_Test::setUnset()
{
    GenericDMXSource src(m_doc);
    src.setOutputEnabled(true);
    src.set(m_fxiId, 0, 100);
    QCOMPARE(src.channelsCount(), quint32(1));
    QList<SceneValue> list = src.channels();
    QCOMPARE(list.size(), 1);
    QCOMPARE(list.first().fxi, m_fxiId);
    QCOMPARE(list.first().channel, quint32(0));
    QCOMPARE(list.first().value, uchar(100));

    src.unset(m_fxiId, 0);
    QCOMPARE(src.channelsCount(), quint32(0));
}

void GenericDMXSource_Test::unsetAll()
{
    GenericDMXSource src(m_doc);
    src.setOutputEnabled(true);
    src.set(m_fxiId, 0, 50);
    src.writeDMX(nullptr, m_doc->inputOutputMap()->universes());
    QCOMPARE(src.channelsCount(), quint32(1));
    src.unsetAll();
    src.writeDMX(nullptr, m_doc->inputOutputMap()->universes());
    QCOMPARE(src.channelsCount(), quint32(0));
}

void GenericDMXSource_Test::crossUniverse()
{
    // 120 RGB pixels (360 channels) at address 361: only 152 channels left
    auto createRow = [this](quint32 universe, Fixture::CrossUniverseMode mode)
    {
        Fixture *fxi = new Fixture(m_doc);
        QLCFixtureDef *rowDef = fxi->genericRGBPanelDef(120, Fixture::RGB, false);
        QLCFixtureMode *rowMode = fxi->genericRGBPanelMode(rowDef, Fixture::RGB, false, 3000, 50);
        fxi->setFixtureDefinition(rowDef, rowMode);
        fxi->setUniverse(universe);
        fxi->setAddress(360);
        fxi->setCrossUniverse(mode);
        m_doc->addFixture(fxi);
        return fxi->id();
    };

    // Pixel: U0 360-509 + U1 0-209
    quint32 pixelID = createRow(0, Fixture::CrossUniversePixel);
    // Fill: U2 360-511 + U3 0-207
    quint32 fillID = createRow(2, Fixture::CrossUniverseFill);

    GenericDMXSource src(m_doc);
    src.setOutputEnabled(true);
    src.set(pixelID, 149, 11);
    src.set(pixelID, 150, 22);
    src.set(pixelID, 359, 33);
    src.set(fillID, 151, 44);
    src.set(fillID, 152, 55);
    src.set(fillID, 359, 66);

    QList<Universe *> ua = m_doc->inputOutputMap()->universes();
    src.writeDMX(nullptr, ua);
    for (Universe *universe : ua)
        universe->processFaders(MasterTimer::tick());

    /* Pixel */
    QCOMPARE(uchar(ua[0]->preGMValues()[509]), uchar(11));
    QCOMPARE(uchar(ua[0]->preGMValues()[510]), uchar(0)); // unused gap
    QCOMPARE(uchar(ua[0]->preGMValues()[511]), uchar(0)); // unused gap
    QCOMPARE(uchar(ua[1]->preGMValues()[0]), uchar(22));
    QCOMPARE(uchar(ua[1]->preGMValues()[209]), uchar(33));

    /* Fill */
    QCOMPARE(uchar(ua[2]->preGMValues()[511]), uchar(44));
    QCOMPARE(uchar(ua[3]->preGMValues()[0]), uchar(55));
    QCOMPARE(uchar(ua[3]->preGMValues()[207]), uchar(66));
}

QTEST_MAIN(GenericDMXSource_Test)
