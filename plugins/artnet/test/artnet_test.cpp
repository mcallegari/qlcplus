/*
  Q Light Controller Plus
  artnet_test.cpp

  Copyright (c) Jano Svitok

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

#define private public
#include "artnet_test.h"
#include "artnetpacketizer.h"
#undef private

/****************************************************************************
 * ArtNet tests
 ****************************************************************************/

void ArtNet_Test::setupArtNetDmx()
{
    ArtNetPacketizer ap;

    QByteArray data;
    const QByteArray empty;
    const QByteArray fifty(50, 10);
    const QByteArray fiftyone(51, 10);
    const QByteArray full(512, 20);

    // empty data
    ap.setupArtNetDmx(data, 0, empty);

    QCOMPARE(data.size(), 20);
    QCOMPARE(data.data(), "Art-Net");

    // full data
    ap.setupArtNetDmx(data, 0, full);

    QCOMPARE(data.size(), 18 + 512);
    QCOMPARE(data.data(), "Art-Net");

    // partial data
    ap.setupArtNetDmx(data, 0, fifty);

    QCOMPARE(data.size(), 18 + 50);
    QCOMPARE(data.data(), "Art-Net");

    ap.setupArtNetDmx(data, 0, fiftyone);

    QCOMPARE(data.size(), 18 + 52);
    QCOMPARE(data.data(), "Art-Net");
}

QTEST_MAIN(ArtNet_Test)
