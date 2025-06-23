/*
  Q Light Controller
  enttecdmxusbpro.h

  Copyright (C) Heikki Junnila

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

#ifndef ENTTECDMXUSBPRO_H
#define ENTTECDMXUSBPRO_H

#include <QByteArray>
#include <QVariant>
#include <QThread>

#include "dmxusbwidget.h"

#define ULTRADMX_PRO_DEV_ID      0x02

class RDMProtocol;

/**
 * This is the base interface class for ENTTEC USB DMX Pro widgets.
 */
class EnttecDMXUSBPro : public QThread, public DMXUSBWidget
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    EnttecDMXUSBPro(DMXInterface *iface,
                    quint32 outputLine, quint32 inputLine = 0);

    virtual ~EnttecDMXUSBPro();

    /** @reimp */
    Type type() const;

    // DMXking port flags
    enum PortType
    {
        Output              = 1 << 0,
        Input               = 1 << 1,
        USB_DMX_Forward     = 1 << 2,
        ArtNet_sACN_Forward = 1 << 3,
        ArtNet_sACN_Select  = 1 << 4
    };

    enum ActionType
    {
        OpenLine,
        CloseLine,
        RDMCommand
    };

    enum RDMOperation
    {
        None = 0,
        Discovery,
        GetSetCommand
    };

    typedef struct
    {
        ActionType action;
        QVariant param1;
        QVariant param2;
        QVariantList param3;
    } InterfaceAction;

    static bool writeLabelRequest(DMXInterface *iface, int label);

    static bool readResponse(DMXInterface *iface, char label, QByteArray &payload);

    static void parsePortFlags(const QByteArray &inArray, QByteArray &outArray);

    static bool detectDMXKingDevice(DMXInterface *iface,
                                    QString &manufName, QString &deviceName,
                                    int &ESTA_ID, int &DEV_ID,
                                    QByteArray &portDirection);

    /** Set a special flag to use DMXKing specific commands */
    void setDMXKingMode();

    /** @reimp */
    QString additionalInfo() const;

private:
    bool m_dmxKingMode;

    /****************************************************************************
     * Open & Close
     ****************************************************************************/
private:
     bool configureLine(ushort dmxLine, bool isMidi);

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
    /** Extract the widget's unique serial number (printed on the bottom) */
    bool extractSerial();

private:
    QString m_proSerial;

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

    /************************************************************************
     * Input/Output Thread
     ************************************************************************/
private:
    /** @reimp - Input/Output thread worker method */
    void run();

    /** Stop input/output thread */
    void stopThread();

    int readData(QByteArray &payload, bool &isMIDI, int RDM = None);

private:
    /** Flag that indicates if the input/output thread is running */
    bool m_isThreadRunning;

    /** Exchange queue between the main thread and
     *  the input/output thread to perform synchronous
     *  operations and guarantee thread safety */
    QList<InterfaceAction> m_actionsQueue;

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
    quint32 m_universe;
};

#endif
