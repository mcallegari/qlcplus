#include <QtTest>

#define protected public
#define private public
#include "audiotriggerwidget.h"
#undef private
#undef protected

#include "audiotriggerwidget_test.h"

void AudioTriggerWidget_Test::basics()
{
    AudioTriggerWidget w;
    w.setBarsNumber(2);
    w.setMaxFrequency(1000);
    QCOMPARE(w.m_barsNumber, 2);
    QCOMPARE(w.m_maxFrequency, 1000);
    QVERIFY(w.m_spectrumBands != NULL);
}

void AudioTriggerWidget_Test::display()
{
    AudioTriggerWidget w;
    w.setBarsNumber(2);
    w.m_spectrumHeight = 200;
    double data[2] = { 1.0, 2.0 };
    w.displaySpectrum(data, 2.0, 0x7FFF);
    QCOMPARE(w.m_volumeBarHeight, quint32(200));
    QCOMPARE(uchar(w.getUcharVolume()), uchar(254));
    QCOMPARE(uchar(w.getUcharBand(1)), uchar(254));
}

QTEST_MAIN(AudioTriggerWidget_Test)
