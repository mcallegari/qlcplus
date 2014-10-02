#ifndef COMMONJSCSS_H
#define COMMONJSCSS_H


#define HTML_HEADER \
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n" \
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n" \
    "<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"

#define WEBSOCKET_JS \
    "var websocket;\n" \
    "function sendCMD(cmd) {\n" \
    " websocket.send(\"QLC+CMD|\" + cmd);\n" \
    "}\n\n" \
    "window.onload = function() {\n" \
    " var url = 'ws://' + window.location.host + '/qlcplusWS';\n" \
    " websocket = new WebSocket(url);\n" \
    " websocket.onopen = function(ev) {\n" \
    "  //alert(\"Websocket open!\");\n" \
    " };\n\n" \
    " websocket.onclose = function(ev) {\n" \
    "  alert(\"QLC+ connection lost !\");\n" \
    " };\n\n" \
    " websocket.onerror = function(ev) {\n" \
    "  alert(\"QLC+ connection error!\");\n" \
    " };\n" \
    " websocket.onmessage = function(ev) {\n" \
    "  //alert(ev.data);\n" \
    "  var msgParams = ev.data.split('|');\n" \
    "  var obj = document.getElementById(msgParams[0]);\n" \
    "  if (msgParams[1] == \"BUTTON\") {\n" \
    "    if (msgParams[2] == 1) { obj.value = \"255\";\n obj.style.border = \"3px solid #00E600\"; }\n" \
    "    else { obj.value = \"0\";\n obj.style.border = \"3px solid #A0A0A0\"; }\n" \
    "  }\n" \
    "  else if (msgParams[1] == \"SLIDER\") {\n" \
    "    obj.value = msgParams[2];\n" \
    "    var labelObj = document.getElementById(\"slv\" + msgParams[0]);\n" \
    "    labelObj.innerHTML = msgParams[2];\n" \
    "  }\n" \
    "  else if (msgParams[1] == \"CUE\") {\n" \
    "    setCueIndex(msgParams[0], msgParams[2]);\n" \
    "    var playBbj = document.getElementById(\"play\" + msgParams[0]);\n" \
    "    if (msgParams[2] == \"-1\")\n" \
    "      playBbj.innerHTML = \"<img src='player_play.png'' width=27></img>\";\n" \
    "    else\n" \
    "      playBbj.innerHTML = \"<img src='player_stop.png'' width=27></img>\";\n" \
    "  }\n" \
    "  else if (msgParams[1] == \"FRAME\") {\n" \
    "    setFramePage(msgParams[0], msgParams[2]);\n" \
    "  }\n" \
    "  else if (msgParams[0] == \"ALERT\") {\n" \
    "    alert(msgParams[1]);\n" \
    "  }\n" \
    " };\n" \
    "};\n"

#define PROJECT_LOADED_JS \
    "var websocket;\n" \
    "window.onload = function() {\n" \
    " var url = 'ws://' + window.location.host + '/qlcplusWS';\n" \
    " websocket = new WebSocket(url);\n" \
    " setInterval(checkProjectLoaded, 100);\n" \
    " websocket.onmessage = function(ev) {\n" \
    "  var msgParams = ev.data.split('|');\n" \
    "  if (msgParams[0] == \"QLC+API\" && " \
    "      msgParams[1] == \"isProjectLoaded\" && " \
    "      msgParams[2] == \"true\")" \
    "        window.location = \"/\";\n" \
    " };\n" \
    "};\n" \
    "function checkProjectLoaded() {\n" \
    " websocket.send(\"QLC+API|isProjectLoaded\");\n" \
    "};\n"

#define HIDDEN_FORM_CSS \
    "form {\n" \
    " position: absolute;\n" \
    " top: -100px;\n" \
    " visibility: hidden;\n" \
    "}\n\n"

#define CONTROL_BAR_CSS \
    ".controlBar {\n" \
    " width: 100%;\n" \
    " height: 40px;\n" \
    " background: linear-gradient(to bottom, #B2D360 0%, #4B9002 100%);\n" \
    " background: -ms-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n" \
    " background: -moz-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n" \
    " background: -o-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n" \
    " background: -webkit-gradient(linear, left top, left bottom, color-stop(0, #B2D360), color-stop(1, #4B9002));\n" \
    " background: -webkit-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n" \
    " font:bold 24px/1.2em sans-serif;\n" \
    " color: #ffffff;\n" \
    "}\n\n"

#define BUTTON_BASE_CSS \
    ".button\n" \
    "{\n" \
    " height: 35px;\n" \
    " margin-left: 5px;" \
    " text-decoration: none;\n" \
    " font: bold 25px/1.2em 'Trebuchet MS',Arial, Helvetica;\n" \
    " display: inline-block;\n" \
    " text-align: center;\n" \
    " color: #fff;\n" \
    " border: 1px solid #9c9c9c;\n" \
    " border: 1px solid rgba(0, 0, 0, 0.3);\n" \
    " text-shadow: 0 1px 0 rgba(0,0,0,0.4);\n" \
    " box-shadow: 0 0 .05em rgba(0,0,0,0.4);\n" \
    " -moz-box-shadow: 0 0 .05em rgba(0,0,0,0.4);\n" \
    " -webkit-box-shadow: 0 0 .05em rgba(0,0,0,0.4);\n" \
    "}\n\n"

