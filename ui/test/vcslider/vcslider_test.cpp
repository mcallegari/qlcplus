/*
  Q Light Controller Plus - Unit test
  vcslider_test.cpp

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

#define private public
#define protected public
#include "virtualconsole.h"
#include "vcslider.h"
#include "vcframe.h"
#include "doc.h"
#undef private
#undef protected

#include "vcslider_test.h"

void VCSlider_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void VCSlider_Test::init()
{
    new VirtualConsole(NULL, m_doc);
}

void VCSlider_Test::cleanup()
{
    delete VirtualConsole::instance();
    m_doc->clearContents();
}

void VCSlider_Test::initial()
{
    VCSlider slider(VirtualConsole::instance()->contents(), m_doc);
    QCOMPARE(slider.sliderMode(), VCSlider::Playback);
    QVERIFY(slider.m_resetButton == nullptr);
}

void VCSlider_Test::resetButtonNull()
{
    VCSlider slider(VirtualConsole::instance()->contents(), m_doc);
    // Monitor not enabled, so m_resetButton is null
    QVERIFY(slider.m_resetButton == nullptr);
    // slotResetButtonClicked should not crash (H2 regression)
    slider.slotResetButtonClicked();
}

void VCSlider_Test::resetButtonSetup()
{
    VCSlider slider(VirtualConsole::instance()->contents(), m_doc);
    slider.setSliderMode(VCSlider::Level);
    // Enable monitor to create the reset button
    slider.setChannelsMonitorEnabled(true);
    QVERIFY(slider.m_resetButton != nullptr);
    // H3 regression: verify the reset button has correct size (not m_cngButton)
    QCOMPARE(slider.m_resetButton->width(), 32);
    QCOMPARE(slider.m_resetButton->height(), 32);
    slider.setChannelsMonitorEnabled(false);
}

void VCSlider_Test::sliderShadowCast()
{
    VCSlider slider(VirtualConsole::instance()->contents(), m_doc);
    slider.setWidgetStyle(VCSlider::WSlider);
    // H4 regression: setSliderShadowValue should not crash if cast fails
    slider.setSliderShadowValue(128);
}

void VCSlider_Test::monitorOverride()
{
    VCSlider slider(VirtualConsole::instance()->contents(), m_doc);
    slider.setSliderMode(VCSlider::Level);
    // Monitor NOT enabled, m_resetButton is null
    // Setting slider value in Level mode should not crash (H2 regression)
    slider.setSliderValue(128);
}

QTEST_MAIN(VCSlider_Test)
