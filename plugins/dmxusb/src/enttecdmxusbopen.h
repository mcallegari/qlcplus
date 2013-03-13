/*
  Q Light Controller
  enttecdmxusbopen.h

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

#ifndef ENTTECDMXUSBOPEN_H
#define ENTTECDMXUSBOPEN_H

#include <QByteArray>
#include <QThread>
#include <QMutex>

#include "dmxusbwidget.h"

class QLCFTDI;

class EnttecDMXUSBOpen : public QThread, public DMXUSBWidget
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /**
     * Construct a new DMXUSBOpen object with the given parent and
     * FTDI device information. Neither can be null.
     *
     * @param serial The device's unique(ish) serial
     * @param name The friendly name of the device
     * @param id The device's unique ID (FTD2XX only)
     * @param parent The owner of this object
     */
    EnttecDMXUSBOpen(const QString& serial, const QString& name, const QString& vendor,
                     quint32 id = 0, QObject* parent = 0);

    /** Destructor */
    virtual ~EnttecDMXUSBOpen();

    /** @reimp */
    DMXUSBWidget::Type type() const;

    /************************************************************************
     * Open & Close
     ************************************************************************/
public:
    /** @reimp */
    bool open();

    /** @reimp */
    bool close();

    /************************************************************************
     * Name & Serial
     ************************************************************************/
public:
    /** @reimp */
    QString additionalInfo() const;

    /************************************************************************
     * Thread
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(const QByteArray& universe);

protected:
    enum TimerGranularity { Unknown, Good, Bad };

    /** Stop the writer thread */
    void stop();

    /** DMX writer thread worker method */
    void run();

protected:
    bool m_running;
    QByteArray m_universe;
    double m_frequency;
    TimerGranularity m_granularity;
};

#endif
