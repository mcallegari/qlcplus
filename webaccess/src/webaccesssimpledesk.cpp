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

    QString JScode = "<script type=\"text/javascript\" src=\"simpledesk.js\"></script>\n";
    JScode += "<script type=\"text/javascript\">\n";
    JScode += "var currentUniverse = " + QString::number(uni) + ";\n";
    JScode += "var currentPage = " + QString::number(page) + ";\n";
    JScode += "var channelsPerPage = " + QString::number(sd->getSlidersNumber()) + ";\n";
    JScode += "</script>\n";

    QString CSScode = "<link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"common.css\">\n";
    CSScode += "<link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"simpledesk.css\">\n";

    QString bodyHTML = "<div class=\"controlBar\">\n"
                       "<a class=\"button button-blue\" href=\"/\"><span>" + tr("Back") + "</span></a>\n"
                       "<a class=\"button button-blue\" href=\"/keypad.html\"><span>DMX Keypad</span></a>\n"
                       "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
                       "</div>\n";

    bodyHTML += "<div style=\"margin: 20px; font: bold 27px/1.2em 'Trebuchet MS',Arial, Helvetica; color: #fff;\">\n";
    bodyHTML += tr("Page") + "  <a class=\"sdButton\" href=\"javascript:previousPage();\">\n"
                "<img src=\"back.png\" title=\""+tr("Previous page")+"\" width=\"27\" ></a>\n";

    bodyHTML += "<div style=\"display: inline-block;\">";
    bodyHTML += "<div id=\"pageDiv\" style=\"vertical-align: middle; text-align: center; color: #000;"
                "width: 50px; background-color: #888; border-radius: 6px;\">" +
                QString::number(page) +  "</div></div>\n";

    bodyHTML += "<a class=\"sdButton\" href=\"javascript:nextPage();\">\n"
                "<img src=\"forward.png\" title=\""+tr("Next page")+"\"  width=\"27\"></a>\n";

    bodyHTML += "<a class=\"sdButton\" href=\"javascript:resetUniverse();\">\n"
                "<img src=\"fileclose.png\" title=\""+tr("Reset universe")+"\" width=\"27\"></a>\n";

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
                    type = QString("%1.#%2")
                            .arg(ch->group())
                            .arg(hexCol.asprintf("%06X", ch->colour()));
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
