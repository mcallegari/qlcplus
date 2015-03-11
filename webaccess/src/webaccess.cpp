/*
  Q Light Controller Plus
  webaccess.cpp

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

#include <QDebug>
#include <QProcess>
#include <QSettings>

#include "webaccess.h"

#include "webaccessconfiguration.h"
#include "webaccesssimpledesk.h"
#include "webaccessnetwork.h"
#include "vcaudiotriggers.h"
#include "virtualconsole.h"
#include "commonjscss.h"
#include "vcsoloframe.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "simpledesk.h"
#include "qlcconfig.h"
#include "webaccess.h"
#include "vccuelist.h"
#include "mongoose.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "function.h"
#include "vclabel.h"
#include "vcframe.h"
#include "qlcfile.h"
#include "chaser.h"
#include "doc.h"

#include "audiocapture.h"
#include "audiorenderer.h"

#define AUTOSTART_PROJECT_NAME "autostart.qxw"

WebAccess* s_instance = NULL;

static int event_handler(struct mg_connection *conn, enum mg_event ev)
{
    if (ev == MG_REQUEST)
    {
        if (conn->is_websocket)
            return s_instance->websocketDataHandler(conn);

        return s_instance->beginRequestHandler(conn);
    }
    else if (ev == MG_WS_HANDSHAKE)
    {
        if (conn->is_websocket)
        {
            s_instance->websocketDataHandler(conn);
            return MG_FALSE;
        }
    }
    else if (ev == MG_AUTH)
    {
        return MG_TRUE;
    }
    else if (ev == MG_CLOSE)
    {
        return s_instance->closeHandler(conn);
    }

    return MG_FALSE;
}

WebAccess::WebAccess(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance, QObject *parent) :
    QThread(parent)
  , m_doc(doc)
  , m_vc(vcInstance)
  , m_sd(sdInstance)
  , m_server(NULL)
  , m_conn(NULL)
  , m_running(false)
  , m_pendingProjectLoaded(false)
{
    Q_ASSERT(s_instance == NULL);
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(m_vc != NULL);

    s_instance = this;

    m_server = mg_create_server(NULL, event_handler);
    mg_set_option(m_server, "listening_port", "9999");
    start();

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    m_netConfig = new WebAccessNetwork();
#endif

    connect(m_vc, SIGNAL(loaded()),
            this, SLOT(slotVCLoaded()));
}

WebAccess::~WebAccess()
{
    m_running = false;
    wait();
    mg_destroy_server(&m_server);
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    delete m_netConfig;
#endif
}

void WebAccess::run()
{
    m_running = true;
    while (isRunning())
    {
        mg_poll_server(m_server, 500);
    }

}

QString WebAccess::loadXMLPost(mg_connection *conn, QString &filename)
{
    const char *data;
    int data_len;
    char vname[1024], fname[1024];
    QString XMLdata = "";

    if (conn != NULL &&
        mg_parse_multipart(conn->content, conn->content_len,
                           vname, sizeof(vname),
                           fname, sizeof(fname),
                           &data, &data_len) > 0)
    {
        XMLdata = QString(data);
        XMLdata.truncate(data_len);
        filename = QString(fname);
        qDebug() << "Filename:" << filename;
    }

    return XMLdata;
}

bool WebAccess::sendFile(mg_connection *conn, QString filename, QString contentType)
{
    QFile resFile(filename);
    if (resFile.open(QIODevice::ReadOnly))
    {
        QByteArray resContent = resFile.readAll();
        qDebug() << "Resource file lenght:" << resContent.length();
        resFile.close();
        QString head = "HTTP/1.1 200 OK\r\n";
        head += "Content-Type: " + contentType + "\r\n";
        head += "Content-Length: %d\r\n\r\n";
        mg_printf(conn, head.toLatin1().data(),
                  resContent.length());
        mg_write(conn, resContent.data(), resContent.length());
        mg_write(conn, "\r\n", 2);
        return true;
    }
    else
        qDebug() << "Failed to open file:" << filename;

    return false;
}

// This function will be called by mongoose on every new request.
mg_result WebAccess::beginRequestHandler(mg_connection *conn)
{

  QString content;

  //const struct mg_request_info *ri = mg_get_request_info(conn);
  qDebug() << Q_FUNC_INFO << conn->request_method << conn->uri;

  if (QString(conn->uri) == "/qlcplusWS" || QString(conn->uri) == "/favicon.ico")
      return MG_FALSE;


  if (QString(conn->uri) == "/loadProject")
  {
      QString prjname;
      QString projectXML = loadXMLPost(conn, prjname);
      qDebug() << "Project XML:\n\n" << projectXML << "\n\n";

      QByteArray postReply =
              QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
              "<script type=\"text/javascript\">\n" PROJECT_LOADED_JS
              "</script></head><body style=\"background-color: #45484d;\">"
              "<div style=\"position: absolute; width: 100%; height: 30px; top: 50%; background-color: #888888;"
              "text-align: center; font:bold 24px/1.2em sans-serif;\">"
              + tr("Loading project...") +
              "</div></body></html>").toUtf8();
      int post_size = postReply.length();
      mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n\r\n"
                "%s",
                post_size, postReply.data());

      m_pendingProjectLoaded = false;

      emit loadProject(projectXML);

      return MG_TRUE;
  }
  else if (QString(conn->uri) == "/config")
  {
      content = WebAccessConfiguration::getHTML(m_doc);
  }
  else if (QString(conn->uri) == "/simpleDesk")
  {
      content = WebAccessSimpleDesk::getHTML(m_doc, m_sd);
  }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
  else if (QString(conn->uri) == "/system")
  {
      content = m_netConfig->getHTML();
  }
#endif
  else if (QString(conn->uri) == "/loadFixture")
  {
      QString fxName;
      QString fixtureXML = loadXMLPost(conn, fxName);
      qDebug() << "Fixture name:" << fxName;
      qDebug() << "Fixture XML:\n\n" << fixtureXML << "\n\n";

      m_doc->fixtureDefCache()->storeFixtureDef(fxName, fixtureXML);

      QByteArray postReply =
                    QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
                    "<script type=\"text/javascript\">\n"
                    " alert(\"" + tr("Fixture stored and loaded") + "\");"
                    " window.location = \"/config\"\n"
                    "</script></head></html>").toUtf8();
      int post_size = postReply.length();
      mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "Content-Length: %d\r\n\r\n"
                      "%s",
                      post_size, postReply.data());

      return MG_TRUE;
  }
  else if (QString(conn->uri).endsWith(".png"))
  {
      if (sendFile(conn, QString(":%1").arg(QString(conn->uri)), "image/png") == true)
          return MG_TRUE;
  }
  else if (QString(conn->uri).endsWith(".css"))
  {
      QString clUri = QString(conn->uri).mid(1);
      if (sendFile(conn, QString("%1%2%3").arg(QLCFile::systemDirectory(WEBFILESDIR).path())
                   .arg(QDir::separator()).arg(clUri), "text/css") == true)
          return MG_TRUE;
  }
  else if (QString(conn->uri).endsWith(".js"))
  {
      QString clUri = QString(conn->uri).mid(1);
      if (sendFile(conn, QString("%1%2%3").arg(QLCFile::systemDirectory(WEBFILESDIR).path())
                   .arg(QDir::separator()).arg(clUri), "text/javascript") == true)
          return MG_TRUE;
  }
  else if (QString(conn->uri) != "/")
      return MG_TRUE;
  else
      content = getVCHTML();

  // Prepare the message we're going to send
  QByteArray contentArray = content.toUtf8();

  //For UTF8 we need to know the amount of bytes, not number of characters.
  int content_length = contentArray.size();

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            content_length, contentArray.data());

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return MG_TRUE;
}

mg_result WebAccess::websocketDataHandler(mg_connection *conn)
{
    if (conn == NULL)
        return MG_TRUE;

    m_conn = conn; // store this to send VC loaded async event

    if (conn->content_len == 0)
        return MG_TRUE;

    QString qData = QString(conn->content);
    qData.truncate(conn->content_len);
    qDebug() << "[websocketDataHandler]" << qData;

    QStringList cmdList = qData.split("|");
    if (cmdList.isEmpty())
        return MG_TRUE;

    if(cmdList[0] == "QLC+CMD")
    {
        if (cmdList.count() < 2)
            return MG_FALSE;

        if(cmdList[1] == "opMode")
            emit toggleDocMode();

        return MG_TRUE;
    }
    else if (cmdList[0] == "QLC+IO")
    {
        if (cmdList.count() < 3)
            return MG_FALSE;

        int universe = cmdList[2].toInt();

        if (cmdList[1] == "INPUT")
        {
            m_doc->inputOutputMap()->setInputPatch(universe, cmdList[3], cmdList[4].toUInt());
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "OUTPUT")
        {
            m_doc->inputOutputMap()->setOutputPatch(universe, cmdList[3], cmdList[4].toUInt(), false);
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "FB")
        {
            m_doc->inputOutputMap()->setOutputPatch(universe, cmdList[3], cmdList[4].toUInt(), true);
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "PROFILE")
        {
            InputPatch *inPatch = m_doc->inputOutputMap()->inputPatch(universe);
            if (inPatch != NULL)
            {
                m_doc->inputOutputMap()->setInputPatch(universe, inPatch->pluginName(), inPatch->input(), cmdList[3]);
                m_doc->inputOutputMap()->saveDefaults();
            }
        }
        else if (cmdList[1] == "PASSTHROUGH")
        {
            quint32 uniIdx = cmdList[2].toUInt();
            if (cmdList[3] == "true")
                m_doc->inputOutputMap()->setUniversePassthrough(uniIdx, true);
            else
                m_doc->inputOutputMap()->setUniversePassthrough(uniIdx, false);
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "AUDIOIN")
        {
            QSettings settings;
            if (cmdList[2] == "__qlcplusdefault__")
                settings.remove(SETTINGS_AUDIO_INPUT_DEVICE);
            else
            {
                settings.setValue(SETTINGS_AUDIO_INPUT_DEVICE, cmdList[2]);
                m_doc->destroyAudioCapture();
            }
        }
        else if (cmdList[1] == "AUDIOOUT")
        {
            QSettings settings;
            if (cmdList[2] == "__qlcplusdefault__")
                settings.remove(SETTINGS_AUDIO_OUTPUT_DEVICE);
            else
                settings.setValue(SETTINGS_AUDIO_OUTPUT_DEVICE, cmdList[2]);
        }
        else
            qDebug() << "[webaccess] Command" << cmdList[1] << "not supported !";

        return MG_TRUE;
    }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    else if(cmdList[0] == "QLC+SYS")
    {
        if (cmdList.at(1) == "NETWORK")
        {
            if (m_netConfig->updateNetworkFile(cmdList) == true)
            {
                QString wsMessage = QString("ALERT|" + tr("Network configuration changed. Reboot to apply the changes."));
                mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toUtf8().data(), wsMessage.length());
                return MG_TRUE;
            }
            else
                qDebug() << "[webaccess] Error writing network configuration file !";

            return MG_TRUE;
        }
        else if (cmdList.at(1) == "AUTOSTART")
        {
            if (cmdList.count() < 3)
                return MG_FALSE;

            QString asName = QString("%1/%2/%3").arg(getenv("HOME")).arg(USERQLCPLUSDIR).arg(AUTOSTART_PROJECT_NAME);
            if (cmdList.at(2) == "none")
                QFile::remove(asName);
            else
                emit storeAutostartProject(asName);
            QString wsMessage = QString("ALERT|" + tr("Autostart configuration changed"));
            mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toUtf8().data(), wsMessage.length());
            return MG_TRUE;
        }
        else if (cmdList.at(1) == "REBOOT")
        {
            QProcess *rebootProcess = new QProcess();
            rebootProcess->start("reboot", QStringList());
        }
        else if (cmdList.at(1) == "HALT")
        {
            QProcess *haltProcess = new QProcess();
            haltProcess->start("halt", QStringList());
        }
    }
#endif
    else if (cmdList[0] == "QLC+API")
    {
        if (cmdList.count() < 2)
            return MG_FALSE;

        QString apiCmd = cmdList[1];
        // compose the basic API reply messages
        QString wsAPIMessage = QString("QLC+API|%1|").arg(apiCmd);

        if (apiCmd == "isProjectLoaded")
        {
            if (m_pendingProjectLoaded)
            {
                wsAPIMessage.append("true");
                m_pendingProjectLoaded = false;
            }
            else
                wsAPIMessage.append("false");
        }
        else if (apiCmd == "getFunctionsNumber")
        {
            wsAPIMessage.append(QString::number(m_doc->functions().count()));
        }
        else if (apiCmd == "getFunctionsList")
        {
            foreach(Function *f, m_doc->functions())
                wsAPIMessage.append(QString("%1|%2|").arg(f->id()).arg(f->name()));
            // remove trailing separator
            wsAPIMessage.truncate(wsAPIMessage.length() - 1);
        }
        else if (apiCmd == "getFunctionType")
        {
            if (cmdList.count() < 3)
                return MG_FALSE;

            quint32 fID = cmdList[2].toUInt();
            Function *f = m_doc->function(fID);
            if (f != NULL)
                wsAPIMessage.append(m_doc->function(fID)->typeString());
            else
                wsAPIMessage.append(Function::typeToString(Function::Undefined));
        }
        else if (apiCmd == "getFunctionStatus")
        {
            if (cmdList.count() < 3)
                return MG_FALSE;

            quint32 fID = cmdList[2].toUInt();
            Function *f = m_doc->function(fID);
            if (f != NULL)
            {
                if (f->isRunning())
                    wsAPIMessage.append("Running");
                else
                    wsAPIMessage.append("Stopped");
            }
            else
                wsAPIMessage.append(Function::typeToString(Function::Undefined));
        }
        else if (apiCmd == "getWidgetsNumber")
        {
            VCFrame *mainFrame = m_vc->contents();
            QList<VCWidget *> chList = mainFrame->findChildren<VCWidget*>();
            wsAPIMessage.append(QString::number(chList.count()));
        }
        else if (apiCmd == "getWidgetsList")
        {
            VCFrame *mainFrame = m_vc->contents();
            foreach(VCWidget *widget, mainFrame->findChildren<VCWidget*>())
                wsAPIMessage.append(QString("%1|%2|").arg(widget->id()).arg(widget->caption()));
            // remove trailing separator
            wsAPIMessage.truncate(wsAPIMessage.length() - 1);
        }
        else if (apiCmd == "getWidgetType")
        {
            if (cmdList.count() < 3)
                return MG_FALSE;

            quint32 wID = cmdList[2].toUInt();
            VCWidget *widget = m_vc->widget(wID);
            if (widget != NULL)
                wsAPIMessage.append(widget->typeToString(widget->type()));
            else
                wsAPIMessage.append(widget->typeToString(VCWidget::UnknownWidget));
        }
        else if (apiCmd == "getWidgetStatus")
        {
            if (cmdList.count() < 3)
                return MG_FALSE;
            quint32 wID = cmdList[2].toUInt();
            VCWidget *widget = m_vc->widget(wID);
            if (widget != NULL)
            {
                switch(widget->type())
                {
                    case VCWidget::ButtonWidget:
                    {
                        VCButton *button = qobject_cast<VCButton*>(widget);
                        if (button->isOn())
                            wsAPIMessage.append("255");
                        else
                            wsAPIMessage.append("0");
                    }
                    break;
                    case VCWidget::SliderWidget:
                    {
                        VCSlider *slider = qobject_cast<VCSlider*>(widget);
                        wsAPIMessage.append(QString::number(slider->sliderValue()));
                    }
                    break;
                    case VCWidget::CueListWidget:
                    {
                        VCCueList *cue = qobject_cast<VCCueList*>(widget);
                        quint32 chaserID = cue->chaserID();
                        Function *f = m_doc->function(chaserID);
                        if (f != NULL && f->isRunning())
                            wsAPIMessage.append(QString("PLAY|%2|").arg(cue->getCurrentIndex()));
                        else
                            wsAPIMessage.append("STOP");
                    }
                    break;
                }
            }
        }
        else if (apiCmd == "getChannelsValues")
        {
            if (cmdList.count() < 4)
                return MG_FALSE;

            quint32 universe = cmdList[2].toUInt() - 1;
            int startAddr = cmdList[3].toInt() - 1;
            int count = 1;
            if (cmdList.count() == 5)
                count = cmdList[4].toInt();

            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(m_doc, m_sd, universe, startAddr, count));
        }
        else if (apiCmd == "sdResetUniverse")
        {
            m_sd->resetUniverse();
            wsAPIMessage = "QLC+API|getChannelsValues|";
            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(
                                m_doc, m_sd, m_sd->getCurrentUniverseIndex(),
                                0, m_sd->getSlidersNumber()));
        }
        //qDebug() << "Simple desk channels:" << wsAPIMessage;

        mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, wsAPIMessage.toUtf8().data(), wsAPIMessage.length());
        return MG_TRUE;
    }
    else if(cmdList[0] == "CH")
    {
        if (cmdList.count() < 3)
            return MG_FALSE;

        uint absAddress = cmdList[1].toInt() - 1;
        int value = cmdList[2].toInt();
        m_sd->setAbsoluteChannelValue(absAddress, uchar(value));

        return MG_TRUE;
    }
    else if(cmdList[0] == "POLL")
        return MG_TRUE;

    if (qData.contains("|") == false)
        return MG_FALSE;

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
                if ((value == 0 && button->isOn()) ||
                    (value != 0 && button->isOn() == false))
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
                if (cmdList.count() < 2)
                    return MG_FALSE;

                VCCueList *cue = qobject_cast<VCCueList*>(widget);
                if (cmdList[1] == "PLAY")
                    cue->slotPlayback();
                else if (cmdList[1] == "PREV")
                    cue->slotPreviousCue();
                else if (cmdList[1] == "NEXT")
                    cue->slotNextCue();
                else if (cmdList[1] == "STEP")
                    cue->playCueAtIndex(cmdList[2].toInt());
            }
            break;
            case VCWidget::FrameWidget:
            case VCWidget::SoloFrameWidget:
            {
                VCFrame *frame = qobject_cast<VCFrame*>(widget);
                frame->blockSignals(true);
                if (cmdList[1] == "NEXT_PG")
                    frame->slotNextPage();
                else if (cmdList[1] == "PREV_PG")
                    frame->slotPreviousPage();
                frame->blockSignals(false);
            }
            break;
            default:
            break;
        }
    }

    return MG_TRUE;
}

mg_result WebAccess::closeHandler(struct mg_connection* conn)
{
    (void)conn;
    m_conn = NULL;
    return MG_TRUE;
}

QString WebAccess::getWidgetHTML(VCWidget *widget)
{
    QString str = "<div class=\"vcwidget\" style=\""
            "left: " + QString::number(widget->x()) + "px; "
            "top: " + QString::number(widget->y()) + "px; "
            "width: " + QString::number(widget->width()) + "px; "
            "height: " + QString::number(widget->height()) + "px; "
            "background-color: " + widget->backgroundColor().name() + ";\">\n";

    str +=  tr("Widget not supported (yet) for web access") + "</div>\n";

    return str;
}

void WebAccess::slotFramePageChanged(int pageNum)
{
    if (m_conn == NULL)
        return;

    VCWidget *frame = (VCWidget *)sender();

    QString wsMessage = QString("%1|FRAME|%2").arg(frame->id()).arg(pageNum);
    QByteArray ba = wsMessage.toUtf8();

    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, ba.data(), ba.length());
}

QString WebAccess::getFrameHTML(VCFrame *frame)
{
    QColor border(90, 90, 90);

    QString str = "<div class=\"vcframe\" style=\"left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " +
           QString::number(frame->width()) +
          "px; height: " + QString::number(frame->height()) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border: 1px solid " + border.name() + ";\">\n";

    if (frame->isHeaderVisible())
    {
        str += "<div class=\"vcframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + "\">" + frame->caption() + "</div>\n";
        if (frame->multipageMode())
        {
            str += "<div style=\"position: absolute; top: 0; right:0; z-index: 1;\">\n";
            str += "<a class=\"vcframeButton\" href=\"javascript:framePreviousPage(";
            str += QString::number(frame->id()) + ");\">";
            str += "<img src=\"back.png\" width=\"27\"></a>\n";
            str += "<div class=\"vcframePageLabel\" id=\"fr" + QString::number(frame->id()) + "Page\">";
            str += QString ("%1 %2").arg(tr("Page")).arg(frame->currentPage() + 1) + "</div>\n";
            str += "<a class=\"vcframeButton\" href=\"javascript:frameNextPage(";
            str += QString::number(frame->id()) + ");\">";
            str += "<img src=\"forward.png\" width=\"27\"></a>\n";
            str += "</div>\n";

            m_JScode += "framesCurrentPage[" + QString::number(frame->id()) + "] = " + QString::number(frame->currentPage()) + ";\n";
            m_JScode += "framesTotalPages[" + QString::number(frame->id()) + "] = " + QString::number(frame->totalPagesNumber()) + ";\n\n";
            connect(frame, SIGNAL(pageChanged(int)),
                    this, SLOT(slotFramePageChanged(int)));
        }
    }

    str += getChildrenHTML(frame, frame->totalPagesNumber(), frame->currentPage());
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
          "border: 1px solid " + border.name() + ";\">\n";

    if (frame->isHeaderVisible())
    {
        str += "<div class=\"vcsoloframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + "\">" + frame->caption() + "</div>\n";
        if (frame->multipageMode())
        {
            str += "<div style=\"position: absolute; top: 0; right:0; z-index: 1;\">\n";
            str += "<a class=\"vcframeButton\" href=\"javascript:framePreviousPage(";
            str += QString::number(frame->id()) + ");\">";
            str += "<img src=\"back.png\" width=\"27\"></a>\n";
            str += "<div class=\"vcframePageLabel\" id=\"fr" + QString::number(frame->id()) + "Page\">";
            str += QString ("%1 %2").arg(tr("Page")).arg(frame->currentPage() + 1) + "</div>\n";
            str += "<a class=\"vcframeButton\" href=\"javascript:frameNextPage(";
            str += QString::number(frame->id()) + ");\">";
            str += "<img src=\"forward.png\" width=\"27\"></a>\n";
            str += "</div>\n";

            m_JScode += "framesCurrentPage[" + QString::number(frame->id()) + "] = " + QString::number(frame->currentPage()) + ";\n";
            m_JScode += "framesTotalPages[" + QString::number(frame->id()) + "] = " + QString::number(frame->totalPagesNumber()) + ";\n\n";
            connect(frame, SIGNAL(pageChanged(int)),
                    this, SLOT(slotFramePageChanged(int)));
        }
    }

    str += getChildrenHTML(frame, frame->totalPagesNumber(), frame->currentPage());
    str += "</div>\n";

    return str;
}

void WebAccess::slotButtonToggled(bool on)
{
    if (m_conn == NULL)
        return;

    VCButton *btn = (VCButton *)sender();

    QString wsMessage = QString::number(btn->id());
    if (on == true)
        wsMessage.append("|BUTTON|1");
    else
        wsMessage.append("|BUTTON|0");

    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toUtf8().data(), wsMessage.length());
}

QString WebAccess::getButtonHTML(VCButton *btn)
{
    QString onCSS = "";
    if (btn->isOn())
        onCSS = "border: 3px solid #00E600;";

    QString str = "<div class=\"vcbutton-wrapper\" style=\""
            "left: " + QString::number(btn->x()) + "px; "
            "top: " + QString::number(btn->y()) + "px;\">\n";
    str +=  "<a class=\"vcbutton\" id=\"" + QString::number(btn->id()) + "\" "
            "href=\"javascript:buttonClick(" + QString::number(btn->id()) + ");\" "
            "style=\""
            "width: " + QString::number(btn->width()) + "px; "
            "height: " + QString::number(btn->height()) + "px; "
            "color: " + btn->foregroundColor().name() + "; "
            "background-color: " + btn->backgroundColor().name() + "; " + onCSS + "\">" +
            btn->caption() + "</a>\n</div>\n";

    connect(btn, SIGNAL(pressedState(bool)),
            this, SLOT(slotButtonToggled(bool)));

    return str;
}

void WebAccess::slotSliderValueChanged(QString val)
{
    if (m_conn == NULL)
        return;

    VCSlider *slider = (VCSlider *)sender();

    QString wsMessage = QString("%1|SLIDER|%2").arg(slider->id()).arg(val);

    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toUtf8().data(), wsMessage.length());
}

QString WebAccess::getSliderHTML(VCSlider *slider)
{
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
            "oninput=\"slVchange(" + slID + ");\" ontouchmove=\"slVchange(" + slID + ");\" "
            "style=\""
            "width: " + QString::number(slider->height() - 50) + "px; "
            "margin-top: " + QString::number(slider->height() - 50) + "px; "
            "margin-left: " + QString::number(slider->width() / 2) + "px;\" "
            "min=\"0\" max=\"255\" step=\"1\" value=\"" +
            QString::number(slider->sliderValue()) + "\">\n";

    str += "<div id=\"sln" + slID + "\" "
            "class=\"vcslLabel\" style=\"bottom:0px;\">" +
            slider->caption() + "</div>\n"
            "</div>\n";

    connect(slider, SIGNAL(valueChanged(QString)),
            this, SLOT(slotSliderValueChanged(QString)));
    return str;
}

QString WebAccess::getLabelHTML(VCLabel *label)
{
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

void WebAccess::slotCueIndexChanged(int idx)
{
    if (m_conn == NULL)
        return;

    VCCueList *cue = (VCCueList *)sender();

    QString wsMessage = QString("%1|CUE|%2").arg(cue->id()).arg(idx);

    mg_websocket_write(m_conn, WEBSOCKET_OPCODE_TEXT, wsMessage.toUtf8().data(), wsMessage.length());
}

QString WebAccess::getCueListHTML(VCCueList *cue)
{
    QString str = "<div id=\"" + QString::number(cue->id()) + "\" "
            "class=\"vccuelist\" style=\"left: " + QString::number(cue->x()) +
            "px; top: " + QString::number(cue->y()) + "px; width: " +
             QString::number(cue->width()) +
            "px; height: " + QString::number(cue->height()) + "px; "
            "background-color: " + cue->backgroundColor().name() + ";\">\n";

    str += "<div style=\"width: 100%; height: " + QString::number(cue->height() - 34) + "px; overflow: scroll;\" >\n";
    str += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    str += "<tr><th>#</th><th>" + tr("Name") + "</th>";
    str += "<th>" + tr("Fade In") + "</th>";
    str += "<th>" + tr("Fade Out") + "</th>";
    str += "<th>" + tr("Duration") + "</th>";
    str += "<th>" + tr("Notes") + "</th></tr>\n";
    Chaser *chaser = cue->chaser();
    Doc *doc = m_vc->getDoc();
    if (chaser != NULL)
    {
        for (int i = 0; i < chaser->stepsCount(); i++)
        {
            QString stepID = QString::number(cue->id()) + "_" + QString::number(i);
            str += "<tr id=\"" + stepID + "\" "
                    "onclick=\"enableCue(" + QString::number(cue->id()) + ", " + QString::number(i) + ");\" "
                    "onmouseover=\"this.style.backgroundColor='#CCD9FF';\" "
                    "onmouseout=\"checkMouseOut(" + QString::number(cue->id()) + ", " + QString::number(i) + ");\">\n";
            ChaserStep step = chaser->stepAt(i);
            str += "<td>" + QString::number(i + 1) + "</td>";
            Function* function = doc->function(step.fid);
            if (function != NULL)
            {
                str += "<td>" + function->name() + "</td>";

                switch (chaser->fadeInMode())
                {
                    case Chaser::Common:
                    {
                        if (chaser->fadeInSpeed() == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(chaser->fadeInSpeed()) + "</td>";
                    }
                    break;
                    case Chaser::PerStep:
                    {
                        if (step.fadeIn == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step.fadeIn) + "</td>";
                    }
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
                    {
                        if (chaser->fadeOutSpeed() == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(chaser->fadeOutSpeed()) + "</td>";
                    }
                    break;
                    case Chaser::PerStep:
                    {
                        if (step.fadeOut == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step.fadeOut) + "</td>";
                    }
                    break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                switch (chaser->durationMode())
                {
                    case Chaser::Common:
                    {
                        if (chaser->duration() == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(chaser->duration()) + "</td>";
                    }
                    break;
                    case Chaser::PerStep:
                    {
                        if (step.fadeOut == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step.duration) + "</td>";
                    }
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

    str += "<a class=\"vccuelistButton\" id=\"play" + QString::number(cue->id()) + "\" ";
    str += "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'PLAY');\">\n";
    str += "<img src=\"player_play.png\" width=\"27\"></a>\n";

    str += "<a class=\"vccuelistButton\" href=\"javascript:sendCueCmd(";
    str += QString::number(cue->id()) + ", 'PREV');\">\n";
    str += "<img src=\"back.png\" width=\"27\"></a>\n";

    str += "<a class=\"vccuelistButton\" href=\"javascript:sendCueCmd(";
    str += QString::number(cue->id()) + ", 'NEXT');\">\n";
    str += "<img src=\"forward.png\" width=\"27\"></a>\n";

    str += "</div>\n";

    connect(cue, SIGNAL(stepChanged(int)),
            this, SLOT(slotCueIndexChanged(int)));

    return str;
}

QString WebAccess::getChildrenHTML(VCWidget *frame, int pagesNum, int currentPageIdx)
{
    if (frame == NULL)
        return QString();

    QString unifiedHTML;
    QStringList pagesHTML;
    VCFrame *lframe = (VCFrame *)frame;

    if (lframe->multipageMode() == true)
    {
        for (int i = 0; i < pagesNum; i++)
        {
            QString fpID = QString("fp%1_%2").arg(frame->id()).arg(i);
            QString pg = "<div class=\"vcframePage\" id=\"" + fpID + "\"";
            if (i == currentPageIdx)
                pg += " style=\"visibility: visible;\"";
            pg += ">\n";
            pagesHTML << pg;
        }
    }

    QList<VCWidget *> chList = frame->findChildren<VCWidget*>();

    qDebug () << "getChildrenHTML: found " << chList.count() << " children";

    foreach (VCWidget *widget, chList)
    {
        if (widget->parentWidget() != frame)
            continue;

        QString str;
        bool restoreDisable = false;

        if (pagesNum > 0 && widget->isEnabled() == false)
        {
            widget->setEnabled(true);
            restoreDisable = true;
        }

        switch (widget->type())
        {
            case VCWidget::FrameWidget:
                str = getFrameHTML((VCFrame *)widget);
            break;
            case VCWidget::SoloFrameWidget:
                str = getSoloFrameHTML((VCSoloFrame *)widget);
            break;
            case VCWidget::ButtonWidget:
                str = getButtonHTML((VCButton *)widget);
            break;
            case VCWidget::SliderWidget:
                str = getSliderHTML((VCSlider *)widget);
            break;
            case VCWidget::LabelWidget:
                str = getLabelHTML((VCLabel *)widget);
            break;
            case VCWidget::AudioTriggersWidget:
                str = getAudioTriggersHTML((VCAudioTriggers *)widget);
            break;
            case VCWidget::CueListWidget:
                str = getCueListHTML((VCCueList *)widget);
            break;
            default:
                str = getWidgetHTML(widget);
            break;
        }
        if (lframe->multipageMode() == true && pagesNum > 0)
        {
            pagesHTML[widget->page()] += str;
            if (restoreDisable)
                widget->setEnabled(false);
        }
        else
            unifiedHTML += str;
    }

    if (pagesNum > 0)
    {
        for(int i = 0; i < pagesHTML.count(); i++)
        {
            unifiedHTML += pagesHTML.at(i);
            unifiedHTML += "</div>\n";
        }
    }
    return unifiedHTML;
}

QString WebAccess::getVCHTML()
{
    m_CSScode = "<link href=\"common.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen\">\n";
    m_CSScode += "<link href=\"virtualconsole.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen\">\n";
    m_JScode = "<script type=\"text/javascript\" src=\"websocket.js\"></script>\n"
               "<script type=\"text/javascript\" src=\"virtualconsole.js\"></script>\n"
               "<script type=\"text/javascript\">\n";

    VCFrame *mainFrame = m_vc->contents();
    QSize mfSize = mainFrame->size();
    QString widgetsHTML =
            "<form action=\"/loadProject\" method=\"POST\" enctype=\"multipart/form-data\">\n"
				"<input id=\"loadTrigger\" type=\"file\" "
				"onchange=\"document.getElementById('submitTrigger').click();\" name=\"qlcprj\" />\n"
				"<input id=\"submitTrigger\" type=\"submit\"/>\n"
            "</form>\n"

            "<div class=\"controlBar\">\n"
            "<a class=\"button button-blue\" href=\"javascript:document.getElementById('loadTrigger').click();\">\n"
            "<span>" + tr("Load project") + "</span></a>\n"

            "<a class=\"button button-blue\" href=\"/simpleDesk\"><span>" + tr("Simple Desk") + "</span></a>\n"

            "<a class=\"button button-blue\" href=\"/config\"><span>" + tr("Configuration") + "</span></a>\n"

            "<div class=\"swInfo\">" + QString(APPNAME) + " " + QString(APPVERSION) + "</div>"
            "</div>\n"
            "<div style=\"position: relative; "
            "width: " + QString::number(mfSize.width()) +
            "px; height: " + QString::number(mfSize.height()) + "px; "
            "background-color: " + mainFrame->backgroundColor().name() + ";\">\n";

    widgetsHTML += getChildrenHTML(mainFrame, 0, 0);

    m_JScode += "\n</script>\n";

    QString str = HTML_HEADER + m_CSScode + m_JScode + "</head>\n<body>\n" + widgetsHTML + "</div>\n</body>\n</html>";
    return str;
}

QString WebAccess::getSimpleDeskHTML()
{
    QString str = HTML_HEADER;
    return str;
}

void WebAccess::slotVCLoaded()
{
    m_pendingProjectLoaded = true;
}
