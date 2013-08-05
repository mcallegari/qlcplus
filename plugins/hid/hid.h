/*
  Q Light Controller
  hid.h

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

#ifndef HID_H
#define HID_H

#include <QEvent>
#include <QList>

#include "qlcioplugin.h"

class HIDPoller;
class HIDDevice;

/*****************************************************************************
 * HIDInputEvent
 *****************************************************************************/

class HIDInputEvent : public QEvent
{
public:
    HIDInputEvent(HIDDevice* device, quint32 input, quint32 channel,
                  uchar value, bool alive);
    ~HIDInputEvent();

    HIDDevice* m_device;
    quint32 m_input;
    quint32 m_channel;
    uchar m_value;
    bool m_alive;
};

/*****************************************************************************
 * HID
 *****************************************************************************/

class HID : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)

    friend class ConfigureHID;
    friend class HIDPoller;

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    void init();

    /** @reimp */
    virtual ~HID();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    /*********************************************************************
     * Inputs
     *********************************************************************/
public:
    /** @reimp */
    void openInput(quint32 input);

    /** @reimp */
    void closeInput(quint32 input);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); Q_UNUSED(key); }

protected:
    void customEvent(QEvent* event);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    void openOutput(quint32 output) { Q_UNUSED(output); }

    /** @reimp */
    void closeOutput(quint32 output) { Q_UNUSED(output); }

    /** @reimp */
    QStringList outputs() { return QStringList(); }

    /** @reimp */
    QString outputInfo(quint32 output) { Q_UNUSED(output); return QString(); }

    /** @reimp */
    void writeChannel(quint32 output, quint32 channel, uchar value)
        { Q_UNUSED(output); Q_UNUSED(channel); Q_UNUSED(value); }

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe)
        { Q_UNUSED(output); Q_UNUSED(universe); }

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

signals:
    /** @reimp */
    void configurationChanged();

    /*********************************************************************
     * Devices
     *********************************************************************/
public:
    void rescanDevices();

protected:
    HIDDevice* device(const QString& path);
    HIDDevice* device(quint32 index);

    void addDevice(HIDDevice* device);
    void removeDevice(HIDDevice* device);

signals:
    void deviceAdded(HIDDevice* device);
    void deviceRemoved(HIDDevice* device);

protected:
    QList <HIDDevice*> m_devices;

    /*********************************************************************
     * Device poller
     *********************************************************************/
public:
    void addPollDevice(HIDDevice* device);
    void removePollDevice(HIDDevice* device);

protected:
    HIDPoller* m_poller;
};

#endif
