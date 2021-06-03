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

#ifndef MIDI_TEST_H
#define MIDI_TEST_H

#include <QObject>

class Midi_Test : public QObject
{
  Q_OBJECT

public:

  static inline
    uchar midi7b_to_dmx( int in )
    { return in >= 127 ? 255 : in * 2; }
  static inline
    uchar midi14b_to_dmx( int in )
    { return in >> 6; }

  static inline
    uchar dmx_to_midi7b( int in )
    { return in / 2; }


  static const quint32 OmniChannelOffset = 1 << 12;

  inline static
    quint32 midiChannel_to_qlcChannel(uchar midi, uchar qlc, quint32 offset)
    { return offset + (qlc == 16 ? midi * OmniChannelOffset : 0); }

  inline static
    int midiChannel_to_qlcValue(uchar midi, uchar qlc, uchar value)
    { return (qlc == 16 || qlc == midi) ? value : -1; }

  inline static
    uchar qlc_to_midiCmd(quint32 qlcChannel, uchar midiChannel, uchar cmd)
    { return cmd + (midiChannel == 16 ? qlcChannel : midiChannel); }

private slots:

  // Tests of data bytes conversion
  void midiToInput_data();
  void midiToInput();

  void feedbackToMidi_data();
  void feedbackToMidi();

  // Tests of MIDI Channel filtering
  void midiToInput_midiChannels_data();
  void midiToInput_midiChannels();

  // TODO Tests of System Common Messages

};

#endif
