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
  // Prepare the message we're going to send
  /*
  const struct mg_request_info *request_info = mg_get_request_info(conn);
  char content[100];
  int content_length = snprintf(content, sizeof(content),
                                "Hello from QLC+! Remote port: %d",
                                request_info->remote_port);
  */
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

QString WebAccess::getVCHTML()
{
    QString str = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                  "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                  "<head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
                  "</head>\n<body>\n";

    VCFrame *mainFrame = m_vc->contents();
    QSize mfSize = mainFrame->size();
    str = str + "<div style=\"width: " + QString::number(mfSize.width()) +
            "px; height: " + QString::number(mfSize.height()) + "px; "
            "background-color: " + mainFrame->backgroundColor().name() + "; \">\n";

    QList<VCWidget *> wlist = m_vc->findChildren<VCWidget*>();
    foreach(VCWidget *widget, wlist)
    {
        switch (widget->type())
        {
            case VCWidget::ButtonWidget:
                str += getVCButtonHTML((VCButton *)widget);
            break;
            default:
            break;
        }
    }

    str += "</div>\n";
    str += "</body>\n</html>";
    return str;
}

QString WebAccess::getVCButtonHTML(VCButton *btn)
{
    QString str = "<div style=\"position: absolute; left: " + QString::number(btn->x()) +
            "px; top: " + QString::number(btn->y()) + "px; width: " + QString::number(btn->width()) +
            "px; height: " + QString::number(btn->height()) + "px; "
            "background-color: " + btn->backgroundColor().name() + "; border-radius: 3px; "
            "border: 2px solid #666666;\"></div>\n";
    return str;
}

WebAccess::WebAccess(VirtualConsole *vcInstance, QObject *parent) :
    QObject(parent)
  , m_vc(vcInstance)
{
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    // List of options. Last element must be NULL.
    const char *options[] = {"listening_ports", "8080", NULL};

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

