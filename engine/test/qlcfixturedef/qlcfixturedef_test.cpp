/*
  Q Light Controller - Unit tests
  qlcfixturedef_test.cpp

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

#include "qlcfixturedef_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"

void QLCFixtureDef_Test::initial()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QVERIFY(fd->manufacturer().isEmpty());
    QVERIFY(fd->model().isEmpty());
    QVERIFY(fd->name() == " ");
    QVERIFY(fd->typeToString(fd->type()) == "Dimmer");
    delete fd;
}

void QLCFixtureDef_Test::manufacturer()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    fd->setManufacturer("Martin");
    QVERIFY(fd->manufacturer() == "Martin");
    QVERIFY(fd->name() == "Martin ");
    delete fd;
}

void QLCFixtureDef_Test::model()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    fd->setModel("MAC600");
    QVERIFY(fd->model() == "MAC600");
    QVERIFY(fd->name() == " MAC600");
    delete fd;
}

void QLCFixtureDef_Test::name()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    fd->setManufacturer("Martin");
    fd->setModel("MAC600");
    QVERIFY(fd->name() == "Martin MAC600");
    delete fd;
}

void QLCFixtureDef_Test::type()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    fd->setType(QLCFixtureDef::Scanner);
    QVERIFY(fd->typeToString(fd->type()) == "Scanner");
    delete fd;
}

void QLCFixtureDef_Test::addChannel()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QVERIFY(fd->channels().size() == 0);

    fd->addChannel(NULL);
    QVERIFY(fd->channels().size() == 0);

    QLCChannel* ch1 = new QLCChannel();
    fd->addChannel(ch1);
    QVERIFY(fd->channels().size() == 1);
    QVERIFY(fd->channels().at(0) == ch1);

    fd->addChannel(ch1);
    QVERIFY(fd->channels().size() == 1);

    QLCChannel* ch2 = new QLCChannel();
    fd->addChannel(ch2);
    QVERIFY(fd->channels().size() == 2);
    QVERIFY(fd->channels().at(0) == ch1);
    QVERIFY(fd->channels().at(1) == ch2);

    delete fd;
}

void QLCFixtureDef_Test::removeChannel()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QLCChannel* ch1 = new QLCChannel();
    QLCChannel* ch2 = new QLCChannel();

    QVERIFY(fd->channels().size() == 0);
    QVERIFY(fd->removeChannel(NULL) == false);
    QVERIFY(fd->removeChannel(ch1) == false);
    QVERIFY(fd->removeChannel(ch2) == false);

    fd->addChannel(ch1);
    fd->addChannel(ch2);
    QVERIFY(fd->channels().size() == 2);

    QVERIFY(fd->removeChannel(ch1) == true);
    QVERIFY(fd->channels().size() == 1);
    QVERIFY(fd->channels().at(0) == ch2);

    QVERIFY(fd->removeChannel(ch1) == false);
    QVERIFY(fd->channels().size() == 1);
    QVERIFY(fd->channels().at(0) == ch2);

    QVERIFY(fd->removeChannel(NULL) == false);
    QVERIFY(fd->channels().size() == 1);
    QVERIFY(fd->channels().at(0) == ch2);

    QVERIFY(fd->removeChannel(ch2) == true);
    QVERIFY(fd->channels().size() == 0);

    delete fd;
}

void QLCFixtureDef_Test::channel()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QLCChannel* ch1 = new QLCChannel();
    ch1->setName("foo");
    fd->addChannel(ch1);

    QLCChannel* ch2 = new QLCChannel();
    ch2->setName("bar");
    fd->addChannel(ch2);

    QLCChannel* ch3 = new QLCChannel();
    ch3->setName("xyzzy");
    fd->addChannel(ch3);

    QVERIFY(fd->channel("foo") == ch1);
    QVERIFY(fd->channel("bar") == ch2);
    QVERIFY(fd->channel("xyzzy") == ch3);
    QVERIFY(fd->channel("foobar") == NULL);

    delete fd;
}

void QLCFixtureDef_Test::channels()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QLCChannel* ch1 = new QLCChannel();
    QLCChannel* ch2 = new QLCChannel();
    QLCChannel* ch3 = new QLCChannel();

    QVERIFY(fd->channels().size() == 0);
    fd->addChannel(ch1);
    QVERIFY(fd->channels().size() == 1);
    QVERIFY(fd->channels().at(0) == ch1);
    fd->addChannel(ch2);
    QVERIFY(fd->channels().size() == 2);
    QVERIFY(fd->channels().at(0) == ch1);
    QVERIFY(fd->channels().at(1) == ch2);
    fd->addChannel(ch3);
    QVERIFY(fd->channels().size() == 3);
    QVERIFY(fd->channels().at(0) == ch1);
    QVERIFY(fd->channels().at(1) == ch2);
    QVERIFY(fd->channels().at(2) == ch3);

    delete fd;
}

void QLCFixtureDef_Test::addMode()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QLCFixtureMode* mode1 = new QLCFixtureMode(fd);
    QLCFixtureMode* mode2 = new QLCFixtureMode(fd);

    QVERIFY(fd->modes().size() == 0);

    fd->addMode(NULL);
    QVERIFY(fd->modes().size() == 0);

    fd->addMode(mode1);
    QVERIFY(fd->modes().size() == 1);
    QVERIFY(fd->modes().at(0) == mode1);

    fd->addMode(mode1);
    QVERIFY(fd->modes().size() == 1);
    QVERIFY(fd->modes().at(0) == mode1);

    fd->addMode(mode2);
    QVERIFY(fd->modes().size() == 2);
    QVERIFY(fd->modes().at(0) == mode1);
    QVERIFY(fd->modes().at(1) == mode2);

    delete fd;
}

void QLCFixtureDef_Test::removeMode()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QLCFixtureMode* mode1 = new QLCFixtureMode(fd);
    QLCFixtureMode* mode2 = new QLCFixtureMode(fd);

    QVERIFY(fd->modes().size() == 0);
    QVERIFY(fd->removeMode(NULL) == false);
    QVERIFY(fd->removeMode(mode1) == false);
    QVERIFY(fd->removeMode(mode2) == false);
    QVERIFY(fd->modes().size() == 0);

    fd->addMode(mode1);
    fd->addMode(mode2);
    QVERIFY(fd->modes().size() == 2);

    QVERIFY(fd->removeMode(mode1) == true);
    QVERIFY(fd->modes().size() == 1);
    QVERIFY(fd->modes().at(0) == mode2);

    QVERIFY(fd->removeMode(mode1) == false);
    QVERIFY(fd->modes().size() == 1);
    QVERIFY(fd->modes().at(0) == mode2);

    QVERIFY(fd->removeMode(NULL) == false);
    QVERIFY(fd->modes().size() == 1);
    QVERIFY(fd->modes().at(0) == mode2);

    QVERIFY(fd->removeMode(mode2) == true);
    QVERIFY(fd->modes().size() == 0);

    delete fd;
}

void QLCFixtureDef_Test::mode()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QLCFixtureMode* mode1 = new QLCFixtureMode(fd);
    mode1->setName("foo");
    fd->addMode(mode1);

    QLCFixtureMode* mode2 = new QLCFixtureMode(fd);
    mode2->setName("bar");
    fd->addMode(mode2);

    QLCFixtureMode* mode3 = new QLCFixtureMode(fd);
    mode3->setName("xyzzy");
    fd->addMode(mode3);

    QVERIFY(fd->mode("foo") == mode1);
    QVERIFY(fd->mode("bar") == mode2);
    QVERIFY(fd->mode("xyzzy") == mode3);
    QVERIFY(fd->mode("foobar") == NULL);

    delete fd;
}

void QLCFixtureDef_Test::modes()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    QLCFixtureMode* mode1 = new QLCFixtureMode(fd);
    QLCFixtureMode* mode2 = new QLCFixtureMode(fd);
    QLCFixtureMode* mode3 = new QLCFixtureMode(fd);

    QVERIFY(fd->modes().size() == 0);
    fd->addMode(mode1);
    QVERIFY(fd->modes().size() == 1);
    QVERIFY(fd->modes().at(0) == mode1);
    fd->addMode(mode2);
    QVERIFY(fd->modes().size() == 2);
    QVERIFY(fd->modes().at(0) == mode1);
    QVERIFY(fd->modes().at(1) == mode2);
    fd->addMode(mode3);
    QVERIFY(fd->modes().size() == 3);
    QVERIFY(fd->modes().at(0) == mode1);
    QVERIFY(fd->modes().at(1) == mode2);
    QVERIFY(fd->modes().at(2) == mode3);

    delete fd;
}

void QLCFixtureDef_Test::copy()
{
    QLCFixtureDef* fd = new QLCFixtureDef();
    fd->setManufacturer("Martin");
    fd->setModel("MAC600");
    fd->setType(QLCFixtureDef::MovingHead);

    QLCChannel* ch = new QLCChannel();
    ch->setName("TestChannel");
    fd->addChannel(ch);

    QLCFixtureMode* mode = new QLCFixtureMode(fd);
    mode->setName("TestMode");
    fd->addMode(mode);
    mode->insertChannel(ch, 0);

    QLCFixtureDef* copy = new QLCFixtureDef(fd);
    QVERIFY(copy->manufacturer() == "Martin");
    QVERIFY(copy->model() == "MAC600");
    QVERIFY(copy->typeToString(copy->type()) == "Moving Head");

    /* Verify that modes and channels get copied and that the channels in
       the copied mode are from the copied fixtureDef and not the one that
       the copy is taken FROM. */
    QVERIFY(copy->channels().at(0)->name() == "TestChannel");
    QVERIFY(copy->modes().at(0)->name() == "TestMode");
    QVERIFY(copy->modes().at(0)->channels().size() == 1);
    QVERIFY(copy->modes().size() == 1);
    QVERIFY(copy->modes().at(0)->channel(0) != ch);
    QVERIFY(copy->modes().at(0)->channel(0) == copy->channels().at(0));
    QVERIFY(copy->channels().at(0)->name() == "TestChannel");
    QVERIFY(copy->modes().at(0)->channel(0)->name() == "TestChannel");

    delete fd;
    delete copy;
}

