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

/* VCCueList */
var cueListsIndices = new Array();
var showPanel = new Array();

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
 setCueIndex(id, idx);
 websocket.send(id + "|STEP|" + idx);
}

function wsSetCueIndex(id, idx) {
 setCueIndex(id, idx);
}

function setCueProgress(id, percent, text) {
 var progressBarObj = document.getElementById("vccuelistPB" + id);
 var progressValObj = document.getElementById("vccuelistPV" + id);
 progressBarObj.style.width = percent + "%";
 progressValObj.innerHTML = text;
}

function showSideFaderPanel(id, checked) {
  var progressBarObj = document.getElementById("fadePanel" + id);
  showPanel[id] = parseInt(checked);
  if (checked === "1") {
    progressBarObj.style.display="block";
  } else {
    progressBarObj.style.display="none";
  }
}

function setCueButtonStyle(id, playImage, playPaused, stopImage, stopPaused) {
  var playObj = document.getElementById("play" + id);
  var stopObj = document.getElementById("stop" + id);
  playObj.classList.remove("vccuelistButtonPaused");
  stopObj.classList.remove("vccuelistButtonPaused");
  playObj.innerHTML = "<img src=\""+playImage+"\" width=\"27\">";
  stopObj.innerHTML = "<img src=\""+stopImage+"\" width=\"27\">";
  if (playPaused === "1") playObj.classList.add("vccuelistButtonPaused");
  if (stopPaused === "1") stopObj.classList.add("vccuelistButtonPaused");
}

function wsShowCrossfadePanel(id) {
  websocket.send(id + "|CUE_SHOWPANEL|" + showPanel[id]);
}

function setCueSideFaderValues(id, topPercent, bottomPercent, topStep, bottomStep, primaryTop, value, isSteps) {
  var topPercentObj = document.getElementById("cueCTP" + id);
  var bottomPercentObj = document.getElementById("cueCBP" + id);
  var topStepObj = document.getElementById("cueCTS" + id);
  var bottomStepObj = document.getElementById("cueCBS" + id);
  var crossfadeValObj = document.getElementById("cueC" + id);

  if (topPercentObj) topPercentObj.innerHTML = topPercent;
  if (bottomPercentObj) bottomPercentObj.innerHTML = bottomPercent;
  if (topStepObj) topStepObj.innerHTML = topStep;
  if (bottomStepObj) bottomStepObj.innerHTML = bottomStep;
  if (crossfadeValObj) crossfadeValObj.value = value;

  if (primaryTop === "1") {
    if (topStepObj) topStepObj.style.backgroundColor = topStep ? "#4E8DDE" : "inherit";
    if (bottomStepObj) bottomStepObj.style.backgroundColor = isSteps === "1" && bottomStep ? "#4E8DDE" : bottomStep ? "orange" : 'inherit';
  } else {
    if (topStepObj) topStepObj.style.backgroundColor = topStep ? "orange" : "inherit";
    if (bottomStepObj) bottomStepObj.style.backgroundColor = isSteps === "1" || bottomStep ? "#4E8DDE" : "inherit";
  }
}

function cueCVchange(id) {
  var cueCVObj = document.getElementById("cueC" + id);
  var msg = id + "|CUE_SIDECHANGE|" + cueCVObj.value;
  websocket.send(msg);
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

/* VCSlider with Knob */
var isDragging = new Array();
var maxVal = new Array();
var minVal = new Array();
var initVal = new Array();
var inverted = new Array();
var selectedID = 0;

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
 // knob
 getPositionFromValue(sliderValue, id);
}

function getPositionFromValue(val, id) {
  var knobRect = document.getElementById("knob" + id).getBoundingClientRect();
  var pie = document.getElementById("pie" + id);
  var spot = document.getElementById("spot" + id);
  if (!knobRect || !pie || !spot) return;
  var knobRadius = knobRect.width / 2;
  var angle = (340 * (val - minVal[id]) / (maxVal[id] - minVal[id])) % 360;
  if (inverted[id]) angle = 340 - angle;
  var posX = Math.cos((angle - 260) * Math.PI / 180) * knobRadius;
  var posY = Math.sin((angle - 260) * Math.PI / 180) * knobRadius;
  spot.style.transform = `translate(-50%, -50%) translate(${Math.round(posX)}px, ${Math.round(posY)}px)`;
  pie.style.setProperty('--degValue', Math.round(angle));
  if (inverted[id]) {
    pie.style.setProperty('--color1', '#555');
    pie.style.setProperty('--color2', 'lime');
  } else {
    pie.style.setProperty('--color1', 'lime');
    pie.style.setProperty('--color2', '#555');
  }
}

