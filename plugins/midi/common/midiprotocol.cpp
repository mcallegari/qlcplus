/*
  Q Light Controller
  midiprotocol.cpp

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

    /* Check that the command came on the correct MIDI channel */
    if (midiChannel <= 0xF && MIDI_CH(cmd) != midiChannel)
        return false;

    switch(MIDI_CMD(cmd))
    {
        case MIDI_NOTE_OFF:
            *channel = CHANNEL_OFFSET_NOTE + quint32(data1);
            *value = 0;
            return true;

        case MIDI_NOTE_ON:
            *channel = CHANNEL_OFFSET_NOTE + quint32(data1);
            *value = MIDI2DMX(data2);
            return true;

        case MIDI_NOTE_AFTERTOUCH:
            *channel = CHANNEL_OFFSET_NOTE_AFTERTOUCH + quint32(data1);
            *value = MIDI2DMX(data2);
            return true;

        case MIDI_CONTROL_CHANGE:
            *channel = CHANNEL_OFFSET_CONTROL_CHANGE + quint32(data1);
            *value = MIDI2DMX(data2);
            return true;

        case MIDI_PROGRAM_CHANGE:
            *channel = quint32(data1); //CHANNEL_OFFSET_PROGRAM_CHANGE;
            *value = MIDI2DMX(data2);
            return true;

        case MIDI_CHANNEL_AFTERTOUCH:
            *channel = CHANNEL_OFFSET_CHANNEL_AFTERTOUCH;
            *value = MIDI2DMX(data1);
            return true;

        case MIDI_PITCH_WHEEL:
            *channel = CHANNEL_OFFSET_PITCH_WHEEL;
            *value = MIDI2DMX(data2);
            return true;

        default:
            return false;
    }
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
        case MIDI_BEAT_STOP:
            *channel = CHANNEL_OFFSET_MBC_PLAYBACK;
            *value = 127;
            return true;

        default:
            return false;
    }
}

bool QLCMIDIProtocol::feedbackToMidi(quint32 channel, uchar value,
                                     uchar midiChannel, uchar* cmd,
                                     uchar* data1, uchar* data2,
                                     bool* data2Valid)
{
    if (channel >= CHANNEL_OFFSET_NOTE && channel <= CHANNEL_OFFSET_NOTE_MAX)
    {
        if (value == 0)
            *cmd = MIDI_NOTE_OFF;
        else
            *cmd = MIDI_NOTE_ON;
        *cmd |= midiChannel;

        *data1 = static_cast <uchar> (channel - CHANNEL_OFFSET_NOTE);
        *data2 = DMX2MIDI(value);
        *data2Valid = true;
    }
    else if (/*channel >= CHANNEL_OFFSET_CONTROL_CHANGE &&*/
             channel <= CHANNEL_OFFSET_CONTROL_CHANGE_MAX)
    {
        *cmd = MIDI_CONTROL_CHANGE | midiChannel;
        *data1 = static_cast <uchar> (channel - CHANNEL_OFFSET_CONTROL_CHANGE);
        *data2 = DMX2MIDI(value);
        *data2Valid = true;
    }
    else if (channel >= CHANNEL_OFFSET_NOTE_AFTERTOUCH &&
             channel <= CHANNEL_OFFSET_NOTE_AFTERTOUCH_MAX)
    {
        *cmd = MIDI_NOTE_AFTERTOUCH | midiChannel;
        *data1 = static_cast <uchar> (channel - CHANNEL_OFFSET_NOTE_AFTERTOUCH);
        *data2 = DMX2MIDI(value);
        *data2Valid = true;
    }
    else if (channel == CHANNEL_OFFSET_CHANNEL_AFTERTOUCH)
    {
        *cmd = MIDI_CHANNEL_AFTERTOUCH | midiChannel;
        *data1 = DMX2MIDI(value);
        *data2Valid = false;
    }
    else if (channel == CHANNEL_OFFSET_PROGRAM_CHANGE)
    {
        *cmd = MIDI_PROGRAM_CHANGE | midiChannel;
        *data1 = DMX2MIDI(value);
        *data2Valid = false;
    }
    else if (channel == CHANNEL_OFFSET_PITCH_WHEEL)
    {
        *cmd = MIDI_PITCH_WHEEL | midiChannel;
        *data1 = DMX2MIDI(value);
        *data2Valid = false;
    }
    //else if (channel == MIDI_BEATC_CLOCK)
    //{
    //    Don't send feedback to MIDI clock
    //}
    else
    {
        return false;
    }

    return true;
}
