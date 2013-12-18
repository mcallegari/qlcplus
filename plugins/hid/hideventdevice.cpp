/*
  Q Light Controller
  hideventdevice.cpp

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

#include <linux/input.h>
#include <errno.h>
#if !defined(WIN32) && !defined(Q_OS_WIN)
  #include <unistd.h>
#endif

#include <QApplication>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "hideventdevice.h"
#include "hid.h"

/**
 * This macro is used to tell if "bit" is set in "array".
 * It selects a byte from the array, and does a boolean AND
 * operation with a byte that only has the relevant bit set.
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit / 8] & (1 << (bit % 8)))

HIDEventDevice::HIDEventDevice(HID* parent, quint32 line,
                               const QString& path)
        : HIDDevice(parent, line, path)
{
    init();
}

HIDEventDevice::~HIDEventDevice()
{
    static_cast<HID*> (parent())->removePollDevice(this);
}

void HIDEventDevice::init()
{
    if (open() == false)
        return;

    qDebug() << "*******************************************************";
    qDebug() << "Device file:" << m_file.fileName();

    /* Device name */
    char name[128] = "Unknown";
    if (ioctl(m_file.handle(), EVIOCGNAME(sizeof(name)), name) <= 0)
    {
        m_name = QString("HID Input %1: %2").arg(m_line + 1)
                 .arg(strerror(errno));
        perror("ioctl EVIOCGNAME");
    }
    else
    {
        m_name = QString("HID Input %1: %2").arg(m_line + 1).arg(name);
        qDebug() << "Device name:" << m_name;
    }

    /* Supported event types */
    uint8_t mask[EV_MAX/8 + 1];
    if (ioctl(m_file.handle(), EVIOCGBIT(0, sizeof(mask)), mask) <= 0)
    {
        qDebug() << "Unable to get device features:"
        << strerror(errno);
    }
    else
    {
        getCapabilities(mask);
    }

    close();
}

void HIDEventDevice::getCapabilities(uint8_t* mask)
{
    Q_ASSERT(mask != NULL);
    Q_ASSERT(m_file.isOpen() == true);

    qDebug() << "Supported event types:";

    for (int i = 0; i < EV_MAX; i++)
    {
        if (test_bit(i, mask))
        {
            switch (i)
            {
            case EV_SYN:
                break;

            case EV_ABS:
                qDebug() << "\tAbsolute Axes";
                getAbsoluteAxesCapabilities();
                break;

            case EV_REL:
                qDebug() << "\tRelative axes";
                break;

            case EV_KEY:
                qDebug() << "\tKeys or Buttons";
                break;

            case EV_LED:
                qDebug() << "\tLEDs";
                break;

            case EV_REP:
                qDebug() << "\tRepeat";
                break;

            case EV_SW:
                qDebug() << "\tSwitches";
                break;

            case EV_SND:
                qDebug() << "\tSounds";
                break;

            case EV_MSC:
                qDebug() << "\tMiscellaneous";
                break;

            case EV_FF:
                qDebug() << "\tForce feedback";
                break;

            case EV_FF_STATUS:
                qDebug() << "\tForce feedback status";
                break;

            case EV_PWR:
                qDebug() << "\tPower";
                break;

            default:
                qDebug() << "\tUnknown event type: " << i;
                break;
            }
        }
    }
}

void HIDEventDevice::getAbsoluteAxesCapabilities()
{
    uint8_t mask[ABS_MAX/8 + 1];
    struct input_absinfo feats;
    int r;

    Q_ASSERT(m_file.isOpen() == true);

    memset(mask, 0, sizeof(mask));
    r = ioctl(m_file.handle(), EVIOCGBIT(EV_ABS, sizeof(mask)), mask);
    if (r < 0)
    {
        qWarning() << "\t\tUnable to get axes:" << strerror(errno);
        return;
    }

    for (int i = 0; i < ABS_MAX; i++)
    {
        if (test_bit(i, mask) == 0)
            continue;

        r = ioctl(m_file.handle(), EVIOCGABS(i), &feats);
        if (r != 0)
        {
            qWarning() << "\t\tUnable to get axes' features:"
                       << strerror(errno);
        }
        else
        {
            qDebug() << "\t\tChannel:" << i
                     << "min:" << feats.minimum
                     << "max:" << feats.maximum
                     << "flatness:" << feats.flat
                     << "fuzz:" << feats.fuzz;

            if (feats.maximum > 0)
                m_scales[i] = feats;
            else
                continue;
        }
    }
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDEventDevice::open()
{
    bool result = false;

    result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
    if (result == false)
    {
        result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadOnly);
        if (result == false)
        {
            qWarning() << "Unable to open" << m_file.fileName()
                       << m_file.errorString();
        }
        else
        {
            qDebug() << "Opened" << m_file.fileName() << "in read only mode";
        }
    }

    return result;
}

void HIDEventDevice::close()
{
    m_file.close();
}

QString HIDEventDevice::path() const
{
    return m_file.fileName();
}

bool HIDEventDevice::readEvent()
{
    struct input_event ev;
    HIDInputEvent* e;
    int r;

    r = read(m_file.handle(), &ev, sizeof(struct input_event));
    if (r > 0)
    {
        uchar val;

        /* Accept only these kinds of events */
        if (ev.type != EV_ABS && ev.type != EV_REL &&
                ev.type != EV_KEY && ev.type != EV_SW)
        {
            return true;
        }

        /* Find scaling data */
        if (m_scales.contains(ev.code) == true)
        {
            /* Scaling data found, this is an abs/rel channel */
            struct input_absinfo sc;

            sc = m_scales[ev.code];

            /* Scale the device's native value range to
               0 - UCHAR_MAX:
               y = (x - from_min) * (to_max / from_range)
            */
            val = (ev.value - sc.minimum);
            val *= (UCHAR_MAX / (sc.maximum - sc.minimum));
        }
        else
        {
            /* Buttons are either fully on or fully off */
            if (ev.value != 0)
                val = UCHAR_MAX;
            else
                val = 0;
        }

        /* Post the event to the global event loop so
           that we can switch context away from the
           poller thread and into the main application
           thread. This is caught in
           HID::customEvent(). */
        e = new HIDInputEvent(this, m_line, ev.code, val, true);
        QApplication::postEvent(parent(), e);

        return true;
    }
    else
    {
        e = new HIDInputEvent(this, 0, 0, 0, false);
        QApplication::postEvent(parent(), e);

        return false;
    }
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDEventDevice::infoText()
{
    QString info;
    QString str;

    info += QString("<TR>");

    /* File name */
    info += QString("<TD>");
    info += m_file.fileName();
    info += QString("</TD>");

    /* Name */
    info += QString("<TD>");
    info += m_name;
    info += QString("</TD>");

    /* Channels */
    info += QString("<TD ALIGN=\"CENTER\">");
    info += tr("N/A");
    info += QString("</TD>");

    info += QString("</TR>");

    return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDEventDevice::feedBack(quint32 channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

