/*
  Q Light Controller Plus
  spiplugin.h

  Copyright (c) Massimo Callegari

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

#ifndef SPIPLUGIN_H
#define SPIPLUGIN_H

#include <QString>
#include <QMutex>
#include <QFile>
#include <QHash>

#include "qlcioplugin.h"

typedef struct
{
    /** number of channels used in a universe */
    ushort m_channels;
    /** absolute address where data of this universe
     *  starts in the m_serializedData array */
    ushort m_absoluteAddress;
    /** flag to instruct the SPI plugin to autodetect
     *  a universe size during a writeUniverse */
    bool m_autoDetection;
} SPIUniverse;

class SPIOutThread;

class SPIPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~SPIPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    void setParameter(QString name, QVariant &value);

private:
    void setAbsoluteAddress(quint32 uniID, SPIUniverse *uni);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output);

    /** @reimp */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

protected:
    /** File handle for /dev/spidev0.0 */
    int m_spifd;

    int m_referenceCount;

    /** Map of <Universe ID/number of channels> */
    QHash<quint32, SPIUniverse*> m_uniChannelsMap;

    /** Array holding all the universes data controlled
     *  by the SPI plugin, ready to be sent as a serial
     *  transfer */
    QByteArray m_serializedData;

    SPIOutThread *m_outThread;
    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input) { Q_UNUSED(input); return false; }

    /** @reimp */
    void closeInput(quint32 input) { Q_UNUSED(input); }

    /** @reimp */
    QStringList inputs() { return QStringList(); }

    /** @reimp */
    QString inputInfo(quint32 input) { Q_UNUSED(input); return QString(); }

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); Q_UNUSED(key); }

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();
};

#endif
