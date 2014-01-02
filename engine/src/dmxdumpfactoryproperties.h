/*
  Q Light Controller Plus
  dmxdumpfactoryproperties.h

  Copyright (c) Massimo Callegari

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

#ifndef DMXDUMPFACTORYPROPERTIES_H
#define DMXDUMPFACTORYPROPERTIES_H

#include <QByteArray>
#include <QList>

/** @addtogroup engine Engine
 * @{
 */

class DmxDumpFactoryProperties
{
public:
    DmxDumpFactoryProperties(int universes);

    bool dumpChannelsMode();
    void setDumpChannelsMode(bool mode);

    bool nonZeroValuesMode();
    void setNonZeroValuesMode(bool mode);

    QByteArray channelsMask();
    void setChannelsMask(QByteArray mask);

    void addChaserID(quint32 id);
    void removeChaserID(quint32 id);
    bool isChaserSelected(quint32 id);

    void setSelectedTarget(int idx);
    int selectedTarget();

private:
    /** array of bytes holding 0s and 1s where 0 is 'inactive channel' and
     *  1 is 'active channel'
     */
    QByteArray m_channelsMask;
    QList<quint32> m_selectedChaserIDs;

    bool m_dumpAllChannels;
    bool m_dumpNonZeroValues;

    int m_selectedTarget;
};

/** @} */

#endif // DMXDUMPFACTORYPROPERTIES_H
