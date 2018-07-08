/*
  Q Light Controller
  shortcutwing_test.cpp

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

#include <QApplication>
#include <QUdpSocket>
#include <QByteArray>
#include <QTest>

#include "shortcutwing.h"
#include "shortcutwing_test.h"

#define SHC_FIRMWARE 147
#define SHC_FLAGS (1 << 7) /* Page Up */	\
		| (1 << 6) /* Page Down */	\
		| (1 << 5) /* Back */		\
		| (1 << 4) /* Go */		\
		| (1 << 0) /* Product (1=PLB, 2=SHC, 3=PGM) */	\
		| (0 << 0) /* Product */

QByteArray ShortcutWing_Test::data()
{
    QByteArray data;

    data.resize(30);
    data[0] = 'W'; /* HEADER */
    data[1] = 'O'; /* HEADER */
    data[2] = 'D'; /* HEADER */
    data[3] = 'D'; /* HEADER */

    data[4] = (char)SHC_FIRMWARE; /* Firmware */

    data[5] = (char)SHC_FLAGS; /* Flags */

    data[6] = (char)255; /* 4 Buttons */
    data[7] = (char)255; /* 8 Buttons */
    data[8] = (char)255; /* 8 Buttons */
    data[9] = (char)255; /* 8 Buttons */
    data[10] = (char)255; /* 8 Buttons */
    data[11] = (char)255; /* 8 Buttons */
    data[12] = (char)255; /* 8 Buttons */
    data[13] = (char)255; /* 8 Buttons */

    data[14] = 0; /* Unused */
    data[15] = 0; /* Unused */
    data[16] = 0; /* Unused */
    data[17] = 0; /* Unused */
    data[18] = 0; /* Unused */
    data[19] = 0; /* Unused */
    data[20] = 0; /* Unused */
    data[21] = 0; /* Unused */
    data[22] = 0; /* Unused */
    data[23] = 0; /* Unused */
    data[24] = 0; /* Unused */
    data[25] = 0; /* Unused */
    data[26] = 0; /* Unused */
    data[27] = 0; /* Unused */
    data[28] = 0; /* Unused */
    data[29] = 0; /* Unused */

    return data;
}

void ShortcutWing_Test::initTestCase()
{
    m_wing = new ShortcutWing(this, QHostAddress::LocalHost, data());
    QVERIFY(m_wing != NULL);
}

void ShortcutWing_Test::firmware()
{
    QVERIFY(m_wing->firmware() == SHC_FIRMWARE);
}

void ShortcutWing_Test::address()
{
    QVERIFY(m_wing->address() == QHostAddress::LocalHost);
}

void ShortcutWing_Test::isOutputData()
{
    QByteArray ba(data());

    QVERIFY(Wing::isOutputData(ba) == true);

    ba[1] = 'I';
    QVERIFY(Wing::isOutputData(ba) == false);
}

void ShortcutWing_Test::name()
{
    QCOMPARE(m_wing->name(), QString("Shortcut ") + tr("at") + QString(" ")
             + QHostAddress(QHostAddress::LocalHost).toString());
}

void ShortcutWing_Test::infoText()
{
    QString str = QString("<B>%1</B>").arg(m_wing->name());
    str += QString("<P>");
    str += tr("Firmware version %1").arg(SHC_FIRMWARE);
    str += QString("<BR>");
    str += tr("Device is operating correctly.");
    str += QString("</P>");
    QCOMPARE(m_wing->infoText(), str);
}

void ShortcutWing_Test::tooShortData()
{
    // Just a stability check; nothing should happen if data is too short
    QByteArray foo;
    foo.append(char(123));
    foo.append(char(45));
    foo.append(char(67));
    foo.append(char(89));
    foo.append(char(123));
    foo.append(char(45));
    foo.append(char(67));
    foo.append(char(89));
    foo.append(char(123));
    foo.append(char(45));
    foo.append(char(67));
    foo.append(char(89));
    foo.append(char(123));
    m_wing->parseData(foo);
}

