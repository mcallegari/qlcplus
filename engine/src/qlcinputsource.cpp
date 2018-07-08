/*
  Q Light Controller
  qlcinputsource.cpp

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

#include <QMutexLocker>
#include <QDebug>

#if defined(WIN32) || defined (Q_OS_WIN)
 #include <Windows.h>
#endif

#include "qlcinputchannel.h"
#include "qlcinputsource.h"
#include "qlcmacros.h"

quint32 QLCInputSource::invalidUniverse = UINT_MAX;
quint32 QLCInputSource::invalidChannel = UINT_MAX;
quint32 QLCInputSource::invalidID = UINT_MAX;

QLCInputSource::QLCInputSource(QThread *parent)
    : QThread(parent)
    , m_universe(invalidUniverse)
    , m_channel(invalidChannel)
    , m_id(invalidID)
    , m_lower(0)
    , m_upper(255)
    , m_workingMode(Absolute)
    , m_sensitivity(20)
    , m_emitExtraPressRelease(false)
    , m_inputValue(0)
    , m_outputValue(0)
    , m_running(false)
{
}

QLCInputSource::QLCInputSource(quint32 universe, quint32 channel, QThread *parent)
    : QThread(parent)
    , m_universe(universe)
    , m_channel(channel)
    , m_lower(0)
    , m_upper(255)
    , m_workingMode(Absolute)
    , m_sensitivity(20)
    , m_emitExtraPressRelease(false)
    , m_inputValue(0)
    , m_outputValue(0)
    , m_running(false)
{
}

QLCInputSource::~QLCInputSource()
{
    if (m_running == true)
    {
        m_running = false;
        wait();
    }
}

bool QLCInputSource::isValid() const
{
    if (universe() != invalidUniverse && channel() != invalidChannel)
        return true;
    else
        return false;
}

void QLCInputSource::setUniverse(quint32 uni)
{
    m_universe = uni;
}

quint32 QLCInputSource::universe() const
{
    return m_universe;
}

void QLCInputSource::setChannel(quint32 ch)
{
    m_channel = ch;
}

quint32 QLCInputSource::channel() const
{
    return m_channel;
}

void QLCInputSource::setPage(ushort pgNum)
{
    quint32 chCopy = m_channel & 0x0000FFFF;
    m_channel = ((quint32)pgNum << 16) | chCopy;
}

ushort QLCInputSource::page() const
{
    return (ushort)(m_channel >> 16);
}

void QLCInputSource::setID(quint32 id)
{
    m_id = id;
}

quint32 QLCInputSource::id() const
{
    return m_id;
}

void QLCInputSource::setRange(uchar lower, uchar upper)
{
    m_lower = lower;
    m_upper = upper;
}

uchar QLCInputSource::lowerValue() const
{
    return m_lower;
}

uchar QLCInputSource::upperValue() const
{
    return m_upper;
}

/*********************************************************************
 * Working mode
 *********************************************************************/

QLCInputSource::WorkingMode QLCInputSource::workingMode() const
{
    return m_workingMode;
}

void QLCInputSource::setWorkingMode(QLCInputSource::WorkingMode mode)
{
    m_workingMode = mode;

    if (m_workingMode == Relative && m_running == false)
    {
        m_inputValue = 127;
        m_running = true;
        start();
    }
    else if ((m_workingMode == Absolute || m_workingMode == Encoder) && m_running == true)
    {
        m_running = false;
        if (m_workingMode == Encoder)
            m_sensitivity = 1;
        wait();
        qDebug() << Q_FUNC_INFO << "Thread stopped for universe" << m_universe << "channel" << m_channel;
    }
}

bool QLCInputSource::needsUpdate()
{
    if (m_workingMode == Relative || m_workingMode == Encoder ||
        m_emitExtraPressRelease == true)
            return true;

    return false;
}

int QLCInputSource::sensitivity() const
{
    return m_sensitivity;
}

void QLCInputSource::setSensitivity(int value)
{
    m_sensitivity = value;
}

bool QLCInputSource::sendExtraPressRelease() const
{
    return m_emitExtraPressRelease;
}

void QLCInputSource::setSendExtraPressRelease(bool enable)
{
    m_emitExtraPressRelease = enable;
}

void QLCInputSource::updateInputValue(uchar value)
{
    QMutexLocker locker(&m_mutex);
    if (m_workingMode == Encoder)
    {
        if (value < m_inputValue)
            m_sensitivity = -qAbs(m_sensitivity);
        else if (value > m_inputValue)
            m_sensitivity = qAbs(m_sensitivity);
        m_inputValue = CLAMP(m_inputValue + (char)m_sensitivity, 0, UCHAR_MAX);
        locker.unlock();
        emit inputValueChanged(m_universe, m_channel, m_inputValue);
    }
    else if (m_emitExtraPressRelease == true)
    {
        locker.unlock();
        emit inputValueChanged(m_universe, m_channel, m_upper);
        emit inputValueChanged(m_universe, m_channel, m_lower);
    }
    else
        m_inputValue = value;
}

void QLCInputSource::updateOuputValue(uchar value)
{
    QMutexLocker locker(&m_mutex);
    m_outputValue = value;
}

void QLCInputSource::run()
{
    qDebug() << Q_FUNC_INFO << "Thread started for universe" << m_universe << "channel" << m_channel;

    uchar inputValueCopy = m_inputValue;
    double dValue = m_outputValue;
    uchar lastOutputValue = m_outputValue;
    bool movementOn = false;

    while (m_running == true)
    {
        msleep(50);

        QMutexLocker locker(&m_mutex);

        if (lastOutputValue != m_outputValue)
            dValue = m_outputValue;

        if (inputValueCopy != m_inputValue || movementOn == true)
        {
            movementOn = false;
            inputValueCopy = m_inputValue;
            double moveAmount = 127 - inputValueCopy;
            if (moveAmount != 0)
            {
                dValue -= (moveAmount / m_sensitivity);
                dValue = CLAMP(dValue, 0, 255);

                uchar newDmxValue = uchar(dValue);
                qDebug() << "double value:" << dValue << "uchar val:" << newDmxValue;
                if (newDmxValue != m_outputValue)
                    emit inputValueChanged(m_universe, m_channel, newDmxValue);

                movementOn = true;
            }
            lastOutputValue = m_outputValue;
        }
    }
}

