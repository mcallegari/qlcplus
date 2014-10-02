/*
  Q Light Controller Plus
  timingstool.cpp

  Copyright (C) Massimo Callegari

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

#include <QBoxLayout>
#include <QSettings>

#include "timingstool.h"
#include "speeddial.h"
#include "showitem.h"
#include "apputil.h"

#define WINDOW_FLAGS Qt::WindowFlags( \
    (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::Window | \
     Qt::WindowStaysOnTopHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint))

#define SETTINGS_GEOMETRY "timingstool/geometry"

TimingsTool::TimingsTool(ShowItem *item, QWidget *parent)
    : QWidget(parent)
    , m_startDial(NULL)
    , m_durationDial(NULL)
    , m_item(item)
{
    Q_ASSERT(item != NULL);

    setWindowFlags(WINDOW_FLAGS);

    QBoxLayout* lay = new QBoxLayout(QBoxLayout::TopToBottom, this);

    /* Create dials */
    m_startDial = new SpeedDial(this);
    m_startDial->setTitle(tr("Start Time"));
    m_startDial->setInfiniteVisibility(false);
    m_startDial->setTapVisibility(false);
    m_startDial->setValue(m_item->getStartTime());
    layout()->addWidget(m_startDial);
    connect(m_startDial, SIGNAL(valueChanged(int)),
            this, SLOT(slotStartTimeChanged(int)));

    m_durationDial = new SpeedDial(this);
    m_durationDial->setTitle(tr("Duration"));
    m_durationDial->setInfiniteVisibility(false);
    m_durationDial->setTapVisibility(false);
    m_durationDial->setValue(m_item->getDuration());
    layout()->addWidget(m_durationDial);
    connect(m_durationDial, SIGNAL(valueChanged(int)),
            this, SLOT(slotDurationChanged(int)));

    lay->addStretch();

    /* Position */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        this->restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

TimingsTool::~TimingsTool()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void TimingsTool::slotStartTimeChanged(int msec)
{
    emit startTimeChanged(m_item, msec);
}

void TimingsTool::slotDurationChanged(int msec)
{
    emit durationChanged(m_item, msec);
}