void QLCFixtureDef_Test::saveLoadXML()
{
    const QString path("qlcfixturedef_test_saveXML.qxf");

    QLCFixtureDef* def = new QLCFixtureDef;
    def->setManufacturer("Foobar");
    def->setModel("Xyzzy");
    def->setType(QLCFixtureDef::Other);

    QLCChannel* ch = new QLCChannel();
    ch->setName("Whatever");
    def->addChannel(ch);

    QLCFixtureMode* mode = new QLCFixtureMode(def);
    mode->setName("Barfoo");
    def->addMode(mode);
    mode->insertChannel(ch, 0);

    QVERIFY(def->saveXML(QString("zxcvb:/path/to/nowhere") + path) != QFile::NoError);
    QCOMPARE(def->saveXML(path), QFile::NoError);

    // Test only QLCFixtureDef's doings and don't go into channel/mode details
    // since they are tested in their individual unit tests.
    QLCFixtureDef* def2 = new QLCFixtureDef;
    QCOMPARE(def2->loadXML(QString()), QFile::OpenError);
    QCOMPARE(def2->loadXML("/path/beyond/this/universe/foo.qxf"), QFile::ReadError);
    QCOMPARE(def2->loadXML("readonly.xml"), QFile::ReadError);

    QCOMPARE(def2->loadXML(path), QFile::NoError);
    QCOMPARE(def2->manufacturer(), def->manufacturer());
    QCOMPARE(def2->model(), def->model());
    QCOMPARE(def2->channels().size(), 1);
    QCOMPARE(def2->channels().at(0)->name(), ch->name());
    QCOMPARE(def2->modes().size(), 1);
    QCOMPARE(def2->modes().at(0)->name(), mode->name());

    delete def;
    delete def2;
    QFile::remove(path);
    QVERIFY(QFile::exists(path) == false);
}

QTEST_APPLESS_MAIN(QLCFixtureDef_Test)
