/*
  Q Light Controller - Unit tests
  qlcinputchannel_test.cpp

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

#include <QtTest>
#include <QtXml>

#include "qlcinputchannel_test.h"
#include "qlcinputchannel.h"

void QLCInputChannel_Test::types()
{
    QStringList list(QLCInputChannel::types());
    QVERIFY(list.size() == 6);
    QVERIFY(list.contains(KXMLQLCInputChannelButton));
    QVERIFY(list.contains(KXMLQLCInputChannelSlider));
    QVERIFY(list.contains(KXMLQLCInputChannelKnob));
    QVERIFY(list.contains(KXMLQLCInputChannelPageUp));
    QVERIFY(list.contains(KXMLQLCInputChannelPageDown));
    QVERIFY(list.contains(KXMLQLCInputChannelPageSet));
}

void QLCInputChannel_Test::type()
{
    QLCInputChannel ch;
    QVERIFY(ch.type() == QLCInputChannel::Button);

    ch.setType(QLCInputChannel::Slider);
    QVERIFY(ch.type() == QLCInputChannel::Slider);

    ch.setType(QLCInputChannel::Button);
    QVERIFY(ch.type() == QLCInputChannel::Button);

    ch.setType(QLCInputChannel::Knob);
    QVERIFY(ch.type() == QLCInputChannel::Knob);

    ch.setType(QLCInputChannel::NextPage);
    QVERIFY(ch.type() == QLCInputChannel::NextPage);

    ch.setType(QLCInputChannel::PrevPage);
    QVERIFY(ch.type() == QLCInputChannel::PrevPage);

    ch.setType(QLCInputChannel::PageSet);
    QVERIFY(ch.type() == QLCInputChannel::PageSet);
}

void QLCInputChannel_Test::typeToString()
{
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Button),
             QString(KXMLQLCInputChannelButton));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Knob),
             QString(KXMLQLCInputChannelKnob));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Slider),
             QString(KXMLQLCInputChannelSlider));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::NextPage),
             QString(KXMLQLCInputChannelPageUp));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::PrevPage),
             QString(KXMLQLCInputChannelPageDown));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::PageSet),
             QString(KXMLQLCInputChannelPageSet));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Type(42)),
             QString(KXMLQLCInputChannelNone));
}

void QLCInputChannel_Test::stringToType()
{
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelButton)),
             QLCInputChannel::Button);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelSlider)),
             QLCInputChannel::Slider);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelKnob)),
             QLCInputChannel::Knob);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelPageUp)),
             QLCInputChannel::NextPage);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelPageDown)),
             QLCInputChannel::PrevPage);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelPageSet)),
             QLCInputChannel::PageSet);
    QCOMPARE(QLCInputChannel::stringToType(QString("foobar")),
             QLCInputChannel::NoType);
}

void QLCInputChannel_Test::name()
{
    QLCInputChannel ch;
    QVERIFY(ch.name().isEmpty());
    ch.setName("Foobar");
    QVERIFY(ch.name() == "Foobar");
}

void QLCInputChannel_Test::copy()
{
    QLCInputChannel ch;
    ch.setType(QLCInputChannel::Slider);
    ch.setName("Foobar");

    QLCInputChannel copy(ch);
    QVERIFY(copy.type() == QLCInputChannel::Slider);
    QVERIFY(copy.name() == "Foobar");

    QLCInputChannel another = ch;
    QVERIFY(another.type() == QLCInputChannel::Slider);
    QVERIFY(another.name() == "Foobar");
}

void QLCInputChannel_Test::load()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Channel");
    doc.appendChild(root);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement type = doc.createElement("Type");
    QDomText typeText = doc.createTextNode("Slider");
    type.appendChild(typeText);
    root.appendChild(type);

    QDomElement foo = doc.createElement("Foobar");
    QDomText fooText = doc.createTextNode("Xyzzy");
    foo.appendChild(fooText);
    root.appendChild(foo);

    QLCInputChannel ch;
    ch.loadXML(root);
    QVERIFY(ch.name() == "Foobar");
    QVERIFY(ch.type() == QLCInputChannel::Slider);
}

void QLCInputChannel_Test::loadWrongType()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Channel");
    doc.appendChild(root);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement type = doc.createElement("Type");
    QDomText typeText = doc.createTextNode("Xyzzy");
    type.appendChild(typeText);
    root.appendChild(type);

    QLCInputChannel ch;
    ch.loadXML(root);
    QVERIFY(ch.name() == "Foobar");
    QVERIFY(ch.type() == QLCInputChannel::NoType);
}

void QLCInputChannel_Test::save()
{
    QLCInputChannel ch;
    ch.setName("Foobar Name");
    ch.setType(QLCInputChannel::Knob);

    QDomDocument doc;
    QDomElement root = doc.createElement("fakerootnode");

    QVERIFY(ch.saveXML(&doc, &root, 12) == true);

    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString(KXMLQLCInputChannel));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCInputChannelName)
            QCOMPARE(tag.text(), QString("Foobar Name"));
        else if (tag.tagName() == KXMLQLCInputChannelType)
            QCOMPARE(tag.text(), QString(KXMLQLCInputChannelKnob));
        else
            QFAIL("Unexpected crap in the XML!");
        node = node.nextSibling();
    }
}

QTEST_APPLESS_MAIN(QLCInputChannel_Test)
