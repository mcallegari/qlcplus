/*
  Q Light Controller
  rgbtext_test.cpp

  Copyright (C) Heikki Junnila

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

#include <QDomDocument>
#include <QFontMetrics>
#include <QDomElement>
#include <QtTest>

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

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    doc.appendChild(root);

    QVERIFY(text.saveXML(&doc, &root) == true);
    root = root.firstChild().toElement();
    QCOMPARE(root.tagName(), QString("Algorithm"));
    QCOMPARE(root.attribute("Type"), QString("Text"));

    int content = 0, font = 0, ani = 0, offset = 0;

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Content")
        {
            QCOMPARE(tag.text(), QString("Foo"));
            content++;
        }
        else if (tag.tagName() == "Font")
        {
            QCOMPARE(tag.text(), text.font().toString());
            font++;
        }
        else if (tag.tagName() == "Animation")
        {
            QCOMPARE(tag.text(), QString("Vertical"));
            ani++;
        }
        else if (tag.tagName() == "Offset")
        {
            QCOMPARE(tag.attribute("X").toInt(), 1);
            QCOMPARE(tag.attribute("Y").toInt(), 2);
            offset++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(content, 1);
    QCOMPARE(font, 1);
    QCOMPARE(ani, 1);
    QCOMPARE(offset, 1);
}

void RGBText_Test::load()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Algorithm");
    root.setAttribute("Type", "Text");
    doc.appendChild(root);

    QDomElement content = doc.createElement("Content");
    QDomText contentText = doc.createTextNode("Foobar");
    content.appendChild(contentText);
    root.appendChild(content);

    QFont fn;
    fn.setPixelSize(1);
    QDomElement font = doc.createElement("Font");
    QDomText fontText = doc.createTextNode(fn.toString());
    font.appendChild(fontText);
    root.appendChild(font);

    QDomElement ani = doc.createElement("Animation");
    QDomText aniText = doc.createTextNode("Horizontal");
    ani.appendChild(aniText);
    root.appendChild(ani);

    QDomElement offset = doc.createElement("Offset");
    offset.setAttribute("X", 10);
    offset.setAttribute("Y", -20);
    root.appendChild(offset);

    // Extra crap
    QDomElement foo = doc.createElement("Foobar");
    root.appendChild(foo);

    RGBText text(m_doc);
    QVERIFY(text.loadXML(root) == true);
    QCOMPARE(text.text(), QString("Foobar"));
    QCOMPARE(text.font(), fn);
    QCOMPARE(text.animationStyle(), RGBText::Horizontal);
    QCOMPARE(text.xOffset(), 10);
    QCOMPARE(text.yOffset(), -20);

    fontText.setData("a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z");
    QVERIFY(text.loadXML(root) == true);
    QCOMPARE(text.font(), fn); // Invalid font is ignored by loadXML()

    offset.setAttribute("X", "Foo");
    QVERIFY(text.loadXML(root) == true);
    QCOMPARE(text.xOffset(), 10); // Invalid offset is ignored by loadXML()

    offset.setAttribute("X", "20");
    offset.setAttribute("Y", "@£$");
    QVERIFY(text.loadXML(root) == true);
    QCOMPARE(text.xOffset(), 20); // Valid X offset
    QCOMPARE(text.yOffset(), -20); // Invalid offset is ignored by loadXML()

    QVERIFY(text.loadXML(foo) == false); // Invalid root node

    root.setAttribute("Type", "Script"); // Invalid type
    QVERIFY(text.loadXML(root) == false);
}

void RGBText_Test::staticLetters()
{
    RGBText text(m_doc);
    text.setText("QLC");
    text.setAnimationStyle(RGBText::StaticLetters);
    QCOMPARE(text.rgbMapStepCount(QSize()), 3); // Q, L, C

    QRgb color(0xFFFFFFFF);

    // Since fonts and their rendering differs from installation to installation,
    // these tests are here only to check that nothing crashes. The end result is
    // more or less OS, platform, HW and SW dependent and testing individual pixels
    // would thus be rather pointless.
    RGBMap map = text.rgbMap(QSize(10, 10), color, 0);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
        QCOMPARE(map[i].size(), 10);

    map = text.rgbMap(QSize(10, 10), color, 1);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
        QCOMPARE(map[i].size(), 10);

    map = text.rgbMap(QSize(10, 10), color, 2);
    QCOMPARE(map.size(), 10);
    for (int i = 0; i < 10; i++)
        QCOMPARE(map[i].size(), 10);

    // Invalid step
    map = text.rgbMap(QSize(10, 10), color, 3);
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
    QCOMPARE(text.rgbMapStepCount(QSize()), fm.width("QLC"));

    // Since fonts and their rendering differs from installation to installation,
    // these tests are here only to check that nothing crashes. The end result is
    // more or less OS, platform, HW and SW dependent and testing individual pixels
    // would thus be rather pointless.
    for (int i = 0; i < fm.width("QLC"); i++)
    {
        RGBMap map = text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), i);
        QCOMPARE(map.size(), 10);
        for (int y = 0; y < 10; y++)
            QCOMPARE(map[y].size(), 10);
    }

    // Invalid step
    RGBMap map = text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), fm.width("QLC"));
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
        RGBMap map = text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), i);
        QCOMPARE(map.size(), 10);
        for (int y = 0; y < 10; y++)
            QCOMPARE(map[y].size(), 10);
    }

    // Invalid step
    RGBMap map = text.rgbMap(QSize(10, 10), QRgb(0xFFFFFFFF), fm.ascent() * 4);
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

QTEST_MAIN(RGBText_Test)
