/*
  Q Light Controller
  speeddial.cpp

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

#include <QFocusEvent>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QLayout>
#include <QTimer>
#include <QDebug>
#include <QDial>
#include <QTime>

#include "mastertimer.h"
#include "speeddial.h"
#include "qlcmacros.h"
#include "function.h"

#define THRESHOLD 10
#define HRS_MAX   (596 - 1) // INT_MAX can hold 596h 31m 23s 647ms
#define MIN_MAX   59
#define SEC_MAX   59
#define MS_MAX    999
#define MS_DIV    10

#define TIMER_HOLD   250
#define TIMER_REPEAT 10

/****************************************************************************
 * FocusSpinBox
 ****************************************************************************/

FocusSpinBox::FocusSpinBox(QWidget* parent)
    : QSpinBox(parent)
{
}

void FocusSpinBox::focusInEvent(QFocusEvent* event)
{
    if (event->gotFocus() == true)
        emit focusGained();
}

/****************************************************************************
 * SpeedDial
 ****************************************************************************/

SpeedDial::SpeedDial(QWidget* parent)
    : QGroupBox(parent)
    , m_timer(new QTimer(this))
    , m_dial(NULL)
    , m_hrs(NULL)
    , m_min(NULL)
    , m_sec(NULL)
    , m_ms(NULL)
    , m_focus(NULL)
    , m_previousDialValue(0)
    , m_preventSignals(false)
    , m_value(0)
    , m_tapTime(new QTime(QTime::currentTime()))
{
    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(0);

    m_minus = new QToolButton(this);
    m_minus->setIconSize(QSize(32, 32));
    m_minus->setIcon(QIcon(":/edit_remove.png"));
    grid->addWidget(m_minus, 0, 0, Qt::AlignVCenter | Qt::AlignLeft);
    connect(m_minus, SIGNAL(pressed()), this, SLOT(slotPlusMinus()));
    connect(m_minus, SIGNAL(released()), this, SLOT(slotPlusMinus()));

    m_dial = new QDial(this);
    m_dial->setWrapping(true);
    m_dial->setNotchesVisible(true);
    m_dial->setTracking(true);
    grid->addWidget(m_dial, 0, 1, 1, 2, Qt::AlignHCenter);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialChanged(int)));

    m_plus = new QToolButton(this);
    m_plus->setIconSize(QSize(32, 32));
    m_plus->setIcon(QIcon(":/edit_add.png"));
    grid->addWidget(m_plus, 0, 3, Qt::AlignVCenter | Qt::AlignRight);
    connect(m_plus, SIGNAL(pressed()), this, SLOT(slotPlusMinus()));
    connect(m_plus, SIGNAL(released()), this, SLOT(slotPlusMinus()));

    m_hrs = new FocusSpinBox(this);
    m_hrs->setRange(0, HRS_MAX);
    m_hrs->setSuffix("h");
    m_hrs->setButtonSymbols(QSpinBox::NoButtons);
    m_hrs->setToolTip(tr("Hours"));
    grid->addWidget(m_hrs, 1, 0);
    connect(m_hrs, SIGNAL(valueChanged(int)), this, SLOT(slotHoursChanged()));
    connect(m_hrs, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_min = new FocusSpinBox(this);
    m_min->setRange(0, MIN_MAX);
    m_min->setSuffix("m");
    m_min->setButtonSymbols(QSpinBox::NoButtons);
    m_min->setToolTip(tr("Minutes"));
    grid->addWidget(m_min, 1, 1);
    connect(m_min, SIGNAL(valueChanged(int)), this, SLOT(slotMinutesChanged()));
    connect(m_min, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_sec = new FocusSpinBox(this);
    m_sec->setRange(0, SEC_MAX);
    m_sec->setSuffix("s");
    m_sec->setButtonSymbols(QSpinBox::NoButtons);
    m_sec->setToolTip(tr("Seconds"));
    grid->addWidget(m_sec, 1, 2);
    connect(m_sec, SIGNAL(valueChanged(int)), this, SLOT(slotSecondsChanged()));
    connect(m_sec, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_ms = new FocusSpinBox(this);
    m_ms->setRange(0, MS_MAX / MS_DIV);
    m_ms->setPrefix(".");
    m_ms->setButtonSymbols(QSpinBox::NoButtons);
    m_ms->setToolTip(tr("Milliseconds"));
    grid->addWidget(m_ms, 1, 3);
    connect(m_ms, SIGNAL(valueChanged(int)), this, SLOT(slotMSChanged()));
    connect(m_ms, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_infiniteCheck = new QCheckBox(this);
    m_infiniteCheck->setText(tr("Infinite"));
    grid->addWidget(m_infiniteCheck, 2, 0, 1, 2);
    connect(m_infiniteCheck, SIGNAL(toggled(bool)), this, SLOT(slotInfiniteChecked(bool)));

    m_tap = new QPushButton(tr("Tap"), this);
    grid->addWidget(m_tap, 2, 2, 1, 2);
    connect(m_tap, SIGNAL(clicked()), this, SLOT(slotTapClicked()));

    m_focus = m_ms;
    m_dial->setRange(m_focus->minimum(), m_focus->maximum());
    m_dial->setSingleStep(m_focus->singleStep());

    m_timer->setInterval(TIMER_HOLD);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotPlusMinusTimeout()));

    m_tapTime->start();
}

SpeedDial::~SpeedDial()
{
    delete m_tapTime;
    m_tapTime = NULL;
}

void SpeedDial::setValue(int ms, bool emitValue)
{
    if (emitValue == false)
        m_preventSignals = true;

    m_value = ms;
    setSpinValues(ms);

    if (ms == (int) Function::infiniteSpeed())
        m_infiniteCheck->setChecked(true);
    else
        m_infiniteCheck->setChecked(false);

    m_preventSignals = false;
}

int SpeedDial::value() const
{
    return m_value;
}

void SpeedDial::tap()
{
    m_tap->click();
}

/*****************************************************************************
 * Private
 *****************************************************************************/

void SpeedDial::setSpinValues(int ms)
{
    if (ms == (int) Function::infiniteSpeed())
    {
        m_hrs->setValue(m_hrs->minimum());
        m_min->setValue(m_min->minimum());
        m_sec->setValue(m_sec->minimum());
        m_ms->setValue(m_ms->minimum());
    }
    else
    {
        ms = CLAMP(ms, 0, INT_MAX);

        m_hrs->setValue(ms / MS_PER_HOUR);
        ms -= (m_hrs->value() * MS_PER_HOUR);

        m_min->setValue(ms / MS_PER_MINUTE);
        ms -= (m_min->value() * MS_PER_MINUTE);

        m_sec->setValue(ms / MS_PER_SECOND);
        ms -= (m_sec->value() * MS_PER_SECOND);

        m_ms->setValue(ms / MS_DIV);
    }
}

int SpeedDial::spinValues() const
{
    int value = 0;

    if (m_infiniteCheck->isChecked() == false)
    {
        value += m_hrs->value() * MS_PER_HOUR;
        value += m_min->value() * MS_PER_MINUTE;
        value += m_sec->value() * MS_PER_SECOND;
        value += m_ms->value() * MS_DIV;
    }
    else
    {
        value = Function::infiniteSpeed();
    }

    return CLAMP(value, 0, INT_MAX);
}

int SpeedDial::dialDiff(int value, int previous, int step)
{
    int diff = value - previous;
    if (diff > THRESHOLD)
        diff = -step;
    else if (diff < (-THRESHOLD))
        diff = step;
    return diff;
}

void SpeedDial::slotPlusMinus()
{
    if (m_minus->isDown() == true || m_plus->isDown() == true)
    {
        slotPlusMinusTimeout();
        m_timer->start(TIMER_HOLD);
    }
    else
    {
        m_timer->stop();
    }
}

void SpeedDial::slotPlusMinusTimeout()
{
    if (m_minus->isDown() == true)
    {
        if (m_dial->value() == m_dial->minimum())
            m_dial->setValue(m_dial->maximum()); // Wrap around
        else
            m_dial->setValue(m_dial->value() - m_dial->singleStep()); // Normal increment
        m_timer->start(TIMER_REPEAT);
    }
    else if (m_plus->isDown() == true)
    {
        if (m_dial->value() == m_dial->maximum())
            m_dial->setValue(m_dial->minimum()); // Wrap around
        else
            m_dial->setValue(m_dial->value() + m_dial->singleStep()); // Normal increment
        m_timer->start(TIMER_REPEAT);
    }
}

void SpeedDial::slotDialChanged(int value)
{
    Q_ASSERT(m_focus != NULL);

    int newValue = dialDiff(value, m_previousDialValue, m_dial->singleStep()) + m_focus->value();
    if (newValue > m_focus->maximum())
    {
        // Incremented value is above m_focus->maximum(). Spill the overflow to the
        // bigger number (unless already incrementing hours).
        if (m_focus == m_ms)
            m_value += (m_ms->singleStep() * MS_DIV);
        else if (m_focus == m_sec)
            m_value += MS_PER_SECOND;
        else if (m_focus == m_min)
            m_value += MS_PER_MINUTE;

        m_value = CLAMP(m_value, 0, INT_MAX);
        setSpinValues(m_value);
    }
    else if (newValue < m_focus->minimum())
    {
        newValue = m_value;
        // Decremented value is below m_focus->minimum(). Spill the underflow to the
        // smaller number (unless already decrementing milliseconds).
        if (m_focus == m_ms)
            newValue -= (m_ms->singleStep() * MS_DIV);
        else if (m_focus == m_sec)
            newValue -= MS_PER_SECOND;
        else if (m_focus == m_min)
            newValue -= MS_PER_MINUTE;

        if (newValue >= 0)
        {
            m_value = newValue;
            m_value = CLAMP(m_value, 0, INT_MAX);
            setSpinValues(m_value);
        }
    }
    else
    {
        // Normal value increment/decrement.
        m_value = newValue;
        m_value = CLAMP(m_value, 0, INT_MAX);
        m_focus->setValue(m_value);
    }

    // Store the current value so it can be compared on the next pass to determine the
    // dial's direction of rotation.
    m_previousDialValue = value;
}

void SpeedDial::slotHoursChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotMinutesChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotSecondsChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotMSChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
}

