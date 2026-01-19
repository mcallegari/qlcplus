/* QLC+ Web VC (QML) */

const VC_TYPE = {
  Unknown: 0,
  Button: 1,
  Slider: 2,
  XYPad: 3,
  Frame: 4,
  SoloFrame: 5,
  Speed: 6,
  CueList: 7,
  Label: 8,
  AudioTriggers: 9,
  Animation: 10,
  Clock: 11,
};

const MATRIX_VIS = {
  Fader: 1 << 0,
  Label: 1 << 1,
  PresetCombo: 1 << 2,
  Color1: 1 << 3,
  Color2: 1 << 4,
  Color3: 1 << 5,
  Color4: 1 << 6,
  Color5: 1 << 7,
};

const SPEED_VIS = {
  Nothing: 0,
  PlusMinus: 1 << 0,
  Dial: 1 << 1,
  Tap: 1 << 2,
  Hours: 1 << 3,
  Minutes: 1 << 4,
  Seconds: 1 << 5,
  Milliseconds: 1 << 6,
  Multipliers: 1 << 7,
  Apply: 1 << 8,
  Beats: 1 << 9,
  XPad: 1 << 10,
};

const SPEED_LABELS = ["", "", "1/16", "1/8", "1/4", "1/2", "1", "2", "4", "8", "16"];

const FA = {
  collapse: "\uf424",
  expand: "\uf422",
  check: "\uf00c",
  angleLeft: "\uf104",
  angleRight: "\uf105",
  play: "\uf04b",
  pause: "\uf04c",
  stop: "\uf04d",
  circleLeft: "\uf359",
  circleRight: "\uf35a",
  xmark: "\uf00d",
  star: "\uf005",
};

const state = {
  socket: null,
  widgets: {},
  pages: [],
  selectedPage: 0,
  projectPoll: null,
  pixelDensity: 1,
};

const vcRoot = document.getElementById("vcRoot");
const pagesBar = document.getElementById("pagesBar");
const wsStatus = document.getElementById("wsStatus");
const brandTitle = document.querySelector(".brand-title");
const appMeta = document.getElementById("appMeta");

function updateWebPixelDensity() {
  const dpr = window.devicePixelRatio || 1;
  const dpi = 96 * dpr;
  const byDpi = dpi * 0.03937;
  const byHeight = (window.screen?.height || window.innerHeight || 0) / 220;
  const pd = Math.max(byDpi, byHeight || 0);
  state.pixelDensity = pd > 0 ? pd : 1;

  const root = document.documentElement;
  root.style.setProperty("--pd", state.pixelDensity.toFixed(4));
  root.style.setProperty("--text-size-default", `${(state.pixelDensity * 4.5).toFixed(2)}px`);
  root.style.setProperty("--icon-size-default", `${(state.pixelDensity * 10).toFixed(2)}px`);
  root.style.setProperty("--icon-size-medium", `${(state.pixelDensity * 8).toFixed(2)}px`);
  root.style.setProperty("--list-item-height", `${(state.pixelDensity * 7).toFixed(2)}px`);
  root.style.setProperty("--medium-item-height", `${(state.pixelDensity * 15).toFixed(2)}px`);
  root.style.setProperty("--big-item-height", `${(state.pixelDensity * 25).toFixed(2)}px`);
}

function applyUiStyle(uiStyle) {
  if (!uiStyle?.colors) return;
  const root = document.documentElement;
  const colors = uiStyle.colors;
  const map = {
    bgStronger: "--bg-stronger",
    bgStrong: "--bg-strong",
    bgMedium: "--bg-medium",
    bgControl: "--bg-control",
    bgLight: "--bg-light",
    bgLighter: "--bg-lighter",
    fgMain: "--fg-main",
    fgMedium: "--fg-medium",
    fgLight: "--fg-light",
    sectionHeader: "--section-header",
    sectionHeaderDiv: "--section-header-div",
    highlight: "--highlight",
    highlightPressed: "--highlight-pressed",
    hover: "--hover",
    selection: "--selection",
    activeDropArea: "--active-drop",
    borderColorDark: "--border-color-dark",
    toolbarStartMain: "--toolbar-start",
    toolbarStartSub: "--toolbar-start-sub",
    toolbarEnd: "--toolbar-end",
    toolbarHoverStart: "--toolbar-hover-start",
    toolbarHoverEnd: "--toolbar-hover-end",
    toolbarSelectionMain: "--toolbar-selection-main",
    toolbarSelectionSub: "--toolbar-selection-sub",
  };

  Object.entries(map).forEach(([key, cssVar]) => {
    const value = colors[key];
    if (typeof value === "string" && value.length) {
      root.style.setProperty(cssVar, value);
    }
  });
}

function setStatus(connected) {
  wsStatus.textContent = connected ? "Connected" : "Disconnected";
  wsStatus.classList.toggle("connected", connected);
}

function sendMessage(msg) {
  if (!state.socket || state.socket.readyState !== 1) return;
  state.socket.send(msg);
}

function sendWidgetValue(id, value) {
  sendMessage(`${id}|${value}`);
}

function sendWidgetCommand(id, cmd, ...args) {
  sendMessage([id, cmd, ...args].join("|"));
}

function formatTime(seconds) {
  const s = Math.max(0, Math.floor(seconds));
  const h = String(Math.floor(s / 3600)).padStart(2, "0");
  const m = String(Math.floor((s % 3600) / 60)).padStart(2, "0");
  const sec = String(s % 60).padStart(2, "0");
  return `${h}:${m}:${sec}`;
}

function sliderDisplayValue(widget, value) {
  if (widget.valueDisplay === "Percentage") {
    const pct = Math.round((value / 255) * 100);
    return `${pct}%`;
  }
  return `${value}`;
}

function timeToQlcString(value, type = 0) {
  if (value === 0) return "0";
  if (value === -2) return "∞";

  let timeString = "";
  let remaining = value;

  if (type === 0) {
    const h = Math.floor(remaining / 3600000);
    remaining -= h * 3600000;

    const m = Math.floor(remaining / 60000);
    remaining -= m * 60000;

    const s = Math.floor(remaining / 1000);
    remaining -= s * 1000;

    if (h) timeString += `${h}h`;
    if (m) timeString += `${h ? String(m).padStart(2, "0") : m}m`;
    if (s) timeString += `${m ? String(s).padStart(2, "0") : s}s`;

    if (remaining) {
      if (remaining < 10 && timeString.length) {
        timeString += `00${remaining}ms`;
      } else if (remaining < 100 && timeString.length) {
        timeString += `0${remaining}ms`;
      } else {
        timeString += `${remaining}ms`;
      }
    }
  } else if (type === 1) {
    if (remaining < 125) return `${remaining}`;
    const beats = Math.floor(remaining / 1000);
    if (beats > 0) timeString = `${beats}`;
    remaining -= beats * 1000;

    const fracMap = {
      125: " 1/8",
      250: " 1/4",
      375: " 3/8",
      500: " 1/2",
      625: " 5/8",
      750: " 3/4",
      875: " 7/8",
    };
    if (fracMap[remaining]) timeString += fracMap[remaining];
  }

  return timeString;
}

