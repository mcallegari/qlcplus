/*
  Q Light Controller Plus - Test Unit
  vcwidget_test.cpp

  Copyright (c) Massimo Callegari

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

#include "vcwidget_test.h"
#include "qlcinputsource.h"
#include "vctestenv.h"
#include "vcwidget.h"

void VCWidget_Test::init()
{
    m_env = new VCTestEnv();
}

void VCWidget_Test::cleanup()
{
    delete m_env;
}

void VCWidget_Test::initial()
{
    VCWidget w(m_env->doc());

    QCOMPARE(w.id(), VCWidget::invalidId());
    QCOMPARE(w.type(), int(VCWidget::UnknownWidget));
    QCOMPARE(w.geometry(), QRectF(0, 0, 0, 0));
    QCOMPARE(w.scaleFactor(), qreal(1.0));
    QCOMPARE(w.zIndex(), 0);
    QCOMPARE(w.allowResize(), true);
    QCOMPARE(w.isDisabled(), false);
    QCOMPARE(w.isVisible(), true);
    QVERIFY(w.caption().isEmpty());
    QCOMPARE(w.backgroundColor(), QColor("#333"));
    QCOMPARE(w.hasCustomBackgroundColor(), false);
    QVERIFY(w.backgroundImage().isEmpty());
    QCOMPARE(w.foregroundColor(), QColor(Qt::white));
    QCOMPARE(w.hasCustomForegroundColor(), false);
    QCOMPARE(w.hasCustomFont(), false);
    QCOMPARE(w.font().family(), QString("Roboto Condensed"));
    QCOMPARE(w.page(), 0);
    QCOMPARE(w.intensity(), qreal(1.0));
    QCOMPARE(w.isEditing(), false);
    QCOMPARE(w.externalControlsCount(), 0);
    QVERIFY(w.inputSources().isEmpty());
    QVERIFY(w.keySequenceMap().isEmpty());
    QCOMPARE(w.supportsPresets(), false);
    QVERIFY(w.presetsResource().isEmpty());
    QVERIFY(w.propertiesResource().isEmpty());
    QVERIFY(w.renderItem() == nullptr);
    QVERIFY(w.hasSoloParent() == false);

    /* The base class can't be copied, loaded or saved */
    QVERIFY(w.createCopy(&w) == nullptr);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    QCOMPARE(w.saveXML(&xmlWriter), false);

    VCXMLReader xml(QByteArray("<Widget/>"));
    QCOMPARE(w.loadXML(xml.reader()), false);

    /* The base class has no default caption */
    w.setID(5);
    QCOMPARE(w.id(), quint32(5));
    QVERIFY(w.caption().isEmpty());
}

