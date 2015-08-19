/*
  Q Light Controller Plus
  vcbutton.h

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

#ifndef VCBUTTON_H
#define VCBUTTON_H

#include "vcwidget.h"

#define KXMLQLCVCButton "Button"

class VCButton : public VCWidget
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCButton(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCButton();

    void setID(quint32 id);

    void render(QQuickView *view, QQuickItem *parent);

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(const QDomElement* root);
    //bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif
