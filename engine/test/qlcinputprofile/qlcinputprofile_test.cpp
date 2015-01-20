/*
  Q Light Controller - Unit tests
  qlcinputprofile_test.cpp

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

#include "qlcinputprofile_test.h"
#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlcchannel.h"

#include "../common/resource_paths.h"

void QLCInputProfile_Test::manufacturer()
{
    QLCInputProfile ip;
    QVERIFY(ip.manufacturer().isEmpty());
    ip.setManufacturer("Behringer");
    QVERIFY(ip.manufacturer() == "Behringer");
}

void QLCInputProfile_Test::model()
{
    QLCInputProfile ip;
    QVERIFY(ip.model().isEmpty());
    ip.setModel("BCF2000");
    QVERIFY(ip.model() == "BCF2000");
}

void QLCInputProfile_Test::name()
{
    QLCInputProfile ip;
    QVERIFY(ip.name() == " ");
    ip.setManufacturer("Behringer");
    QVERIFY(ip.name() == "Behringer ");
    ip.setModel("BCF2000");
    QVERIFY(ip.name() == "Behringer BCF2000");
}

void QLCInputProfile_Test::addChannel()
{
    QLCInputProfile ip;

    QLCInputChannel* ich1 = new QLCInputChannel();
    ip.insertChannel(0, ich1);
    QVERIFY(ip.channel(0) == ich1);
    QVERIFY(ip.channels().size() == 1);

    /* Shouldn't overwrite the existing mapping */
    QLCInputChannel* ich2 = new QLCInputChannel();
    ip.insertChannel(0, ich2);
    QVERIFY(ip.channel(0) == ich1);

    ip.insertChannel(5, ich2);
    QVERIFY(ip.channel(0) == ich1);
    QVERIFY(ip.channel(1) == NULL);
    QVERIFY(ip.channel(2) == NULL);
    QVERIFY(ip.channel(3) == NULL);
    QVERIFY(ip.channel(4) == NULL);
    QVERIFY(ip.channel(5) == ich2);
}

void QLCInputProfile_Test::removeChannel()
{
    QLCInputProfile ip;

    QLCInputChannel* ich1 = new QLCInputChannel();
    ip.insertChannel(0, ich1);
    QVERIFY(ip.channel(0) == ich1);

    QLCInputChannel* ich2 = new QLCInputChannel();
    ip.insertChannel(5, ich2);
    QVERIFY(ip.channel(0) == ich1);
    QVERIFY(ip.channel(5) == ich2);

    QVERIFY(ip.removeChannel(1) == false);
    QVERIFY(ip.removeChannel(2) == false);
    QVERIFY(ip.removeChannel(3) == false);
    QVERIFY(ip.removeChannel(4) == false);

    QVERIFY(ip.channel(0) == ich1);
    QVERIFY(ip.channel(5) == ich2);
    QVERIFY(ip.removeChannel(0) == true);
    QVERIFY(ip.channel(0) == NULL);
    QVERIFY(ip.channel(5) == ich2);
    QVERIFY(ip.removeChannel(5) == true);
    QVERIFY(ip.channel(0) == NULL);
    QVERIFY(ip.channel(5) == NULL);
}

void QLCInputProfile_Test::remapChannel()
{
    QLCInputProfile ip;

    QLCInputChannel* ich1 = new QLCInputChannel();
    ich1->setName("Foobar");
    ip.insertChannel(0, ich1);
    QVERIFY(ip.channel(0) == ich1);

    QLCInputChannel* ich2 = new QLCInputChannel();
    ip.insertChannel(5, ich2);
    QVERIFY(ip.channel(0) == ich1);
    QVERIFY(ip.channel(5) == ich2);

    QVERIFY(ip.remapChannel(ich1, 9000) == true);
    QVERIFY(ip.channel(0) == NULL);
    QVERIFY(ip.channel(5) == ich2);
    QVERIFY(ip.channel(9000) == ich1);
    QVERIFY(ip.channel(9000)->name() == "Foobar");

    QVERIFY(ip.remapChannel(NULL, 9000) == false);
    QVERIFY(ip.channels().size() == 2);
    QVERIFY(ip.channel(0) == NULL);
    QVERIFY(ip.channel(5) == ich2);
    QVERIFY(ip.channel(9000) == ich1);
    QVERIFY(ip.channel(9000)->name() == "Foobar");

    QLCInputChannel* ich3 = new QLCInputChannel();
    QVERIFY(ip.remapChannel(ich3, 5) == false);
    QVERIFY(ip.channels().size() == 2);
    QVERIFY(ip.channel(0) == NULL);
    QVERIFY(ip.channel(5) == ich2);
    QVERIFY(ip.channel(9000) == ich1);
    QVERIFY(ip.channel(9000)->name() == "Foobar");

    delete ich3;
}