void VCWidget_Test::typeConversions()
{
    QCOMPARE(VCWidget::stringToType("Button"), VCWidget::ButtonWidget);
    QCOMPARE(VCWidget::stringToType("Slider"), VCWidget::SliderWidget);
    QCOMPARE(VCWidget::stringToType("Knob"), VCWidget::SliderWidget);
    QCOMPARE(VCWidget::stringToType("XYPad"), VCWidget::XYPadWidget);
    QCOMPARE(VCWidget::stringToType("Frame"), VCWidget::FrameWidget);
    QCOMPARE(VCWidget::stringToType("Solo frame"), VCWidget::SoloFrameWidget);
    QCOMPARE(VCWidget::stringToType("Speed"), VCWidget::SpeedWidget);
    QCOMPARE(VCWidget::stringToType("Cue list"), VCWidget::CueListWidget);
    QCOMPARE(VCWidget::stringToType("Label"), VCWidget::LabelWidget);
    QCOMPARE(VCWidget::stringToType("Audio Triggers"), VCWidget::AudioTriggersWidget);
    QCOMPARE(VCWidget::stringToType("Animation"), VCWidget::AnimationWidget);
    QCOMPARE(VCWidget::stringToType("Clock"), VCWidget::ClockWidget);
    QCOMPARE(VCWidget::stringToType("Foobar"), VCWidget::UnknownWidget);
    QCOMPARE(VCWidget::stringToType(QString()), VCWidget::UnknownWidget);

    QCOMPARE(VCWidget::typeToString(VCWidget::ButtonWidget), QString("Button"));
    QCOMPARE(VCWidget::typeToString(VCWidget::SliderWidget), QString("Slider"));
    QCOMPARE(VCWidget::typeToString(VCWidget::FrameWidget), QString("Frame"));
    QCOMPARE(VCWidget::typeToString(VCWidget::SoloFrameWidget), QString("Solo Frame"));
    QCOMPARE(VCWidget::typeToString(VCWidget::SpeedWidget), QString("Speed"));
    QCOMPARE(VCWidget::typeToString(VCWidget::XYPadWidget), QString("XY Pad"));
    QCOMPARE(VCWidget::typeToString(VCWidget::CueListWidget), QString("Cue list"));
    QCOMPARE(VCWidget::typeToString(VCWidget::LabelWidget), QString("Label"));
    QCOMPARE(VCWidget::typeToString(VCWidget::AudioTriggersWidget), QString("Audio Triggers"));
    QCOMPARE(VCWidget::typeToString(VCWidget::AnimationWidget), QString("Animation"));
    QCOMPARE(VCWidget::typeToString(VCWidget::ClockWidget), QString("Clock"));
    QCOMPARE(VCWidget::typeToString(VCWidget::UnknownWidget), QString("Unknown"));
    QCOMPARE(VCWidget::typeToString(1000), QString("Unknown"));

    QCOMPARE(VCWidget::typeToIcon(VCWidget::ButtonWidget), QString("qrc:/button.svg"));
    QCOMPARE(VCWidget::typeToIcon(VCWidget::AudioTriggersWidget), QString("qrc:/audioinput.svg"));
    QCOMPARE(VCWidget::typeToIcon(VCWidget::AnimationWidget), QString("qrc:/rgbmatrix.svg"));
    QCOMPARE(VCWidget::typeToIcon(VCWidget::UnknownWidget), QString("qrc:/virtualconsole.svg"));
}

void VCWidget_Test::geometry()
{
    VCWidget w(m_env->doc());
    QSignalSpy spy(&w, &VCWidget::geometryChanged);

    w.setGeometry(QRectF(10, 20, 30, 40));
    QCOMPARE(w.geometry(), QRectF(10, 20, 30, 40));
    QCOMPARE(spy.count(), 1);

    /* Same geometry: no change */
    w.setGeometry(QRectF(10, 20, 30, 40));
    QCOMPARE(spy.count(), 1);

    /* The geometry is stored unscaled and returned scaled */
    w.setScaleFactor(2.0);
    QCOMPARE(w.scaleFactor(), qreal(2.0));
    QCOMPARE(w.geometry(), QRectF(20, 40, 60, 80));
    QCOMPARE(spy.count(), 2);

    w.setScaleFactor(2.0);
    QCOMPARE(spy.count(), 2);

    w.setGeometry(QRectF(50, 50, 100, 100));
    QCOMPARE(w.geometry(), QRectF(50, 50, 100, 100));
    w.setScaleFactor(1.0);
    QCOMPARE(w.geometry(), QRectF(25, 25, 50, 50));
}

