/*
  Q Light Controller - Unit tests
  qlcphysical_test.h

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

#ifndef QLCPHYSICAL_TEST_H
#define QLCPHYSICAL_TEST_H

#include <QObject>
#include "qlcphysical.h"

class QLCPhysical_Test : public QObject
{
    Q_OBJECT

private slots:
    void bulbType();
    void bulbLumens();
    void bulbColourTemp();

    void weight();
    void width();
    void height();
    void depth();

    void lensName();
    void lensDegreesMin();
    void lensDegreesMax();

    void focusType();
    void focusPanMax();
    void focusTiltMax();
    void layoutSize();

    void powerConsumption();
    void dmxConnector();

    void copy();
    void load();
    void loadWrongRoot();
    void save();

private:
    QLCPhysical p;
};

#endif
