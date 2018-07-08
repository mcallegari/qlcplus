/*
  Q Light Controller Plus
  virtualconsole.js
  
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

/* VCButton */
function buttonPress(id) {
 websocket.send(id + "|255");
}

function buttonRelease(id) {
 websocket.send(id + "|0");
}

function wsSetButtonState(id, state) {
 var obj = document.getElementById(id);
 if (state === "255") {
  obj.value = "255";
  obj.style.border = "3px solid #00E600";
 } else if (state === "127") {
  obj.value = "127";
  obj.style.border = "3px solid #FFAA00";
 } else {
  obj.value = "0";
  obj.style.border = "3px solid #A0A0A0";
 }
}

/* VCCueList */
var cueListsIndices = new Array();

function setCueIndex(id, idx) {
 var oldIdx = cueListsIndices[id];
 if (oldIdx != undefined && oldIdx !== "-1") {
   var oldCueObj = document.getElementById(id + "_" + oldIdx);
   oldCueObj.style.backgroundColor="#FFFFFF";
 }
 cueListsIndices[id] = idx;
 var currCueObj = document.getElementById(id + "_" + idx);
 if (idx !== "-1") {
   currCueObj.style.backgroundColor="#5E7FDF";
 }
}

function sendCueCmd(id, cmd) {
 if (cmd === "PLAY") {
   var obj = document.getElementById("play" + id);
   if (cueListsIndices[id] === "-1") {
     obj.innerHTML = "<img src=\"player_pause.png\" width=\"27\">";
     setCueIndex(id, "0");
   }
   else {
     obj.innerHTML = "<img src=\"player_play.png\" width=\"27\">";
   }
 }
 websocket.send(id + "|" + cmd);
}

function checkMouseOut(id, idx) {
 var obj = document.getElementById(id + "_" + idx);
 if(idx == cueListsIndices[id]) {
   obj.style.backgroundColor="#5E7FDF";
 }
 else {
   obj.style.backgroundColor="#FFFFFF";
 }
}

function enableCue(id, idx) {
 var btnObj = document.getElementById("play" + id);
 btnObj.innerHTML = "<img src=\"player_pause.png\" width=\"27\">";
 setCueIndex(id, idx);
 websocket.send(id + "|STEP|" + idx);
}

function wsSetCueIndex(id, idx) {
 setCueIndex(id, idx);
 var playObj = document.getElementById("play" + id);
 if (idx === "-1") {
    playObj.innerHTML = "<img src=\"player_play.png\" width=\"27\">";
 }
 else {
    playObj.innerHTML = "<img src=\"player_pause.png\" width=\"27\">";
 }
}

/* VCFrame */
var framesWidth = new Array();
var framesHeight = new Array();
var framesTotalPages = new Array();
var framesCurrentPage = new Array();

function updateFrameLabel(id) {
 var framePageObj = document.getElementById("fr" + id + "Page");
 var newLabel = "Page " + (framesCurrentPage[id] + 1);
 framePageObj.innerHTML = newLabel;
}

function frameToggleCollapse(id) {
  var frameObj = document.getElementById("fr" + id);
  var mpHeader = document.getElementById("frMpHdr" + id);
  var origWidth = framesWidth[id];
  var origHeight = framesHeight[id];

  if (frameObj.clientWidth === origWidth)
  {
    frameObj.style.width = "200px";
    if (mpHeader) {
      mpHeader.style.visibility = "hidden";
    }
  }
  else
  {
    frameObj.style.width = origWidth + "px";
    if (mpHeader) {
      mpHeader.style.visibility = "visible";
    }
  }

  if (frameObj.clientHeight === origHeight) {
    frameObj.style.height = "36px";
  }
  else {
    frameObj.style.height = origHeight + "px";
  }
}

function frameNextPage(id) {
 websocket.send(id + "|NEXT_PG");
}

function framePreviousPage(id) {
 websocket.send(id + "|PREV_PG");
}

function setFramePage(id, page) {
 var iPage = parseInt(page);
 if (framesCurrentPage[id] === iPage || iPage >= framesTotalPages[id]) { return; }
 var framePageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
 framePageObj.style.visibility = "hidden";
 framesCurrentPage[id] = iPage;
 var frameNewPageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
 frameNewPageObj.style.visibility = "visible";
 updateFrameLabel(id);
}

/* VCSlider */
function slVchange(id) {
 var slObj = document.getElementById(id);
 var sldMsg = id + "|" + slObj.value;
 websocket.send(sldMsg);
}

function wsSetSliderValue(id, sliderValue, displayValue) {
 var obj = document.getElementById(id);
 obj.value = sliderValue;
 var labelObj = document.getElementById("slv" + id);
 labelObj.innerHTML = displayValue;
}

/* VCAudioTriggers */
function atButtonClick(id) {
 var obj = document.getElementById(id);
 if (obj.value === "0" || obj.value == undefined) {
  obj.value = "255";
 }
 else {
  obj.value = "0";
 }
 var btnMsg = id + "|" + obj.value;
 websocket.send(btnMsg);
}

function wsSetAudioTriggersEnabled(id, enabled) {
 var obj = document.getElementById(id);
 if (enabled === "255") {
  obj.value = "255";
  obj.style.border = "3px solid #00E600";
  obj.style.backgroundColor = "#D7DE75";
 }
 else {
  obj.value = "0";
  obj.style.border = "3px solid #A0A0A0";
  obj.style.backgroundColor = "#D6D2D0";
 }
}
