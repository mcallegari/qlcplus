/*
  Q Light Controller Plus - Test Unit
  vcbutton_test.h

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

#ifndef VCBUTTON_TEST_H
#define VCBUTTON_TEST_H

#include <QObject>

class VCTestEnv;
class Scene;

class VCButton_Test final : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void initial();
    void actionStrings();
    void properties();
    void functionAttachment();
    void toggle();
    void flash();
    void flashReleasedWhenHidden();
    void blackout();
    void inputValue();
    void copy();
    void saveLoad();
    void saveLoadStopAll();

private:
    Scene *createScene(QString name);

private:
    VCTestEnv *m_env;
};

#endif
