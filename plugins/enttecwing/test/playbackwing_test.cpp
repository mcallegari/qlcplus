/*
  Q Light Controller
  playbackwing_test.cpp

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

#include "playbackwing.h"
#include "playbackwing_test.h"

#define PLB_FIRMWARE 192
#define PLB_FLAGS (1 << 7) /* Page Up */	\
		| (1 << 6) /* Page Down */	\
		| (1 << 5) /* Back */		\
		| (1 << 4) /* Go */		\
		| (0 << 1) /* Product (1=PLB, 2=SHC, 3=PGM) */	\
		| (1 << 0) /* Product */

QByteArray PlaybackWing_Test::data()
{
    QByteArray data;

    data.resize(28);
    data[0] = 'W'; /* HEADER */
    data[1] = 'O'; /* HEADER */
    data[2] = 'D'; /* HEADER */
    data[3] = 'D'; /* HEADER */

    data[4] = (char)PLB_FIRMWARE; /* Firmware */

    data[5] = (char)PLB_FLAGS; /* Flags */

    data[6] = 0; /* Unused */

    data[7] = (char)255; /* Buttons */
    data[8] = (char)255; /* Buttons */
    data[9] = (char)255; /* Buttons */
    data[10] = (char)255; /* Buttons */
    data[11] = (char)255; /* Buttons */

    data[12] = 0; /* Unused */
    data[13] = 0; /* Unused */
    data[14] = 0; /* Unused */

    data[15] = 0; /* Fader 0 */
    data[16] = 0; /* Fader 1 */
    data[17] = 0; /* Fader 2 */
    data[18] = 0; /* Fader 3 */
    data[19] = 0; /* Fader 4 */
    data[20] = 0; /* Fader 5 */
    data[21] = 0; /* Fader 6 */
    data[22] = 0; /* Fader 7 */
    data[23] = 0; /* Fader 8 */
    data[24] = 0; /* Fader 9 */

    data[25] = 0; /* Unused */
    data[26] = 0; /* Unused */
    data[27] = 0; /* Unused */

    return data;
}

void PlaybackWing_Test::initTestCase()
{
    m_wing = new PlaybackWing(this, QHostAddress::LocalHost, data());
    QVERIFY(m_wing != NULL);
}

void PlaybackWing_Test::firmware()
{
    QVERIFY(m_wing->firmware() == PLB_FIRMWARE);
}

void PlaybackWing_Test::address()
{
    QVERIFY(m_wing->address() == QHostAddress::LocalHost);
}

void PlaybackWing_Test::isOutputData()
{
    QByteArray ba(data());

    QVERIFY(Wing::isOutputData(ba) == true);

    ba[1] = 'I';
    QVERIFY(Wing::isOutputData(ba) == false);
}

void PlaybackWing_Test::name()
{
    QCOMPARE(m_wing->name(), QString("Playback ") + tr("at") + QString(" ")
             + QHostAddress(QHostAddress::LocalHost).toString());
}

void PlaybackWing_Test::infoText()
{
    QString str = QString("<B>%1</B>").arg(m_wing->name());
    str += QString("<P>");
    str += tr("Firmware version %1").arg(PLB_FIRMWARE);
    str += QString("<BR>");
    str += tr("Device is operating correctly.");
    str += QString("</P>");
    QCOMPARE(m_wing->infoText(), str);
}

void PlaybackWing_Test::tooShortData()
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
    m_wing->parseData(foo);
}

