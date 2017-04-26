/*
  Q Light Controller
  midiprotocol.cpp

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

#include <QtCore>

#include "midiprotocol.h"
#include "qlcmacros.h"

/****************************************************************************
 * MIDI conversion functions
 ****************************************************************************/

bool QLCMIDIProtocol::midiToInput(uchar cmd, uchar data1, uchar data2,
                                  uchar midiChannel, quint32* channel,
                                  uchar* value)
{
    /* Check if cmd is a MIDI COMMAND byte */
    if (!MIDI_IS_CMD(cmd))
        return false;

    /** Use a special handler function for system common messages */
    if (MIDI_IS_SYSCOMMON(cmd))
        return midiSysCommonToInput(cmd, data1, data2, channel, value);

    uchar midi_ch = MIDI_CH(cmd);

    /* Check that the command came on the correct MIDI channel */
    if (midiChannel <= 0xF && midi_ch != midiChannel)
        return false;

    switch(MIDI_CMD(cmd))
    {
        case MIDI_NOTE_OFF:
            *channel = CHANNEL_OFFSET_NOTE + quint32(data1);
            *value = 0;
        break;

        case MIDI_NOTE_ON:
            *channel = CHANNEL_OFFSET_NOTE + quint32(data1);
            *value = MIDI2DMX(data2);
        break;

        case MIDI_NOTE_AFTERTOUCH:
            *channel = CHANNEL_OFFSET_NOTE_AFTERTOUCH + quint32(data1);
            *value = MIDI2DMX(data2);
        break;

        case MIDI_CONTROL_CHANGE:
            *channel = CHANNEL_OFFSET_CONTROL_CHANGE + quint32(data1);
            *value = MIDI2DMX(data2);
        break;

        case MIDI_PROGRAM_CHANGE:
            *channel = CHANNEL_OFFSET_PROGRAM_CHANGE + quint32(data1);
            *value = MIDI2DMX(data2);
        break;

        case MIDI_CHANNEL_AFTERTOUCH:
            *channel = CHANNEL_OFFSET_CHANNEL_AFTERTOUCH;
            *value = MIDI2DMX(data1);
        break;

        case MIDI_PITCH_WHEEL:
            *channel = CHANNEL_OFFSET_PITCH_WHEEL;
            *value = (data2 << 1) | ((data1 >> 6) & 0x01); // get 8-bit value from MSB/LSB
        break;

        default:
            return false;
        break;
    }

    // in OMNI mode, bitmask the MIDI channel in the 4 MSB bits
    if (midiChannel == MAX_MIDI_CHANNELS)
        *channel = ((quint32)midi_ch << 12) | *channel;

    return true;
}

bool QLCMIDIProtocol::midiSysCommonToInput(uchar cmd, uchar data1, uchar data2,
                                           quint32* channel, uchar* value)
{
    Q_UNUSED(data1);
    Q_UNUSED(data2);

    switch (cmd)
    {
        case MIDI_BEAT_CLOCK:
            *channel = CHANNEL_OFFSET_MBC_BEAT;
            *value = 127;
            return true;

        case MIDI_BEAT_START:
        case MIDI_BEAT_CONTINUE:
            *channel = CHANNEL_OFFSET_MBC_PLAYBACK;
            *value = 127;
            return true;

        case MIDI_BEAT_STOP:
            *channel = CHANNEL_OFFSET_MBC_STOP;
            *value = 127;
            return true;

        default:
            return false;
    }
}

bool QLCMIDIProtocol::feedbackToMidi(quint32 channel, uchar value,
                                     uchar midiChannel, bool sendNoteOff, uchar* cmd,
                                     uchar* data1, uchar* data2)
{
    // for OMNI mode, retrieve the original MIDI channel where data was sent
    if (midiChannel == MAX_MIDI_CHANNELS)
        midiChannel = channel >> 12;

    // Remove the 4 MSB bits to retrieve the QLC+ channel to be processed
    channel = channel & 0x0FFF;

    if (channel <= CHANNEL_OFFSET_CONTROL_CHANGE_MAX)
    {
        *cmd = MIDI_CONTROL_CHANGE | midiChannel;
        *data1 = static_cast <uchar> (channel - CHANNEL_OFFSET_CONTROL_CHANGE);
        *data2 = DMX2MIDI(value);
    }
    else if (channel >= CHANNEL_OFFSET_NOTE && channel <= CHANNEL_OFFSET_NOTE_MAX)
    {
        if (value == 0 && sendNoteOff)
            *cmd = MIDI_NOTE_OFF;
        else
            *cmd = MIDI_NOTE_ON;
        *cmd |= midiChannel;

        *data1 = static_cast <uchar> (channel - CHANNEL_OFFSET_NOTE);
        *data2 = DMX2MIDI(value);
    }
    else if (channel >= CHANNEL_OFFSET_NOTE_AFTERTOUCH &&
             channel <= CHANNEL_OFFSET_NOTE_AFTERTOUCH_MAX)
    {
        *cmd = MIDI_NOTE_AFTERTOUCH | midiChannel;
        *data1 = static_cast <uchar> (channel - CHANNEL_OFFSET_NOTE_AFTERTOUCH);
        *data2 = DMX2MIDI(value);
    }
    else if (channel >= CHANNEL_OFFSET_PROGRAM_CHANGE &&
             channel <= CHANNEL_OFFSET_PROGRAM_CHANGE_MAX)
    {
        *cmd = MIDI_PROGRAM_CHANGE | midiChannel;
        *data1 = DMX2MIDI(value);
    }
    else if (channel == CHANNEL_OFFSET_CHANNEL_AFTERTOUCH)
    {
        *cmd = MIDI_CHANNEL_AFTERTOUCH | midiChannel;
        *data1 = DMX2MIDI(value);
    }
    else if (channel == CHANNEL_OFFSET_PITCH_WHEEL)
    {
        *cmd = MIDI_PITCH_WHEEL | midiChannel;
        *data1 = ((value & 0x01) << 6);             // LSB (low bit of value)
        *data2 = DMX2MIDI(value);                   // MSB (high 7 bits of value)
    }
    //else if (channel == MIDI_BEAT_CLOCK)
    //{
    //    Don't send feedback to MIDI clock
    //}
    else
    {
        return false;
    }

    return true;
}
