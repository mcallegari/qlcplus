/*
  Q Light Controller
  midiplugin.h

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

class MidiPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "qlcplus.sourceforge.net" FILE "midi.json")
#endif

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
    void openOutput(quint32 output);

    /** @reimp */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 output, const QByteArray& universe);

private:
    /** Get an output device by its output index */
    MidiOutputDevice* outputDevice(quint32 output) const;

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
    void sendFeedBack(quint32 output, quint32 channel, uchar value, const QString& key);

    void sendRaw(quint32 output, const QString &data);

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
};

#endif
