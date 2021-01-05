/*
  Q Light Controller
  rgbmatrix_test.h

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

#ifndef RGBMATRIX_TEST_H
#define RGBMATRIX_TEST_H

#include <QObject>
#include <QList>

#ifdef QT_QML_LIB
  #include "rgbscriptv4.h"
#else
  #include "rgbscript.h"
#endif

class Doc;
class RGBMatrix_Test : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void initial();
    void group();
    void color();
    void copy();
    void previewMaps();
    void property();
    void loadSave();

private:
    Doc* m_doc;
};

#endif
