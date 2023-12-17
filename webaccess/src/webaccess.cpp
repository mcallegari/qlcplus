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
#include "vcclock.h"
#include "vcmatrix.h"
#include "rgbalgorithm.h"
#include "qlcfile.h"
#include "chaser.h"
#include "doc.h"

#include "audiocapture.h"
#include "audiorenderer.h"

#include "qhttpserver.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "qhttpconnection.h"

#define DEFAULT_PORT_NUMBER    9999
#define AUTOSTART_PROJECT_NAME "autostart.qxw"

WebAccess::WebAccess(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                     int portNumber, bool enableAuth, QString passwdFile, QObject *parent) :
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
    connect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)),
            this, SLOT(slotFunctionStarted(quint32)));
    connect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)),
            this, SLOT(slotFunctionStopped(quint32)));

    m_httpServer->listen(QHostAddress::Any, portNumber ? portNumber : DEFAULT_PORT_NUMBER);

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
        QString clUri = QString(":%1").arg(reqUrl);
        QFile resFile(clUri);
        if (!resFile.exists())
            clUri = reqUrl;
        if (sendFile(resp, clUri, "image/png") == true)
            return;
    }
    else if (reqUrl.endsWith(".jpg"))
    {
        if (sendFile(resp, reqUrl, "image/jpg") == true)
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
            m_doc->inputOutputMap()->setInputPatch(universe, cmdList[3], "", cmdList[4].toUInt());
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "OUTPUT")
        {
            m_doc->inputOutputMap()->setOutputPatch(universe, cmdList[3], "", cmdList[4].toUInt(), false);
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "FB")
        {
            m_doc->inputOutputMap()->setOutputPatch(universe, cmdList[3], "", cmdList[4].toUInt(), true);
            m_doc->inputOutputMap()->saveDefaults();
        }
        else if (cmdList[1] == "PROFILE")
        {
            InputPatch *inPatch = m_doc->inputOutputMap()->inputPatch(universe);
            if (inPatch != NULL)
            {
                m_doc->inputOutputMap()->setInputPatch(universe, inPatch->pluginName(), "", inPatch->input(), cmdList[3]);
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
            qDebug() << "[webaccess] Command" << cmdList[1] << "not supported!";

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
            qDebug() << "[webaccess] Command" << cmdList[1] << "not supported!";

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
                qDebug() << "[webaccess] Error writing network configuration file!";

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
        else if (apiCmd == "setFunctionStatus") 
	{
            if (cmdList.count() < 4)
                return;

            quint32 fID = cmdList[2].toUInt();
            quint32 newStatus = cmdList[3].toUInt();
            Function *f = m_doc->function(fID);

            if (f != NULL)
            {
                if (!f->isRunning() && newStatus)
                    f->start(m_doc->masterTimer(), FunctionParent::master());
                else if (f->isRunning() && !newStatus)
                    f->stop(FunctionParent::master());
            }
            return;
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

    /** Handle direct widget operations;
     *  Commands start with the widget ID,
     *  followed by specific parameters */

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
                slider->setSliderValue(value, false, true);
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
                else if (cmdList[1] == "CUE_SHOWPANEL")
                    cue->slotSideFaderButtonChecked(cmdList[2] == "1" ? false : true);
                else if (cmdList[1] == "CUE_SIDECHANGE")
                    cue->slotSetSideFaderValue((cmdList[2]).toInt());
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
            case VCWidget::ClockWidget:
            {
                VCClock *clock = qobject_cast<VCClock*>(widget);
                if (cmdList[1] == "S")
                    clock->playPauseTimer();
                else if (cmdList[1] == "R")
                    clock->resetTimer();
            }
            break;
            case VCWidget::AnimationWidget:
            {
                VCMatrix *matrix = qobject_cast<VCMatrix*>(widget);
                if (cmdList[1] == "MATRIX_SLIDER_CHANGE")
                    matrix->slotSetSliderValue(cmdList[2].toInt());
                if (cmdList[1] == "MATRIX_COMBO_CHANGE")
                    matrix->slotSetAnimationValue(cmdList[2]);
                if (cmdList[1] == "MATRIX_COLOR_CHANGE" && cmdList[2] == "START")
                    matrix->slotStartColorChanged(cmdList[3].toInt());
                if (cmdList[1] == "MATRIX_COLOR_CHANGE" && cmdList[2] == "END")
                    matrix->slotEndColorChanged(cmdList[3].toInt());
                if (cmdList[1] == "MATRIX_KNOB")
                    matrix->slotMatrixControlKnobValueChanged(cmdList[2].toInt(), cmdList[3].toInt());
                if (cmdList[1] == "MATRIX_PUSHBUTTON")
                    matrix->slotMatrixControlPushButtonClicked(cmdList[2].toInt());
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

void WebAccess::slotFunctionStarted(quint32 fid)
{
    QString wsMessage = QString("FUNCTION|%1|Running").arg(fid);

    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotFunctionStopped(quint32 fid)
{
    QString wsMessage = QString("FUNCTION|%1|Stopped").arg(fid);

    sendWebSocketMessage(wsMessage.toUtf8());
}

bool WebAccess::sendFile(QHttpResponse *response, QString filename, QString contentType)
{
    QFile resFile(filename);
    if (resFile.open(QIODevice::ReadOnly))
    {
        QByteArray resContent = resFile.readAll();
        //qDebug() << "Resource file length:" << resContent.length();
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

QString WebAccess::getWidgetBackgroundImage(VCWidget *widget)
{
    if (widget == NULL || widget->backgroundImage().isEmpty())
        return QString();

    QString str = QString("background-image: url(%1); ").arg(widget->backgroundImage());
    str += "background-position: center; ";
    str += "background-repeat: no-repeat; ";
    //str += "background-size: cover; ";

    return str;
}

QString WebAccess::getWidgetHTML(VCWidget *widget)
{
    QString str = "<div class=\"vcwidget\" style=\""
            "left: " + QString::number(widget->x()) + "px; "
            "top: " + QString::number(widget->y()) + "px; "
            "width: " + QString::number(widget->width()) + "px; "
            "height: " + QString::number(widget->height()) + "px; "
            "background-color: " + widget->backgroundColor().name() + ";" +
            getWidgetBackgroundImage(widget) + "\">\n";

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
          "background-color: " + frame->backgroundColor().name() + "; " +
          getWidgetBackgroundImage(frame) +
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
          "background-color: " + frame->backgroundColor().name() + "; " +
          getWidgetBackgroundImage(frame) +
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
            "color: " + btn->foregroundColor().name() + "; " +
            getWidgetBackgroundImage(btn) +
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
            "background-color: " + slider->backgroundColor().name() + ";" +
            getWidgetBackgroundImage(slider) + "\">\n";

    str += "<div style=\"height: 100%; display: flex; flex-direction: column; justify-content: space-between; \">";

    str += "<div id=\"slv" + slID + "\" class=\"vcslLabel\">" + slider->topLabelText() + "</div>\n";

    int mt = slider->invertedAppearance() ? -slider->height() + 50 : slider->height() - 50;
    int rotate = slider->invertedAppearance() ? 90 : 270;
    int min = 0;
    int max = 255;
    if (slider->sliderMode() == VCSlider::Level) {
        min = slider->levelLowLimit();
        max = slider->levelHighLimit();
    }

    str +=  "<input type=\"range\" class=\"vVertical\" "
            "id=\"" + slID + "\" "
            "oninput=\"slVchange(" + slID + ");\" ontouchmove=\"slVchange(" + slID + ");\" "
            "style=\"display: "+(slider->widgetStyle() == VCSlider::SliderWidgetStyle::WSlider ? "block" : "none") +"; "
            "width: " + QString::number(slider->height() - 50) + "px; "
            "margin-top: " + QString::number(mt) + "px; "
            "margin-left: " + QString::number(slider->width() / 2) + "px; "
            "--rotate: "+QString::number(rotate)+"\" "
            "min=\""+QString::number(min)+"\" max=\""+QString::number(max)+"\" "
            "step=\"1\" value=\"" + QString::number(slider->sliderValue()) + "\">\n";

    if (slider->widgetStyle() == VCSlider::SliderWidgetStyle::WKnob) {
        int shortSide = slider->width() > slider->height() ? slider->height() : slider->width();
        shortSide = shortSide - 50;
        float arcWidth = shortSide / 15;
        float pieWidth = shortSide - (arcWidth * 2);
        float knobWrapperWidth = pieWidth - arcWidth;
        float knobWidth = knobWrapperWidth - (arcWidth * 3);
        float spotWidth = knobWrapperWidth * 2 / 15;
        if (spotWidth < 6) spotWidth = 6;

        str += "<div class=\"pieWrapper\" data=\"" + slID + "\">";
        str += "<div class=\"pie\" id=\"pie" + slID + "\" style=\"--degValue:0;--pieWidth: "+QString::number(pieWidth)+"px;\">";
        str += "<div class=\"knobWrapper\" id=\"knobWrapper" + slID + "\" style=\"--knobWrapperWidth: "+QString::number(knobWrapperWidth)+"px;\">";
        str += "<div class=\"knob\" id=\"knob" + slID + "\" style=\"--knobWidth: "+QString::number(knobWidth)+"px;\">";
        str += "<div class=\"spot\" id=\"spot" + slID + "\" style=\"--spotWidth: "+QString::number(spotWidth)+"px;\"></div>";
        str += "</div>\n</div>\n</div>\n</div>\n";

        m_JScode += "maxVal[" + slID + "] = " + QString::number(max) + "; \n";
        m_JScode += "minVal[" + slID + "] = " + QString::number(min) + "; \n";
        m_JScode += "initVal[" + slID + "] = " + QString::number(slider->sliderValue()) + "; \n";
        m_JScode += "inverted[" + slID + "] = " + QString::number(slider->invertedAppearance()) + "; \n";
        m_JScode += "isDragging[" + slID + "] = false;\n";
    }

    str += "<div id=\"sln" + slID + "\" class=\"vcslLabel\">" +slider->caption() + "</div>";

    str += "</div>\n";
    str += "</div>\n";

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
            "background-color: " + label->backgroundColor().name() + "; " +
            getWidgetBackgroundImage(label) + "\">" +
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

void WebAccess::slotCueProgressStateChanged()
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == NULL)
        return;

    QString wsMessage = QString("%1|CUE_PROGRESS|%2|%3").arg(cue->id()).arg(cue->progressPercent()).arg(cue->progressText());

    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotCueShowSideFaderPanel()
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == NULL)
        return;

    QString wsMessage = QString("%1|CUE_SHOWPANEL|%2").arg(cue->id()).arg(cue->sideFaderButtonChecked());

    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotCueSideFaderValueChanged()
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == NULL)
        return;

    QString wsMessage = QString("%1|CUE_SIDECHANGE|%2|%3|%4|%5|%6|%7|%8")
                            .arg(cue->id())
                            .arg(cue->topPercentageValue())
                            .arg(cue->bottomPercentageValue())
                            .arg(cue->topStepValue())
                            .arg(cue->bottomStepValue())
                            .arg(cue->primaryTop())
                            .arg(cue->sideFaderValue())
                            .arg(cue->sideFaderMode() == VCCueList::FaderMode::Steps);

    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotCuePlaybackStateChanged()
{
    VCCueList *cue = qobject_cast<VCCueList *>(sender());
    if (cue == NULL)
        return;
    Chaser *chaser = cue->chaser();
    QString playbackButtonImage = "player_play.png";
    bool playbackButtonPaused = false;
    QString stopButtonImage = "player_stop.png";
    bool stopButtonPaused = false;

    if (chaser->isRunning()) {
        if (cue->playbackLayout() == VCCueList::PlayPauseStop) {
            if (chaser->isPaused()) {
                playbackButtonImage = "player_play.png";
                playbackButtonPaused = true;
            } else {
                playbackButtonImage  = "player_pause.png";
            }
        } else if (cue->playbackLayout() == VCCueList::PlayStopPause) {
            playbackButtonImage = "player_stop.png";
            stopButtonImage = "player_pause.png";
            if (chaser->isPaused()) {
                stopButtonPaused = true;
            }
        }
    } else {
        if (cue->playbackLayout() == VCCueList::PlayStopPause) {
            stopButtonImage = "player_pause.png";
        }
    }

    QString wsMessage = QString("%1|CUE_CHANGE|%2|%3|%4|%5")
                            .arg(cue->id())
                            .arg(playbackButtonImage)
                            .arg(QString::number(playbackButtonPaused))
                            .arg(stopButtonImage)
                            .arg(QString::number(stopButtonPaused));

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

    QString topStepBgColor = "inherit";
    QString bottomStepBgColor = "inherit";
    QString playbackButtonImage = "player_play.png";
    bool playbackButtonPaused = false;
    QString stopButtonImage = "player_stop.png";
    bool stopButtonPaused = false;

    Chaser *chaser = cue->chaser();
    Doc *doc = m_vc->getDoc();

    if (cue->primaryTop())
    {
        topStepBgColor = cue->topStepValue() != "" ? "#4E8DDE" : "inherit";
        bottomStepBgColor = cue->sideFaderMode() == VCCueList::FaderMode::Steps && cue->bottomStepValue() != "" ? "#4E8DDE" : cue->bottomStepValue() != "" ? "orange" : "inherit";
    }
    else
    {
        topStepBgColor = cue->topStepValue() != "" ? "orange" : "inherit";
        bottomStepBgColor = cue->sideFaderMode() == VCCueList::FaderMode::Steps || cue->bottomStepValue() != "" ? "#4E8DDE" : "inherit";
    }

    // fader mode
    if (cue->sideFaderMode() != VCCueList::FaderMode::None)
    {
        str += "<div style=\"display: flex; flex-direction: row; align-items: center; justify-content: space-between; \">";
        str += "<div id=\"fadePanel"+QString::number(cue->id())+"\" "
               "style=\"display: " + (cue->isSideFaderVisible() ? "block" : "none") + "; width: 45px; height: " +
               QString::number(cue->height() - 2) + "px;\">";
        if (cue->sideFaderMode() == VCCueList::FaderMode::Crossfade)
        {
            str += "<div style=\"position: relative;\">";
            str += "<div id=\"cueCTP"+QString::number(cue->id())+"\" class=\"vcslLabel\" style=\"top:0px;\">" +
                   cue->topPercentageValue() + "</div>\n";
            str += "<div id=\"cueCTS"+QString::number(cue->id())+"\" class=\"vcslLabel\" "
                   "style=\"top:25px; border: solid 1px #aaa; background-color: "+ topStepBgColor +" \">" +
                   cue->topStepValue() + "</div>\n";

            str += "<input type=\"range\" class=\"vVertical\" id=\"cueC"+QString::number(cue->id())+"\" "
                   "oninput=\"cueCVchange("+QString::number(cue->id())+");\" ontouchmove=\"cueCVchange("+QString::number(cue->id())+");\" "
                   "style=\"width: " + QString::number(cue->height() - 100) + "px; margin-top: " +
                   QString::number(cue->height() - 100) + "px; margin-left: 22px;\" ";
            str += "min=\"0\" max=\"100\" step=\"1\" value=\"" + QString::number(cue->sideFaderValue()) + "\">\n";

            str += "<div id=\"cueCBS"+QString::number(cue->id())+"\" class=\"vcslLabel\" "
                   "style=\"bottom:25px; border: solid 1px #aaa;  background-color: "+ bottomStepBgColor +"\">" +
                   cue->bottomStepValue() + "</div>\n";
            str += "<div id=\"cueCBP"+QString::number(cue->id())+"\" class=\"vcslLabel\" style=\"bottom:0px;\">" +
                   cue->bottomPercentageValue() + "</div>\n";
            str += "</div>";
        }
        if (cue->sideFaderMode() == VCCueList::FaderMode::Steps)
        {
            str += "<div style=\"position: relative;\">";
            str += "<div id=\"cueCTP"+QString::number(cue->id())+"\" class=\"vcslLabel\" style=\"top:0px;\">" +
                   cue->topPercentageValue() + "</div>\n";

            str += "<input type=\"range\" class=\"vVertical\" id=\"cueC"+QString::number(cue->id())+"\" "
                   "oninput=\"cueCVchange("+QString::number(cue->id())+");\" ontouchmove=\"cueCVchange("+QString::number(cue->id())+");\" "
                   "style=\"width: " + QString::number(cue->height() - 50) + "px; margin-top: " +
                   QString::number(cue->height() - 50) + "px; margin-left: 22px;\" ";
            str += "min=\"0\" max=\"255\" step=\"1\" value=\"" + QString::number(cue->sideFaderValue()) + "\">\n";

            str += "<div id=\"cueCBS"+QString::number(cue->id())+"\" class=\"vcslLabel\" style=\"bottom:25px; border: solid 1px #aaa; \">" +
                   cue->bottomStepValue() + "</div>\n";
            str += "</div>";
        }
        str += "</div>";
        m_JScode += "showPanel[" + QString::number(cue->id()) + "] = " + QString::number(cue->sideFaderButtonChecked()) + ";\n";
    }

    str += "<div style=\"width: 100%;\"><div style=\"width: 100%; height: " + QString::number(cue->height() - 54) + "px; overflow: scroll;\" >\n";

    str += "<table class=\"hovertable\" style=\"width: 100%;\">\n";
    str += "<tr><th>#</th><th>" + tr("Name") + "</th>";
    str += "<th>" + tr("Fade In") + "</th>";
    str += "<th>" + tr("Fade Out") + "</th>";
    str += "<th>" + tr("Duration") + "</th>";
    str += "<th>" + tr("Notes") + "</th></tr>\n";

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

    // progress bar
    str += "<div class=\"vccuelistProgress\">";
    str += "<div class=\"vccuelistProgressBar\" id=\"vccuelistPB"+QString::number(cue->id())+"\" style=\"width: " +
           QString::number(cue->progressPercent())+ "%; \"></div>";
    str += "<div class=\"vccuelistProgressVal\" id=\"vccuelistPV"+QString::number(cue->id())+"\">" +
           QString(cue->progressText()) + "</div>";
    str += "</div>";

    // play, stop, next, and preview buttons
    if (cue->sideFaderMode() != VCCueList::FaderMode::None)
    {
        str += "<div style=\"width: 100%; display: flex; flex-direction: row; align-items: center; justify-content: space-between; \">";
        str += "<a class=\"vccuelistFadeButton\" id=\"fade" + QString::number(cue->id()) + "\" ";
        str += "href=\"javascript:wsShowCrossfadePanel(" + QString::number(cue->id()) + ");\">\n";
        str += "<img src=\"slider.png\" width=\"27\"></a>\n";
    }
    str += "<div style=\"width: 100%; display: flex; flex-direction: row; align-items: center; justify-content: space-between; \">";

    if (chaser != NULL && chaser->isRunning())
    {
        if (cue->playbackLayout() == VCCueList::PlayPauseStop)
        {
            if (chaser->isPaused())
            {
                playbackButtonImage = "player_play.png";
                playbackButtonPaused = true;
            }
            else
            {
                playbackButtonImage  = "player_pause.png";
            }
        }
        else if (cue->playbackLayout() == VCCueList::PlayStopPause)
        {
            playbackButtonImage = "player_stop.png";
            stopButtonImage = "player_pause.png";
            if (chaser->isPaused())
                stopButtonPaused = true;
        }
    }
    else
    {
        if (cue->playbackLayout() == VCCueList::PlayStopPause)
            stopButtonImage = "player_pause.png";
    }

    str += "<a class=\"vccuelistButton"+QString(playbackButtonPaused ? " vccuelistButtonPaused" : "")+"\" id=\"play" + QString::number(cue->id()) + "\" ";
    str += "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'PLAY');\">\n";
    str += "<img src=\""+playbackButtonImage+"\" width=\"27\"></a>\n";

    str += "<a class=\"vccuelistButton"+QString(stopButtonPaused ? " vccuelistButtonPaused" : "")+"\" id=\"stop" + QString::number(cue->id()) + "\" ";
    str += "href=\"javascript:sendCueCmd(" + QString::number(cue->id()) + ", 'STOP');\">\n";
    str += "<img src=\""+stopButtonImage+"\" width=\"27\"></a>\n";

    str += "<a class=\"vccuelistButton\" href=\"javascript:sendCueCmd(";
    str += QString::number(cue->id()) + ", 'PREV');\">\n";
    str += "<img src=\"back.png\" width=\"27\"></a>\n";

    str += "<a class=\"vccuelistButton\" href=\"javascript:sendCueCmd(";
    str += QString::number(cue->id()) + ", 'NEXT');\" style=\"margin-right: 0px!important;\">\n";
    str += "<img src=\"forward.png\" width=\"27\"></a>\n";

    if (cue->sideFaderMode() != VCCueList::FaderMode::None) {
        str += "</div>\n";
    }
    str +="</div></div>";
    if (cue->sideFaderMode() != VCCueList::FaderMode::None) {
        str += "</div>\n";
    }

    str += "</div>\n";

    connect(cue, SIGNAL(stepChanged(int)),
            this, SLOT(slotCueIndexChanged(int)));
    connect(cue, SIGNAL(progressStateChanged()),
            this, SLOT(slotCueProgressStateChanged()));
    connect(cue, SIGNAL(sideFaderButtonChecked()),
            this, SLOT(slotSideFaderButtonChecked(bool)));
    connect(cue, SIGNAL(sideFaderButtonToggled()),
            this, SLOT(slotCueShowSideFaderPanel()));
    connect(cue, SIGNAL(sideFaderValueChanged()),
            this, SLOT(slotCueSideFaderValueChanged()));
    connect(cue, SIGNAL(playbackButtonClicked()),
            this, SLOT(slotCuePlaybackStateChanged()));
    connect(cue, SIGNAL(stopButtonClicked()),
            this, SLOT(slotCuePlaybackStateChanged()));
    connect(cue, SIGNAL(playbackStatusChanged()),
            this, SLOT(slotCuePlaybackStateChanged()));

    return str;
}

void WebAccess::slotClockTimeChanged(quint32 time)
{
    VCClock *clock = qobject_cast<VCClock *>(sender());
    if (clock == NULL)
        return;

    QString wsMessage = QString("%1|CLOCK|%2").arg(clock->id()).arg(time);
    sendWebSocketMessage(wsMessage.toUtf8());
}

QString WebAccess::getClockHTML(VCClock *clock)
{
    QString str = "<div class=\"vclabel-wrapper\" style=\""
            "left: " + QString::number(clock->x()) + "px; "
            "top: " + QString::number(clock->y()) + "px;\">\n";
    str +=  "<a id=\"" + QString::number(clock->id()) + "\" class=\"vclabel ";

    if (clock->clockType() == VCClock::Stopwatch ||
        clock->clockType() == VCClock::Countdown)
    {
        str += "vcclockcount\" href=\"javascript:controlWatch(";
        str += QString::number(clock->id()) + ", 'S')\" ";
        str += "oncontextmenu=\"javascript:controlWatch(";
        str += QString::number(clock->id()) + ", 'R'); return false;\"";
        connect(clock, SIGNAL(timeChanged(quint32)),
                this, SLOT(slotClockTimeChanged(quint32)));
    }
    else
    {
        str += "vcclock\" href=\"javascript:void(0)\"";
    }

    str +=  "style=\"width: " + QString::number(clock->width()) + "px; "
            "height: " + QString::number(clock->height()) + "px; "
            "color: " + clock->foregroundColor().name() + "; "
            "background-color: " + clock->backgroundColor().name() + ";" +
            getWidgetBackgroundImage(clock) + "\">";


    if (clock->clockType() == VCClock::Stopwatch)
    {
        str += "00:00:00";
    }
    else if (clock->clockType() == VCClock::Countdown)
    {
        QString curTime = QString("%1:%2:%3").arg(clock->getHours(), 2, 10, QChar('0'))
                                             .arg(clock->getMinutes(), 2, 10, QChar('0'))
                                             .arg(clock->getSeconds(), 2, 10, QChar('0'));
        str += curTime;
    }


    str += "</a></div>\n";

    return str;
}

void WebAccess::slotMatrixSliderValueChanged(int value)
{
    VCMatrix *matrix = qobject_cast<VCMatrix *>(sender());
    if (matrix == NULL)
        return;

    QString wsMessage = QString("%1|MATRIX_SLIDER|%2").arg(matrix->id()).arg(value);
    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotMatrixStartColorChanged()
{
    VCMatrix *matrix = qobject_cast<VCMatrix *>(sender());
    if (matrix == NULL)
        return;

    QString wsMessage = QString("%1|MATRIX_START_COLOR|%2").arg(matrix->id()).arg(matrix->startColor().name());
    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotMatrixEndColorChanged()
{
    VCMatrix *matrix = qobject_cast<VCMatrix *>(sender());
    if (matrix == NULL)
        return;

    QString wsMessage = QString("%1|MATRIX_END_COLOR|%2").arg(matrix->id()).arg(matrix->endColor().name());
    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotMatrixAnimationValueChanged(QString name)
{
    VCMatrix *matrix = qobject_cast<VCMatrix *>(sender());
    if (matrix == NULL)
        return;

    QString wsMessage = QString("%1|MATRIX_COMBO|%2").arg(matrix->id()).arg(name);
    sendWebSocketMessage(wsMessage.toUtf8());
}

void WebAccess::slotMatrixControlKnobValueChanged(int controlID, int value)
{
    VCMatrix *matrix = qobject_cast<VCMatrix *>(sender());
    if (matrix == NULL)
        return;

    QString wsMessage = QString("%1|MATRIX_KNOB|%2|%3").arg(matrix->id()).arg(controlID).arg(value);
    sendWebSocketMessage(wsMessage.toUtf8());
}

QString WebAccess::getMatrixHTML(VCMatrix *matrix)
{
    QString str = "<div id=\"" + QString::number(matrix->id()) + "\" "
                  "class=\"vcmatrix\" style=\"left: " + QString::number(matrix->x()) +
                  "px; top: " + QString::number(matrix->y()) + "px; width: " +
                  QString::number(matrix->width()) +
                  "px; height: " + QString::number(matrix->height()) + "px; "
                  "background-color: " + matrix->backgroundColor().name() + ";\">\n";

    str += "<div style=\"display: flex; flex-direction: row; align-items: center; width: 100%; height: 100%; \">";
    if (matrix->visibilityMask() & VCMatrix::Visibility::ShowSlider) {
        str +=  "<div style=\"height: 100%; width: 50px; \">";
        str +=  "<input type=\"range\" class=\"vVertical\" "
                "id=\"msl" + QString::number(matrix->id()) + "\" "
                "oninput=\"matrixSliderValueChange(" + QString::number(matrix->id()) + ");\" ontouchmove=\"matrixSliderValueChange(" + QString::number(matrix->id()) + ");\" "
                "style=\"width: " + QString::number(matrix->height() - 20) + "px; "
                "margin-top: " + QString::number(matrix->height() - 10) + "px; margin-left: 25px; \""
                "min=\"1\" max=\"255\" step=\"1\" value=\"" + QString::number(matrix->sliderValue()) + "\">\n";
        str +=  "</div>";
    }
    str +=  "<div style=\"display: flex; flex-direction: column; align-items: center; justify-content: space-around; height: 100%; width: 100%; margin: 8px; \">";
    if (matrix->visibilityMask() & VCMatrix::Visibility::ShowLabel) {
        str += "<div style=\"text-align: center; width: 100%; margin-top: 4px; margin-bottom: 4px; \">"+matrix->caption()+"</div>";
    }
    str += "<div style=\"display: flex; flex-direction: row; align-items: center; justify-content: space-around; width: 100%; margin-top: 4px; margin-bottom: 4px; \">";
    if (matrix->visibilityMask() & VCMatrix::Visibility::ShowStartColorButton) {

        str += "<input type=\"color\" id=\"msc"+QString::number(matrix->id())+"\" class=\"vMatrix\" value=\""+(matrix->startColor().name())+"\" "
               "oninput=\"matrixStartColorChange(" + QString::number(matrix->id()) + ");\" ontouchmove=\"matrixStartColorChange(" + QString::number(matrix->id()) + ");\" "
               " />";
    }
    if (matrix->visibilityMask() & VCMatrix::Visibility::ShowEndColorButton) {
        str += "<input type=\"color\" id=\"mec"+QString::number(matrix->id())+"\" class=\"vMatrix\" value=\""+(matrix->endColor().name())+"\" "
               "oninput=\"matrixEndColorChange(" + QString::number(matrix->id()) + ");\" ontouchmove=\"matrixEndColorChange(" + QString::number(matrix->id()) + ");\" "
               " />";
    }
    str += "</div>";
    if (matrix->visibilityMask() & VCMatrix::Visibility::ShowPresetCombo) {
        QStringList list = RGBAlgorithm::algorithms(m_doc);

        str += "<div style=\"width: 100%; margin-top: 4px; margin-bottom: 4px; \"><select class=\"matrixSelect\" id=\"mcb" + QString::number(matrix->id()) + "\" onchange=\"matrixComboChanged("+QString::number(matrix->id())+");\">";
        for(int i = 0; i < list.length(); i++) {
            str += "<option value=\""+list[i]+"\" "+(list[i] == matrix->animationValue() ? "selected" : "" )+" >"+list[i]+"</option>";
        }
        str += "</select></div>";
    }
    QList<VCMatrixControl *> customControls = matrix->customControls();
    if (customControls.length() > 0) {
        m_JScode += "matrixID = "+QString::number(matrix->id())+"; \n";
        str += "<div style=\"display: flex; flex-direction: row; flex-wrap: wrap; align-content: flex-start; width: 100%; height: 100%; margin-top: 4px; margin-bottom: 4px; \">";
        for (int i = 0; i < customControls.length(); i++) {
            VCMatrixControl *control = customControls[i];
            if (control->m_type == VCMatrixControl::StartColor) {
                str += "<div class=\"pushButton\" style=\"width: 32px; height: 32px; "
                       "background-color: "+(control->m_color.name())+"; margin-right: 4px; margin-bottom: 4px; \" "
                       "onclick=\"wcMatrixPushButtonClicked("+(QString::number(control->m_id))+")\">S</div>";
            } else if (control->m_type == VCMatrixControl::EndColor) {
                str += "<div class=\"pushButton\" style=\"width: 32px; height: 32px; "
                       "background-color: "+(control->m_color.name())+"; margin-right: 4px; margin-bottom: 4px; \" "
                       "onclick=\"wcMatrixPushButtonClicked("+(QString::number(control->m_id))+")\">E</div>";
            } else if (control->m_type == VCMatrixControl::ResetEndColor) {
                QString btnLabel = tr("End Color Reset");
                str += "<div class=\"pushButton\" style=\"width: 66px; justify-content: flex-start!important; height: 32px; "
                       "background-color: #BBBBBB; margin-right: 4px; margin-bottom: 4px; \" "
                       "onclick=\"wcMatrixPushButtonClicked("+(QString::number(control->m_id))+")\">"+btnLabel+"</div>";
            } else if (control->m_type == VCMatrixControl::Animation || control->m_type == VCMatrixControl::Text) {
                QString btnLabel = control->m_resource;
                if (!control->m_properties.isEmpty())
                {
                        btnLabel += " (";
                        QHashIterator<QString, QString> it(control->m_properties);
                        while(it.hasNext())
                        {
                            it.next();
                            btnLabel += it.value();
                            if (it.hasNext())
                                btnLabel += ",";
                        }
                        btnLabel += ")";
                }
                str += "<div class=\"pushButton\" style=\"max-width: 66px; justify-content: flex-start!important; height: 32px; "
                       "background-color: #BBBBBB; margin-right: 4px; margin-bottom: 4px; \" "
                       "onclick=\"wcMatrixPushButtonClicked("+(QString::number(control->m_id))+")\">"+btnLabel+"</div>";
            } else if (control->m_type == VCMatrixControl::StartColorKnob || control->m_type == VCMatrixControl::EndColorKnob) {
                KnobWidget *knob = qobject_cast<KnobWidget*>(matrix->getWidget(control));
                QString slID = QString::number(control->m_id);
                QColor color = control->m_type == VCMatrixControl::StartColorKnob ? control->m_color : control->m_color.darker(250);

                str += "<div class=\"mpieWrapper\" data=\"" + slID + "\" style=\"margin-right: 4px; margin-bottom: 4px; \">";
                str += "<div class=\"mpie\" id=\"mpie" + slID + "\" style=\"--degValue:0; \">";
                str += "<div class=\"mknobWrapper\" id=\"mknobWrapper" + slID + "\">";
                str += "<div class=\"mknob\" id=\"mknob" + slID + "\" style=\"background-color: "+(color.name())+";\">";
                str += "<div class=\"mspot\" id=\"mspot" + slID + "\"></div>";
                str += "</div>\n</div>\n</div>\n</div>\n";

                m_JScode += "m_initVal[" + slID + "] = "+QString::number(knob->value())+"; \n";
                m_JScode += "m_isDragging[" + slID + "] = false;\n";
                connect(matrix, SIGNAL(matrixControlKnobValueChanged(int, int)),
                        this, SLOT(slotMatrixControlKnobValueChanged(int, int)));

            }
        }
        str += "</div>";
    }
    str += "</div>";

    str += "</div>";
    str += "</div>\n";

    connect(matrix, SIGNAL(sliderValueChanged(int)),
            this, SLOT(slotMatrixSliderValueChanged(int)));
    connect(matrix, SIGNAL(startColorChanged()),
            this, SLOT(slotMatrixStartColorChanged()));
    connect(matrix, SIGNAL(endColorChanged()),
            this, SLOT(slotMatrixEndColorChanged()));
    connect(matrix, SIGNAL(animationValueChanged(QString)),
            this, SLOT(slotMatrixAnimationValueChanged(QString)));

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
                pg += " style=\"visibility: inherit;\"";
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
            case VCWidget::ClockWidget:
                str = getClockHTML(qobject_cast<VCClock *>(widget));
            break;
            case VCWidget::AnimationWidget:
                str = getMatrixHTML(qobject_cast<VCMatrix *>(widget));
            break;
            default:
                str = getWidgetHTML(widget);
            break;
        }
        if (lframe->multipageMode() == true && pagesNum > 0)
        {
            if (widget->page() < pagesHTML.count())
            {
                pagesHTML[widget->page()] += str;
                if (restoreDisable)
                    widget->setEnabled(false);
            }
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
