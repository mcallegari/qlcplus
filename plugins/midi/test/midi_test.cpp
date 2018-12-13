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

#include "midi_test.h"
#include "midiprotocol.h"

/****************************************************************************
 * MIDI tests
 ****************************************************************************/

void Midi_Test::midiToInput_data()
{
  QTest::addColumn<uchar>("cmd");
  QTest::addColumn<uchar>("data1");
  QTest::addColumn<uchar>("data2");
  QTest::addColumn<quint32>("channel_expected");
  QTest::addColumn<uchar>("value_expected");

  uchar cmd;
  quint32 channel;

  // Note OFF 0x80
  // Channel = data1
  // Value = 0, whatever is the velocity (data2)
  cmd = MIDI_NOTE_OFF;
  channel = CHANNEL_OFFSET_NOTE;
  for( uchar i = 0; i < 128; ++i )
    QTest::addRow("Note OFF key=%d vel=%d", i, i) << cmd << i << i << channel + i << (uchar) 0;

  // Note ON 0x90
  // Channel = data1
  // Value = data2 * 2 (and 255 for data2=127)
  cmd = MIDI_NOTE_ON;
  channel = CHANNEL_OFFSET_NOTE;
  for( uchar i = 0; i < 128; ++i ) {
    uchar tmp2 = 127 - i;
    QTest::addRow("Note ON key=%d vel=%d", i, tmp2) << cmd << i << tmp2 << channel + i << midi7b_to_dmx(tmp2);
  }

  // Note Aftertouch 0xA0
  // Channel = data1
  // Value = data2 * 2 (and 255 for data2=127)
  cmd = MIDI_NOTE_AFTERTOUCH;
  channel = CHANNEL_OFFSET_NOTE_AFTERTOUCH;
  for( uchar i = 0; i < 128; ++i )
    QTest::addRow("Note Aftertouch key=%d vel=%d", i, i) << cmd << i << i << channel + i << midi7b_to_dmx(i);

  // Control Change 0xB0
  // Channel = data1
  // Value = data2 * 2 (and 255 for data2=127)
  cmd = MIDI_CONTROL_CHANGE;
  channel = CHANNEL_OFFSET_CONTROL_CHANGE;
  for( uchar i = 0; i < 128; ++i ) {
    uchar tmp2 = 127 - i;
    QTest::addRow("CC ctrl=%d val=%d", i, tmp2) << cmd << i << tmp2 << channel + i << midi7b_to_dmx(tmp2);
  }

  // Program Change 0xC0
  // value = data1 * 2 (and 255 for data2=127)
  cmd = MIDI_PROGRAM_CHANGE;
  channel = CHANNEL_OFFSET_PROGRAM_CHANGE;
  for( uchar i = 0; i < 128; ++i ) {
    uchar tmp2 = 127 - i;
    QTest::addRow("Program Change prg=%d", i) << cmd << i << tmp2 << channel << midi7b_to_dmx(i);
  }

  // Channel Aftertouch 0xD0
  // value = data1 * 2 (and 255 for data2=127)
  cmd = MIDI_CHANNEL_AFTERTOUCH;
  channel = CHANNEL_OFFSET_CHANNEL_AFTERTOUCH;
  for( uchar i = 0; i < 128; ++i ) {
    uchar tmp2 = 127 - i;
    QTest::addRow("Channel Aftertouch val=%d", i) << cmd << i << tmp2 << channel << midi7b_to_dmx(i);
  }

  // Pitch Bend 0xE0
  // value = data1 for LSB & data2 for MSB
  cmd = MIDI_PITCH_WHEEL;
  channel = CHANNEL_OFFSET_PITCH_WHEEL;
  for( int i = 0; i < 0x4000; i+=100 ) {
    uchar data1 = i & 0x7F;
    uchar data2 = i >> 7;
    QTest::addRow("Pitch Wheel val=%d", i) << cmd << data1 << data2 << channel << midi14b_to_dmx(i);
  }

}

void Midi_Test::midiToInput()
{
  quint32 channel_result;
  uchar value_result;

  QFETCH(uchar, cmd);
  QFETCH(uchar, data1);
  QFETCH(uchar, data2);
  QFETCH(quint32, channel_expected);
  QFETCH(uchar, value_expected);

  QVERIFY2(
    QLCMIDIProtocol::midiToInput(
      cmd, data1, data2, 0 /* midiChannel */, &channel_result, &value_result),
    "Expecting valid answer from midiToInput");

  QCOMPARE(channel_result, channel_expected);
  QCOMPARE(value_result, value_expected);
}