void ShortcutWing_Test::buttons_data()
{
    QByteArray ba(data());

    /* Create columns for a QByteArray that is fed to Wing::parseData()
       on each row, the channel number to read and the value expected for
       that channel after each parseData() call. */
    QTest::addColumn<QByteArray> ("ba");
    QTest::addColumn<int> ("channel");
    QTest::addColumn<int> ("value");

    /* First test that the button state is read as OFF, then set the
       button's state ON in the byte array (simulating a UDP packet that
       has been read from the network) and read the value again. Low bit (0)
       means that the button is down, high bit (1) means it's up. */

    /*             ROW NAME     DATA   CH   VAL */
    QTest::newRow("Button 0") << ba << 0 << 0;
    ba[13] = 127; /* 0111 1111 */
    QTest::newRow("Button 0") << ba << 0 << 255;

    QTest::newRow("Button 1") << ba << 1 << 0;
    ba[13] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 1") << ba << 1 << 255;

    QTest::newRow("Button 2") << ba << 2 << 0;
    ba[13] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 2") << ba << 2 << 255;

    QTest::newRow("Button 3") << ba << 3 << 0;
    ba[13] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 3") << ba << 3 << 255;

    QTest::newRow("Button 4") << ba << 4 << 0;
    ba[13] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 4") << ba << 4 << 255;

    QTest::newRow("Button 5") << ba << 5 << 0;
    ba[13] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 5") << ba << 5 << 255;

    QTest::newRow("Button 6") << ba << 6 << 0;
    ba[13] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 6") << ba << 6 << 255;

    QTest::newRow("Button 7") << ba << 7 << 0;
    ba[13] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 7") << ba << 7 << 255;

    QTest::newRow("Button 8") << ba << 8 << 0;
    ba[12] = 127; /* 0111 1111 */
    QTest::newRow("Button 8") << ba << 8 << 255;

    QTest::newRow("Button 9") << ba << 9 << 0;
    ba[12] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 9") << ba << 9 << 255;

    QTest::newRow("Button 10") << ba << 10 << 0;
    ba[12] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 10") << ba << 10 << 255;

    QTest::newRow("Button 11") << ba << 11 << 0;
    ba[12] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 11") << ba << 11 << 255;

    QTest::newRow("Button 12") << ba << 12 << 0;
    ba[12] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 12") << ba << 12 << 255;

    QTest::newRow("Button 13") << ba << 13 << 0;
    ba[12] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 13") << ba << 13 << 255;

    QTest::newRow("Button 14") << ba << 14 << 0;
    ba[12] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 14") << ba << 14 << 255;

    QTest::newRow("Button 15") << ba << 15 << 0;
    ba[12] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 15") << ba << 15 << 255;

    QTest::newRow("Button 16") << ba << 16 << 0;
    ba[11] = 127; /* 0111 1111 */
    QTest::newRow("Button 16") << ba << 16 << 255;

    QTest::newRow("Button 17") << ba << 17 << 0;
    ba[11] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 17") << ba << 17 << 255;

    QTest::newRow("Button 18") << ba << 18 << 0;
    ba[11] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 18") << ba << 18 << 255;

    QTest::newRow("Button 19") << ba << 19 << 0;
    ba[11] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 19") << ba << 19 << 255;

    QTest::newRow("Button 20") << ba << 20 << 0;
    ba[11] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 20") << ba << 20 << 255;

    QTest::newRow("Button 21") << ba << 21 << 0;
    ba[11] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 21") << ba << 21 << 255;

    QTest::newRow("Button 22") << ba << 22 << 0;
    ba[11] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 22") << ba << 22 << 255;

    QTest::newRow("Button 23") << ba << 23 << 0;
    ba[11] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 23") << ba << 23 << 255;

    QTest::newRow("Button 24") << ba << 24 << 0;
    ba[10] = 127; /* 0111 1111 */
    QTest::newRow("Button 24") << ba << 24 << 255;

    QTest::newRow("Button 25") << ba << 25 << 0;
    ba[10] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 25") << ba << 25 << 255;

    QTest::newRow("Button 26") << ba << 26 << 0;
    ba[10] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 26") << ba << 26 << 255;

    QTest::newRow("Button 27") << ba << 27 << 0;
    ba[10] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 27") << ba << 27 << 255;

    QTest::newRow("Button 28") << ba << 28 << 0;
    ba[10] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 28") << ba << 28 << 255;

    QTest::newRow("Button 29") << ba << 29 << 0;
    ba[10] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 29") << ba << 29 << 255;

    QTest::newRow("Button 30") << ba << 30 << 0;
    ba[10] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 30") << ba << 30 << 255;

    QTest::newRow("Button 31") << ba << 31 << 0;
    ba[10] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 31") << ba << 31 << 255;

    QTest::newRow("Button 32") << ba << 32 << 0;
    ba[9] = 127; /* 0111 1111 */
    QTest::newRow("Button 32") << ba << 32 << 255;

    QTest::newRow("Button 33") << ba << 33 << 0;
    ba[9] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 33") << ba << 33 << 255;

    QTest::newRow("Button 34") << ba << 34 << 0;
    ba[9] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 34") << ba << 34 << 255;

    QTest::newRow("Button 35") << ba << 35 << 0;
    ba[9] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 35") << ba << 35 << 255;

    QTest::newRow("Button 36") << ba << 36 << 0;
    ba[9] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 36") << ba << 36 << 255;

    QTest::newRow("Button 37") << ba << 37 << 0;
    ba[9] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 37") << ba << 37 << 255;

    QTest::newRow("Button 38") << ba << 38 << 0;
    ba[9] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 38") << ba << 38 << 255;

    QTest::newRow("Button 39") << ba << 39 << 0;
    ba[9] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 39") << ba << 39 << 255;

    QTest::newRow("Button 40") << ba << 40 << 0;
    ba[8] = 127; /* 0111 1111 */
    QTest::newRow("Button 40") << ba << 40 << 255;

    QTest::newRow("Button 41") << ba << 41 << 0;
    ba[8] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 41") << ba << 41 << 255;

    QTest::newRow("Button 42") << ba << 42 << 0;
    ba[8] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 42") << ba << 42 << 255;

    QTest::newRow("Button 43") << ba << 43 << 0;
    ba[8] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 43") << ba << 43 << 255;

    QTest::newRow("Button 44") << ba << 44 << 0;
    ba[8] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 44") << ba << 44 << 255;

    QTest::newRow("Button 45") << ba << 45 << 0;
    ba[8] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 45") << ba << 45 << 255;

    QTest::newRow("Button 46") << ba << 46 << 0;
    ba[8] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 46") << ba << 46 << 255;

    QTest::newRow("Button 47") << ba << 47 << 0;
    ba[8] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 47") << ba << 47 << 255;

    QTest::newRow("Button 48") << ba << 48 << 0;
    ba[7] = 127; /* 0111 1111 */
    QTest::newRow("Button 48") << ba << 48 << 255;

    QTest::newRow("Button 49") << ba << 49 << 0;
    ba[7] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 49") << ba << 49 << 255;

    QTest::newRow("Button 50") << ba << 50 << 0;
    ba[7] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 50") << ba << 50 << 255;

    QTest::newRow("Button 51") << ba << 51 << 0;
    ba[7] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 51") << ba << 51 << 255;

    QTest::newRow("Button 52") << ba << 52 << 0;
    ba[7] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 52") << ba << 52 << 255;

    QTest::newRow("Button 53") << ba << 53 << 0;
    ba[7] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 53") << ba << 53 << 255;

    QTest::newRow("Button 54") << ba << 54 << 0;
    ba[7] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 54") << ba << 54 << 255;

    QTest::newRow("Button 55") << ba << 55 << 0;
    ba[7] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 55") << ba << 55 << 255;

    QTest::newRow("Button 56") << ba << 56 << 0;
    ba[6] = 127; /* 0111 1111 */
    QTest::newRow("Button 56") << ba << 56 << 255;

    QTest::newRow("Button 57") << ba << 57 << 0;
    ba[6] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 57") << ba << 57 << 255;

    QTest::newRow("Button 58") << ba << 58 << 0;
    ba[6] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 58") << ba << 58 << 255;

    QTest::newRow("Button 59") << ba << 59 << 0;
    ba[6] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 59") << ba << 59 << 255;
}

void ShortcutWing_Test::buttons()
{
    QFETCH(QByteArray, ba);
    QFETCH(int, channel);
    QFETCH(int, value);

    m_wing->parseData(ba);
    QVERIFY(m_wing->cacheValue(channel) == (unsigned char) value);
}

void ShortcutWing_Test::cleanupTestCase()
{
    delete m_wing;
    m_wing = NULL;
}
