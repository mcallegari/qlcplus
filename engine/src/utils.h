/*
  Q Light Controller Plus
  utils.h

  Copyright (c) Massimo Callegari
                David Garyga

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

#ifndef UTILS_H
#define UTILS_H

/** @addtogroup engine Engine
 * @{
 */

struct Utils
{

    static bool vectorRemove(QVector<int>& vec, int val)
    {
        bool ret = false;
        for (int i = 0; i < vec.size(); ++i)
        {
            if (val == vec.at(i))
            {
                vec.remove(i);
                ret = true;
            }
        }
        return ret;
    }

    static void vectorSortedAddUnique(QVector<int>& vec, int val)
    {
        for (int i = 0; i < vec.size(); ++i)
        {
            if (val < vec.at(i))
            {
                vec.insert(i, val);
                return;
            }
            if (val == vec.at(i))
                return;
        }
        vec.append(val);
    }

};

/** @} */

#endif
