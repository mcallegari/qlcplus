/*
  Q Light Controller Plus - Test Unit
  vcproperties_test.cpp

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

#ifndef VCPROPERTIES_TEST_H
#define VCPROPERTIES_TEST_H

#include <QObject>

class Doc;
class VCProperties_Test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void initial();
    void copy();
    void loadXMLSad();
    void loadXMLHappy();
    void loadXMLHappyNoInput();
    void loadXMLInput();
    void saveXML();

private:
    Doc* m_doc;
};

#endif
