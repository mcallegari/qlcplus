/*
  Q Light Controller
  vcxypadfixture_test.h

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

#ifndef VCXYPADFIXTURE_TEST_H
#define VCXYPADFIXTURE_TEST_H

#include <QObject>

class Doc;
class VCXYPadFixture_Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void initial();
    void params();
    void paramsDegrees();
    void fromVariantBelowZero();
    void fromVariantAboveOne();
    void fromVariantWithinRange();
    void fromVariantWrongSize();
    void fromVariantWrongVariant();
    void toVariant();
    void copy();
    void compare();
    void name();

    void loadXMLWrongRoot();
    void loadXMLHappy();
    void loadXMLSad();
    void saveXMLHappy();
    void saveXMLSad();

    void armNoFixture();
    void armDimmer();
    void arm8bit();
    void arm16bit();
    void disarm();

    void writeDimmer();
    void write8bitNoReverse();
    void write8bitReverse();
    void write16bitNoReverse();
    void write16bitReverse();
    void writeRange();
    void writeRange_data();
    void readRange();
    void readRange_data();

    void cleanupTestCase();

private:
    Doc* m_doc;
};

#endif
