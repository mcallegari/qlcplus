/*
  Q Light Controller Plus
  inputprofileeditor.h

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

#ifndef INPUTPROFILEEDITOR_H
#define INPUTPROFILEEDITOR_H

#include <QQmlListProperty>
#include <QVariant>
#include <QObject>
#include <QMap>

#include "qlcinputprofile.h"
#include "midiprotocol.h"

class Doc;
class QLCInputChannel;

class InputProfileEditor : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InputProfileEditor)

    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged FINAL)
    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged FINAL)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(QLCInputProfile::Type type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(bool midiNoteOff READ midiNoteOff WRITE setMidiNoteOff NOTIFY midiNoteOffChanged FINAL)
    Q_PROPERTY(QVariant channels READ channels NOTIFY channelsChanged FINAL)
    Q_PROPERTY(QVariantList channelTypeModel READ channelTypeModel CONSTANT)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    InputProfileEditor(QObject *parent = nullptr);
    InputProfileEditor(QLCInputProfile *profile, Doc *doc, QObject *parent = nullptr);
    ~InputProfileEditor();

    /* Get/Set the profile modified state */
    bool modified() const;
    void setModified(bool newModified = true);

    /* Get/Set the manufacturer of the profile currently being edited */
    QString manufacturer() const;
    void setManufacturer(const QString &newManufacturer);

    /* Get/Set the model of the profile currently being edited */
    QString model() const;
    void setModel(const QString &newModel);

    /* Get/Set the type of the profile currently being edited */
    QLCInputProfile::Type type();
    void setType(const QLCInputProfile::Type &newType);

    /* Get/Set MIDI Note Off setting */
    bool midiNoteOff();
    void setMidiNoteOff(const bool &newNoteOff);

protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key);

signals:
    void modifiedChanged();
    void manufacturerChanged();
    void modelChanged();
    void typeChanged();
    void midiNoteOffChanged();

private:
    Doc *m_doc;
    QLCInputProfile *m_profile;
    bool m_modified;
    bool m_detection;

    /************************************************************************
     * Channels
     ************************************************************************/
public:
    enum MIDIMessageType
    {
        ControlChange = 0,
        NoteOnOff,
        NoteAftertouch,
        ProgramChange,
        ChannelAfterTouch,
        PitchWheel,
        MBCPlayback,
        MBCBeat,
        MBCStop
    };
    Q_ENUM(MIDIMessageType)

    enum MIDIMessageOffset
    {
        ControlChangeOffset = CHANNEL_OFFSET_CONTROL_CHANGE,
        NoteOffset = CHANNEL_OFFSET_NOTE,
        NoteAfterTouchOffset = CHANNEL_OFFSET_NOTE_AFTERTOUCH,
        ProgramChangeOffset = CHANNEL_OFFSET_PROGRAM_CHANGE,
        ChannelAfterTouchOffset = CHANNEL_OFFSET_CHANNEL_AFTERTOUCH,
        PitchWheelOffset = CHANNEL_OFFSET_PITCH_WHEEL,
        MBCPlaybackOffset = CHANNEL_OFFSET_MBC_PLAYBACK,
        MBCBeatOffset = CHANNEL_OFFSET_MBC_BEAT,
        MBCStopOffset = CHANNEL_OFFSET_MBC_STOP
    };
    Q_ENUM(MIDIMessageOffset)

    /* Enable/Disable input detection */
    Q_INVOKABLE void toggleDetection();

    /* Return a QML-ready list of channels of the profile
     * currently being edited */
    QVariant channels();

    /* Return a QML-ready list of channel types to be
     * used by a combo box component */
    QVariantList channelTypeModel();

    /* Get a copy of the channel with the provided number */
    Q_INVOKABLE QLCInputChannel *getEditChannel(int channelNumber);

    /* Check if a channel number already exists */
    Q_INVOKABLE int saveChannel(int originalChannelNumber, int channelNumber);

    /* Remove the input channel with the provided channel number */
    Q_INVOKABLE bool removeChannel(int channelNumber);

signals:
    void channelsChanged();

private:
    // map of <channel, values> used to detect if
    // an input signal comes from a button or a fader
    QMap<quint32, QVector<uchar>> m_channelsMap;

    QLCInputChannel *m_editChannel;
};

#endif