void VCWidget_Test::properties()
{
    VCWidget w(m_env->doc());

    QSignalSpy captionSpy(&w, &VCWidget::captionChanged);
    w.setCaption("Foo");
    QCOMPARE(w.caption(), QString("Foo"));
    QCOMPARE(captionSpy.count(), 1);
    QCOMPARE(captionSpy.at(0).at(0).toString(), QString("Foo"));
    w.setCaption("Foo");
    QCOMPARE(captionSpy.count(), 1);

    QSignalSpy zSpy(&w, &VCWidget::zIndexChanged);
    w.setZIndex(3);
    QCOMPARE(w.zIndex(), 3);
    QCOMPARE(zSpy.count(), 1);
    w.setZIndex(3);
    QCOMPARE(zSpy.count(), 1);

    QSignalSpy resizeSpy(&w, &VCWidget::allowResizeChanged);
    w.setAllowResize(false);
    QCOMPARE(w.allowResize(), false);
    QCOMPARE(resizeSpy.count(), 1);
    w.setAllowResize(false);
    QCOMPARE(resizeSpy.count(), 1);

    m_env->doc()->resetModified();
    QSignalSpy disableSpy(&w, &VCWidget::disabledStateChanged);
    w.setDisabled(true);
    QCOMPARE(w.isDisabled(), true);
    QCOMPARE(disableSpy.count(), 1);
    QCOMPARE(m_env->doc()->isModified(), true);
    w.setDisabled(true);
    QCOMPARE(disableSpy.count(), 1);
    w.setDisabled(false);
    QCOMPARE(w.isDisabled(), false);
    QCOMPARE(disableSpy.count(), 2);

    QSignalSpy visibleSpy(&w, &VCWidget::isVisibleChanged);
    w.setVisible(false);
    QCOMPARE(w.isVisible(), false);
    QCOMPARE(visibleSpy.count(), 1);
    w.setVisible(false);
    QCOMPARE(visibleSpy.count(), 1);

    QSignalSpy pageSpy(&w, &VCWidget::pageChanged);
    w.setPage(2);
    QCOMPARE(w.page(), 2);
    QCOMPARE(pageSpy.count(), 1);
    w.setPage(2);
    QCOMPARE(pageSpy.count(), 1);

    QSignalSpy editSpy(&w, &VCWidget::isEditingChanged);
    w.setIsEditing(true);
    QCOMPARE(w.isEditing(), true);
    QCOMPARE(editSpy.count(), 1);
    w.setIsEditing(true);
    QCOMPARE(editSpy.count(), 1);
    w.setIsEditing(false);
    QCOMPARE(w.isEditing(), false);
    QCOMPARE(editSpy.count(), 2);

    w.adjustIntensity(0.5);
    QCOMPARE(w.intensity(), qreal(0.5));

    /* setupLookAndFeel sets the default font size and the page */
    w.setupLookAndFeel(10, 4);
    QCOMPARE(w.page(), 4);
    QCOMPARE(w.font().pixelSize(), 27);
}

void VCWidget_Test::colors()
{
    VCWidget w(m_env->doc());

    QSignalSpy bgSpy(&w, &VCWidget::backgroundColorChanged);
    w.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(w.backgroundColor(), QColor(Qt::red));
    QCOMPARE(w.hasCustomBackgroundColor(), true);
    QCOMPARE(bgSpy.count(), 1);
    w.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(bgSpy.count(), 1);

    w.resetBackgroundColor();
    QCOMPARE(w.backgroundColor(), QColor("#333"));
    QCOMPARE(w.hasCustomBackgroundColor(), false);
    QCOMPARE(bgSpy.count(), 2);

    /* Background image */
    QSignalSpy imgSpy(&w, &VCWidget::backgroundImageChanged);
    w.setBackgroundImage("/tmp/foo.png");
    QCOMPARE(w.backgroundImage(), QString("/tmp/foo.png"));
    QCOMPARE(imgSpy.count(), 1);
    w.setBackgroundImage("/tmp/foo.png");
    QCOMPARE(imgSpy.count(), 1);

    /* URLs are converted to local paths */
    w.setBackgroundImage("file:///tmp/bar.png");
    QCOMPARE(w.backgroundImage(), QString("/tmp/bar.png"));
    QCOMPARE(imgSpy.count(), 2);

    /* Setting a background color clears the background image */
    w.setBackgroundColor(QColor(Qt::blue));
    QVERIFY(w.backgroundImage().isEmpty());
    QCOMPARE(imgSpy.count(), 3);

    /* Foreground color */
    QSignalSpy fgSpy(&w, &VCWidget::foregroundColorChanged);
    w.setForegroundColor(QColor(Qt::green));
    QCOMPARE(w.foregroundColor(), QColor(Qt::green));
    QCOMPARE(w.hasCustomForegroundColor(), true);
    QCOMPARE(fgSpy.count(), 1);
    w.setForegroundColor(QColor(Qt::green));
    QCOMPARE(fgSpy.count(), 1);

    w.resetForegroundColor();
    QCOMPARE(w.foregroundColor(), QColor(Qt::white));
    QCOMPARE(w.hasCustomForegroundColor(), false);
    QCOMPARE(fgSpy.count(), 2);
}

