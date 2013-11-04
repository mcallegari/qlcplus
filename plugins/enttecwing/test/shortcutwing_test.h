/*
  Q Light Controller
  shortcutwing_test.h

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

#ifndef SHORTCUTWING_TEST_H
#define SHORTCUTWING_TEST_H

#include <QByteArray>
#include <QObject>

class ShortcutWing;
class ShortcutWing_Test : public QObject
{
    Q_OBJECT

private:
    QByteArray data();

private slots:
    void initTestCase();

    void firmware();
    void address();
    void isOutputData();
    void name();
    void infoText();
    void tooShortData();

    void buttons_data();
    void buttons();

    void cleanupTestCase();

private:
    ShortcutWing* m_wing;
};

#endif