function onMouseMove(e) {
  if (isDragging[selectedID]) {
    pie = document.getElementById("pie" + selectedID);
    knob = document.getElementById("knob" + selectedID);
    spot = document.getElementById("spot" + selectedID);
    knobValue = document.getElementById(selectedID);
    var knobRect = knob.getBoundingClientRect();

    var knobCenterX = knobRect.left + knobRect.width / 2;
    var knobCenterY = knobRect.top + knobRect.height / 2;
    var knobRadius = knobRect.width / 2;

    var deltaX = e.clientX - knobCenterX;
    var deltaY = e.clientY - knobCenterY;
    var distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);

    var newPosX = deltaX * knobRadius / distance;
    var newPosY = deltaY * knobRadius / distance;
    var angle = Math.atan2(deltaY, deltaX);


    var angleDegrees = (angle * 180) / Math.PI;
    var normalizedAngle = (angleDegrees + 260 + 360) % 360; // Adjust for initial rotation and make sure it's positive
    var newValue = Math.round((normalizedAngle / 360) * ((maxVal[selectedID] - minVal[selectedID]) * 18 / 17)) + minVal[selectedID];
    if (inverted[selectedID]) {
      newValue = (maxVal[selectedID] + minVal[selectedID]) - newValue;
    }
    if (newValue >= minVal[selectedID] && newValue <= maxVal[selectedID]) {
      spot.style.transform = `translate(-50%, -50%) translate(${newPosX}px, ${newPosY}px)`;
      pie.style.setProperty('--degValue', normalizedAngle);
      knobValue.value = newValue;
      websocket.send(selectedID + "|" + newValue);
    }
  }
}
function onMouseUp() {
  isDragging[selectedID] = false;
  var knob = document.getElementById("knob" + selectedID);
  knob.style.transition = "transform 0.2s ease";
  document.removeEventListener("mousemove", onMouseMove);
  document.removeEventListener("mouseup", onMouseUp);
  document.removeEventListener("mousedown", onMouseMove);
}
// Initial position
window.addEventListener("load", (event) => {
  var pieWrapper = document.querySelectorAll(".pieWrapper");
  pieWrapper.forEach(function(item) {
    item.addEventListener("mousedown", () => {
      selectedID = item.getAttribute("data");
      isDragging[selectedID] = true;
      var knob = document.getElementById("knob" + selectedID);
      knob.style.transition = "none";
      document.addEventListener("mousemove", onMouseMove);
      document.addEventListener("mouseup", onMouseUp);
      document.addEventListener("mousedown", onMouseMove);
    });
    getPositionFromValue(initVal[item.getAttribute("data")], item.getAttribute("data"));
  });
});

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

/* VCMatrix */
var matrixID = 0;
var m_isDragging = new Array();
var m_initVal = new Array();
var m_selectedID = 0;

function hexToUint(color) {
  color = color.replace('#', '');
  var intValue = parseInt(color, 16);
  return intValue;
}
function matrixSliderValueChange(id) {
  var slObj = document.getElementById("msl" + id);
  var sldMsg = id + "|MATRIX_SLIDER_CHANGE|" + slObj.value;
  websocket.send(sldMsg);
}

function setMatrixSliderValue(id, sliderValue) {
  var slObj = document.getElementById("msl" + id);
  slObj.value = sliderValue;
}

function matrixComboChanged(id) {
  var combo = document.querySelector("#mcb" + id);
  var mcbMsg = id + "|MATRIX_COMBO_CHANGE|" + combo.value;
  websocket.send(mcbMsg);
}

function setMatrixComboValue(id, comboValue) {
  var combo = document.querySelector("#mcb" + id);
  combo.value = comboValue;
}

function matrixStartColorChange(id) {
  var colorObj = document.querySelector("#msc" + id);
  var colorMsg = id + "|MATRIX_COLOR_CHANGE|START|" + hexToUint(colorObj.value);
    console.log(colorMsg);
  websocket.send(colorMsg);
}

