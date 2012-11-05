/*
  Q Light Controller
  grouphead.h

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

#ifndef GROUPHEAD_H
#define GROUPHEAD_H

#include "fixture.h"

class GroupHead
{
public:
    GroupHead(quint32 aFxi = Fixture::invalidId(), int aHead = -1);
    GroupHead(const GroupHead& another);
    ~GroupHead();

    bool isValid() const;
    bool operator==(const GroupHead& another) const;

public:
    quint32 fxi;
    int head;
};

#endif
