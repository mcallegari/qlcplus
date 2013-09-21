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

#include "virtualconsole.h"
#include "qlcconfig.h"
#include "webaccess.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "mongoose.h"
#include "doc.h"

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
  m_buttonFound = false;
  m_frameFound = false;
  m_labelFound = false;
  m_cueListFound = false;
  m_sliderFound = false;
  m_knobFound = false;
  m_xyPadFound = false;
  m_speedDialFound = false;

  const struct mg_request_info *ri = mg_get_request_info(conn);
  qDebug() << Q_FUNC_INFO << ri->request_method << ri->uri;

  if (is_websocket_request(conn))
  {
    handle_websocket_request(conn);
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
            "Content-Length: %d\r\n"        // Always set Content-Length
            "\r\n"
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
            if (m_doc->mode() == Doc::Design)
                m_doc->setMode(Doc::Operate);
            else
                m_doc->setMode(Doc::Design);
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
                if (value == 0)
                    button->releaseFunction();
                else
                    button->pressFunction();
            }
            default:
            break;
        }
    }

    return 1;
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
        if (widget->parentWidget() != frame)
            continue;

        switch (widget->type())
        {
            case VCWidget::FrameWidget:
            case VCWidget::SoloFrameWidget:
            {
                str += getVCFrameHTML((VCFrame *)widget);
            }
            break;
            case VCWidget::ButtonWidget:
            {
                VCButton *button = qobject_cast<VCButton*>(widget);
                str += getVCButtonHTML(button);
            }
            break;
            case VCWidget::SliderWidget:
            {
                VCSlider *slider = qobject_cast<VCSlider*>(widget);
                str += getVCSliderHTML(slider);
            }
            break;
            default:
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
            "function sendWSmessage(msg) {\n"
            " websocket.send(msg);\n"
            "};\n\n"

            "function sendCMD(cmd)\n"
            "{\n"
            " sendWSmessage(\"QLC+CMD|\" + cmd);\n"
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
            ".controlBar {\n"
            " width: 100%;\n"
            " height: 40px;\n"
            " background: linear-gradient(to bottom, #B2D360 0%, #4B9002 100%);\n"
            " background: -ms-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " background: -moz-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " background: -o-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " background: -webkit-gradient(linear, left top, left bottom, color-stop(0, #B2D360), color-stop(1, #4B9002));\n"
            " background: -webkit-linear-gradient(top, #B2D360 0%, #4B9002 100%);\n"
            " font:bold 26px/1.2em sans-serif;\n"
            " color: #ffffff;\n"
            " border-spacing:5px 0px;\n"
            "}\n"
            ".cmdButton a {\n"
            " height: 36px;\n"
            " display: table-cell;\n"
            " border: 2px solid #0E172E;\n"
            " border-radius: 5px;\n"
            " background-color: #2E4B95;\n"
            " text-align:center;\n"
            " vertical-align: middle;\n"
            "}\n"
            ".cmdButton a:hover {\n"
            " background-color: #3D64C7;\n"
            "}\n"
            ".swInfo {\n"
            " position: absolute;\n"
            " right: 0;\n"
            " top: 0;\n"
            " font-size: 20px;\n"
            "};\n"
            "</style>\n";

    VCFrame *mainFrame = m_vc->contents();
    QSize mfSize = mainFrame->size();
    QString widgetsHTML = "<div class=\"controlBar\">\n"
            "<div class=\"cmdButton\"><a onclick=\"sendCMD('opMode');\">Operate mode</a></div>\n"
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

QString WebAccess::getVCFrameHTML(VCFrame *frame)
{
    QColor border(90, 90, 90);

    if (frame->type() == VCWidget::SoloFrameWidget)
        border = QColor(255, 0, 0);

    QString str = "<div style=\"position: absolute; left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " +
           QString::number(frame->width()) +
          "px; height: " + QString::number(frame->height()) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border-radius: 3px;\n"
          "border: 1px solid " + border.name() + ";\">\n";

    str += getChildrenHTML(frame);
    str += "</div>\n";

    return str;
}

QString WebAccess::getButtonJS()
{
    if (m_buttonFound == true)
        return QString();

    QString str = "function buttonClick(id) {\n"
                " var obj = document.getElementById(id);\n"
                " if (obj.value == \"0\" || obj.value == undefined) {\n"
                "  obj.value = \"255\";\n"
                "  obj.style.border = \"3px solid #00E600\";\n"
                " }\n"
                " else {\n"
                "  obj.value = \"0\";\n"
                "  obj.style.border = \"3px solid #A0A0A0\";\n"
                " }\n"
                " var btnMsg = id + \"|\" + obj.value;\n"
                " sendWSmessage(btnMsg);\n"
                "};\n";
    return str;
}

QString WebAccess::getButtonCSS()
{
    if (m_buttonFound == true)
        return QString();

    QString str = "<style>\n"
            ".vcbutton-wrapper {\n"
            "position: absolute;\n"
            "}\n\n"
            ".vcbutton {\n"
            "display: table-cell;\n"
            "border: 3px solid #A0A0A0;\n"
            "border-radius: 3px;\n"
            "font-family: arial, verdana, sans-serif;\n"
            "text-align:center;\n"
            "vertical-align: middle;\n"
            "}\n"
            "</style>\n";

    m_buttonFound = true;
    return str;
}

QString WebAccess::getVCButtonHTML(VCButton *btn)
{
    m_JScode += getButtonJS();
    m_CSScode += getButtonCSS();
    QString str = "<div class=\"vcbutton-wrapper\" style=\""
            "left: " + QString::number(btn->x()) + "px; "
            "top: " + QString::number(btn->y()) + "px;\">\n";
    str +=  "<div class=\"vcbutton\" id=\"" + QString::number(btn->id()) + "\" "
            "onclick=\"buttonClick(" + QString::number(btn->id()) + ");\" "
            "style=\""
            "width: " + QString::number(btn->width()) + "px; "
            "height: " + QString::number(btn->height()) + "px; "
            "color: " + btn->foregroundColor().name() + "; "
            "background-color: " + btn->backgroundColor().name() + "\">" +
            btn->caption() + "</div>\n</div>\n";
    return str;
}

QString WebAccess::getSliderCSS()
{
    if (m_sliderFound == true)
        return QString();

    QString str = "<style>\n"
            ".vcslider {\n"
            "position: absolute;\n"
            "border: 1px solid #777777;\n"
            "border-radius: 3px;\n"
            "}\n"

            "input[type=\"range\"].vVertical {\n"
            "-webkit-appearance: none;\n"
            "height: 4px;\n"
            "border: 1px solid #8E8A86;\n"
            "background-color: #888888;\n"
            "-webkit-transform:rotate(270deg);\n"
            "-webkit-transform-origin: 0% 50%;\n"
            "-moz-transform:rotate(270deg);\n"
            "-o-transform:rotate(270deg);\n"
            "-ms-transform:rotate(270deg);\n"
            "-ms-transform-origin:0% 50%;\n"
            "transform:rotate(270deg);\n"
            "transform-origin:0% 50%;\n"
            "}\n"

            "input[type=\"range\"]::-webkit-slider-thumb {\n"
            "-webkit-appearance: none;\n"
            "background-color: #999999;\n"
            "border-radius: 4px;\n"
            "border: 1px solid #5c5c5c;\n"
            "width: 20px;\n"
            "height: 36px;\n"
            "}\n"
            "</style>\n";
    m_sliderFound = true;
    return str;
}

QString WebAccess::getVCSliderHTML(VCSlider *slider)
{
    m_CSScode += getSliderCSS();
    QString str = "<div class=\"vcslider\" style=\""
            "left: " + QString::number(slider->x()) + "px; "
            "top: " + QString::number(slider->y()) + "px; "
            "width: " + QString::number(slider->width()) + "px; "
            "height: " + QString::number(slider->height()) + "px; "
            "background-color: " + slider->backgroundColor().name() + ";\">\n";

    str +=  "<input type=\"range\" class=\"vVertical\" style=\""
            "width: " + QString::number(slider->height()) + "px; "
            "margin-top: " + QString::number(slider->height()) + "px; "
            "margin-left: " + QString::number(slider->width() / 2) + "px;\" "
            "min=\"0\" max=\"255\" step=\"1\" value=\"" +
            QString::number(slider->sliderValue()) + "\" />\n</div>\n";
    return str;
}

WebAccess::WebAccess(Doc *doc, VirtualConsole *vcInstance, QObject *parent) :
    QObject(parent)
  , m_doc(doc)
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

