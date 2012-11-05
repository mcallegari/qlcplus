/*
  Q Light Controller
  dmxslider.cpp

  Copyright (C) Heikki Junnila

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
