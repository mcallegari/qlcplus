/*
  Q Light Controller
  grouphead.h

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

#ifndef GROUPHEAD_H
#define GROUPHEAD_H

#include "fixture.h"

/** @addtogroup engine Engine
 * @{
 */

class GroupHead
{
public:
    GroupHead(quint32 aFxi = Fixture::invalidId(), int aHead = -1);
    GroupHead(const GroupHead& another);
    ~GroupHead();

    GroupHead& operator=(const GroupHead& grp);

    bool isValid() const;
    bool operator==(const GroupHead& another) const;

public:
    quint32 fxi;
    int head;
};

/** @} */

#endif
