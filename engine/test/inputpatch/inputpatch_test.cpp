/*
  Q Light Controller - Unit test
  inputpatch_test.cpp

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

#include <QtTest>

#define private public
#include "iopluginstub.h"
#include "inputpatch_test.h"
#include "qlcioplugin.h"
#include "inputpatch.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define TESTPLUGINDIR "../iopluginstub"

static QDir testPluginDir()
{
    QDir dir(TESTPLUGINDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));
    return dir;
}

void InputPatch_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_doc->ioPluginCache()->load(testPluginDir());
    QVERIFY(m_doc->ioPluginCache()->plugins().size() != 0);
}

void InputPatch_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void InputPatch_Test::defaults()
{
    InputPatch ip(0, this);
    QVERIFY(ip.m_plugin == NULL);
    QVERIFY(ip.m_pluginLine == QLCIOPlugin::invalidLine());
    QVERIFY(ip.m_profile == NULL);
    QVERIFY(ip.pluginName() == KInputNone);
    QVERIFY(ip.inputName() == KInputNone);
    QVERIFY(ip.profileName() == KInputNone);
}

void InputPatch_Test::patch()
{
    InputOutputMap im(m_doc, 4);

    QCOMPARE(m_doc->ioPluginCache()->plugins().size(), 1);
    IOPluginStub* stub = static_cast<IOPluginStub*> (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QLCInputProfile prof1;
    prof1.setManufacturer("Foo");
    prof1.setManufacturer("Bar");

    InputPatch* ip = new InputPatch(0, this);
    ip->set(stub, 0, &prof1);
    QVERIFY(ip->m_plugin == stub);
    QVERIFY(ip->m_pluginLine == 0);
    QVERIFY(ip->m_profile == &prof1);
    QVERIFY(ip->pluginName() == stub->name());
    QVERIFY(ip->inputName() == stub->inputs()[0]);
    QVERIFY(ip->profileName() == prof1.name());
    QVERIFY(stub->m_openInputs.size() == 1);
    QVERIFY(stub->m_openInputs.at(0) == 0);

    QLCInputProfile prof2;
    prof2.setManufacturer("Xyzzy");
    prof2.setManufacturer("Foobar");

    ip->set(stub, 3, &prof2);
    QVERIFY(ip->m_plugin == stub);
    QVERIFY(ip->m_pluginLine == 3);
    QVERIFY(ip->m_profile == &prof2);
    QVERIFY(ip->pluginName() == stub->name());
    QVERIFY(ip->inputName() == stub->inputs()[3]);
    QVERIFY(ip->profileName() == prof2.name());
    QVERIFY(stub->m_openInputs.size() == 1);
    QVERIFY(stub->m_openInputs.at(0) == 3);

    ip->reconnect();
    QVERIFY(ip->m_plugin == stub);
    QVERIFY(ip->m_pluginLine == 3);
    QVERIFY(ip->m_profile == &prof2);
    QVERIFY(ip->pluginName() == stub->name());
    QVERIFY(ip->inputName() == stub->inputs()[3]);
    QVERIFY(ip->profileName() == prof2.name());
    QVERIFY(stub->m_openInputs.size() == 1);
    QVERIFY(stub->m_openInputs.at(0) == 3);

    delete ip;
    QVERIFY(stub->m_openInputs.size() == 0);
}

QTEST_APPLESS_MAIN(InputPatch_Test)
