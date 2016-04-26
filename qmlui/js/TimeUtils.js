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

/**
  * Returns the number of milliseconds from the given string
  * in the QLC+ format. Example: 2h34m12s870ms
  * Returns -2 on infinite time string
  */
function qlcStringToMs(str)
{
    if (str === "∞")
        return -2;

    var finalTime = 0

    var currStr = ""

    for (var i = 0; i < str.length; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            currStr += str[i]
        }
        else if (str[i] === 'h')
        {
            console.log("Hours: " + currStr)
            finalTime += parseInt(currStr) * 1000 * 60 * 60
            currStr = ""
        }
        else if (str[i] === 'm' && str[i + 1] === 's')
        {
            console.log("Millisecs: " + currStr)
            finalTime += parseInt(currStr)
            break
        }
        else if (str[i] === 'm')
        {
            console.log("minutes: " + currStr)
            finalTime += parseInt(currStr) * 1000 * 60
            currStr = ""
        }
        else if (str[i] === 's')
        {
            console.log("seconds: " + currStr)
            finalTime += parseInt(currStr) * 1000
            currStr = ""
        }
    }

    return finalTime
}

function msToQlcString(ms)
{
    if (ms === -2)
        return "∞"

    var h = Math.floor(ms / 3600000)
    ms -= (h * 3600000)

    var m = Math.floor(ms / 60000)
    ms -= (m * 60000)

    var s = Math.floor(ms / 1000)
    ms -= (s * 1000)

    //console.log("h: " + h + ", m: " + m + ", s: " + s + ", ms: " + ms)

    var timeString = ""

    if (h)
        timeString += ((h < 10) ? "0" + h : h) + "h"
    if (m)
        timeString += ((m < 10) ? "0" + m : m) + "m"
    if (s)
        timeString += ((s < 10) ? "0" + s : s) + "s"

    if (ms)
    {
        if (ms < 10)
            timeString = timeString + "00" + ms + "ms"
        else if (ms < 100)
            timeString = timeString + "0" + ms + "ms"
        else
            timeString = timeString + ms + "ms"
    }

    //console.log("Final time string: " + timeString)

    return timeString
}

/**
  * Return a value in milliseconds, for the given
  * position in pixels and the given timescale,
  * where 100px corresponds to 1 second on a 1.0 timescale factor
  */
function posToMs(x, timescale)
{
    //100 : 1000 / timescale = x : result
    return parseInt(10 * timescale * x)
}

/**
  * Return a value in pixels, for the given
  * time in milliseconds and the given timescale,
  * where 100px corresponds to 1 second on a 1.0 timescale factor
  */
function timeToSize(time, timescale)
{
    return ((time * 100) / 1000) / timescale;
}
