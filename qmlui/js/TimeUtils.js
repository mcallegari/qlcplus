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

/**
 * Returns a string from the provided time in milliseconds.
 * It builds a stripped version of the time where
 * - hours are not displayed if equal to 0
 * - milliseconds are not displayed if equal to 0
 * The format is hh:mm:ss.xx
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
        finalTime += "." + ((ms < 10) ? "0" + parseInt(ms) : parseInt(ms))

    return finalTime
}

/**
 * Returns a string from the provided time in milliseconds,
 * considering the requested precision to display miliseconds.
 *
 * It always returns a full string in the format: hh:mm:ss.xx
 * Precision can be 1 (during playback) or 2 (when stopped)
 */
function msToStringWithPrecision(ms, precision)
{
    var h = Math.floor(ms / 3600000)
    ms -= (h * 3600000)

    var m = Math.floor(ms / 60000)
    ms -= (m * 60000)

    var s = Math.floor(ms / 1000)
    ms -= (s * 1000)

    var finalTime = ""
    finalTime += ((h < 10) ? "0" + h : h) + ":"
    finalTime += ((m < 10) ? "0" + m : m) + ":"
    finalTime += ((s < 10) ? "0" + s : s)
    if (precision === 1)
        finalTime += "." + ((ms < 10) ? "0" : parseInt(ms / 100))
    else
    {
        if (ms < 10)
            finalTime += ".00"
        else if (ms < 100)
            finalTime += ".0" + parseInt(ms / 10)
        else
            finalTime += "." + parseInt(ms / 10)
    }

    return finalTime
}

function posToMs(x, timescale)
{
    //100 : 1000 / timescale = x : result
    return parseInt(10 * timescale * x)
}

function timeToSize(time, timescale)
{
    return ((time * 100) / 1000) / timescale;
}
