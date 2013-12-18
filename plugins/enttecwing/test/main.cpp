/*
  Q Light Controller
  main.cpp

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

#include <QtTest>

#include "playbackwing_test.h"
#include "shortcutwing_test.h"
#include "programwing_test.h"
#include "wing_test.h"

int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);
    int r;

    Wing_Test ewi;
    r = QTest::qExec(&ewi, argc, argv);
    if (r != 0)
        return r;

    PlaybackWing_Test epl;
    r = QTest::qExec(&epl, argc, argv);
    if (r != 0)
        return r;

    ShortcutWing_Test esh;
    r = QTest::qExec(&esh, argc, argv);
    if (r != 0)
        return r;

    ProgramWing_Test epr;
    r = QTest::qExec(&epr, argc, argv);
    if (r != 0)
        return r;

    return 0;
}
