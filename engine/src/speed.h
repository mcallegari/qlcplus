/*
  Q Light Controller Plus
  speed.h

  Copyright (C) 2016 Massimo Callegari
                     David Garyga

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

#ifndef SPEED_H
#define SPEED_H

#include <QObject>
#include <QtGlobal>

struct Speed
{
    enum TempoType
    {
        Original = -1,
        Ms = 0,
        Beats = 1,
    };
    Q_ENUMS(TempoType);

    quint32 value;
    TempoType tempoType;

    explicit Speed(quint32 ms = Speed::originalValue())
        : value(ms)
        , tempoType(Ms)
    {}

    void switchTempoType(TempoType newType, float bpm = qSNaN());

    QString tempoTypeString() const;
    static TempoType stringToTempoType(QString const& str);
    static QString tempoTypeToString(TempoType tempoType);

    static quint32 originalValue();
    static quint32 infiniteValue();
    static QChar infiniteSymbol();

    static quint32 stringToMs(QString const& str);
    static QString msToString(quint32 msValue);

    static quint32 add(quint32 left, quint32 right);
    static quint32 sub(quint32 left, quint32 right);
    static quint32 normalize(quint32 value);

    static quint32 stringToBeats(QString const& str);
    static QString beatsToString(quint32 beats);

    // beats is *1000, beatTime in s
    static quint32 beatsToMs(quint32 beats, float beatTime);
    // beatTime in s, return in beats *1000
    static quint32 msToBeats(quint32 ms, float beatTime);
};

#endif
