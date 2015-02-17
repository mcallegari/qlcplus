/*
  Q Light Controller
  speeddial.cpp

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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFocusEvent>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QTimer>
#include <QDebug>
#include <QDial>

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
#define TAP_STOP_TIMEOUT 30000

#define DEFAULT_VISIBILITY_MASK 0x00FF

const QString tapDefaultSS = "QPushButton { background-color: #DDDDDD; border: 2px solid #6A6A6A; border-radius: 5px; }"
                             "QPushButton:pressed { background-color: #AAAAAA; }"
                             "QPushButton:disabled { border: 2px solid #BBBBBB; }";

const QString tapTickSS = "QPushButton { background-color: #DDDDDD; border: 3px solid #2B2595; border-radius: 5px; }"
                          "QPushButton:pressed { background-color: #AAAAAA; }"
                          "QPushButton:disabled { border: 2px solid #BBBBBB; }";


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
    , m_tapTime(NULL)
    , m_tapTickTimer(NULL)
    , m_tapTick(false)
    , m_visibilityMask(DEFAULT_VISIBILITY_MASK)
{
    new QVBoxLayout(this);
    layout()->setSpacing(0);
    layout()->setMargin(2);

    QHBoxLayout* topHBox = new QHBoxLayout();
    QVBoxLayout* pmVBox = new QVBoxLayout();
    layout()->addItem(topHBox);

    m_plus = new QToolButton(this);
    m_plus->setIconSize(QSize(32, 32));
    m_plus->setIcon(QIcon(":/edit_add.png"));
    pmVBox->addWidget(m_plus, Qt::AlignVCenter | Qt::AlignLeft);
    connect(m_plus, SIGNAL(pressed()), this, SLOT(slotPlusMinus()));
    connect(m_plus, SIGNAL(released()), this, SLOT(slotPlusMinus()));

    m_minus = new QToolButton(this);
    m_minus->setIconSize(QSize(32, 32));
    m_minus->setIcon(QIcon(":/edit_remove.png"));
    pmVBox->addWidget(m_minus, Qt::AlignVCenter | Qt::AlignLeft);
    connect(m_minus, SIGNAL(pressed()), this, SLOT(slotPlusMinus()));
    connect(m_minus, SIGNAL(released()), this, SLOT(slotPlusMinus()));
    topHBox->addItem(pmVBox);

    m_dial = new QDial(this);
    m_dial->setWrapping(true);
    m_dial->setNotchesVisible(true);
    m_dial->setTracking(true);
    topHBox->addWidget(m_dial);
    connect(m_dial, SIGNAL(valueChanged(int)), this, SLOT(slotDialChanged(int)));

    m_tap = new QPushButton(tr("Tap"), this);
    m_tap->setStyleSheet(tapDefaultSS);
    m_tap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    topHBox->addWidget(m_tap);
    connect(m_tap, SIGNAL(clicked()), this, SLOT(slotTapClicked()));

    QHBoxLayout* timeHBox = new QHBoxLayout();
    layout()->addItem(timeHBox);

    m_hrs = new FocusSpinBox(this);
    m_hrs->setRange(0, HRS_MAX);
    m_hrs->setSuffix("h");
    m_hrs->setButtonSymbols(QSpinBox::NoButtons);
    m_hrs->setToolTip(tr("Hours"));
    timeHBox->addWidget(m_hrs);
    connect(m_hrs, SIGNAL(valueChanged(int)), this, SLOT(slotHoursChanged()));
    connect(m_hrs, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_min = new FocusSpinBox(this);
    m_min->setRange(0, MIN_MAX);
    m_min->setSuffix("m");
    m_min->setButtonSymbols(QSpinBox::NoButtons);
    m_min->setToolTip(tr("Minutes"));
    timeHBox->addWidget(m_min);
    connect(m_min, SIGNAL(valueChanged(int)), this, SLOT(slotMinutesChanged()));
    connect(m_min, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_sec = new FocusSpinBox(this);
    m_sec->setRange(0, SEC_MAX);
    m_sec->setSuffix("s");
    m_sec->setButtonSymbols(QSpinBox::NoButtons);
    m_sec->setToolTip(tr("Seconds"));
    timeHBox->addWidget(m_sec);
    connect(m_sec, SIGNAL(valueChanged(int)), this, SLOT(slotSecondsChanged()));
    connect(m_sec, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_ms = new FocusSpinBox(this);
    m_ms->setRange(0, MS_MAX / MS_DIV);
    m_ms->setPrefix(".");
    m_ms->setButtonSymbols(QSpinBox::NoButtons);
    m_ms->setToolTip(tr("Milliseconds"));
    timeHBox->addWidget(m_ms);
    connect(m_ms, SIGNAL(valueChanged(int)), this, SLOT(slotMSChanged()));
    connect(m_ms, SIGNAL(focusGained()), this, SLOT(slotSpinFocusGained()));

    m_infiniteCheck = new QCheckBox(this);
    m_infiniteCheck->setText(tr("Infinite"));
    layout()->addWidget(m_infiniteCheck);
    connect(m_infiniteCheck, SIGNAL(toggled(bool)), this, SLOT(slotInfiniteChecked(bool)));

    m_focus = m_ms;
    m_dial->setRange(m_focus->minimum(), m_focus->maximum());
    m_dial->setSingleStep(m_focus->singleStep());

    m_timer->setInterval(TIMER_HOLD);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotPlusMinusTimeout()));
}

SpeedDial::~SpeedDial()
{
    stopTimers();
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

void SpeedDial::stopTimers(bool stopTime, bool stopTapTimer)
{
    if (stopTime && m_tapTime != NULL)
    {
        delete m_tapTime;
        m_tapTime = NULL;
    }
    if (stopTapTimer && m_tapTickTimer != NULL)
    {
        m_tapTickTimer->stop();
        delete m_tapTickTimer;
        m_tapTickTimer = NULL;
        m_tap->setStyleSheet(tapDefaultSS);
    }
}

/*****************************************************************************
 * Private
 *****************************************************************************/

