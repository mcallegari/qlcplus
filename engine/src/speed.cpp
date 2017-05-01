/*
  Q Light Controller Plus
  speed.cpp

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

#include "speed.h"

#include <QDebug>
#include <cmath>

#include "qlcmacros.h"

const QString KTimeTypeString  ( "Time" );
const QString KBeatsTypeString ( "Beats" );

void Speed::switchTempoType(TempoType newType, float beatTime)
{
    if (newType == tempoType)
        return;

    tempoType = newType;

    if (std::isnan(beatTime))
        return;

    switch (tempoType)
    {
        /* Beats -> Ms */
        case Ms:
            value = beatsToMs(value, beatTime);
        break;

        /* Time -> Beats */
        case Beats:
            value = msToBeats(value, beatTime);
        break;
        default:
            qDebug() << "Error. Unhandled tempo type" << tempoType;
        break;
    }
}

QString Speed::tempoTypeString() const
{
    return tempoTypeToString(tempoType);
}

Speed::TempoType Speed::stringToTempoType(QString const& str)
{
    if (str == KTimeTypeString)
        return Ms;
    else
        return Beats;
}

QString Speed::tempoTypeToString(TempoType tempoType)
{
    switch (tempoType)
    {
        default:
        case Ms:
            return KTimeTypeString;
        case Beats:
            return KBeatsTypeString;
    }
}

quint32 Speed::originalValue()
{
    return (quint32)-1;
}

quint32 Speed::infiniteValue()
{
    return (quint32)-2;
}

QChar Speed::infiniteSymbol()
{
    return QChar(0x221E);
}

namespace
{
quint32 stringSplit(QString& string, QString splitNeedle)
{
    QStringList splitResult;
    // Filter out "ms" because "m" and "s" may wrongly use it
    splitResult = string.split("ms");
    if (splitResult.count() > 1)
        splitResult = splitResult.at(0).split(splitNeedle);
    else
        splitResult = string.split(splitNeedle);

    if (splitResult.count() > 1)
    {
        string.remove(0, string.indexOf(splitNeedle) + 1);
        return splitResult.at(0).toUInt();
    }
    return 0;
}
}

quint32 Speed::stringToMs(QString const& str)
{
    quint32 ms = 0;

    if (str == infiniteSymbol())
        return infiniteValue();

    QString string(str);

    ms += stringSplit(string, "h") * 1000 * 60 * 60;
    ms += stringSplit(string, "m") * 1000 * 60;
    ms += stringSplit(string, "s") * 1000;

    if (string.contains("."))
    {
        // lround avoids toDouble precison issues (.03 transforms to .029)
        ms += lround(string.toDouble() * 1000.0);
    }
    else
    {
        if (string.contains("ms"))
            string = string.split("ms").at(0);
        ms += string.toUInt();
    }

    return ms;
}

QString Speed::msToString(quint32 ms)
{
    QString str;
    if (ms == infiniteValue())
    {
        str = infiniteSymbol();
    }
    else
    {
        quint32 h, m, s;

        h = ms / MS_PER_HOUR;
        ms -= (h * MS_PER_HOUR);

        m = ms / MS_PER_MINUTE;
        ms -= (m * MS_PER_MINUTE);

        s = ms / MS_PER_SECOND;
        ms -= (s * MS_PER_SECOND);

        if (h != 0)
            str += QString("%1h").arg(h, 1, 10, QChar('0'));
        if (m != 0)
            str += QString("%1m").arg(m, str.size() ? 2 : 1, 10, QChar('0'));
        if (s != 0)
            str += QString("%1s").arg(s, str.size() ? 2 : 1, 10, QChar('0'));
        if (ms != 0 || str.size() == 0)
            str += QString("%1ms").arg(ms, str.size() ? 3 : 1, 10, QChar('0'));
    }

    return str;
}

quint32 Speed::add(quint32 left, quint32 right)
{
    if (left == originalValue() && right == originalValue())
        return originalValue();
    left = normalize(left);
    right = normalize(right);
    if (left == infiniteValue() || right == infiniteValue())
        return infiniteValue();
    return normalize(left + right);
}

quint32 Speed::sub(quint32 left, quint32 right)
{
    left = normalize(left);
    right = normalize(right);
    if (right == infiniteValue())
        return 0;
    if (left == infiniteValue())
        return infiniteValue();
    if (right >= left)
        return 0;
    return normalize(left - right);
}

quint32 Speed::normalize(quint32 value)
{
    if ((int)value < 0)
        return infiniteValue();
    return value;
}

quint32 Speed::stringToBeats(QString const& str)
{
    if (str == infiniteSymbol())
        return infiniteValue();
    return str.toDouble() * 1000;
}

QString Speed::beatsToString(quint32 beats)
{
    if (beats == infiniteValue())
        return infiniteSymbol();
    double val = double(beats) / 1000.0;
    return QString("%1 beats").arg(val);
}

quint32 Speed::beatsToMs(quint32 beats, float beatTime)
{
    if (beats == originalValue() || beats == infiniteValue())
        return beats;
    return normalize(((float)beats * beatTime) / 1000);
}

quint32 Speed::msToBeats(quint32 ms, float beatTime)
{
    if (ms == originalValue() || ms == infiniteValue())
        return ms;
    return normalize(((float)ms / beatTime) * 1000);
}
