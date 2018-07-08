/*
  Q Light Controller
  wing_test.cpp

  Copyright (c) Heikki Junnila

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

#define protected public
#include "wing.h"
#undef protected

#include "wing_test.h"

WingStub::WingStub(QObject* parent, const QHostAddress& host, const QByteArray& ba)
    : Wing(parent, host, ba)
{
    Q_UNUSED(parent);
    Q_UNUSED(host);
    Q_UNUSED(ba);
}

WingStub::~WingStub()
{
}

QString WingStub::name() const
{
    return QString("WingStub");
}

void WingStub::parseData(const QByteArray& ba)
{
    Q_UNUSED(ba);
}

void Wing_Test::resolveType()
{
    QByteArray ba;
    QCOMPARE(Wing::resolveType(ba), Wing::Unknown);

    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));

    QCOMPARE(Wing::resolveType(ba), Wing::Unknown);

    ba[5] = char(0x01);
    QCOMPARE(Wing::resolveType(ba), Wing::Playback);

    ba[5] = char(0x02);
    QCOMPARE(Wing::resolveType(ba), Wing::Shortcut);

    ba[5] = char(0x03);
    QCOMPARE(Wing::resolveType(ba), Wing::Program);
}

void Wing_Test::resolveFirmware()
{
    QByteArray ba;
    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x00));

    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));

    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x00));

    ba[4] = char(0x01);
    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x01));

    ba[4] = char(0x54);
    QCOMPARE(Wing::resolveFirmware(ba), uchar(0x54));
}

void Wing_Test::isOutputData()
{
    QByteArray ba;
    QCOMPARE(Wing::isOutputData(ba), false);

    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    QCOMPARE(Wing::isOutputData(ba), false);

    ba[0] = 'W';
    ba[1] = 'O';
    ba[2] = 'D';
    ba[3] = 'D';
    QCOMPARE(Wing::isOutputData(ba), true);

    ba[0] = 'V';
    ba[1] = 'O';
    ba[2] = 'D';
    ba[3] = 'D';
    QCOMPARE(Wing::isOutputData(ba), false);

    ba[0] = 'W';
    ba[1] = 'O';
    ba[2] = 'T';
    ba[3] = 'D';
    QCOMPARE(Wing::isOutputData(ba), false);
}

void Wing_Test::initial()
{
    QHostAddress addr("192.168.1.5");
    QByteArray ba;
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x00));
    ba.append(char(0x80));
    ba.append(char(0x02));

    WingStub es(this, addr, ba);
    QCOMPARE(es.address(), addr);
    QCOMPARE(es.type(), Wing::Shortcut);
    QCOMPARE(es.firmware(), uchar(0x80));
    QCOMPARE(es.page(), uchar(0));

    // Just for coverage :)
    es.feedBack(0, 1);
}

void Wing_Test::page()
{
    QHostAddress addr;
    QByteArray ba;
    WingStub es(this, addr, ba);

    uchar i;
    for (i = 0; i < 98; i++)
    {
        QCOMPARE(es.page(), i);
        es.nextPage();
    }

    es.nextPage();
    QCOMPARE(es.page(), uchar(0));

    es.previousPage();
    QCOMPARE(es.page(), uchar(98));

    for (i = 98; i > 0; i--)
    {
        QCOMPARE(es.page(), i);
        es.previousPage();
    }

    es.previousPage();
    QCOMPARE(es.page(), uchar(98));
}

void Wing_Test::bcd()
{
    QCOMPARE(Wing::toBCD(uchar(99)), uchar(0x99));
    QCOMPARE(Wing::toBCD(uchar(17)), uchar(0x17));
    QCOMPARE(Wing::toBCD(uchar(0)), uchar(0x00));
    QCOMPARE(Wing::toBCD(uchar(1)), uchar(0x01));
    QCOMPARE(Wing::toBCD(uchar(10)), uchar(0x10));
}

void Wing_Test::cache()
{
    QHostAddress addr;
    QByteArray ba;
    WingStub es(this, addr, ba);
    es.m_values = QByteArray(3, 0);

    es.setCacheValue(0, uchar(255));
    QCOMPARE(es.cacheValue(0), uchar(255));
    QCOMPARE(es.cacheValue(1), uchar(0));

    es.setCacheValue(1, uchar(255));
    QCOMPARE(es.cacheValue(0), uchar(255));
    QCOMPARE(es.cacheValue(1), uchar(255));

    es.setCacheValue(2, uchar(255));
    QCOMPARE(es.cacheValue(0), uchar(255));
    QCOMPARE(es.cacheValue(1), uchar(255));
    QCOMPARE(es.cacheValue(2), uchar(255));

    es.setCacheValue(3, uchar(255));
    QCOMPARE(es.cacheValue(3), uchar(0));
}
