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

// Speed contains a speed value
// and helper functions to manipulate speed values
class Speed
{
    Q_OBJECT

public:
    explicit Speed(quint32 ms = Speed::originalValue())
      : value(ms), tempoType(Ms)
    {
    }

    /*********************************************************************
    * Tempo type
    *********************************************************************/
    enum TempoType
    {
        Original = -1,
        Ms = 0,
        Beats = 1,
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(TempoType)
#endif

    /**
     * The current speed value.
     * Might be in ms or in beats(*1000)
     */
    quint32 value;
    /**
     * The current tempo type.
     */
    TempoType tempoType;

    /*
     * Set the tempo type of this speed.
     * When switching from a type to another,
     * the current value will be converted to the new type.
     *
     * @param newType the tempo type
     * @param beatTime the current beat time, should be taken from MasterTimer
     */
    void switchTempoType(TempoType newType, float beatTime = qSNaN());

    /**
     * Get the string representation for the current tempo type
     */
    QString tempoTypeString() const;

    /**
     * Convert a string to a tempo type
     *
     * @param str The string to convert
     */
    static TempoType stringToTempoType(QString const& str);

    /**
     * Convert a tempo type to a string
     *
     * @param type Tempo type to convert
     */
    static QString tempoTypeToString(TempoType tempoType);

    /**
     * The "original" value for speed.
     * An speed that's supposed to override another speed will not
     * do anything if its value is equal to originalValue().
     */
    static quint32 originalValue();
    /** The "infinite" value for speed */
    static quint32 infiniteValue();
    /** The "infinite" symbol for representation of the infinite value */
    static QChar infiniteSymbol();

    /** Pretty-print the given speed in ms value into a QString */
    static QString msToString(quint32 msValue);
    /** Returns value in ms of a string created by speedToString */
    static quint32 stringToMs(QString const& str);

    /** Pretty-print the given speed in beats(*1000) value into a QString */
    static QString beatsToString(quint32 beats);
    /** Returns value in beats(*1000) of a string created by speedToString */
    static quint32 stringToBeats(QString const& str);

    /** Safe speed operations */
    static quint32 add(quint32 left, quint32 right);
    static quint32 sub(quint32 left, quint32 right);
    static quint32 normalize(quint32 value);

    /** Convert a beats(*1000) value to a time value in milliseconds */
    static quint32 beatsToMs(quint32 beats, float beatTime);
    /** Convert a time value in milliseconds to a beats(*1000) value */
    static quint32 msToBeats(quint32 ms, float beatTime);
};

#endif
