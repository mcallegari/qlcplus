/*
  Q Light Controller Plus - Unit tests
  channelsgroup_test.cpp

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QDir>

#define protected public
#define private public
#include "channelsgroup_test.h"
#include "channelsgroup.h"
#include "qlcinputsource.h"
#include "fixture.h"
#include "qlcfixturedef.h"
#include "qlcfixturemode.h"
#include "qlcfile.h"
#include "doc.h"
#undef private
#undef protected

#include "../common/resource_paths.h"

void ChannelsGroup_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
    m_currentAddr = 0;
}

void ChannelsGroup_Test::cleanupTestCase()
{
    delete m_doc;
}

void ChannelsGroup_Test::init()
{
    m_doc->clearContents();
    m_currentAddr = 0;
}

void ChannelsGroup_Test::initial()
{
    ChannelsGroup grp(m_doc);

    QCOMPARE(grp.id(), ChannelsGroup::invalidId());
    QCOMPARE(grp.name(), QString("New Group"));
    QVERIFY(grp.getChannels().isEmpty());
    QVERIFY(grp.inputSource().isNull());
}

void ChannelsGroup_Test::id()
{
    ChannelsGroup grp(m_doc);
    QSignalSpy spy(&grp, SIGNAL(changed(quint32)));

    grp.setId(10);
    QCOMPARE(grp.id(), quint32(10));
    QCOMPARE(spy.size(), 0);
}

void ChannelsGroup_Test::name()
{
    ChannelsGroup grp(m_doc);
    QSignalSpy spy(&grp, SIGNAL(changed(quint32)));

    grp.setName("MyGroup");
    QCOMPARE(grp.name(), QString("MyGroup"));
    QCOMPARE(spy.size(), 1);
}

void ChannelsGroup_Test::addChannel()
{
    ChannelsGroup grp(m_doc);
    QVERIFY(grp.addChannel(ChannelsGroup::invalidId(), 0) == false);
    QVERIFY(grp.getChannels().isEmpty());

    Fixture *fxi = new Fixture(m_doc);
    fxi->setChannels(4);
    fxi->setAddress(m_currentAddr);
    m_currentAddr += fxi->channels();
    m_doc->addFixture(fxi);

    QVERIFY(grp.addChannel(fxi->id(), 1) == true);
    QCOMPARE(grp.getChannels().size(), 1);
    SceneValue scv = grp.getChannels().first();
    QCOMPARE(scv.fxi, fxi->id());
    QCOMPARE(scv.channel, quint32(1));
}

void ChannelsGroup_Test::slotFixtureRemoved()
{
    ChannelsGroup grp(m_doc);
    QSignalSpy spy(&grp, SIGNAL(changed(quint32)));

    Fixture *f1 = new Fixture(m_doc);
    f1->setChannels(2);
    f1->setAddress(m_currentAddr);
    m_currentAddr += f1->channels();
    m_doc->addFixture(f1);

    Fixture *f2 = new Fixture(m_doc);
    f2->setChannels(2);
    f2->setAddress(m_currentAddr);
    m_currentAddr += f2->channels();
    m_doc->addFixture(f2);

    grp.addChannel(f1->id(), 0);
    grp.addChannel(f2->id(), 1);
    QCOMPARE(grp.getChannels().size(), 2);

    grp.slotFixtureRemoved(42);
    QCOMPARE(grp.getChannels().size(), 2);
    QCOMPARE(spy.size(), 0);

    grp.slotFixtureRemoved(f1->id());
    QCOMPARE(grp.getChannels().size(), 1);
    QCOMPARE(grp.getChannels().first().fxi, f2->id());
    QCOMPARE(spy.size(), 1);
}

void ChannelsGroup_Test::saveLoad()
{
    ChannelsGroup grp(m_doc);
    grp.setId(7);
    grp.setName("SaveGroup");

    Fixture *fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    fxi->setAddress(m_currentAddr);
    m_currentAddr += fxi->channels();
    m_doc->addFixture(fxi);

    grp.addChannel(fxi->id(), 0);
    grp.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(2, 3)));

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    QVERIFY(grp.saveXML(&xmlWriter));
    xmlWriter.setDevice(nullptr);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    ChannelsGroup grp2(m_doc);
    QVERIFY(grp2.loadXML(xmlReader) == true);
    QCOMPARE(grp2.id(), quint32(7));
    QCOMPARE(grp2.name(), QString("SaveGroup"));
    QCOMPARE(grp2.getChannels().size(), 1);
    SceneValue scv = grp2.getChannels().first();
    QCOMPARE(scv.fxi, fxi->id());
    QCOMPARE(scv.channel, quint32(0));
    QVERIFY(!grp2.inputSource().isNull());
    QCOMPARE(grp2.inputSource()->universe(), quint32(2));
    QCOMPARE(grp2.inputSource()->channel(), quint32(3));
}

void ChannelsGroup_Test::loader()
{
    Fixture *fxi = new Fixture(m_doc);
    fxi->setChannels(1);
    fxi->setAddress(m_currentAddr);
    m_currentAddr += fxi->channels();
    m_doc->addFixture(fxi);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    xmlWriter.writeStartElement(KXMLQLCChannelsGroup);
    xmlWriter.writeAttribute("ID", "5");
    xmlWriter.writeAttribute("Name", "LoadedGroup");
    xmlWriter.writeCharacters(QString("%1,0").arg(fxi->id()));
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(nullptr);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    int before = m_doc->channelsGroups().count();
    QVERIFY(ChannelsGroup::loader(xmlReader, m_doc) == true);
    QCOMPARE(m_doc->channelsGroups().count(), before + 1);
    ChannelsGroup *grp = m_doc->channelsGroups().last();
    QCOMPARE(grp->id(), quint32(5));
    QCOMPARE(grp->name(), QString("LoadedGroup"));
}

void ChannelsGroup_Test::inputSource()
{
    ChannelsGroup grp(m_doc);
    grp.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(1, 2)));

    QSignalSpy spy(&grp, SIGNAL(valueChanged(quint32,uchar)));

    m_doc->setMode(Doc::Design);
    grp.slotInputValueChanged(1, 2, 100);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).at(0).toUInt(), quint32(2));
    QCOMPARE(spy.at(0).at(1).toUInt(), quint32(100));

    grp.slotInputValueChanged(1, 3, 50);
    QCOMPARE(spy.size(), 1);

    m_doc->setMode(Doc::Operate);
    grp.slotInputValueChanged(1, 2, 50);
    QCOMPARE(spy.size(), 1);
}

QTEST_MAIN(ChannelsGroup_Test)
