/*
  Q Light Controller
  enttecdmxusbprorx.cpp

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#ifndef ENTTECDMXUSBPRORX_H
#define ENTTECDMXUSBPRORX_H

#include <QThread>
#include <QMutex>
#include "enttecdmxusbpro.h"

class EnttecDMXUSBProRX : public QThread, public EnttecDMXUSBPro
{
    Q_OBJECT

public:
    EnttecDMXUSBProRX(const QString& serial, const QString& name, const QString& vendor,
                      quint32 input, QLCFTDI *ftdi = NULL, quint32 id = 0);
    ~EnttecDMXUSBProRX();

    /** @reimp */
    Type type() const;

private:
    const quint32 m_input;

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
     * DMX reception
     ************************************************************************/
    /** @reimp */
    QString uniqueName() const;

signals:
    /** Tells that the value of a received DMX channel has changed */
    void valueChanged(quint32 input, quint32 channel, uchar value);

private:
    /** Stop DMX receiver thread */
    void stop();

    /** DMX receiver thread worker method */
    void run();

private:
    bool m_running;
    QMutex m_mutex;
    QByteArray m_universe;
};

#endif
