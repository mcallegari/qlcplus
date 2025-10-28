/*
  Q Light Controller - Unit tests
  qlcfixturedef_test.h

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

#ifndef QLCFIXTUREDEF_TEST_H
#define QLCFIXTUREDEF_TEST_H

#include <QObject>

class QLCFixtureDef_Test final : public QObject
{
    Q_OBJECT

private slots:
    void initial();
    void manufacturer();
    void model();
    void name();
    void type();
    void addChannel();
    void removeChannel();
    void channel();
    void channels();
    void addMode();
    void removeMode();
    void mode();
    void modes();
    void copy();
    void saveLoadXML();
};

#endif
