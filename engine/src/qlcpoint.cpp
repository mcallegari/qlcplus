/*
  Q Light Controller
  qlcpoint.cpp

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

#include <QPair>

#include "qlcpoint.h"

QLCPoint::QLCPoint()
    : QPoint()
{
}

QLCPoint::QLCPoint(int x, int y)
    : QPoint(x, y)
{
}

bool QLCPoint::operator<(const QLCPoint &pt) const
{
    if (y() < pt.y())
        return true;
    if (y() == pt.y() && x() < pt.x())
        return true;

    return false;
}

uint qHash(const QLCPoint& key)
{
    uint hash;
    hash = key.x() << 16;
    hash = hash | (key.y() & 0xFFFF);
    hash = hash & (~0U);
    return hash;
}