void VCWidget_Test::font()
{
    VCWidget w(m_env->doc());

    /* The default font size doesn't make the font custom */
    w.setDefaultFontSize(20);
    QCOMPARE(w.font().pixelSize(), 20);
    QCOMPARE(w.hasCustomFont(), false);

    QSignalSpy spy(&w, &VCWidget::fontChanged);
    QFont font("Arial", 12);
    w.setFont(font);
    QCOMPARE(w.font(), font);
    QCOMPARE(w.hasCustomFont(), true);
    QCOMPARE(spy.count(), 1);

    /* Same custom font: no change */
    w.setFont(font);
    QCOMPARE(spy.count(), 1);

    w.resetFont();
    QCOMPARE(w.font().family(), QString("Roboto Condensed"));
    QCOMPARE(w.font().pixelSize(), 16);
    QCOMPARE(w.hasCustomFont(), false);
    QCOMPARE(spy.count(), 2);
}

void VCWidget_Test::externalControls()
{
    VCWidget w(m_env->doc());

    w.registerExternalControl(3, "Bar", false);
    w.registerExternalControl(0, "Foo", true);
    QCOMPARE(w.externalControlsCount(), 2);

    /* The list is sorted by control ID */
    QVariantList list = w.externalControlsList().toList();
    QCOMPARE(list.count(), 2);
    QCOMPARE(list.at(0).toMap().value("mLabel").toString(), QString("Foo"));
    QCOMPARE(list.at(0).toMap().value("mValue").toInt(), 0);
    QCOMPARE(list.at(1).toMap().value("mLabel").toString(), QString("Bar"));
    QCOMPARE(list.at(1).toMap().value("mValue").toInt(), 3);

    /* Registering an existing ID updates it */
    w.registerExternalControl(3, "Baz", false);
    QCOMPARE(w.externalControlsCount(), 2);
    list = w.externalControlsList().toList();
    QCOMPARE(list.at(1).toMap().value("mLabel").toString(), QString("Baz"));

    QCOMPARE(w.unregisterExternalControl(3), true);
    QCOMPARE(w.unregisterExternalControl(3), false);
    QCOMPARE(w.externalControlsCount(), 1);
}

void VCWidget_Test::inputSources()
{
    VCWidget w(m_env->doc());
    QSignalSpy spy(&w, &VCWidget::inputSourcesListChanged);

    /* Sources are refused when the widget has no external controls */
    QSharedPointer<QLCInputSource> src1(new QLCInputSource(0, 10));
    src1->setID(QLCInputSource::invalidID);
    w.addInputSource(src1);
    QCOMPARE(w.inputSources().count(), 0);
    QCOMPARE(spy.count(), 0);

    /* A source without ID gets the first registered control ID */
    w.registerExternalControl(1, "Foo", true);
    w.registerExternalControl(2, "Bar", true);
    w.addInputSource(src1);
    QCOMPARE(w.inputSources().count(), 1);
    QCOMPARE(src1->id(), quint32(1));
    QCOMPARE(spy.count(), 1);

    /* null sources are ignored */
    w.addInputSource(QSharedPointer<QLCInputSource>());
    QCOMPARE(w.inputSources().count(), 1);

    QVERIFY(w.inputSource(1, 0, 10) == src1);
    QVERIFY(w.inputSource(2, 0, 10).isNull());
    QVERIFY(w.inputSource(1, 1, 10).isNull());

    QVariantList list = w.inputSourcesList();
    QCOMPARE(list.count(), 1);
    QVariantMap map = list.first().toMap();
    QCOMPARE(map.value("type").toInt(), int(VCWidget::Controller));
    QCOMPARE(map.value("id").toUInt(), quint32(1));
    QCOMPARE(map.value("universe").toUInt(), quint32(0));
    QCOMPARE(map.value("channel").toUInt(), quint32(10));
    QCOMPARE(map.contains("invalid"), false);

    /* Update universe and channel. The page is taken from the widget */
    w.setPage(2);
    QVERIFY(w.updateInputSource(src1, 1, 20) == true);
    QCOMPARE(src1->universe(), quint32(1));
    QCOMPARE(src1->channel() & 0xFFFF, quint32(20));
    QCOMPARE(src1->page(), ushort(2));
    QVERIFY(w.updateInputSource(QSharedPointer<QLCInputSource>(), 1, 20) == false);

    /* Back to page 0 to make the lookups by channel easier */
    src1->setPage(0);
    QCOMPARE(src1->channel(), quint32(20));

    QVERIFY(w.updateInputSourceControlID(1, 20, 2) == true);
    QCOMPARE(src1->id(), quint32(2));
    QVERIFY(w.updateInputSourceControlID(5, 5, 2) == false);

    QVERIFY(w.updateInputSourceFeedbackValues(1, 20, 10, 200, 100) == true);
    QCOMPARE(src1->feedbackValue(QLCInputFeedback::LowerValue), uchar(10));
    QCOMPARE(src1->feedbackValue(QLCInputFeedback::UpperValue), uchar(200));
    QCOMPARE(src1->feedbackValue(QLCInputFeedback::MonitorValue), uchar(100));
    QVERIFY(w.updateInputSourceFeedbackValues(5, 5, 1, 2, 3) == false);

    /* Extra params are 1-based in the UI and 0-based in the source */
    QVERIFY(w.updateInputSourceExtraParams(1, 20, 2, 3, 4) == true);
    QCOMPARE(src1->feedbackExtraParams(QLCInputFeedback::LowerValue).toInt(), 1);
    QCOMPARE(src1->feedbackExtraParams(QLCInputFeedback::UpperValue).toInt(), 2);
    QCOMPARE(src1->feedbackExtraParams(QLCInputFeedback::MonitorValue).toInt(), 3);
    QVERIFY(w.updateInputSourceExtraParams(5, 5, 1, 2, 3) == false);

    /* Remap all the sources to another page */
    QSharedPointer<QLCInputSource> src2(new QLCInputSource(0, 30));
    src2->setID(1);
    w.addInputSource(src2);
    QCOMPARE(w.inputSources().count(), 2);
    w.remapInputSources(3);
    QCOMPARE(src1->page(), ushort(3));
    QCOMPARE(src2->page(), ushort(3));
    QCOMPARE(src2->channel() & 0xFFFF, quint32(30));

    /* Delete: the ID, universe and channel must all match */
    int count = spy.count();
    w.deleteInputSurce(1, 1, src1->channel());
    QCOMPARE(w.inputSources().count(), 2);
    QCOMPARE(spy.count(), count);
    w.deleteInputSurce(2, 1, src1->channel());
    QCOMPARE(w.inputSources().count(), 1);
    QCOMPARE(spy.count(), count + 1);

    w.deleteAllInputSources();
    QCOMPARE(w.inputSources().count(), 0);
    QCOMPARE(spy.count(), count + 2);
}

