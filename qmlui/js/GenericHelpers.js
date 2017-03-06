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

function fixtureIconFromType(type)
{
    if (type === "Color Changer")
        return "qrc:/fixture.svg";
    else if (type === "Dimmer")
        return "qrc:/dimmer.svg";
    else if (type === "Moving Head")
        return "qrc:/movinghead.svg";
    else if (type === "Flower")
        return "qrc:/flower.svg";
    else if (type === "Effect")
        return "qrc:/effect.svg";
    else if (type === "Laser")
        return "qrc:/laser.svg";
    else
        return "qrc:/fixture.svg";
}
    
function pluginIconFromName(name)
{
    switch(name)
    {
        case "ArtNet": return "qrc:/artnetplugin.svg"; break;
        case "DMX USB": return "qrc:/dmxusbplugin.svg"; break;
        case "HID": return "qrc:/hidplugin.svg"; break;
        case "OLA": return "qrc:/olaplugin.svg"; break;
        case "MIDI": return "qrc:/midiplugin.svg"; break;
        case "OSC": return "qrc:/oscplugin.svg"; break;
        case "E1.31": return "qrc:/e131plugin.svg"; break;
        case "Loopback": return "qrc:/loop.svg"; break;
    }
    return "";
}
