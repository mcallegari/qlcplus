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

#include "webaccessauth.h"
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

#include "qhttpserver.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "qhttpconnection.h"

#define AUTOSTART_PROJECT_NAME "autostart.qxw"

WebAccess::WebAccess(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                     bool enableAuth, QString passwdFile, QObject *parent) :
    QObject(parent)
  , m_doc(doc)
  , m_vc(vcInstance)
  , m_sd(sdInstance)
  , m_auth(NULL)
  , m_pendingProjectLoaded(false)
{
    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(m_vc != NULL);

    if (enableAuth)
    {
        m_auth = new WebAccessAuth(QString("QLC+ web access"));
        m_auth->loadPasswordsFile(passwdFile);
    }

    m_httpServer = new QHttpServer(this);
    connect(m_httpServer, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
            this, SLOT(slotHandleRequest(QHttpRequest*, QHttpResponse*)));
    connect(m_httpServer, SIGNAL(webSocketDataReady(QHttpConnection*,QString)),
            this, SLOT(slotHandleWebSocketRequest(QHttpConnection*,QString)));
    connect(m_httpServer, SIGNAL(webSocketConnectionClose(QHttpConnection*)),
            this, SLOT(slotHandleWebSocketClose(QHttpConnection*)));

    m_httpServer->listen(QHostAddress::Any, 9999);

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    m_netConfig = new WebAccessNetwork();
#endif

    connect(m_vc, SIGNAL(loaded()),
            this, SLOT(slotVCLoaded()));
}

WebAccess::~WebAccess()
{
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    delete m_netConfig;
#endif
    foreach(QHttpConnection *conn, m_webSocketsList)
        delete conn;

    if (m_auth)
        delete m_auth;
}

