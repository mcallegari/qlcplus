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

function grandMasterValueChanged(value, displayValue) {
 obj = document.getElementById("vcGMSlider");
 obj.value = value;
 var labelObj = document.getElementById("vcGMSliderLabel");
 labelObj.innerHTML = displayValue;
}

function grandMasterValueChange() {
 obj = document.getElementById("vcGMSlider");
 websocket.send("GM_VALUE|" + obj.value);
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

function setButtonDisableState(id, disable) {
  var btnObj = document.getElementById(id);
  if (disable === "1") {
    btnObj.removeAttribute("onmousedown");
    btnObj.removeAttribute("onmouseup");
    btnObj.classList.add('vcbutton-disabled');
  } else {
    btnObj.setAttribute("onmousedown", "buttonPress("+id+");");
    btnObj.setAttribute("onmouseup", "buttonRelease("+id+");");
    btnObj.classList.remove('vcbutton-disabled');
  }
}

/* VCLabel */
function setLabelDisableState(id, disable) {
  var lblObj = document.getElementById("lbl" + id);
  if (disable === "1") {
    lblObj.classList.add('vclabel-disabled');
  } else {
    lblObj.classList.remove('vclabel-disabled');
  }
}

/* VCCueList */
var cueListsIndices = new Array();
var showPanel = new Array();
var isDisableCue = new Array();

function setCueIndex(id, idx) {
 var oldIdx = cueListsIndices[id];
 if (oldIdx != undefined && oldIdx !== "-1") {
   var oldCueObj = document.getElementById(id + "_" + oldIdx);
   oldCueObj.style.backgroundColor = "";
 }
 cueListsIndices[id] = idx;
 var currCueObj = document.getElementById(id + "_" + idx);
 if (idx !== "-1") {
   currCueObj.style.setProperty("background-color", "#5E7FDF", "important");
 }
}

function sendCueCmd(id, cmd) {
 if (isDisableCue[id]) return;
 websocket.send(id + "|" + cmd);
}

function enableCue(id, idx) {
 if (isDisableCue[id]) return;
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

function changeCueNoteToEditMode(id, idx) {
 var cueNoteSpanObj = document.getElementById("cueNoteSpan" + id + "_" + idx);
 var cueNoteInputObj = document.getElementById("cueNoteInput" + id + "_" + idx);
 cueNoteSpanObj.style.display = "none";
 cueNoteInputObj.style.display = "block";
 cueNoteInputObj.focus();
}

function changeCueNoteToTextMode(id, idx) {
 var cueNoteSpanObj = document.getElementById("cueNoteSpan" + id + "_" + idx);
 var cueNoteInputObj = document.getElementById("cueNoteInput" + id + "_" + idx);
 cueNoteSpanObj.style.display = "block";
 cueNoteInputObj.style.display = "none";
 var newNote = cueNoteInputObj.value;
 cueNoteSpanObj.innerHTML = newNote;
 websocket.send(id + "|CUE_STEP_NOTE|" + idx + "|" + newNote);
}

function setCueStepNote(id, idx, note) {
 var cueNoteSpanObj = document.getElementById("cueNoteSpan" + id + "_" + idx);
 var cueNoteInputObj = document.getElementById("cueNoteInput" + id + "_" + idx);
 cueNoteSpanObj.style.display = "block";
 cueNoteInputObj.style.display = "none";
 cueNoteSpanObj.innerHTML = note;
 cueNoteInputObj.value = note;
}

function showSideFaderPanel(id, checked) {
  var progressBarObj = document.getElementById("fadePanel" + id);
  showPanel[id] = parseInt(checked, 10);
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
  if (isDisableCue[id]) return;
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
  if (isDisableCue[id]) return;
  var cueCVObj = document.getElementById("cueC" + id);
  var msg = id + "|CUE_SIDECHANGE|" + cueCVObj.value;
  websocket.send(msg);
}

function setCueDisableState(id, disable) {
  isDisableCue[id] = parseInt(disable, 10);
  var cueTable = document.getElementById("cueTable" + id);
  var fadeObj = document.getElementById("fade" + id);
  var playObj = document.getElementById("play" + id);
  var stopObj = document.getElementById("stop" + id);
  var nextObj = document.getElementById("next" + id);
  var prevObj = document.getElementById("prev" + id);
  var cueCObj = document.getElementById("cueC" + id);
  var cueCTPObj = document.getElementById("cueCTP" + id);
  var cueCBPObj = document.getElementById("cueCBP" + id);

  if (disable === "1") {
    fadeObj.classList.add('vccuelistFadeButton-disabled');
    cueTable.classList.add('cell-disabled');
    playObj.classList.add('vccuelistButton-disabled');
    stopObj.classList.add('vccuelistButton-disabled');
    nextObj.classList.add('vccuelistButton-disabled');
    prevObj.classList.add('vccuelistButton-disabled');
    cueCObj.setAttribute("disabled", "diabled");
    cueCObj.classList.add('vVertical-disabled');
    cueCTPObj.classList.add('vcslLabel-disabled');
    cueCBPObj.classList.add('vcslLabel-disabled');
  } else {
    fadeObj.classList.remove('vccuelistFadeButton-disabled');
    cueTable.classList.remove('cell-disabled');
    playObj.classList.remove('vccuelistButton-disabled');
    stopObj.classList.remove('vccuelistButton-disabled');
    nextObj.classList.remove('vccuelistButton-disabled');
    prevObj.classList.remove('vccuelistButton-disabled');
    cueCObj.removeAttribute("disabled");
    cueCObj.classList.remove('vVertical-disabled');
    cueCTPObj.classList.remove('vcslLabel-disabled');
    cueCBPObj.classList.remove('vcslLabel-disabled');
  }
}

/* VCFrame */
var framesWidth = new Array();
var framesHeight = new Array();
var framesTotalPages = new Array();
var framesCurrentPage = new Array();
var frameDisableState = new Array();
var frameCaption = new Array();
var framesPageNames = new Array();

function updateFrameLabel(id) {
  var framePageObj = document.getElementById("fr" + id + "Page");
  var newLabel = framesPageNames[id][framesCurrentPage[id]];
  framePageObj.innerHTML = newLabel;

  var frameCaptionObj = document.getElementById("fr" + id + "Caption");
  var frMpHdr = document.getElementById("frMpHdr" + id);
  var newCaption = frameCaption[id];
  if (frMpHdr) { // if multi page mode
    newCaption = frameCaption[id] ? frameCaption[id] + " - " + newLabel : newLabel;
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

function frameNextPage(id) {
 websocket.send(id + "|NEXT_PG");
}

function framePreviousPage(id) {
 websocket.send(id + "|PREV_PG");
}

function setFramePage(id, page) {
 var iPage = parseInt(page, 10);
 if (framesCurrentPage[id] === iPage || iPage >= framesTotalPages[id]) { return; }
 var framePageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
 framePageObj.style.visibility = "hidden";
 framesCurrentPage[id] = iPage;
 var frameNewPageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
 frameNewPageObj.style.visibility = "visible";
 updateFrameLabel(id);
}

function setFrameDisableState(id, disable) {
  var frameObj = document.getElementById("frEnBtn" + id);
  if (disable === "1") {
    frameDisableState[id] = 1;
    frameObj.style.background = "#E0DFDF";
  } else {
    frameDisableState[id] = 0;
    frameObj.style.background = "#D7DE75";
  }
}

function frameDisableStateChange(id) {
  websocket.send(id + "|FRAME_DISABLE|" + frameDisableState[id]);
}

/* VCSlider with Knob */
var isDragging = new Array();
var isDisableKnob = new Array();
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

function setSliderDisableState(id, disable) {
  var sliderObj = document.getElementById(id);
  var slvObj = document.getElementById("slv" + id);
  var slnObj = document.getElementById("sln" + id);
  var pie = document.getElementById("pie" + id);
  isDisableKnob[id] = parseInt(disable, 10);
  isDragging[id] = false;
  if (disable === "1") {
    if (pie) {
      if (inverted[id]) {
        pie.style.setProperty('--color1', '#555');
        pie.style.setProperty('--color2', '#c0c0c0');
      } else {
        pie.style.setProperty('--color1', '#c0c0c0');
        pie.style.setProperty('--color2', '#555');
      }
    }
    sliderObj.setAttribute("disabled", "diabled");
    sliderObj.classList.add('vVertical-disabled');
    slvObj.classList.add('vcslLabel-disabled');
    slnObj.classList.add('vcslLabel-disabled');
  } else {
    if (pie) {
      if (inverted[id]) {
        pie.style.setProperty('--color1', '#555');
        pie.style.setProperty('--color2', 'lime');
      } else {
        pie.style.setProperty('--color1', 'lime');
        pie.style.setProperty('--color2', '#555');
      }
    }
    sliderObj.removeAttribute("disabled");
    sliderObj.classList.remove('vVertical-disabled');
    slvObj.classList.remove('vcslLabel-disabled');
    slnObj.classList.remove('vcslLabel-disabled');
  }
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
    pie.style.setProperty('--color2', isDisableKnob[id] ? '#c0c0c0' : 'lime');
  } else {
    pie.style.setProperty('--color1', isDisableKnob[id] ? '#c0c0c0' : 'lime');
    pie.style.setProperty('--color2', '#555');
  }
}

function onMouseMove(e) {
  if (isDisableKnob[selectedID]) return;
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
  if (isDisableKnob[selectedID]) return;
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
 var msg = id + "|" + op;
 websocket.send(msg);
}

function wsUpdateClockTime(id, time) {
 var obj = document.getElementById(id);
 var s = time;
 var h, m;
 h = parseInt(s / 3600, 10);
 s -= (h * 3600);
 m = parseInt(s / 60, 10);
 s -= (m * 60);

 var timeString = hmsToString(h, m, s);
 obj.innerHTML = timeString;
}

function setClockDisableState(id, disable) {
  var clockObj = document.getElementById(id);

  if (disable === "1") {
    clockObj.removeAttribute("href");
    clockObj.removeAttribute("oncontextmenu");
    clockObj.classList.add('vclabel-disabled');
  } else {
    clockObj.setAttribute("href", "javascript:controlWatch("+id+", 'S');");
    clockObj.setAttribute("oncontextmenu", "javascript:controlWatch("+id+", 'R'); return false;");
    clockObj.classList.remove('vclabel-disabled');
  }
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

function matrixColor1Change(id) {
  var colorObj = document.querySelector("#mc1i" + id);
  var colorMsg = id + "|MATRIX_COLOR_CHANGE|COLOR_1|" + hexToUint(colorObj.value);
  websocket.send(colorMsg);
}

function setMatrixColor1Value(id, color) {
  var combo = document.querySelector("#mc1i" + id);
  combo.value = color;
}

function matrixColor2Change(id) {
  var colorObj = document.querySelector("#mc2i" + id);
  var colorMsg = id + "|MATRIX_COLOR_CHANGE|COLOR_2|" + hexToUint(colorObj.value);
  websocket.send(colorMsg);
}

function setMatrixColor2Value(id, color) {
  var combo = document.querySelector("#mc2i" + id);
  combo.value = color;
}

function matrixColor3Change(id) {
  var colorObj = document.querySelector("#mc3i" + id);
  var colorMsg = id + "|MATRIX_COLOR_CHANGE|COLOR_3|" + hexToUint(colorObj.value);
  websocket.send(colorMsg);
}

function setMatrixColor3Value(id, color) {
  var combo = document.querySelector("#mc3i" + id);
  combo.value = color;
}

function matrixColor4Change(id) {
  var colorObj = document.querySelector("#mc4i" + id);
  var colorMsg = id + "|MATRIX_COLOR_CHANGE|COLOR_4|" + hexToUint(colorObj.value);
  websocket.send(colorMsg);
}

function setMatrixColor4Value(id, color) {
  var combo = document.querySelector("#mc4i" + id);
  combo.value = color;
}

function matrixColor5Change(id) {
  var colorObj = document.querySelector("#mc5i" + id);
  var colorMsg = id + "|MATRIX_COLOR_CHANGE|COLOR_5|" + hexToUint(colorObj.value);
  websocket.send(colorMsg);
}

function setMatrixColor5Value(id, color) {
  var combo = document.querySelector("#mc5i" + id);
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
  getPositionFromValueForMatrix(parseInt(value, 10), parseInt(controlID, 10));
}

function wcMatrixPushButtonClicked(controlID) {
  var matMsg = matrixID + "|MATRIX_PUSHBUTTON|" + controlID;
  websocket.send(matMsg);
}
