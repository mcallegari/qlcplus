/*
  Q Light Controller Plus
  loopback.h

  Copyright (c) Jano Svitok

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

#ifndef LOOPBACK_H
#define LOOPBACK_H

#include <QString>

#include "qlcioplugin.h"
#include "qlcmacros.h"

class QLC_DECLSPEC Loopback : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** @reimp */
    virtual ~Loopback();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    /*************************************************************************
     * Outputs
     *************************************************************************/
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

    /** @reimp */
    void sendFeedBack(quint32 universe, quint32 input, quint32 channel, uchar value, const QVariant &params);

private:
    //! loopback line -> channel data
    QMap<quint32, QByteArray> m_channelData;

    typedef QMap<quint32, quint32> TLineUniverseMap;

    //! output line -> universe
    TLineUniverseMap m_outputMap;

    //! input line -> universe
    TLineUniverseMap m_inputMap;
};

#endif