function setMatrixStartColorValue(id, color) {
  var combo = document.querySelector("#msc" + id);
  combo.value = color;
}

function matrixEndColorChange(id) {
  var colorObj = document.querySelector("#mec" + id);
  var colorMsg = id + "|MATRIX_COLOR_CHANGE|END|" + hexToUint(colorObj.value);
  websocket.send(colorMsg);
}

function setMatrixEndColorValue(id, color) {
  var combo = document.querySelector("#mec" + id);
  combo.value = color;
}

function getPositionFromValueForMatrix(val, id) {
  var mknobRect = document.getElementById("mknob" + id).getBoundingClientRect();
  var mpie = document.getElementById("mpie" + id);
  var mspot = document.getElementById("mspot" + id);
  if (!mknobRect || !mpie || !mspot) return;
  var mknobRadius = mknobRect.width / 2;
  var angle = (340 * val / 255) % 360;
  var posX = Math.cos((angle - 260) * Math.PI / 180) * mknobRadius;
  var posY = Math.sin((angle - 260) * Math.PI / 180) * mknobRadius;
  mspot.style.transform = `translate(-50%, -50%) translate(${Math.round(posX)}px, ${Math.round(posY)}px)`;
  mpie.style.setProperty('--degValue', Math.round(angle));
}

function onMouseMoveForMatrix(e) {
  if (m_isDragging[m_selectedID]) {
    mpie = document.getElementById("mpie" + m_selectedID);
    mknob = document.getElementById("mknob" + m_selectedID);
    mspot = document.getElementById("mspot" + m_selectedID);
    var mknobRect = mknob.getBoundingClientRect();

    var mknobCenterX = mknobRect.left + mknobRect.width / 2;
    var mknobCenterY = mknobRect.top + mknobRect.height / 2;
    var mknobRadius = mknobRect.width / 2;

    var deltaX = e.clientX - mknobCenterX;
    var deltaY = e.clientY - mknobCenterY;
    var distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);

    var newPosX = deltaX * mknobRadius / distance;
    var newPosY = deltaY * mknobRadius / distance;
    var angle = Math.atan2(deltaY, deltaX);


    var angleDegrees = (angle * 180) / Math.PI;
    var normalizedAngle = (angleDegrees + 260 + 360) % 360; // Adjust for initial rotation and make sure it's positive
    var newValue = Math.round((normalizedAngle / 360) * (255 * 18 / 17)) + 0;
    if (newValue >= 0 && newValue <= 255) {
      mspot.style.transform = `translate(-50%, -50%) translate(${newPosX}px, ${newPosY}px)`;
      mpie.style.setProperty('--degValue', normalizedAngle);
      websocket.send(matrixID + "|MATRIX_KNOB|" + m_selectedID + "|" + newValue);
    }
  }
}

function onMouseUpForMatrix() {
  m_isDragging[m_selectedID] = false;
  var mknob = document.getElementById("mknob" + m_selectedID);
  mknob.style.transition = "transform 0.2s ease";
  document.removeEventListener("mousemove", onMouseMoveForMatrix);
  document.removeEventListener("mouseup", onMouseUpForMatrix);
  document.removeEventListener("mousedown", onMouseMoveForMatrix);
}

// Initial position
window.addEventListener("load", (event) => {
  var mpieWrapper = document.querySelectorAll(".mpieWrapper");
  mpieWrapper.forEach(function(item) {
    item.addEventListener("mousedown", () => {
      m_selectedID = item.getAttribute("data");
      m_isDragging[m_selectedID] = true;
      var mknob = document.getElementById("mknob" + m_selectedID);
      mknob.style.transition = "none";
      document.addEventListener("mousemove", onMouseMoveForMatrix);
      document.addEventListener("mouseup", onMouseUpForMatrix);
      document.addEventListener("mousedown", onMouseMoveForMatrix);
    });
    getPositionFromValueForMatrix(m_initVal[item.getAttribute("data")], item.getAttribute("data"));
  });
});

function setMatrixControlKnobValue(controlID, value) {
  getPositionFromValueForMatrix(parseInt(value), parseInt(controlID));
}

function wcMatrixPushButtonClicked(controlID) {
  var matMsg = matrixID + "|MATRIX_PUSHBUTTON|" + controlID;
  websocket.send(matMsg);
}
