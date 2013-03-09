/*
  Q Light Controller - Unit test
  inputmap_test.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QSignalSpy>
#include <QtTest>

#define private public
#include "outputpluginstub.h"
#include "inputmap_test.h"
#include "qlcinputsource.h"
#include "inputpatch.h"
#include "qlcconfig.h"
#include "inputmap.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define TESTPLUGINDIR "../outputpluginstub"
#define OUTPUT_TESTPLUGINDIR "../inputpluginstub"
#define ENGINEDIR "../../src"
#define PROFILEDIR "../../../inputprofiles"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

void InputMap_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_doc->ioPluginCache()->load(testPluginDir());
    QVERIFY(m_doc->ioPluginCache()->plugins().size() != 0);
}

void InputMap_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void InputMap_Test::initial()
{
    InputMap im(m_doc, 4);
    QVERIFY(im.universes() == 4);
    QVERIFY(im.m_universes == 4);
    QVERIFY(im.editorUniverse() == 0);
    QVERIFY(im.m_editorUniverse == 0);
    QVERIFY(im.m_patch.size() == 4);
    QVERIFY(im.pluginNames().size() == 1);
    QVERIFY(im.m_profiles.size() == 0);
    QVERIFY(im.profileNames().size() == 0);
}

void InputMap_Test::editorUniverse()
{
    InputMap im(m_doc, 4);

    QVERIFY(im.editorUniverse() == 0);
    im.setEditorUniverse(3);
    QVERIFY(im.editorUniverse() == 3);
    im.setEditorUniverse(4);
    QVERIFY(im.editorUniverse() == 0);
    im.setEditorUniverse(1);
    QVERIFY(im.editorUniverse() == 1);
    im.setEditorUniverse(2);
    QVERIFY(im.editorUniverse() == 2);
}

void InputMap_Test::pluginNames()
{
    InputMap im(m_doc, 4);
    QCOMPARE(im.pluginNames().size(), 1);
    QCOMPARE(im.pluginNames().at(0), QString("Output Plugin Stub"));
}

void InputMap_Test::pluginInputs()
{
    InputMap im(m_doc, 4);

    QVERIFY(im.pluginInputs("Foo").size() == 0);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.pluginInputs(stub->name()).size() == 4);
    QVERIFY(im.pluginInputs(stub->name()) == stub->inputs());
}

void InputMap_Test::configurePlugin()
{
    InputMap im(m_doc, 4);

    QCOMPARE(im.canConfigurePlugin("Foo"), false);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
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

void InputMap_Test::pluginStatus()
{
    InputMap im(m_doc, 4);

    QVERIFY(im.pluginStatus("Foo", QLCIOPlugin::invalidLine()).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("Bar", 0).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("Baz", 1).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("Xyzzy", 2).contains("Nothing selected"));
    QVERIFY(im.pluginStatus("AYBABTU", 3).contains("Nothing selected"));

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QVERIFY(im.pluginStatus(stub->name(), QLCIOPlugin::invalidLine()) == stub->inputInfo(QLCIOPlugin::invalidLine()));
    QVERIFY(im.pluginStatus(stub->name(), 0) == stub->inputInfo(0));
    QVERIFY(im.pluginStatus(stub->name(), 1) == stub->inputInfo(1));
    QVERIFY(im.pluginStatus(stub->name(), 2) == stub->inputInfo(2));
}

void InputMap_Test::profiles()
{
    InputMap im(m_doc, 4);
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

void InputMap_Test::setPatch()
{
    InputMap im(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QLCInputProfile* prof = new QLCInputProfile();
    prof->setManufacturer("Foo");
    prof->setModel("Bar");
    im.addProfile(prof);

    QVERIFY(im.patch(0)->plugin() == NULL);
    QVERIFY(im.patch(0)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 0) == InputMap::invalidUniverse());

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == InputMap::invalidUniverse());

    QVERIFY(im.setPatch(0, "Foobar", 0, prof->name()) == true);
    QVERIFY(im.patch(0)->plugin() == NULL);
    QVERIFY(im.patch(0)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(0)->profile() == prof);
    QVERIFY(im.mapping(stub->name(), 0) == InputMap::invalidUniverse());

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == InputMap::invalidUniverse());

    QVERIFY(im.setPatch(0, stub->name(), 0) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 0) == 0);

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == NULL);
    QVERIFY(im.patch(2)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(2)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == InputMap::invalidUniverse());

    QVERIFY(im.setPatch(2, stub->name(), 3, prof->name()) == true);
    QVERIFY(im.patch(0)->plugin() == stub);
    QVERIFY(im.patch(0)->input() == 0);
    QVERIFY(im.patch(0)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 0) == 0);

    QVERIFY(im.patch(1)->plugin() == NULL);
    QVERIFY(im.patch(1)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(1)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 1) == InputMap::invalidUniverse());

    QVERIFY(im.patch(2)->plugin() == stub);
    QVERIFY(im.patch(2)->input() == 3);
    QVERIFY(im.patch(2)->profile() == prof);
    QVERIFY(im.mapping(stub->name(), 2) == InputMap::invalidUniverse());

    QVERIFY(im.patch(3)->plugin() == NULL);
    QVERIFY(im.patch(3)->input() == QLCIOPlugin::invalidLine());
    QVERIFY(im.patch(3)->profile() == NULL);
    QVERIFY(im.mapping(stub->name(), 3) == 2);

    // Universe out of bounds
    QVERIFY(im.setPatch(im.universes(), stub->name(), 0) == false);
}

void InputMap_Test::slotValueChanged()
{
    InputMap im(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
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

void InputMap_Test::slotConfigurationChanged()
{
    InputMap im(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QSignalSpy spy(&im, SIGNAL(pluginConfigurationChanged(QString)));
    stub->configure();
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QString(stub->name()));
}

void InputMap_Test::loadInputProfiles()
{
    InputMap im(m_doc, 4);

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

void InputMap_Test::inputSourceNames()
{
    InputMap im(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*> (m_doc->ioPluginCache()->plugins().at(0));
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

    QVERIFY(im.inputSourceNames(QLCInputSource(0, InputMap::invalidChannel()), uni, ch) == false);
    QVERIFY(im.inputSourceNames(QLCInputSource(InputMap::invalidUniverse(), 0), uni, ch) == false);
    QVERIFY(im.inputSourceNames(QLCInputSource(), uni, ch) == false);
}

void InputMap_Test::profileDirectories()
{
    QDir dir = InputMap::systemProfileDirectory();
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtInputProfile)));
    QVERIFY(dir.absolutePath().contains(INPUTPROFILEDIR));

    dir = InputMap::userProfileDirectory();
    QVERIFY(dir.exists() == true);
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtInputProfile)));
    QVERIFY(dir.absolutePath().contains(USERINPUTPROFILEDIR));
}

QTEST_APPLESS_MAIN(InputMap_Test)
