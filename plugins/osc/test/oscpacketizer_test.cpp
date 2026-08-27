/*
  Q Light Controller Plus
  oscpacketizer_test.cpp

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

#include "oscpacketizer_test.h"

#define private public
#include "oscpacketizer.h"
#undef private

static void appendBundleMessage(QByteArray &bundle, const QByteArray &message)
{
    bundle.append(char((message.size() >> 24) & 0xff));
    bundle.append(char((message.size() >> 16) & 0xff));
    bundle.append(char((message.size() >> 8) & 0xff));
    bundle.append(char(message.size() & 0xff));
    bundle.append(message);
}

void OSCPacketizer_Test::parsePacketRejectsTruncatedTypeTags()
{
    OSCPacketizer op;
    QByteArray data;

    data.append("/x");
    data.append(char(0x00));
    data.append(',');
    data.append('f');

    QCOMPARE(op.parsePacket(data).size(), 0);
}

void OSCPacketizer_Test::parseMessageRejectsTruncatedTypeTags()
{
    OSCPacketizer op;
    QString path;
    QByteArray values;
    QByteArray data;

    data.append("/x");
    data.append(char(0x00));
    data.append(',');
    data.append('f');

    QVERIFY(op.parseMessage(data, path, values) == false);
}

void OSCPacketizer_Test::parseMessageRejectsTruncatedStringArgument()
{
    OSCPacketizer op;
    QString path;
    QByteArray values;
    QByteArray data;

    data.append("/x");
    data.append(QByteArray(2, 0));
    data.append(",s");
    data.append(QByteArray(2, 0));
    data.append("unterminated");

    QVERIFY(op.parseMessage(data, path, values) == false);
}

void OSCPacketizer_Test::parsePacketStopsAtTruncatedBundleEntry()
{
    OSCPacketizer op;
    QByteArray message;
    QByteArray data;

    op.setupOSCDmx(message, 1, 2, 128);

    data.append("#bundle");
    data.append(char(0x00));
    data.append(QByteArray(8, 0));
    appendBundleMessage(data, message);
    data.append(QByteArray(3, 0));

    QCOMPARE(op.parsePacket(data).size(), 1);
}

void OSCPacketizer_Test::parsePacketAcceptsBundleMessage()
{
    OSCPacketizer op;
    QByteArray message;
    QByteArray bundle;

    op.setupOSCDmx(message, 1, 2, 128);

    bundle.append("#bundle");
    bundle.append(char(0x00));
    bundle.append(QByteArray(8, 0));
    appendBundleMessage(bundle, message);

    QList<QPair<QString, QByteArray> > messages = op.parsePacket(bundle);
    QCOMPARE(messages.size(), 1);
    QCOMPARE(messages.first().first, QString("/1/dmx/2"));
    QCOMPARE(messages.first().second.size(), 1);
}

QTEST_MAIN(OSCPacketizer_Test)
