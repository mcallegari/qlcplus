/*
  Q Light Controller - Unit test
  mastertimer_stub.h

  Copyright (c) Heikki Junnila

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

#ifndef MASTERTIMER_STUB_H
#define MASTERTIMER_STUB_H

#include <QObject>

#include "mastertimer.h"
#include "outputmap.h"
#include "dmxsource.h"

/****************************************************************************
 * MasterTimer Stub
 ****************************************************************************/

class MasterTimerStub : public MasterTimer
{
    Q_OBJECT

public:
    MasterTimerStub(Doc* doc, UniverseArray& universes);
    ~MasterTimerStub();

    void startFunction(Function* function);
    void stopFunction(Function* function);
    QList <Function*> m_functionList;

    void registerDMXSource(DMXSource* source);
    void unregisterDMXSource(DMXSource* source);
    QList <DMXSource*> m_dmxSourceList;

    UniverseArray& m_universes;
};

#endif