function calculateBPMByTapIntervals(tapHistory) {
  const history = tapHistory.slice();
  while (history.length > 16) history.shift();

  const sorted = history.slice().sort((a, b) => a - b);
  const median = sorted[Math.floor(sorted.length / 2)];

  let n = 1;
  let tapx = 0;
  let tapy = 0;
  let sumX = 0;
  let sumY = 0;
  let sumXX = 0;
  let sumXY = 0;

  for (let i = 0; i < history.length; i += 1) {
    const intervalMs = history[i];
    n += 1;
    tapx += Math.floor((median / 2 + intervalMs) / median);
    tapy += intervalMs;
    sumX += tapx;
    sumY += tapy;
    sumXX += tapx * tapx;
    sumXY += tapx * tapy;
  }

  return (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
}

function splitMilliseconds(value) {
  let remaining = Math.max(0, parseInt(value, 10) || 0);
  const hours = Math.floor(remaining / 3600000);
  remaining -= hours * 3600000;
  const minutes = Math.floor(remaining / 60000);
  remaining -= minutes * 60000;
  const seconds = Math.floor(remaining / 1000);
  remaining -= seconds * 1000;
  const ms = remaining;
  return { hours, minutes, seconds, ms };
}

function applyFont(el, font) {
  if (!font) return;
  if (font.family) el.style.fontFamily = font.family;
  if (font.pixelSize && font.pixelSize > 0) {
    el.style.fontSize = `${font.pixelSize}px`;
  } else if (font.pointSize && font.pointSize > 0) {
    el.style.fontSize = `${font.pointSize}pt`;
  }
  if (font.bold) el.style.fontWeight = "700";
  if (font.italic) el.style.fontStyle = "italic";
}

function applyWidgetBase(el, widget) {
  el.classList.add("vc-widget");
  el.style.left = `${widget.geometry.x}px`;
  el.style.top = `${widget.geometry.y}px`;
  el.style.width = `${widget.geometry.w}px`;
  el.style.height = `${widget.geometry.h}px`;
  el.dataset.id = widget.id;
  el.dataset.page = widget.page;
  el.dataset.disabled = widget.disabled ? "true" : "false";

  if (widget.bgColor) el.style.backgroundColor = widget.bgColor;
  if (widget.bgImage) el.style.setProperty("--vc-bg-image", `url(${widget.bgImage})`);
  else el.style.setProperty("--vc-bg-image", "none");
  if (widget.fgColor) el.style.color = widget.fgColor;
  applyFont(el, widget.font);

  if (widget.disabled) el.classList.add("is-disabled");
  if (widget.visible === false) el.style.display = "none";
  return el;
}

function lightenColor(hex, factor) {
  if (!hex || !hex.startsWith("#") || hex.length !== 7) return hex;
  const r = Math.min(255, Math.round(parseInt(hex.slice(1, 3), 16) * factor));
  const g = Math.min(255, Math.round(parseInt(hex.slice(3, 5), 16) * factor));
  const b = Math.min(255, Math.round(parseInt(hex.slice(5, 7), 16) * factor));
  return `#${r.toString(16).padStart(2, "0")}${g.toString(16).padStart(2, "0")}${b
    .toString(16)
    .padStart(2, "0")}`;
}

function renderButton(widget) {
  const btn = applyWidgetBase(document.createElement("div"), widget);
  btn.classList.add("vc-button");
  btn.textContent = widget.caption || "Button";
  btn.dataset.action = widget.actionType ?? 0;
  const bg = widget.bgColor || "#3a3a3a";
  const bgLight = lightenColor(bg, 1.3);
  const borderWidth = widget.geometry.w > 80 ? 3 : 2;
  btn.style.setProperty("--vc-bg", bg);
  btn.style.setProperty("--vc-bg-light", bgLight || bg);
  btn.style.setProperty("--vc-inner-border-width", `${borderWidth}px`);
  if (widget.bgImage) {
    btn.style.setProperty("--vc-bg-image", `url(${widget.bgImage})`);
  } else {
    btn.style.setProperty("--vc-bg-image", "none");
  }

  const actionType = widget.actionType ?? 0;
  const isFlash = actionType === 1;
  const iconMap = {
    1: "flash",
    2: "blackout",
    3: "stopall",
  };
  if (iconMap[actionType]) {
    const icon = document.createElement("img");
    icon.className = "vc-button-icon";
    icon.src = `/qrc/${iconMap[actionType]}.svg`;
    icon.alt = iconMap[actionType];
    btn.appendChild(icon);
  }
  const onPress = (ev) => {
    if (isFlash) {
      ev.preventDefault();
      sendWidgetValue(widget.id, 255);
    }
  };
  const onRelease = (ev) => {
    if (isFlash) {
      ev.preventDefault();
      sendWidgetValue(widget.id, 0);
    }
  };

  if (window.PointerEvent) {
    btn.addEventListener("pointerdown", (ev) => {
      if (isFlash) {
        btn.setPointerCapture(ev.pointerId);
        onPress(ev);
      }
    });
    btn.addEventListener("pointerup", onRelease);
    btn.addEventListener("pointercancel", onRelease);
  } else {
    btn.addEventListener("mousedown", (ev) => {
      if (ev.button !== 0) return;
      onPress(ev);
    });
    btn.addEventListener("mouseup", onRelease);
    btn.addEventListener("touchstart", onPress);
    btn.addEventListener("touchend", onRelease);
    btn.addEventListener("touchcancel", onRelease);
  }

  btn.addEventListener("click", (ev) => {
    if (isFlash) return;
    ev.preventDefault();
    const action = widget.actionType ?? 0;
    const isActive = btn.classList.contains("state-active");
    if (action === 0 || action === 2) {
      sendWidgetValue(widget.id, isActive ? 0 : 255);
    } else {
      sendWidgetValue(widget.id, 255);
    }
  });

  state.widgets[widget.id] = { type: "button", el: btn, data: widget };
  updateButtonState(widget.id, widget.state);
  return btn;
}

function renderLabel(widget) {
  const label = applyWidgetBase(document.createElement("div"), widget);
  label.classList.add("vc-label");
  label.textContent = widget.caption || "";
  state.widgets[widget.id] = { type: "label", el: label, data: widget };
  return label;
}

function renderSlider(widget) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add("vc-slider");
  if (widget.widgetStyle === "Knob") root.classList.add("knob");

  const valueLabel = document.createElement("div");
  valueLabel.className = "slider-value";
  valueLabel.textContent = sliderDisplayValue(widget, widget.value);

  const input = document.createElement("input");
  input.type = "range";
  input.min = widget.rangeLow ?? 0;
  input.max = widget.rangeHigh ?? 255;
  input.step = 1;
  input.value = widget.value ?? 0;
  input.className = "range-vertical";
  const length = Math.max(60, widget.geometry.h - 50);
  input.style.setProperty("--slider-length", `${length}px`);
  const fillColor = widget.sliderMode === "Submaster" ? "#77dd73" : "#38b0ff";
  input.style.setProperty("--slider-fill-color", fillColor);
  input.style.setProperty("--slider-empty-color", "#888888");
  const min = parseInt(input.min, 10);
  const max = parseInt(input.max, 10);
  const val = parseInt(input.value, 10);
  const fill = max > min ? ((val - min) / (max - min)) * 100 : 0;
  input.style.setProperty("--slider-fill", `${fill}%`);
  let thumbGradient = "linear-gradient(0deg, #cccccc 0%, #555555 45%, #000000 50%, #555555 55%, #888888 100%)";
  if (widget.sliderMode === "Submaster") {
    thumbGradient = "linear-gradient(0deg, #4c4c4c 0%, #2c2c2c 45%, #000000 50%, #111111 55%, #131313 100%)";
  } else if (widget.sliderMode === "GrandMaster") {
    thumbGradient = "linear-gradient(0deg, #a81919 0%, #db2020 45%, #000000 50%, #db2020 55%, #a81919 100%)";
  }
  input.style.setProperty("--slider-thumb-gradient", thumbGradient);
  const iconSize = state.pixelDensity * 10;
  const sliderWidth = widget.geometry.w;
  const thumbWidth = Math.min(sliderWidth, iconSize * 0.75);
  const thumbHeight = Math.min(iconSize, sliderWidth);
  input.style.setProperty("--slider-thumb-width", `${thumbHeight}px`);
  input.style.setProperty("--slider-thumb-height", `${thumbWidth}px`);

  input.addEventListener("input", () => {
    const val = parseInt(input.value, 10);
    valueLabel.textContent = sliderDisplayValue(widget, val);
    const fill = max > min ? ((val - min) / (max - min)) * 100 : 0;
    input.style.setProperty("--slider-fill", `${fill}%`);
    sendWidgetValue(widget.id, val);
  });

  const caption = document.createElement("div");
  caption.className = "slider-caption";
  caption.textContent = widget.caption || "";

  let resetBtn = null;
  if (widget.monitor) {
    resetBtn = document.createElement("button");
    resetBtn.className = "slider-reset-btn";
    resetBtn.innerHTML = `<span class="fa-icon">${FA.xmark}</span>`;
    resetBtn.addEventListener("click", () => {
      sendWidgetCommand(widget.id, "SLIDER_OVERRIDE", 0);
    });
    if (widget.isOverriding) resetBtn.classList.add("is-overriding");
  }

  const trackWrap = document.createElement("div");
  trackWrap.className = "slider-track";
  trackWrap.appendChild(input);

  root.append(valueLabel, trackWrap, caption);
  if (resetBtn) root.appendChild(resetBtn);
  state.widgets[widget.id] = { type: "slider", el: root, input, valueLabel, resetBtn, data: widget };
  requestAnimationFrame(() => {
    const trackHeight = trackWrap.clientHeight || 40;
    input.style.setProperty("--slider-length", `${trackHeight}px`);
  });
  return root;
}

