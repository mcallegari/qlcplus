/*
  Q Light Controller
  hid.h

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
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

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
