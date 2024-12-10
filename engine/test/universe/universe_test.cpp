/*
  Q Light Controller - Unit test
  universe_test.cpp

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

#include <QtTest>
#include <sys/time.h>

#include "universe_test.h"

#define protected public
#include "universe.h"
#undef protected

#include "grandmaster.h"

void Universe_Test::init()
{
    m_gm = new GrandMaster(this);
    m_uni = new Universe(0, m_gm, this);
}

void Universe_Test::cleanup()
{
    delete m_uni; m_uni = 0;
    delete m_gm; m_gm = 0;
}

void Universe_Test::initial()
{
    QCOMPARE(m_uni->name(), QString("Universe 1"));
    QCOMPARE(m_uni->id(), quint32(0));
    QCOMPARE(m_uni->usedChannels(), ushort(0));
    QCOMPARE(m_uni->totalChannels(), ushort(0));
    QCOMPARE(m_uni->hasChanged(), false);
    QCOMPARE(m_uni->passthrough(), false);
    QVERIFY(m_uni->inputPatch() == NULL);
    QVERIFY(m_uni->outputPatch(0) == NULL);
    QVERIFY(m_uni->feedbackPatch() == NULL);
    QVERIFY(m_uni->intensityChannels().isEmpty());

    QByteArray const preGM = m_uni->preGMValues();

    QCOMPARE(preGM.length(), 512);

    QByteArray const *postGM = m_uni->postGMValues();
    QVERIFY(postGM != NULL);
    QCOMPARE(postGM->length(), 512);

    for (ushort i = 0; i < 512; ++i)
    {
        QVERIFY(m_uni->channelCapabilities(i) == Universe::Undefined);
        QCOMPARE(int(preGM.at(i)), 0);
        QCOMPARE(int(postGM->at(i)), 0);
    }
}

void Universe_Test::channelCapabilities()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Intensity);
    m_uni->setChannelCapability(2, QLCChannel::Pan);
    m_uni->setChannelCapability(3, QLCChannel::Tilt, Universe::HTP);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);

    QVERIFY(m_uni->channelCapabilities(0) == (Universe::Intensity|Universe::HTP));
    QVERIFY(m_uni->channelCapabilities(1) == (Universe::Intensity|Universe::HTP));
    QVERIFY(m_uni->channelCapabilities(2) == Universe::LTP);
    QVERIFY(m_uni->channelCapabilities(3) == Universe::HTP);
    QVERIFY(m_uni->channelCapabilities(4) == (Universe::Intensity|Universe::HTP));
    QCOMPARE(m_uni->totalChannels(), ushort(5));
}

void Universe_Test::blendModes()
{
    QVERIFY(Universe::blendModeToString(Universe::NormalBlend) == "Normal");
    QVERIFY(Universe::blendModeToString(Universe::MaskBlend) == "Mask");
    QVERIFY(Universe::blendModeToString(Universe::AdditiveBlend) == "Additive");
    QVERIFY(Universe::blendModeToString(Universe::SubtractiveBlend) == "Subtractive");

    QVERIFY(Universe::stringToBlendMode("Foo") == Universe::NormalBlend);
    QVERIFY(Universe::stringToBlendMode("Normal") == Universe::NormalBlend);
    QVERIFY(Universe::stringToBlendMode("Mask") == Universe::MaskBlend);
    QVERIFY(Universe::stringToBlendMode("Additive") == Universe::AdditiveBlend);
    QVERIFY(Universe::stringToBlendMode("Subtractive") == Universe::SubtractiveBlend);

    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);
    m_uni->setChannelCapability(9, QLCChannel::Intensity);
    m_uni->setChannelCapability(11, QLCChannel::Intensity);

    QVERIFY(m_uni->write(0, 255) == true);
    QVERIFY(m_uni->write(4, 128) == true);
    QVERIFY(m_uni->write(9, 100) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(128));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(100));
    QCOMPARE(quint8(m_uni->postGMValues()->at(11)), quint8(0));

    /* check masking on 0 remains 0 */
    QVERIFY(m_uni->writeBlended(11, 128, 1, Universe::MaskBlend) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(11)), quint8(0));

    /* check 180 masked on 128 gets halved */
    QVERIFY(m_uni->writeBlended(4, 180, 1, Universe::MaskBlend) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(90));

    /* chek adding 50 to 100 is actually 150 */
    QVERIFY(m_uni->writeBlended(9, 50, 1, Universe::AdditiveBlend) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(150));

    /* chek subtracting 55 to 255 is actually 200 */
    QVERIFY(m_uni->writeBlended(0, 55, 1, Universe::SubtractiveBlend) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(200));

    QVERIFY(m_uni->writeBlended(0, 255, 1, Universe::SubtractiveBlend) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    /* check an unknown blend mode */
    QVERIFY(m_uni->writeBlended(9, 255, 1, Universe::BlendMode(42)) == false);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(150));
}

