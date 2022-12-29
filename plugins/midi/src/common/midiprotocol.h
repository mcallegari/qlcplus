/*
  Q Light Controller
  midiprotocol.h

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

#ifndef MIDIPROTOCOL_H
#define MIDIPROTOCOL_H

/****************************************************************************
 * MIDI conversion functions
 ****************************************************************************/

namespace QLCMIDIProtocol
{
    /**
     * Convert MIDI message to QLC input data
     *
     * @param cmd MIDI command byte
     * @param data1 MIDI first data byte
     * @param data2 MIDI second data byte (unused for some commands)
     * @param midiChannel MIDI channel to expect data on (255 for all)
     * @param channel The input channel that is mapped from the given MIDI data
     * @param value The value for the input channel
     * @return true if the values were parsed successfully, otherwise false
     */
    bool midiToInput(uchar cmd, uchar data1, uchar data2, uchar midiChannel,
                     quint32* channel, uchar* value);

    /**
     * Convert MIDI system common message to QLC input data
     *
     * @param cmd MIDI system common command byte
     * @param data1 MIDI first data byte (unused for some commands)
     * @param data2 MIDI second data byte (unused for some commands)
     * @param channel The input channel that is mapped from the given MIDI data
     * @param value The value for the input channel
     * @return true if the values were parsed successfully, otherwise false
     */
    bool midiSysCommonToInput(uchar cmd, uchar data1, uchar data2,
                              quint32* channel, uchar* value);

    /**
     * Convert QLC feedback data to MIDI message
     *
     * @param channel The input channel that receives feedback data
     * @param value The channel's feedback value
     * @param MIDI channel to send data on (0-15)
     * @param sendNoteOff if true, for value 0 Note Off is sent.
     *                    If false, Note On with velocity 0 is sent.
     * @param cmd MIDI command byte
     * @param data1 MIDI first data byte
     * @param data2 MIDI second data byte
     * @return true if the values were parsed successfully, otherwise false
     */
    bool feedbackToMidi(quint32 channel, uchar value,
                        uchar midiChannel, bool sendNoteOff,
                        uchar* cmd, uchar* data1,
                        uchar* data2);
}

/****************************************************************************
 * MIDI helper macros
 ****************************************************************************/
/** Extract the MIDI channel part from a MIDI message (0x*0 - 0x*F) */
#define MIDI_CH(x) (x & 0x0F)

/** Extract the MIDI command part from a MIDI message (0x8* - 0xE*) */
#define MIDI_CMD(x) (x & 0xF0)

/** Check, whether a byte contains a MIDI command */
#define MIDI_IS_CMD(x) ((x & 0x80) ? true : false)

/** Check, whether the message is a system common message (0xF*) */
#define MIDI_IS_SYSCOMMON(x) (((x & 0xF0) == 0xF0) ? true : false)

/** Convert MIDI value to DMX value and make 127 == 255 (because 2*127=254) */
#define MIDI2DMX(x) uchar((x == 127U) ? 255U : x << 1)

/** Convert DMX value to MIDI value */
#define DMX2MIDI(x) (uchar(x >> 1) & 0x7F)

/****************************************************************************
 * MIDI channels
 ****************************************************************************/
#define MAX_MIDI_DMX_CHANNELS   128
#define MAX_MIDI_CHANNELS       16

/****************************************************************************
 * MIDI commands with a MIDI channel (0-16)
 ****************************************************************************/
#define MIDI_NOTE_OFF           0x80
#define MIDI_NOTE_ON            0x90
#define MIDI_NOTE_AFTERTOUCH    0xA0
#define MIDI_CONTROL_CHANGE     0xB0
#define MIDI_PROGRAM_CHANGE     0xC0
#define MIDI_CHANNEL_AFTERTOUCH 0xD0
#define MIDI_PITCH_WHEEL        0xE0

/****************************************************************************
 * MIDI system common commands (no MIDI channel)
 ****************************************************************************/
#define MIDI_SYSEX              0xF0
#define MIDI_SYSEX_EOX          0x7F
#define MIDI_TIME_CODE          0xF1
#define MIDI_SONG_POSITION      0xF2
#define MIDI_SONG_SELECT        0xF3
#define MIDI_BEAT_CLOCK         0xF8
#define MIDI_BEAT_START         0xFA
#define MIDI_BEAT_CONTINUE      0xFB
#define MIDI_BEAT_STOP          0xFC

/****************************************************************************
 * MIDI control/msg -> QLC input channel mappings
 ****************************************************************************/
// Most MIDI sliderboards use control change messages -> put them first
#define CHANNEL_OFFSET_CONTROL_CHANGE      0
#define CHANNEL_OFFSET_CONTROL_CHANGE_MAX  127

#define CHANNEL_OFFSET_NOTE                128
#define CHANNEL_OFFSET_NOTE_MAX            255

#define CHANNEL_OFFSET_NOTE_AFTERTOUCH     256
#define CHANNEL_OFFSET_NOTE_AFTERTOUCH_MAX 383

#define CHANNEL_OFFSET_PROGRAM_CHANGE      384
#define CHANNEL_OFFSET_PROGRAM_CHANGE_MAX  511

#define CHANNEL_OFFSET_CHANNEL_AFTERTOUCH  512
#define CHANNEL_OFFSET_PITCH_WHEEL         513

#define MIDI_BEAT_CLOCK_PPQ                24
#define CHANNEL_OFFSET_MBC_PLAYBACK        529
#define CHANNEL_OFFSET_MBC_BEAT            530
#define CHANNEL_OFFSET_MBC_STOP            531

#endif
