/*
  Q Light Controller
  stubwidget.cpp

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

#include <QDomDocument>
#include <QDomElement>

#include "stubwidget.h"

StubWidget::StubWidget(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
{
}

StubWidget::~StubWidget()
{
}

VCWidget* StubWidget::createCopy(VCWidget* parent)
{
    return parent;
}

bool StubWidget::loadXML(const QDomElement* vc_root)
{
    Q_UNUSED(vc_root);
    return true;
}

bool StubWidget::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    Q_UNUSED(doc);
    Q_UNUSED(vc_root);
    return true;
}
