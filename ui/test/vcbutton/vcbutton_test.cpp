/*
  Q Light Controller Plus - Unit test
  vcbutton_test.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>
#include <QtTest>
#include <QMenu>

#define protected public
#define private public
#include "qlcfixturedefcache.h"
#include "qlcinputsource.h"
#include "virtualconsole.h"
#include "vcbutton_test.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "vcbutton.h"
#include "vcframe.h"
#include "scene.h"
#include "doc.h"
#undef private
#undef protected

static const QKeySequence keySequenceA(Qt::Key_A);
static const QKeySequence keySequenceB(Qt::Key_B);

void VCButton_Test::initTestCase()
{
    m_doc = NULL;
}

void VCButton_Test::init()
{
    m_doc = new Doc(this);
    new VirtualConsole(NULL, m_doc);

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(4);
    m_doc->addFixture(fxi);
}

void VCButton_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_doc;
}

void VCButton_Test::initial()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    QCOMPARE(btn.objectName(), QString("VCButton"));
    QCOMPARE(btn.frameStyle(), (int) KVCFrameStyleNone);
    QCOMPARE(btn.caption(), QString());
    QCOMPARE(btn.size(), QSize(50, 50));
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.startupIntensity(), qreal(1.0));
    QCOMPARE(btn.isStartupIntensityEnabled(), false);
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(btn.action(), VCButton::Toggle);
    QCOMPARE(btn.iconPath(), QString());
    QVERIFY(btn.m_chooseIconAction != NULL);
    QVERIFY(btn.m_resetIconAction != NULL);

    // Only for coverage
    btn.setBackgroundImage(QString());
}

void VCButton_Test::function()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    btn.setFunction(42);
    QCOMPARE(btn.function(), Function::invalidId());

    Scene* s = new Scene(m_doc);
    s->setName("Test1");
    m_doc->addFunction(s);
    btn.setFunction(s->id());
    QCOMPARE(btn.function(), s->id());
    QCOMPARE(btn.toolTip(), QString("Test1"));

    Scene* s2 = new Scene(m_doc);
    s2->setName("Test2");
    m_doc->addFunction(s2);
    btn.setFunction(s2->id());
    QCOMPARE(btn.function(), s2->id());
    QCOMPARE(btn.toolTip(), QString("Test2"));

    btn.setFunction(s2->id() + 1);
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());

    btn.setFunction(s2->id());
    QCOMPARE(btn.function(), s2->id());
    QCOMPARE(btn.toolTip(), QString("Test2"));

    m_doc->deleteFunction(s2->id());
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());

    m_doc->deleteFunction(s->id());
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());
}

void VCButton_Test::action()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    btn.setAction(VCButton::Flash);
    QCOMPARE(btn.action(), VCButton::Flash);
    btn.setAction(VCButton::Toggle);
    QCOMPARE(btn.action(), VCButton::Toggle);

    QCOMPARE(VCButton::actionToString(VCButton::Toggle), QString("Toggle"));
    QCOMPARE(VCButton::actionToString(VCButton::Flash), QString("Flash"));
    QCOMPARE(VCButton::actionToString((VCButton::Action) 31337), QString("Toggle"));
    QCOMPARE(VCButton::stringToAction("Toggle"), VCButton::Toggle);
    QCOMPARE(VCButton::stringToAction("Flash"), VCButton::Flash);
    QCOMPARE(VCButton::stringToAction("Foobar"), VCButton::Toggle);
}

void VCButton_Test::intensity()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    btn.enableStartupIntensity(true);
    QCOMPARE(btn.isStartupIntensityEnabled(), true);
    btn.enableStartupIntensity(false);
    QCOMPARE(btn.isStartupIntensityEnabled(), false);

    for (qreal i = -0.5; i < 1.2; i += 0.01)
    {
        btn.setStartupIntensity(i);
        QCOMPARE(btn.startupIntensity(), CLAMP(i, qreal(0.0), qreal(1.0)));
    }
}

void VCButton_Test::bgcolor()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    m_doc->resetModified();
    btn.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::Button), QColor(Qt::red));
    QCOMPARE(m_doc->isModified(), true);
    QVERIFY(btn.foregroundColor() != QColor(Qt::red));
}

void VCButton_Test::fgcolor()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    m_doc->resetModified();
    btn.setForegroundColor(QColor(Qt::red));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::ButtonText), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::WindowText), QColor(Qt::red));
    QCOMPARE(m_doc->isModified(), true);
    QVERIFY(btn.backgroundColor() != QColor(Qt::red));
}

void VCButton_Test::resetColors()
{
    QWidget w;

    VCButton btn(&w, m_doc);

    btn.setForegroundColor(QColor(Qt::red));
    btn.setBackgroundColor(QColor(Qt::blue));
    m_doc->resetModified();
    btn.resetForegroundColor();
    QCOMPARE(btn.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::blue));
    QCOMPARE(m_doc->isModified(), true);

    btn.resetForegroundColor();
    QCOMPARE(btn.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::blue));

    btn.setForegroundColor(QColor(Qt::red));
    btn.setBackgroundColor(QColor(Qt::blue));
    m_doc->resetModified();
    btn.resetBackgroundColor();
    QCOMPARE(btn.backgroundColor(), w.palette().color(QPalette::Button));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
    QCOMPARE(m_doc->isModified(), true);

    btn.resetBackgroundColor();
    QCOMPARE(btn.backgroundColor(), w.palette().color(QPalette::Button));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
}

void VCButton_Test::iconPath()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    m_doc->resetModified();
    btn.setIconPath("../../../resources/icons/png/qlcplus.png");
    QCOMPARE(btn.iconPath(), QString("../../../resources/icons/png/qlcplus.png"));
    QCOMPARE(m_doc->isModified(), true);

    m_doc->resetModified();
    btn.slotResetIcon();
    QCOMPARE(btn.iconPath(), QString());
    QCOMPARE(m_doc->isModified(), true);
}

void VCButton_Test::on()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    btn.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(0, 1)));

    QCOMPARE(btn.state(), VCButton::Inactive);

    btn.setState(VCButton::Inactive);
    QCOMPARE(btn.state(), VCButton::Inactive);

    btn.setState(VCButton::Active);
    QCOMPARE(btn.state(), VCButton::Active);

    btn.setState(VCButton::Active);
    QCOMPARE(btn.state(), VCButton::Active);

    btn.setState(VCButton::Inactive);
    QCOMPARE(btn.state(), VCButton::Inactive);
}

void VCButton_Test::keySequence()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    QCOMPARE(btn.keySequence(), QKeySequence());

    QKeySequence seq(keySequenceA);
    btn.setKeySequence(seq);
    QCOMPARE(btn.keySequence(), seq);

    seq = QKeySequence(keySequenceB);
    QVERIFY(btn.keySequence() != seq);
    btn.setKeySequence(seq);
    QCOMPARE(btn.keySequence(), seq);
}

void VCButton_Test::copy()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setIconPath("../../../resources/icons/png/qlcplus.png");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.setKeySequence(QKeySequence(keySequenceB));
    btn.enableStartupIntensity(true);
    btn.setStartupIntensity(qreal(0.2));

    VCFrame parent(&w, m_doc);
    VCButton* copy = qobject_cast<VCButton*> (btn.createCopy(&parent));
    QVERIFY(copy != NULL);
    QCOMPARE(copy->caption(), QString("Foobar"));
    QCOMPARE(copy->iconPath(), QString("../../../resources/icons/png/qlcplus.png"));
    QCOMPARE(copy->function(), sc->id());
    QCOMPARE(copy->action(), VCButton::Flash);
    QCOMPARE(copy->keySequence(), QKeySequence(keySequenceB));
    QCOMPARE(copy->isStartupIntensityEnabled(), true);
    QCOMPARE(copy->startupIntensity(), qreal(0.2));
    delete copy;
}

void VCButton_Test::load()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);
    m_doc->setWorkspacePath(QDir("../../../resources/icons/png").absolutePath());

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Button");
    xmlWriter.writeAttribute("Caption", "Pertti");
    xmlWriter.writeAttribute("Icon", "qlcplus.png");

    xmlWriter.writeStartElement("WindowState");
    xmlWriter.writeAttribute("X", "20");
    xmlWriter.writeAttribute("Y", "20");
    xmlWriter.writeAttribute("Width", "60");
    xmlWriter.writeAttribute("Height", "60");
    xmlWriter.writeAttribute("Visible", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Appearance");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("ID", QString::number(sc->id()));
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Input");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Action", "Flash");

    xmlWriter.writeTextElement("Key", QKeySequence(keySequenceA).toString());

    xmlWriter.writeStartElement("Intensity");
    xmlWriter.writeAttribute("Adjust", "True");
    xmlWriter.writeCharacters("60");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCButton btn(&w, m_doc);
    QCOMPARE(btn.loadXML(xmlReader), true);
    QCOMPARE(btn.caption(), QString("Pertti"));
    QCOMPARE(btn.iconPath(), QFileInfo(QString("../../../resources/icons/png/qlcplus.png")).canonicalFilePath());
    QCOMPARE(btn.function(), sc->id());
    QCOMPARE(btn.action(), VCButton::Flash);
    QCOMPARE(btn.keySequence(), QKeySequence(keySequenceA));
    QCOMPARE(btn.isStartupIntensityEnabled(), true);
    QCOMPARE(btn.startupIntensity(), qreal(0.6));
    QCOMPARE(btn.pos(), QPoint(20, 20));
    QCOMPARE(btn.size(), QSize(60, 60));

    buffer.close();
    QByteArray bData = buffer.data();
    bData.replace("Adjust=\"True\"", "Adjust=\"False\"");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(btn.loadXML(xmlReader), true);
    QCOMPARE(btn.caption(), QString("Pertti"));
    QCOMPARE(btn.iconPath(), QFileInfo(QString("../../../resources/icons/png/qlcplus.png")).canonicalFilePath());
    QCOMPARE(btn.function(), sc->id());
    QCOMPARE(btn.action(), VCButton::Flash);
    QCOMPARE(btn.keySequence(), QKeySequence(keySequenceA));
    QCOMPARE(btn.isStartupIntensityEnabled(), false);
    QCOMPARE(btn.startupIntensity(), qreal(0.6));
    QCOMPARE(btn.pos(), QPoint(20, 20));
    QCOMPARE(btn.size(), QSize(60, 60));

    buffer.close();
    bData = buffer.data();
    bData.replace("<Button", "<Buton");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(btn.loadXML(xmlReader), false);
}

void VCButton_Test::save()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);
    m_doc->setWorkspacePath(QDir("../../../resources/icons/png").absolutePath());

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setIconPath("../../../resources/icons/png/qlcplus.png");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.setKeySequence(QKeySequence(keySequenceB));
    btn.enableStartupIntensity(true);
    btn.setStartupIntensity(qreal(0.2));
    btn.setFlashForceLTP(true);
    btn.setFlashOverride(true);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    int function = 0, action = 0, key = 0, intensity = 0, wstate = 0, appearance = 0, flashProperties = 0;
    QCOMPARE(btn.saveXML(&xmlWriter), true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Button"));
    QCOMPARE(xmlReader.attributes().value("Icon").toString(), QString("qlcplus.png"));
    QCOMPARE(xmlReader.attributes().value("Caption").toString(), QString("Foobar"));

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Function")
        {
            function++;
            QCOMPARE(xmlReader.attributes().value("ID").toString(), QString::number(sc->id()));
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Action")
        {
            QCOMPARE(xmlReader.attributes().value("Override").toString(), QString("1"));
            QCOMPARE(xmlReader.attributes().value("ForceLTP").toString(), QString("1"));
            flashProperties++;
            QCOMPARE(xmlReader.readElementText(), QString("Flash"));
            action++;
        }
        else if (xmlReader.name().toString() == "Key")
        {
            key++;
            QCOMPARE(xmlReader.readElementText(), QKeySequence(keySequenceB).toString());
        }
        else if (xmlReader.name().toString() == "Intensity")
        {
            intensity++;
            QCOMPARE(xmlReader.attributes().value("Adjust").toString(), QString("True"));
            QCOMPARE(xmlReader.readElementText(), QString("20"));
        }
        else if (xmlReader.name().toString() == "WindowState")
        {
            wstate++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Appearance")
        {
            appearance++;
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
            xmlReader.skipCurrentElement();
        }
    }

    QCOMPARE(function, 1);
    QCOMPARE(action, 1);
    QCOMPARE(key, 1);
    QCOMPARE(intensity, 1);
    QCOMPARE(wstate, 1);
    QCOMPARE(appearance, 1);
    QCOMPARE(flashProperties, 1);
}

void VCButton_Test::customMenu()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    QMenu* menu = btn.customMenu(NULL);
    QVERIFY(menu != NULL);
    QCOMPARE(menu->title(), tr("Icon"));
    QCOMPARE(menu->actions().size(), 2);
    QCOMPARE(menu->actions()[0], btn.m_chooseIconAction);
    QCOMPARE(menu->actions()[1], btn.m_resetIconAction);
    delete menu;
}

void VCButton_Test::toggle()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    sc->setValue(0, 0, 255);
    sc->setFadeInSpeed(1000);
    sc->setFadeOutSpeed(1000);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Toggle);
    btn.setKeySequence(QKeySequence(keySequenceB));
    btn.enableStartupIntensity(true);
    btn.setStartupIntensity(qreal(0.2));

    // Mouse button press in design mode doesn't toggle the function
    QCOMPARE(m_doc->mode(), Doc::Design);
    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), QPoint(0, 0), QPoint(0, 0),
                   Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    btn.mousePressEvent(&ev);
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 0);
    QMouseEvent ev2(QEvent::MouseButtonRelease, QPoint(0, 0), QPoint(0, 0), QPoint(0, 0),
                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    btn.mouseReleaseEvent(&ev2);
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 0);

    // Mouse button press in operate mode should toggle the function
    m_doc->setMode(Doc::Operate);
    btn.slotKeyPressed(QKeySequence(keySequenceB));
    // tell MasterTimer to process start queue
    m_doc->masterTimer()->timerTick();
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 1);
    QCOMPARE(m_doc->masterTimer()->m_functionList[0], sc);
    QCOMPARE(sc->getAttributeValue(Function::Intensity), btn.startupIntensity());
    btn.slotKeyReleased(QKeySequence(keySequenceB));
    m_doc->masterTimer()->timerTick(); // Allow MasterTimer to take the function under execution
    QCOMPARE(sc->stopped(), false);
    QCOMPARE(btn.state(), VCButton::Active);

    QMouseEvent ev3(QEvent::MouseButtonPress, QPoint(0, 0), QPoint(0, 0), QPoint(0, 0),
                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    btn.mousePressEvent(&ev3);
    QCOMPARE(sc->m_stop, true);
    QCOMPARE(btn.state(), VCButton::Active);

    btn.slotFunctionStopped(sc->id());
    QCOMPARE(btn.state(), VCButton::Inactive);
    VCButton another(&w, m_doc);
    QVERIFY(btn.palette().color(QPalette::Button) != another.palette().color(QPalette::Button));
    QTest::qWait(500);
    QVERIFY(btn.palette().color(QPalette::Button) == another.palette().color(QPalette::Button));

    QMouseEvent ev4(QEvent::MouseButtonRelease, QPoint(0, 0), QPoint(0, 0), QPoint(0, 0),
                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    btn.mouseReleaseEvent(&ev4);
}

void VCButton_Test::flash()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.setKeySequence(QKeySequence(keySequenceB));
    btn.enableStartupIntensity(false);
    btn.setStartupIntensity(qreal(0.2));

    QSignalSpy spy(sc, SIGNAL(flashing(quint32,bool)));

    m_doc->setMode(Doc::Operate);
    btn.slotKeyPressed(QKeySequence(keySequenceB));
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 0);
    QCOMPARE(btn.state(), VCButton::Active);
    QCOMPARE(sc->getAttributeValue(Function::Intensity), qreal(1.0));
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0].toUInt(), sc->id());
    QCOMPARE(spy[0][1].toBool(), true);

    btn.slotKeyReleased(QKeySequence(keySequenceB));
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spy[1][0].toUInt(), sc->id());
    QCOMPARE(spy[1][1].toBool(), false);

    btn.slotFunctionFlashing(sc->id() + 1, true);
    QCOMPARE(btn.state(), VCButton::Inactive);

    m_doc->setMode(Doc::Design);
}

void VCButton_Test::input()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    sc->setValue(0, 0, 255);
    sc->setFadeInSpeed(1000);
    sc->setFadeOutSpeed(1000);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.enableStartupIntensity(true);
    btn.setStartupIntensity(qreal(1.0));
    btn.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(0, 0)));

    btn.slotInputValueChanged(0, 0, 255);
    QCOMPARE(btn.state(), VCButton::Inactive);

    m_doc->setMode(Doc::Operate);

    btn.slotInputValueChanged(0, 0, 255);
    QCOMPARE(btn.state(), VCButton::Active);

    btn.slotInputValueChanged(0, 0, 1);
    QCOMPARE(btn.state(), VCButton::Active);

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(btn.state(), VCButton::Inactive);

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(btn.state(), VCButton::Inactive);

    btn.slotInputValueChanged(0, 0, 1);
    QCOMPARE(btn.state(), VCButton::Active);

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(btn.state(), VCButton::Inactive);

    btn.setAction(VCButton::Toggle);

    btn.slotInputValueChanged(0, 0, 255);
    m_doc->masterTimer()->timerTick();
    QCOMPARE(btn.state(), VCButton::Active);
    QCOMPARE(sc->getAttributeValue(Function::Intensity), btn.startupIntensity());

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(sc->m_stop, false);

    btn.slotInputValueChanged(0, 0, 255);
    QCOMPARE(sc->m_stop, true);

    // Test that blackout gets toggled thru ext input
    btn.setAction(VCButton::Blackout);
    btn.slotInputValueChanged(0, 0, 1);
    QCOMPARE(m_doc->inputOutputMap()->blackout(), true);
    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(m_doc->inputOutputMap()->blackout(), true);
    btn.slotInputValueChanged(0, 0, 255);
    QCOMPARE(m_doc->inputOutputMap()->blackout(), false);
    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(m_doc->inputOutputMap()->blackout(), false);

    // Test that panic gets toggled thru ext input
    m_doc->masterTimer()->startFunction(sc);
    QCOMPARE(m_doc->masterTimer()->runningFunctions(), 1);
    m_doc->masterTimer()->start();
    btn.setAction(VCButton::StopAll);
    btn.slotInputValueChanged(0, 0, 1);
    QTest::qWait(100);
    QCOMPARE(m_doc->masterTimer()->runningFunctions(), 0);
    m_doc->masterTimer()->stop();
}

void VCButton_Test::paint()
{
    QWidget w;

    VCButton btn(&w, m_doc);

    w.show();
    btn.show();

    QTest::qWait(1);

    btn.setState(VCButton::Active);
    btn.update();
    QTest::qWait(1);
    btn.setState(VCButton::Inactive);
    btn.update();
    QTest::qWait(1);
    btn.setIconPath("../../../resources/icons/png/qlcplus.png");
    btn.update();
    QTest::qWait(1);
    btn.setCaption("Foobar");
    btn.update();
    QTest::qWait(1);
    btn.setAction(VCButton::Flash);
    btn.update();
    QTest::qWait(1);
    m_doc->setMode(Doc::Operate);
    btn.update();
    QTest::qWait(1);
}

void VCButton_Test::toggleAndFlash()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);

    VCButton toggleBtn(&w, m_doc);
    toggleBtn.setFunction(sc->id());
    toggleBtn.setAction(VCButton::Toggle);
    toggleBtn.setKeySequence(QKeySequence(keySequenceA));

    VCButton flashBtn(&w, m_doc);
    flashBtn.setFunction(sc->id());
    flashBtn.setAction(VCButton::Flash);
    flashBtn.setKeySequence(QKeySequence(keySequenceB));

    m_doc->setMode(Doc::Operate);

    // push toggle button
    toggleBtn.slotKeyPressed(QKeySequence(keySequenceA));
    // tell MasterTimer to process start queue
    m_doc->masterTimer()->timerTick();
    QCOMPARE(toggleBtn.state(), VCButton::Active);
    QCOMPARE(flashBtn.state(), VCButton::Inactive);

    // push flash button
    flashBtn.slotKeyPressed(QKeySequence(keySequenceB));
    QCOMPARE(toggleBtn.state(), VCButton::Active);
    QCOMPARE(flashBtn.state(), VCButton::Active);

    // flash button released
    flashBtn.slotKeyReleased(QKeySequence(keySequenceB));
    QCOMPARE(toggleBtn.state(), VCButton::Active);
    QCOMPARE(flashBtn.state(), VCButton::Inactive);

    // push toggle button once more
    toggleBtn.slotKeyPressed(QKeySequence(keySequenceA));
    // tell MasterTimer to process start queue
    m_doc->masterTimer()->timerTick();
    QCOMPARE(toggleBtn.state(), VCButton::Inactive);
    QCOMPARE(flashBtn.state(), VCButton::Inactive);
    QCOMPARE(sc->m_stop, true);
    toggleBtn.slotFunctionStopped(sc->id());
}

QTEST_MAIN(VCButton_Test)
