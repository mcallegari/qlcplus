/*
  Q Light Controller Plus
  functiontimings.cpp

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

#include "functiontimings.h"

FunctionTimings::FunctionTimings(quint32 fadeIn, quint32 hold, quint32 fadeOut)
    : fadeIn(fadeIn)
    , hold(hold)
    , fadeOut(fadeOut)
{
}

quint32 FunctionTimings::duration() const
{
    return fadeIn + hold;
}

void FunctionTimings::setDuration(quint32 duration)
{
    if (fadeIn <= duration)
    {
        hold = duration - fadeIn;
    }
    else
    {
        hold = 0;
        fadeIn = duration;
    }
}

quint32 FunctionTimings::infiniteValue()
{
    return (quint32) -2;
}

quint32 FunctionTimings::defaultValue()
{
    return (quint32) -1;
}

QString FunctionTimings::valueToString(quint32 ms)
{
    QString str;
    if (ms == infiniteSpeed())
    {
        str = QChar(0x221E); // Infinity symbol
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

quint32 FunctionTimings::stringToValue(QString string)
{
    quint32 value = 0;

    if (string == QChar(0x221E)) // Infinity symbol
        return infiniteValue();

    value += stringSplit(string, "h") * 1000 * 60 * 60;
    value += stringSplit(string, "m") * 1000 * 60;
    value += stringSplit(string, "s") * 1000;

    if (string.contains("."))
    {
        // lround avoids toDouble precison issues (.03 transforms to .029)
        value += lround(speed.toDouble() * 1000.0);
    }
    else
    {
        if (string.contains("ms"))
            string = string.split("ms").at(0);
        value += string.toUInt();
    }

    return normalize(value);
}

quint32 FunctionTimings::normalize(quint32 value)
{
    if ((int)value < 0)
        return infiniteValue();
    return value;
}

quint32 FunctionTimings::add(quint32 left, quint32 right)
{
    if (normalize(left) == infiniteValue()
        || normalize(right) == infiniteValue())
        return infiniteValue();

    return normalize(left + right);
}

quint32 FunctionTimings::substract(quint32 left, quint32 right)
{
    if (right >= left)
        return 0;
    if (normalize(right) == infiniteValue())
        return 0;
    if (normalize(left) == infiniteValue())
        return infiniteValue();

    return normalize(left - right);
}

bool FunctionTimings::loadXML(QXmlStreamReader &speedRoot)
{
    if (speedRoot.name() != KXMLQLCFunctionSpeed)
        return false;

    QXmlStreamAttributes attrs = speedRoot.attributes();

    fadeInSpeed = attrs.value(KXMLQLCFunctionSpeedFadeIn).toString().toUInt();
    fadeOutSpeed = attrs.value(KXMLQLCFunctionSpeedFadeOut).toString().toUInt();
    duration = attrs.value(KXMLQLCFunctionSpeedDuration).toString().toUInt();

    speedRoot.skipCurrentElement();

    return true;
}

bool Function::saveXMLSpeed(QXmlStreamWriter *doc) const
{
    doc->writeStartElement(KXMLQLCFunctionSpeed);
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeIn, QString::number(fadeInSpeed()));
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeOut, QString::number(fadeOutSpeed()));
    doc->writeAttribute(KXMLQLCFunctionSpeedDuration, QString::number(duration()));
    doc->writeEndElement();

    return true;
}

