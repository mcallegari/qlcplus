/*
  Q Light Controller - Unit test
  efx_test.h

  Copyright (c) Heikki Junnila

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

#ifndef EFX_TEST_H
#define EFX_TEST_H

#include <QObject>

class Doc;
class EFX_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void initial();
    void algorithmNames();
    void stringToAlgorithm();
    void width();
    void height();
    void rotation();
    void xOffset();
    void yOffset();
    void xFrequency();
    void yFrequency();
    void xPhase();
    void yPhase();
    void fixtures();
    void propagationMode();

    void previewCircle();
    void previewEight();
    void previewLine();
    void previewLine2();
    void previewDiamond();
    void previewSquare();
    void previewSquareChoppy();
    void previewSquareTrue();
    void previewLeaf();
    void previewLissajous();

    void previewCircleBackwards();
    void previewEightBackwards();
    void previewLineBackwards();
    void previewLine2Backwards();
    void previewDiamondBackwards();
    void previewSquareBackwards();
    void previewSquareChoppyBackwards();
    void previewLeafBackwards();
    void previewLissajousBackwards();

    void rotateAndScale();
    void widthHeightOffset();

    void copyFrom();
    void createCopy();

    void loadXAxis();
    void loadYAxis();
    void loadYAxisWrongRoot();
    void loadAxisNoXY();
    void loadSuccessLegacy();
    void loadSuccess();
    void loadWrongType();
    void loadWrongRoot();
    void loadDuplicateFixture();
    void save();

    void preRunPostRun();
    void adjustIntensity();

private:
    Doc* m_doc;
};

#endif
