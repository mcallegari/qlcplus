/*
  Q Light Controller
  vcframeproperties_test.cpp

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

#include <QFrame>
#include <QtTest>

#define protected public
#include "vcframeproperties_test.h"
#include "qlcfixturedefcache.h"
#include "vcframeproperties.h"
#include "virtualconsole.h"
#include "mastertimer.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "doc.h"
#undef protected

void VCFrameProperties_Test::initTestCase()
{
    m_doc = NULL;
}

void VCFrameProperties_Test::init()
{
    m_doc = new Doc(this);
    new VirtualConsole(NULL, m_doc);
}

void VCFrameProperties_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_doc;
}

void VCFrameProperties_Test::initial()
{
    VCFrame* frame = new VCFrame(VirtualConsole::instance()->contents(), m_doc);
    frame->setAllowChildren(false);
    frame->setAllowResize(true);

    QWidget w;
    VCFrameProperties prop(&w, frame, m_doc);
    QCOMPARE(prop.m_allowChildrenCheck->isChecked(), false);
    QCOMPARE(prop.m_allowResizeCheck->isChecked(), true);
    prop.m_allowChildrenCheck->setChecked(true);
    prop.m_allowResizeCheck->setChecked(false);
    prop.accept();
    QCOMPARE(prop.allowChildren(), true);
    QCOMPARE(prop.allowResize(), false);
}

QTEST_MAIN(VCFrameProperties_Test)
