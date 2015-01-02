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
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

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

    /** @reimp */
    void setParameter(QString name, QVariant &value)
    { Q_UNUSED(name); Q_UNUSED(value); }

    /*************************************************************************
     * Outputs
     *************************************************************************/
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

private:
    bool m_outputCurrentlyOpen[QLCIOPLUGINS_UNIVERSES];
    qint32* m_values[QLCIOPLUGINS_UNIVERSES];

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

private:
    bool m_inputCurrentlyOpen[QLCIOPLUGINS_UNIVERSES];

    /*************************************************************************
     * Configuration
     *************************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();
};

#endif
