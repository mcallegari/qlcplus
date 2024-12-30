/*
  Q Light Controller Plus - Unit tests
  rgbtext_test.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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
#include <QFontMetrics>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#define private public
#include "rgbtext_test.h"
#include "rgbtext.h"
#undef private

#include "doc.h"

void RGBText_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void RGBText_Test::cleanupTestCase()
{
    delete m_doc;
}

void RGBText_Test::initial()
{
    RGBText text(m_doc);
    QCOMPARE(text.text(), QString(" Q LIGHT CONTROLLER + "));
    QCOMPARE(text.animationStyle(), RGBText::Horizontal);
    QCOMPARE(text.xOffset(), 0);
    QCOMPARE(text.yOffset(), 0);
    QCOMPARE(text.author(), QString("Heikki Junnila"));
    QCOMPARE(text.apiVersion(), 1);
    QCOMPARE(text.type(), RGBAlgorithm::Text);
    QCOMPARE(text.name(), QString("Text"));
}

void RGBText_Test::text()
{
    RGBText text(m_doc);
    text.setText("Foo");
    QCOMPARE(text.text(), QString("Foo"));
}

void RGBText_Test::font()
{
    RGBText text(m_doc);
    QFont font(text.font());
    font.setPixelSize(font.pixelSize() + 5);
    text.setFont(font);
    QCOMPARE(text.font(), font);
}

void RGBText_Test::animationStyle()
{
    QCOMPARE(RGBText::animationStyleToString(RGBText::Vertical), QString("Vertical"));
    QCOMPARE(RGBText::animationStyleToString(RGBText::Horizontal), QString("Horizontal"));
    QCOMPARE(RGBText::animationStyleToString(RGBText::StaticLetters), QString("Letters"));
    QCOMPARE(RGBText::animationStyleToString(RGBText::AnimationStyle(12345)), QString("Letters"));

    QCOMPARE(RGBText::stringToAnimationStyle("Vertical"), RGBText::Vertical);
    QCOMPARE(RGBText::stringToAnimationStyle("Horizontal"), RGBText::Horizontal);
    QCOMPARE(RGBText::stringToAnimationStyle("Letters"), RGBText::StaticLetters);
    QCOMPARE(RGBText::stringToAnimationStyle("Foobar"), RGBText::StaticLetters);

    QStringList styles = RGBText::animationStyles();
    QCOMPARE(styles.size(), 3);
    QVERIFY(styles.contains("Vertical") == true);
    QVERIFY(styles.contains("Horizontal") == true);
    QVERIFY(styles.contains("Letters") == true);

    RGBText text(m_doc);
    text.setAnimationStyle(RGBText::Vertical);
    QCOMPARE(text.animationStyle(), RGBText::Vertical);

    text.setAnimationStyle(RGBText::Horizontal);
    QCOMPARE(text.animationStyle(), RGBText::Horizontal);

    text.setAnimationStyle(RGBText::StaticLetters);
    QCOMPARE(text.animationStyle(), RGBText::StaticLetters);

    text.setAnimationStyle(RGBText::AnimationStyle(31337));
    QCOMPARE(text.animationStyle(), RGBText::StaticLetters);

    text.setAnimationStyle(RGBText::AnimationStyle(-1));
    QCOMPARE(text.animationStyle(), RGBText::StaticLetters);
}

void RGBText_Test::offset()
{
    RGBText text(m_doc);
    text.setXOffset(5);
    QCOMPARE(text.xOffset(), 5);

    text.setYOffset(100000);
    QCOMPARE(text.yOffset(), 100000);
}

void RGBText_Test::clone()
{
    RGBText text(m_doc);
    text.setText("Foo");
    QFont font(text.font());
    font.setPixelSize(font.pixelSize() + 5);
    text.setFont(font);
    text.setAnimationStyle(RGBText::Vertical);
    text.setXOffset(1);
    text.setYOffset(2);

    RGBText text2 = text;
    QCOMPARE(text2.text(), QString("Foo"));
    QCOMPARE(text2.font(), text.font());
    QCOMPARE(text2.animationStyle(), RGBText::Vertical);
    QCOMPARE(text2.xOffset(), 1);
    QCOMPARE(text2.yOffset(), 2);

    RGBAlgorithm* algo = text.clone();
    QCOMPARE(algo->type(), RGBAlgorithm::Text);
    RGBText* text3 = static_cast<RGBText*> (algo);
    QVERIFY(text3 != NULL);
    QCOMPARE(text3->text(), QString("Foo"));
    QCOMPARE(text3->font(), text.font());
    QCOMPARE(text3->animationStyle(), RGBText::Vertical);
    QCOMPARE(text3->xOffset(), 1);
    QCOMPARE(text3->yOffset(), 2);

    delete algo;
}

void RGBText_Test::save()
{
    RGBText text(m_doc);
    text.setText("Foo");
    text.setAnimationStyle(RGBText::Vertical);
    text.setXOffset(1);
    text.setYOffset(2);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(text.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Algorithm"));
    QCOMPARE(xmlReader.attributes().value("Type").toString(), QString("Text"));

    int content = 0, font = 0, ani = 0, offset = 0;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Content")
        {
            QCOMPARE(xmlReader.readElementText(), QString("Foo"));
            content++;
        }
        else if (xmlReader.name().toString() == "Font")
        {
            QCOMPARE(xmlReader.readElementText(), text.font().toString());
            font++;
        }
        else if (xmlReader.name().toString() == "Animation")
        {
            QCOMPARE(xmlReader.readElementText(), QString("Vertical"));
            ani++;
        }
        else if (xmlReader.name().toString() == "Offset")
        {
            QCOMPARE(xmlReader.attributes().value("X").toString().toInt(), 1);
            QCOMPARE(xmlReader.attributes().value("Y").toString().toInt(), 2);
            offset++;
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }

    QCOMPARE(content, 1);
    QCOMPARE(font, 1);
    QCOMPARE(ani, 1);
    QCOMPARE(offset, 1);
}

void RGBText_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Algorithm");
    xmlWriter.writeAttribute("Type", "Text");

    xmlWriter.writeTextElement("Content", "Foobar");

    QFont fn;
    fn.setPixelSize(1);
    xmlWriter.writeTextElement("Font", fn.toString());
    xmlWriter.writeTextElement("Animation", "Horizontal");

    xmlWriter.writeStartElement("Offset");
    xmlWriter.writeAttribute("X", "10");
    xmlWriter.writeAttribute("Y", "-20");
    xmlWriter.writeEndElement();

    // Extra crap
    xmlWriter.writeStartElement("Foobar");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    RGBText text(m_doc);
    QVERIFY(text.loadXML(xmlReader) == true);
    QCOMPARE(text.text(), QString("Foobar"));
    QCOMPARE(text.font(), fn);
    QCOMPARE(text.animationStyle(), RGBText::Horizontal);
    QCOMPARE(text.xOffset(), 10);
    QCOMPARE(text.yOffset(), -20);

    buffer.close();
    QByteArray bData = buffer.data();
    bData.replace(fn.toString().toUtf8(), "a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(text.loadXML(xmlReader) == true);
    QCOMPARE(text.font(), fn); // Invalid font is ignored by loadXML()

    buffer.close();
    bData = buffer.data();
    bData.replace("X=\"10\"", "X=\"Foo\"");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(text.loadXML(xmlReader) == true);
    QCOMPARE(text.xOffset(), 10); // Invalid offset is ignored by loadXML()

    buffer.close();
    bData = buffer.data();
    bData.replace("X=\"Foo\"", "X=\"20\"");
    bData.replace("Y=\"-20\"", "Y=\"@£$\"");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(text.loadXML(xmlReader) == true);
    QCOMPARE(text.xOffset(), 20); // Valid X offset
    QCOMPARE(text.yOffset(), -20); // Invalid offset is ignored by loadXML()

    buffer.close();
    buffer.setData(QByteArray());
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    xmlWriter.setDevice(&buffer);
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.seek(0);

    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(text.loadXML(xmlReader) == false); // Invalid root node

    buffer.close();
    buffer.setData(QByteArray());
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    xmlWriter.setDevice(&buffer);
    xmlWriter.writeStartElement("Algorithm");
    xmlWriter.writeAttribute("Type", "Script"); // Invalid type
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.seek(0);

    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(text.loadXML(xmlReader) == false);
}

void RGBText_Test::staticLetters()
{
    RGBText text(m_doc);
    text.setText("QLC");
    text.setAnimationStyle(RGBText::StaticLetters);
    QCOMPARE(text.rgbMapStepCount(QSize()), 3); // Q, L, C

    QRgb color(0xFFFFFFFF);
    RGBMap map;

    // Since fonts and their rendering differs from installation to installation,
    // these tests are here only to check that nothing crashes. The end result is
    // more or less OS, platform, HW and SW dependent and testing individual pixels
    // would thus be rather pointless.
    text.rgbMap(QSize(10, 10), color, 0, map);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
        QCOMPARE(map[i].size(), 10);

    text.rgbMap(QSize(10, 10), color, 1, map);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
        QCOMPARE(map[i].size(), 10);

    text.rgbMap(QSize(10, 10), color, 2, map);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
        QCOMPARE(map[i].size(), 10);

    // Invalid step
    text.rgbMap(QSize(10, 10), color, 3, map);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
    {
        QCOMPARE(map[i].size(), 10);
        for (int j = 0; j < 10; j++)
        {
            QCOMPARE(map[i][j], QColor(Qt::black).rgb());
        }
    }
}

void RGBText_Test::horizontalScroll()
{
    RGBText text(m_doc);
    text.setText("QLC");
    text.setAnimationStyle(RGBText::Horizontal);

    QFontMetrics fm(text.font());
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
    QCOMPARE(text.rgbMapStepCount(QSize()), fm.width("QLC"));
#else
    QCOMPARE(text.rgbMapStepCount(QSize()), fm.horizontalAdvance("QLC"));
#endif

    // Since fonts and their rendering differs from installation to installation,
    // these tests are here only to check that nothing crashes. The end result is
    // more or less OS, platform, HW and SW dependent and testing individual pixels
    // would thus be rather pointless.
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
    for (int i = 0; i < fm.width("QLC"); i++)
#else
    for (int i = 0; i < fm.horizontalAdvance("QLC"); i++)
#endif
    {
        RGBMap map;
        text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), i, map);
        QCOMPARE(map.size(), 10);
        for (int y = 0; y < 10; y++)
            QCOMPARE(map[y].size(), 10);
    }

    RGBMap map;
    // Invalid step
#if (QT_VERSION < QT_VERSION_CHECK(5, 13, 0))
    text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), fm.width("QLC"), map);
#else
    text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), fm.horizontalAdvance("QLC"), map);
#endif
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
    {
        QCOMPARE(map[i].size(), 10);
        for (int j = 0; j < 10; j++)
        {
            QCOMPARE(map[i][j], QRgb(0));
        }
    }
}

void RGBText_Test::verticalScroll()
{
    RGBText text(m_doc);
    text.setText("QLC");
    text.setAnimationStyle(RGBText::Vertical);

    QFontMetrics fm(text.font());
    QCOMPARE(text.rgbMapStepCount(QSize()), fm.ascent() * 3); // Q, L, C

    // Since fonts and their rendering differs from installation to installation,
    // these tests are here only to check that nothing crashes. The end result is
    // more or less OS, platform, HW and SW dependent and testing individual pixels
    // would thus be rather pointless.
    for (int i = 0; i < fm.ascent() * 3; i++)
    {
        RGBMap map;
        text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), i, map);
        QCOMPARE(map.size(), 10);
        for (int y = 0; y < 10; y++)
            QCOMPARE(map[y].size(), 10);
    }

    // Invalid step
    RGBMap map;
    text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), fm.ascent() * 4, map);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
    {
        QCOMPARE(map[i].size(), 10);
        for (int j = 0; j < 10; j++)
        {
            QCOMPARE(map[i][j], QRgb(0));
        }
    }
}

void RGBText_Test::unused()
{
    RGBText text(m_doc);
    QVector<uint> colors;
    text.rgbMapSetColors(colors);
    QCOMPARE(text.rgbMapGetColors().isEmpty(), true);
}

QTEST_MAIN(RGBText_Test)
