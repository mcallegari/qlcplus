/*
  Q Light Controller Plus
  webaccessescaping_test.cpp

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

#include <QTest>

#include "commonjscss.h"
#include "webaccessescaping_test.h"

void WebAccessEscaping_Test::leavesPlainAsciiUnchanged()
{
    QCOMPARE(webAccessJsStringEscaped("QLC+ fixture 123"),
             QString("QLC+ fixture 123"));
}

void WebAccessEscaping_Test::escapesJavaScriptStringDelimiters()
{
    QCOMPARE(webAccessJsStringEscaped("'\"\\"),
             QString("\\'\\\"\\\\"));
}

void WebAccessEscaping_Test::escapesScriptBreakingCharacters()
{
    QString text;
    text.append('<');
    text.append('>');
    text.append('&');
    text.append(QChar(0x2028));
    text.append(QChar(0x2029));

    QCOMPARE(webAccessJsStringEscaped(text),
             QString("\\x3C>&\\u2028\\u2029"));
}

void WebAccessEscaping_Test::escapesControlCharacters()
{
    QString text;
    text.append(QChar(0x0000));
    text.append('\b');
    text.append('\f');
    text.append('\n');
    text.append('\r');
    text.append('\t');

    QCOMPARE(webAccessJsStringEscaped(text),
             QString("\\u0000\\b\\f\\n\\r\\t"));
}

QTEST_MAIN(WebAccessEscaping_Test)
