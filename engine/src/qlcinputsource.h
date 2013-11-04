/*;
  Q Light Controller
  qlcinputsource.h

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
