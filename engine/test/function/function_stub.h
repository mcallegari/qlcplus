/*
  Q Light Controller
  function_stub.h

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef FUNCTION_STUB_H
#define FUNCTION_STUB_H

#include "function.h"

class Doc;

class Function_Stub : public Function
{
    Q_OBJECT

public:
    Function_Stub(Doc* doc);
    ~Function_Stub();

    Function* createCopy(Doc* parent, bool addToDoc = true);

    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
    bool loadXML(const QDomElement& root);

    void preRun(MasterTimer* timer);
    void write(MasterTimer* timer, UniverseArray* universes);
    void postRun(MasterTimer* timer, UniverseArray* universes);

public slots:
    void slotFixtureRemoved(quint32 id);

public:
    int m_preRunCalls;
    int m_writeCalls;
    int m_postRunCalls;

    quint32 m_slotFixtureRemovedId;
};

#endif

