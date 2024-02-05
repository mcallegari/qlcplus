/*
  Q Light Controller
  grouphead.cpp

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

GroupHead &GroupHead::operator=(const GroupHead &grp)
{
    if (this != &grp)
    {
        fxi = grp.fxi;
        head = grp.head;
    }

    return *this;
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