void QLCInputProfile_Test::channel()
{
    QLCInputProfile ip;

    QLCInputChannel* ich1 = new QLCInputChannel();
    ip.insertChannel(0, ich1);

    QLCInputChannel* ich2 = new QLCInputChannel();
    ip.insertChannel(5, ich2);

    QVERIFY(ip.channel(0) == ich1);
    QVERIFY(ip.channel(1) == NULL);
    QVERIFY(ip.channel(2) == NULL);
    QVERIFY(ip.channel(3) == NULL);
    QVERIFY(ip.channel(4) == NULL);
    QVERIFY(ip.channel(5) == ich2);
}

void QLCInputProfile_Test::channels()
{
    QLCInputProfile ip;
    QVERIFY(ip.channels().size() == 0);

    QLCInputChannel* ich1 = new QLCInputChannel();
    ip.insertChannel(0, ich1);

    QLCInputChannel* ich2 = new QLCInputChannel();
    ip.insertChannel(5, ich2);

    QVERIFY(ip.channels().size() == 2);
    QVERIFY(ip.channels().contains(0) == true);
    QVERIFY(ip.channels().contains(1) == false);
    QVERIFY(ip.channels().contains(2) == false);
    QVERIFY(ip.channels().contains(3) == false);
    QVERIFY(ip.channels().contains(4) == false);
    QVERIFY(ip.channels().contains(5) == true);
    QVERIFY(ip.channels()[0] == ich1);
    QVERIFY(ip.channels()[5] == ich2);
}

void QLCInputProfile_Test::channelNumber()
{
    QLCInputProfile ip;
    QVERIFY(ip.channels().size() == 0);

    QLCInputChannel* ich1 = new QLCInputChannel();
    ip.insertChannel(0, ich1);

    QLCInputChannel* ich2 = new QLCInputChannel();
    ip.insertChannel(6510, ich2);

    QLCInputChannel* ich3 = new QLCInputChannel();
    ip.insertChannel(5, ich3);

    QCOMPARE(ip.channelNumber(NULL), QLCChannel::invalid());
    QCOMPARE(ip.channelNumber(ich1), quint32(0));
    QCOMPARE(ip.channelNumber(ich2), quint32(6510));
    QCOMPARE(ip.channelNumber(ich3), quint32(5));
}

void QLCInputProfile_Test::copy()
{
    QLCInputProfile ip;
    ip.setManufacturer("Behringer");
    ip.setModel("BCF2000");

    QLCInputChannel* ich1 = new QLCInputChannel();
    ich1->setName("Channel 1");
    ip.insertChannel(0, ich1);

    QLCInputChannel* ich2 = new QLCInputChannel();
    ich2->setName("Channel 2");
    ip.insertChannel(5, ich2);

    QLCInputChannel* ich3 = new QLCInputChannel();
    ich3->setName("Channel 3");
    ip.insertChannel(2, ich3);

    QLCInputChannel* ich4 = new QLCInputChannel();
    ich4->setName("Channel 4");
    ip.insertChannel(9000, ich4);

    QLCInputProfile copy = ip;
    QVERIFY(copy.manufacturer() == "Behringer");
    QVERIFY(copy.model() == "BCF2000");

    QVERIFY(copy.channels().size() == 4);

    /* Verify that it's a deep copy */
    QVERIFY(copy.channel(0) != ich1);
    QVERIFY(copy.channel(0) != NULL);
    QVERIFY(copy.channel(0)->name() == "Channel 1");

    QVERIFY(copy.channel(5) != ich2);
    QVERIFY(copy.channel(5) != NULL);
    QVERIFY(copy.channel(5)->name() == "Channel 2");

    QVERIFY(copy.channel(2) != ich3);
    QVERIFY(copy.channel(2) != NULL);
    QVERIFY(copy.channel(2)->name() == "Channel 3");

    QVERIFY(copy.channel(9000) != ich4);
    QVERIFY(copy.channel(9000) != NULL);
    QVERIFY(copy.channel(9000)->name() == "Channel 4");
}

void QLCInputProfile_Test::assign()
{
    QLCInputProfile ip;
    ip.setManufacturer("Behringer");
    ip.setModel("BCF2000");

    QLCInputChannel* ich1 = new QLCInputChannel;
    ich1->setName("Channel 1");
    ip.insertChannel(0, ich1);

    QLCInputChannel* ich2 = new QLCInputChannel;
    ich2->setName("Channel 2");
    ip.insertChannel(5, ich2);

    QLCInputChannel* ich3 = new QLCInputChannel;
    ich3->setName("Channel 3");
    ip.insertChannel(2, ich3);

    QLCInputChannel* ich4 = new QLCInputChannel;
    ich4->setName("Channel 4");
    ip.insertChannel(9000, ich4);

    QLCInputProfile ip2;
    QLCInputChannel* ich5 = new QLCInputChannel;
    ich5->setName("First channel");
    ip2.insertChannel(0, ich5);
    QCOMPARE(ip2.channels().size(), 1);

    /* Test the assignment operator */
    ip2 = ip;
    QCOMPARE(ip2.channels().size(), 4);
    QVERIFY(ip2.channel(0) != NULL);
    QVERIFY(ip2.channel(0) != ich1);
    QVERIFY(ip2.channel(0) != ip.channel(0));
    QCOMPARE(ip2.channel(0)->name(), QString("Channel 1"));

    QVERIFY(ip2.channel(5) != NULL);
    QVERIFY(ip2.channel(5) != ich2);
    QVERIFY(ip2.channel(5) != ip.channel(5));
    QCOMPARE(ip2.channel(5)->name(), QString("Channel 2"));

    QVERIFY(ip2.channel(2) != NULL);
    QVERIFY(ip2.channel(2) != ich3);
    QVERIFY(ip2.channel(2) != ip.channel(2));
    QCOMPARE(ip2.channel(2)->name(), QString("Channel 3"));

    QVERIFY(ip2.channel(9000) != NULL);
    QVERIFY(ip2.channel(9000) != ich4);
    QVERIFY(ip2.channel(9000) != ip.channel(9000));
    QCOMPARE(ip2.channel(9000)->name(), QString("Channel 4"));
}

