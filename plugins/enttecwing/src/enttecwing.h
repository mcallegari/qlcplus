/*
  Q Light Controller
  enttecwing.h

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

#ifndef ENTTECWING_H
#define ENTTECWING_H

#include <QHostAddress>
#include <QStringList>
#include <QList>

#include "qlcioplugin.h"
#include "wing.h"

class QUdpSocket;

/*****************************************************************************
 * EWingInput
 *****************************************************************************/

class EnttecWing : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** @reimp */
    void init();

    /** @reimp */
    virtual ~EnttecWing();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    /** @reimp */
    void setParameter(QString name, QVariant &value)
    { Q_UNUSED(name); Q_UNUSED(value); }

    /** Attempt to bind the socket to listen to EWing::UDPPort */
    bool reBindSocket();

    /*************************************************************************
     * Outputs
     *************************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output) { Q_UNUSED(output); return false; }

    /** @reimp */
    void closeOutput(quint32 output) { Q_UNUSED(output); }

    /** @reimp */
    QStringList outputs() { return QStringList(); }

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
    { Q_UNUSED(output); Q_UNUSED(universe); Q_UNUSED(data); }

    /** @reimp */
    QString outputInfo(quint32 output) { Q_UNUSED(output); return QString(); }

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input);

    /** @reimp */
    void closeInput(quint32 input);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key);

    /*************************************************************************
     * Configuration
     *************************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

signals:
    /** @reimp */
    void configurationChanged();

    /*************************************************************************
     * Devices
     *************************************************************************/
protected:
    /**
     * Create a new wing object from the given datagram packet. Looks up
     * the exact wing type from data and creates a PlaybackWing,
     * a ShortcutWing or a ProgramWing.
     *
     * @param parent The parent object that owns the new wing object
     * @param address The address of the physical wing board
     * @param data A UDP datagram packet originating from a wing
     *
     * @return A new Wing object or NULL if an error occurred
     */
    static Wing* createWing(QObject* parent, const QHostAddress& address,
                            const QByteArray& data);

    /** Find a specific device by its host address and type */
    Wing* device(const QHostAddress& address, Wing::Type type);

    /** Find a device by its index (input line) */
    Wing* device(quint32 index);

    /** Add a newly-created device to the plugin's list of devices */
    void addDevice(Wing* device);

    /** Remove a specific device from the plugin */
    void removeDevice(Wing* device);

protected slots:
    void slotReadSocket();
    void slotValueChanged(quint32 channel, uchar value);

protected:
    QList <Wing*> m_devices;
    QUdpSocket* m_socket;
    QString m_errorString;
};

#endif