void PlaybackWing_Test::buttons_data()
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
    QTest::newRow("Button 0") << ba << 10 << 0;
    ba[11] = 127; /* 0111 1111 */
    QTest::newRow("Button 0") << ba << 10 << 255;

    QTest::newRow("Button 1") << ba << 11 << 0;
    ba[11] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 1") << ba << 11 << 255;

    QTest::newRow("Button 2") << ba << 12 << 0;
    ba[11] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 2") << ba << 12 << 255;

    QTest::newRow("Button 3") << ba << 13 << 0;
    ba[11] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 3") << ba << 13 << 255;

    QTest::newRow("Button 4") << ba << 14 << 0;
    ba[11] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 4") << ba << 14 << 255;

    QTest::newRow("Button 5") << ba << 15 << 0;
    ba[11] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 5") << ba << 15 << 255;

    QTest::newRow("Button 6") << ba << 16 << 0;
    ba[11] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 6") << ba << 16 << 255;

    QTest::newRow("Button 7") << ba << 17 << 0;
    ba[11] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 7") << ba << 17 << 255;

    QTest::newRow("Button 8") << ba << 18 << 0;
    ba[10] = (char)127; /* 0111 1111 */
    QTest::newRow("Button 8") << ba << 18 << 255;

    QTest::newRow("Button 9") << ba << 19 << 0;
    ba[10] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 9") << ba << 19 << 255;

    QTest::newRow("Button 10") << ba << 20 << 0;
    ba[10] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 10") << ba << 20 << 255;

    QTest::newRow("Button 11") << ba << 21 << 0;
    ba[10] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 11") << ba << 21 << 255;

    QTest::newRow("Button 12") << ba << 22 << 0;
    ba[10] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 12") << ba << 22 << 255;

    QTest::newRow("Button 13") << ba << 23 << 0;
    ba[10] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 13") << ba << 23 << 255;

    QTest::newRow("Button 14") << ba << 24 << 0;
    ba[10] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 14") << ba << 24 << 255;

    QTest::newRow("Button 15") << ba << 25 << 0;
    ba[10] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 15") << ba << 25 << 255;

    QTest::newRow("Button 16") << ba << 26 << 0;
    ba[9] = 127; /* 0111 1111 */
    QTest::newRow("Button 16") << ba << 26 << 255;

    QTest::newRow("Button 17") << ba << 27 << 0;
    ba[9] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 17") << ba << 27 << 255;

    QTest::newRow("Button 18") << ba << 28 << 0;
    ba[9] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 18") << ba << 28 << 255;

    QTest::newRow("Button 19") << ba << 29 << 0;
    ba[9] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 19") << ba << 29 << 255;

    QTest::newRow("Button 20") << ba << 30 << 0;
    ba[9] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 20") << ba << 30 << 255;

    QTest::newRow("Button 21") << ba << 31 << 0;
    ba[9] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 21") << ba << 31 << 255;

    QTest::newRow("Button 22") << ba << 32 << 0;
    ba[8] = (char)191; /* 1011 1111 */
    QTest::newRow("Button 22") << ba << 32 << 255;

    QTest::newRow("Button 23") << ba << 33 << 0;
    ba[8] = (char)223; /* 1101 1111 */
    QTest::newRow("Button 23") << ba << 33 << 255;

    QTest::newRow("Button 24") << ba << 34 << 0;
    ba[8] = (char)239; /* 1110 1111 */
    QTest::newRow("Button 24") << ba << 34 << 255;

    QTest::newRow("Button 25") << ba << 35 << 0;
    ba[8] = (char)247; /* 1111 0111 */
    QTest::newRow("Button 25") << ba << 35 << 255;

    QTest::newRow("Button 26") << ba << 36 << 0;
    ba[8] = (char)251; /* 1111 1011 */
    QTest::newRow("Button 26") << ba << 36 << 255;

    QTest::newRow("Button 27") << ba << 37 << 0;
    ba[9] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 27") << ba << 37 << 255;

    QTest::newRow("Button 28") << ba << 38 << 0;
    ba[9] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 28") << ba << 38 << 255;

    QTest::newRow("Button 29") << ba << 39 << 0;
    ba[8] = 127; /* 0111 1111 */
    QTest::newRow("Button 29") << ba << 39 << 255;

    QTest::newRow("Button 30") << ba << 40 << 0;
    ba[8] = (char)253; /* 1111 1101 */
    QTest::newRow("Button 30") << ba << 40 << 255;

    QTest::newRow("Button 31") << ba << 41 << 0;
    ba[8] = (char)254; /* 1111 1110 */
    QTest::newRow("Button 31") << ba << 41 << 255;
}

void PlaybackWing_Test::buttons()
{
    QFETCH(QByteArray, ba);
    QFETCH(int, channel);
    QFETCH(int, value);

    m_wing->parseData(ba);
    QVERIFY(m_wing->cacheValue(channel) == (unsigned char) value);
}

void PlaybackWing_Test::faders_data()
{
    QByteArray ba(data());

    QTest::addColumn<QByteArray> ("ba");
    QTest::addColumn<int> ("channel");
    QTest::addColumn<int> ("value");

    ba[15] = 127;
    QTest::newRow("Fader 0") << ba << 0 << 127;
    ba[16] = (char)191;
    QTest::newRow("Fader 1") << ba << 1 << 191;
    ba[17] = (char)223;
    QTest::newRow("Fader 2") << ba << 2 << 223;
    ba[18] = (char)239;
    QTest::newRow("Fader 3") << ba << 3 << 239;
    ba[19] = (char)247;
    QTest::newRow("Fader 4") << ba << 4 << 247;
    ba[20] = (char)251;
    QTest::newRow("Fader 5") << ba << 5 << 251;
    ba[21] = (char)253;
    QTest::newRow("Fader 6") << ba << 6 << 253;
    ba[22] = (char)254;
    QTest::newRow("Fader 7") << ba << 7 << 254;
    ba[23] = 1;
    QTest::newRow("Fader 8") << ba << 8 << 1;
    ba[24] = 13;
    QTest::newRow("Fader 9") << ba << 9 << 13;
}

void PlaybackWing_Test::faders()
{
    // TODO: who did the page changes need to fix this !
    QFETCH(QByteArray, ba);
    //QFETCH(int, channel);
    //QFETCH(int, value);

    m_wing->parseData(ba);
    //QVERIFY(m_wing->cacheValue(channel) == (unsigned char) value);
}

void PlaybackWing_Test::cleanupTestCase()
{
    delete m_wing;
    m_wing = NULL;
}