void VCWidget_Test::keySequences()
{
    VCWidget w(m_env->doc());
    QSignalSpy spy(&w, &VCWidget::inputSourcesListChanged);

    m_env->doc()->resetModified();
    w.addKeySequence(QKeySequence("A"), 1);
    QCOMPARE(w.keySequenceMap().count(), 1);
    QCOMPARE(w.keySequenceMap().value(QKeySequence("A")), quint32(1));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_env->doc()->isModified(), true);

    w.updateKeySequence(QKeySequence("A"), QKeySequence("Ctrl+B"), 2);
    QCOMPARE(w.keySequenceMap().count(), 1);
    QVERIFY(w.keySequenceMap().contains(QKeySequence("A")) == false);
    QCOMPARE(w.keySequenceMap().value(QKeySequence("Ctrl+B")), quint32(2));
    QCOMPARE(spy.count(), 2);

    w.updateKeySequenceControlID(QKeySequence("Ctrl+B"), 5);
    QCOMPARE(w.keySequenceMap().value(QKeySequence("Ctrl+B")), quint32(5));

    QVariantList list = w.inputSourcesList();
    QCOMPARE(list.count(), 1);
    QVariantMap map = list.first().toMap();
    QCOMPARE(map.value("type").toInt(), int(VCWidget::Keyboard));
    QCOMPARE(map.value("id").toUInt(), quint32(5));
    QCOMPARE(map.value("keySequence").toString(), QString("Ctrl+B"));

    w.deleteKeySequence(QKeySequence("Ctrl+B"));
    QCOMPARE(w.keySequenceMap().count(), 0);
    QCOMPARE(spy.count(), 3);

    w.addKeySequence(QKeySequence("C"), 0);
    w.addKeySequence(QKeySequence("D"), 0);
    QCOMPARE(w.keySequenceMap().count(), 2);
    w.deleteAllKeySequences();
    QCOMPARE(w.keySequenceMap().count(), 0);
}

