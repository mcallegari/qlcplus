var websocket;
function sendCMD(cmd) {
 websocket.send("QLC+CMD|" + cmd);
}

window.onload = function() {
 var url = 'ws://' + window.location.host + '/qlcplusWS';
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
  var msgParams = ev.data.split('|');
  var obj = document.getElementById(msgParams[0]);
  if (msgParams[1] == "BUTTON") {
    if (msgParams[2] == 1) { 
      obj.value = "255";
      obj.style.border = "3px solid #00E600"; 
    }
    else { 
      obj.value = "0";
      obj.style.border = "3px solid #A0A0A0";
    }
  }
  else if (msgParams[1] == "SLIDER") {
    obj.value = msgParams[2];
    var labelObj = document.getElementById("slv" + msgParams[0]);
    labelObj.innerHTML = msgParams[2];
  }
  else if (msgParams[1] == "CUE") {
    setCueIndex(msgParams[0], msgParams[2]);
    var playBbj = document.getElementById("play" + msgParams[0]);
    if (msgParams[2] == "-1")
      playBbj.innerHTML = "<img src=\"player_play.png\" width=\"27\">";
    else
      playBbj.innerHTML = "<img src=\"player_stop.png\" width=\"27\">";
  }
  else if (msgParams[1] == "FRAME") {
    setFramePage(msgParams[0], msgParams[2]);
  }
  else if (msgParams[0] == "ALERT") {
    alert(msgParams[1]);
  }
 };
};
