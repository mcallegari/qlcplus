/*;
  Q Light Controller
  qlcinputsource.h

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

#ifndef QLCINPUTSOURCE_H
#define QLCINPUTSOURCE_H

#include <QtCore>

class InputMap;

class QLCInputSource
{
public:
    static quint32 invalidUniverse;
    static quint32 invalidChannel;

public:
    QLCInputSource();
    QLCInputSource(quint32 universe, quint32 channel);

    bool isValid() const;

    QLCInputSource& operator=(const QLCInputSource& source);
    bool operator==(const QLCInputSource& source) const;

    void setUniverse(quint32 uni);
    quint32 universe() const;

    void setChannel(quint32 ch);
    quint32 channel() const;

    void setPage(ushort pgNum);
    ushort page() const;

private:
    quint32 m_universe;
    quint32 m_channel;
};

#endif
