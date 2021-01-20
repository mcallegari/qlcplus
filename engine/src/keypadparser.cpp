/*
  Q Light Controller Plus
  keypadparser.cpp

  Copyright (c) Massimo Callegari

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

#include "keypadparser.h"
#include "qlcmacros.h"

KeyPadParser::KeyPadParser()
{

}

QList<SceneValue> KeyPadParser::parseCommand(Doc *doc, QString command, QByteArray &uniData)
{
    QList<SceneValue> values;
    if (doc == NULL)
        return values;

    QStringList tokens = command.split(" ");

    int lastCommand = CommandNone;
    quint32 fromChannel = 0;
    quint32 toChannel = 0;
    quint32 byChannel = 1;
    float fromValue = 0;
    float toValue = 0;
    int thruCount = 0;

    foreach (QString token, tokens)
    {
        if (token.isEmpty())
            continue;

        if (token == "AT")
        {
            lastCommand = CommandAT;
        }
        else if (token == "THRU")
        {
            lastCommand = CommandTHRU;
        }
        else if (token == "FULL")
        {
            lastCommand = CommandFULL;
        }
        else if (token == "ZERO")
        {
            lastCommand = CommandZERO;
        }
        else if (token == "BY")
        {
            lastCommand = CommandBY;
        }
        else if (token == "+")
        {
            lastCommand = CommandPlus;
        }
        else if (token == "-")
        {
            lastCommand = CommandMinus;
        }
        else if (token == "+%")
        {
            lastCommand = CommandPlusPercent;
        }

        else if (token == "-%")
        {
            lastCommand = CommandMinusPercent;
        }
        else if (token == "%")
        {
            if (lastCommand == CommandPlus)
                lastCommand = CommandPlusPercent;
            else if (lastCommand == CommandMinus)
                lastCommand = CommandMinusPercent;
        }
        else
        {
            // most likely a number
            bool ok = false;
            int number = token.toUInt(&ok);

            if (ok == false)
                continue;

            switch (lastCommand)
            {
                case CommandNone:
                    // no command: this is a channel number
                    fromChannel = number;
                    toChannel = fromChannel;
                break;
                case CommandAT:
                    fromValue = float(number);
                    toValue = fromValue;
                break;
                case CommandTHRU:
                    if (thruCount == 0)
                        toChannel = number;
                    else
                        toValue = float(number);
                    thruCount++;
                break;
                case CommandFULL:
                    fromValue = toValue = 255;
                break;
                case CommandZERO:
                    fromValue = toValue = 0;
                break;
                case CommandBY:
                    byChannel = number;
                break;
                case CommandPlus:
                case CommandMinus:
                    toValue = number;
                break;
                case CommandPlusPercent:
                case CommandMinusPercent:
                    toValue = float(number) / 100.0;
                break;
            }
        }
    }

    float valueDelta = 0;
    if (toValue != fromValue)
    {
        valueDelta = (float(toChannel) - float(fromChannel)) / float(byChannel);
        valueDelta = (float(toValue) - float(fromValue)) / valueDelta;
    }

    for (quint32 i = fromChannel - 1; i <= toChannel - 1; i += byChannel)
    {
        uchar uniValue = 0;
        SceneValue scv;

        if (quint32(uniData.length()) >= i)
            uniValue = uchar(uniData.at(i));

        scv.channel = i;
        if (lastCommand == CommandPlus)
            scv.value = CLAMP(uniValue + toValue, 0, 255);
        else if (lastCommand == CommandMinus)
            scv.value = CLAMP(uniValue - toValue, 0, 255);
        else if (lastCommand == CommandPlusPercent)
            scv.value = CLAMP(uniValue * (1.0 + toValue), 0, 255);
        else if (lastCommand == CommandMinusPercent)
            scv.value = CLAMP(uniValue - (float(uniValue) * toValue), 0, 255);
        else
            scv.value = uchar(fromValue);

        values.append(scv);
        fromValue += valueDelta;
    }

    return values;
}
