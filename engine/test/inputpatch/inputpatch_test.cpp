/*
  Q Light Controller - Unit test
  inputpatch_test.cpp

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

#include <QtTest>
#include <QtXml>

#define private public
#include "outputpluginstub.h"
#include "inputpatch_test.h"
#include "qlcioplugin.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "qlcfile.h"
#include "doc.h"
#undef private

#define TESTPLUGINDIR "../outputpluginstub"

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
    QVERIFY(ip.m_input == QLCIOPlugin::invalidLine());
    QVERIFY(ip.m_profile == NULL);
    QVERIFY(ip.pluginName() == KInputNone);
    QVERIFY(ip.inputName() == KInputNone);
    QVERIFY(ip.profileName() == KInputNone);
    QVERIFY(ip.m_feedbackEnabled == true);
    QVERIFY(ip.feedbackEnabled() == true);
}

void InputPatch_Test::patch()
{
    InputMap im(m_doc, 4);

    QCOMPARE(m_doc->ioPluginCache()->plugins().size(), 1);
    OutputPluginStub* stub = static_cast<OutputPluginStub*> (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    QLCInputProfile prof1;
    prof1.setManufacturer("Foo");
    prof1.setManufacturer("Bar");

    InputPatch* ip = new InputPatch(0, this);
    ip->set(stub, 0, false, &prof1);
    QVERIFY(ip->m_plugin == stub);
    QVERIFY(ip->m_input == 0);
    QVERIFY(ip->m_profile == &prof1);
    QVERIFY(ip->pluginName() == stub->name());
    QVERIFY(ip->inputName() == stub->inputs()[0]);
    QVERIFY(ip->profileName() == prof1.name());
    QVERIFY(stub->m_openInputs.size() == 1);
    QVERIFY(stub->m_openInputs.at(0) == 0);
    QVERIFY(ip->m_feedbackEnabled == false);
    QVERIFY(ip->feedbackEnabled() == false);

    QLCInputProfile prof2;
    prof2.setManufacturer("Xyzzy");
    prof2.setManufacturer("Foobar");

    ip->set(stub, 3, true, &prof2);
    QVERIFY(ip->m_plugin == stub);
    QVERIFY(ip->m_input == 3);
    QVERIFY(ip->m_profile == &prof2);
    QVERIFY(ip->pluginName() == stub->name());
    QVERIFY(ip->inputName() == stub->inputs()[3]);
    QVERIFY(ip->profileName() == prof2.name());
    QVERIFY(stub->m_openInputs.size() == 1);
    QVERIFY(stub->m_openInputs.at(0) == 3);
    QVERIFY(ip->m_feedbackEnabled == true);
    QVERIFY(ip->feedbackEnabled() == true);

    ip->reconnect();
    QVERIFY(ip->m_plugin == stub);
    QVERIFY(ip->m_input == 3);
    QVERIFY(ip->m_profile == &prof2);
    QVERIFY(ip->pluginName() == stub->name());
    QVERIFY(ip->inputName() == stub->inputs()[3]);
    QVERIFY(ip->profileName() == prof2.name());
    QVERIFY(stub->m_openInputs.size() == 1);
    QVERIFY(stub->m_openInputs.at(0) == 3);
    QVERIFY(ip->m_feedbackEnabled == true);
    QVERIFY(ip->feedbackEnabled() == true);

    delete ip;
    QVERIFY(stub->m_openInputs.size() == 0);
}

QTEST_APPLESS_MAIN(InputPatch_Test)