void WebAccess::slotHandleRequest(QHttpRequest *req, QHttpResponse *resp)
{
    WebAccessUser user;

    if(m_auth)
    {
        user = m_auth->authenticateRequest(req, resp);
    
        if(user.level < LOGGED_IN_LEVEL)
        {
            m_auth->sendUnauthorizedResponse(resp);
            return;
        }
    }
    
    QString reqUrl = req->url().toString();
    QString content;

    qDebug() << Q_FUNC_INFO << req->methodString() << req->url();

    if (reqUrl == "/qlcplusWS")
    {
        resp->setHeader("Upgrade", "websocket");
        resp->setHeader("Connection", "Upgrade");
        QByteArray hash = resp->getWebSocketHandshake(req->header("sec-websocket-key"));
        //QByteArray hash = resp->getWebSocketHandshake("zTvHabaaTOEORzqK+d1yxw==");
        qDebug() << "Websocket handshake:" << hash;
        resp->setHeader("Sec-WebSocket-Accept", hash);
        QHttpConnection *conn = resp->enableWebSocket(true);
        if (conn != NULL)
        {
            // Allocate user for WS on heap so it doesn't go out of scope
            conn->userData = new WebAccessUser(user);
            m_webSocketsList.append(conn);
        }

        resp->writeHead(101);
        resp->end(QByteArray());

        return;
    }
    else if (reqUrl == "/loadProject")
    {
        if(m_auth && user.level < SUPER_ADMIN_LEVEL)
        {
            m_auth->sendUnauthorizedResponse(resp);
            return;
        }
        QByteArray projectXML = req->body();

        projectXML.remove(0, projectXML.indexOf("\n\r\n") + 3);
        projectXML.truncate(projectXML.lastIndexOf("\n\r\n"));

        //qDebug() << "Project XML:\n\n" << QString(projectXML) << "\n\n";
        qDebug() << "Workspace XML received. Content-Length:" << req->headers().value("content-length") << projectXML.size();

        QByteArray postReply =
                QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
                "<script type=\"text/javascript\">\n" PROJECT_LOADED_JS
                "</script></head><body style=\"background-color: #45484d;\">"
                "<div style=\"position: absolute; width: 100%; height: 30px; top: 50%; background-color: #888888;"
                "text-align: center; font:bold 24px/1.2em sans-serif;\">"
                + tr("Loading project...") +
                "</div></body></html>").toUtf8();

        resp->setHeader("Content-Type", "text/html");
        resp->setHeader("Content-Length", QString::number(postReply.size()));
        resp->writeHead(200);
        resp->end(postReply);

        m_pendingProjectLoaded = false;

        emit loadProject(QString(projectXML).toUtf8());

        return;
    }
    else if (reqUrl == "/loadFixture")
    {
        if(m_auth && user.level < SUPER_ADMIN_LEVEL)
        {
            m_auth->sendUnauthorizedResponse(resp);
            return;
        }
        QByteArray fixtureXML = req->body();
        int fnamePos = fixtureXML.indexOf("filename=") + 10;
        QString fxName = fixtureXML.mid(fnamePos, fixtureXML.indexOf("\"", fnamePos) - fnamePos);

        fixtureXML.remove(0, fixtureXML.indexOf("\n\r\n") + 3);
        fixtureXML.truncate(fixtureXML.lastIndexOf("\n\r\n"));

        qDebug() << "Fixture name:" << fxName;
        qDebug() << "Fixture XML:\n\n" << fixtureXML << "\n\n";

        m_doc->fixtureDefCache()->storeFixtureDef(fxName, QString(fixtureXML).toUtf8());

        QByteArray postReply =
                      QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
                      "<script type=\"text/javascript\">\n"
                      " alert(\"" + tr("Fixture stored and loaded") + "\");"
                      " window.location = \"/config\"\n"
                      "</script></head></html>").toUtf8();

        resp->setHeader("Content-Type", "text/html");
        resp->setHeader("Content-Length", QString::number(postReply.size()));
        resp->writeHead(200);
        resp->end(postReply);

        return;
    }
    else if (reqUrl == "/config")
    {
        if(m_auth && user.level < SUPER_ADMIN_LEVEL)
        {
            m_auth->sendUnauthorizedResponse(resp);
            return;
        }
        content = WebAccessConfiguration::getHTML(m_doc, m_auth);
    }
    else if (reqUrl == "/simpleDesk")
    {
        if(m_auth && user.level < SIMPLE_DESK_AND_VC_LEVEL)
        {
            m_auth->sendUnauthorizedResponse(resp);
            return;
        }
        content = WebAccessSimpleDesk::getHTML(m_doc, m_sd);
    }
  #if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    else if (reqUrl == "/system")
    {
        if(m_auth && user.level < SUPER_ADMIN_LEVEL)
        {
            m_auth->sendUnauthorizedResponse(resp);
            return;
        }
        content = m_netConfig->getHTML();
    }
  #endif
    else if (reqUrl.endsWith(".png"))
    {
        if (sendFile(resp, QString(":%1").arg(reqUrl), "image/png") == true)
            return;
    }
    else if (reqUrl.endsWith(".css"))
    {
        QString clUri = reqUrl.mid(1);
        if (sendFile(resp, QString("%1%2%3").arg(QLCFile::systemDirectory(WEBFILESDIR).path())
                     .arg(QDir::separator()).arg(clUri), "text/css") == true)
            return;
    }
    else if (reqUrl.endsWith(".js"))
    {
        QString clUri = reqUrl.mid(1);
        if (sendFile(resp, QString("%1%2%3").arg(QLCFile::systemDirectory(WEBFILESDIR).path())
                     .arg(QDir::separator()).arg(clUri), "text/javascript") == true)
            return;
    }
    else if (reqUrl.endsWith(".html"))
    {
        QString clUri = reqUrl.mid(1);
        if (sendFile(resp, QString("%1%2%3").arg(QLCFile::systemDirectory(WEBFILESDIR).path())
                     .arg(QDir::separator()).arg(clUri), "text/html") == true)
            return;
    }
    else if (reqUrl != "/")
    {
        resp->writeHead(404);
        resp->setHeader("Content-Type", "text/plain");
        resp->setHeader("Content-Length", "14");
        resp->end(QByteArray("404 Not found"));
        return;
    }
    else
        content = getVCHTML();

    // Prepare the message we're going to send
    QByteArray contentArray = content.toUtf8();

    // Send HTTP reply to the client
    resp->setHeader("Content-Type", "text/html");
    resp->setHeader("Content-Length", QString::number(contentArray.size()));
    resp->writeHead(200);
    resp->end(contentArray);

    return;
}

