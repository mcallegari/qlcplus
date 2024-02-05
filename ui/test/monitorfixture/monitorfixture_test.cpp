/*
  Q Light Controller
  monitorfixture_test.cpp

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

#include <QLabel>
#include <QtTest>

#define protected public
#define private public
#include "monitorfixture.h"
#undef protected
#undef private

#include "monitorfixture_test.h"
#include "qlcfixturedefcache.h"
#include "qlcmacros.h"
#include "doc.h"

void MonitorFixture_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_currentAddr = 0;
}

void MonitorFixture_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void MonitorFixture_Test::initial()
{
    QWidget w;

    MonitorFixture mof(&w, m_doc);
    QCOMPARE(mof.fixture(), Fixture::invalidId());
    QVERIFY(mof.m_fixtureLabel == NULL);
    QCOMPARE(mof.m_channelStyle, MonitorProperties::DMXChannels);
    QCOMPARE(mof.m_valueStyle, MonitorProperties::DMXValues);
    QCOMPARE(mof.frameStyle(), QFrame::StyledPanel | QFrame::Sunken);
    QVERIFY(mof.layout() != NULL);
    QCOMPARE(mof.m_channelLabels.size(), 0);
    QCOMPARE(mof.m_valueLabels.size(), 0);
    QCOMPARE(mof.autoFillBackground(), true);
    QCOMPARE(mof.backgroundRole(), QPalette::Window);
}

void MonitorFixture_Test::fixture()
{
    QWidget w;

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(6);
    fxi->setAddress(m_currentAddr);
    fxi->setName("Foobar");
    m_doc->addFixture(fxi);
    QVERIFY(fxi->id() != Fixture::invalidId());
    m_currentAddr += fxi->channels();

    MonitorFixture mof(&w, m_doc);
    mof.setFixture(fxi->id());
    QCOMPARE(mof.fixture(), fxi->id());
    QVERIFY(mof.m_fixtureLabel != NULL);
    QCOMPARE(mof.m_fixtureLabel->text(), QString("<B>Foobar</B>"));
    QCOMPARE(mof.m_channelLabels.size(), 6);
    QCOMPARE(mof.m_valueLabels.size(), 6);
    for (int i = 0; i < mof.m_channelLabels.size(); i++)
    {
        QVERIFY(mof.m_channelLabels[i] != NULL);
        QCOMPARE(mof.m_channelLabels[i]->text(), QString());

        QVERIFY(mof.m_valueLabels[i] != NULL);
        QCOMPARE(mof.m_valueLabels[i]->text(), QString("000"));
    }
}

void MonitorFixture_Test::lessThan()
{
    QWidget w;

    Fixture* fxi1 = new Fixture(m_doc);
    fxi1->setChannels(6);
    fxi1->setName("Foo");
    fxi1->setAddress(m_currentAddr);
    m_doc->addFixture(fxi1);
    QVERIFY(fxi1->id() != Fixture::invalidId());
    m_currentAddr += fxi1->channels();

    MonitorFixture mof1(&w, m_doc);
    mof1.setFixture(fxi1->id());

    Fixture* fxi2 = new Fixture(m_doc);
    fxi2->setChannels(4);
    fxi2->setName("Bar");
    fxi2->setAddress(m_currentAddr);
    m_doc->addFixture(fxi2);
    QVERIFY(fxi2->id() != Fixture::invalidId());
    m_currentAddr += fxi2->channels();

    MonitorFixture mof2(&w, m_doc);
    mof2.setFixture(fxi2->id());

    QVERIFY(mof1 < mof2);
    QVERIFY(!(mof2 < mof1));

    fxi1->setAddress(1000);
    fxi2->setAddress(500);

    QVERIFY(mof1 < mof2);
    QVERIFY(!(mof2 < mof1));

    fxi1->setAddress(507);
    fxi2->setAddress(500);

    QVERIFY(mof2 < mof1);
    QVERIFY(!(mof1 < mof2));
}

void MonitorFixture_Test::channelValueStyles()
{
    QWidget w;

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(6);
    fxi->setAddress(m_currentAddr);
    fxi->setName("Foobar");
    m_doc->addFixture(fxi);
    QVERIFY(fxi->id() != Fixture::invalidId());
    m_currentAddr += fxi->channels();

    MonitorFixture mof(&w, m_doc);
    mof.setFixture(fxi->id());

    mof.updateLabelStyles();
    for (int i = 0; i < mof.m_channelLabels.size(); i++)
    {
        QString str;
        QVERIFY(mof.m_channelLabels[i] != NULL);
        QCOMPARE(mof.m_channelLabels[i]->text(), str.asprintf("<B>%.3d</B>", i + fxi->address() + 1));

        QVERIFY(mof.m_valueLabels[i] != NULL);
        QCOMPARE(mof.m_valueLabels[i]->text(), QString("000"));
    }

    mof.slotChannelStyleChanged(MonitorProperties::RelativeChannels);
    mof.updateLabelStyles();
    for (int i = 0; i < mof.m_channelLabels.size(); i++)
    {
        QString str;
        QVERIFY(mof.m_channelLabels[i] != NULL);
        QCOMPARE(mof.m_channelLabels[i]->text(), str.asprintf("<B>%.3d</B>", i + 1));

        QVERIFY(mof.m_valueLabels[i] != NULL);
        QCOMPARE(mof.m_valueLabels[i]->text(), QString("000"));
    }

    mof.slotChannelStyleChanged(MonitorProperties::DMXChannels);
    for (int i = 0; i < mof.m_channelLabels.size(); i++)
    {
        QString str;
        QVERIFY(mof.m_channelLabels[i] != NULL);
        QCOMPARE(mof.m_channelLabels[i]->text(), str.asprintf("<B>%.3d</B>", i + fxi->address() + 1));

        QVERIFY(mof.m_valueLabels[i] != NULL);
        QCOMPARE(mof.m_valueLabels[i]->text(), QString("000"));
    }

    for (int i = 0; i < mof.m_valueLabels.size(); i++)
    {
        QString str;
        mof.m_valueLabels[i]->setText(str.asprintf("%.3d", (i + 1) * 10));
    }

    mof.slotValueStyleChanged(MonitorProperties::PercentageValues);
    for (int i = 0; i < mof.m_channelLabels.size(); i++)
    {
        QString str;
        QVERIFY(mof.m_channelLabels[i] != NULL);
        QCOMPARE(mof.m_channelLabels[i]->text(), str.asprintf("<B>%.3d</B>", i + fxi->address() + 1));

        QVERIFY(mof.m_valueLabels[i] != NULL);
        QCOMPARE(mof.m_valueLabels[i]->text(), str.asprintf("%.3d", (int) ceil(SCALE(qreal((i + 1) * 10),
                                                                                     qreal(0), qreal(UCHAR_MAX),
                                                                                     qreal(0), qreal(100)))));
    }
}

void MonitorFixture_Test::updateValues()
{
    QWidget w;

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(6);
    fxi->setAddress(m_currentAddr);
    fxi->setName("Foobar");
    m_doc->addFixture(fxi);
    QVERIFY(fxi->id() != Fixture::invalidId());
    m_currentAddr += fxi->channels();

    QByteArray ba(512, 0);
    for (int i = 0; i < 6; i++)
        ba[i + fxi->address()] = 127 + i;
    fxi->setChannelValues(ba);

    MonitorFixture mof(&w, m_doc);
    mof.setFixture(fxi->id());

    for (int i = 0; i < mof.m_valueLabels.size(); i++)
    {
        QString str;
        QCOMPARE(mof.m_valueLabels[i]->text(), str.asprintf("%.3d", 127 + i));
    }

    mof.slotValueStyleChanged(MonitorProperties::PercentageValues);
    for (int i = 0; i < mof.m_valueLabels.size(); i++)
    {
        QString str;
        QCOMPARE(mof.m_valueLabels[i]->text(), str.asprintf("%.3d",
            int(ceil(SCALE(qreal(127 + i), qreal(0), qreal(UCHAR_MAX), qreal(0), qreal(100))))));
    }
}

QTEST_MAIN(MonitorFixture_Test)