function renderXYPad(widget) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add("vc-xypad");

  const grid = document.createElement("div");
  grid.className = "xypad-grid";

  const makeSpacer = () => {
    const spacer = document.createElement("div");
    spacer.className = "xypad-spacer";
    return spacer;
  };

  const rangeTop = document.createElement("div");
  rangeTop.className = "xypad-range xypad-range-top";
  const rangeTopMin = document.createElement("input");
  rangeTopMin.type = "range";
  rangeTopMin.className = "range-horizontal range-min";
  rangeTopMin.min = 0;
  rangeTopMin.max = 255;
  const rangeTopMax = document.createElement("input");
  rangeTopMax.type = "range";
  rangeTopMax.className = "range-horizontal range-max";
  rangeTopMax.min = 0;
  rangeTopMax.max = 255;
  rangeTop.append(rangeTopMin, rangeTopMax);

  const rangeLeft = document.createElement("div");
  rangeLeft.className = "xypad-range xypad-range-left";
  const rangeLeftMin = document.createElement("input");
  rangeLeftMin.type = "range";
  rangeLeftMin.className = "range-vertical range-min";
  rangeLeftMin.min = 0;
  rangeLeftMin.max = 255;
  const rangeLeftMax = document.createElement("input");
  rangeLeftMax.type = "range";
  rangeLeftMax.className = "range-vertical range-max";
  rangeLeftMax.min = 0;
  rangeLeftMax.max = 255;
  rangeLeft.append(rangeLeftMin, rangeLeftMax);

  const area = document.createElement("div");
  area.className = "xypad-area";
  const rangeWindow = document.createElement("div");
  rangeWindow.className = "xypad-range-window";
  const handle = document.createElement("div");
  handle.className = "xypad-handle";
  area.append(rangeWindow, handle);

  const sliderRight = document.createElement("div");
  sliderRight.className = "xypad-slider xypad-slider-right";
  const ySlider = document.createElement("input");
  ySlider.type = "range";
  ySlider.className = "range-vertical";
  ySlider.min = 0;
  ySlider.max = 255;
  ySlider.step = 1;
  sliderRight.appendChild(ySlider);

  const sliderBottom = document.createElement("div");
  sliderBottom.className = "xypad-slider xypad-slider-bottom";
  const xSlider = document.createElement("input");
  xSlider.type = "range";
  xSlider.className = "range-horizontal";
  xSlider.min = 0;
  xSlider.max = 255;
  xSlider.step = 1;
  sliderBottom.appendChild(xSlider);

  const clamp = (val, min, max) => Math.min(Math.max(val, min), max);
  const getRanges = () => {
    const h = widget.horizontalRange || { min: 0, max: 255 };
    const v = widget.verticalRange || { min: 0, max: 255 };
    const hMin = clamp(parseFloat(h.min ?? 0), 0, 255);
    const hMax = clamp(parseFloat(h.max ?? 255), 0, 255);
    const vMin = clamp(parseFloat(v.min ?? 0), 0, 255);
    const vMax = clamp(parseFloat(v.max ?? 255), 0, 255);
    return {
      hMin: Math.min(hMin, hMax),
      hMax: Math.max(hMin, hMax),
      vMin: Math.min(vMin, vMax),
      vMax: Math.max(vMin, vMax),
    };
  };

  const updateRangeWindow = () => {
    const { hMin, hMax, vMin, vMax } = getRanges();
    const width = area.clientWidth || 1;
    const height = area.clientHeight || 1;
    const left = (hMin / 255) * width;
    const top = (vMin / 255) * height;
    const right = (hMax / 255) * width;
    const bottom = (vMax / 255) * height;
    rangeWindow.style.left = `${left}px`;
    rangeWindow.style.top = `${top}px`;
    rangeWindow.style.width = `${Math.max(0, right - left)}px`;
    rangeWindow.style.height = `${Math.max(0, bottom - top)}px`;
    const visible = hMin > 0 || hMax < 255 || vMin > 0 || vMax < 255;
    rangeWindow.style.display = visible ? "block" : "none";
    rangeTop.style.setProperty("--range-min", `${(hMin / 255) * 100}%`);
    rangeTop.style.setProperty("--range-max", `${(hMax / 255) * 100}%`);
    const vMinPct = (vMin / 255) * 100;
    const vMaxPct = (vMax / 255) * 100;
    rangeLeft.style.setProperty("--range-min", `${100 - vMaxPct}%`);
    rangeLeft.style.setProperty("--range-max", `${100 - vMinPct}%`);
  };

  let currentPos = { x: 0, y: 0 };
  function setHandle(x, y) {
    const { hMin, hMax, vMin, vMax } = getRanges();
    const clampedX = clamp(x, hMin, hMax);
    const clampedY = clamp(y, vMin, vMax);
    const width = area.clientWidth;
    const height = area.clientHeight;
    const px = (clampedX / 255) * width;
    const py = (clampedY / 255) * height;
    handle.style.left = `${px}px`;
    handle.style.top = `${py}px`;
    xSlider.value = clampedX;
    ySlider.value = 255 - clampedY;
    sliderBottom.style.setProperty("--slider-fill", `${(clampedX / 255) * 100}%`);
    sliderRight.style.setProperty("--slider-fill", `${(ySlider.value / 255) * 100}%`);
    currentPos = { x: clampedX, y: clampedY };
  }

  let dragging = false;
  const updateFromEvent = (ev) => {
    const rect = area.getBoundingClientRect();
    const relX = Math.min(Math.max(ev.clientX - rect.left, 0), rect.width);
    const relY = Math.min(Math.max(ev.clientY - rect.top, 0), rect.height);
    const { hMin, hMax, vMin, vMax } = getRanges();
    const x = clamp((relX / rect.width) * 255, hMin, hMax);
    const y = clamp((relY / rect.height) * 255, vMin, vMax);
    setHandle(x, y);
    sendWidgetCommand(widget.id, "XYPAD", x.toFixed(2), y.toFixed(2));
  };

  area.addEventListener("pointerdown", (ev) => {
    dragging = true;
    area.setPointerCapture(ev.pointerId);
    updateFromEvent(ev);
  });
  area.addEventListener("pointermove", (ev) => {
    if (!dragging) return;
    updateFromEvent(ev);
  });
  area.addEventListener("pointerup", () => {
    dragging = false;
  });
  area.addEventListener("pointercancel", () => {
    dragging = false;
  });

  xSlider.addEventListener("input", () => {
    const x = parseFloat(xSlider.value);
    const { vMin, vMax } = getRanges();
    const y = clamp(255 - parseFloat(ySlider.value), vMin, vMax);
    setHandle(x, y);
    sendWidgetCommand(widget.id, "XYPAD", x.toFixed(2), y.toFixed(2));
  });

  ySlider.addEventListener("input", () => {
    const { hMin, hMax } = getRanges();
    const x = clamp(parseFloat(xSlider.value), hMin, hMax);
    const y = clamp(255 - parseFloat(ySlider.value), 0, 255);
    setHandle(x, y);
    sendWidgetCommand(widget.id, "XYPAD", x.toFixed(2), y.toFixed(2));
  });

  const syncRangeInputs = () => {
    const { hMin, hMax, vMin, vMax } = getRanges();
    rangeTopMin.value = hMin;
    rangeTopMax.value = hMax;
    rangeLeftMin.value = vMin;
    rangeLeftMax.value = vMax;
  };

  const sendRangeUpdate = (axis) => {
    const { hMin, hMax, vMin, vMax } = getRanges();
    if (axis === "h") {
      widget.horizontalRange = { min: hMin, max: hMax };
      sendWidgetCommand(widget.id, "XYPAD_RANGE_H", hMin.toFixed(2), hMax.toFixed(2));
    } else {
      widget.verticalRange = { min: vMin, max: vMax };
      sendWidgetCommand(widget.id, "XYPAD_RANGE_V", vMin.toFixed(2), vMax.toFixed(2));
    }
    updateRangeWindow();
    setHandle(parseFloat(xSlider.value), 255 - parseFloat(ySlider.value));
  };

  const updateRangeValue = (axis, which, value) => {
    const ranges = getRanges();
    if (axis === "h") {
      if (which === "min") ranges.hMin = clamp(value, 0, ranges.hMax);
      else ranges.hMax = clamp(value, ranges.hMin, 255);
      widget.horizontalRange = { min: ranges.hMin, max: ranges.hMax };
    } else {
      if (which === "min") ranges.vMin = clamp(value, 0, ranges.vMax);
      else ranges.vMax = clamp(value, ranges.vMin, 255);
      widget.verticalRange = { min: ranges.vMin, max: ranges.vMax };
    }
  };

  const pickRangeTarget = (axis, value) => {
    const { hMin, hMax, vMin, vMax } = getRanges();
    if (axis === "h") {
      return Math.abs(value - hMin) <= Math.abs(value - hMax) ? "min" : "max";
    }
    return Math.abs(value - vMin) <= Math.abs(value - vMax) ? "min" : "max";
  };

  const setActiveRangeHandle = (axis, which) => {
    if (axis === "h") {
      rangeTopMin.classList.toggle("is-active", which === "min");
      rangeTopMax.classList.toggle("is-active", which === "max");
    } else {
      rangeLeftMin.classList.toggle("is-active", which === "min");
      rangeLeftMax.classList.toggle("is-active", which === "max");
    }
  };

  const rangeValueFromPointer = (axis, ev, container) => {
    const rect = container.getBoundingClientRect();
    if (axis === "h") {
      const rel = clamp(ev.clientX - rect.left, 0, rect.width);
      return (rel / rect.width) * 255;
    }
    const rel = clamp(ev.clientY - rect.top, 0, rect.height);
    return (rel / rect.height) * 255;
  };

  const handleRangePointer = (axis, ev, container) => {
    const value = rangeValueFromPointer(axis, ev, container);
    const which = pickRangeTarget(axis, value);
    setActiveRangeHandle(axis, which);
    updateRangeValue(axis, which, value);
    syncRangeInputs();
    sendRangeUpdate(axis);
  };

  const rangeDrag = { active: false, axis: null, which: null, container: null };
  const startRangeDrag = (axis, ev, container) => {
    rangeDrag.active = true;
    rangeDrag.axis = axis;
    rangeDrag.container = container;
    const value = rangeValueFromPointer(axis, ev, container);
    rangeDrag.which = pickRangeTarget(axis, value);
    setActiveRangeHandle(axis, rangeDrag.which);
    updateRangeValue(axis, rangeDrag.which, value);
    syncRangeInputs();
    sendRangeUpdate(axis);
    container.setPointerCapture(ev.pointerId);
  };
  const moveRangeDrag = (ev) => {
    if (!rangeDrag.active || !rangeDrag.container) return;
    const value = rangeValueFromPointer(rangeDrag.axis, ev, rangeDrag.container);
    updateRangeValue(rangeDrag.axis, rangeDrag.which, value);
    syncRangeInputs();
    sendRangeUpdate(rangeDrag.axis);
  };
  const endRangeDrag = (ev) => {
    if (!rangeDrag.active || !rangeDrag.container) return;
    rangeDrag.container.releasePointerCapture(ev.pointerId);
    rangeDrag.active = false;
  };

  rangeTop.addEventListener("pointerdown", (ev) => {
    startRangeDrag("h", ev, rangeTop);
  });
  rangeTop.addEventListener("pointermove", moveRangeDrag);
  rangeTop.addEventListener("pointerup", endRangeDrag);
  rangeTop.addEventListener("pointercancel", endRangeDrag);

  rangeLeft.addEventListener("pointerdown", (ev) => {
    startRangeDrag("v", ev, rangeLeft);
  });
  rangeLeft.addEventListener("pointermove", moveRangeDrag);
  rangeLeft.addEventListener("pointerup", endRangeDrag);
  rangeLeft.addEventListener("pointercancel", endRangeDrag);

  rangeTopMin.addEventListener("input", () => {
    setActiveRangeHandle("h", "min");
    updateRangeValue("h", "min", parseFloat(rangeTopMin.value));
    syncRangeInputs();
    sendRangeUpdate("h");
  });
  rangeTopMax.addEventListener("input", () => {
    setActiveRangeHandle("h", "max");
    updateRangeValue("h", "max", parseFloat(rangeTopMax.value));
    syncRangeInputs();
    sendRangeUpdate("h");
  });
  rangeLeftMin.addEventListener("input", () => {
    setActiveRangeHandle("v", "min");
    updateRangeValue("v", "min", parseFloat(rangeLeftMin.value));
    syncRangeInputs();
    sendRangeUpdate("v");
  });
  rangeLeftMax.addEventListener("input", () => {
    setActiveRangeHandle("v", "max");
    updateRangeValue("v", "max", parseFloat(rangeLeftMax.value));
    syncRangeInputs();
    sendRangeUpdate("v");
  });

  const pos = widget.position || { x: 0, y: 0 };
  requestAnimationFrame(() => {
    syncRangeInputs();
    setActiveRangeHandle("h", "min");
    setActiveRangeHandle("v", "min");
    updateRangeWindow();
    setHandle(pos.x, pos.y);
  });

  const resizeObserver = new ResizeObserver(() => {
    updateRangeWindow();
    setHandle(currentPos.x, currentPos.y);
  });
  resizeObserver.observe(area);

  grid.append(
    makeSpacer(),
    rangeTop,
    makeSpacer(),
    rangeLeft,
    area,
    sliderRight,
    makeSpacer(),
    sliderBottom,
    makeSpacer()
  );

  root.appendChild(grid);
  state.widgets[widget.id] = {
    type: "xypad",
    el: root,
    handle,
    data: widget,
    setHandle,
    updateRangeWindow,
    rangeInputs: { rangeTopMin, rangeTopMax, rangeLeftMin, rangeLeftMax },
  };
  return root;
}

