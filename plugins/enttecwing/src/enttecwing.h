/*
  Q Light Controller
  enttecwing.h

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

    /** Attempt to bind the socket to listen to EWing::UDPPort */
    void reBindSocket();

    /*************************************************************************
     * Outputs
     *************************************************************************/
public:
    /** @reimp */
    void openOutput(quint32 output) { Q_UNUSED(output); }

    /** @reimp */
    void closeOutput(quint32 output) { Q_UNUSED(output); }

    /** @reimp */
    QStringList outputs() { return QStringList(); }

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe)
        { Q_UNUSED(output); Q_UNUSED(universe); }

    /** @reimp */
    QString outputInfo(quint32 output) { Q_UNUSED(output); return QString(); }

    /*************************************************************************
     * Inputs
     *************************************************************************/
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
    void slotPageChanged(quint32 pagesize, quint32 page);

protected:
    QList <Wing*> m_devices;
    QUdpSocket* m_socket;
    QString m_errorString;
};

#endif
