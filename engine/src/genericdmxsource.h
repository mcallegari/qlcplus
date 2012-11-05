/*
  Q Light Controller
  genericdmxsource.h

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

#ifndef GENERICDMXSOURCE_H
#define GENERICDMXSOURCE_H

#include <QMutex>
#include <QPair>
#include <QMap>

#include "scenevalue.h"
#include "dmxsource.h"

class Doc;

/**
 * This is a generic DMX source, that registers itself to doc->masterTimer() when
 * started and unregisters when deleted. Values set with set() are written to
 * UniverseArray on each writeDMX() call (called by MasterTimer); HTP values continuously
 * and LTP values only once (after which they will be removed from m_values).
 */
class GenericDMXSource : public DMXSource
{
public:
    GenericDMXSource(Doc* doc);
    ~GenericDMXSource();

    /** Set the value of a fixture channel */
    void set(quint32 fxi, quint32 ch, uchar value);

    /** Unset the value of a fixture channel */
    void unset(quint32 fxi, quint32 ch);

    /** Enable/disable output */
    void setOutputEnabled(bool enable);

    /** Check, whether output is enabled */
    bool isOutputEnabled() const;

    /** @reimp */
    void writeDMX(MasterTimer* timer, UniverseArray* ua);

private:
    Doc* m_doc;
    QMutex m_mutex;
    QMap <QPair<quint32,quint32>,uchar> m_values;
    bool m_outputEnabled;
};

#endif