#define BUTTON_SPAN_CSS \
    ".button, .button span  {\n" \
    " -moz-border-radius: .3em;\n" \
    " border-radius: .3em;\n" \
    "}\n\n" \
    ".button span {\n" \
    " border-top: 1px solid #fff;\n" \
    " border-top: 1px solid rgba(255, 255, 255, 0.5);\n" \
    " display: block;\n" \
    " padding: 0 10px 0 10px;\n" \
    " background-image: -webkit-gradient(linear, 0 0, 100% 100%, color-stop(.25, rgba(0, 0, 0, 0.05)), color-stop(.25, transparent), to(transparent)),\n" \
    " background-image: -moz-linear-gradient(45deg, rgba(0, 0, 0, 0.05) 25%, transparent 25%, transparent),\n" \
    "}\n\n"

#define BUTTON_STATE_CSS \
    ".button:hover {\n" \
    " box-shadow: 0 0 .1em rgba(0,0,0,0.4);\n" \
    " -moz-box-shadow: 0 0 .1em rgba(0,0,0,0.4);\n" \
    " -webkit-box-shadow: 0 0 .1em rgba(0,0,0,0.4);\n" \
    "}\n\n" \
    ".button:active {\n" \
    " position: relative;\n" \
    " top: 1px;\n" \
    "}\n\n"

#define BUTTON_BLUE_CSS \
    ".button-blue {\n" \
    " background: #4477a1;\n" \
    " background: -webkit-gradient(linear, left top, left bottom, from(#81a8cb), to(#4477a1) );\n" \
    " background: -moz-linear-gradient(-90deg, #81a8cb, #4477a1);\n" \
    "}\n\n" \
    ".button-blue:hover {\n" \
    " background: #81a8cb;\n" \
    " background: -webkit-gradient(linear, left top, left bottom, from(#4477a1), to(#81a8cb) );\n" \
    " background: -moz-linear-gradient(-90deg, #4477a1, #81a8cb);\n" \
    "}\n\n" \
    ".button-blue:active { background: #4477a1; }\n\n"

#define SWINFO_CSS \
    ".swInfo {\n" \
    " position: absolute;\n" \
    " right: 5px;\n" \
    " top: 5px;\n" \
    " font-size: 18px;\n" \
    "}\n\n"

#define TABLE_CSS \
    "table.hovertable {\n" \
    " font-family: verdana,arial,sans-serif;\n" \
    " font-size:11px;\n" \
    " color:#333333;\n" \
    " border-width: 1px;\n" \
    " border-color: #999999;\n" \
    " border-collapse: collapse;\n" \
    "}\n\n" \
    "table.hovertable th {\n" \
    " background-color:#DCD9D6;\n" \
    " border-width: 1px;\n" \
    " padding: 3px;\n" \
    " border-style: solid;\n" \
    " border-color: #a9c6c9;\n" \
    "}\n\n" \
    "table.hovertable tr {\n" \
    " background-color:#ffffff;\n" \
    "}\n\n" \
    "table.hovertable td {\n" \
    " border-width: 1px;\n" \
    " padding: 3px;\n" \
    " border-style: solid;\n" \
    " border-color: #a9c6c9;\n" \
    "}\n\n"

#define SLIDER_CSS \
        ".sdSlider {\n" \
        "position: relative;\n" \
        "display: inline-block;\n" \
        "border: 1px solid #777777;\n" \
        "border-radius: 3px;\n" \
        "}\n\n" \
        ".sdslLabel {\n" \
        "height:20px;\n" \
        "text-align:center;\n" \
        "font:normal 16px sans-serif;\n" \
        "}\n\n" \
        "input[type=\"range\"].vVertical {\n" \
        "-webkit-appearance: none;\n" \
        "height: 4px;\n" \
        "border: 1px solid #8E8A86;\n" \
        "background-color: #888888;\n" \
        "-webkit-transform:rotate(270deg);\n" \
        "-webkit-transform-origin: 0% 50%;\n" \
        "-moz-transform:rotate(270deg);\n" \
        "-o-transform:rotate(270deg);\n" \
        "-ms-transform:rotate(270deg);\n" \
        "-ms-transform-origin:0% 50%;\n" \
        "transform:rotate(270deg);\n" \
        "transform-origin:0% 50%;\n" \
        "}\n\n" \
        "input[type=\"range\"]::-webkit-slider-thumb {\n" \
        "-webkit-appearance: none;\n" \
        "background-color: #999999;\n" \
        "border-radius: 4px;\n" \
        "border: 1px solid #5c5c5c;\n" \
        "width: 20px;\n" \
        "height: 34px;\n" \
        "}\n\n"

#endif // COMMONJSCSS_H
