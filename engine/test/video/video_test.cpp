/*
  Q Light Controller Plus - Unit test
  video_test.cpp

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
#include <QBuffer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QVector3D>
#include <QRect>

#define protected public
#define private public
#include "mastertimer_stub.h"
#include "video_test.h"
#include "video.h"
#include "doc.h"
#undef private
#undef protected

void Video_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void Video_Test::cleanupTestCase()
{
    delete m_doc;
}

void Video_Test::cleanup()
{
    m_doc->clearContents();
}

void Video_Test::basic()
{
    Video v(m_doc);
    QCOMPARE(v.type(), Function::VideoType);
    QCOMPARE(v.name(), QString("New Video"));
    QVERIFY(v.sourceUrl().isEmpty());
    QVERIFY(v.isPicture() == false);
    QCOMPARE(v.fullscreen(), false);
}

void Video_Test::properties()
{
    Video v(m_doc);
    v.setSourceUrl("http://example.com/pic.png");
    v.setCustomGeometry(QRect(1,2,3,4));
    v.setRotation(QVector3D(1,2,3));
    v.setZIndex(5);
    v.setScreen(2);
    v.setFullscreen(true);

    QCOMPARE(v.sourceUrl(), QString("http://example.com/pic.png"));
    QVERIFY(v.isPicture());
    QCOMPARE(v.zIndex(), 5);
    QCOMPARE(v.screen(), 2);
    QCOMPARE(v.fullscreen(), true);
    QCOMPARE(v.customGeometry(), QRect(1,2,3,4));
    QCOMPARE(v.rotation(), QVector3D(1,2,3));
}

void Video_Test::saveLoad()
{
    Video v(m_doc);
    v.setSourceUrl("http://example.com/movie.mp4");
    v.setCustomGeometry(QRect(10,20,30,40));
    v.setRotation(QVector3D(4,5,6));
    v.setZIndex(7);
    v.setScreen(3);
    v.setFullscreen(true);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    v.saveXML(&xmlWriter);
    xmlWriter.setDevice(nullptr);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Video v2(m_doc);
    QVERIFY(v2.loadXML(xmlReader));
    QCOMPARE(v2.sourceUrl(), QString("http://example.com/movie.mp4"));
#ifdef QMLUI
    QCOMPARE(v2.customGeometry(), QRect(10,20,30,40));
    QCOMPARE(v2.rotation(), QVector3D(4,5,6));
    QCOMPARE(v2.zIndex(), 7);
#endif
    QCOMPARE(v2.screen(), 3);
    QCOMPARE(v2.fullscreen(), true);
}

QTEST_MAIN(Video_Test)