function renderAudioTriggers(widget) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add("vc-audio");

  const audioState = {
    enabled: !!widget.enabled,
    volume: widget.volume ?? 100,
  };

  const header = document.createElement("div");
  header.className = "audio-header";

  const title = document.createElement("div");
  title.className = "audio-title";
  title.textContent = widget.caption || "Audio";
  applyFont(title, widget.font);

  const button = document.createElement("button");
  button.className = "frame-btn frame-enable audio-enable";
  button.innerHTML = `<span class="fa-icon">${FA.check}</span>`;
  const updateEnableUI = (enabled) => {
    button.title = enabled ? "Disable" : "Enable";
    button.classList.toggle("is-disabled", !enabled);
  };
  updateEnableUI(audioState.enabled);
  button.addEventListener("click", () => {
    const nextEnabled = !audioState.enabled;
    audioState.enabled = nextEnabled;
    updateEnableUI(nextEnabled);
    const next = nextEnabled ? 255 : 0;
    sendWidgetValue(widget.id, next);
  });

  header.append(title, button);

  const body = document.createElement("div");
  body.className = "audio-body";

  const bars = document.createElement("div");
  bars.className = "audio-bars";
  const count = Math.max(1, widget.bars || 8);
  for (let i = 0; i < count; i++) {
    const bar = document.createElement("div");
    bar.className = "audio-bar";
    bar.style.height = `${30 + (i % 3) * 20}%`;
    bars.appendChild(bar);
  }

  const faderWrap = document.createElement("div");
  faderWrap.className = "audio-fader";
  const fader = document.createElement("input");
  fader.type = "range";
  fader.className = "range-vertical";
  fader.min = 0;
  fader.max = 100;
  fader.step = 1;
  fader.value = audioState.volume;
  fader.style.setProperty("--slider-fill-color", "#38b0ff");
  fader.style.setProperty("--slider-empty-color", "#888888");
  fader.style.setProperty("--slider-fill", `${(fader.value / 100) * 100}%`);
  fader.style.setProperty(
    "--slider-thumb-gradient",
    "linear-gradient(0deg, #cccccc 0%, #555555 45%, #000000 50%, #555555 55%, #888888 100%)"
  );
  const iconSize = state.pixelDensity * 10;
  const thumbWidth = iconSize * 0.75;
  fader.style.setProperty("--slider-thumb-width", `${iconSize}px`);
  fader.style.setProperty("--slider-thumb-height", `${thumbWidth}px`);
  fader.addEventListener("input", () => {
    const value = parseInt(fader.value, 10);
    fader.style.setProperty("--slider-fill", `${(value / 100) * 100}%`);
    audioState.volume = value;
    sendWidgetCommand(widget.id, "AUDIO_VOLUME", value);
  });
  faderWrap.appendChild(fader);

  body.append(bars, faderWrap);

  root.append(header, body);
  state.widgets[widget.id] = {
    type: "audio",
    el: root,
    button,
    fader,
    data: widget,
    state: audioState,
    updateEnableUI,
  };
  return root;
}

function renderMatrix(widget) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add("vc-matrix");

  const grid = document.createElement("div");
  grid.className = "animation-grid";
  const matrixState = { fader: null, combo: null, colors: {} };

  if (widget.visibilityMask & MATRIX_VIS.Fader) {
    const faderWrap = document.createElement("div");
    faderWrap.className = "animation-fader";
    const fader = document.createElement("input");
    fader.type = "range";
    fader.className = "range-vertical";
    fader.min = 0;
    fader.max = 255;
    fader.step = 1;
    fader.value = widget.faderLevel ?? 0;
    fader.style.setProperty("--slider-fill-color", "#38b0ff");
    fader.style.setProperty("--slider-empty-color", "#888888");
    fader.style.setProperty("--slider-fill", `${(fader.value / 255) * 100}%`);
    fader.style.setProperty(
      "--slider-thumb-gradient",
      "linear-gradient(0deg, #cccccc 0%, #555555 45%, #000000 50%, #555555 55%, #888888 100%)"
    );
    const iconSize = state.pixelDensity * 10;
    const width = widget.geometry?.w ?? iconSize;
    const thumbWidth = Math.min(width, iconSize * 0.75);
    const thumbHeight = Math.min(iconSize, width);
    fader.style.setProperty("--slider-thumb-width", `${thumbHeight}px`);
    fader.style.setProperty("--slider-thumb-height", `${thumbWidth}px`);
    fader.addEventListener("input", () => {
      const value = parseInt(fader.value, 10);
      const fill = (value / 255) * 100;
      fader.style.setProperty("--slider-fill", `${fill}%`);
      sendWidgetCommand(widget.id, "MATRIX_SLIDER", fader.value);
    });
    matrixState.fader = fader;
    faderWrap.appendChild(fader);
    grid.appendChild(faderWrap);
  } else {
    grid.classList.add("no-fader");
  }

  if (widget.visibilityMask & MATRIX_VIS.Label) {
    const caption = document.createElement("div");
    caption.className = "animation-label";
    caption.textContent = widget.caption || "Animation";
    applyFont(caption, widget.font);
    if (widget.fgColor) caption.style.color = widget.fgColor;
    grid.appendChild(caption);
  }

  const colorsRow = document.createElement("div");
  colorsRow.className = "animation-colors";
  const colors = [
    { key: "color1", bit: MATRIX_VIS.Color1, cmd: "MATRIX_COLOR_1" },
    { key: "color2", bit: MATRIX_VIS.Color2, cmd: "MATRIX_COLOR_2" },
    { key: "color3", bit: MATRIX_VIS.Color3, cmd: "MATRIX_COLOR_3" },
    { key: "color4", bit: MATRIX_VIS.Color4, cmd: "MATRIX_COLOR_4" },
    { key: "color5", bit: MATRIX_VIS.Color5, cmd: "MATRIX_COLOR_5" },
  ];

  colors.forEach((c) => {
    if (widget.visibilityMask & c.bit) {
      const input = document.createElement("input");
      input.type = "color";
      input.className = "animation-color";
      input.value = widget[c.key] || "#ffffff";
      input.addEventListener("input", () => {
        sendWidgetCommand(widget.id, c.cmd, input.value);
      });
      matrixState.colors[c.key] = input;
      colorsRow.appendChild(input);
    }
  });
  grid.appendChild(colorsRow);

  if (widget.visibilityMask & MATRIX_VIS.PresetCombo) {
    const combo = document.createElement("select");
    combo.className = "animation-combo";
    (widget.algorithms || []).forEach((name, idx) => {
      const opt = document.createElement("option");
      opt.value = idx;
      opt.textContent = name;
      if (idx === widget.algorithmIndex) opt.selected = true;
      combo.appendChild(opt);
    });
    combo.addEventListener("change", () => {
      sendWidgetCommand(widget.id, "MATRIX_COMBO", combo.value);
    });
    matrixState.combo = combo;
    grid.appendChild(combo);
  }

  root.append(grid);
  state.widgets[widget.id] = {
    type: "matrix",
    el: root,
    data: widget,
    grid,
    matrixState,
  };
  requestAnimationFrame(() => {
    if (matrixState.fader) {
      const wrap = matrixState.fader.parentElement;
      const height = wrap?.clientHeight || 40;
      matrixState.fader.style.setProperty("--slider-length", `${height}px`);
    }
  });
  return root;
}

