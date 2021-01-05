/*
  Q Light Controller Plus
  simpledesk.js

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

var websocket;

function getPage(uni, page) {
 var address = ((page - 1) * channelsPerPage) + 1;
 var wsMsg = "QLC+API|getChannelsValues|" + uni + "|" + address + "|" + channelsPerPage;
 websocket.send(wsMsg);
}

window.onload = function() {
   var url = "ws://" + window.location.host + "/qlcplusWS";
   websocket = new WebSocket(url);
   websocket.onopen = function(ev) {
    getPage(1, 1);
   };
   websocket.onclose = function(ev) {
    alert("QLC+ connection lost!");
   };
   websocket.onerror = function(ev) {
    alert("QLC+ connection error!");
   };
   websocket.onmessage = function(ev) {
    //alert(ev.data);
    var msgParams = ev.data.split("|");
    if (msgParams[0] === "QLC+API") {
      if (msgParams[1] === "getChannelsValues") {
        drawPage(ev.data);
      }
    }
   };
};

function getGroupIconName(grp) {
   if (grp === 0) { return "intensity.png"; }
   else if (grp === 1) { return "colorwheel.png"; }
   else if (grp === 2) { return "gobo.png"; }
   else if (grp === 3) { return "speed.png"; }
   else if (grp === 4) { return "pan.png"; }
   else if (grp === 5) { return "tilt.png"; }
   else if (grp === 6) { return "shutter.png"; }
   else if (grp === 7) { return "prism.png"; }
   else if (grp === 8) { return "beam.png"; }
   else if (grp === 9) { return "star.png"; }
   else if (grp === 10) { return "configure.png"; }
   return "";
}

function getSliderTopCode(type) {
   if (type === "")
   {
      return "<div style='width:34px; height:34px; margin:2px 0 0 1px; background:transparent;'></div>";
   }
   var aType = type.split(".");
   if (aType.length === 1) {
      return "<img src=" + getGroupIconName(parseInt(type)) + " style='margin-left:2px;'>";
   } else {
      if (aType[1] === "#000000") {
         return "<img src=" + getGroupIconName(0) + ">";
      } else {
         return "<div style='width:34px; height:34px; margin:2px 0 0 1px; background:" + aType[1] + ";'></div>";
      }
   }
}

function drawPage(data) {
 var cObj = document.getElementById("slidersContainer");
 var code = "";
 var cVars = data.split("|");
 for (i = 2; i < cVars.length; i+=3) {
     var chNum = parseInt(cVars[i]);
     code += "<div class='sdSlider' style='width: 36px; height: 372px; background-color: #aaa; margin-left:2px;'>";
     code += getSliderTopCode(cVars[i + 2]);
     code += "<div id='sdslv" + chNum + "' class='sdslLabel' style='top:2px;'>" + cVars[i + 1]  + "</div>";
     code += "<input type='range' class='vVertical' id='" + chNum + "' ";
     code += "oninput='sdSlVchange(" + chNum + ");' ontouchmove='sdSlVchange(" + chNum + ");' ";
     code += "style='width: 250px; margin-top: 250px; margin-left: 18px; ";
     code += "min='0' max='255' step='1' value='" + cVars[i + 1] + "' >";
     code += "<div id='sdsln" + chNum + "' class='sdslLabel' ";
     code += "style='bottom:30px;'>" + chNum + "</div>";
     code += "<a class='sdButton' style='margin-left: 1px; width: 30px; height: 30px;' href='javascript:resetChannel(" + chNum + ");'>";
     code += "<img src='fileclose.png' title='Reset channel' width='28'></a>";
     code += "</div>";
 }
 cObj.innerHTML = code;
}

function nextPage() {
 currentPage++;
 if (currentPage * channelsPerPage > 512) {
   currentPage = 1;
 }
 var pgObj = document.getElementById("pageDiv");
 pgObj.innerHTML = currentPage;
 getPage(currentUniverse, currentPage);
}

function previousPage() {
 if (currentPage === 1) {
   currentPage = (512 / channelsPerPage);
 } else {
   currentPage--;
 }
 var pgObj = document.getElementById("pageDiv");
 pgObj.innerHTML = currentPage;
 getPage(currentUniverse, currentPage);
}

function universeChanged(uniIdx) {
 currentUniverse = parseInt(uniIdx) + 1;
 currentPage = 1;
 var pgObj = document.getElementById("pageDiv");
 pgObj.innerHTML = currentPage;
 getPage(currentUniverse, currentPage);
}

function resetChannel(pageCh) {
 var chNum = ((currentUniverse - 1) * 512) + parseInt(pageCh);
 var wsMsg = "QLC+API|sdResetChannel|" + chNum;
 websocket.send(wsMsg);
}

function resetUniverse() {
 currentPage = 1;
 var wsMsg = "QLC+API|sdResetUniverse";
 websocket.send(wsMsg);
}

function sdSlVchange(id) {
 var slObj = document.getElementById(id);
 var labelObj = document.getElementById("sdslv" + id);
 labelObj.innerHTML = slObj.value;
 var chNum = ((currentUniverse - 1) * 512) + parseInt(id);
 var sldMsg = "CH|" + chNum + "|" + slObj.value;
 websocket.send(sldMsg);
}
