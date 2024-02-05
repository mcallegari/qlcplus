/*
  Q Light Controller
  vcdockarea.cpp

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

#include <QVBoxLayout>
#include <QString>
#include <QDebug>

#include "grandmasterslider.h"
#include "inputoutputmap.h"
#include "vcdockarea.h"

VCDockArea::VCDockArea(QWidget* parent, InputOutputMap *ioMap)
    : QFrame(parent)
{
    Q_ASSERT(ioMap != NULL);

    new QHBoxLayout(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(1);

    m_gm = new GrandMasterSlider(this, ioMap);
    layout()->addWidget(m_gm);
}

VCDockArea::~VCDockArea()
{
}

void VCDockArea::setGrandMasterInvertedAppearance(GrandMaster::SliderMode mode)
{
    Q_ASSERT(m_gm != NULL);
    if (mode == GrandMaster::Normal)
        m_gm->setInvertedAppearance(false);
    else
        m_gm->setInvertedAppearance(true);
}

