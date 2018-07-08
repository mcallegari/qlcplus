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

    /** Return if all the channels or only those
     *  in $m_channelsMask will be dumped */
    bool dumpChannelsMode();

    /** Set the dump channels mode.
     *  True means all channels will be dumped.
     *  False means only the channels in $m_channelsMask
     *  will be dumped */
    void setDumpChannelsMode(bool mode);

    /** Return true if only non-zero values will be dumped.
     *  Otherwise false is returned (meaning any value) */
    bool nonZeroValuesMode();

    /** Set a flag to dump only the channels with non zero values. */
    void setNonZeroValuesMode(bool mode);

    /** Return the current map of the selected DMX channels to dump.
     *  The array has size = universes * 512 */
    QByteArray channelsMask();

    /** Set the map of DMX channels for dump, in the form
     *  of an array of 0/1 valued bytes.
     *  A 0 means the channel is excluded from dump, while
     *  a 1 means the channel is included for dump */
    void setChannelsMask(QByteArray mask);

private:
    /** Array of bytes holding 0s and 1s where 0 is 'inactive channel' and
     *  1 is 'active channel'
     *  The size of this array is the number of universes * 512.
     *  Indices of the array indicate the absolute DMX channel address.
     */
    QByteArray m_channelsMask;

    bool m_dumpAllChannels;
    bool m_dumpNonZeroValues;

    /************************************************************************
     * Dump target
     ***********************************************************************/

public:
    enum TargetType
    {
        Chaser = 0,
        VCButton,
        VCSlider
    };

    /** Add a Chaser ID to the list of Chasers that
     *  will include the dumped Scene */
    void addChaserID(quint32 id);

    /** Remove a Chaser ID from the list of Chasers
     *  included in the dump process */
    void removeChaserID(quint32 id);

    /** Return true if a Chaser ID is selected in the
     *  dump process */
    bool isChaserSelected(quint32 id);

    /** Select the type of dump that will be performed.
     *  See TargetType. */
    void setSelectedTarget(TargetType type);

    /** Return the current target that will be used in
     *  the dump process */
    TargetType selectedTarget();

private:
    /** Variable holding the type of dump going to
     *  be performed. See TargetType */
    TargetType m_selectedTarget;

    /** A list of the Chaser IDs on which the dumped
     *  Scene will be added */
    QList<quint32> m_selectedChaserIDs;
};

/** @} */

#endif // DMXDUMPFACTORYPROPERTIES_H
