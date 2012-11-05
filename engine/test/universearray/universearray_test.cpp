/*
  Q Light Controller - Unit test
  universearray_test.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QtTest>
#include <sys/time.h>

#include "universearray_test.h"

#define protected public
#include "universearray.h"
#undef protected

void UniverseArray_Test::initial()
{
    UniverseArray ua(127);
    QCOMPARE(ua.size(), 127);
    QCOMPARE(ua.gMChannelMode(), UniverseArray::GMIntensity);
    QCOMPARE(ua.gMValueMode(), UniverseArray::GMReduce);
    QCOMPARE(ua.gMValue(), uchar(255));
    QCOMPARE(ua.gMFraction(), double(1.0));
}

void UniverseArray_Test::gMChannelMode()
{
    UniverseArray ua(1);

    ua.setGMChannelMode(UniverseArray::GMAllChannels);
    QCOMPARE(ua.gMChannelMode(), UniverseArray::GMAllChannels);

    ua.setGMChannelMode(UniverseArray::GMIntensity);
    QCOMPARE(ua.gMChannelMode(), UniverseArray::GMIntensity);

    QCOMPARE(UniverseArray::stringToGMChannelMode("All"), UniverseArray::GMAllChannels);
    QCOMPARE(UniverseArray::stringToGMChannelMode("Intensity"), UniverseArray::GMIntensity);
    QCOMPARE(UniverseArray::stringToGMChannelMode("foobar"), UniverseArray::GMIntensity);

    QCOMPARE(UniverseArray::gMChannelModeToString(UniverseArray::GMAllChannels), QString("All"));
    QCOMPARE(UniverseArray::gMChannelModeToString(UniverseArray::GMIntensity), QString("Intensity"));
    QCOMPARE(UniverseArray::gMChannelModeToString(UniverseArray::GMChannelMode(42)), QString("Intensity"));
}

void UniverseArray_Test::gMValueMode()
{
    UniverseArray ua(1);

    ua.setGMValueMode(UniverseArray::GMLimit);
    QCOMPARE(ua.gMValueMode(), UniverseArray::GMLimit);

    ua.setGMValueMode(UniverseArray::GMReduce);
    QCOMPARE(ua.gMValueMode(), UniverseArray::GMReduce);

    QCOMPARE(UniverseArray::stringToGMValueMode("Limit"), UniverseArray::GMLimit);
    QCOMPARE(UniverseArray::stringToGMValueMode("Reduce"), UniverseArray::GMReduce);
    QCOMPARE(UniverseArray::stringToGMValueMode("xyzzy"), UniverseArray::GMReduce);

    QCOMPARE(UniverseArray::gMValueModeToString(UniverseArray::GMLimit), QString("Limit"));
    QCOMPARE(UniverseArray::gMValueModeToString(UniverseArray::GMReduce), QString("Reduce"));
    QCOMPARE(UniverseArray::gMValueModeToString(UniverseArray::GMValueMode(31337)), QString("Reduce"));
}

void UniverseArray_Test::gMValue()
{
    UniverseArray ua(1);

    for (int i = 0; i < UCHAR_MAX; i++)
    {
        ua.setGMValue(uchar(i));
        QCOMPARE(ua.gMValue(), uchar(i));
        QCOMPARE(ua.gMFraction(), (double(i) / double(UCHAR_MAX)));
    }

    ua.setGMValue(0);
    QCOMPARE(ua.gMValue(), uchar(0));
    QCOMPARE(ua.gMFraction(), double(0));

    ua.setGMValue(255);
    QCOMPARE(ua.gMValue(), uchar(255));
    QCOMPARE(ua.gMFraction(), double(1));
}

void UniverseArray_Test::setGMValue()
{
    UniverseArray ua(5);

    ua.write(0, 10, QLCChannel::Intensity);
    ua.write(1, 20, QLCChannel::Intensity);
    ua.write(2, 30, QLCChannel::Pan);
    ua.write(3, 40, QLCChannel::Tilt);
    ua.write(4, 50, QLCChannel::Intensity);

    ua.setGMValue(63);
    QCOMPARE(ua.postGMValues()->at(0), char(2));
    QCOMPARE(ua.postGMValues()->at(1), char(5));
    QCOMPARE(ua.postGMValues()->at(2), char(30));
    QCOMPARE(ua.postGMValues()->at(3), char(40));
    QCOMPARE(ua.postGMValues()->at(4), char(12));

    ua.setGMChannelMode(UniverseArray::GMAllChannels);
    QCOMPARE(ua.postGMValues()->at(0), char(2));
    QCOMPARE(ua.postGMValues()->at(1), char(5));
    QCOMPARE(ua.postGMValues()->at(2), char(7));
    QCOMPARE(ua.postGMValues()->at(3), char(10));
    QCOMPARE(ua.postGMValues()->at(4), char(12));

    ua.setGMChannelMode(UniverseArray::GMIntensity);
    QCOMPARE(ua.postGMValues()->at(0), char(2));
    QCOMPARE(ua.postGMValues()->at(1), char(5));
    QCOMPARE(ua.postGMValues()->at(2), char(30));
    QCOMPARE(ua.postGMValues()->at(3), char(40));
    QCOMPARE(ua.postGMValues()->at(4), char(12));

    ua.setGMValueMode(UniverseArray::GMLimit);
    QCOMPARE(ua.postGMValues()->at(0), char(10));
    QCOMPARE(ua.postGMValues()->at(1), char(20));
    QCOMPARE(ua.postGMValues()->at(2), char(30));
    QCOMPARE(ua.postGMValues()->at(3), char(40));
    QCOMPARE(ua.postGMValues()->at(4), char(50));

    ua.setGMValue(5);
    QCOMPARE(ua.postGMValues()->at(0), char(5));
    QCOMPARE(ua.postGMValues()->at(1), char(5));
    QCOMPARE(ua.postGMValues()->at(2), char(30));
    QCOMPARE(ua.postGMValues()->at(3), char(40));
    QCOMPARE(ua.postGMValues()->at(4), char(5));

    ua.setGMChannelMode(UniverseArray::GMAllChannels);
    QCOMPARE(ua.postGMValues()->at(0), char(5));
    QCOMPARE(ua.postGMValues()->at(1), char(5));
    QCOMPARE(ua.postGMValues()->at(2), char(5));
    QCOMPARE(ua.postGMValues()->at(3), char(5));
    QCOMPARE(ua.postGMValues()->at(4), char(5));
}

void UniverseArray_Test::applyGM()
{
    UniverseArray ua(1);

    QCOMPARE(ua.m_gMIntensityChannels.size(), 0);
    QCOMPARE(ua.m_gMNonIntensityChannels.size(), 0);
    QCOMPARE(ua.applyGM(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(ua.applyGM(0, 200, QLCChannel::Colour), uchar(200));

    ua.setGMValue(127);
    QCOMPARE(ua.applyGM(0, 50, QLCChannel::Intensity), uchar(25));
    QCOMPARE(ua.applyGM(0, 200, QLCChannel::Intensity), uchar(100));
    QCOMPARE(ua.applyGM(0, 200, QLCChannel::Colour), uchar(200));

    ua.setGMValueMode(UniverseArray::GMLimit);
    QCOMPARE(ua.applyGM(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(ua.applyGM(0, 200, QLCChannel::Intensity), uchar(127));
    QCOMPARE(ua.applyGM(0, 255, QLCChannel::Colour), uchar(255));

    ua.setGMChannelMode(UniverseArray::GMAllChannels);
    QCOMPARE(ua.applyGM(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(ua.applyGM(0, 200, QLCChannel::Intensity), uchar(127));
    QCOMPARE(ua.applyGM(0, 255, QLCChannel::Colour), uchar(127));

    ua.setGMValueMode(UniverseArray::GMReduce);
    QCOMPARE(ua.applyGM(0, 50, QLCChannel::Intensity), uchar(25));
    QCOMPARE(ua.applyGM(0, 200, QLCChannel::Intensity), uchar(100));
    QCOMPARE(ua.applyGM(0, 255, QLCChannel::Colour), uchar(127));

    QCOMPARE(ua.m_gMIntensityChannels.size(), 1);
    QCOMPARE(ua.m_gMNonIntensityChannels.size(), 1);
}

void UniverseArray_Test::write()
{
    UniverseArray ua(10);

    QVERIFY(ua.write(10, 255, QLCChannel::Intensity) == false);
    QCOMPARE(ua.postGMValues()->at(9), char(0));
    QCOMPARE(ua.postGMValues()->at(4), char(0));
    QCOMPARE(ua.postGMValues()->at(0), char(0));

    QVERIFY(ua.write(9, 255, QLCChannel::Intensity) == true);
    QCOMPARE(ua.postGMValues()->at(9), char(255));
    QCOMPARE(ua.postGMValues()->at(4), char(0));
    QCOMPARE(ua.postGMValues()->at(0), char(0));

    QVERIFY(ua.write(0, 255, QLCChannel::Intensity) == true);
    QCOMPARE(ua.postGMValues()->at(9), char(255));
    QCOMPARE(ua.postGMValues()->at(4), char(0));
    QCOMPARE(ua.postGMValues()->at(0), char(255));

    ua.setGMValue(127);
    QCOMPARE(ua.postGMValues()->at(9), char(127));
    QCOMPARE(ua.postGMValues()->at(4), char(0));
    QCOMPARE(ua.postGMValues()->at(0), char(127));

    QVERIFY(ua.write(4, 200, QLCChannel::Intensity) == true);
    QCOMPARE(ua.postGMValues()->at(9), char(127));
    QCOMPARE(ua.postGMValues()->at(4), char(100));
    QCOMPARE(ua.postGMValues()->at(0), char(127));
}

void UniverseArray_Test::reset()
{
    UniverseArray ua(128);
    int i;

    for (i = 0; i < 128; i++)
    {
        ua.write(i, 200, QLCChannel::Intensity);
        QCOMPARE(ua.postGMValues()->at(i), char(200));
    }

    // Reset channels 10-127 (512 shouldn't cause a crash)
    ua.reset(10, 512);
    for (i = 0; i < 10; i++)
        QCOMPARE(ua.postGMValues()->at(i), char(200));
    for (i = 10; i < 128; i++)
        QCOMPARE(ua.postGMValues()->at(i), char(0));

    // Reset all
    ua.reset();
    for (i = 0; i < 128; i++)
        QCOMPARE(ua.postGMValues()->at(i), char(0));
}

void UniverseArray_Test::setGMValueEfficiency()
{
    UniverseArray* ua = new UniverseArray(512 * 4);
    int i;

    for (i = 0; i < int(512 * 4); i++)
        ua->write(i, 200, QLCChannel::Intensity);

    /* This applies 50%(127) Grand Master to ALL channels in all universes.
       I'm not really sure what kinds of figures to expect here, since this
       is just one part in the overall processor load. Typically I get ~0.37ms
       on an Intel Core 2 E6550@2.33GHz, which looks plausible to me:
       DMX frame interval is 1/44Hz =~ 23ms. Applying GM to ALL channels takes
       less than 1ms so there's a full 22ms to spare after GM. */
    QBENCHMARK
    {
        // This is slower than plain write() because UA has to dig out each
        // Intensity-enabled channel from its internal QSet.
        ua->setGMValue(127);
    }

    for (i = 0; i < int(512 * 4); i++)
        QCOMPARE(ua->postGMValues()->at(i), char(100));
}

void UniverseArray_Test::writeEfficiency()
{
    UniverseArray* ua = new UniverseArray(512 * 4);
    ua->setGMValue(127);

    int i;
    /* This applies 50%(127) Grand Master to ALL channels in all universes.
       I'm not really sure what kinds of figures to expect here, since this
       is just one part in the overall processor load. Typically I get ~0.15ms
       on an Intel Core 2 E6550@2.33GHz, which looks plausible to me:
       DMX frame interval is 1/44Hz =~ 23ms. Applying GM to ALL channels takes
       less than 1ms so there's a full 22ms to spare after GM. */
    QBENCHMARK
    {
        for (i = 0; i < int(512 * 4); i++)
            ua->write(i, 200, QLCChannel::Intensity);
    }

    for (i = 0; i < int(512 * 4); i++)
        QCOMPARE(ua->postGMValues()->at(i), char(100));
}

QTEST_APPLESS_MAIN(UniverseArray_Test)
