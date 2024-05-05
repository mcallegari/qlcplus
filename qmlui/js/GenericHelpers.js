/*
  Q Light Controller Plus
  GenericHelpers.js

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

function pluginIconFromName(name)
{
    switch (name)
    {
        case "ArtNet": return "qrc:/artnetplugin.svg";
        case "DMX USB": return "qrc:/dmxusbplugin.svg";
        case "HID": return "qrc:/hidplugin.svg";
        case "OLA": return "qrc:/olaplugin.svg";
        case "MIDI": return "qrc:/midiplugin.svg";
        case "OSC": return "qrc:/oscplugin.svg";
        case "E1.31": return "qrc:/e131plugin.svg";
        case "Loopback": return "qrc:/loop.svg";
        default: return "";
    }
}

function getHTMLColor(r, g, b)
{
    var color = r << 16 | g << 8 | b;
    var colStr = color.toString(16);
    return "#" + "000000".substr(0, 6 - colStr.length) + colStr;
}
