/*
  Q Light Controller
  wing_test.h

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

#ifndef WING_TEST_H
#define WING_TEST_H

#include <QObject>
#include "wing.h"

class WingStub final : public Wing
{
    Q_OBJECT
public:
    WingStub(QObject* parent, const QHostAddress& host, const QByteArray& ba);
    ~WingStub();

    QString name() const override;
    void parseData(const QByteArray& ba) override;
};

class Wing_Test final : public QObject
{
    Q_OBJECT

private slots:
    void resolveType();
    void resolveFirmware();
    void isOutputData();
    void initial();
    void page();
    void bcd();
    void cache();
};

#endif
