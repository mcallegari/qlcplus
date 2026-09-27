/*
  Q Light Controller Plus
  commonjscss.cpp

  Copyright (c) Q Light Controller Plus

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

#include "commonjscss.h"

QString webAccessJsStringEscaped(const QString &text)
{
    QString escaped;
    escaped.reserve(text.length() * 2);

    for (const QChar &ch : text)
    {
        const char16_t code = ch.unicode();

        switch (code)
        {
            case '\\':
                escaped += "\\\\";
            break;
            case '\'':
                escaped += "\\'";
            break;
            case '"':
                escaped += "\\\"";
            break;
            case '\b':
                escaped += "\\b";
            break;
            case '\f':
                escaped += "\\f";
            break;
            case '\n':
                escaped += "\\n";
            break;
            case '\r':
                escaped += "\\r";
            break;
            case '\t':
                escaped += "\\t";
            break;
            case '<':
                escaped += "\\x3C";
            break;
            default:
                if (code < 0x20 || code == 0x2028 || code == 0x2029)
                {
                    escaped += "\\u";
                    escaped += QString::number(code, 16).rightJustified(4, '0');
                }
                else
                {
                    escaped += ch;
                }
            break;
        }
    }

    return escaped;
}
