/*
  Q Light Controller Plus
  dmxusb.h

  Copyright (C) Heikki Junnila
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

#ifndef DMXUSB_H
#define DMXUSB_H

#include "qlcioplugin.h"

class DMXUSBWidget;

class DMXUSB : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** @reimp */
    virtual ~DMXUSB();

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

    /** Find out what kinds of widgets there are currently connected */
    bool rescanWidgets();

    /** Get currently connected widgets (input & output) */
    QList <DMXUSBWidget*> widgets() const;

private:
    /** List of references to the discovered USB widgets */
    QList <DMXUSBWidget*> m_widgets;

    /************************************************************************
     * Outputs
     ************************************************************************/
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
    /**
     *  List of references to USB widgets ordered by output lines.
     *  If a widget has multiple outputs, it will appear in this
     *  list multiple times
     */
    QList <DMXUSBWidget*> m_outputs;

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
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); Q_UNUSED(key); }

private:
    /**
     *  List of references to USB widgets ordered by input lines.
     *  If a widget has multiple inputs, it will appear in this
     *  list multiple times
     */
    QList <DMXUSBWidget*> m_inputs;

    /********************************************************************
     * Configuration
     ********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();
};

#endif
