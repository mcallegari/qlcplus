/*
  Q Light Controller Plus
  gpioplugin.h

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

#ifndef GPIOPLUGIN_H
#define GPIOPLUGIN_H

#include <QFile>

#include "qlcioplugin.h"

#define GPIO_PARAM_USAGE "pinUsage"

typedef struct
{
    int m_number;
    bool m_enabled;
    int m_usage;
    QFile *m_file;
    uchar m_value;
    uchar m_count;
} GPIOPinInfo;

class ReadThread;

class GPIOPlugin : public QLCIOPlugin
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
    virtual ~GPIOPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    QList<GPIOPinInfo *> gpioList() const;

protected:
    QList<GPIOPinInfo *> m_gpioList;
    ReadThread *m_readerThread;
    quint32 m_inputUniverse, m_outputUniverse;

    /*********************************************************************
     * GPIO PIN methods
     *********************************************************************/
public:
    enum PinUsage
    {
        NoUsage     = 1 << 0,
        OutputUsage = 1 << 1,
        InputUsage  = 1 << 2
    };

    QString pinUsageToString(PinUsage usage);
    PinUsage stringToPinUsage(QString usage);

private:
    void setPinStatus(int gpioNumber, bool enable);
    void setPinUsage(int gpioNumber, PinUsage usage);
    void setPinValue(int gpioNumber, uchar value);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output, quint32 universe);

    /** @reimp */
    void closeOutput(quint32 output, quint32 universe);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input, quint32 universe);

    /** @reimp */
    void closeInput(quint32 input, quint32 universe);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

protected slots:
    void slotValueChanged(quint32 channel, uchar value);

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    /** @reimp */
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value);
};

#endif
