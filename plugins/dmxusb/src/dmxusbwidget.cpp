/*
  Q Light Controller Plus
  dmxusbwidget.cpp

  Copyright (C) Heikki Junnila
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

#include <QStringList>
#include <QDebug>
#include "dmxusbwidget.h"

DMXUSBWidget::DMXUSBWidget(const QString& serial, const QString& name, const QString& vendor,
                           quint32 outputLine, quint32 id)
{
    m_outputBaseLine = outputLine;
    m_inputBaseLine = 0;

    m_inputOpenMask = 0;
    m_outputOpenMask = 0;

    setOutputsNumber(1);
    setInputsNumber(0);

    m_ftdi = new QLCFTDI(serial, name, vendor, id);
}

DMXUSBWidget::~DMXUSBWidget()
{
    delete m_ftdi;
}

QLCFTDI* DMXUSBWidget::ftdi() const
{
    return m_ftdi;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool DMXUSBWidget::open(quint32 line, bool input)
{
    if (input == true && m_inputsMap.contains(line))
    {
        quint32 devLine = m_inputsMap[line];
        m_inputOpenMask |= (1 << devLine);
    }
    else if (input == false && m_outputsMap.contains(line))
    {
        quint32 devLine = m_outputsMap[line];
        m_outputOpenMask |= (1 << devLine);
    }
    else
    {
        qWarning() << "Line" << line << "doesn't belong to any mapped inputs nor to outputs !";
        return false;
    }

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", Input mask:" << m_inputOpenMask << ", Output Mask:" << m_outputOpenMask;

    if (isOpen() == true)
        return true; //close();

    if (this->type() == DMXUSBWidget::DMX4ALL)
    {
        if (m_ftdi->openByPID(QLCFTDI::DMX4ALLPID) == false)
            return close();
    }
    else
    {
        if (m_ftdi->open() == false)
            return close(line);
    }

    if (m_ftdi->reset() == false)
        return close(line);

    if (m_ftdi->setBaudRate() == false)
        return close(line);

    if (m_ftdi->setLineProperties() == false)
        return close(line);

    if (m_ftdi->setFlowControl() == false)
        return close(line);

    if (m_ftdi->purgeBuffers() == false)
        return close(line);

    qDebug() << Q_FUNC_INFO << "FTDI correctly opened and configured";

    return true;
}

bool DMXUSBWidget::close(quint32 line, bool input)
{
    if (input == true && m_inputsMap.contains(line))
    {
        quint32 devLine = m_inputsMap[line];
        m_inputOpenMask &= ~(1 << devLine);
    }
    else if (input == false && m_outputsMap.contains(line))
    {
        quint32 devLine = m_outputsMap[line];
        m_outputOpenMask &= ~(1 << devLine);
    }
    else
    {
        qWarning() << "Line" << line << "doesn't belong to any mapped inputs nor to outputs !";
        return false;
    }

    qDebug() << Q_FUNC_INFO << "Line:" << line << ", Input mask:" << m_inputOpenMask << ", Output Mask:" << m_outputOpenMask;

    if (m_inputOpenMask == 0 && m_outputOpenMask == 0)
    {
        qDebug() << Q_FUNC_INFO << "All inputs/outputs have been closed. Close FTDI too.";
        if (m_ftdi->isOpen())
            return m_ftdi->close();
        else
            return true;
    }

    return true;
}

bool DMXUSBWidget::isOpen()
{
    return m_ftdi->isOpen();
}

/********************************************************************
 * Outputs
 ********************************************************************/

void DMXUSBWidget::setOutputsNumber(int num)
{
    m_outputsNumber = num;
    m_outputsMap.clear();
    for (ushort i = 0; i < num; i++)
        m_outputsMap[m_outputBaseLine + i] = i;
    qDebug() << "[setOutputsNumber] base line:" << m_outputBaseLine << "outputMap:" << m_outputsMap;
}

int DMXUSBWidget::outputsNumber()
{
    return m_outputsNumber;
}

QStringList DMXUSBWidget::outputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_outputsNumber; i++)
        names << uniqueName(i, false);
    return names;
}

/********************************************************************
 * Inputs
 ********************************************************************/

void DMXUSBWidget::setInputsNumber(int num)
{
    m_inputsNumber = num;
    m_inputsMap.clear();
    for (ushort i = 0; i < num; i++)
        m_inputsMap[m_inputBaseLine + i] = i;
}

int DMXUSBWidget::inputsNumber()
{
    return m_inputsNumber;
}

QStringList DMXUSBWidget::inputNames()
{
    QStringList names;
    for (ushort i = 0; i < m_inputsNumber; i++)
        names << uniqueName(i, true);
    return names;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString DMXUSBWidget::name() const
{
    return m_ftdi->name();
}

QString DMXUSBWidget::serial() const
{
    return m_ftdi->serial();
}

QString DMXUSBWidget::uniqueName(ushort line, bool input) const
{
    Q_UNUSED(line)
    Q_UNUSED(input)
    return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

void DMXUSBWidget::setRealName(QString devName)
{
    m_realName = devName;
}

QString DMXUSBWidget::realName() const
{
    return m_realName;
}

QString DMXUSBWidget::vendor() const
{
    return m_ftdi->vendor();
}

/****************************************************************************
 * Write universe
 ****************************************************************************/

bool DMXUSBWidget::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)

    return false;
}
