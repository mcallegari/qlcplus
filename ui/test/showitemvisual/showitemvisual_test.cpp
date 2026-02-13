/*
  Q Light Controller Plus - Unit test
  showitemvisual_test.cpp

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
#include "rgbmatrixitem.h"
#include "efxitem.h"
#include "showfunction.h"
#include "rgbmatrix.h"
#include "show.h"
#include "efx.h"
#include "doc.h"
#undef private
#undef protected

#include "showitemvisual_test.h"

void ShowItemVisual_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void ShowItemVisual_Test::cleanupTestCase()
{
    delete m_doc;
}

void ShowItemVisual_Test::efxZeroDuration()
{
    EFX *efx = new EFX(m_doc);
    efx->setDuration(0);  // Zero duration
    m_doc->addFunction(efx);

    ShowFunction sf(0);
    sf.setFunctionID(efx->id());
    sf.setStartTime(0);
    sf.setDuration(5000);

    EFXItem efxItem(efx, &sf);

    // C2 regression: paint with zero duration must not crash (divide-by-zero)
    QPixmap pixmap(200, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    efxItem.paint(&painter, nullptr, nullptr);
    painter.end();

    // If we get here, no crash occurred
    QVERIFY(true);

    m_doc->deleteFunction(efx->id());
}

void ShowItemVisual_Test::rgbmatrixZeroDuration()
{
    RGBMatrix *rgbm = new RGBMatrix(m_doc);
    rgbm->setDuration(0);  // Zero duration
    m_doc->addFunction(rgbm);

    ShowFunction sf(0);
    sf.setFunctionID(rgbm->id());
    sf.setStartTime(0);
    sf.setDuration(5000);

    RGBMatrixItem rgbItem(rgbm, &sf);

    // C2 regression: paint with zero duration must not crash (divide-by-zero)
    QPixmap pixmap(200, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    rgbItem.paint(&painter, nullptr, nullptr);
    painter.end();

    QVERIFY(true);

    m_doc->deleteFunction(rgbm->id());
}

QTEST_MAIN(ShowItemVisual_Test)
