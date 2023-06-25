/*
  Q Light Controller - Unit test
  outputpatch_test.cpp

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
#include "outputpatch_test.h"
#include "outputpatch.h"
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

void OutputPatch_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_doc->ioPluginCache()->load(testPluginDir());
    QVERIFY(m_doc->ioPluginCache()->plugins().size() != 0);
}

void OutputPatch_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

void OutputPatch_Test::defaults()
{
    OutputPatch op(this);
    QVERIFY(op.m_plugin == NULL);
    QVERIFY(op.m_pluginLine == QLCIOPlugin::invalidLine());
    QVERIFY(op.pluginName() == KOutputNone);
    QVERIFY(op.outputName() == KOutputNone);
}

void OutputPatch_Test::patch()
{
    InputOutputMap om(m_doc, 4);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    OutputPatch* op = new OutputPatch(0, this);
    op->set(stub, 0);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_pluginLine == 0);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[0]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 0);

    op->set(stub, 3);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_pluginLine == 3);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[3]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 3);

    op->reconnect();
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_pluginLine == 3);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[3]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 3);

    delete op;
    QVERIFY(stub->m_openOutputs.size() == 0);
}

void OutputPatch_Test::dump()
{
    QByteArray uni(513, char(0));
    uni[0] = 100;
    uni[169] = 50;
    uni[511] = 25;

    OutputPatch* op = new OutputPatch(0, this);

    IOPluginStub* stub = static_cast<IOPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    op->set(stub, 0);
    QVERIFY(stub->m_universe[0] == (char) 0);
    QVERIFY(stub->m_universe[169] == (char) 0);
    QVERIFY(stub->m_universe[511] == (char) 0);

    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 100);
    QVERIFY(stub->m_universe[169] == (char) 50);
    QVERIFY(stub->m_universe[511] == (char) 25);

    /* Test the pause state */
    op->setPaused(true);
    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 100);
    QVERIFY(stub->m_universe[169] == (char) 50);
    QVERIFY(stub->m_universe[511] == (char) 25);

    uni[0] = 1;
    uni[169] = 2;
    uni[511] = 3;

    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 100);
    QVERIFY(stub->m_universe[169] == (char) 50);
    QVERIFY(stub->m_universe[511] == (char) 25);

    op->setPaused(false);
    op->dump(0, uni, true);
    QVERIFY(stub->m_universe[0] == (char) 1);
    QVERIFY(stub->m_universe[169] == (char) 2);
    QVERIFY(stub->m_universe[511] == (char) 3);

    delete op;
}

QTEST_APPLESS_MAIN(OutputPatch_Test)