void VCWidget_Test::saveLoadInputControlUnified()
{
    VCWidget w(m_env->doc());
    w.registerExternalControl(0, "Foo", true);

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 5));
    src->setID(0);
    src->setFeedbackValue(QLCInputFeedback::LowerValue, 10);
    src->setFeedbackValue(QLCInputFeedback::UpperValue, 200);
    src->setFeedbackValue(QLCInputFeedback::MonitorValue, 100);
    w.addInputSource(src);
    w.addKeySequence(QKeySequence("Ctrl+K"), 0);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    QVERIFY(w.saveXMLInputControl(&xmlWriter, 0, true, "Control"));
    xmlWriter.writeEndDocument();
    buffer.close();

    /* <Control><Input ID="0" Universe="0" Channel="5" ... Key="Ctrl+K"/></Control> */
    VCXMLReader xml(buffer.data());
    QCOMPARE(xml.reader().name().toString(), QString("Control"));
    QVERIFY(xml.reader().readNextStartElement());
    QCOMPARE(xml.reader().name().toString(), QString("Input"));

    VCWidget w2(m_env->doc());
    w2.registerExternalControl(0, "Foo", true);
    QVERIFY(w2.loadXMLInputSource(xml.reader(), 0));

    QCOMPARE(w2.inputSources().count(), 1);
    QSharedPointer<QLCInputSource> src2 = w2.inputSources().first();
    QCOMPARE(src2->id(), quint32(0));
    QCOMPARE(src2->universe(), quint32(0));
    QCOMPARE(src2->channel(), quint32(5));
    QCOMPARE(src2->feedbackValue(QLCInputFeedback::LowerValue), uchar(10));
    QCOMPARE(src2->feedbackValue(QLCInputFeedback::UpperValue), uchar(200));
    QCOMPARE(src2->feedbackValue(QLCInputFeedback::MonitorValue), uchar(100));
    QCOMPARE(w2.keySequenceMap().count(), 1);
    QCOMPARE(w2.keySequenceMap().value(QKeySequence("Ctrl+K")), quint32(0));

    /* Nothing is written for a control without sources and keys */
    QBuffer emptyBuffer;
    emptyBuffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter emptyWriter(&emptyBuffer);
    QVERIFY(w.saveXMLInputControl(&emptyWriter, 7, true, "Control"));
    QVERIFY(emptyBuffer.data().isEmpty());

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Foo Universe=\"0\" Channel=\"1\"/>"));
    QCOMPARE(w2.loadXMLInputSource(wrongXml.reader(), 0), false);
}

void VCWidget_Test::saveLoadInputControlSplit()
{
    VCWidget w(m_env->doc());
    w.registerExternalControl(2, "Foo", true);

    QSharedPointer<QLCInputSource> src(new QLCInputSource(1, 7));
    src->setID(2);
    w.addInputSource(src);
    w.addKeySequence(QKeySequence("Q"), 2);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    xmlWriter.writeStartElement("Root");
    QVERIFY(w.saveXMLInputControl(&xmlWriter, 2, false));
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    buffer.close();

    /* <Root><Input ID="2" Universe="1" Channel="7"/><Key>Q</Key></Root> */
    VCXMLReader xml(buffer.data());
    VCWidget w2(m_env->doc());
    w2.registerExternalControl(2, "Foo", true);

    int inputs = 0, keys = 0;
    while (xml.reader().readNextStartElement())
    {
        if (xml.reader().name() == QString("Input"))
        {
            QVERIFY(w2.loadXMLInputSource(xml.reader(), 2));
            inputs++;
        }
        else if (xml.reader().name() == QString("Key"))
        {
            QVERIFY(w2.loadXMLInputKey(xml.reader(), 2));
            keys++;
        }
        else
        {
            QFAIL("Unexpected tag");
        }
    }
    QCOMPARE(inputs, 1);
    QCOMPARE(keys, 1);

    QCOMPARE(w2.inputSources().count(), 1);
    QCOMPARE(w2.inputSources().first()->id(), quint32(2));
    QCOMPARE(w2.inputSources().first()->universe(), quint32(1));
    QCOMPARE(w2.inputSources().first()->channel(), quint32(7));
    QCOMPARE(w2.keySequenceMap().value(QKeySequence("Q")), quint32(2));

    /* An empty key is refused */
    VCXMLReader emptyKey(QByteArray("<Key></Key>"));
    QCOMPARE(w2.loadXMLInputKey(emptyKey.reader(), 2), false);
}

QTEST_MAIN(VCWidget_Test)
