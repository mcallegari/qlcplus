/*
  Q Light Controller
  dmxsource.h

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

#ifndef DMXSOURCE_H
#define DMXSOURCE_H

class MasterTimer;
class UniverseArray;

/**
 * DMXSource should be inherited/implemented by such object that wish to
 * write DMX data to QLC's DMX universes. Each DMXSource is polled periodically
 * by MasterTimer for new/changed DMX data.
 */
class DMXSource
{
public:
    virtual ~DMXSource() {}

    /**
     * Write the source's current values to the given universe buffer.
     *
     * @param timer The calling MasterTimer instance
     * @param universes Universe buffer to write to
     */
    virtual void writeDMX(MasterTimer* timer, UniverseArray* universes) = 0;
};

#endif