void WebAccess::slotHandleWebSocketRequest(QHttpConnection *conn, QString data)
{
    if (conn == NULL)
        return;
    
    WebAccessUser* user = static_cast<WebAccessUser*>(conn->userData);

    qDebug() << "[websocketDataHandler]" << data;

    QStringList cmdList = data.split("|");
    if (cmdList.isEmpty())
        return;

    if(cmdList[0] == "QLC+CMD")
    {
        if (cmdList.count() < 2)
            return;

        if(cmdList[1] == "opMode")
            emit toggleDocMode();

        return;
    }
    else if (cmdList[0] == "QLC+IO")
    {
        if(m_auth && user && user->level < SUPER_ADMIN_LEVEL)
            return;
        
        if (cmdList.count() < 3)
            return;

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

        return;
    }
    else if(cmdList[0] == "QLC+AUTH" && m_auth)
    {
        if(user && user->level < SUPER_ADMIN_LEVEL)
            return;
        
        if (cmdList.at(1) == "ADD_USER")
        {
            QString username = cmdList.at(2);
            QString password = cmdList.at(3);
            int level = cmdList.at(4).toInt();
            if(username.isEmpty() || password.isEmpty())
            {
                QString wsMessage = QString("ALERT|" + tr("Username and password are required fields."));
                conn->webSocketWrite(QHttpConnection::TextFrame, wsMessage.toUtf8());
                return;
            }
            if(level <= 0)
            {
                QString wsMessage = QString("ALERT|" + tr("User level has to be a positive integer."));
                conn->webSocketWrite(QHttpConnection::TextFrame, wsMessage.toUtf8());
                return;
            }

            m_auth->addUser(username, password, (WebAccessUserLevel)level);
        }
        else if (cmdList.at(1) == "DEL_USER")
        {
            QString username = cmdList.at(2);
            if(! username.isEmpty())
                m_auth->deleteUser(username);
        }
        else if (cmdList.at(1) == "SET_USER_LEVEL")
        {
            QString username = cmdList.at(2);
            int level = cmdList.at(3).toInt();
            if(username.isEmpty())
            {
                QString wsMessage = QString("ALERT|" + tr("Username is required."));
                conn->webSocketWrite(QHttpConnection::TextFrame, wsMessage.toUtf8());
                return;
            }
            if(level <= 0)
            {
                QString wsMessage = QString("ALERT|" + tr("User level has to be a positive integer."));
                conn->webSocketWrite(QHttpConnection::TextFrame, wsMessage.toUtf8());
                return;
            }

            m_auth->setUserLevel(username, (WebAccessUserLevel)level);
        }
        else
            qDebug() << "[webaccess] Command" << cmdList[1] << "not supported !";
        
        if(! m_auth->savePasswordsFile())
        {
            QString wsMessage = QString("ALERT|" + tr("Error while saving passwords file."));
            conn->webSocketWrite(QHttpConnection::TextFrame, wsMessage.toUtf8());
            return;
        }
    }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    else if(cmdList[0] == "QLC+SYS")
    {
        if(m_auth && user && user->level < SUPER_ADMIN_LEVEL)
            return;
        
        if (cmdList.at(1) == "NETWORK")
        {
            if (m_netConfig->updateNetworkFile(cmdList) == true)
            {
                QString wsMessage = QString("ALERT|" + tr("Network configuration changed. Reboot to apply the changes."));
                conn->webSocketWrite(QHttpConnection::TextFrame, wsMessage.toUtf8());
                return;
            }
            else
                qDebug() << "[webaccess] Error writing network configuration file !";

            return;
        }
        else if (cmdList.at(1) == "AUTOSTART")
        {
            if (cmdList.count() < 3)
                return;

            QString asName = QString("%1/%2/%3").arg(getenv("HOME")).arg(USERQLCPLUSDIR).arg(AUTOSTART_PROJECT_NAME);
            if (cmdList.at(2) == "none")
                QFile::remove(asName);
            else
                emit storeAutostartProject(asName);
            QString wsMessage = QString("ALERT|" + tr("Autostart configuration changed"));
            conn->webSocketWrite(QHttpConnection::TextFrame, wsMessage.toUtf8());
            return;
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
        if(m_auth && user && user->level < VC_ONLY_LEVEL)
            return;
        
        if (cmdList.count() < 2)
            return;

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
                return;

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
                return;

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
                return;

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
                return;

            quint32 wID = cmdList[2].toUInt();
            VCWidget *widget = m_vc->widget(wID);
            if (widget != NULL)
            {
                switch(widget->type())
                {
                    case VCWidget::ButtonWidget:
                    {
                        VCButton *button = qobject_cast<VCButton*>(widget);
                        if (button->state() == VCButton::Active)
                            wsAPIMessage.append("255");
                        else if (button->state() == VCButton::Monitoring)
                            wsAPIMessage.append("127");
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
            if(m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
                return;
            
            if (cmdList.count() < 4)
                return;

            quint32 universe = cmdList[2].toUInt() - 1;
            int startAddr = cmdList[3].toInt() - 1;
            int count = 1;
            if (cmdList.count() == 5)
                count = cmdList[4].toInt();

            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(m_doc, m_sd, universe, startAddr, count));
        }
        else if (apiCmd == "sdResetChannel")
        {
            if(m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
                return;

            if (cmdList.count() < 3)
                return;

            quint32 chNum = cmdList[2].toUInt() - 1;
            m_sd->resetChannel(chNum);
            wsAPIMessage = "QLC+API|getChannelsValues|";
            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(
                                m_doc, m_sd, m_sd->getCurrentUniverseIndex(),
                                (m_sd->getCurrentPage() - 1) * m_sd->getSlidersNumber(), m_sd->getSlidersNumber()));
        }
        else if (apiCmd == "sdResetUniverse")
        {
            if(m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
                return;

            m_sd->resetUniverse();
            wsAPIMessage = "QLC+API|getChannelsValues|";
            wsAPIMessage.append(WebAccessSimpleDesk::getChannelsMessage(
                                m_doc, m_sd, m_sd->getCurrentUniverseIndex(),
                                0, m_sd->getSlidersNumber()));
        }
        //qDebug() << "Simple desk channels:" << wsAPIMessage;

        conn->webSocketWrite(QHttpConnection::TextFrame, wsAPIMessage.toUtf8());
        return;
    }
    else if(cmdList[0] == "CH")
    {
        if(m_auth && user && user->level < SIMPLE_DESK_AND_VC_LEVEL)
            return;
        
        if (cmdList.count() < 3)
            return;

        uint absAddress = cmdList[1].toInt() - 1;
        int value = cmdList[2].toInt();
        m_sd->setAbsoluteChannelValue(absAddress, uchar(value));

        return;
    }
    else if(cmdList[0] == "POLL")
        return;

    if (data.contains("|") == false)
        return;

    if(m_auth && user && user->level < VC_ONLY_LEVEL)
        return;
    
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
                if(value)
                    button->pressFunction();
                else
                    button->releaseFunction();
            }
            break;
            case VCWidget::SliderWidget:
            {
                VCSlider *slider = qobject_cast<VCSlider*>(widget);
                slider->setSliderValue(value);
                slider->updateFeedback();
            }
            break;
            case VCWidget::AudioTriggersWidget:
            {
                VCAudioTriggers *triggers = qobject_cast<VCAudioTriggers*>(widget);
                triggers->toggleEnableButton(value ? true : false);
            }
            break;
            case VCWidget::CueListWidget:
            {
                if (cmdList.count() < 2)
                    return;

                VCCueList *cue = qobject_cast<VCCueList*>(widget);
                if (cmdList[1] == "PLAY")
                    cue->slotPlayback();
                else if (cmdList[1] == "STOP")
                    cue->slotStop();
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
                if (cmdList[1] == "NEXT_PG")
                    frame->slotNextPage();
                else if (cmdList[1] == "PREV_PG")
                    frame->slotPreviousPage();
            }
            break;
            default:
            break;
        }
    }
}

void WebAccess::slotHandleWebSocketClose(QHttpConnection *conn)
{
    if(conn->userData)
    {
        WebAccessUser* user = static_cast<WebAccessUser*>(conn->userData);
        delete user;
        conn->userData = 0;
    }

    m_webSocketsList.removeOne(conn);
}

bool WebAccess::sendFile(QHttpResponse *response, QString filename, QString contentType)
{
    QFile resFile(filename);
    if (resFile.open(QIODevice::ReadOnly))
    {
        QByteArray resContent = resFile.readAll();
        qDebug() << "Resource file length:" << resContent.length();
        resFile.close();

        response->setHeader("Content-Type", contentType);
        response->setHeader("Content-Length", QString::number(resContent.size()));
        response->writeHead(200);
        response->end(resContent);

        return true;
    }
    else
        qDebug() << "Failed to open file:" << filename;

    return false;
}

void WebAccess::sendWebSocketMessage(QByteArray message)
{
    foreach(QHttpConnection *conn, m_webSocketsList)
        conn->webSocketWrite(QHttpConnection::TextFrame, message);
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
    VCWidget *frame = qobject_cast<VCWidget *>(sender());
    if (frame == NULL)
        return;

    QString wsMessage = QString("%1|FRAME|%2").arg(frame->id()).arg(pageNum);
    QByteArray ba = wsMessage.toUtf8();

    sendWebSocketMessage(ba);
}

QString WebAccess::getFrameHTML(VCFrame *frame)
{
    QColor border(90, 90, 90);
    QSize origSize = frame->originalSize();
    int w = frame->isCollapsed() ? 200 : origSize.width();
    int h = frame->isCollapsed() ? 36 : origSize.height();

    QString str = "<div class=\"vcframe\" id=\"fr" + QString::number(frame->id()) + "\" "
          "style=\"left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " + QString::number(w) +
          "px; height: " + QString::number(h) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border: 1px solid " + border.name() + ";\">\n";

    str += getChildrenHTML(frame, frame->totalPagesNumber(), frame->currentPage());

    if (frame->isHeaderVisible())
    {
        str += "<a class=\"vcframeButton\" style=\"position: absolute; left: 0; \" href=\"javascript:frameToggleCollapse(";
        str += QString::number(frame->id()) + ");\"><img src=\"expand.png\" width=\"27\"></a>\n";
        str += "<div class=\"vcframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + ";\"><div class=\"vcFrameText\">" + frame->caption() + "</div></div>\n";

        m_JScode += "framesWidth[" + QString::number(frame->id()) + "] = " + QString::number(origSize.width()) + ";\n";
        m_JScode += "framesHeight[" + QString::number(frame->id()) + "] = " + QString::number(origSize.height()) + ";\n";

        if (frame->multipageMode())
        {
            str += "<div id=\"frMpHdr" + QString::number(frame->id()) + "\"";
            str += "style=\"position: absolute; top: 0; right: 2px;\">\n";
            str += "<a class=\"vcframeButton\" href=\"javascript:framePreviousPage(";
            str += QString::number(frame->id()) + ");\">";
            str += "<img src=\"back.png\" width=\"27\"></a>";
            str += "<div class=\"vcframePageLabel\"><div class=\"vcFrameText\" id=\"fr" + QString::number(frame->id()) + "Page\">";
            str += QString ("%1 %2").arg(tr("Page")).arg(frame->currentPage() + 1) + "</div></div>";
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

    str += "</div>\n";

    return str;
}

QString WebAccess::getSoloFrameHTML(VCSoloFrame *frame)
{
    QColor border(255, 0, 0);
    QSize origSize = frame->originalSize();
    int w = frame->isCollapsed() ? 200 : origSize.width();
    int h = frame->isCollapsed() ? 36 : origSize.height();

    QString str = "<div class=\"vcframe\" id=\"fr" + QString::number(frame->id()) + "\" "
          "style=\"left: " + QString::number(frame->x()) +
          "px; top: " + QString::number(frame->y()) + "px; width: " + QString::number(w) +
          "px; height: " + QString::number(h) + "px; "
          "background-color: " + frame->backgroundColor().name() + "; "
          "border: 1px solid " + border.name() + ";\">\n";

    str += getChildrenHTML(frame, frame->totalPagesNumber(), frame->currentPage());

    if (frame->isHeaderVisible())
    {
        str += "<a class=\"vcframeButton\" style=\"position: absolute; left: 0; \" href=\"javascript:frameToggleCollapse(";
        str += QString::number(frame->id()) + ");\"><img src=\"expand.png\" width=\"27\"></a>\n";
        str += "<div class=\"vcsoloframeHeader\" style=\"color:" +
                frame->foregroundColor().name() + ";\"><div class=\"vcFrameText\">" + frame->caption() + "</div></div>\n";

        m_JScode += "framesWidth[" + QString::number(frame->id()) + "] = " + QString::number(origSize.width()) + ";\n";
        m_JScode += "framesHeight[" + QString::number(frame->id()) + "] = " + QString::number(origSize.height()) + ";\n";

        if (frame->multipageMode())
        {
            str += "<div id=\"frMpHdr" + QString::number(frame->id()) + "\"";
            str += "style=\"position: absolute; top: 0; right: 2px;\">\n";
            str += "<a class=\"vcframeButton\" href=\"javascript:framePreviousPage(";
            str += QString::number(frame->id()) + ");\">";
            str += "<img src=\"back.png\" width=\"27\"></a>";
            str += "<div class=\"vcframePageLabel\"><div class=\"vcFrameText\" id=\"fr" + QString::number(frame->id()) + "Page\">";
            str += QString ("%1 %2").arg(tr("Page")).arg(frame->currentPage() + 1) + "</div></div>";
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

    str += "</div>\n";

    return str;
}

void WebAccess::slotButtonStateChanged(int state)
{
    VCButton *btn = qobject_cast<VCButton *>(sender());
    if (btn == NULL)
        return;

    qDebug() << "Button state changed" << state;

    QString wsMessage = QString::number(btn->id());
    if (state == VCButton::Active)
        wsMessage.append("|BUTTON|255");
    else if (state == VCButton::Monitoring)
        wsMessage.append("|BUTTON|127");
    else
        wsMessage.append("|BUTTON|0");

    sendWebSocketMessage(wsMessage.toUtf8());
}

QString WebAccess::getButtonHTML(VCButton *btn)
{
    QString onCSS = "";
    if (btn->state() == VCButton::Active)
        onCSS = "border: 3px solid #00E600;";
    else if (btn->state() == VCButton::Monitoring)
        onCSS = "border: 3px solid #FFAA00;";

    QString str = "<div class=\"vcbutton-wrapper\" style=\""
            "left: " + QString::number(btn->x()) + "px; "
            "top: " + QString::number(btn->y()) + "px;\">\n";
    str +=  "<a class=\"vcbutton\" id=\"" + QString::number(btn->id()) + "\" "
            "href=\"javascript:void(0);\" "
            "onmousedown=\"buttonPress(" + QString::number(btn->id()) + ");\" "
            "onmouseup=\"buttonRelease(" + QString::number(btn->id()) + ");\" "
            "style=\""
            "width: " + QString::number(btn->width()) + "px; "
            "height: " + QString::number(btn->height()) + "px; "
            "color: " + btn->foregroundColor().name() + "; "
            "background-color: " + btn->backgroundColor().name() + "; " + onCSS + "\">" +
            btn->caption() + "</a>\n</div>\n";

    connect(btn, SIGNAL(stateChanged(int)),
            this, SLOT(slotButtonStateChanged(int)));

    return str;
}

void WebAccess::slotSliderValueChanged(QString val)
{
    VCSlider *slider = qobject_cast<VCSlider *>(sender());
    if (slider == NULL)
        return;

    // <ID>|SLIDER|<SLIDER VALUE>|<DISPLAY VALUE>
    QString wsMessage = QString("%1|SLIDER|%2|%3").arg(slider->id()).arg(slider->sliderValue()).arg(val);

    sendWebSocketMessage(wsMessage.toUtf8());
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
            slider->topLabelText() + "</div>\n";

    str +=  "<input type=\"range\" class=\"vVertical\" "
            "id=\"" + slID + "\" "
            "oninput=\"slVchange(" + slID + ");\" ontouchmove=\"slVchange(" + slID + ");\" "
            "style=\""
            "width: " + QString::number(slider->height() - 50) + "px; "
            "margin-top: " + QString::number(slider->height() - 50) + "px; "
            "margin-left: " + QString::number(slider->width() / 2) + "px;\" ";

    if (slider->sliderMode() == VCSlider::Level)
        str += "min=\"" + QString::number(slider->levelLowLimit()) + "\" max=\"" +
                QString::number(slider->levelHighLimit()) + "\" ";
    else
        str += "min=\"0\" max=\"255\" ";

    str += "step=\"1\" value=\"" + QString::number(slider->sliderValue()) + "\">\n";

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

void WebAccess::slotAudioTriggersToggled(bool toggle)
{
    VCAudioTriggers *triggers = qobject_cast<VCAudioTriggers *>(sender());
    if (triggers == NULL)
        return;

    qDebug() << "AudioTriggers state changed " << toggle;

    QString wsMessage = QString("%1|AUDIOTRIGGERS|%2").arg(triggers->id()).arg(toggle ? 255 : 0);

    sendWebSocketMessage(wsMessage.toUtf8());
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

    connect(triggers, SIGNAL(captureEnabled(bool)),
            this, SLOT(slotAudioTriggersToggled(bool)));

    return str;
}

void WebAccess::slotCueIndexChanged(int idx)
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == NULL)
        return;

    QString wsMessage = QString("%1|CUE|%2").arg(cue->id()).arg(idx);

    sendWebSocketMessage(wsMessage.toUtf8());
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
            ChaserStep *step = chaser->stepAt(i);
            str += "<td>" + QString::number(i + 1) + "</td>";
            Function* function = doc->function(step->fid);
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
                        if (step->fadeIn == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step->fadeIn) + "</td>";
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
                        if (step->fadeOut == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step->fadeOut) + "</td>";
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
                        if (step->fadeOut == Function::infiniteSpeed())
                            str += "<td>&#8734;</td>";
                        else
                            str += "<td>" + Function::speedToString(step->duration) + "</td>";
                    }
                    break;
                    default:
                    case Chaser::Default:
                        str += "<td></td>";
                }

                str += "<td>" + step->note + "</td>\n";
            }
            str += "</td>\n";
        }
    }
    str += "</table>\n";
    str += "</div>\n";

    str += "<a class=\"vccuelistButton\" id=\"play" + QString::number(cue->id()) + "\" ";
    str += "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'PLAY');\">\n";
    str += "<img src=\"player_play.png\" width=\"27\"></a>\n";

    str += "<a class=\"vccuelistButton\" id=\"stop" + QString::number(cue->id()) + "\" ";
    str += "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'STOP');\">\n";
    str += "<img src=\"player_stop.png\" width=\"27\"></a>\n";

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
    VCFrame *lframe = qobject_cast<VCFrame *>(frame);
    if (lframe == NULL)
        return "";

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
                str = getFrameHTML(qobject_cast<VCFrame *>(widget));
            break;
            case VCWidget::SoloFrameWidget:
                str = getSoloFrameHTML(qobject_cast<VCSoloFrame *>(widget));
            break;
            case VCWidget::ButtonWidget:
                str = getButtonHTML(qobject_cast<VCButton *>(widget));
            break;
            case VCWidget::SliderWidget:
                str = getSliderHTML(qobject_cast<VCSlider *>(widget));
            break;
            case VCWidget::LabelWidget:
                str = getLabelHTML(qobject_cast<VCLabel *>(widget));
            break;
            case VCWidget::AudioTriggersWidget:
                str = getAudioTriggersHTML(qobject_cast<VCAudioTriggers *>(widget));
            break;
            case VCWidget::CueListWidget:
                str = getCueListHTML(qobject_cast<VCCueList *>(widget));
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
