/*
  Q Light Controller
  inputchanneleditor.cpp

  Copyright (C) Heikki Junnila

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

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QIcon>

#include "qlcchannel.h"
#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "inputchanneleditor.h"

#define KMidiMessageCC 0
#define KMidiMessageNoteOnOff 1
#define KMidiMessageNoteAftertouch 2
#define KMidiMessagePC 3
#define KMidiMessageChannelAftertouch 4
#define KMidiMessagePitchWheel 5
#define KMidiMessageMBCPlayback 6
#define KMidiMessageMBCBeat 7

#define KMidiChannelOffset 4096

#include "../../plugins/midi/common/midiprotocol.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputChannelEditor::InputChannelEditor(QWidget* parent,
                                       const QLCInputProfile* profile,
                                       const QLCInputChannel* channel,
                                       QLCInputProfile::Type profileType)
        : QDialog(parent)
{
    m_channel = 0;
    m_type = QLCInputChannel::NoType;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /* Connect to these already now so that the handlers get called
       during initialization. */
    connect(m_numberSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotNumberChanged(int)));
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_typeCombo, SIGNAL(activated(const QString&)),
            this, SLOT(slotTypeActivated(const QString &)));

    /* Fill type combo with type icons and names */
    QStringListIterator it(QLCInputChannel::types());
    while (it.hasNext() == true)
    {
        QString str(it.next());
        m_typeCombo->addItem(QLCInputChannel::stringToIcon(str), str);
    }

    if (channel != NULL && profile != NULL)
    {
        QString type;
        quint32 num;

        /* Channel number */
        num = profile->channelNumber(channel);
        if (num != QLCChannel::invalid())
            m_numberSpin->setValue(num + 1);
        else
            m_numberSpin->setValue(1);

        /* Channel name */
        m_nameEdit->setText(channel->name());

        /* Channel type */
        m_type = channel->type();
        type = QLCInputChannel::typeToString(channel->type());
        m_typeCombo->setCurrentIndex(m_typeCombo->findText(type));

        if (profileType == QLCInputProfile::Midi)
        {
            slotNumberChanged(m_numberSpin->value());

            connect(m_midiChannelSpin, SIGNAL(valueChanged(int)),
                this, SLOT(slotMidiChanged()));
            connect(m_midiMessageCombo, SIGNAL(activated(int)),
                this, SLOT(slotMidiChanged()));
            connect(m_midiParamSpin, SIGNAL(valueChanged(int)),
                this, SLOT(slotMidiChanged()));
        }
        else
        {
            m_midiGroup->hide();
            adjustSize();
        }
    }
    else
    {
        /* Multiple channels are being edited. Disable the channel
           number spin. */
        m_numberSpin->setEnabled(false);
        m_midiGroup->hide();
        adjustSize();
    }
}

InputChannelEditor::~InputChannelEditor()
{
}

/****************************************************************************
 * Properties
 ****************************************************************************/

quint32 InputChannelEditor::channel() const
{
    return m_channel;
}

QString InputChannelEditor::name() const
{
    return m_name;
}

QLCInputChannel::Type InputChannelEditor::type() const
{
    return m_type;
}

void InputChannelEditor::slotNumberChanged(int number)
{
    m_channel = number - 1;

    int midiChannel = 0;
    int midiMessage = 0;
    int midiParam = 0;

    numberToMidi(m_channel, midiChannel, midiMessage, midiParam);

    m_midiChannelSpin->setValue(midiChannel);
    m_midiMessageCombo->setCurrentIndex(midiMessage);
    if (midiParam >= 0)
        m_midiParamSpin->setValue(midiParam);

    enableMidiParam(midiMessage, midiParam);
}

void InputChannelEditor::slotNameEdited(const QString& text)
{
    m_name = text;
}

void InputChannelEditor::slotTypeActivated(const QString& text)
{
    m_type = QLCInputChannel::stringToType(text);
}

/****************************************************************************
 * MIDI
 ****************************************************************************/

