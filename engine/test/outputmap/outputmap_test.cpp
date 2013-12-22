/*
  Q Light Controller - Unit test
  outputmap_test.cpp

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

#include <QSignalSpy>
#include <QtTest>

#define private public
#include "outputpluginstub.h"
#include "outputmap_test.h"
#include "qlcioplugin.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "universe.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define TESTPLUGINDIR "../outputpluginstub"
#define INPUT_TESTPLUGINDIR "../inputpluginstub"
#define ENGINEDIR "../../src"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

void OutputMap_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_doc->ioPluginCache()->load(testPluginDir());
    QVERIFY(m_doc->ioPluginCache()->plugins().size() != 0);
}

void OutputMap_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void OutputMap_Test::initial()
{
    OutputMap om(m_doc, 4);
    QVERIFY(om.m_universes == 4);
    QVERIFY(om.universes() == 4);
    QVERIFY(om.m_blackout == false);
    QVERIFY(om.blackout() == false);
    QVERIFY(om.m_universeArray.size() == 4);
    QVERIFY(om.m_universeChanged == false);
    QVERIFY(om.m_patch.size() == 4);
    QVERIFY(om.m_universeMutex.tryLock() == true);
    om.m_universeMutex.unlock();

    for (int u = 0; u < om.m_universeArray.size(); u++)
    {
        for (quint32 i = 0; i < 512; i++)
            QVERIFY(om.m_universeArray[u]->preGMValues().data()[i] == 0);
    }
}

void OutputMap_Test::setPatch()
{
    OutputMap om(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.setPatch(0, "Foobar", 0) == true);
    QVERIFY(om.patch(0)->plugin() == NULL);
    QVERIFY(om.patch(0)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(1)->plugin() == NULL);
    QVERIFY(om.patch(1)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(2)->plugin() == NULL);
    QVERIFY(om.patch(2)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(3)->plugin() == NULL);
    QVERIFY(om.patch(3)->output() == QLCIOPlugin::invalidLine());

    QVERIFY(om.setPatch(4, stub->name(), 0) == false);
    QVERIFY(om.patch(0)->plugin() == NULL);
    QVERIFY(om.patch(0)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(1)->plugin() == NULL);
    QVERIFY(om.patch(1)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(2)->plugin() == NULL);
    QVERIFY(om.patch(2)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(3)->plugin() == NULL);
    QVERIFY(om.patch(3)->output() == QLCIOPlugin::invalidLine());

    QVERIFY(om.setPatch(4, stub->name(), 4) == false);
    QVERIFY(om.patch(0)->plugin() == NULL);
    QVERIFY(om.patch(0)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(1)->plugin() == NULL);
    QVERIFY(om.patch(1)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(2)->plugin() == NULL);
    QVERIFY(om.patch(2)->output() == QLCIOPlugin::invalidLine());
    QVERIFY(om.patch(3)->plugin() == NULL);
    QVERIFY(om.patch(3)->output() == QLCIOPlugin::invalidLine());

    QVERIFY(om.setPatch(3, stub->name(), 0) == true);
    QVERIFY(om.patch(3)->plugin() == stub);
    QVERIFY(om.patch(3)->output() == 0);

    QVERIFY(om.setPatch(2, stub->name(), 1) == true);
    QVERIFY(om.patch(2)->plugin() == stub);
    QVERIFY(om.patch(2)->output() == 1);

    QVERIFY(om.setPatch(1, stub->name(), 2) == true);
    QVERIFY(om.patch(1)->plugin() == stub);
    QVERIFY(om.patch(1)->output() == 2);

    QVERIFY(om.setPatch(0, stub->name(), 3) == true);
    QVERIFY(om.patch(0)->plugin() == stub);
    QVERIFY(om.patch(0)->output() == 3);
}

void OutputMap_Test::claimReleaseDumpReset()
{
    OutputMap om(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    om.setPatch(0, stub->name(), 0);
    om.setPatch(1, stub->name(), 1);
    om.setPatch(2, stub->name(), 2);
    om.setPatch(3, stub->name(), 3);

    QList<Universe*> unis = om.claimUniverses();
    for (int i = 0; i < 512; i++)
        unis[0]->write(i, 'a');
    for (int i = 0; i < 512; i++)
        unis[1]->write(i, 'b');
    for (int i = 0; i < 512; i++)
        unis[2]->write(i, 'c');
    for (int i = 0; i < 512; i++)
        unis[3]->write(i, 'd');
    om.releaseUniverses();

    om.dumpUniverses();

    for (int i = 0; i < 512; i++)
        QCOMPARE(stub->m_universe.data()[i], 'a');

    for (int i = 512; i < 1024; i++)
        QCOMPARE(stub->m_universe.data()[i], 'b');

    for (int i = 1024; i < 1536; i++)
        QCOMPARE(stub->m_universe.data()[i], 'c');

    for (int i = 1536; i < 2048; i++)
        QCOMPARE(stub->m_universe.data()[i], 'd');

    om.resetUniverses();
    for (int u = 0; u < om.m_universeArray.size(); u++)
    {
        for (quint32 i = 0; i < 512; i++)
            QVERIFY(om.m_universeArray.at(u)->preGMValues().data()[i] == 0);
    }
}

void OutputMap_Test::blackout()
{
    OutputMap om(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    om.setPatch(0, stub->name(), 0);
    om.setPatch(1, stub->name(), 1);
    om.setPatch(2, stub->name(), 2);
    om.setPatch(3, stub->name(), 3);

    QList<Universe*> unis = om.claimUniverses();
    for (int i = 0; i < 512; i++)
        unis[0]->write(i, 'a');
    for (int i = 0; i < 512; i++)
        unis[1]->write(i, 'b');
    for (int i = 0; i < 512; i++)
        unis[2]->write(i, 'c');
    for (int i = 0; i < 512; i++)
        unis[3]->write(i, 'd');
    om.releaseUniverses();
    om.dumpUniverses();

    om.setBlackout(true);
    QVERIFY(om.blackout() == true);
    om.dumpUniverses();

    for (int i = 0; i < 2048; i++)
        QVERIFY(stub->m_universe[i] == (char) 0);

    om.setBlackout(true);
    QVERIFY(om.blackout() == true);
    om.dumpUniverses();

    for (int i = 0; i < 2048; i++)
        QVERIFY(stub->m_universe[i] == (char) 0);

    om.toggleBlackout();
    QVERIFY(om.blackout() == false);
    om.dumpUniverses();

    for (int i = 0; i < 512; i++)
        QVERIFY(stub->m_universe[i] == 'a');
    for (int i = 512; i < 1024; i++)
        QVERIFY(stub->m_universe[i] == 'b');
    for (int i = 1024; i < 1536; i++)
        QVERIFY(stub->m_universe[i] == 'c');
    for (int i = 1536; i < 2048; i++)
        QVERIFY(stub->m_universe[i] == 'd');

    om.setBlackout(false);
    QVERIFY(om.blackout() == false);
    om.dumpUniverses();

    for (int i = 0; i < 512; i++)
        QVERIFY(stub->m_universe[i] == 'a');
    for (int i = 512; i < 1024; i++)
        QVERIFY(stub->m_universe[i] == 'b');
    for (int i = 1024; i < 1536; i++)
        QVERIFY(stub->m_universe[i] == 'c');
    for (int i = 1536; i < 2048; i++)
        QVERIFY(stub->m_universe[i] == 'd');

    om.toggleBlackout();
    QVERIFY(om.blackout() == true);
    om.dumpUniverses();

    for (int i = 0; i < 2048; i++)
        QVERIFY(stub->m_universe[i] == (char) 0);
}

void OutputMap_Test::pluginNames()
{
    OutputMap om(m_doc, 4);
    QVERIFY(om.pluginNames().size() == 1);
    QCOMPARE(om.pluginNames().at(0), QString("Output Plugin Stub"));
}

void OutputMap_Test::pluginOutputs()
{
    OutputMap om(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QStringList ls(om.pluginOutputs(stub->name()));
    QVERIFY(ls == stub->outputs());

    QVERIFY(om.pluginOutputs("Foobar").isEmpty() == true);
}

void OutputMap_Test::universeNames()
{
    OutputMap om(m_doc, 4);

    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QVERIFY(om.universeNames().at(0).contains("None"));
    QVERIFY(om.universeNames().at(1).contains("None"));
    QVERIFY(om.universeNames().at(2).contains("None"));
    QVERIFY(om.universeNames().at(3).contains("None"));

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    om.setPatch(0, stub->name(), 3);
    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QCOMPARE(om.universeNames().at(0), QString("1: Output Plugin Stub (4: Stub 4)"));
    QCOMPARE(om.universeNames().at(1), QString("2: None (None)"));
    QCOMPARE(om.universeNames().at(2), QString("3: None (None)"));
    QCOMPARE(om.universeNames().at(3), QString("4: None (None)"));

    om.setPatch(3, stub->name(), 2);
    QCOMPARE(quint32(om.universeNames().size()), om.universes());
    QCOMPARE(om.universeNames().at(0), QString("1: Output Plugin Stub (4: Stub 4)"));
    QCOMPARE(om.universeNames().at(1), QString("2: None (None)"));
    QCOMPARE(om.universeNames().at(2), QString("3: None (None)"));
    QCOMPARE(om.universeNames().at(3), QString("4: Output Plugin Stub (3: Stub 3)"));
}

void OutputMap_Test::configure()
{
    OutputMap om(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QCOMPARE(om.canConfigurePlugin("Foo"), false);
    QCOMPARE(om.canConfigurePlugin(stub->name()), false);
    stub->m_canConfigure = true;
    QCOMPARE(om.canConfigurePlugin(stub->name()), true);

    om.configurePlugin("Foo");
    QCOMPARE(stub->m_configureCalled, 0);
    om.configurePlugin(stub->name());
    QCOMPARE(stub->m_configureCalled, 1);
}

void OutputMap_Test::slotConfigurationChanged()
{
    OutputMap om(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QSignalSpy spy(&om, SIGNAL(pluginConfigurationChanged(QString)));
    stub->configure();
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(stub->name()));
}

void OutputMap_Test::mapping()
{
    OutputMap om(m_doc, 4);

    for (quint32 i = 0; i < 20; i++)
        QCOMPARE(om.mapping("Dummy Output", i), QLCIOPlugin::invalidLine());

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.setPatch(1, stub->name(), quint32(0)) == true);
    QCOMPARE(om.mapping("Dummy Output", 0), QLCIOPlugin::invalidLine());
    QCOMPARE(om.mapping("Dummy Output", 1), OutputMap::invalidUniverse());
    QCOMPARE(om.mapping("Dummy Output", 2), QLCIOPlugin::invalidLine());
    QCOMPARE(om.mapping("Dummy Output", 3), QLCIOPlugin::invalidLine());
    QCOMPARE(om.mapping(stub->name(), 0), quint32(1));
    QCOMPARE(om.mapping(stub->name(), 1), OutputMap::invalidUniverse());
    QCOMPARE(om.mapping(stub->name(), 2), OutputMap::invalidUniverse());
    QCOMPARE(om.mapping(stub->name(), 3), OutputMap::invalidUniverse());
}

void OutputMap_Test::pluginStatus()
{
    OutputMap om(m_doc, 4);

    QVERIFY(om.pluginStatus("Foo", QLCIOPlugin::invalidLine()).contains("Nothing selected"));
    QVERIFY(om.pluginStatus("Bar", 0).contains("Nothing selected"));
    QVERIFY(om.pluginStatus("Baz", 1).contains("Nothing selected"));
    QVERIFY(om.pluginStatus("Xyzzy", 2).contains("Nothing selected"));
    QVERIFY(om.pluginStatus("AYBABTU", 3).contains("Nothing selected"));

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.pluginStatus(stub->name(), 4) == stub->outputInfo(QLCIOPlugin::invalidLine()));
    QVERIFY(om.pluginStatus(stub->name(), 0) == stub->outputInfo(0));
    QVERIFY(om.pluginStatus(stub->name(), 1) == stub->outputInfo(1));
    QVERIFY(om.pluginStatus(stub->name(), 2) == stub->outputInfo(2));
}

QTEST_APPLESS_MAIN(OutputMap_Test)
