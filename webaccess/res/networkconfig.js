function systemCmd(cmd, iface, mode, addr, mask, gw, ssid, wpapsk)
{
 websocket.send("QLC+SYS|" + cmd + "|" + iface + "|" + mode + "|" + addr + "|" + mask + "|" + gw + "|" + ssid + "|" + wpapsk);
}

function showStatic(iface, enable) {
 var divName = iface + "StaticFields";
 var obj=document.getElementById(divName);
 if (enable == true)
   obj.style.visibility='visible';
 else
   obj.style.visibility='hidden';
}

function applyParams(iface) {
 var radioGroup = iface + "NetGroup";
 var radios = document.getElementsByName(radioGroup);
 var ssidObj = document.getElementById(iface+"SSID");
 var ssidVal = '';
 if (ssidObj != null) ssidVal = ssidObj.value;
 var wpapskObj = document.getElementById(iface+"WPAPSK");
 var wpapskVal = '';
 if (wpapskObj != null) wpapskVal = wpapskObj.value;
 if (radios[0].checked)
   systemCmd("NETWORK", iface, "dhcp", '', '', '', ssidVal, wpapskVal);
 else if (radios[1].checked) {
   var addrName=iface+"IPaddr";
   var maskName=iface+"Netmask";
   var gwName=iface+"Gateway";
   systemCmd("NETWORK", iface, "static", document.getElementById(addrName).value,
      document.getElementById(maskName).value, document.getElementById(gwName).value, ssidVal, wpapskVal);
 }
}

function setAutostart() {
 var radios = document.getElementsByName('autostart');
 if (radios[0].checked)
   websocket.send('QLC+SYS|AUTOSTART|none');
 else
   websocket.send('QLC+SYS|AUTOSTART|current');
}