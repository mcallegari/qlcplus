/*
  Q Light Controller Plus
  vcsoloframeproperties.cpp

  Copyright (c) David Garyga

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

#include <QDebug>

#include "vcsoloframeproperties.h"
#include "vcsoloframe.h"

VCSoloFrameProperties::VCSoloFrameProperties(QWidget* parent, VCSoloFrame *frame, Doc *doc)
    : VCFrameProperties(parent, frame, doc)
    , m_soloframe(frame)
{
    // Setup UI
    setupSoloframeUi();

    // Setup state
    m_soloframeMixingCb->setChecked(m_soloframe->soloframeMixing());
}

void VCSoloFrameProperties::setupSoloframeUi()
{
    QGroupBox* groupBox = new QGroupBox(tab);
    new QVBoxLayout(groupBox);
    groupBox->setTitle(tr("Solo Frame properties"));

    m_soloframeMixingCb = new QCheckBox(groupBox);
    m_soloframeMixingCb->setText(tr("Mix sliders in playback mode"));
    groupBox->layout()->addWidget(m_soloframeMixingCb);

    // insert groupBox before the vertical spacer
    tabLayout->insertWidget(tabLayout->count() - 1, groupBox);
}

VCSoloFrameProperties::~VCSoloFrameProperties()
{
}

void VCSoloFrameProperties::accept()
{
    m_soloframe->setSoloframeMixing(m_soloframeMixingCb->isChecked());

    VCFrameProperties::accept();
}