function renderSpeed(widget) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add("vc-speed");

  const mask = widget.visibilityMask ?? 0;
  const hasCaption = !!(widget.caption && widget.caption.length);
  const showDial = !!(mask & SPEED_VIS.Dial);
  const showTap = !!(mask & SPEED_VIS.Tap);
  const showBeats = !!(mask & SPEED_VIS.Beats);
  const showTime =
    !!(mask & SPEED_VIS.Hours) ||
    !!(mask & SPEED_VIS.Minutes) ||
    !!(mask & SPEED_VIS.Seconds) ||
    !!(mask & SPEED_VIS.Milliseconds);
  const showMultipliers = !!(mask & SPEED_VIS.Multipliers);
  const showApply = !!(mask & SPEED_VIS.Apply);
  const presets = Array.isArray(widget.presetsList) ? widget.presetsList : [];
  const columns = showTap ? 6 : 4;

  const grid = document.createElement("div");
  grid.className = "speed-grid";
  grid.style.gridTemplateColumns = `repeat(${columns}, minmax(0, 1fr))`;

  const rows = [];
  let rowIndex = 1;
  let dialRowStart = rowIndex;
  let timeRowIndex = null;
  let multiplierRowIndex = null;
  let applyRowIndex = null;
  let presetRowIndex = null;

  if (hasCaption) {
    rows.push("var(--list-item-height)");
    const caption = document.createElement("div");
    caption.className = "speed-caption";
    caption.textContent = widget.caption;
    caption.style.gridColumn = `1 / span ${columns}`;
    caption.style.gridRow = `${rowIndex}`;
    grid.appendChild(caption);
    rowIndex += 1;
  }

  dialRowStart = rowIndex;
  rows.push("1fr", "1fr");
  rowIndex += 2;

  if (showTime) {
    rows.push("var(--list-item-height)");
    timeRowIndex = rowIndex;
    rowIndex += 1;
  }

  if (showMultipliers) {
    rows.push("var(--list-item-height)");
    multiplierRowIndex = rowIndex;
    rowIndex += 1;
  }

  if (showApply) {
    rows.push("var(--list-item-height)");
    applyRowIndex = rowIndex;
    rowIndex += 1;
  }

  if (presets.length > 0) {
    rows.push("auto");
    presetRowIndex = rowIndex;
    rowIndex += 1;
  }

  grid.style.gridTemplateRows = rows.join(" ");

  const speedState = {
    type: "speed",
    el: root,
    data: widget,
    grid,
    dial: null,
    beatButtons: {},
    timeInputs: {},
    multiplierLabel: null,
    presets: [],
    tap: null,
  };

  if (showDial) {
    const dialWrap = document.createElement("div");
    dialWrap.className = "speed-dial";
    dialWrap.style.gridColumn = `1 / span ${showTap ? 4 : columns}`;
    dialWrap.style.gridRow = `${dialRowStart} / span 2`;

    const dialKnob = document.createElement("div");
    dialKnob.className = "speed-dial-knob";
    const dialIndicator = document.createElement("div");
    dialIndicator.className = "speed-dial-indicator";
    dialKnob.appendChild(dialIndicator);
    dialWrap.appendChild(dialKnob);

    const dialState = {
      el: dialKnob,
      indicator: dialIndicator,
      value: 0,
      lastValue: 0,
      dragging: false,
      step: 1,
    };

    const updateDialAngle = (value) => {
      const angle = (value / 1000) * 360;
      dialIndicator.style.setProperty("--dial-angle", `${angle}deg`);
    };

    const clampDialValue = (value) => {
      let v = value;
      if (v > 1000) v = 0;
      if (v < 0) v = 1000;
      return v;
    };

    const dialValueFromPointer = (ev) => {
      const rect = dialKnob.getBoundingClientRect();
      const cx = rect.left + rect.width / 2;
      const cy = rect.top + rect.height / 2;
      const dx = ev.clientX - cx;
      const dy = ev.clientY - cy;
      let angle = (Math.atan2(dy, dx) * 180) / Math.PI;
      angle += 90;
      if (angle < 0) angle += 360;
      return Math.round((angle / 360) * 1000);
    };

    const applyDialChange = (newValue) => {
      const value = clampDialValue(newValue);
      let diff = value - dialState.lastValue;
      const threshold = 50;
      if (diff > threshold) diff = -dialState.step;
      else if (diff < -threshold) diff = dialState.step;
      dialState.lastValue = value;
      dialState.value = value;
      updateDialAngle(value);

      const min = widget.timeMin ?? 0;
      const max = widget.timeMax ?? 10000;
      const current = widget.currentTime ?? 0;
      const next = Math.min(max, Math.max(min, current + diff));
      if (next !== current) {
        widget.currentTime = next;
        updateSpeedState(widget.id, next, widget.currentFactor ?? 0);
        sendWidgetCommand(widget.id, "SPEED_TIME", next);
      }
    };

    dialKnob.addEventListener("pointerdown", (ev) => {
      if (ev.button !== 0) return;
      dialState.dragging = true;
      dialKnob.setPointerCapture(ev.pointerId);
      applyDialChange(dialValueFromPointer(ev));
    });
    dialKnob.addEventListener("pointermove", (ev) => {
      if (!dialState.dragging) return;
      applyDialChange(dialValueFromPointer(ev));
    });
    dialKnob.addEventListener("pointerup", (ev) => {
      dialState.dragging = false;
      dialKnob.releasePointerCapture(ev.pointerId);
    });
    dialKnob.addEventListener("pointercancel", () => {
      dialState.dragging = false;
    });
    dialKnob.addEventListener("wheel", (ev) => {
      ev.preventDefault();
      const dir = ev.deltaY < 0 ? 1 : -1;
      applyDialChange(dialState.value + dir * dialState.step);
    });

    updateDialAngle(dialState.value);
    speedState.dial = dialState;
    grid.appendChild(dialWrap);
  }

  const makeSpeedButton = (label) => {
    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = "speed-btn";
    btn.textContent = label;
    return btn;
  };

  if (showBeats) {
    const beatDefs = [
      { label: "1/16", value: 2, row: dialRowStart, col: 1 },
      { label: "1/8", value: 3, row: dialRowStart, col: 2 },
      { label: "1/4", value: 4, row: dialRowStart, col: 3 },
      { label: "1/2", value: 5, row: dialRowStart, col: 4 },
      { label: "2", value: 7, row: dialRowStart + 1, col: 1 },
      { label: "4", value: 8, row: dialRowStart + 1, col: 2 },
      { label: "8", value: 9, row: dialRowStart + 1, col: 3 },
      { label: "16", value: 10, row: dialRowStart + 1, col: 4 },
    ];

    beatDefs.forEach((beat) => {
      const btn = makeSpeedButton(beat.label);
      btn.classList.add("speed-beat");
      btn.dataset.factor = String(beat.value);
      btn.style.gridColumn = `${beat.col}`;
      btn.style.gridRow = `${beat.row}`;
      btn.addEventListener("click", () => {
        sendWidgetCommand(widget.id, "SPEED_FACTOR", beat.value);
      });
      speedState.beatButtons[beat.value] = btn;
      grid.appendChild(btn);
    });
  }

  if (showTap) {
    const tapBtn = makeSpeedButton("TAP");
    tapBtn.classList.add("speed-tap");
    tapBtn.style.gridColumn = `${columns - 1} / span 2`;
    tapBtn.style.gridRow = `${dialRowStart} / span 2`;
    grid.appendChild(tapBtn);

    const tapState = {
      btn: tapBtn,
      lastTap: 0,
      history: [],
      timerId: null,
      blinkOn: false,
    };

    const setTapBlink = (on) => {
      tapState.blinkOn = on;
      tapBtn.classList.toggle("is-blink", on);
    };

    const stopTap = () => {
      if (tapState.timerId) {
        clearInterval(tapState.timerId);
        tapState.timerId = null;
      }
      setTapBlink(false);
      tapState.lastTap = 0;
      tapState.history = [];
    };

    const runTap = () => {
      const now = Date.now();
      if (tapState.lastTap !== 0 && now - tapState.lastTap < 1500) {
        const newTime = now - tapState.lastTap;
        tapState.history.push(newTime);
        const tapValue = calculateBPMByTapIntervals(tapState.history);
        if (!Number.isNaN(tapValue)) {
          widget.currentTime = Math.round(tapValue);
          updateSpeedState(widget.id, widget.currentTime, widget.currentFactor ?? 0);
          sendWidgetCommand(widget.id, "SPEED_TIME", widget.currentTime);
          if (tapState.timerId) clearInterval(tapState.timerId);
          tapState.timerId = setInterval(() => {
            setTapBlink(!tapState.blinkOn);
          }, widget.currentTime || 500);
        }
      } else {
        stopTap();
      }
      tapState.lastTap = now;
    };

    tapBtn.addEventListener("pointerdown", (ev) => {
      if (ev.button !== 0) return;
      runTap();
    });
    tapBtn.addEventListener("contextmenu", (ev) => {
      ev.preventDefault();
      stopTap();
    });
    speedState.tap = tapState;
  }

  if (showTime && timeRowIndex !== null) {
    const timeRow = document.createElement("div");
    timeRow.className = "speed-time-row";
    timeRow.style.gridColumn = `1 / span ${columns}`;
    timeRow.style.gridRow = `${timeRowIndex}`;

    const makeSpin = (key, suffix, max) => {
      const wrap = document.createElement("div");
      wrap.className = "speed-spin";
      const input = document.createElement("input");
      input.type = "number";
      input.min = "0";
      input.max = String(max);
      input.step = "1";
      const label = document.createElement("span");
      label.textContent = suffix;
      wrap.append(input, label);
      speedState.timeInputs[key] = input;
      return wrap;
    };

    if (mask & SPEED_VIS.Hours) timeRow.appendChild(makeSpin("hours", "h", 999));
    if (mask & SPEED_VIS.Minutes) timeRow.appendChild(makeSpin("minutes", "m", 59));
    if (mask & SPEED_VIS.Seconds) timeRow.appendChild(makeSpin("seconds", "s", 59));
    if (mask & SPEED_VIS.Milliseconds) timeRow.appendChild(makeSpin("ms", "ms", 999));

    const commitTime = () => {
      const h = parseInt(speedState.timeInputs.hours?.value || "0", 10);
      const m = parseInt(speedState.timeInputs.minutes?.value || "0", 10);
      const s = parseInt(speedState.timeInputs.seconds?.value || "0", 10);
      const ms = parseInt(speedState.timeInputs.ms?.value || "0", 10);
      const total = Math.max(0, h * 3600000 + m * 60000 + s * 1000 + ms);
      widget.currentTime = total;
      updateSpeedState(widget.id, total, widget.currentFactor ?? 0);
      sendWidgetCommand(widget.id, "SPEED_TIME", total);
    };

    Object.values(speedState.timeInputs).forEach((input) => {
      input.addEventListener("change", commitTime);
      input.addEventListener("input", commitTime);
    });

    grid.appendChild(timeRow);
  }

  if (showMultipliers && multiplierRowIndex !== null) {
    const row = document.createElement("div");
    row.className = "speed-multi-row";
    row.style.gridColumn = `1 / span ${columns}`;
    row.style.gridRow = `${multiplierRowIndex}`;

    const minusBtn = makeSpeedButton("-");
    minusBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "SPEED_DOWN"));
    const plusBtn = makeSpeedButton("+");
    plusBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "SPEED_UP"));

    const label = document.createElement("div");
    label.className = "speed-multi-label";
    const labelTop = document.createElement("div");
    const labelBottom = document.createElement("div");
    label.append(labelTop, labelBottom);
    speedState.multiplierLabel = { top: labelTop, bottom: labelBottom };

    const resetBtn = makeSpeedButton("");
    resetBtn.classList.add("speed-reset");
    resetBtn.innerHTML = `<span class="fa-icon">${FA.xmark}</span>`;
    resetBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "SPEED_FACTOR", 6));

    row.append(minusBtn, label, plusBtn, resetBtn);
    grid.appendChild(row);
  }

  if (showApply && applyRowIndex !== null) {
    const applyBtn = makeSpeedButton("Apply");
    applyBtn.classList.add("speed-apply");
    applyBtn.style.gridColumn = `1 / span ${columns}`;
    applyBtn.style.gridRow = `${applyRowIndex}`;
    applyBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "SPEED_APPLY"));
    grid.appendChild(applyBtn);
  }

  if (presetRowIndex !== null) {
    const presetsRow = document.createElement("div");
    presetsRow.className = "speed-presets";
    presetsRow.style.gridColumn = `1 / span ${columns}`;
    presetsRow.style.gridRow = `${presetRowIndex}`;

    presets.forEach((preset) => {
      const btn = makeSpeedButton(preset.name || "");
      btn.classList.add("speed-preset");
      btn.addEventListener("click", () => {
        const value = parseInt(preset.value, 10) || 0;
        widget.currentTime = value;
        updateSpeedState(widget.id, value, widget.currentFactor ?? 0);
        sendWidgetCommand(widget.id, "SPEED_TIME", value);
      });
      speedState.presets.push({ value: preset.value, el: btn });
      presetsRow.appendChild(btn);
    });
    grid.appendChild(presetsRow);
  }

  root.appendChild(grid);
  state.widgets[widget.id] = speedState;
  updateSpeedState(widget.id, widget.currentTime ?? 0, widget.currentFactor ?? 0);
  return root;
}

