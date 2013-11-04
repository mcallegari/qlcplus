/*
  Q Light Controller
  alsamidiutil.h

  Copyright (c) Heikki Junnila

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

#ifndef ALSAMIDIUTIL_H
#define ALSAMIDIUTIL_H

#include <QVariant>

struct _snd_seq;
typedef _snd_seq snd_seq_t;

struct snd_seq_addr;
typedef snd_seq_addr snd_seq_addr_t;

class AlsaMidiUtil
{
private:
    AlsaMidiUtil() { }
    ~AlsaMidiUtil() { }

public:
    static QVariant addressToVariant(const snd_seq_addr_t* addr);
    static bool variantToAddress(const QVariant& var, snd_seq_addr_t* addr);
    static QString extractName(snd_seq_t* alsa, const snd_seq_addr_t* addr);
};

#endif
