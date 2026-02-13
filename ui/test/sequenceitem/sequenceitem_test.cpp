/*
  Q Light Controller Plus - Unit test
  sequenceitem_test.cpp

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
#include <QPainter>
#include <QPixmap>

#define private public
#define protected public
#include "sequenceitem.h"
#include "showfunction.h"
#include "chaserstep.h"
#include "sequence.h"
#include "chaser.h"
#include "scene.h"
#include "show.h"
#include "doc.h"
#undef private
#undef protected

#include "sequenceitem_test.h"

void SequenceItem_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void SequenceItem_Test::cleanupTestCase()
{
    delete m_doc;
}

void SequenceItem_Test::fadeOutClampedToWidth()
{
    Show *show = new Show(m_doc);
    m_doc->addFunction(show);

    Scene *scene = new Scene(m_doc);
    m_doc->addFunction(scene);

    Sequence *seq = new Sequence(m_doc);
    seq->setBoundSceneID(scene->id());
    m_doc->addFunction(seq);

    // Add a step with a very large fade-out
    ChaserStep step;
    step.fid = scene->id();
    step.fadeIn = 0;
    step.hold = 1000;
    step.fadeOut = 50000;  // Very large fade-out to exceed item bounds
    step.duration = step.fadeIn + step.hold;
    seq->addStep(step);

    ShowFunction sf(0);
    sf.setFunctionID(seq->id());
    sf.setStartTime(0);
    sf.setDuration(2000);  // Short duration, fade-out should be clamped

    SequenceItem seqItem(seq, &sf);

    // M1 regression: render should not paint beyond item width
    QPixmap pixmap(seqItem.boundingRect().size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    seqItem.paint(&painter, nullptr, nullptr);
    painter.end();

    // Check that no painted pixels exist beyond the item width
    QImage img = pixmap.toImage();
    int itemWidth = seqItem.m_width;
    bool beyondBounds = false;
    for (int x = itemWidth; x < img.width(); x++)
    {
        for (int y = 0; y < img.height(); y++)
        {
            if (img.pixelColor(x, y).alpha() > 0)
            {
                beyondBounds = true;
                break;
            }
        }
        if (beyondBounds) break;
    }
    QCOMPARE(beyondBounds, false);
}

void SequenceItem_Test::fadeInRendering()
{
    Show *show = new Show(m_doc);
    m_doc->addFunction(show);

    Scene *scene = new Scene(m_doc);
    m_doc->addFunction(scene);

    Sequence *seq = new Sequence(m_doc);
    seq->setBoundSceneID(scene->id());
    m_doc->addFunction(seq);

    ChaserStep step;
    step.fid = scene->id();
    step.fadeIn = 500;
    step.hold = 1000;
    step.fadeOut = 0;
    step.duration = step.fadeIn + step.hold;
    seq->addStep(step);

    ShowFunction sf(0);
    sf.setFunctionID(seq->id());
    sf.setStartTime(0);

    SequenceItem seqItem(seq, &sf);

    // Should render without crash
    QPixmap pixmap(seqItem.boundingRect().size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    seqItem.paint(&painter, nullptr, nullptr);
    painter.end();

    QVERIFY(pixmap.width() > 0);
}

QTEST_MAIN(SequenceItem_Test)
