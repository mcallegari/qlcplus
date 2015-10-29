#ifndef COMMONJSCSS_H
#define COMMONJSCSS_H

#define HTML_HEADER \
	"<!DOCTYPE html>\n" \
    "<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" >\n" \
	"<title>QLC+ Webaccess</title>\n"

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

#endif // COMMONJSCSS_H