void Universe_Test::grandMasterIntensityReduce()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Intensity);
    m_uni->setChannelCapability(2, QLCChannel::Pan);
    m_uni->setChannelCapability(3, QLCChannel::Tilt);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);
    QCOMPARE(m_uni->usedChannels(), ushort(0));
    QCOMPARE(m_uni->totalChannels(), ushort(5));

    m_uni->write(0, 10);
    m_uni->write(1, 20);
    m_uni->write(2, 30);
    m_uni->write(3, 40);
    m_uni->write(4, 50);

    m_gm->setValue(63);
    QCOMPARE(int(m_uni->postGMValues()->at(0)), int(2));
    QCOMPARE(int(m_uni->postGMValues()->at(1)), int(5));
    QCOMPARE(int(m_uni->postGMValues()->at(2)), int(30));
    QCOMPARE(int(m_uni->postGMValues()->at(3)), int(40));
    QCOMPARE(int(m_uni->postGMValues()->at(4)), int(12));
    QCOMPARE(m_uni->usedChannels(), ushort(5));
}

void Universe_Test::grandMasterIntensityLimit()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Intensity);
    m_uni->setChannelCapability(2, QLCChannel::Pan);
    m_uni->setChannelCapability(3, QLCChannel::Tilt);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);
    QCOMPARE(m_uni->usedChannels(), ushort(0));
    QCOMPARE(m_uni->totalChannels(), ushort(5));

    m_uni->write(0, 10);
    m_uni->write(1, 20);
    m_uni->write(2, 30);
    m_uni->write(3, 40);
    m_uni->write(4, 50);

    m_gm->setValueMode(GrandMaster::Limit);

    m_gm->setValue(63);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(10));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(20));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(30));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(40));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(50));

    m_gm->setValue(5);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(30));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(40));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(5));
    QCOMPARE(m_uni->usedChannels(), ushort(5));
}

void Universe_Test::grandMasterAllChannelsReduce()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Intensity);
    m_uni->setChannelCapability(2, QLCChannel::Pan);
    m_uni->setChannelCapability(3, QLCChannel::Tilt);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);
    QCOMPARE(m_uni->usedChannels(), ushort(0));
    QCOMPARE(m_uni->totalChannels(), ushort(5));

    m_uni->write(0, 10);
    m_uni->write(1, 20);
    m_uni->write(2, 30);
    m_uni->write(3, 40);
    m_uni->write(4, 50);

    m_gm->setChannelMode(GrandMaster::AllChannels);

    m_gm->setValue(63);
    QCOMPARE(int(m_uni->postGMValues()->at(0)), int(2));
    QCOMPARE(int(m_uni->postGMValues()->at(1)), int(5));
    QCOMPARE(int(m_uni->postGMValues()->at(2)), int(7));
    QCOMPARE(int(m_uni->postGMValues()->at(3)), int(10));
    QCOMPARE(int(m_uni->postGMValues()->at(4)), int(12));
    QCOMPARE(m_uni->usedChannels(), ushort(5));
}

void Universe_Test::grandMasterAllChannelsLimit()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Intensity);
    m_uni->setChannelCapability(2, QLCChannel::Pan);
    m_uni->setChannelCapability(3, QLCChannel::Tilt);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);
    QCOMPARE(m_uni->usedChannels(), ushort(0));
    QCOMPARE(m_uni->totalChannels(), ushort(5));

    m_uni->write(0, 10);
    m_uni->write(1, 20);
    m_uni->write(2, 30);
    m_uni->write(3, 40);
    m_uni->write(4, 50);

    m_gm->setChannelMode(GrandMaster::AllChannels);
    m_gm->setValueMode(GrandMaster::Limit);

    m_gm->setValue(63);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(10));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(20));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(30));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(40));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(50));

    m_gm->setValue(5);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(5));
    QCOMPARE(m_uni->usedChannels(), ushort(5));
}

void Universe_Test::applyGM()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Pan);

    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i));
    }

    m_gm->setValue(127);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i/2));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i));
    }

    m_gm->setChannelMode(GrandMaster::AllChannels);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i/2));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i/2));
    }

    m_gm->setValueMode(GrandMaster::Limit);
    m_gm->setChannelMode(GrandMaster::Intensity);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i < 127 ? i : 127));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i));
    }

    m_gm->setChannelMode(GrandMaster::AllChannels);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i < 127 ? i : 127));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i < 127 ? i : 127));
    }
}

