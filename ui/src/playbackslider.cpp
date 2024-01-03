/*
  Q Light Controller
  playbackslider.cpp

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

#include <QIntValidator>
#include <QApplication>
#include <QToolButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QLabel>
#include <QDebug>
#include <QSize>
#include <QIcon>

#include "playbackslider.h"
#include "clickandgoslider.h"

PlaybackSlider::PlaybackSlider(QWidget* parent)
    : QWidget(parent)
    , m_select(NULL)
    , m_value(NULL)
    , m_slider(NULL)
    , m_label(NULL)
    , m_flash(NULL)
    , m_previousValue(-1)
{
    new QVBoxLayout(this);
    layout()->setSpacing(1);
    layout()->setContentsMargins(1, 1, 1, 1);

    /* Select button */
    m_select = new QToolButton(this);
    m_select->setIcon(QIcon(":/check.png"));
    m_select->setIconSize(QSize(32, 32));
    m_select->setToolTip(tr("Select"));
    layout()->addWidget(m_select);
    layout()->setAlignment(m_select, Qt::AlignHCenter);
    connect(m_select, SIGNAL(clicked()), this, SIGNAL(selected()));

    /* Value label */
    m_value = new QLabel(this);
    m_value->setAlignment(Qt::AlignHCenter);
    layout()->addWidget(m_value);
    layout()->setAlignment(m_value, Qt::AlignHCenter);

    /* Value slider */
    m_slider = new ClickAndGoSlider(this);
    m_slider->setRange(0, UCHAR_MAX);
    //m_slider->setTickInterval(16);
    //m_slider->setTickPosition(QSlider::TicksBothSides);
    m_slider->setFixedWidth(32);
    m_slider->setSliderStyleSheet(CNG_DEFAULT_STYLE);
    layout()->addWidget(m_slider);
    layout()->setAlignment(m_slider, Qt::AlignHCenter);
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderChanged(int)));

    /* Label */
    m_label = new QLabel(this);
    m_label->setWordWrap(true);
    layout()->addWidget(m_label);
    layout()->setAlignment(m_label, Qt::AlignHCenter);

    /* Flash button */
    m_flash = new QToolButton(this);
    m_flash->setIcon(QIcon(":/flash.png"));
    m_flash->setIconSize(QSize(32, 32));
    m_flash->setToolTip(tr("Flash"));
    layout()->addWidget(m_flash);
    layout()->setAlignment(m_flash, Qt::AlignHCenter);
    connect(m_flash, SIGNAL(pressed()), this, SLOT(slotFlashPressed()));
    connect(m_flash, SIGNAL(released()), this, SLOT(slotFlashReleased()));

    slotSliderChanged(0);
}

PlaybackSlider::~PlaybackSlider()
{
}

void PlaybackSlider::setValue(uchar value)
{
    m_slider->setValue(value);
}

uchar PlaybackSlider::value() const
{
    return m_slider->value();
}

void PlaybackSlider::setLabel(const QString& text)
{
    m_label->setText(text);
}

QString PlaybackSlider::label() const
{
    return m_label->text();
}

void PlaybackSlider::setSelected(bool sel)
{
    if (sel == true)
    {
        QPalette pal(QApplication::palette());
        pal.setColor(QPalette::Window, pal.color(QPalette::Highlight));
        setPalette(pal);
        setAutoFillBackground(true);
        m_slider->setFocus(Qt::MouseFocusReason);
    }
    else
    {
        setPalette(QApplication::palette());
        setAutoFillBackground(false);
    }
}

void PlaybackSlider::slotSliderChanged(int value)
{
    if (value == m_previousValue)
        return;

    m_value->setText(QString::number(value));

    if (value == 0)
        emit stopped();
    else if (value > 0 && m_previousValue == 0)
        emit started();

    m_previousValue = value;
    emit valueChanged(value);
}

void PlaybackSlider::slotFlashPressed()
{
    emit flashing(true);
}

void PlaybackSlider::slotFlashReleased()
{
    emit flashing(false);
}
