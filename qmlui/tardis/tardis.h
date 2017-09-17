/*
  Q Light Controller Plus
  tardis.h

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

#ifndef TARDIS_H
#define TARDIS_H

#include <QObject>
#include <QQuickView>

#include "tardisactions.h"

class FixtureManager;
class FunctionManager;
class ShowManager;
class VirtualConsole;
class Doc;

class Tardis : public QObject
{
public:
    explicit Tardis(QQuickView *view, Doc *doc,
                    FixtureManager *fxMgr, FunctionManager *funcMgr,
                    ShowManager *showMgr, VirtualConsole *vc,
                    QObject *parent = 0);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;

    /** Reference to the Fixture Manager */
    FixtureManager *m_fixtureManager;
    /** Reference to the Function Manager */
    FunctionManager *m_functionManager;
    /** Reference to the Show Manager */
    ShowManager *m_showManager;
    /** Reference to the Virtual Console */
    VirtualConsole *m_virtualConsole;
};

#endif /* TARDIS_H */
