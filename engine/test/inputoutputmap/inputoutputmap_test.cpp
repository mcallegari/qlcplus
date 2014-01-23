/*
  Q Light Controller - Unit test
  inputoutputmap_test.cpp

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
#include "iopluginstub.h"
#include "inputoutputmap_test.h"
#include "qlcinputsource.h"
#include "qlcioplugin.h"
#include "inputpatch.h"
#include "outputpatch.h"
#include "qlcconfig.h"
#include "inputoutputmap.h"
#include "qlcfile.h"
#include "doc.h"
#include "universe.h"
#undef private

#define TESTPLUGINDIR "../iopluginstub"
#define ENGINEDIR "../../src"
#define PROFILEDIR "../../../inputprofiles"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

void InputOutputMap_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_doc->ioPluginCache()->load(testPluginDir());
    QVERIFY(m_doc->ioPluginCache()->plugins().size() != 0);
}

void InputOutputMap_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void InputOutputMap_Test::initial()
{
    InputOutputMap iom(m_doc, 4);
    QCOMPARE(iom.universes(), 4);
    QVERIFY(iom.m_patch.size() == 4);
    QVERIFY(iom.inputPluginNames().size() == 1);
    QVERIFY(iom.outputPluginNames().size() == 1);
    QVERIFY(iom.m_profiles.size() == 0);
    QVERIFY(iom.profileNames().size() == 0);
}

void InputOutputMap_Test::pluginNames()
{
    InputOutputMap im(m_doc, 4);
    QCOMPARE(im.pluginNames().size(), 1);
    QCOMPARE(im.pluginNames().at(0), QString("Output Plugin Stub"));
}

void InputOutputMap_Test::pluginInputs()
{
    InputOutputMap im(m_doc, 4);

    QVERIFY(im.pluginInputs("Foo").size() == 0);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.pluginInputs(stub->name()).size() == 4);
    QVERIFY(im.pluginInputs(stub->name()) == stub->inputs());
}

void InputOutputMap_Test::configurePlugin()
{
    InputOutputMap im(m_doc, 4);

    QCOMPARE(im.canConfigurePlugin("Foo"), false);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QCOMPARE(im.canConfigurePlugin("Foo"), false);
    QCOMPARE(im.canConfigurePlugin(stub->name()), false);
    stub->m_canConfigure = true;
    QCOMPARE(im.canConfigurePlugin(stub->name()), true);

    /* Must be able to call multiple times */
    im.configurePlugin(stub->name());
    QVERIFY(stub->m_configureCalled == 1);
    im.configurePlugin(stub->name());
    QVERIFY(stub->m_configureCalled == 2);
    im.configurePlugin(stub->name());
    QVERIFY(stub->m_configureCalled == 3);
}

void InputOutputMap_Test::pluginStatus()
{
    InputOutputMap im(m_doc, 4);

    QVERIFY(im.pluginStatus("Foo", QLCIOPlugin::invalidLine()).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("Bar", 0).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("Baz", 1).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("Xyzzy", 2).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("AYBABTU", 3).contains("Nothing selected"));

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.pluginStatus(stub->name(), QLCIOPlugin::invalidLine()) == stub->inputInfo(QLCIOPlugin::invalidLine()));
    QVERIFY(im.pluginStatus(stub->name(), 0) == stub->inputInfo(0));
    QVERIFY(im.pluginStatus(stub->name(), 1) == stub->inputInfo(1));
    QVERIFY(im.pluginStatus(stub->name(), 2) == stub->inputInfo(2));
}

void InputOutputMap_Test::profiles()
{
    InputOutputMap im(m_doc, 4);
    QVERIFY(im.m_profiles.size() == 0);

    QLCInputProfile* prof = new QLCInputProfile();
    prof->setManufacturer("Foo");
    prof->setModel("Bar");

    QVERIFY(im.addProfile(prof) == true);
    QVERIFY(im.m_profiles.size() == 1);
    QVERIFY(im.addProfile(prof) == false);
    QVERIFY(im.m_profiles.size() == 1);

    QVERIFY(im.profileNames().size() == 1);
    QVERIFY(im.profileNames().at(0) == prof->name());
    QVERIFY(im.profile(prof->name()) == prof);
    QVERIFY(im.profile("Foobar") == NULL);

    QVERIFY(im.removeProfile("Foobar") == false);
    QVERIFY(im.m_profiles.size() == 1);
    QVERIFY(im.removeProfile(prof->name()) == true);
    QVERIFY(im.m_profiles.size() == 0);
}

void InputOutputMap_Test::setPatch()
{
    InputOutputMap im(m_doc, 4);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QLCInputProfile* prof = new QLCInputProfile();
    prof->setManufacturer("Foo");
    prof->setModel("Bar");
    im.addProfile(prof);

    QVERIFY(im.patch(0)->plugin() == NULL);
    QVERIFY(im.patch(0)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 0) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 2) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == InputOutputMap::invalidUniverse());

    QVERIFY(im.setPatch(0, "Foobar", 0, prof->name()) == true);
    QVERIFY(im.patch(0)->plugin() == NULL);
    QVERIFY(im.patch(0)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(0)->profile() == prof);
    QVERIFY(im.mapping(stub->name(), 0) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 2) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == InputOutputMap::invalidUniverse());

    QVERIFY(im.setPatch(0, stub->name(), 0) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 0) == 0);

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 2) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == InputOutputMap::invalidUniverse());

    QVERIFY(im.setPatch(2, stub->name(), 3, prof->name()) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 0) == 0);

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == stub);
    QVERIFY(im.patch(2)->input() == 3);
    QVERIFY(im.patch(2)->profile() == prof);
    QVERIFY(im.mapping(stub->name(), 2) == InputOutputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == 2);

    // Universe out of bounds
    QVERIFY(im.setPatch(im.universes(), stub->name(), 0) == false);
}