void SpeedDial::slotInfiniteChecked(bool state)
{
    m_minus->setEnabled(!state);
    m_dial->setEnabled(!state);
    m_plus->setEnabled(!state);
    m_hrs->setEnabled(!state);
    m_min->setEnabled(!state);
    m_sec->setEnabled(!state);
    m_ms->setEnabled(!state);
    m_tap->setEnabled(!state);

    if (state == true)
    {
        m_value = Function::infiniteSpeed();
        if (m_preventSignals == false)
            emit valueChanged(Function::infiniteSpeed());
    }
    else
    {
        m_value = spinValues();
        if (m_preventSignals == false)
            emit valueChanged(m_value);
    }
}

void SpeedDial::slotSpinFocusGained()
{
    m_focus = qobject_cast <FocusSpinBox*> (QObject::sender());
    Q_ASSERT(m_focus != NULL);
    m_dial->setRange(m_focus->minimum(), m_focus->maximum());
    m_dial->setSingleStep(m_focus->singleStep());
}

void SpeedDial::slotTapClicked()
{
    // Round the elapsed time to the nearest full 10th ms.
    int remainder = m_tapTime->elapsed() % MS_DIV;
    m_value = m_tapTime->elapsed() - remainder;
    if (remainder >= (MS_DIV / 2))
        m_value += MS_DIV;
    setSpinValues(m_value);
    m_tapTime->restart();
    emit tapped();
}
