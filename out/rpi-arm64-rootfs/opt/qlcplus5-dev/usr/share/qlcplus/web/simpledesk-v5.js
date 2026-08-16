/*
  Q Light Controller Plus
  simpledesk-v5.js

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
var slidersContainer;
var pageDisplay;
var universeSelect;
var fadersSelect;
var wsStatus;
var pollTimer = null;
var lastStartChannel = null;
var lastChannelCount = null;

var GROUP_ICONS = {
  0: "intensity",
  1: "colorwheel",
  2: "gobo",
  3: "speed",
  4: "pan",
  5: "tilt",
  6: "shutter",
  7: "prism",
  8: "beam",
  9: "star",
  10: "configure",
};

function sendMessage(msg) {
  if (!websocket || websocket.readyState !== 1) return;
  websocket.send(msg);
}

function setStatus(connected) {
  if (!wsStatus) return;
  wsStatus.textContent = "";
  wsStatus.classList.toggle("connected", connected);
  wsStatus.classList.toggle("disconnected", !connected);
  wsStatus.setAttribute("aria-label", connected ? "Connected" : "Disconnected");
  wsStatus.title = connected ? "Connected" : "Disconnected";
}

function updateWebPixelDensity() {
  var dpr = window.devicePixelRatio || 1;
  var dpi = 96 * dpr;
  var byDpi = dpi * 0.03937;
  var byHeight = (window.screen && window.screen.height ? window.screen.height : window.innerHeight || 0) / 220;
  var pd = Math.max(byDpi, byHeight || 0);
  var density = pd > 0 ? pd : 1;

  var root = document.documentElement;
  root.style.setProperty("--pd", density.toFixed(4));
  root.style.setProperty("--text-size-default", (density * 4.5).toFixed(2) + "px");
  root.style.setProperty("--icon-size-default", (density * 10).toFixed(2) + "px");
  root.style.setProperty("--icon-size-medium", (density * 8).toFixed(2) + "px");
  root.style.setProperty("--list-item-height", (density * 7).toFixed(2) + "px");
  root.style.setProperty("--medium-item-height", (density * 15).toFixed(2) + "px");
  root.style.setProperty("--big-item-height", (density * 25).toFixed(2) + "px");
}

function updateRangeFill(input) {
  var val = parseInt(input.value, 10);
  var min = parseInt(input.min, 10);
  var max = parseInt(input.max, 10);
  var fill = max > min ? ((val - min) / (max - min)) * 100 : 0;
  input.style.setProperty("--slider-fill", fill + "%");
}

function readRootCssNumber(name, fallback) {
  var value = parseFloat(getComputedStyle(document.documentElement).getPropertyValue(name));
  return isNaN(value) ? fallback : value;
}

function getResponsiveUiScale() {
  var width = window.innerWidth || document.documentElement.clientWidth || 0;
  var phoneScale = readRootCssNumber("--ui-scale-phone", 1);
  var tabletScale = readRootCssNumber("--ui-scale-tablet", 1);
  var tabletWideScale = readRootCssNumber("--ui-scale-tablet-wide", 1);
  if (width <= 700) return phoneScale;
  if (width <= 900) return tabletScale;
  if (width <= 1280) return tabletWideScale;
  return 1;
}

function getEffectiveIconSize() {
  var cssIconSize = parseFloat(getComputedStyle(document.documentElement).getPropertyValue("--icon-size-default"));
  if (!isNaN(cssIconSize)) return cssIconSize;
  var density = readRootCssNumber("--pd", 1);
  return density * 10 * getResponsiveUiScale();
}

function getUnifiedSliderThumbSize() {
  var iconSize = getEffectiveIconSize();
  var scale = readRootCssNumber("--slider-thumb-scale", 1.0);
  var aspect = readRootCssNumber("--slider-thumb-aspect", 0.75);
  var width = Math.max(6, iconSize * scale);
  var height = Math.max(6, width * aspect);
  return { width: width, height: height };
}

function isSimpleDeskSliderTouchTarget(target) {
  if (!target || typeof target.closest !== "function") return false;
  var range = target.closest("input[type='range']");
  if (!range) return false;
  return !!range.closest(".sd-slider");
}

function updatePageDisplay() {
  if (pageDisplay) pageDisplay.textContent = currentPage;
}

function normalizedUniverse() {
  var uni = parseInt(currentUniverse, 10);
  if (isNaN(uni) || uni < 1) uni = 1;
  currentUniverse = uni;
  return uni;
}

function getMaxPages() {
  var perPage = Math.max(1, parseInt(channelsPerPage, 10) || 1);
  return Math.ceil(512 / perPage);
}

function requestPage() {
  var address = ((currentPage - 1) * channelsPerPage) + 1;
  var uni = normalizedUniverse();
  sendMessage("QLC+API|getChannelsValues|" + uni + "|" + address + "|" + channelsPerPage);
}

function applyChannelValue(chNum, value, isOverriding) {
  var input = document.getElementById(String(chNum));
  if (!input) return;
  input.value = value;
  updateRangeFill(input);
  var labelObj = document.getElementById("sdslv" + chNum);
  if (labelObj) labelObj.textContent = value;
  var slider = input.closest(".sd-slider");
  if (slider) slider.classList.toggle("is-active", !!isOverriding);
}

function applyChannelType(chNum, type) {
  var input = document.getElementById(String(chNum));
  if (!input) return;
  var slider = input.closest(".sd-slider");
  if (!slider) return;
  var topNode = slider.querySelector(".sd-top, .sd-top-icon, .sd-top-color");
  if (!topNode) return;
  var prevType = topNode.getAttribute("data-type");
  if (prevType === type) return;
  topNode.outerHTML = getTopMarkup(type);
}

function connect() {
  var url = "ws://" + window.location.host + "/qlcplusWS";
  websocket = new WebSocket(url);
  setStatus(false);

  websocket.onopen = function() {
    setStatus(true);
    requestPage();
  };

  websocket.onclose = function() {
    setStatus(false);
    console.log("QLC+ connection is closed. Reconnect will be attempted in 1 second.");
    setTimeout(function () {
      connect();
    }, 1000);
  };

  websocket.onerror = function(ev) {
    console.error("QLC+ connection encountered error. Closing socket");
    console.error("Error: " + ev.data);
    setStatus(false);
    websocket.close();
  };

  websocket.onmessage = function(ev) {
    var msgParams = ev.data.split("|");
    if (msgParams[0] === "QLC+API" && msgParams[1] === "getChannelsValues") {
      drawPage(ev.data);
    }
  };
}

function drawPage(data) {
  var cVars = data.split("|");
  var html = "";
  var payloadSize = cVars.length - 2;
  var stride = payloadSize % 4 === 0 ? 4 : 3;
  var total = stride > 0 ? payloadSize / stride : 0;
  var firstChannel = total > 0 ? parseInt(cVars[2], 10) : null;
  var hasLayout =
    slidersContainer.childElementCount === total &&
    total > 0 &&
    firstChannel !== null &&
    lastStartChannel === firstChannel &&
    lastChannelCount === total;
  for (var i = 2; i < cVars.length; i += stride) {
    var chNum = parseInt(cVars[i], 10);
    var value = parseInt(cVars[i + 1], 10);
    var type = cVars[i + 2] || "";
    var isOverriding = stride === 4 ? (cVars[i + 3] === "1" || cVars[i + 3] === "true") : false;
    if (!hasLayout) {
      html += "<div class='vc-slider sd-slider" + (isOverriding ? " is-active" : "") + "' data-ch='" + chNum + "'>";
      html += getTopMarkup(type);
      html += "<div id='sdslv" + chNum + "' class='slider-value'>" + value + "</div>";
      html += "<div class='slider-track'><input type='range' class='range-vertical sd-range' id='" + chNum + "' min='0' max='255' step='1' value='" + value + "'></div>";
      html += "<div id='sdsln" + chNum + "' class='slider-caption'>" + chNum + "</div>";
      html += "<button class='slider-reset-btn sd-reset-btn' data-ch='" + chNum + "' type='button'><span class='fa-icon'>&#xf00d;</span></button>";
      html += "</div>";
    } else {
      applyChannelValue(chNum, value, isOverriding);
      applyChannelType(chNum, type);
    }
  }
  if (!hasLayout) {
    slidersContainer.innerHTML = html;
    lastStartChannel = firstChannel;
    lastChannelCount = total;
  }

  if (!hasLayout) {
    var inputs = slidersContainer.querySelectorAll(".sd-range");
    inputs.forEach(function(input) {
      input.style.setProperty("--slider-fill-color", "#38b0ff");
      input.style.setProperty("--slider-empty-color", "#555555");
      updateRangeFill(input);
    });

    var sliders = slidersContainer.querySelectorAll(".sd-slider");
    sliders.forEach(function(slider) {
      var track = slider.querySelector(".slider-track");
      var input = slider.querySelector("input[type='range']");
      if (!track || !input) return;
      var trackHeight = track.clientHeight || 120;
      input.style.setProperty("--slider-length", trackHeight + "px");
      var thumbSize = getUnifiedSliderThumbSize();
      input.style.setProperty("--slider-thumb-width", `${thumbSize.width}px`);
      input.style.setProperty("--slider-thumb-height", `${thumbSize.height}px`);
    });
  }
}

function getTopMarkup(type) {
  if (!type) {
    return "<div class='sd-top sd-top-empty' data-type=''></div>";
  }
  var aType = type.split(".");
  if (aType.length > 1) {
    var color = aType[1];
    if (color === "#000000") {
      return "<div class='sd-top sd-top-icon' data-type='" + type + "'><img src='/qrc/intensity.svg' alt=''></div>";
    }
    return "<div class='sd-top sd-top-color' data-type='" + type + "' style='background:" + color + ";'></div>";
  }
  var icon = GROUP_ICONS[parseInt(type, 10)] || "other";
  return "<div class='sd-top sd-top-icon' data-type='" + type + "'><img src='/qrc/" + icon + ".svg' alt=''></div>";
}

function nextPage() {
  currentPage += 1;
  var maxPages = getMaxPages();
  if (currentPage > maxPages) currentPage = 1;
  updatePageDisplay();
  requestPage();
}

function previousPage() {
  if (currentPage === 1) {
    currentPage = getMaxPages();
  } else {
    currentPage -= 1;
  }
  updatePageDisplay();
  requestPage();
}

function universeChanged(uniIdx) {
  currentUniverse = parseInt(uniIdx, 10) + 1;
  currentPage = 1;
  updatePageDisplay();
  requestPage();
}

function resetChannel(pageCh) {
  var uni = normalizedUniverse();
  var chNum = ((uni - 1) * 512) + parseInt(pageCh, 10);
  sendMessage("QLC+API|sdResetChannel|" + chNum);
}

function resetUniverse() {
  var uni = normalizedUniverse();
  currentPage = 1;
  updatePageDisplay();
  sendMessage("QLC+API|sdResetUniverse|" + uni);
  requestPage();
}

window.addEventListener("load", function() {
  updateWebPixelDensity();
  window.addEventListener("resize", function() {
    updateWebPixelDensity();
  });
  document.addEventListener("contextmenu", function(ev) {
    if (isSimpleDeskSliderTouchTarget(ev.target)) ev.preventDefault();
  });
  slidersContainer = document.getElementById("slidersContainer");
  pageDisplay = document.getElementById("pageDisplay");
  universeSelect = document.getElementById("universeSelect");
  fadersSelect = document.getElementById("fadersSelect");
  wsStatus = document.getElementById("wsStatus");
  setStatus(false);

  normalizedUniverse();
  updatePageDisplay();
  if (fadersSelect) fadersSelect.value = String(channelsPerPage);

  var prevBtn = document.getElementById("pagePrev");
  var nextBtn = document.getElementById("pageNext");
  var resetBtn = document.getElementById("resetUniverseBtn");

  if (prevBtn) prevBtn.addEventListener("click", previousPage);
  if (nextBtn) nextBtn.addEventListener("click", nextPage);
  if (resetBtn) resetBtn.addEventListener("click", resetUniverse);
  if (universeSelect) {
    universeSelect.addEventListener("change", function(ev) {
      universeChanged(ev.target.value);
    });
  }
  if (fadersSelect) {
    fadersSelect.addEventListener("change", function(ev) {
      channelsPerPage = parseInt(ev.target.value, 10) || channelsPerPage;
      currentPage = 1;
      updatePageDisplay();
      requestPage();
    });
  }

  slidersContainer.addEventListener("input", function(ev) {
    if (!ev.target.classList.contains("sd-range")) return;
    var id = ev.target.id;
    var labelObj = document.getElementById("sdslv" + id);
    if (labelObj) labelObj.textContent = ev.target.value;
    updateRangeFill(ev.target);
    var slider = ev.target.closest(".sd-slider");
    if (slider && parseInt(ev.target.value, 10) > 0) slider.classList.add("is-active");
    var uni = normalizedUniverse();
    var chNum = ((uni - 1) * 512) + parseInt(id, 10);
    sendMessage("CH|" + chNum + "|" + ev.target.value);
  });

  slidersContainer.addEventListener("click", function(ev) {
    var btn = ev.target.closest(".sd-reset-btn");
    if (!btn) return;
    resetChannel(btn.dataset.ch);
  });

  slidersContainer.addEventListener("pointerdown", function(ev) {
    if (ev.button !== 0) return;
    if (ev.target.closest(".sd-reset-btn") || ev.target.closest("input[type='range']")) return;
    var track = ev.target.closest(".slider-track");
    if (!track) return;
    ev.preventDefault();
    var input = track.querySelector("input[type='range']");
    if (!input) return;
    var rect = track.getBoundingClientRect();
    if (rect.height <= 0) return;
    var min = parseInt(input.min, 10);
    var max = parseInt(input.max, 10);
    var ratio = (ev.clientY - rect.top) / rect.height;
    ratio = Math.max(0, Math.min(1, ratio));
    var value = Math.round(max - ratio * (max - min));
    input.value = value;
    updateRangeFill(input);
    var labelObj = document.getElementById("sdslv" + input.id);
    if (labelObj) labelObj.textContent = value;
    var slider = input.closest(".sd-slider");
    if (slider) slider.classList.toggle("is-active", value > 0);
    var uni = normalizedUniverse();
    var chNum = ((uni - 1) * 512) + parseInt(input.id, 10);
    sendMessage("CH|" + chNum + "|" + value);
  });

  if (pollTimer) clearInterval(pollTimer);
  pollTimer = setInterval(function() {
    if (document.hidden) return;
    requestPage();
  }, 700);

  connect();
});
