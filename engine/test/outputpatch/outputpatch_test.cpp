/*
  Q Light Controller - Unit test
  outputpatch_test.cpp

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
#include "outputpatch_test.h"
#include "outputpatch.h"
#include "outputmap.h"
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
    QVERIFY(op.m_output == QLCIOPlugin::invalidLine());
    QVERIFY(op.pluginName() == KOutputNone);
    QVERIFY(op.outputName() == KOutputNone);
}

void OutputPatch_Test::patch()
{
    OutputMap om(m_doc, 4);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    OutputPatch* op = new OutputPatch(this);
    op->set(stub, 0);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_output == 0);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[0]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 0);

    op->set(stub, 3);
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_output == 3);
    QVERIFY(op->pluginName() == stub->name());
    QVERIFY(op->outputName() == stub->outputs()[3]);
    QVERIFY(stub->m_openOutputs.size() == 1);
    QVERIFY(stub->m_openOutputs.at(0) == 3);

    op->reconnect();
    QVERIFY(op->m_plugin == stub);
    QVERIFY(op->m_output == 3);
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

    OutputMap om(m_doc, 4);
    OutputPatch* op = new OutputPatch(this);

    OutputPluginStub* stub = static_cast<OutputPluginStub*>
                                (m_doc->ioPluginCache()->plugins().at(0));
    QVERIFY(stub != NULL);

    op->set(stub, 0);
    QVERIFY(stub->m_universe[0] == (char) 0);
    QVERIFY(stub->m_universe[169] == (char) 0);
    QVERIFY(stub->m_universe[511] == (char) 0);

    op->dump(uni);
    QVERIFY(stub->m_universe[0] == (char) 100);
    QVERIFY(stub->m_universe[169] == (char) 50);
    QVERIFY(stub->m_universe[511] == (char) 25);

    delete op;
}

QTEST_APPLESS_MAIN(OutputPatch_Test)
