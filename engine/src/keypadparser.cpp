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


KeyPadParser::KeyPadParser()
{

}

QList<SceneValue> KeyPadParser::parseCommand(Doc *doc, QString command)
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
            fromValue = toValue = 255;
        }
        else if (token == "ZERO")
        {
            lastCommand = CommandZERO;
            fromValue = toValue = 0;
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
                    // nothing to do?
                break;
                case CommandZERO:
                    // nothing to do?
                break;
                case CommandBY:
                    byChannel = number;
                break;
                case CommandPlus:
                    // TODO
                break;
                case CommandMinus:
                    // TODO
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

    for (quint32 i = fromChannel; i <= toChannel; i += byChannel)
    {
        SceneValue scv;
        scv.channel = i;
        scv.value = uchar(fromValue);

        values.append(scv);
        fromValue += valueDelta;
    }

    return values;
}
