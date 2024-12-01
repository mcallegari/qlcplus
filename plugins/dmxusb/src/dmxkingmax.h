/*
  Q Light Controller
  dmxkingmax.h

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

#ifndef DMXKINGMAX_H
#define DMXKINGMAX_H

#include <QByteArray>
#include <QThread>
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#include <QRecursiveMutex>
#else
#include <QMutex>
#endif

#include "dmxusbwidget.h"

class RDMProtocol;

/**
 * This is the base interface class for DMXKing MAX USB widgets.
 */
class DMXKingMAX : public QThread, public DMXUSBWidget
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    DMXKingMAX(DMXInterface *iface,
               quint32 outputLine, quint32 inputLine = 0);

    virtual ~DMXKingMAX();

    /** @reimp */
    Type type() const;

    /** @reimp */
    QString additionalInfo() const;

private:
    /** Send a request for information to the device */
    void getDeviceInfo();
    bool sendUDPMessage(QByteArray &data);
    bool receiveUDPMessage(QByteArray &data);

    /** Stop output thread */
    void stopThread();

    /** Input/Output thread worker method */
    void run();

private:
    bool m_threadRunning;

    /****************************************************************************
     * Open & Close
     ****************************************************************************/
public:
    /** @reimp */
    virtual bool open(quint32 line, bool input = false);

    /** @reimp */
    virtual bool close(quint32 line = 0, bool input = false);

    /************************************************************************
     * Name & Serial
     ************************************************************************/
public:
    /** @reimp */
    QString uniqueName(ushort line = 0, bool input = false) const;

private:
    QString m_serialNumber;

    /************************************************************************
     * Input
     ************************************************************************/
signals:
    /** Tells that the value of a received DMX channel has changed */
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);

    /************************************************************************
     * Output
     ************************************************************************/
public:
    /** @reimp */
    bool writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged);

    /********************************************************************
     * RDM
     ********************************************************************/
public:
    /** @reimp */
    bool supportRDM();

    /** @reimp */
    bool sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params);

signals:
    void rdmValueChanged(quint32 universe, quint32 line, QVariantMap data);

private:
    RDMProtocol *m_rdm;
};

#endif
