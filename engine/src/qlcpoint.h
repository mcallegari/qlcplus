/*
  Q Light Controller
  qlcpoint.h

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

#ifndef QLCPOINT_H
#define QLCPOINT_H

#include <QPoint>

/** @addtogroup engine Engine
 * @{
 */

class QLCPoint : public QPoint
{
public:
    QLCPoint();
    QLCPoint(int x, int y);

    /** Comparator function for qSort() */
    bool operator< (const QLCPoint& pt) const;
};

uint qHash(const QLCPoint& key);

/** @} */

#endif
