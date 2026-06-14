/*
  Q Light Controller Plus
  idnoptimizer.cpp

  Copyright (c) Daniel Schröder

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

QTextStream out33(stdout);

IdnOptimizer::IdnOptimizer(){
    oldData.fill(0x00, 512);
}

IdnOptimizer::~IdnOptimizer(){}


IdnOptimizer::PacketInformation IdnOptimizer::optimize(const QByteArray& data, const bool checkNullValues){
    QByteArray newData = data;
    QByteArray fill;
    newData.append(fill.fill(0x00, 512), 512-data.length());
    QList<int> changedVal = changedValues(oldData, newData);
    QSet<int> changedValSet(changedVal.begin(), changedVal.end());
    changed = changed.unite(changedValSet);

    oldData = newData;

    QSet<int> temp = changed;

    QMutableSetIterator<int> i(changed);
    while(i.hasNext()){
        int val = i.next();

        if(oldData.at(val) == 0x00){
            nullChannelBuffer << val;
        }

        if(nullChannelBuffer.count(val) > 5 && checkNullValues){
            changed.remove(val);
        }
    }

    QList<int> changedList(changed.begin(), changed.end());
    return getRanges(changedList);
}

QList<int> IdnOptimizer::changedValues(QByteArray oldData, QByteArray newData){
  QList<int> changedChannelBuffer;
  for(int i = 0; i < oldData.length(); i++){
      if(newData.at(i) == oldData.at(i)){
          continue;
        }else{
          changedChannelBuffer << i;
      }
     }
  return changedChannelBuffer;
}

IdnOptimizer::PacketInformation IdnOptimizer::getRanges(QList<int> changed){
    sort(changed.begin(), changed.end(), std::greater<int>());
  QList<QPair<int, int> > resultRanges;
  
  PacketInformation pi;
  pi.numberOfSingleChannels = 0;
  pi.numberOfChannelPacks = 0;
  pi.byteCount = 0;

  QPair<int, int> range;

  for(int i = 0; i < changed.length(); i++){
    if(i > 0 && i < changed.length()-1){
      if(changed[i] - changed[i+1] == 1 && changed[i-1]-changed[i] != 1){
        //start of continues channels
        range.second = changed[i];
        continue;
      }

      if(changed[i] - changed[i+1] != 1 && changed[i-1] - changed[i] == 1){
        //end of contiguous channels
        range.first = changed[i];
        resultRanges.append(range);
        pi.numberOfChannelPacks++;
        pi.byteCount += range.second - range.first+1;
        continue;
      }

      if(changed[i] - changed[i+1] != 1 && changed[i-1] - changed[i] != 1){
        //single channel
        range.first = changed[i];
        range.second = changed[i];
        resultRanges.append(range);
        pi.numberOfSingleChannels++;
        continue;
      }
    }else{
      if(i == 0 && changed.length() > 1 && changed[i]-changed[i+1] == 1){
        //start of contiguous row of channels
        range.second = changed[i];
        continue;
      }
      else if(i == changed.length()-1 && i > 0 && changed[i-1] - changed[i] == 1){
        //end of contiguous row of channels
        range.first = changed[i];
        resultRanges.append(range);
        pi.numberOfChannelPacks++;
        pi.byteCount += range.second - range.first+1;
        continue;
      }else{
        //single channel
        range.first = changed[i];
        range.second = changed[i];
        resultRanges.append(range);
        pi.numberOfSingleChannels++;
        continue;
      }
    }
  }
  pi.byteCount += pi.numberOfSingleChannels;
  pi.ranges = resultRanges;
  return pi;
}