void Universe_Test::write()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);
    m_uni->setChannelCapability(9, QLCChannel::Intensity);
    m_uni->setChannelCapability(UNIVERSE_SIZE - 1, QLCChannel::Intensity);

    QVERIFY(m_uni->write(UNIVERSE_SIZE - 1, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(UNIVERSE_SIZE - 1)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    QVERIFY(m_uni->write(9, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(UNIVERSE_SIZE - 1)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    QVERIFY(m_uni->write(0, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(UNIVERSE_SIZE - 1)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(255));

    m_gm->setValue(127);
    QCOMPARE(quint8(m_uni->postGMValues()->at(UNIVERSE_SIZE - 1)), quint8(127));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(127));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(127));

    QVERIFY(m_uni->write(4, 200) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(UNIVERSE_SIZE - 1)), quint8(127));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(127));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(100));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(127));
}

void Universe_Test::writeRelative()
{
    // 127 == 0
    QVERIFY(m_uni->writeRelative(9, 127, 1) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    // 255 == +128
    QVERIFY(m_uni->writeRelative(9, 255, 1) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(128));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    // 0 == -127
    QVERIFY(m_uni->writeRelative(9, 0, 1) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(1));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 85) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(85));

    QVERIFY(m_uni->writeRelative(9, 117, 1) == true);

    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(75));
    QVERIFY(m_uni->write(9, 65) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(65));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));
    QVERIFY(m_uni->writeRelative(9, 255, 1) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 0) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QVERIFY(m_uni->writeRelative(9, 0, 1) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));


    m_uni->reset();

    // write 4887 = 19*256+23
    QVERIFY(m_uni->write(9, 19) == true);
    QVERIFY(m_uni->write(10, 23) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(19));
    QCOMPARE(quint8(m_uni->postGMValues()->at(10)), quint8(23));

    // write relative 30067 = 117*256+115
    QVERIFY(m_uni->writeRelative(9, 30067, 2) == true);

    // expect 2442 = 9*256+138
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(9));
    QCOMPARE(quint8(m_uni->postGMValues()->at(10)), quint8(138));



    // write 4887 = 19*256+23
    QVERIFY(m_uni->write(9, 19) == true);
    QVERIFY(m_uni->write(10, 23) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(19));
    QCOMPARE(quint8(m_uni->postGMValues()->at(10)), quint8(23));

    // write relative 27507 = 107*256+115
    QVERIFY(m_uni->writeRelative(9, 27507, 2) == true);

    // expect 0 (due to clamping)
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(10)), quint8(0));



    // write 48663 = 190*256+23
    QVERIFY(m_uni->write(9, 190) == true);
    QVERIFY(m_uni->write(10, 23) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(190));
    QCOMPARE(quint8(m_uni->postGMValues()->at(10)), quint8(23));

    // write relative 58995 = 230*256+115
    QVERIFY(m_uni->writeRelative(9, 58995, 2) == true);

    // expect 65535 = 255*256+255 (due to clamping)
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(10)), quint8(255));



}

void Universe_Test::reset()
{
    int i;

    for (i = 0; i < 512; i++)
        m_uni->setChannelCapability(i, QLCChannel::Intensity);

    for (i = 0; i < 128; i++)
    {
        m_uni->write(i, 200);
        QCOMPARE(quint8(m_uni->postGMValues()->at(i)), quint8(200));
    }

    // Reset channels 10-127 (512 shouldn't cause a crash)
    m_uni->reset(10, 512);
    for (i = 0; i < 10; i++)
        QCOMPARE(quint8(m_uni->postGMValues()->at(i)), quint8(200));
    for (i = 10; i < 128; i++)
        QCOMPARE(int(m_uni->postGMValues()->at(i)), 0);

    // Reset all
    m_uni->reset();
    for (i = 0; i < 128; i++)
        QCOMPARE((int)m_uni->postGMValues()->at(i), 0);
}

void Universe_Test::loadEmpty()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Universe");
    xmlWriter.writeAttribute("Name", "Universe 123");
    //xmlWriter.writeAttribute("ID", "1");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(m_uni->loadXML(xmlReader, 0, 0) == true);
    QCOMPARE(m_uni->name(), QString("Universe 123"));
    //QCOMPARE(m_uni->id(), 1U);
    QCOMPARE(m_uni->passthrough(), false);
}

void Universe_Test::loadPassthroughTrue()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Universe");
    xmlWriter.writeAttribute("Name", "Universe 123");
    //xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("Passthrough", "True");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(m_uni->loadXML(xmlReader, 0, 0) == true);
    QCOMPARE(m_uni->name(), QString("Universe 123"));
    //QCOMPARE(m_uni->id(), 1U);
    QCOMPARE(m_uni->passthrough(), true);
}

void Universe_Test::loadPassthrough1()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Universe");
    xmlWriter.writeAttribute("Name", "Universe 123");
    //xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("Passthrough", "1");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(m_uni->loadXML(xmlReader, 0, 0) == true);
    QCOMPARE(m_uni->name(), QString("Universe 123"));
    //QCOMPARE(m_uni->id(), 1U);
    QCOMPARE(m_uni->passthrough(), true);
}

