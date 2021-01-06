/*
  Q Light Controller Plus
  midi_test.cpp

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

#include <QTest>

#define private public
#include "midi_test.h"
#include "midiprotocol.h"

#undef private

/****************************************************************************
 * MIDI tests
 ****************************************************************************/

void Midi_Test::midiToInput()
{
    quint32 channel = 0;
    uchar value = 0;

    uchar midiChannel = 7;
    uchar cmd = MIDI_NOTE_ON | midiChannel;
    uchar data1 = 10;
    uchar data2 = 127;

    QLCMIDIProtocol::midiToInput(cmd, data1, data2, midiChannel, &channel, &value);

    QCOMPARE(channel, 138U);
    QCOMPARE(value, uchar(255U));
}

void Midi_Test::feedbackToMidi()
{
    uchar cmd = 0;
    uchar data1 = 0;
    uchar data2 = 0;

    quint32 channel = 387;
    uchar value = 4;
    uchar midiChannel = 7;

    QLCMIDIProtocol::feedbackToMidi(channel, value, midiChannel, false, &cmd, &data1, &data2);

    QCOMPARE(cmd, MIDI_PROGRAM_CHANGE | midiChannel);
    QCOMPARE(data1, uchar(2U));
}

void Midi_Test::feedbackToMidi_omni()
{
    uchar cmd = 0;
    uchar data1 = 0;
    uchar data2 = 0;

    quint32 channel = 20866;
    uchar value = 255;
    uchar midiChannel = MAX_MIDI_CHANNELS;

    QLCMIDIProtocol::feedbackToMidi(channel, value, midiChannel, false, &cmd, &data1, &data2);

    QCOMPARE(cmd, MIDI_PROGRAM_CHANGE | 0x05);
    QCOMPARE(data1, uchar(127U));
}

void Midi_Test::feedbackToMidi_omni_paged()
{
    uchar cmd = 0;
    uchar data1 = 0;
    uchar data2 = 0;

    quint32 channel = 20866 | 17 << 16;
    uchar value = 255;
    uchar midiChannel = MAX_MIDI_CHANNELS;

    QLCMIDIProtocol::feedbackToMidi(channel, value, midiChannel, false, &cmd, &data1, &data2);

    QCOMPARE(cmd, MIDI_PROGRAM_CHANGE | 0x05);
    QCOMPARE(data1, uchar(127U));
}

QTEST_MAIN(Midi_Test)
