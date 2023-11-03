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

function initVirtualConsole() {
  updateTime();
}

/* VCButton */
function buttonPress(id) {
  websocket.send(id + "|255");
}

function buttonRelease(id) {
  websocket.send(id + "|0");
}

function wsSetButtonState(id, state) {
  var btnObj = document.getElementById(id);
  if (state === "255") {
    btnObj.value = "255";
    btnObj.style.border = "3px solid #00E600";
  } else if (state === "127") {
    btnObj.value = "127";
    btnObj.style.border = "3px solid #FFAA00";
  } else {
    btnObj.value = "0";
    btnObj.style.border = "3px solid #A0A0A0";
  }
}

window.addEventListener("load",() => {
  var buttons = document.getElementsByClassName("vcbutton");
  for (var btn of buttons) {
    btn.addEventListener("touchstart", (event) => {
      event.preventDefault();
      buttonPress(event.target.id);
    }, false);
    btn.addEventListener("touchend", (event) => {
      event.preventDefault();
      buttonRelease(event.target.id);
    }, false);
  }
});

function setButtonDisableState(id, disable) {
  var btnObj = document.getElementById(id);
  if (disable === "1") {
    btnObj.removeAttribute("onmousedown");
    btnObj.removeAttribute("onmouseup");
    btnObj.classList.add('vcbutton-disable');
  } else {
    btnObj.setAttribute("onmousedown", "buttonPress("+id+");");
    btnObj.setAttribute("onmouseup", "buttonRelease("+id+");");
    btnObj.classList.remove('vcbutton-disable');
  }
}

/* VCCueList */
var cueListsIndices = [];

function setCueIndex(id, idx) {
  var oldIdx = cueListsIndices[id];
  if (oldIdx !== undefined && oldIdx !== "-1") {
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
    } else {
      obj.innerHTML = "<img src=\"player_play.png\" width=\"27\">";
    }
  }
  websocket.send(id + "|" + cmd);
}

function checkMouseOut(id, idx) {
  var obj = document.getElementById(id + "_" + idx);
  if(idx === cueListsIndices[id]) {
    obj.style.backgroundColor="#5E7FDF";
  } else {
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
  } else {
    playObj.innerHTML = "<img src=\"player_pause.png\" width=\"27\">";
  }
}

/* VCFrame */
var framesWidth = [];
var framesHeight = [];
var framesTotalPages = [];
var framesCurrentPage = [];

var enableStatus = [];
var caption = [];

function updateFrameLabel(id) {
  var framePageObj = document.getElementById("fr" + id + "Page");
  var newLabel = "Page " + (framesCurrentPage[id] + 1);
  framePageObj.innerHTML = newLabel;

  var frameCaptionObj = document.getElementById("fr" + id + "Caption");
  var frMpHdr = document.getElementById("frMpHdr" + id);
  var newCaption = caption[id];
  if (frMpHdr) { // if multi page mode
    newCaption = caption[id] ? caption[id] + " - " + newLabel : newLabel;
  }
  frameCaptionObj.innerHTML = newCaption;
}

function frameToggleCollapse(id) {
  var frameObj = document.getElementById("fr" + id);
  var vcframeHeader = document.getElementById("vcframeHeader" + id);
  var frEnBtn = document.getElementById("frEnBtn" + id);
  var frMpHdrPrev = document.getElementById("frMpHdrPrev" + id);
  var frMpHdrNext = document.getElementById("frMpHdrNext" + id);
  var frPglbl = document.getElementById("frPglbl" + id);
  var origWidth = framesWidth[id];
  var origHeight = framesHeight[id];
  var ew = frEnBtn ? 36 : 0;
  var pw = 0;
  if (frameObj.clientWidth === origWidth) {
    pw = frMpHdrPrev && frMpHdrNext ? 64 : 0;
    frameObj.style.width = "200px";
    if (frPglbl) frPglbl.style.width = "60px";
    if (frMpHdrPrev) frMpHdrPrev.style.display = "none";
    if (frMpHdrNext) frMpHdrNext.style.display = "none";
    vcframeHeader.style.width = (200 - pw - ew - 36) + "px";
  } else {
    pw = frMpHdrPrev && frMpHdrNext ? 168 : 0;
    frameObj.style.width = origWidth + "px";
    if (frPglbl) frPglbl.style.width = "100px";
    if (frMpHdrPrev) frMpHdrPrev.style.display = "block";
    if (frMpHdrNext) frMpHdrNext.style.display = "block";
    vcframeHeader.style.width = (origWidth - pw - ew - 36) + "px";
  }
  if (frameObj.clientHeight === origHeight) {
    frameObj.style.height = "36px";
  } else {
    frameObj.style.height = origHeight + "px";
  }
}

function frameChangeEnableStatus(id) {
  websocket.send(id + "|ENABLE|" + enableStatus[id]);
}

function setFramEnableStatus(id, status) {
  var frameObj = document.getElementById("frEnBtn" + id);
  if (parseInt(status) === 1) {
    enableStatus[id] = 1;
    frameObj.style.background = "#D7DE75";
  } else {
    enableStatus[id] = 0;
    frameObj.style.background = "#E0DFDF";
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
  var frameNewPageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
  framePageObj.style.visibility = "hidden";
  framesCurrentPage[id] = iPage;
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
  if (obj.value === "0" || obj.value === undefined) {
    obj.value = "255";
  } else {
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
  } else {
    obj.value = "0";
    obj.style.border = "3px solid #A0A0A0";
    obj.style.backgroundColor = "#D6D2D0";
  }
}

/* VCClock */
function hmsToString(h, m, s) {
  h = (h < 10) ? "0" + h : h;
  m = (m < 10) ? "0" + m : m;
  s = (s < 10) ? "0" + s : s;

  var timeString = h + ":" + m + ":" + s;
  return timeString;
}

function updateTime() {
  var date = new Date();
  var h = date.getHours();
  var m = date.getMinutes();
  var s = date.getSeconds();

  var timeString = hmsToString(h, m, s);
  var clocks = document.getElementsByClassName("vcclock");
  for (var clk of clocks) {
    clk.innerHTML = timeString;
  }

  if (clocks.length)
  setTimeout(updateTime, 1000);
}

function controlWatch(id, op) {
  var obj = document.getElementById(id);
  var msg = id + "|" + op;
  websocket.send(msg);
}

function wsUpdateClockTime(id, time) {
  var obj = document.getElementById(id);
  var s = time;
  var h, m;
  h = parseInt(s / 3600);
  s -= (h * 3600);
  m = parseInt(s / 60);
  s -= (m * 60);

  var timeString = hmsToString(h, m, s);
  obj.innerHTML = timeString;
}
