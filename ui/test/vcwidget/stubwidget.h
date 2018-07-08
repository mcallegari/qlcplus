/*
  Q Light Controller Plus - Test Unit
  stubwidget.h

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

#ifndef STUBWIDGET_H
#define STUBWIDGET_H

#include "vcwidget.h"

class StubWidget : public VCWidget
{
    Q_OBJECT

public:
    StubWidget(QWidget* parent, Doc* doc);
    ~StubWidget();

    VCWidget* createCopy(VCWidget* parent);

    /** @reimp */
    void updateFeedback() { }
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