function renderClock(widget) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add("vc-clock");

  const display = document.createElement("div");
  display.textContent = formatTime(widget.currentTime || 0);
  root.appendChild(display);

  let timer = null;
  let running = false;
  let counter = widget.clockType === 2 ? widget.targetTime : 0;

  if (widget.clockType !== 0) {
    root.style.cursor = "pointer";
    root.addEventListener("click", () => {
      running = !running;
      if (running && !timer) {
        timer = setInterval(() => {
          if (widget.clockType === 1) counter += 1;
          else counter = Math.max(0, counter - 1);
          display.textContent = formatTime(counter);
        }, 1000);
      } else if (!running && timer) {
        clearInterval(timer);
        timer = null;
      }
    });
    root.addEventListener("contextmenu", (ev) => {
      ev.preventDefault();
      counter = widget.clockType === 2 ? widget.targetTime : 0;
      display.textContent = formatTime(counter);
    });
  }

  state.widgets[widget.id] = { type: "clock", el: root, display, data: widget };
  return root;
}

function renderCueList(widget) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add("vc-cuelist");

  const content = document.createElement("div");
  content.className = "cue-content";

  const stepsEl = document.createElement("div");
  stepsEl.className = "cue-steps";
  const table = document.createElement("table");
  table.className = "cue-table";
  const colgroup = document.createElement("colgroup");
  const colWidths = [
    "var(--icon-size-medium)",
    "calc(var(--big-item-height) * 1.5)",
    "calc(var(--big-item-height) * 0.5)",
    "calc(var(--big-item-height) * 0.5)",
    "calc(var(--big-item-height) * 0.5)",
    "calc(var(--big-item-height) * 0.5)",
  ];
  colWidths.forEach((width) => {
    const col = document.createElement("col");
    col.style.width = width;
    colgroup.appendChild(col);
  });
  const col = document.createElement("col");
  colgroup.appendChild(col);
  table.appendChild(colgroup);

  const thead = document.createElement("thead");
  const headerRow = document.createElement("tr");
  const headerLabels = ["#", "Function", "Fade In", "Hold", "Fade Out", "Duration", "Note"];
  headerLabels.forEach((label) => {
    const th = document.createElement("th");
    th.textContent = label;
    headerRow.appendChild(th);
  });
  thead.appendChild(headerRow);
  table.appendChild(thead);

  const tbody = document.createElement("tbody");
  const steps = widget.steps || [];
  steps.forEach((step, index) => {
    const row = document.createElement("tr");
    row.className = "cue-step";
    row.dataset.index = index;

    const idx = document.createElement("td");
    idx.className = "cue-index";
    idx.textContent = index + 1;
    const name = document.createElement("td");
    name.className = "cue-name";
    name.textContent = step.funcName || `Function ${step.funcID || ""}`;
    const fadeIn = document.createElement("td");
    fadeIn.className = "cue-time";
    fadeIn.textContent = timeToQlcString(step.fadeIn ?? 0);
    const hold = document.createElement("td");
    hold.className = "cue-time";
    hold.textContent = timeToQlcString(step.hold ?? 0);
    const fadeOut = document.createElement("td");
    fadeOut.className = "cue-time";
    fadeOut.textContent = timeToQlcString(step.fadeOut ?? 0);
    const duration = document.createElement("td");
    duration.className = "cue-time";
    duration.textContent = timeToQlcString(step.duration ?? 0);

    const noteCell = document.createElement("td");
    noteCell.className = "cue-note-cell";
    const note = document.createElement("input");
    note.className = "cue-note";
    note.value = step.note || "";
    note.addEventListener("change", () => {
      sendWidgetCommand(widget.id, "CUE_STEP_NOTE", index, note.value);
    });
    note.addEventListener("click", (ev) => ev.stopPropagation());

    row.addEventListener("click", () => {
      sendWidgetCommand(widget.id, "STEP", index);
    });

    noteCell.appendChild(note);
    row.append(idx, name, fadeIn, hold, fadeOut, duration, noteCell);
    tbody.appendChild(row);
  });
  table.appendChild(tbody);
  stepsEl.appendChild(table);

  const controlsRow = document.createElement("div");
  controlsRow.className = "cue-controls-row";
  const playBtn = document.createElement("button");
  playBtn.className = "cue-control-btn";
  playBtn.innerHTML = `<span class="fa-icon">${FA.play}</span>`;
  playBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "PLAY"));
  const stopBtn = document.createElement("button");
  stopBtn.className = "cue-control-btn";
  stopBtn.innerHTML = `<span class="fa-icon">${FA.stop}</span>`;
  stopBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "STOP"));
  const prevBtn = document.createElement("button");
  prevBtn.className = "cue-control-btn";
  prevBtn.innerHTML = `<span class="fa-icon">${FA.circleLeft}</span>`;
  prevBtn.style.color = "lightcyan";
  prevBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "PREV"));
  const nextBtn = document.createElement("button");
  nextBtn.className = "cue-control-btn";
  nextBtn.innerHTML = `<span class="fa-icon">${FA.circleRight}</span>`;
  nextBtn.style.color = "lightcyan";
  nextBtn.addEventListener("click", () => sendWidgetCommand(widget.id, "NEXT"));
  controlsRow.append(playBtn, stopBtn, prevBtn, nextBtn);

  content.append(stepsEl, controlsRow);
  root.appendChild(content);

  let sideFader = null;
  if (widget.sideFaderMode && widget.sideFaderMode !== 0) {
    sideFader = document.createElement("div");
    sideFader.className = "cue-sidefader";
    const suffix = widget.sideFaderMode === 1 ? "%" : "";
    const topValue = document.createElement("div");
    topValue.className = "cue-side-value";
    topValue.textContent = `${widget.sideFaderLevel ?? 0}${suffix}`;

    const topLabel = document.createElement("div");
    topLabel.className = "cue-side-label";
    const bottomLabel = document.createElement("div");
    bottomLabel.className = "cue-side-label";

    const sliderWrap = document.createElement("div");
    sliderWrap.className = "cue-side-slider";
    const sfInput = document.createElement("input");
    sfInput.type = "range";
    sfInput.className = "range-vertical";
    sfInput.min = 0;
    sfInput.max = widget.sideFaderMode === 1 ? 100 : 255;
    sfInput.value = widget.sideFaderLevel ?? 0;
    const listItemHeight = state.pixelDensity * 7;
    const labelsCount = widget.sideFaderMode === 1 ? 4 : 2;
    const padding = state.pixelDensity * 4;
    const sfLength = Math.max(40, widget.geometry.h - listItemHeight * labelsCount - padding);
    sfInput.style.setProperty("--slider-length", `${sfLength}px`);
    const iconSize = state.pixelDensity * 10;
    const sideWidth = iconSize * 1.2;
    const thumbWidth = Math.min(iconSize, sideWidth);
    const thumbHeight = Math.min(sideWidth, iconSize * 0.75);
    sfInput.style.setProperty("--slider-thumb-width", `${thumbHeight}px`);
    sfInput.style.setProperty("--slider-thumb-height", `${thumbWidth}px`);
    sfInput.style.setProperty("--slider-fill-color", "#38b0ff");
    sfInput.style.setProperty("--slider-empty-color", "#888888");
    const sfMin = parseInt(sfInput.min, 10);
    const sfMax = parseInt(sfInput.max, 10);
    const sfVal = parseInt(sfInput.value, 10);
    const sfFill = sfMax > sfMin ? ((sfVal - sfMin) / (sfMax - sfMin)) * 100 : 0;
    sfInput.style.setProperty("--slider-fill", `${sfFill}%`);
    sfInput.addEventListener("input", () => {
      const value = parseInt(sfInput.value, 10);
      const fill = sfMax > sfMin ? ((value - sfMin) / (sfMax - sfMin)) * 100 : 0;
      sfInput.style.setProperty("--slider-fill", `${fill}%`);
      const cueWidget = state.widgets[widget.id];
      if (cueWidget) {
        cueWidget.data.sideFaderLevel = value;
        updateCueSideLabels(cueWidget);
      }
      sendWidgetCommand(widget.id, "CUE_SIDECHANGE", sfInput.value);
    });
    sliderWrap.appendChild(sfInput);

    const bottomValue = document.createElement("div");
    bottomValue.className = "cue-side-value";
    bottomValue.textContent =
      widget.sideFaderMode === 1 ? `${(sfInput.max - sfInput.value)}${suffix}` : "";

    sideFader.append(topValue, topLabel, sliderWrap, bottomLabel, bottomValue);
    root.insertBefore(sideFader, content);
    sideFader._topValue = topValue;
    sideFader._bottomValue = bottomValue;
    sideFader._topLabel = topLabel;
    sideFader._bottomLabel = bottomLabel;
    sideFader._input = sfInput;
  }

  state.widgets[widget.id] = {
    type: "cuelist",
    el: root,
    stepsEl,
    playBtn,
    stopBtn,
    prevBtn,
    nextBtn,
    sideFader,
    data: widget,
  };
  updateCueState(widget.id, widget.playbackStatus, widget.playbackIndex);
  return root;
}

