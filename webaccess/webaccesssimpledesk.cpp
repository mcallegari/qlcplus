/*
  Q Light Controller Plus
  webaccesssimpledesk.cpp

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

#include <QSettings>
#include <QDebug>

#include "webaccesssimpledesk.h"
#include "commonjscss.h"
#include "simpledesk.h"
#include "qlcconfig.h"
#include "doc.h"

WebAccessSimpleDesk::WebAccessSimpleDesk(QObject *parent) :
    QObject(parent)
{
}

QString WebAccessSimpleDesk::getHTML(Doc *doc, SimpleDesk *sd)
{
    int uni = sd->getCurrentUniverseIndex() + 1;
    int page = sd->getCurrentPage();

    QString JScode = "<script language=\"javascript\" type=\"text/javascript\">\n";

    JScode += "var currentUniverse = " + QString::number(uni) + ";\n";
    JScode += "var currentPage = " + QString::number(page) + ";\n";
    JScode += "var channelsPerPage = " + QString::number(sd->getSlidersNumber()) + ";\n";

    JScode += "var websocket;\n"
              "window.onload = function() {\n"
              " var url = 'ws://' + window.location.host + '/qlcplusWS';\n"
              " websocket = new WebSocket(url);\n"
              " websocket.onopen = function(ev) {\n"
              "  getPage(1, 1);\n"
              " };\n\n"
              " websocket.onclose = function(ev) {\n"
              "  alert(\"QLC+ connection lost !\");\n"
              " };\n\n"
              " websocket.onerror = function(ev) {\n"
              "  alert(\"QLC+ connection error!\");\n"
              " };\n"
              " websocket.onmessage = function(ev) {\n"
              "  //alert(ev.data);\n"
              "  var msgParams = ev.data.split('|');\n"
              "  if (msgParams[0] == \"QLC+API\") {\n"
              "    if (msgParams[1] == \"getChannelsValues\") {\n"
              "      drawPage(ev.data);\n"
              "    }\n"
              "  }\n"
              " };\n"
              "};\n\n";

    JScode += "function getGroupIconName(grp) {\n"
              " if (grp == 0) return \"intensity.png\";\n"
              " else if (grp == 1) return \"colorwheel.png\";\n"
              " else if (grp == 2) return \"gobo.png\";\n"
              " else if (grp == 3) return \"speed.png\";\n"
              " else if (grp == 4) return \"pan.png\";\n"
              " else if (grp == 5) return \"tilt.png\";\n"
              " else if (grp == 6) return \"shutter.png\";\n"
              " else if (grp == 7) return \"prism.png\";\n"
              " else if (grp == 8) return \"beam.png\";\n"
              " else if (grp == 9) return \"star.png\";\n"
              " else if (grp == 10) return \"configure.png\";\n"
              " return \"\";\n"
              "}\n\n";

    JScode += "function getSliderTopCode(type) {\n"
              " if (type == '')\n"
              " return \"<div style='width:34px; height:34px; margin:2px 0 0 1px; background:transparent;'></div>\";"
              " var aType = type.split('.');\n"
              " if (aType.length == 1)\n"
              "   return \"<img src=\" + getGroupIconName(parseInt(type)) + \" style='margin-left:2px;'></img>\";\n"
              " else {\n"
              "  if (aType[1] == '#000000')\n"
              "    return \"<img src=\" + getGroupIconName(0) + \"></img>\";\n"
              "  else\n"
              "    return \"<div style='width:34px; height:34px; margin:2px 0 0 1px; background:\" + aType[1] + \";'></div>\";"
              " }\n"
              "}\n\n";

    JScode += "function drawPage(data) {\n"
              " var cObj = document.getElementById(\"slidersContainer\");\n"
              " var code = \"\";\n"
              " var cVars = data.split('|');\n"
              " for (i = 2; i < cVars.length; i+=3) {\n"
              "   var chNum = parseInt(cVars[i]);\n"
              "   code += \"<div class='sdSlider' style='width: 36px; height: 332px; background-color: #aaa; margin-left:2px;'>\";\n"
              "   code += getSliderTopCode(cVars[i + 2]);\n"
              "   code += \"<div id='sdslv\" + chNum + \"' class='sdslLabel' style='top:2px;'>\" + cVars[i + 1]  + \"</div>\";\n"
              "   code += \"<input type='range' class='vVertical' id='\" + chNum + \"' \";\n"
              "   code += \"oninput='sdSlVchange(\" + chNum + \");' ontouchmove='sdSlVchange(\" + chNum + \");' \";\n"
              "   code += \"style='width: 250px; margin-top: 250px; margin-left: 18px; \";\n"
              "   code += \"min='0' max='255' step='1' value='\" + cVars[i + 1] + \"' />\";\n"
              "   code += \"<div id='sdsln\" + chNum + \"' class='sdslLabel' \";\n"
              "   code += \"style='bottom:0px;'>\" + chNum + \"</div>\";\n"
              "   code += \"</div>\";\n"
              " }\n"
              " cObj.innerHTML = code;\n"
              "}\n\n";

    JScode += "function nextPage() {\n"
              " currentPage++;\n"
              " if (currentPage * channelsPerPage > 512)\n"
              "   currentPage = 1;\n"
              " var pgObj = document.getElementById(\"pageDiv\");\n"
              " pgObj.innerHTML = currentPage;\n"
              " getPage(currentUniverse, currentPage);\n"
              "}\n\n";

    JScode += "function previousPage() {\n"
              " if (currentPage == 1)\n"
              "   currentPage = (512 / channelsPerPage);\n"
              " else"
              "   currentPage--;\n"
              " var pgObj = document.getElementById(\"pageDiv\");\n"
              " pgObj.innerHTML = currentPage;\n"
              " getPage(currentUniverse, currentPage);\n"
              "}\n\n";

    JScode += "function getPage(uni, page) {\n"
              " var address = ((page - 1) * channelsPerPage) + 1;\n"
              " var wsMsg = \"QLC+API|getChannelsValues|\" + uni + \"|\" + address + \"|\" + channelsPerPage;\n"
              " websocket.send(wsMsg);\n"
              "}\n\n";

    JScode += "function universeChanged(uniIdx) {\n"
              " currentUniverse = parseInt(uniIdx) + 1;\n"
              " currentPage = 1;\n"
              " var pgObj = document.getElementById(\"pageDiv\");\n"
              " pgObj.innerHTML = currentPage;\n"
              " getPage(currentUniverse, currentPage);\n"
              "}\n\n";

    JScode += "function resetUniverse() {\n"
              " currentPage = 1;\n"
              " var wsMsg = \"QLC+API|sdResetUniverse\";\n"
              " websocket.send(wsMsg);\n"
              "}\n\n";

    JScode += "function sdSlVchange(id) {\n"
            " var slObj = document.getElementById(id);\n"
            " var labelObj = document.getElementById(\"sdslv\" + id);\n"
            " labelObj.innerHTML = slObj.value;\n"
            " var chNum = ((currentUniverse - 1) * 512) + parseInt(id);\n"
            " var sldMsg = \"CH|\" + chNum + \"|\" + slObj.value;\n"
            " websocket.send(sldMsg);\n"
            "}\n";
    JScode += "</script>\n";

    QString CSScode = "<style>\n"
            "html { height: 100%; background-color: #111; }\n"
            "body {\n"
            " margin: 0px;\n"
            " background-image: linear-gradient(to bottom, #45484d 0%, #111 100%);\n"
            " background-image: -webkit-linear-gradient(top, #45484d 0%, #111 100%);\n"
            "}\n"

            ".styled-select select {\n"
            "   background: #aaa;\n"
            "   width: 250px;\n"
            "   height: 30px;\n"
            "   margin-left: 15px;\n"
            "   font-size: 16px;\n"
            "   line-height: 1;\n"
            "   border: 1px solid #bbb;\n"
            "   border-radius: 4px;\n"
            "   }"

            ".sdButton {\n"
            " display: inline-block;\n"
            " vertical-align: top;\n"
            " background: linear-gradient(to bottom, #F6F6F6 0%, #AAAAAA 100%);\n"
            " background: -ms-linear-gradient(top, #F6F6F6 0%, #AAAAAA 100%);\n"
            " background: -moz-linear-gradient(top, #F6F6F6 0%, #AAAAAA 100%);\n"
            " background: -o-linear-gradient(top, #F6F6F6 0%, #AAAAAA 100%);\n"
            " background: -webkit-gradient(linear, left top, left bottom, color-stop(0, #F6F6F6), color-stop(1, #AAAAAA));\n"
            " background: -webkit-linear-gradient(top, #F6F6F6 0%, #AAAAAA 100%);\n"
            " border-radius: 3px;\n"
            " border: 1px solid #808080;\n"
            " padding: 1px;\n"
            " height: 28px;\n"
            " width: 60px;\n"
            " text-align: center;\n"
            "}\n"

            ".sdButton:active { background: #868585; }\n"

            CONTROL_BAR_CSS
            BUTTON_BASE_CSS
            BUTTON_SPAN_CSS
            BUTTON_STATE_CSS
            BUTTON_BLUE_CSS
            SLIDER_CSS
            SWINFO_CSS
            "</style>\n";

    QString bodyHTML = "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>" + tr("Back") + "</span></a>\n"
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
                       "</div>\n";

    bodyHTML += "<div style=\"margin: 20px; font: bold 27px/1.2em 'Trebuchet MS',Arial, Helvetica; color: #fff;\">\n";
    bodyHTML += tr("Page") + "  <a class=\"sdButton\" href=\"javascript:previousPage();\">\n"
                "<img src=\"back.png\" width=27></img></a>\n";

    bodyHTML += "<div style=\"display: inline-block;\">";
    bodyHTML += "<div id=\"pageDiv\" style=\"vertical-align: middle; text-align: center; color: #000;"
                "width: 50px; background-color: #888; border-radius: 6px;\">" +
                QString::number(page) +  "</div></div>\n";

    bodyHTML += "<a class=\"sdButton\" href=\"javascript:nextPage();\">\n"
                "<img src=\"forward.png\" width=27></img></a>\n";

    bodyHTML += "<a class=\"sdButton\" href=\"javascript:resetUniverse();\">\n"
                "<img src=\"fileclose.png\" width=27></img></a>\n";

    bodyHTML += "<div style=\"display: inline-block; margin-left: 50px;\">" + tr("Universe") + "</div>\n"
                "<div class=\"styled-select\" style=\"display: inline-block;\">\n"
                "<select onchange=\"universeChanged(this.value);\">\n";

    QStringList uniList = doc->inputOutputMap()->universeNames();
    for (int i = 0; i < uniList.count(); i++)
    {
        bodyHTML += "<option value=\"" + QString::number(i) + "\">" + uniList.at(i) + "</option>\n";
    }
    bodyHTML += "</select></div>\n";
    bodyHTML += "</div>\n";

    bodyHTML += "<div id=\"slidersContainer\"></div>\n\n";

    QString str = HTML_HEADER + JScode + CSScode + "</head>\n<body>\n" + bodyHTML + "</body>\n</html>";

    return str;
}

QString WebAccessSimpleDesk::getChannelsMessage(Doc *doc, SimpleDesk *sd,
                                                quint32 universe, int startAddr, int chNumber)
{
    QString message;
    quint32 universeAddr = (universe << 9);
    qDebug () << "Uni addr:" << universeAddr;

    for (int i = startAddr; i < startAddr + chNumber; i++)
    {
        QString type = "";
        uchar value = sd->getAbsoluteChannelValue(universeAddr + i);
        Fixture* fxi = doc->fixture(doc->fixtureForAddress(universeAddr + i));
        if (fxi != NULL)
        {
            const QLCChannel *ch = fxi->channel(universeAddr + i - fxi->universeAddress());
            if (ch != NULL)
            {
                if (ch->group() == QLCChannel::Intensity)
                {
                    QString hexCol;
                    hexCol.sprintf("%06X", ch->colour());
                    type = QString("%1.#%2").arg(ch->group()).arg(hexCol);
                }
                else
                    type = QString::number(ch->group());
            }
        }

        message.append(QString("%1|%2|%3|").arg(i + 1).arg(value).arg(type));
    }
    // remove trailing separator
    message.truncate(message.length() - 1);

    qDebug() << "Message to send:" << message;
    return message;
}