void SpeedDial::setSpinValues(int ms)
{
    // block signals to prevent each single SpinBox to send
    // a valueChanged signal. For example going from 1m0s to 59s
    // would send two signals: 0 and then 59000.
    // We want to avoid that non-sense 0
    // Just send one single signal when everything has changed
    m_hrs->blockSignals(true);
    m_min->blockSignals(true);
    m_sec->blockSignals(true);
    m_ms->blockSignals(true);

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
    m_hrs->blockSignals(false);
    m_min->blockSignals(false);
    m_sec->blockSignals(false);
    m_ms->blockSignals(false);
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
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
        QString msText = m_ms->text();
        int msInt = m_ms->value();
        if (msInt < 10 && msText.contains("0") == false)
            value += (msInt * MS_DIV * 10);
        else
            value += (msInt * MS_DIV);
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

    // stop tap button blinking if it was
    stopTimers();

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
    // stop tap button blinking if it was
    stopTimers();
}

void SpeedDial::slotMinutesChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
    // stop tap button blinking if it was
    stopTimers();
}

void SpeedDial::slotSecondsChanged()
{
    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
    // stop tap button blinking if it was
    stopTimers();
}

void SpeedDial::slotMSChanged()
{
    m_ms->blockSignals(true);
    if (m_ms->value() < 10)
        m_ms->setPrefix(".0");
    else
        m_ms->setPrefix(".");
    m_ms->blockSignals(false);

    if (m_preventSignals == false)
    {
        m_value = spinValues();
        emit valueChanged(m_value);
    }
    // stop tap button blinking if it was
    stopTimers();
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
    // stop tap button blinking if it was
    stopTimers();
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
    if (m_tapTime == NULL)
    {
        m_tapTime = new QTime(QTime::currentTime());
        if (m_tapTickTimer == NULL)
        {
            m_tapTickTimer = new QTimer();
            connect(m_tapTickTimer, SIGNAL(timeout()),
                    this, SLOT(slotTapTimeout()));
        }
        m_tapTime->start();
        return;
    }
    // Round the elapsed time to the nearest full 10th ms.
    int remainder = m_tapTime->elapsed() % MS_DIV;
    m_value = m_tapTime->elapsed() - remainder;
    if (remainder >= (MS_DIV / 2))
        m_value += MS_DIV;
    setSpinValues(m_value);
    m_tapTime->restart();
    if (m_tapTickTimer)
    {
        m_tapTickTimer->setInterval(m_value);
        m_tapTickTimer->start();
    }
    emit tapped();
}

void SpeedDial::slotTapTimeout()
{
    if (m_tapTick == false)
        m_tap->setStyleSheet(tapTickSS);
    else
        m_tap->setStyleSheet(tapDefaultSS);
    m_tapTick = !m_tapTick;

    if (m_tapTime && m_tapTime->elapsed() >= TAP_STOP_TIMEOUT)
    {
        stopTimers(true, false);
    }
}

ushort SpeedDial::defaultVisibilityMask()
{
    return DEFAULT_VISIBILITY_MASK;
}

ushort SpeedDial::visibilityMask()
{
    return m_visibilityMask;
}

void SpeedDial::setVisibilityMask(ushort mask)
{
    if (mask & PlusMinus)
    {
        m_plus->show();
        m_minus->show();
    }
    else
    {
        m_plus->hide();
        m_minus->hide();
    }

    if (mask & Dial) m_dial->show();
    else m_dial->hide();

    if (mask & Tap) m_tap->show();
    else m_tap->hide();

    if (mask & Hours) m_hrs->show();
    else m_hrs->hide();

    if (mask & Minutes) m_min->show();
    else m_min->hide();

    if (mask & Seconds) m_sec->show();
    else m_sec->hide();

    if (mask & Milliseconds) m_ms->show();
    else m_ms->hide();

    if (mask & Infinite) m_infiniteCheck->show();
    else m_infiniteCheck->hide();

    m_visibilityMask = mask;
}
