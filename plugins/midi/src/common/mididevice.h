/*
  Q Light Controller
  mididevice.h

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

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include <QVariant>
#include <QObject>

#include "miditemplate.h"

class MidiDevice : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Device Type
     ************************************************************************/
public:
    enum DeviceType { Input, Output };

    DeviceType deviceType() const;

    static QString deviceTypeToString(DeviceType deviceType);

private:
    DeviceType m_deviceType;


    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    MidiDevice(const QVariant& uid, const QString& name, DeviceType deviceType, QObject* parent = 0);
    virtual ~MidiDevice();

    QVariant uid() const;
    QString name() const;

private:
    void loadSettings();
    void saveSettings() const;

private:
    const QVariant m_uid;
    const QString m_name;


    /************************************************************************
     * MIDI Channel
     ************************************************************************/
public:
    void setMidiChannel(int channel);
    int midiChannel() const;

private:
    int m_midiChannel;

    /************************************************************************
     * Mode
     ************************************************************************/
public:
    enum Mode { ControlChange, Note, ProgramChange };

    void setMode(Mode mode);
    Mode mode() const;

    static QString modeToString(Mode mode);
    static Mode stringToMode(const QString& mode);

private:
    Mode m_mode;

    /************************************************************************
    * Send Note OFF
    ************************************************************************/
public:
    void setSendNoteOff(bool sendNoteOff);
    bool sendNoteOff() const;

private:
    bool m_sendNoteOff;

    /************************************************************************
     * Midi template
     ************************************************************************/
public:
    void setMidiTemplateName(QString midiTemplateName);
    QString midiTemplateName() const;

private:
    QString m_midiTemplateName;

    /************************************************************************
     * Virtual Open/Close
     ************************************************************************/
public:
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};

#endif
