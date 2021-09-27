/*
  Q Light Controller Plus
  keypadparser.h

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

#ifndef KEYPADPARSER_H
#define KEYPADPARSER_H

#include "scenevalue.h"

class KeyPadParser
{
public:
    KeyPadParser();

    enum KeyPadCommands
    {
        CommandNone,
        CommandAT,
        CommandTHRU,
        CommandFULL,
        CommandZERO,
        CommandBY,
        CommandPlus,
        CommandPlusPercent,
        CommandMinus,
        CommandMinusPercent
    };

    /** Parse a string command from the UI Keypad.
     *  @param doc: reference to the project document
     *  @param command: the command as string
     *  @param uniData: universe data to compute percentage increase/decrease
     *
     *  @return a list of SceneValues
     */
    QList<SceneValue> parseCommand(Doc *doc, QString command, QByteArray &uniData);

private:
    QVector<quint16> m_channels;
};

#endif // KEYPADPARSER_H
