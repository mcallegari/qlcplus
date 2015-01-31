/* VCButton */
function buttonClick(id) {
 var obj = document.getElementById(id);
 if (obj.value == "0" || obj.value == undefined) {
  obj.value = "255";
  obj.style.border = "3px solid #00E600";
 }
 else {
  obj.value = "0";
  obj.style.border = "3px solid #A0A0A0";
 }
 websocket.send(id + "|" + obj.value);
}

/* VCCueList */
var cueListsIndices = new Array();

function setCueIndex(id, idx) {
 var oldIdx = cueListsIndices[id];
 if (oldIdx != undefined && oldIdx != -1) {
   var oldCueObj = document.getElementById(id + "_" + oldIdx);
   oldCueObj.style.backgroundColor='#FFFFFF';
 }
 cueListsIndices[id] = idx;
 var currCueObj = document.getElementById(id + "_" + idx);
 if (idx != "-1")
   currCueObj.style.backgroundColor='#5E7FDF';
}

function sendCueCmd(id, cmd) {
 if (cmd == "PLAY") {
   var obj = document.getElementById("play" + id);
   if (cueListsIndices[id] == -1) {
     obj.innerHTML = "<img src=\"player_stop.png\" width=\"27\">";
     setCueIndex(id, 0);
   }
   else {
     obj.innerHTML = "<img src=\"player_play.png\" width=\"27\">";
     setCueIndex(id, -1);
   }
 }
 websocket.send(id + "|" + cmd);
}

function checkMouseOut(id, idx) {
 var obj = document.getElementById(id + "_" + idx);
 if(idx == cueListsIndices[id])
   obj.style.backgroundColor='#5E7FDF';
 else
   obj.style.backgroundColor='#FFFFFF';
 }

function enableCue(id, idx) {
 var btnObj = document.getElementById("play" + id);
 btnObj.innerHTML = "<img src=\"player_stop.png\" width=\"27\">";
 setCueIndex(id, idx);
 websocket.send(id + "|STEP|" + idx);
}

/* VCFrame */
var framesTotalPages = new Array();
var framesCurrentPage = new Array();

function updateFrameLabel(id) {
 var framePageObj = document.getElementById("fr" + id + "Page");
 var newLabel = "Page " + (framesCurrentPage[id] + 1);
 framePageObj.innerHTML = newLabel;
}

function frameNextPage(id) {
 var currPage = framesCurrentPage[id];
 var pagesNum = framesTotalPages[id];
 if (currPage + 1 < pagesNum) {
  var framePageObj = document.getElementById("fp" + id + "_" + currPage);
  framePageObj.style.visibility = 'hidden';
  framesCurrentPage[id]++;
  var frameNextPageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
  frameNextPageObj.style.visibility = 'visible';
  updateFrameLabel(id);
  websocket.send(id + "|NEXT_PG");
 }
}

function framePreviousPage(id) {
 var currPage = framesCurrentPage[id];
 if (currPage - 1 >= 0) {
  var framePageObj = document.getElementById("fp" + id + "_" + currPage);
  framePageObj.style.visibility = 'hidden';
  framesCurrentPage[id]--;
  var framePrevPageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
  framePrevPageObj.style.visibility = 'visible';
  updateFrameLabel(id);
  websocket.send(id + "|PREV_PG");
 }
}

function setFramePage(id, page) {
 var iPage = parseInt(page);
 if (framesCurrentPage[id] == iPage || iPage >= framesTotalPages[id]) return;
 var framePageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
 framePageObj.style.visibility = 'hidden';
 framesCurrentPage[id] = iPage;
 var frameNewPageObj = document.getElementById("fp" + id + "_" + framesCurrentPage[id]);
 frameNewPageObj.style.visibility = 'visible';
 updateFrameLabel(id);
}

/* VCSlider */
function slVchange(id) {
 var slObj = document.getElementById(id);
 var obj = document.getElementById("slv" + id);
 obj.innerHTML = slObj.value;
 var sldMsg = id + "|" + slObj.value;
 websocket.send(sldMsg);
}

/* VCAudioTriggers */
function atButtonClick(id) {
 var obj = document.getElementById(id);
 if (obj.value == "0" || obj.value == undefined) {
  obj.value = "255";
  obj.style.border = "3px solid #00E600";
  obj.style.backgroundColor = "#D7DE75";
 }
 else {
  obj.value = "0";
  obj.style.border = "3px solid #A0A0A0";
  obj.style.backgroundColor = "#D6D2D0";
 }
 var btnMsg = id + "|" + obj.value;
 websocket.send(btnMsg);
}