void InputOutputMap_Test::slotValueChanged()
{
    InputOutputMap im(m_doc, 4);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.setPatch(0, stub->name(), 0) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);

    QSignalSpy spy(&im, SIGNAL(inputValueChanged(quint32, quint32, uchar, const QString&)));
    stub->emitValueChanged(0, 15, UCHAR_MAX);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);

    /* Invalid mapping for this plugin -> no signal */
    stub->emitValueChanged(3, 15, UCHAR_MAX);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);

    /* Invalid mapping for this plugin -> no signal */
    stub->emitValueChanged(1, 15, UCHAR_MAX);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);

    stub->emitValueChanged(0, 5, 127);
    QVERIFY(spy.size() == 2);
    QVERIFY(spy.at(0).at(0) == 0);
    QVERIFY(spy.at(0).at(1) == 15);
    QVERIFY(spy.at(0).at(2) == UCHAR_MAX);
    QVERIFY(spy.at(1).at(0) == 0);
    QVERIFY(spy.at(1).at(1) == 5);
    QVERIFY(spy.at(1).at(2) == 127);
}

void InputOutputMap_Test::slotConfigurationChanged()
{
    InputOutputMap im(m_doc, 4);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QSignalSpy spy(&im, SIGNAL(pluginConfigurationChanged(QString)));
    stub->configure();
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(stub->name()));
}

void InputOutputMap_Test::loadInputProfiles()
{
    InputOutputMap im(m_doc, 4);

    // No profiles in a nonexistent directory
    QDir dir("/path/to/a/nonexistent/place/beyond/this/universe");
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    im.loadProfiles(dir);
    QVERIFY(im.profileNames().isEmpty() == true);

    // No profiles in an existing directory
    dir = testPluginDir();
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    im.loadProfiles(dir);
    QVERIFY(im.profileNames().isEmpty() == true);

    // Should be able to load profiles
    dir.setPath(PROFILEDIR);
    im.loadProfiles(dir);
    QStringList names(im.profileNames());
    QVERIFY(names.size() > 0);

    // Shouldn't load duplicates
    im.loadProfiles(dir);
    QCOMPARE(names, im.profileNames());
}

void InputOutputMap_Test::inputSourceNames()
{
    InputOutputMap im(m_doc, 4);

    IOPluginStub* stub = static_cast<IOPluginStub*> (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QDir dir(PROFILEDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    im.loadProfiles(dir);

    QString uni, ch;
    QVERIFY(im.inputSourceNames(QLCInputSource(0, 0), uni, ch) == true);
    QCOMPARE(uni, tr("%1: ??").arg(1));
    QCOMPARE(ch, tr("%1: ?").arg(1));

    QVERIFY(im.setPatch(0, stub->name(), 0, QString("Generic MIDI")) == true);
    QVERIFY(im.inputSourceNames(QLCInputSource(0, 0), uni, ch) == true);
    QCOMPARE(uni, tr("%1: Generic MIDI").arg(1));
    QCOMPARE(ch, tr("%1: Bank select MSB").arg(1));

    uni.clear();
    ch.clear();
    QVERIFY(im.inputSourceNames(QLCInputSource(0, 50000), uni, ch) == true);
    QCOMPARE(uni, tr("%1: Generic MIDI").arg(1));
    QCOMPARE(ch, tr("%1: ?").arg(50001));

    QVERIFY(im.setPatch(0, stub->name(), 0, QString()) == true);

    uni.clear();
    ch.clear();
    QVERIFY(im.inputSourceNames(QLCInputSource(0, 0), uni, ch) == true);
    QCOMPARE(uni, tr("%1: %2").arg(1).arg(stub->name()));
    QCOMPARE(ch, tr("%1: ?").arg(1));

    QVERIFY(im.inputSourceNames(QLCInputSource(0, InputOutputMap::invalidChannel()), uni, ch) == false);
    QVERIFY(im.inputSourceNames(QLCInputSource(InputOutputMap::invalidUniverse(), 0), uni, ch) == false);
    QVERIFY(im.inputSourceNames(QLCInputSource(), uni, ch) == false);
}

void InputOutputMap_Test::profileDirectories()
{
    QDir dir = InputOutputMap::systemProfileDirectory();
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtInputProfile)));
    QVERIFY(dir.absolutePath().contains(INPUTPROFILEDIR));

    dir = InputOutputMap::userProfileDirectory();
    QVERIFY(dir.exists() == true);
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtInputProfile)));
    QVERIFY(dir.absolutePath().contains(USERINPUTPROFILEDIR));
}

#ifdef OUTPUT_TESTS
void OutputMap_Test::initial()
{
    OutputMap om(m_doc, 4);
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
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

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(om.pluginStatus(stub->name(), 4) == stub->outputInfo(QLCIOPlugin::invalidLine()));
    QVERIFY(om.pluginStatus(stub->name(), 0) == stub->outputInfo(0));
    QVERIFY(om.pluginStatus(stub->name(), 1) == stub->outputInfo(1));
    QVERIFY(om.pluginStatus(stub->name(), 2) == stub->outputInfo(2));
}

#endif

QTEST_APPLESS_MAIN(InputOutputMap_Test)

