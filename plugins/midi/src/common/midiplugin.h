/*
  Q Light Controller
  midiplugin.h

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

#ifndef MIDIPLUGIN_H
#define MIDIPLUGIN_H

#define KExtMidiTemplate ".qxm" // 'Q'LC+ 'X'ml 'M'idi template

#include <QStringList>
#include <QList>
#include <QDir>

#include "qlcioplugin.h"
#include "miditemplate.h"

class ConfigureMIDIPlugin;
class MidiOutputDevice;
class MidiInputDevice;
class MidiEnumerator;
class MidiTemplate;
class QString;

#define MIDI_MIDICHANNEL "midichannel"
#define MIDI_MODE "mode"
#define MIDI_INITMESSAGE "initmessage"

class MidiPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    friend class ConfigureMidiPlugin;

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** @reimp */
    ~MidiPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

private:
    MidiEnumerator* m_enumerator;

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

private:
    /** Get an output device by its output index */
    MidiOutputDevice* outputDevice(quint32 output) const;

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
    void sendFeedBack(quint32 universe, quint32 output, quint32 channel, uchar value, const QVariant &params);

    void sendSysEx(quint32 output, const QByteArray &data);

private:
    /** Get an output device by its output index */
    MidiInputDevice* inputDevice(quint32 input) const;

private slots:
    /** Catch MIDI input device valueChanged signals */
    void slotValueChanged(const QVariant& uid, ushort channel, uchar value);

    /*************************************************************************
     * Configuration
     *************************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    /** @reimp */
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value);

    /*************************************************************************
     * Midi templates
     *************************************************************************/
public:
    QDir userMidiTemplateDirectory();

    QDir systemMidiTemplateDirectory();

    bool addMidiTemplate(MidiTemplate* templ);

    MidiTemplate* midiTemplate(QString name);

    void loadMidiTemplates(const QDir& dir);

    QList <MidiTemplate*> midiTemplates();

private:
    /** List that contains all available midi templates */
    QList <MidiTemplate*> m_midiTemplates;

    /********************************************************************
     * Hotplug
     ********************************************************************/
public slots:
    void slotDeviceAdded(uint vid, uint pid);
    void slotDeviceRemoved(uint vid, uint pid);
};

#endif
