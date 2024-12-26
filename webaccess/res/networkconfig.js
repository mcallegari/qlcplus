/*
  Q Light Controller Plus
  networkconfig.js

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

function systemCmd(cmd, iface, mode, addr, mask, gw, ssid, wpapsk)
{
 websocket.send("QLC+SYS|" + cmd + "|" + iface + "|" + mode + "|" + addr + "|" + mask + "|" + gw + "|" + ssid + "|" + wpapsk);
}

function showStatic(iface, enable) {
 document.getElementById(iface + "IPaddr").disabled = !enable;
 document.getElementById(iface + "Netmask").disabled = !enable;
 document.getElementById(iface + "Gateway").disabled = !enable;
}

function applyParams(iface) {
 var radioGroup = iface + "NetGroup";
 var radios = document.getElementsByName(radioGroup);
 var ssidObj = document.getElementById(iface+"SSID");
 var ssidVal = "";
 if (ssidObj != null) { ssidVal = ssidObj.value; }
 var wpapskObj = document.getElementById(iface+"WPAPSK");
 var wpapskVal = "";
 if (wpapskObj != null) { wpapskVal = wpapskObj.value; }
 if (radios[0].checked) {
   systemCmd("NETWORK", iface, "dhcp", "", "", "", ssidVal, wpapskVal);
 } else if (radios[1].checked) {
   var addrName=iface+"IPaddr";
   var maskName=iface+"Netmask";
   var gwName=iface+"Gateway";
   systemCmd("NETWORK", iface, "static", document.getElementById(addrName).value,
      document.getElementById(maskName).value, document.getElementById(gwName).value, ssidVal, wpapskVal);
 }
}

function enableHotspot(enable) {
 var ssidObj = document.getElementById("hotspotSSID");
 var ssidVal = "";
 if (ssidObj != null) { ssidVal = ssidObj.value; }
 var wpapskObj = document.getElementById("hotspotWPAPSK");
 var wpapskVal = "";
 if (wpapskObj != null) { wpapskVal = wpapskObj.value; }
 systemCmd("HOTSPOT", enable ? "1" : "0", ssidVal, wpapskVal);
}


function setAutostart() {
 var radios = document.getElementsByName("autostart");
 if (radios[0].checked) {
   websocket.send("QLC+SYS|AUTOSTART|none");
 } else {
   websocket.send("QLC+SYS|AUTOSTART|current");
 }
}