void Midi_Test::midiToInput_midiChannels_data()
{
  QTest::addColumn<uchar>("cmd");
  QTest::addColumn<uchar>("data1");
  QTest::addColumn<uchar>("data2");
  QTest::addColumn<uchar>("midiChannel");
  QTest::addColumn<quint32>("qlc_channel_expected");
  QTest::addColumn<int>("qlc_value_expected");

  for(uchar midiChannel = 0; midiChannel < 16; ++midiChannel)
  {
    uchar data1 = midiChannel * 7;
    uchar data2 = midiChannel * 3;

    for(uchar qlcMidiChannel = 2; qlcMidiChannel <= 16; qlcMidiChannel+=7)
    {
      QTest::addRow("Note OFF midi=%d qlc=%d", midiChannel, qlcMidiChannel)
        << uchar(MIDI_NOTE_OFF + midiChannel) << data1 << data2 << qlcMidiChannel
        << midiChannel_to_qlcChannel( midiChannel, qlcMidiChannel, CHANNEL_OFFSET_NOTE + data1++ )
        << midiChannel_to_qlcValue( midiChannel, qlcMidiChannel, 0 );

      QTest::addRow("Note ON midi=%d qlc=%d", midiChannel, qlcMidiChannel)
        << uchar(MIDI_NOTE_ON + midiChannel) << data1 << data2 << qlcMidiChannel
        << midiChannel_to_qlcChannel( midiChannel, qlcMidiChannel, CHANNEL_OFFSET_NOTE + data1++ )
        << midiChannel_to_qlcValue( midiChannel, qlcMidiChannel, midi7b_to_dmx(data2++) );

      QTest::addRow("Note Aftertouch midi=%d qlc=%d", midiChannel, qlcMidiChannel)
        << uchar(MIDI_NOTE_AFTERTOUCH + midiChannel) << data1 << data2 << qlcMidiChannel
        << midiChannel_to_qlcChannel( midiChannel, qlcMidiChannel, CHANNEL_OFFSET_NOTE_AFTERTOUCH + data1++ )
        << midiChannel_to_qlcValue( midiChannel, qlcMidiChannel, midi7b_to_dmx(data2++) );

      QTest::addRow("CC midi=%d qlc=%d", midiChannel, qlcMidiChannel)
        << uchar(MIDI_CONTROL_CHANGE + midiChannel) << data1 << data2 << qlcMidiChannel
        << midiChannel_to_qlcChannel( midiChannel, qlcMidiChannel, CHANNEL_OFFSET_CONTROL_CHANGE + data1++ )
        << midiChannel_to_qlcValue( midiChannel, qlcMidiChannel, midi7b_to_dmx(data2++) );

      QTest::addRow("Program Change midi=%d qlc=%d", midiChannel, qlcMidiChannel)
        << uchar(MIDI_PROGRAM_CHANGE + midiChannel) << data1 << data2++ << qlcMidiChannel
        << midiChannel_to_qlcChannel( midiChannel, qlcMidiChannel, CHANNEL_OFFSET_PROGRAM_CHANGE )
        << midiChannel_to_qlcValue( midiChannel, qlcMidiChannel, midi7b_to_dmx(data1++) );

      QTest::addRow("Channel Aftertouch midi=%d qlc=%d", midiChannel, qlcMidiChannel)
        << uchar(MIDI_CHANNEL_AFTERTOUCH + midiChannel) << data1 << data2++ << qlcMidiChannel
        << midiChannel_to_qlcChannel( midiChannel, qlcMidiChannel, CHANNEL_OFFSET_CHANNEL_AFTERTOUCH )
        << midiChannel_to_qlcValue( midiChannel, qlcMidiChannel, midi7b_to_dmx(data1++) );

      int val = data2 << 7 | data1;
      QTest::addRow("Pitch Wheel midi=%d qlc=%d", midiChannel, qlcMidiChannel)
        << uchar(MIDI_PITCH_WHEEL + midiChannel) << data1 << data2 << qlcMidiChannel
        << midiChannel_to_qlcChannel( midiChannel, qlcMidiChannel, CHANNEL_OFFSET_PITCH_WHEEL )
        << midiChannel_to_qlcValue( midiChannel, qlcMidiChannel, midi14b_to_dmx(val) );
    }
  }
}

void Midi_Test::midiToInput_midiChannels()
{
  quint32 channel_result;
  uchar value_result;

  QFETCH(uchar, cmd);
  QFETCH(uchar, data1);
  QFETCH(uchar, data2);
  QFETCH(uchar, midiChannel);
  QFETCH(quint32, qlc_channel_expected);
  QFETCH(int, qlc_value_expected);

  bool result = QLCMIDIProtocol::midiToInput(
      cmd, data1, data2, midiChannel,
      &channel_result, &value_result);

  if(qlc_value_expected >= 0)
  {
    QVERIFY2(result, "Expecting valid answer from midiToInput");
    QCOMPARE(channel_result, qlc_channel_expected);
    QCOMPARE(value_result, qlc_value_expected);
  }
  else
  {
    QVERIFY2(!result, "Expecting midiToInput to ignore the call");
  }
}

QTEST_MAIN(Midi_Test)
