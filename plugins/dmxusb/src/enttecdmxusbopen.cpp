/*
  Q Light Controller
  enttecdmxusbopen.cpp

  Copyright (C) Heikki Junnila
        		Christopher Staite

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#include <QSettings>
#include <QDebug>
#include <math.h>
#include <QTime>

#include "enttecdmxusbopen.h"
#include "qlcmacros.h"
#include "qlcftdi.h"

#define DMX_MAB 16
#define DMX_BREAK 110
#define DMX_CHANNELS 512
#define SETTINGS_FREQUENCY "enttecdmxusbopen/frequency"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOpen::EnttecDMXUSBOpen(const QString& serial, const QString& name, const QString& vendor,
                                   quint32 id, QObject* parent)
    : QThread(parent)
    , DMXUSBWidget(serial, name, vendor, NULL, id)
    , m_running(false)
    , m_universe(QByteArray(513, 0))
    , m_frequency(30)
    , m_granularity(Unknown)
{
    QSettings settings;
    QVariant var = settings.value(SETTINGS_FREQUENCY);
    if (var.isValid() == true)
        m_frequency = var.toDouble();
}

EnttecDMXUSBOpen::~EnttecDMXUSBOpen()
{
    stop();
}

DMXUSBWidget::Type EnttecDMXUSBOpen::type() const
{
    return DMXUSBWidget::OpenTX;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBOpen::open()
{
    if (DMXUSBWidget::open() == false)
        return close();

    if (ftdi()->clearRts() == false)
        return close();

    start(QThread::TimeCriticalPriority);
    return true;
}

bool EnttecDMXUSBOpen::close()
{
    stop();
    return DMXUSBWidget::close();
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBOpen::additionalInfo() const
{
    QString info;
    QString gran;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2").arg(tr("Protocol")).arg("Open DMX USB");
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Manufacturer"))
                                         .arg(vendor());
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2Hz").arg(tr("DMX Frame Frequency"))
                                      .arg(m_frequency);
    info += QString("<BR>");
    if (m_granularity == Bad)
        gran = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("Bad"));
    else if (m_granularity == Good)
        gran = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Good"));
    else
        gran = tr("Patch this widget to a universe to find out.");
    info += QString("<B>%1:</B> %2").arg(tr("System Timer Accuracy")).arg(gran);
    info += QString("</P>");

    return info;
}

/****************************************************************************
 * Thread
 ****************************************************************************/

bool EnttecDMXUSBOpen::writeUniverse(const QByteArray& universe)
{
    m_universe.replace(1, MIN(universe.size(), m_universe.size()), universe);
    return true;
}

void EnttecDMXUSBOpen::stop()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void EnttecDMXUSBOpen::run()
{
    // One "official" DMX frame can take (1s/44Hz) = 23ms
    int frameTime = (int) floor(((double)1000 / m_frequency) + (double)0.5);

    // Wait for device to settle in case the device was opened just recently
    // Also measure, whether timer granularity is OK
    QTime time;
    time.start();
    usleep(1000);
    if (time.elapsed() > 3)
        m_granularity = Bad;
    else
        m_granularity = Good;

    m_running = true;
    while (m_running == true)
    {
        // Measure how much time passes during these calls
        time.restart();

        if (ftdi()->setBreak(true) == false)
            goto framesleep;

        if (m_granularity == Good)
            usleep(DMX_BREAK);

        if (ftdi()->setBreak(false) == false)
            goto framesleep;

        if (m_granularity == Good)
            usleep(DMX_MAB);

        if (ftdi()->write(m_universe) == false)
            goto framesleep;

framesleep:
        // Sleep for the remainder of the DMX frame time
        if (m_granularity == Good)
            while (time.elapsed() < frameTime) { usleep(1000); }
        else
            while (time.elapsed() < frameTime) { /* Busy sleep */ }
    }
}