void QLCInputProfile_Test::load()
{
    QDomDocument doc;

    QDomElement profile = doc.createElement("InputProfile");
    QDomElement manuf = doc.createElement("Manufacturer");
    QDomText manufText = doc.createTextNode("Behringer");
    manuf.appendChild(manufText);
    profile.appendChild(manuf);
    QDomElement model = doc.createElement("Model");
    QDomText modelText = doc.createTextNode("BCF2000");
    model.appendChild(modelText);
    profile.appendChild(model);
    doc.appendChild(profile);

    QDomElement ch = doc.createElement("Channel");
    ch.setAttribute("Number", 492);
    profile.appendChild(ch);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    ch.appendChild(name);

    QDomElement type = doc.createElement("Type");
    QDomText typeText = doc.createTextNode("Slider");
    type.appendChild(typeText);
    ch.appendChild(type);

    QLCInputProfile ip;
    QVERIFY(ip.loadXML(doc) == true);
    QVERIFY(ip.manufacturer() == "Behringer");
    QVERIFY(ip.model() == "BCF2000");
    QVERIFY(ip.channels().size() == 1);
    QVERIFY(ip.channel(492) != NULL);
    QVERIFY(ip.channel(492)->name() == "Foobar");
    QVERIFY(ip.channel(492)->type() == QLCInputChannel::Slider);
}

void QLCInputProfile_Test::loadNoProfile()
{
    QDomDocument doc;
    QLCInputProfile ip;
    QVERIFY(ip.loadXML(doc) == false);

    QDomElement root = doc.createElement("Whatever");
    doc.appendChild(root);
    QVERIFY(ip.loadXML(doc) == false);
}

void QLCInputProfile_Test::loader()
{
    QLCInputProfile* prof = QLCInputProfile::loader("foobar");
    QVERIFY(prof == NULL);

    prof = QLCInputProfile::loader("broken.xml");
    QVERIFY(prof == NULL);

    QString path(INTERNAL_PROFILEDIR "Generic-MIDI.qxi");
    prof = QLCInputProfile::loader(path);
    QVERIFY(prof != NULL);
    QCOMPARE(prof->path(), path);
    QCOMPARE(prof->name(), QString("Generic MIDI"));
    QCOMPARE(prof->channels().size(), 256);
}

void QLCInputProfile_Test::save()
{
    QLCInputProfile ip;
    ip.setManufacturer("TestManufacturer");
    ip.setModel("TestModel");

    QLCInputChannel* ich1 = new QLCInputChannel();
    ich1->setName("Channel 1");
    ip.insertChannel(0, ich1);

    QLCInputChannel* ich2 = new QLCInputChannel();
    ich2->setName("Channel 2");
    ip.insertChannel(5, ich2);

    QLCInputChannel* ich3 = new QLCInputChannel();
    ich3->setName("Channel 3");
    ip.insertChannel(2, ich3);

    QString path("test.qxi");
    QVERIFY(ip.saveXML(path) == true);

#if !defined(WIN32) && !defined(Q_OS_WIN)
    QFile::Permissions perm = QFile::permissions(path);
    QFile::setPermissions(path, 0);
    QVERIFY(ip.saveXML(path) == false);
    QFile::setPermissions(path, perm);
#endif

    QLCInputProfile* prof = QLCInputProfile::loader(path);
    QVERIFY(prof != NULL);
    QCOMPARE(prof->manufacturer(), ip.manufacturer());
    QCOMPARE(prof->model(), ip.model());
    QCOMPARE(prof->name(), ip.name());
    QCOMPARE(prof->channels().size(), ip.channels().size());
    QCOMPARE(prof->channels()[0]->name(), ich1->name());
    QCOMPARE(prof->channels()[2]->name(), ich3->name());
    QCOMPARE(prof->channels()[5]->name(), ich2->name());

    QVERIFY(QFile::remove(path) == true);
    delete prof;
}

QTEST_APPLESS_MAIN(QLCInputProfile_Test)
