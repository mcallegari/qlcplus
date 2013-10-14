/*
  Q Light Controller Plus
  webaccess.cpp

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QDebug>
#include <QDomDocument>

#include "vcaudiotriggers.h"
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "qlcconfig.h"
#include "webaccess.h"
#include "vccuelist.h"
#include "mongoose.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "function.h"
#include "vclabel.h"
#include "vcframe.h"
#include "chaser.h"

#define POST_DATA_SIZE 1024

WebAccess* s_instance = NULL;

static int begin_request_handler(struct mg_connection *conn)
{
    return s_instance->beginRequestHandler(conn);
}

static void websocket_ready_handler(struct mg_connection *conn)
{
    s_instance->websocketReadyHandler(conn);
}

static int websocket_data_handler(struct mg_connection *conn, int flags,
                                  char *data, size_t data_len)
{
    return s_instance->websocketDataHandler(conn, flags, data, data_len);
}

// This function will be called by mongoose on every new request.
int WebAccess::beginRequestHandler(mg_connection *conn)
{
  m_genericFound = false;
  m_buttonFound = false;
  m_frameFound = false;
  m_soloFrameFound = false;
  m_labelFound = false;
  m_cueListFound = false;
  m_sliderFound = false;
  m_knobFound = false;
  m_xyPadFound = false;
  m_speedDialFound = false;
  m_audioTriggersFound = false;

  const struct mg_request_info *ri = mg_get_request_info(conn);
  qDebug() << Q_FUNC_INFO << ri->request_method << ri->uri;

  if (QString(ri->uri) == "/qlcplusWS")
      return 0;

  if (QString(ri->uri) == "/loadProject")
  {
      char post_data[POST_DATA_SIZE];
      QString projectXML = "";
      bool done = false;

      while(!done)
      {
          int read = mg_read(conn, post_data, sizeof(post_data));

          qDebug() << "POST: received: " << read << "bytes";

          QString recv(post_data);

          if (read < POST_DATA_SIZE)
          {
              recv.truncate(read);
              done = true;
          }
          projectXML += recv;
      }

      projectXML.remove(0, projectXML.indexOf("\n\r") + 2);
      projectXML.truncate(projectXML.indexOf("\n\r"));
      qDebug() << "Project XML:\n\n" << projectXML << "\n\n";

      QByteArray postReply =
              QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
              "<script type=\"text/javascript\">\n"
              " window.location = \"/\"\n"
              "</script></head></html>").toAscii();
      int post_size = postReply.length();
      mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n\r\n"
                "%s",
                post_size, postReply.data());

      emit loadProject(projectXML);

      return 1;
  }

  if (QString(ri->uri) != "/")
      return 1;

  // Prepare the message we're going to send
  QString content = getVCHTML();
  int content_length = content.length();
  QByteArray contentArray = content.toAscii();

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            content_length, contentArray.data());

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return 1;
}

void WebAccess::websocketReadyHandler(mg_connection *conn)
{
    qDebug() << Q_FUNC_INFO;
    static const char *message = "server ready";
    mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, message, strlen(message));
}

int WebAccess::websocketDataHandler(mg_connection *conn, int flags, char *data, size_t data_len)
{
    Q_UNUSED(conn)
    Q_UNUSED(flags)

    QString qData = QString(data);
    qData.truncate((int)data_len);
    qDebug() << "[websocketDataHandler]" << qData;

    QStringList cmdList = qData.split("|");
    if (cmdList.isEmpty())
        return 1;

    if(cmdList[0] == "QLC+CMD")
    {
        if (cmdList.count() < 2)
            return 0;

        if(cmdList[1] == "opMode")
        {
            emit toggleDocMode();
        }

        return 1;
    }

    quint32 widgetID = cmdList[0].toUInt();
    VCWidget *widget = m_vc->widget(widgetID);
    uchar value = 0;
    if (cmdList.count() > 1)
        value = (uchar)cmdList[1].toInt();
    if (widget != NULL)
    {
        switch(widget->type())
        {
            case VCWidget::ButtonWidget:
            {
                VCButton *button = qobject_cast<VCButton*>(widget);
                button->pressFunction();
            }
            break;
            case VCWidget::SliderWidget:
            {
                VCSlider *slider = qobject_cast<VCSlider*>(widget);
                slider->setSliderValue(value);
            }
            break;
            case VCWidget::AudioTriggersWidget:
            {
                VCAudioTriggers *triggers = qobject_cast<VCAudioTriggers*>(widget);
                triggers->slotEnableButtonToggled(value ? true : false);
            }
            break;
            case VCWidget::CueListWidget:
            {
                VCCueList *cue = qobject_cast<VCCueList*>(widget);
                if (cmdList[1] == "PLAY")
                    cue->slotPlayback();
                else if (cmdList[1] == "PREV")
                    cue->slotPreviousCue();
                else if (cmdList[1] == "NEXT")
                    cue->slotNextCue();
            }
            break;
            default:
            break;
        }
    }

    return 1;
}

QString WebAccess::getWidgetHTML(VCWidget *widget)
{
    if (m_genericFound == false)
    {
        m_CSScode += widget->getCSS();
        m_genericFound = true;
    }

    QString str = "<div class=\"vcwidget\" style=\""
            "left: " + QString::number(widget->x()) + "px; "
            "top: " + QString::number(widget->y()) + "px; "
            "width: " + QString::number(widget->width()) + "px; "
            "height: " + QString::number(widget->height()) + "px; "
            "background-color: " + widget->backgroundColor().name() + ";\">\n";

    str +=  tr("Widget not supported (yet) for web access") + "</div>\n";

    return str;
}

QString WebAccess::getFrameHTML(VCFrame *frame)
{
    QColor border(90, 90, 90);

    QString str = "<div class=\"vcframe\" style=\"left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " +
           QString::number(frame->width()) +
          "px; height: " + QString::number(frame->height()) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border-radius: 4px;\n"
          "border: 1px solid " + border.name() + ";\">\n";
    if (frame->isHeaderVisible())
    {
        if (m_frameFound == false)
        {
            m_CSScode += frame->getCSS();
            m_frameFound = true;
        }
        str += "<div class=\"vcframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + "\">" + frame->caption() + "</div>\n";
    }

    str += getChildrenHTML(frame);
    str += "</div>\n";

    return str;
}

QString WebAccess::getSoloFrameHTML(VCSoloFrame *frame)
{
    QColor border(255, 0, 0);

    QString str = "<div class=\"vcsoloframe\" style=\"left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " +
           QString::number(frame->width()) +
          "px; height: " + QString::number(frame->height()) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border-radius: 4px;\n"
          "border: 1px solid " + border.name() + ";\">\n";
    if (frame->isHeaderVisible())
    {
        if (m_soloFrameFound == false)
        {
            m_CSScode += frame->getCSS();
            m_soloFrameFound = true;
        }
        str += "<div class=\"vcsoloframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + "\">" + frame->caption() + "</div>\n";
    }

    str += getChildrenHTML(frame);
    str += "</div>\n";

    return str;
}


QString WebAccess::getButtonHTML(VCButton *btn)
{
    if (m_buttonFound == false)
    {
        m_JScode += btn->getJS();
        m_CSScode += btn->getCSS();
        m_buttonFound = true;
    }

    QString str = "<div class=\"vcbutton-wrapper\" style=\""
            "left: " + QString::number(btn->x()) + "px; "
            "top: " + QString::number(btn->y()) + "px;\">\n";
    str +=  "<a class=\"vcbutton\" id=\"" + QString::number(btn->id()) + "\" "
            "href=\"javascript:buttonClick(" + QString::number(btn->id()) + ");\" "
            "style=\""
            "width: " + QString::number(btn->width()) + "px; "
            "height: " + QString::number(btn->height()) + "px; "
            "color: " + btn->foregroundColor().name() + "; "
            "background-color: " + btn->backgroundColor().name() + "\">" +
            btn->caption() + "</a>\n</div>\n";
    return str;
}

QString WebAccess::getSliderHTML(VCSlider *slider)
{
    if (m_sliderFound == false)
    {
        m_JScode += slider->getJS();
        m_CSScode += slider->getCSS();
        m_sliderFound = true;
    }

    QString slID = QString::number(slider->id());

    QString str = "<div class=\"vcslider\" style=\""
            "left: " + QString::number(slider->x()) + "px; "
            "top: " + QString::number(slider->y()) + "px; "
            "width: " + QString::number(slider->width()) + "px; "
            "height: " + QString::number(slider->height()) + "px; "
            "background-color: " + slider->backgroundColor().name() + ";\">\n";

    str += "<div id=\"slv" + slID + "\" "
            "class=\"vcslLabel\" style=\"top:0px;\">" +
            QString::number(slider->sliderValue()) + "</div>\n";

    str +=  "<input type=\"range\" class=\"vVertical\" "
            "id=\"" + slID + "\" "
            "onchange=\"slVchange(" + slID + ");\" style=\""
            "width: " + QString::number(slider->height() - 50) + "px; "
            "margin-top: " + QString::number(slider->height() - 50) + "px; "
            "margin-left: " + QString::number(slider->width() / 2) + "px;\" "
            "min=\"0\" max=\"255\" step=\"1\" value=\"" +
            QString::number(slider->sliderValue()) + "\" />\n";

    str += "<div id=\"sln" + slID + "\" "
            "class=\"vcslLabel\" style=\"bottom:0px;\">" +
            slider->caption() + "</div>\n"
            "</div>\n";
    return str;
}

QString WebAccess::getLabelHTML(VCLabel *label)
{
    if (m_labelFound == false)
    {
        m_CSScode += label->getCSS();
        m_labelFound = true;
    }

    QString str = "<div class=\"vclabel-wrapper\" style=\""
            "left: " + QString::number(label->x()) + "px; "
            "top: " + QString::number(label->y()) + "px;\">\n";
    str +=  "<div class=\"vclabel\" style=\""
            "width: " + QString::number(label->width()) + "px; "
            "height: " + QString::number(label->height()) + "px; "
            "color: " + label->foregroundColor().name() + "; "
            "background-color: " + label->backgroundColor().name() + "\">" +
            label->caption() + "</div>\n</div>\n";

    return str;
}

QString WebAccess::getAudioTriggersHTML(VCAudioTriggers *triggers)
{
    if (m_audioTriggersFound == false)
    {
        m_CSScode += triggers->getCSS();
        m_JScode += triggers->getJS();
        m_audioTriggersFound = true;
    }

    QString str = "<div class=\"vcaudiotriggers\" style=\"left: " + QString::number(triggers->x()) +
          "px; top: " + QString::number(triggers->y()) + "px; width: " +
           QString::number(triggers->width()) +
          "px; height: " + QString::number(triggers->height()) + "px; "
          "background-color: " + triggers->backgroundColor().name() + ";\">\n";

    str += "<div class=\"vcaudioHeader\" style=\"color:" +
            triggers->foregroundColor().name() + "\">" + triggers->caption() + "</div>\n";

    str += "<div class=\"vcatbutton-wrapper\">\n";
    str += "<a  class=\"vcatbutton\" id=\"" + QString::number(triggers->id()) + "\" "
            "href=\"javascript:atButtonClick(" + QString::number(triggers->id()) + ");\" "
            "style=\""
            "width: " + QString::number(triggers->width() - 2) + "px; "
            "height: " + QString::number(triggers->height() - 42) + "px;\">"
            + tr("Enable") + "</a>\n";

    str += "</div></div>\n";

    return str;
}

QString WebAccess::getCueListHTML(VCCueList *cue)
{
    if (m_cueListFound == false)
    {
        m_CSScode += cue->getCSS();
        m_JScode += cue->getJS();
        m_cueListFound = true;
    }

    QString str = "<div class=\"vccuelist\" style=\"left: " + QString::number(cue->x()) +
            "px; top: " + QString::number(cue->y()) + "px; width: " +
             QString::number(cue->width()) +
            "px; height: " + QString::number(cue->height()) + "px; "
            "background-color: " + cue->backgroundColor().name() + ";\">\n";

    str += "<div style=\"width: 100%; height: " + QString::number(cue->height() - 32) + "px; overflow: scroll;\" >\n";
    str += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    str += "<tr><th>#</th><th>Name</th><th>Fade In</th><th>Fade Out</th><th>Duration</th><th>Notes</th></tr>\n";
    Chaser *chaser = cue->chaser();
    Doc *doc = m_vc->getDoc();
    if (chaser != NULL)
    {
        for (int i = 0; i < chaser->stepsCount(); i++)
        {
            str += "<tr onmouseover=\"this.style.backgroundColor='#92BDDF';\" "
                    "onmouseout=\"this.style.backgroundColor='#ffffff';\">\n";
            ChaserStep step = chaser->stepAt(i);
            str += "<td>" + QString::number(i + 1) + "</td>";
            Function* function = doc->function(step.fid);
            if (function != NULL)
            {
                str += "<td>" + function->name() + "</td>";

                switch (chaser->fadeInMode())
                {
                    case Chaser::Common:
                        str += "<td>" + Function::speedToString(chaser->fadeInSpeed()) + "</td>";
                        break;
                    case Chaser::PerStep:
                        str += "<td>" + Function::speedToString(step.fadeIn) + "</td>";
                        break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                //if (step.hold != 0)
                //    str +=  "<td>" + Function::speedToString(step.hold) + "</td>";
                //else str += "<td></td>";

                switch (chaser->fadeOutMode())
                {
                    case Chaser::Common:
                        str += "<td>" + Function::speedToString(chaser->fadeOutSpeed()) + "</td>";
                        break;
                    case Chaser::PerStep:
                        str += "<td>" + Function::speedToString(step.fadeOut) + "</td>";
                        break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                switch (chaser->durationMode())
                {
                    case Chaser::Common:
                        str += "<td>" + Function::speedToString(chaser->duration()) + "</td>";
                        break;
                    case Chaser::PerStep:
                        str += "<td>" + Function::speedToString(step.duration) + "</td>";
                        break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                str += "<td>" + step.note + "</td>\n";
            }
            str += "</td>\n";
        }
    }
    str += "</table>\n";
    str += "</div>\n";

    str += "<a class=\"button button-blue\" style=\"height: 29px; font-size: 24px;\" "
            "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'PLAY');\">\n"
            "<span id=\"" + QString::number(cue->id()) + "\">Play</span></a>\n";
    str += "<a class=\"button button-blue\" style=\"height: 29px; font-size: 24px;\" "
            "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'PREV');\">\n"
            "<span>Previous</span></a>\n";
    str += "<a class=\"button button-blue\" style=\"height: 29px; font-size: 24px;\" "
            "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'NEXT');\">\n"
            "<span>Next</span></a>\n";
    str += "</div>\n";

    return str;
}

QString WebAccess::getChildrenHTML(VCWidget *frame)
{
    if (frame == NULL)
        return QString();

    QString str;

    QList<VCWidget *> chList = frame->findChildren<VCWidget*>();

    qDebug () << "getChildrenHTML: found " << chList.count() << " children";

    foreach (VCWidget *widget, chList)
    {
        if (widget->parentWidget() != frame || widget->isVisible() == false)
            continue;

        switch (widget->type())
        {
            case VCWidget::FrameWidget:
                str += getFrameHTML((VCFrame *)widget);
            break;
            case VCWidget::SoloFrameWidget:
                str += getSoloFrameHTML((VCSoloFrame *)widget);
            break;
            case VCWidget::ButtonWidget:
                str += getButtonHTML((VCButton *)widget);
            break;
            case VCWidget::SliderWidget:
                str += getSliderHTML((VCSlider *)widget);
            break;
            case VCWidget::LabelWidget:
                str += getLabelHTML((VCLabel *)widget);
            break;
            case VCWidget::AudioTriggersWidget:
                str += getAudioTriggersHTML((VCAudioTriggers *)widget);
            break;
            case VCWidget::CueListWidget:
                str += getCueListHTML((VCCueList *)widget);
            break;
            default:
                str += getWidgetHTML(widget);
            break;
        }
    }

    return str;
}

QString WebAccess::getVCHTML()
{
    QString mainHTML = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                  "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                  "<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n";

    m_JScode = "<script language=\"javascript\" type=\"text/javascript\">\n"
            "var websocket;\n"
            "function sendCMD(cmd)\n"
            "{\n"
            " websocket.send(\"QLC+CMD|\" + cmd);\n"
            "};\n\n"

            "window.onload = function() {\n"
            " var url = 'ws://' + window.location.host + '/qlcplusWS';\n"
            " websocket = new WebSocket(url);\n"

            " websocket.onopen = function(ev) {\n"
            "  //alert(\"Websocket open!\");\n"
            " };\n\n"

            " websocket.onclose = function(ev) {\n"
            "  //alert(\"Websocket close!\");\n"
            " };\n\n"

            " websocket.onerror = function(ev) {\n"
            "  alert(\"Websocket error!\");\n"
            " };\n"
            "};\n";

    m_CSScode = "<style>\n"
            "body { margin: 0px; }\n"

            "form {\n"
            " position: absolute;\n"
            " top: -100px;\n"
            " visibility: hidden;\n"
            "}\n\n"

            ".controlBar {\n"
            " width: 100%;\n"
            " height: 40px;\n"
            " background: linear-gradient(to bottom, #B2D360 0%, #4B9002 100%);\n"
            " background: -ms-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " background: -moz-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " background: -o-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " background: -webkit-gradient(linear, left top, left bottom, color-stop(0, #B2D360), color-stop(1, #4B9002));\n"
            " background: -webkit-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " font:bold 24px/1.2em sans-serif;\n"
            " color: #ffffff;\n"
            "}\n\n"

            ".button\n"
            "{\n"
            " height: 36px;\n"
            " margin-left: 5px;"
            " text-decoration: none;\n"
            " font: bold 27px/1.2em 'Trebuchet MS',Arial, Helvetica;\n"
            " display: inline-block;\n"
            " text-align: center;\n"
            " color: #fff;\n"
            " border: 1px solid #9c9c9c;\n"
            " border: 1px solid rgba(0, 0, 0, 0.3);\n"
            " text-shadow: 0 1px 0 rgba(0,0,0,0.4);\n"
            " box-shadow: 0 0 .05em rgba(0,0,0,0.4);\n"
            " -moz-box-shadow: 0 0 .05em rgba(0,0,0,0.4);\n"
            " -webkit-box-shadow: 0 0 .05em rgba(0,0,0,0.4);\n"
            "}\n\n"

            ".button, .button span  {\n"
            " -moz-border-radius: .3em;\n"
            " border-radius: .3em;\n"
            "}\n\n"

            ".button span {\n"
            " border-top: 1px solid #fff;\n"
            " border-top: 1px solid rgba(255, 255, 255, 0.5);\n"
            " display: block;\n"
            " padding: 0 10px 0 10px;\n"
            " background-image: -webkit-gradient(linear, 0 0, 100% 100%, color-stop(.25, rgba(0, 0, 0, 0.05)), color-stop(.25, transparent), to(transparent)),\n"
            " background-image: -moz-linear-gradient(45deg, rgba(0, 0, 0, 0.05) 25%, transparent 25%, transparent),\n"
            "}\n\n"

            ".button:hover {\n"
            " box-shadow: 0 0 .1em rgba(0,0,0,0.4);\n"
            " -moz-box-shadow: 0 0 .1em rgba(0,0,0,0.4);\n"
            " -webkit-box-shadow: 0 0 .1em rgba(0,0,0,0.4);\n"
            "}\n\n"

            ".button:active {\n"
            " position: relative;\n"
            " top: 1px;\n"
            "}\n\n"

            ".button-blue {\n"
            " background: #4477a1;\n"
            " background: -webkit-gradient(linear, left top, left bottom, from(#81a8cb), to(#4477a1) );\n"
            " background: -moz-linear-gradient(-90deg, #81a8cb, #4477a1);\n"
            "}\n\n"

            ".button-blue:hover {\n"
            " background: #81a8cb;\n"
            " background: -webkit-gradient(linear, left top, left bottom, from(#4477a1), to(#81a8cb) );\n"
            " background: -moz-linear-gradient(-90deg, #4477a1, #81a8cb);\n"
            "}\n\n"

            ".button-blue:active { background: #4477a1; }\n\n"

            ".swInfo {\n"
            " position: absolute;\n"
            " right: 0;\n"
            " top: 0;\n"
            " font-size: 20px;\n"
            "}\n"
            "</style>\n";

    VCFrame *mainFrame = m_vc->contents();
    QSize mfSize = mainFrame->size();
    QString widgetsHTML =
            "<form action=\"/loadProject\" method=\"POST\" enctype=\"multipart/form-data\">\n"
            "<input id=\"loadTrigger\" type=\"file\" "
            "onchange=\"document.getElementById('submitTrigger').click();\" name=\"qlcprj\" />\n"
            "<input id=\"submitTrigger\" type=\"submit\"/></form>"

            "<div class=\"controlBar\">\n"
            "<a class=\"button button-blue\" href=\"javascript:document.getElementById('loadTrigger').click();\">\n"
            "<span>Load project</span></a>\n"

            //"<a class=\"button button-blue\" href=\"javascript:sendCMD('opMode');\"><span>Operate mode</span></a>\n"

            "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
            "</div>\n"
            "<div style=\"position: relative; "
            "width: " + QString::number(mfSize.width()) +
            "px; height: " + QString::number(mfSize.height()) + "px; "
            "background-color: " + mainFrame->backgroundColor().name() + "; \" />\n";

    widgetsHTML += getChildrenHTML(mainFrame);

    m_JScode += "\n</script>\n";

    QString str = mainHTML + m_JScode + m_CSScode + "</head>\n<body>\n" + widgetsHTML + "</body>\n</html>";
    return str;
}

WebAccess::WebAccess(VirtualConsole *vcInstance, QObject *parent) :
    QObject(parent)
  , m_vc(vcInstance)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    // List of options. Last element must be NULL.
    const char *options[] = {"listening_ports", "9999", NULL};

    // Prepare callbacks structure. We have only one callback, the rest are NULL.
    memset(&m_callbacks, 0, sizeof(m_callbacks));
    m_callbacks.begin_request = begin_request_handler;
    m_callbacks.websocket_ready = websocket_ready_handler;
    m_callbacks.websocket_data = websocket_data_handler;

    // Start the web server.
    m_ctx = mg_start(&m_callbacks, NULL, options);
}

WebAccess::~WebAccess()
{
    mg_stop(m_ctx);
}