function renderFrame(widget, isSolo) {
  const root = applyWidgetBase(document.createElement("div"), widget);
  root.classList.add(isSolo ? "vc-soloframe" : "vc-frame");

  let headerHeight = 0;
  let pageSelect = null;
  let enableBtn = null;
  let collapseBtn = null;

  if (widget.showHeader) {
    const header = document.createElement("div");
    header.className = "vc-frame-header";
    headerHeight = 28;

    collapseBtn = document.createElement("button");
    collapseBtn.className = "frame-btn";
    collapseBtn.innerHTML = `<span class="fa-icon">${widget.isCollapsed ? FA.expand : FA.collapse}</span>`;
    collapseBtn.title = "Expand/Collapse";
    collapseBtn.addEventListener("click", () => {
      const next = widget.isCollapsed ? 0 : 1;
      sendWidgetCommand(widget.id, "COLLAPSE", next);
      widget.isCollapsed = next === 1;
      if (content) content.style.display = widget.isCollapsed ? "none" : "";
      collapseBtn.innerHTML = `<span class="fa-icon">${widget.isCollapsed ? FA.expand : FA.collapse}</span>`;
    });
    header.appendChild(collapseBtn);

    const title = document.createElement("div");
    title.className = "frame-title";
    title.textContent = widget.caption || "";
    if (!widget.caption) title.classList.add("is-empty");
    header.appendChild(title);

    const controls = document.createElement("div");
    controls.className = "frame-controls";

    if (widget.showEnable) {
      enableBtn = document.createElement("button");
      enableBtn.className = "frame-btn frame-enable";
      enableBtn.innerHTML = `<span class="fa-icon">${FA.check}</span>`;
      enableBtn.classList.toggle("is-disabled", widget.disabled);
      enableBtn.addEventListener("click", () => {
        const nextDisabled = !widget.disabled;
        widget.disabled = nextDisabled;
        enableBtn.classList.toggle("is-disabled", nextDisabled);
        sendWidgetCommand(widget.id, "FRAME_DISABLE", nextDisabled ? 1 : 0);
      });
      controls.append(enableBtn);
    }

    if (widget.multiPageMode) {
      const prevBtn = document.createElement("button");
      prevBtn.className = "frame-btn";
      prevBtn.innerHTML = `<span class="fa-icon">${FA.angleLeft}</span>`;
      prevBtn.addEventListener("click", () => {
        const nextPage = Math.max(0, (widget.currentPage || 0) - 1);
        applyFramePage(widget.id, nextPage);
        sendWidgetCommand(widget.id, "PREV_PG");
      });

      pageSelect = document.createElement("select");
      pageSelect.className = "frame-select";
      const labels = widget.pageLabels || [];
      for (let i = 0; i < (widget.totalPages || labels.length); i += 1) {
        const opt = document.createElement("option");
        opt.value = i;
        opt.textContent = labels[i] || `Page ${i + 1}`;
        pageSelect.appendChild(opt);
      }
      pageSelect.value = widget.currentPage;
      pageSelect.addEventListener("change", () => {
        const nextPage = parseInt(pageSelect.value, 10);
        applyFramePage(widget.id, nextPage);
        sendWidgetCommand(widget.id, "PAGE", nextPage);
      });

      const nextBtn = document.createElement("button");
      nextBtn.className = "frame-btn";
      nextBtn.innerHTML = `<span class="fa-icon">${FA.angleRight}</span>`;
      nextBtn.addEventListener("click", () => {
        const maxPage = Math.max(0, (widget.totalPages || 1) - 1);
        const nextPage = Math.min(maxPage, (widget.currentPage || 0) + 1);
        applyFramePage(widget.id, nextPage);
        sendWidgetCommand(widget.id, "NEXT_PG");
      });
      controls.append(prevBtn, pageSelect, nextBtn);
    }

    header.appendChild(controls);
    root.appendChild(header);
  }

  const content = document.createElement("div");
  content.className = "vc-frame-content";
  content.style.top = "0px";
  content.style.height = "100%";
  content.style.position = "absolute";
  content.style.left = "0";
  if (widget.isCollapsed) content.style.display = "none";
  root.appendChild(content);

  (widget.children || []).forEach((child) => {
    const childEl = renderWidget(child);
    content.appendChild(childEl);
  });

  state.widgets[widget.id] = {
    type: "frame",
    el: root,
    content,
    pageSelect,
    enableBtn,
    collapseBtn,
    data: widget,
  };

  if (widget.multiPageMode) {
    applyFramePage(widget.id, widget.currentPage);
  }

  return root;
}

function renderWidget(widget) {
  switch (widget.typeId) {
    case VC_TYPE.Button:
      return renderButton(widget);
    case VC_TYPE.Slider:
      return renderSlider(widget);
    case VC_TYPE.Label:
      return renderLabel(widget);
    case VC_TYPE.XYPad:
      return renderXYPad(widget);
    case VC_TYPE.AudioTriggers:
      return renderAudioTriggers(widget);
    case VC_TYPE.CueList:
      return renderCueList(widget);
    case VC_TYPE.Frame:
      return renderFrame(widget, false);
    case VC_TYPE.SoloFrame:
      return renderFrame(widget, true);
    case VC_TYPE.Animation:
      return renderMatrix(widget);
    case VC_TYPE.Speed:
      return renderSpeed(widget);
    case VC_TYPE.Clock:
      return renderClock(widget);
    default: {
      const fallback = applyWidgetBase(document.createElement("div"), widget);
      fallback.textContent = widget.caption || widget.type || "Widget";
      return fallback;
    }
  }
}

function applyFramePage(id, pageIndex) {
  const widget = state.widgets[id];
  if (!widget || !widget.content) return;

  widget.data.currentPage = pageIndex;
  if (widget.pageSelect) {
    widget.pageSelect.value = pageIndex;
  }
  const children = widget.content.children;
  for (const child of children) {
    const pageAttr = child.dataset.page;
    if (pageAttr === undefined || pageAttr === null) continue;
    const childPage = parseInt(pageAttr, 10);
    child.style.display = childPage === pageIndex ? "" : "none";
  }

  requestAnimationFrame(() => {
    const visible = widget.content.querySelectorAll(".vc-widget:not([style*=\"display: none\"])");
    visible.forEach((child) => {
      const isDisabled = child.dataset.disabled === "true";
      child.classList.toggle("is-disabled", isDisabled);
    });
    const sliders = widget.content.querySelectorAll(".vc-slider");
    sliders.forEach((slider) => {
      const track = slider.querySelector(".slider-track");
      const input = slider.querySelector('input[type="range"]');
      if (!track || !input) return;
      const trackHeight = track.clientHeight || 40;
      input.style.setProperty("--slider-length", `${trackHeight}px`);
    });
    const faders = widget.content.querySelectorAll(".animation-fader input[type=\"range\"]");
    faders.forEach((fader) => {
      const wrap = fader.parentElement;
      if (!wrap) return;
      const height = wrap.clientHeight || 40;
      fader.style.setProperty("--slider-length", `${height}px`);
    });
  });
}

function renderPages(vcData) {
  pagesBar.innerHTML = "";
  vcRoot.innerHTML = "";
  state.widgets = {};

  state.pages = vcData.pages || [];
  state.selectedPage = vcData.selectedPage || 0;

  state.pages.forEach((page, idx) => {
    const tab = document.createElement("div");
    tab.className = "page-tab";
    tab.textContent = page.caption || `Page ${idx + 1}`;
    tab.addEventListener("click", () => {
      setSelectedPage(idx);
      sendMessage(`VC_PAGE|${idx}`);
    });
    pagesBar.appendChild(tab);

    const pageEl = document.createElement("div");
    pageEl.className = "vc-page";
    pageEl.dataset.pageIndex = idx;
    pageEl.style.position = "absolute";
    pageEl.style.left = "0px";
    pageEl.style.top = "0px";
    pageEl.style.width = `${page.geometry.w}px`;
    pageEl.style.height = `${page.geometry.h}px`;
    pageEl.style.backgroundColor = page.bgColor || "#333333";
    if (page.bgImage) {
      pageEl.style.backgroundImage = `url(${page.bgImage})`;
      pageEl.style.backgroundSize = "contain";
      pageEl.style.backgroundPosition = "center";
      pageEl.style.backgroundRepeat = "no-repeat";
    }
    if (page.fgColor) pageEl.style.color = page.fgColor;

    (page.children || []).forEach((child) => {
      const el = renderWidget(child);
      pageEl.appendChild(el);
    });

    vcRoot.appendChild(pageEl);
  });

  setSelectedPage(state.selectedPage);
}

function setSelectedPage(index) {
  state.selectedPage = index;
  const tabs = pagesBar.querySelectorAll(".page-tab");
  tabs.forEach((tab, idx) => tab.classList.toggle("active", idx === index));

  const pages = vcRoot.querySelectorAll(".vc-page");
  pages.forEach((pageEl) => {
    const idx = parseInt(pageEl.dataset.pageIndex, 10);
    pageEl.style.display = idx === index ? "" : "none";
  });
}

function renderVC(vcData) {
  if (!vcData) return;
  applyUiStyle(vcData.uiStyle);
  const appName = vcData.app?.name || "QLC+";
  const appVersion = vcData.app?.version || "";
  if (brandTitle) brandTitle.textContent = appName;
  appMeta.textContent = appVersion;
  renderPages(vcData);
}

function updateButtonState(id, stateValue) {
  const widget = state.widgets[id];
  if (!widget) return;
  const val = parseInt(stateValue, 10);
  const isActive = val === 255 || val === 2;
  const isMonitoring = val === 127 || val === 1;
  widget.el.classList.toggle("state-active", isActive);
  widget.el.classList.toggle("state-monitoring", isMonitoring);
  widget.data.state = val;
}

function updateSlider(id, value, displayValue) {
  const widget = state.widgets[id];
  if (!widget) return;
  widget.input.value = value;
  widget.valueLabel.textContent = displayValue || sliderDisplayValue(widget.data, parseInt(value, 10));
  const min = parseInt(widget.input.min, 10);
  const max = parseInt(widget.input.max, 10);
  const val = parseInt(value, 10);
  const fill = max > min ? ((val - min) / (max - min)) * 100 : 0;
  widget.input.style.setProperty("--slider-fill", `${fill}%`);
}

function updateAudioState(id, enabled) {
  const widget = state.widgets[id];
  if (!widget) return;
  if (!["0", "1", "true", "false"].includes(enabled)) return;
  const nextEnabled = enabled === "1" || enabled === "true";
  widget.state.enabled = nextEnabled;
  widget.updateEnableUI(nextEnabled);
}

function updateAudioVolume(id, value) {
  const widget = state.widgets[id];
  if (!widget?.fader) return;
  const volume = parseInt(value, 10);
  if (Number.isNaN(volume)) return;
  widget.state.volume = volume;
  widget.fader.value = volume;
  widget.fader.style.setProperty("--slider-fill", `${(volume / 100) * 100}%`);
}

function updateCueSideLabels(widget) {
  if (!widget?.sideFader) return;

  const mode = widget.data?.sideFaderMode ?? 0;
  const playbackIndex = widget.data?.playbackIndex ?? -1;
  const nextIndex = widget.data?.nextStepIndex ?? -1;
  const primaryTop = widget.data?.primaryTop ?? true;
  const input = widget.sideFader._input;
  const suffix = mode === 1 ? "%" : "";
  const max = parseInt(input?.max ?? "100", 10);
  const value = parseInt(input?.value ?? "0", 10);
  const fill = max > 0 ? (value / max) * 100 : 0;

  if (input) input.style.setProperty("--slider-fill", `${fill}%`);

  widget.sideFader._topValue.textContent = `${value}${suffix}`;
  widget.sideFader._bottomValue.textContent = mode === 1 ? `${max - value}${suffix}` : "";

  const topLabel = widget.sideFader._topLabel;
  const bottomLabel = widget.sideFader._bottomLabel;

  if (mode === 2) {
    topLabel.style.display = "none";
    widget.sideFader._bottomValue.style.display = "none";
  } else {
    topLabel.style.display = "";
    widget.sideFader._bottomValue.style.display = "";
  }

  if (playbackIndex < 0) {
    topLabel.textContent = "";
    bottomLabel.textContent = "";
    topLabel.style.background = "transparent";
    bottomLabel.style.background = "transparent";
    return;
  }

  if (mode === 2) {
    topLabel.textContent = "";
    topLabel.style.background = "transparent";
    bottomLabel.textContent = `#${playbackIndex + 1}`;
    bottomLabel.style.background = "var(--highlight)";
  } else {
    const topIndex = primaryTop ? playbackIndex : nextIndex;
    const bottomIndex = primaryTop ? nextIndex : playbackIndex;
    topLabel.textContent = topIndex >= 0 ? `#${topIndex + 1}` : "";
    bottomLabel.textContent = bottomIndex >= 0 ? `#${bottomIndex + 1}` : "";
    topLabel.style.background = primaryTop ? "var(--highlight)" : "orange";
    bottomLabel.style.background = primaryTop ? "orange" : "var(--highlight)";
  }
}

