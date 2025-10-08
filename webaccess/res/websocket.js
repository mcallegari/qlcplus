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

function connect() {
  var url = "ws://" + window.location.host + "/qlcplusWS";
  websocket = new WebSocket(url);
  websocket.onopen = function () {
    //alert(\"Websocket open!\");
  };

  websocket.onclose = function () {
    console.log(
      "QLC+ connection is closed. Reconnect will be attempted in 1 second."
    );
    setTimeout(function () {
      connect();
    }, 1000);
  };

  websocket.onerror = function () {
    console.error("QLC+ connection encountered error. Closing socket");
    ws.close();
  };

  websocket.onmessage = function (ev) {
    //console.log(ev.data);
    var msgParams = ev.data.split("|");
    if (msgParams[0] === "GM_VALUE") {
      grandMasterValueChanged(msgParams[1], msgParams[2]);
    }

    if (msgParams[1] === "BUTTON") {
      wsSetButtonState(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "BUTTON_DISABLE") {
      setButtonDisableState(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "LABEL_DISABLE") {
      setLabelDisableState(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "SLIDER") {
      // Slider message is <ID>|SLIDER|<SLIDER VALUE>|<DISPLAY VALUE>
      wsSetSliderValue(msgParams[0], msgParams[2], msgParams[3]);
    } else if (msgParams[1] === "SLIDER_DISABLE") {
      setSliderDisableState(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "AUDIOTRIGGERS") {
      wsSetAudioTriggersEnabled(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "CUE") {
      wsSetCueIndex(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "CUE_DISABLE") {
      setCueDisableState(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "CLOCK") {
      wsUpdateClockTime(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "CLOCK_DISABLE") {
      setClockDisableState(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "FRAME") {
      setFramePage(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "FRAME_DISABLE") {
      setFrameDisableState(msgParams[0], msgParams[2]);
    } else if (msgParams[0] === "ALERT") {
      alert(msgParams[1]);
    } else if (msgParams[1] === "CUE_STEP_NOTE") {
      setCueStepNote(msgParams[0], msgParams[2], msgParams[3]);
    } else if (msgParams[1] === "CUE_PROGRESS") {
      // CUE message is <ID>|CUE_PERCENT|<PERCENT>|<TEXT>
      setCueProgress(msgParams[0], msgParams[2], msgParams[3]);
    } else if (msgParams[1] === "CUE_SIDECHANGE") {
      // CUE message is <ID>|CUE_SIDECHANGE|<TOP_PERCENT>|<BOTTOMPERCENT>|<TOP_STEP>|<BOTTOM_STEP>|<PRIMARY_TOP>|<STEP_VALUE>|<IS_STEPS_MODE>
      setCueSideFaderValues(msgParams[0], msgParams[2], msgParams[3], msgParams[4], msgParams[5], msgParams[6], msgParams[7], msgParams[8]);
    } else if (msgParams[1] === "CUE_SHOWPANEL") {
      showSideFaderPanel(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "CUE_CHANGE") {
      setCueButtonStyle(msgParams[0], msgParams[2], msgParams[3], msgParams[4], msgParams[5]);
    } else if (msgParams[1] === "MATRIX_SLIDER") {
      setMatrixSliderValue(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "MATRIX_COLOR_1") {
      setMatrixStartColorValue(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "MATRIX_COLOR_2") {
      setMatrixEndColorValue(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "MATRIX_COLOR_3") {
      setMatrixEndColorValue(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "MATRIX_COLOR_4") {
      setMatrixEndColorValue(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "MATRIX_COLOR_5") {
      setMatrixEndColorValue(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "MATRIX_COMBO") {
      setMatrixComboValue(msgParams[0], msgParams[2]);
    } else if (msgParams[1] === "MATRIX_KNOB") {
      setMatrixControlKnobValue(msgParams[2], msgParams[3]);
    }
  };
  initVirtualConsole();
}

window.onload = connect();
