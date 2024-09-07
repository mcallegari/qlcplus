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
    int m_line;
    bool m_enabled;
    int m_direction;
    uchar m_value;
    uchar m_count;
    QString m_name;
} GPIOLineInfo;

class ReadThread;

class GPIOPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

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

    std::string chipName() const;
    void setChipName(QString name);

    void updateLinesList();

    QList<GPIOLineInfo *> gpioList() const;

protected:
    std::string m_chipName;
    QList<GPIOLineInfo *> m_gpioList;
    ReadThread *m_readerThread;
    quint32 m_inputUniverse, m_outputUniverse;

    /*********************************************************************
     * GPIO line methods
     *********************************************************************/
public:
    enum LineDirection
    {
        NoDirection     = 1 << 0,
        OutputDirection = 1 << 1,
        InputDirection  = 1 << 2
    };

    QString lineDirectionToString(LineDirection usage);
    LineDirection stringToLineDirection(QString usage);

private:
    void setLineStatus(int lineNumber, bool enable);
    void setLineDirection(int lineNumber, LineDirection direction);
    void setLineValue(int lineNumber, uchar value);

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
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged);

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
