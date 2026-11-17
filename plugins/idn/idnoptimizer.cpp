/*
  Q Light Controller Plus
  idnoptimizer.cpp

  Copyright (c) Daniel Schröder
  Updated by Mauritz Kauffmann, 2026

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

#include "idnoptimizer.h"

#include <QTextStream>

IdnOptimizer::IdnOptimizer()
{
    oldData.fill(0x00, 512);
    memset(zeroFrames, 0x00, sizeof(zeroFrames));
}

IdnOptimizer::~IdnOptimizer()
{}


IdnOptimizer::PacketInformation IdnOptimizer::optimize(const QByteArray& data, const bool checkNullValues, int rangeBegin, int rangeEnd)
{
    QByteArray newData = data.leftJustified(512, 0x00, true);
    QList<int> changedVal = changedValues(oldData, newData, rangeBegin, rangeEnd);
    QSet<int> changedValSet(changedVal.begin(), changedVal.end());
    changed.unite(changedValSet);

    oldData = newData;

    QMutableSetIterator<int> i(changed);
    while (i.hasNext()) 
    {
        int ch = i.next();

        if (oldData.at(ch) != 0x00) 
        {
            zeroFrames[ch] = 0;
            continue;
        }

        if (zeroFrames[ch] < IDN_ZERO_HOLD_FRAMES) 
        {
            zeroFrames[ch]++;
        } 
        else if (checkNullValues) 
        {
            i.remove();
        }
    }

    QList<int> changedList(changed.begin(), changed.end());
    return getRanges(changedList);
}

QList<int> IdnOptimizer::changedValues(const QByteArray& oldData, const QByteArray& newData, int rangeBegin,
                                       int rangeEnd)
{
    QList<int> changedChannelBuffer;
    for (int i = rangeBegin - 1; i < rangeEnd; i++) 
    {
        if (newData.at(i) == oldData.at(i))
        {
            continue;
        } 
        else 
        {
            changedChannelBuffer << i;
        }
    }
    return changedChannelBuffer;
}

IdnOptimizer::PacketInformation IdnOptimizer::getRanges(QList<int> changed)
{
    sort(changed.begin(), changed.end(), std::less<int>());
    QList<QPair<int, int> > resultRanges;

    PacketInformation pi;
    pi.numberOfSingleChannels = 0;
    pi.numberOfChannelPacks = 0;
    pi.byteCount = 0;

    for (int i = 0; i < changed.length(); i++)
    {
        int first = changed[i];
        int last = first;

        while (i + 1 < changed.length() && changed[i + 1] == last + 1) 
        {
            last = changed[++i];
        }

        pi.ranges.append(qMakePair(first, last));
        pi.byteCount += last - first + 1;

        if(first == last) 
        {
            pi.numberOfSingleChannels++;
        } 
        else 
        {
            pi.numberOfChannelPacks++;
        }
    }
    return pi;
}
