/*
  Q Light Controller Plus
  TimeUtils.js

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

function msToString(ms)
{
    var h = Math.floor(ms / 3600000)
    ms -= (h * 3600000)

    var m = Math.floor(ms / 60000)
    ms -= (m * 60000)

    var s = Math.floor(ms / 1000)
    ms -= (s * 1000)

    var finalTime = ""
    if (h)
        finalTime += ((h < 10) ? "0" + h : h) + ":"

    finalTime += ((m < 10) ? "0" + m : m) + ":"
    finalTime += ((s < 10) ? "0" + s : s)
    if (ms)
        finalTime += "." + ((ms < 10) ? "0" + ms : ms)

    return finalTime
}

function posToMs(x, timescale)
{
    //100 : 1000 / timescale = x : res
    return 10 * timescale * x
}

function timeToSize(time, timescale)
{
    return ((time * 100) / 1000) / timescale;
}