void InputChannelEditor::numberToMidi(int number, int & channel, int & message, int & param)
{
    channel = number / KMidiChannelOffset + 1;
    number = number % KMidiChannelOffset;
    param = -1;
    if (number <= CHANNEL_OFFSET_CONTROL_CHANGE_MAX)
    {
        message = KMidiMessageCC;
        param = number - CHANNEL_OFFSET_CONTROL_CHANGE;
    } 
    else if (number <= CHANNEL_OFFSET_NOTE_MAX)
    {
        message = KMidiMessageNoteOnOff;
        param = number - CHANNEL_OFFSET_NOTE;
    } 
    else if (number <= CHANNEL_OFFSET_NOTE_AFTERTOUCH_MAX)
    {
        message = KMidiMessageNoteAftertouch;
        param = number - CHANNEL_OFFSET_NOTE_AFTERTOUCH;
    } 
    else if (number <= CHANNEL_OFFSET_PROGRAM_CHANGE_MAX)
    {
        message = KMidiMessagePC;
        param = number - CHANNEL_OFFSET_PROGRAM_CHANGE;
    } 
    else if (number == CHANNEL_OFFSET_CHANNEL_AFTERTOUCH)
    {
        message = KMidiMessageChannelAftertouch;
    } 
    else if (number == CHANNEL_OFFSET_MBC_PLAYBACK)
    {
        message = KMidiMessageMBCPlayback;
    } 
    else // if (number == CHANNEL_OFFSET_MBC_BEAT)
    {
        message = KMidiMessageMBCBeat;
    } 
}

int InputChannelEditor::midiToNumber(int channel, int message, int param)
{
    switch (message)
    {
    case KMidiMessageCC:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_CONTROL_CHANGE + (param);
    case KMidiMessageNoteOnOff:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_NOTE + (param);
    case KMidiMessageNoteAftertouch:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_NOTE_AFTERTOUCH + (param);
    case KMidiMessagePC:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_PROGRAM_CHANGE + (param);
    case KMidiMessageChannelAftertouch:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_CHANNEL_AFTERTOUCH;
    case KMidiMessagePitchWheel:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_PITCH_WHEEL;
    case KMidiMessageMBCPlayback:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_MBC_PLAYBACK;
    case KMidiMessageMBCBeat:
        return (channel - 1) * KMidiChannelOffset + CHANNEL_OFFSET_MBC_BEAT;
    default:
        return 0;
    }
}

void InputChannelEditor::slotMidiChanged()
{
    int midiChannel = m_midiChannelSpin->value();
    int midiMessage = m_midiMessageCombo->currentIndex();
    int midiParam = m_midiParamSpin->value();

    enableMidiParam(midiMessage, midiParam);

    m_channel = midiToNumber(midiChannel, midiMessage, midiParam);
    m_numberSpin->setValue(m_channel + 1);
}

void InputChannelEditor::enableMidiParam(int midiMessage, int midiParam)
{
    switch (midiMessage)
    {
    case KMidiMessageNoteOnOff:
    case KMidiMessageNoteAftertouch:
        m_midiParamLabel->setEnabled(true);
        m_midiParamSpin->setEnabled(true);

        m_midiNoteLabel->setEnabled(true);
        m_midiNote->setEnabled(true);
        m_midiNote->setText(noteToString(midiParam));
        break;

    case KMidiMessageCC:
    case KMidiMessagePC:
        m_midiParamLabel->setEnabled(true);
        m_midiParamSpin->setEnabled(true);

        m_midiNoteLabel->setEnabled(false);
        m_midiNote->setEnabled(false);
        m_midiNote->setText("--");
        break;

    case KMidiMessageChannelAftertouch:
    case KMidiMessagePitchWheel:
    case KMidiMessageMBCPlayback:
    case KMidiMessageMBCBeat:
        m_midiParamLabel->setEnabled(false);
        m_midiParamSpin->setEnabled(false);

        m_midiNoteLabel->setEnabled(false);
        m_midiNote->setEnabled(false);
        m_midiNote->setText("--");
        break;
    }
}

QString InputChannelEditor::noteToString(int note)
{
    int octave = note / 12 - 1;
    int pitch = note % 12;

    switch(pitch)
    {
    case 0:
        return QString("C%1").arg(octave);
    case 1:
        return QString("C#%1").arg(octave);
    case 2:
        return QString("D%1").arg(octave);
    case 3:
        return QString("D#%1").arg(octave);
    case 4:
        return QString("E%1").arg(octave);
    case 5:
        return QString("F%1").arg(octave);
    case 6:
        return QString("F#%1").arg(octave);
    case 7:
        return QString("G%1").arg(octave);
    case 8:
        return QString("G#%1").arg(octave);
    case 9:
        return QString("A%1").arg(octave);
    case 10:
        return QString("A#%1").arg(octave);
    case 11:
        return QString("B%1").arg(octave);
    default:
        return "--";
    }
}

