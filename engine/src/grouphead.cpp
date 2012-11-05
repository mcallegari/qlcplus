/*
  Q Light Controller
  grouphead.cpp

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

#include "grouphead.h"

GroupHead::GroupHead(quint32 aFxi, int aHead)
    : fxi(aFxi)
    , head(aHead)
{
}

GroupHead::GroupHead(const GroupHead& another)
    : fxi(another.fxi)
    , head(another.head)
{
}

GroupHead::~GroupHead()
{
}

bool GroupHead::isValid() const
{
    if (fxi != Fixture::invalidId() && head >= 0)
        return true;
    else
        return false;
}

bool GroupHead::operator==(const GroupHead& another) const
{
    if (fxi == another.fxi && head == another.head)
        return true;
    else
        return false;
}