function updateCueState(id, status, index, nextIndex, sideLevel, primaryTop) {
  const widget = state.widgets[id];
  if (!widget) return;
  const statusVal = parseInt(status, 10);
  widget.data.playbackStatus = statusVal;
  widget.data.playbackIndex = index;
  if (typeof nextIndex !== "undefined") widget.data.nextStepIndex = nextIndex;
  if (typeof sideLevel !== "undefined") widget.data.sideFaderLevel = sideLevel;
  if (typeof primaryTop !== "undefined") widget.data.primaryTop = primaryTop;

  if (widget.playBtn && widget.stopBtn) {
    const layout = widget.data.playbackLayout ?? 0;
    if (layout === 0) {
      widget.playBtn.innerHTML = `<span class="fa-icon">${statusVal === 1 ? FA.pause : FA.play}</span>`;
      widget.stopBtn.innerHTML = `<span class="fa-icon">${FA.stop}</span>`;
      widget.playBtn.style.background = statusVal === 1 ? "darkorange" : statusVal === 2 ? "green" : "var(--bg-light)";
      widget.stopBtn.style.background = statusVal === 0 ? "var(--bg-light)" : "red";
    } else {
      widget.playBtn.innerHTML = `<span class="fa-icon">${statusVal === 0 ? FA.play : FA.stop}</span>`;
      widget.stopBtn.innerHTML = `<span class="fa-icon">${FA.pause}</span>`;
      widget.playBtn.style.background = statusVal === 0 ? "var(--bg-light)" : "red";
      widget.stopBtn.style.background = statusVal === 2 ? "darkorange" : "var(--bg-light)";
    }
  }
  const rows = widget.stepsEl.querySelectorAll(".cue-step");
  rows.forEach((row) => {
    row.classList.toggle("active", parseInt(row.dataset.index, 10) === index);
  });

  if (widget.sideFader?._input && typeof sideLevel !== "undefined") {
    widget.sideFader._input.value = sideLevel;
  }
  updateCueSideLabels(widget);
}

function updateMatrixState(id, fader, algorithmIndex, colors) {
  const widget = state.widgets[id];
  if (!widget) return;
  widget.data.faderLevel = parseInt(fader, 10);
  widget.data.algorithmIndex = parseInt(algorithmIndex, 10);
  if (widget.matrixState?.fader) {
    widget.matrixState.fader.value = fader;
    const fill = (parseInt(fader, 10) / 255) * 100;
    widget.matrixState.fader.style.setProperty("--slider-fill", `${fill}%`);
  }
  if (widget.matrixState?.combo) widget.matrixState.combo.value = algorithmIndex;
  if (widget.matrixState?.colors) {
    const keys = ["color1", "color2", "color3", "color4", "color5"];
    keys.forEach((key, idx) => {
      const input = widget.matrixState.colors[key];
      if (input && colors[idx]) input.value = colors[idx];
    });
  }
}

function updateSpeedState(id, time, factor) {
  const widget = state.widgets[id];
  if (!widget) return;
  widget.data.currentTime = parseInt(time, 10) || 0;
  widget.data.currentFactor = parseInt(factor, 10) || 0;

  if (widget.timeInputs) {
    const parts = splitMilliseconds(widget.data.currentTime);
    if (widget.timeInputs.hours) widget.timeInputs.hours.value = parts.hours;
    if (widget.timeInputs.minutes) widget.timeInputs.minutes.value = parts.minutes;
    if (widget.timeInputs.seconds) widget.timeInputs.seconds.value = parts.seconds;
    if (widget.timeInputs.ms) widget.timeInputs.ms.value = parts.ms;
  }

  if (widget.beatButtons) {
    Object.entries(widget.beatButtons).forEach(([key, btn]) => {
      btn.classList.toggle("is-active", parseInt(key, 10) === widget.data.currentFactor);
    });
  }

  if (widget.multiplierLabel) {
    const label = SPEED_LABELS[widget.data.currentFactor] || "";
    widget.multiplierLabel.top.textContent = `${label}x`;
    widget.multiplierLabel.bottom.textContent = timeToQlcString(widget.data.currentTime, 0);
  }

  if (Array.isArray(widget.presets)) {
    widget.presets.forEach((preset) => {
      const value = parseInt(preset.value, 10) || 0;
      preset.el.classList.toggle("is-active", value === widget.data.currentTime);
    });
  }
}

function updateClock(id, time) {
  const widget = state.widgets[id];
  if (!widget) return;
  if (widget.el && widget.el.style.display === "none") {
    return;
  }
  if (widget.data.clockType === 0) {
    widget.display.textContent = formatTime(parseInt(time, 10));
  }
}

function updateSliderOverride(id, isOverriding) {
  const widget = state.widgets[id];
  if (!widget?.resetBtn) return;
  widget.resetBtn.classList.toggle("is-overriding", isOverriding);
  widget.data.isOverriding = isOverriding;
}

function handleSocketMessage(ev) {
  const msg = ev.data.split("|");
  if (msg[0] === "QLC+API") {
    if (msg[1] === "isProjectLoaded" && msg[2] === "true") fetchVC();
    return;
  }

  if (msg[0] === "GM_VALUE") {
    return;
  }

  if (msg[0] === "FUNCTION") {
    return;
  }

  if (msg[0] === "VC_PAGE") {
    setSelectedPage(parseInt(msg[1], 10));
    return;
  }

  const id = parseInt(msg[0], 10);
  const cmd = msg[1];
  if (Number.isNaN(id)) return;

  switch (cmd) {
    case "BUTTON":
      updateButtonState(id, msg[2]);
      break;
    case "BUTTON_DISABLE":
    case "SLIDER_DISABLE":
    case "LABEL_DISABLE":
    case "FRAME_DISABLE":
    case "CUE_DISABLE":
    case "WIDGET_DISABLE": {
      const widget = state.widgets[id];
      const isDisabled = msg[2] === "1" || msg[2] === "true";
      if (widget) {
        widget.el.classList.toggle("is-disabled", isDisabled);
        widget.el.dataset.disabled = isDisabled ? "true" : "false";
        widget.data.disabled = isDisabled;
      }
      if (cmd === "FRAME_DISABLE" && widget?.enableBtn) {
        widget.data.disabled = isDisabled;
        widget.enableBtn.classList.toggle("is-disabled", isDisabled);
      }
      break;
    }
    case "SLIDER":
      updateSlider(id, msg[2], msg[3]);
      break;
    case "SLIDER_OVERRIDE":
      updateSliderOverride(id, msg[2] === "1" || msg[2] === "true");
      break;
    case "AUDIOTRIGGERS":
      updateAudioState(id, msg[2]);
      break;
    case "AUDIO_VOLUME":
      updateAudioVolume(id, msg[2]);
      break;
    case "CUE":
      updateCueState(id, state.widgets[id]?.data.playbackStatus ?? 0, parseInt(msg[2], 10));
      break;
    case "CUE_STATE":
      updateCueState(
        id,
        parseInt(msg[2], 10),
        parseInt(msg[3], 10),
        parseInt(msg[4], 10),
        parseInt(msg[5], 10),
        msg[6] === "1" || msg[6] === "true"
      );
      break;
    case "CUE_SIDE": {
      const widget = state.widgets[id];
      if (widget?.sideFader) {
        const input = widget.sideFader._input;
        if (input) input.value = msg[2];
        widget.data.sideFaderLevel = parseInt(msg[2], 10);
        widget.data.primaryTop = msg[3] === "1" || msg[3] === "true";
        updateCueSideLabels(widget);
      }
      break;
    }
    case "FRAME":
      applyFramePage(id, parseInt(msg[2], 10));
      break;
    case "MATRIX_STATE":
      updateMatrixState(id, msg[2], msg[3], msg.slice(4));
      break;
    case "XYPAD": {
      const widget = state.widgets[id];
      if (widget?.setHandle) widget.setHandle(parseFloat(msg[2]), parseFloat(msg[3]));
      break;
    }
    case "SPEED_STATE":
      updateSpeedState(id, msg[2], msg[3]);
      break;
    case "CLOCK":
      updateClock(id, msg[2]);
      break;
    default:
      break;
  }
}

function fetchVC() {
  fetch(`/vc.json?ts=${Date.now()}`)
    .then((res) => res.json())
    .then(renderVC)
    .catch((err) => console.error("VC JSON error", err));
}

function startProjectPolling() {
  if (state.projectPoll) clearInterval(state.projectPoll);
  state.projectPoll = setInterval(() => {
    sendMessage("QLC+API|isProjectLoaded");
  }, 1500);
}

function connect() {
  const url = `ws://${window.location.host}/qlcplusWS`;
  state.socket = new WebSocket(url);

  state.socket.onopen = () => {
    setStatus(true);
    fetchVC();
    startProjectPolling();
  };

  state.socket.onclose = () => {
    setStatus(false);
    if (state.projectPoll) clearInterval(state.projectPoll);
    setTimeout(connect, 1000);
  };

  state.socket.onerror = () => {
    state.socket.close();
  };

  state.socket.onmessage = handleSocketMessage;
}

window.addEventListener("load", () => {
  updateWebPixelDensity();
  window.addEventListener("resize", () => updateWebPixelDensity());
  document.getElementById("loadProjectBtn").addEventListener("click", () => {
    document.getElementById("loadTrigger").click();
  });
  document.getElementById("loadTrigger").addEventListener("change", () => {
    document.getElementById("submitTrigger").click();
  });
  connect();
});