void Universe_Test::loadWrong()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("U");
    xmlWriter.writeAttribute("Name", "Universe 123");
    //xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("Passthrough", "1");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(m_uni->loadXML(xmlReader, 0, 0) == false);
}

void Universe_Test::loadPassthroughFalse()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Universe");
    xmlWriter.writeAttribute("Name", "Universe 123");
    //xmlWriter.writeAttribute("ID", "1");
    xmlWriter.writeAttribute("Passthrough", "False");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(m_uni->loadXML(xmlReader, 0, 0) == true);
    QCOMPARE(m_uni->name(), QString("Universe 123"));
    //QCOMPARE(m_uni->id(), 1U);
    QCOMPARE(m_uni->passthrough(), false);
}

void Universe_Test::saveEmpty()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    m_uni->setName("Universe 123");
    m_uni->setID(1);

    QVERIFY(m_uni->saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Universe"));
    QCOMPARE(xmlReader.attributes().value("Name").toString(), QString("Universe 123"));
    QCOMPARE(xmlReader.attributes().value("ID").toString(), QString("1"));
    QCOMPARE(xmlReader.attributes().hasAttribute("Passthrough"), false);
}

void Universe_Test::savePasthroughTrue()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    m_uni->setName("Universe 123");
    m_uni->setID(1);
    m_uni->setPassthrough(true);

    QVERIFY(m_uni->saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Universe"));
    QCOMPARE(xmlReader.attributes().value("Name").toString(), QString("Universe 123"));
    QCOMPARE(xmlReader.attributes().value("ID").toString(), QString("1"));
    QCOMPARE(xmlReader.attributes().value("Passthrough").toString(), QString("True"));
}

void Universe_Test::setGMValueEfficiency()
{
    int i;

    for (i = 0; i < 512; i++)
        m_uni->setChannelCapability(i, QLCChannel::Intensity);

    for (i = 0; i < 512; i++)
        m_uni->write(i, 200);

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
        m_gm->setValue(127);
    }

    for (i = 0; i < 512; i++)
        QCOMPARE(int(m_uni->postGMValues()->at(i)), int(100));
}

void Universe_Test::writeEfficiency()
{
    m_gm->setValue(127);

    int i;
    for (i = 0; i < 512; i++)
        m_uni->setChannelCapability(i, QLCChannel::Intensity);

    QBENCHMARK
    {
        for (i = 0; i < 512; i++)
            m_uni->write(i, 200);
    }

    for (i = 0; i < 512; i++)
        QCOMPARE(int(m_uni->postGMValues()->at(i)), int(100));
}

void Universe_Test::hasChangedEfficiency()
{
    for (int i = 0; i < 512; i++)
    {
        m_uni->write(i, 200);
        QCOMPARE(m_uni->hasChanged(), true);
    }

    QBENCHMARK
    {
        for (int i = 0; i < 512; i++)
        {
            m_uni->write(i, 200);
            m_uni->hasChanged();
        }
    }
}

void Universe_Test::hasNotChangedEfficiency()
{
    m_uni->write(UNIVERSE_SIZE - 1, 200);
    m_uni->hasChanged();
    QCOMPARE(m_uni->hasChanged(), false);

    QBENCHMARK
    {
        for (int i = 0; i < 512; i++)
        {
            m_uni->hasChanged();
        }
    }
}

void Universe_Test::zeroIntensityChannelsEfficiency()
{
    m_gm->setValue(255);
    int i;

    for (i = 0; i < 512; i++)
        m_uni->setChannelCapability(i, QLCChannel::Intensity);

    for (i = 0; i < 512; i++)
        m_uni->write(i, 200);

    QBENCHMARK
    {
        m_uni->zeroIntensityChannels();
    }

    for (i = 0; i < 512; i++)
        QCOMPARE(int(m_uni->postGMValues()->at(i)), int(0));
}

void Universe_Test::zeroIntensityChannelsEfficiency2()
{
    int i;

    for (i = 0; i < 512; i++)
    {
        if (i % 2)
            m_uni->setChannelCapability(i, QLCChannel::Intensity);
        else
            m_uni->setChannelCapability(i, QLCChannel::Shutter);

        m_uni->write(i, 200);
    }

    QBENCHMARK
    {
        m_uni->zeroIntensityChannels();
    }

    for (i = 0; i < 512; i++)
    {
        if (i % 2)
            QCOMPARE(int(m_uni->postGMValues()->at(i)), int(0));
        else
            QCOMPARE(quint8(m_uni->postGMValues()->at(i)), quint8(200));
    }
}

QTEST_APPLESS_MAIN(Universe_Test)
