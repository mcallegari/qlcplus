/*
  Q Light Controller Plus
  websocket.js
  
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
function sendCMD(cmd) {
 websocket.send("QLC+CMD|" + cmd);
}

window.onload = function() {
 var url = "ws://" + window.location.host + "/qlcplusWS";
 websocket = new WebSocket(url);
 websocket.onopen = function(ev) {
  //alert(\"Websocket open!\");
 };

 websocket.onclose = function(ev) {
  alert("QLC+ connection lost !");
 };

 websocket.onerror = function(ev) {
  alert("QLC+ connection error!");
 };

 websocket.onmessage = function(ev) {
  //alert(ev.data);
  var msgParams = ev.data.split("|");
  if (msgParams[1] === "BUTTON") {
    wsSetButtonState(msgParams[0], msgParams[2]);
  }
  else if (msgParams[1] === "SLIDER") {
    // Slider message is <ID>|SLIDER|<SLIDER VALUE>|<DISPLAY VALUE>
    wsSetSliderValue(msgParams[0], msgParams[2], msgParams[3]);
  }
  else if (msgParams[1] === "AUDIOTRIGGERS") {
    wsSetAudioTriggersEnabled(msgParams[0], msgParams[2]);
  }
  else if (msgParams[1] === "CUE") {
    wsSetCueIndex(msgParams[0], msgParams[2]);
  }
  else if (msgParams[1] === "FRAME") {
    setFramePage(msgParams[0], msgParams[2]);
  }
  else if (msgParams[0] === "ALERT") {
    alert(msgParams[1]);
  }
 };
};
