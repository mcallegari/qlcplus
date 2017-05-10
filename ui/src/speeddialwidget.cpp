/*
  Q Light Controller
  speeddialwidget.cpp

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

#include <QSettings>
#include <QGroupBox>
#include <QLineEdit>
#include <QLayout>
#include <QDebug>

#include "speeddialwidget.h"
#include "mastertimer.h"
#include "speeddial.h"
#include "apputil.h"

SpeedDialWidget::SpeedDialWidget(QWidget* parent)
    : MultiSpeedDialWidget(1, parent)
{
    // disconnect(m_fadeIn[0], SIGNAL(valueChanged(int)), this, SIGNAL(fadeInChanged(quint32, int)));
    // disconnect(m_fadeIn[0], SIGNAL(tapped()), this, SIGNAL(fadeInTapped(quint32)));
    // disconnect(m_fadeOut[0], SIGNAL(valueChanged(int)), this, SIGNAL(fadeOutChanged(quint32, int)));
    // disconnect(m_fadeOut[0], SIGNAL(tapped()), this, SIGNAL(fadeOutTapped(quint32)));
    // disconnect(m_hold[0], SIGNAL(valueChanged(int)), this, SIGNAL(holdChanged(quint32, int)));
    // disconnect(m_hold[0], SIGNAL(tapped()), this, SIGNAL(holdTapped(quint32)));

    connect(m_fadeIn[0], SIGNAL(valueChanged(int)), this, SIGNAL(fadeInChanged(int)));
    connect(m_fadeIn[0], SIGNAL(tapped()), this, SIGNAL(fadeInTapped()));
    connect(m_fadeOut[0], SIGNAL(valueChanged(int)), this, SIGNAL(fadeOutChanged(int)));
    connect(m_fadeOut[0], SIGNAL(tapped()), this, SIGNAL(fadeOutTapped()));
    connect(m_hold[0], SIGNAL(valueChanged(int)), this, SIGNAL(holdChanged(int)));
    connect(m_hold[0], SIGNAL(tapped()), this, SIGNAL(holdTapped()));
}

SpeedDialWidget::~SpeedDialWidget()
{
}

/****************************************************************************
 * Speed settings
 ****************************************************************************/

void SpeedDialWidget::setFadeInTitle(const QString& title)
{
    return MultiSpeedDialWidget::setFadeInTitle(0, title);
}

void SpeedDialWidget::setFadeInEnabled(bool enable)
{
    return MultiSpeedDialWidget::setFadeInEnabled(0, enable);
}

void SpeedDialWidget::setFadeInVisible(bool set)
{
    return MultiSpeedDialWidget::setFadeInVisible(0, set);
}

void SpeedDialWidget::setFadeIn(int ms)
{
    return MultiSpeedDialWidget::setFadeIn(0, ms);
}

int SpeedDialWidget::fadeIn() const
{
    return MultiSpeedDialWidget::fadeIn(0);
}

void SpeedDialWidget::setFadeOutTitle(const QString& title)
{
    return MultiSpeedDialWidget::setFadeOutTitle(0, title);
}

void SpeedDialWidget::setFadeOutEnabled(bool enable)
{
    return MultiSpeedDialWidget::setFadeOutEnabled(0, enable);
}

void SpeedDialWidget::setFadeOutVisible(bool set)
{
    return MultiSpeedDialWidget::setFadeOutVisible(0, set);
}

void SpeedDialWidget::setFadeOut(int ms)
{
    return MultiSpeedDialWidget::setFadeOut(0, ms);
}

int SpeedDialWidget::fadeOut() const
{
    return MultiSpeedDialWidget::fadeOut(0);
}

void SpeedDialWidget::setHoldTitle(const QString& title)
{
    return MultiSpeedDialWidget::setHoldTitle(0, title);
}

void SpeedDialWidget::setHoldEnabled(bool enable)
{
    return MultiSpeedDialWidget::setHoldEnabled(0, enable);
}

void SpeedDialWidget::setHoldVisible(bool set)
{
    return MultiSpeedDialWidget::setHoldVisible(0, set);
}

void SpeedDialWidget::setHold(int ms)
{
    return MultiSpeedDialWidget::setHold(0, ms);
}

int SpeedDialWidget::hold() const
{
    return MultiSpeedDialWidget::hold(0);
}
