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

#include "webaccess.h"
#include "virtualconsole.h"
#include "vcbutton.h"
#include "vcframe.h"

WebAccess* s_instance = NULL;

static int begin_request_handler(struct mg_connection *conn)
{
    return s_instance->beginRequestHandler(conn);
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

  // Prepare the message we're going to send
  QString content = getVCHTML();
  int content_length = content.length();

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"        // Always set Content-Length
            "\r\n"
            "%s",
            content_length, content.toAscii().data());

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return 1;
}

QString WebAccess::getButtonStyle()
{
    if (m_buttonFound == true)
        return QString();

    QString str = "<style>\n"
            ".vcbutton-wrapper {\n"
            "position: absolute;\n"
            "}\n\n"
            ".vcbutton {\n"
            "display: table-cell;\n"
            "border: 2px solid #666666;\n"
            "border-radius: 3px;\n"
            "font-family: arial, verdana, sans-serif;\n"
            "text-align:center;\n"
            "vertical-align: middle;\n"
            "}\n"
            "</style>\n";

    m_buttonFound = true;
    return str;
}

QString WebAccess::getChildrenHTML(VCWidget *frame)
{
    if (frame == NULL)
        return QString();

    QString str;
    const QObjectList chList = frame->children();

    qDebug () << "getChildrenHTML: found " << chList.count() << " children";

    foreach (QObject *obj, chList)
    {
        VCWidget *widget = (VCWidget *)obj;
        switch (widget->type())
        {
            case VCWidget::FrameWidget:
            case VCWidget::SoloFrameWidget:
                str += getVCFrameHTML((VCFrame *)widget);
            break;
            case VCWidget::ButtonWidget:
            {
                VCButton *button = qobject_cast<VCButton*>(widget);
                str += getVCButtonHTML(button);
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

    VCFrame *mainFrame = m_vc->contents();
    QSize mfSize = mainFrame->size();
    QString widgetsHTML = "<div style=\"width: " + QString::number(mfSize.width()) +
            "px; height: " + QString::number(mfSize.height()) + "px; "
            "background-color: " + mainFrame->backgroundColor().name() + "; \">\n";

    widgetsHTML += getChildrenHTML(mainFrame);
    widgetsHTML += "</div>\n";

    QString str = mainHTML + m_CSScode + m_JScode + "</head>\n<body>\n" + widgetsHTML + "</body>\n</html>";
    return str;
}

QString WebAccess::getVCFrameHTML(VCFrame *frame)
{
    QColor border(0, 0, 0);

    if (frame->type() == VCWidget::SoloFrameWidget)
        border = QColor(255, 0, 0);

    QString str = "<div style=\"position: absolute; left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " +
           QString::number(frame->width()) +
          "px; height: " + QString::number(frame->height()) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border: 1px solid " + border.name() + ";\">\n";

    str += getChildrenHTML(frame);

    str += "</div>\n";

    return str;
}

QString WebAccess::getVCButtonHTML(VCButton *btn)
{
    m_CSScode += getButtonStyle();
    QString str = "<div class=\"vcbutton-wrapper\" style=\"left: " + QString::number(btn->x()) +
            "px; top: " + QString::number(btn->y()) + "px;\">\n";
    str +=  "<div class=\"vcbutton\" style=\"width: " + QString::number(btn->width()) +
            "px; height: " + QString::number(btn->height()) + "px; color: " + btn->foregroundColor().name() + "; "
            "background-color: " + btn->backgroundColor().name() + "\">" + btn->caption() + "</div>\n</div>\n";
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

    // Start the web server.
    m_ctx = mg_start(&m_callbacks, NULL, options);
}

WebAccess::~WebAccess()
{
    mg_stop(m_ctx);
}

