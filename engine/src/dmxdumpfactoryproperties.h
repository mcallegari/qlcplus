/*
  Q Light Controller Plus
  dmxdumpfactoryproperties.h

  Copyright (c) Massimo Callegari

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

#ifndef DMXDUMPFACTORYPROPERTIES_H
#define DMXDUMPFACTORYPROPERTIES_H

#include <QByteArray>
#include <QList>

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

#endif // DMXDUMPFACTORYPROPERTIES_H
