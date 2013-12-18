/*
  Q Light Controller
  dmxslider.cpp

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
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QSlider>
#include <QLabel>
#include <QDebug>

#include "dmxslider.h"
#include "apputil.h"

DMXSlider::DMXSlider(QWidget* parent)
    : QWidget(parent)
    , m_edit(NULL)
    , m_slider(NULL)
    , m_label(NULL)
{
    new QVBoxLayout(this);
    layout()->setSpacing(1);
    layout()->setContentsMargins(1, 1, 1, 1);

    /* Value editor */
    m_edit = new QLineEdit(this);
    m_edit->setMaxLength(3);
    m_edit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_edit->setAlignment(Qt::AlignHCenter);
    QIntValidator* validator = new QIntValidator(m_edit);
    validator->setRange(0, UCHAR_MAX);
    m_edit->setValidator(validator);
    layout()->addWidget(m_edit);
    layout()->setAlignment(m_edit, Qt::AlignHCenter);
    connect(m_edit, SIGNAL(textEdited(QString)), this, SLOT(slotValueEdited(QString)));

    /* Value slider */
    m_slider = new QSlider(this);
    m_slider->setRange(0, UCHAR_MAX);
    m_slider->setTickInterval(16);
    m_slider->setTickPosition(QSlider::TicksBothSides);
    m_slider->setStyle(AppUtil::saneStyle());
    layout()->addWidget(m_slider);
    layout()->setAlignment(m_slider, Qt::AlignRight);
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderChanged(int)));

    /* Label */
    m_label = new QLabel(this);
    m_label->setWordWrap(true);
    layout()->addWidget(m_label);
    layout()->setAlignment(m_label, Qt::AlignHCenter);

    slotSliderChanged(0);
}

DMXSlider::~DMXSlider()
{
}

void DMXSlider::setValue(uchar value)
{
    m_slider->setValue(value);
}

uchar DMXSlider::value() const
{
    return m_slider->value();
}

void DMXSlider::setLabel(const QString& text)
{
    m_label->setText(text);
}

QString DMXSlider::label() const
{
    return m_label->text();
}

void DMXSlider::setVerticalLabel(const QString& text)
{
    m_verticalText = text;
    update();
}

QString DMXSlider::verticalLabel() const
{
    return m_verticalText;
}

void DMXSlider::slotValueEdited(const QString& text)
{
    setValue(text.toInt());
}

void DMXSlider::slotSliderChanged(int value)
{
    m_edit->setText(QString::number(value));
    emit valueChanged(value);
}

void DMXSlider::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    if (verticalLabel().isEmpty() == false)
    {
        QPainter p(this);
        p.rotate(270);
        p.drawText(-(height() - 20), 20, verticalLabel());
    }
}
