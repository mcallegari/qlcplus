/*
  Q Light Controller Plus
  webaccessbase.cpp

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
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QProcess>
#include <QSettings>

#include "webaccessbase.h"

#include "webaccessconfiguration.h"
#include "webaccesssimpledesk.h"
#include "webaccessnetwork.h"
#include "commonjscss.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "doc.h"
#include "inputpatch.h"
#include "audiocapture.h"
#include "audiorenderer.h"
#include "qhttpserver.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"
#include "qhttpconnection.h"

#define DEFAULT_PORT_NUMBER    9999
#define AUTOSTART_PROJECT_NAME "autostart.qxw"

WebAccessBase::WebAccessBase(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                             int portNumber, bool enableAuth, const QString &passwdFile,
                             QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_vc(vcInstance)
    , m_sd(sdInstance)
    , m_auth(nullptr)
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    , m_netConfig(nullptr)
#endif
    , m_httpServer(nullptr)
    , m_pendingProjectLoaded(false)
{
    Q_ASSERT(m_doc != nullptr);
    Q_ASSERT(m_vc != nullptr);

    if (enableAuth)
    {
        m_auth = new WebAccessAuth(QString("QLC+ web access"));
        m_auth->loadPasswordsFile(passwdFile);
    }

    m_httpServer = new QHttpServer(this);
    connect(m_httpServer, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
            this, SLOT(slotHandleHTTPRequest(QHttpRequest*, QHttpResponse*)));
    connect(m_httpServer, SIGNAL(webSocketDataReady(QHttpConnection*,QString)),
            this, SLOT(slotHandleWebSocketRequest(QHttpConnection*,QString)));
    connect(m_httpServer, SIGNAL(webSocketConnectionClose(QHttpConnection*)),
            this, SLOT(slotHandleWebSocketClose(QHttpConnection*)));

    m_httpServer->listen(QHostAddress::Any, portNumber ? portNumber : DEFAULT_PORT_NUMBER);

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    m_netConfig = new WebAccessNetwork();
#endif

    connect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)),
            this, SLOT(slotFunctionStarted(quint32)));
    connect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)),
            this, SLOT(slotFunctionStopped(quint32)));
}

WebAccessBase::~WebAccessBase()
{
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    delete m_netConfig;
#endif
    foreach (QHttpConnection *conn, m_webSocketsList)
        delete conn;

    delete m_auth;
}

bool WebAccessBase::sendFile(QHttpResponse *response, QString filename, QString contentType) const
{
    QFile resFile(filename);
#if defined(WIN32) || defined(Q_OS_WIN)
    if (!resFile.exists())
    {
        filename.remove(0, 1);
        filename.replace("//", ":\\");
        filename.replace('/', '\\');
        resFile.setFileName(filename);
    }
#endif
    if (resFile.open(QIODevice::ReadOnly))
    {
        QByteArray resContent = resFile.readAll();
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

void WebAccessBase::sendWebSocketMessage(const QString &message) const
{
    foreach (QHttpConnection *conn, m_webSocketsList)
        conn->webSocketWrite(message);
}

QString WebAccessBase::webFilePath(const QString &relativePath) const
{
    QString basePath = QLCFile::systemDirectory(WEBFILESDIR).path();
    return QString("%1%2%3").arg(basePath).arg(QDir::separator()).arg(relativePath);
}

bool WebAccessBase::servePng(QHttpResponse *resp, const QString &reqUrl) const
{
    QString localFilePath = QString(":%1").arg(reqUrl);
    QFile resFile(localFilePath);
    if (!resFile.exists())
    {
        localFilePath = reqUrl;
        resFile.setFileName(localFilePath);
        if (!resFile.exists())
            localFilePath = webFilePath(reqUrl.mid(1));
    }

    return sendFile(resp, localFilePath, "image/png");
}

bool WebAccessBase::serveWebFile(QHttpResponse *resp, const QString &reqUrl, const QString &contentType) const
{
    return sendFile(resp, webFilePath(reqUrl.mid(1)), contentType);
}

bool WebAccessBase::authenticateRequest(const QHttpRequest *req, QHttpResponse *resp, WebAccessUser &user) const
{
    if (!m_auth)
        return true;

    user = m_auth->authenticateRequest(req, resp);
    if (user.level < LOGGED_IN_LEVEL)
    {
        m_auth->sendUnauthorizedResponse(resp);
        return false;
    }

    return true;
}

bool WebAccessBase::acceptWebSocket(QHttpResponse *resp, const WebAccessUser &user)
{
    if (!resp)
        return false;

    QHttpConnection *conn = resp->enableWebSocket();
    if (conn != nullptr)
    {
        conn->userData = new WebAccessUser(user);
        m_webSocketsList.append(conn);
        return true;
    }

    return false;
}

QByteArray WebAccessBase::extractProjectXml(const QHttpRequest *req) const
{
    if (req == nullptr)
        return QByteArray();

    QByteArray projectXML = req->body();
    projectXML.remove(0, projectXML.indexOf("\n\r\n") + 3);
    projectXML.truncate(projectXML.lastIndexOf("\n\r\n"));
    return projectXML;
}

void WebAccessBase::sendProjectLoadingResponse(QHttpResponse *resp) const
{
    if (resp == nullptr)
        return;

    QByteArray postReply =
            QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
            "<script>\n" PROJECT_LOADED_JS
            "</script></head><body style=\"background-color: #45484d;\">"
            "<div style=\"position: absolute; width: 100%; height: 30px; top: 50%; background-color: #888888;"
            "text-align: center; font:bold 24px/1.2em sans-serif;\">"
            + tr("Loading project...") +
            "</div></body></html>").toUtf8();

    resp->setHeader("Content-Type", "text/html");
    resp->setHeader("Content-Length", QString::number(postReply.size()));
    resp->writeHead(200);
    resp->end(postReply);
}

void WebAccessBase::sendNotFound(QHttpResponse *resp) const
{
    if (resp == nullptr)
        return;

    resp->writeHead(404);
    resp->setHeader("Content-Type", "text/plain");
    resp->setHeader("Content-Length", "14");
    resp->end(QByteArray("404 Not found"));
}

void WebAccessBase::sendHtmlResponse(QHttpResponse *resp, const QString &content) const
{
    if (resp == nullptr)
        return;

    QByteArray contentArray = content.toUtf8();
    resp->setHeader("Content-Type", "text/html");
    resp->setHeader("Content-Length", QString::number(contentArray.size()));
    resp->writeHead(200);
    resp->end(contentArray);
}

bool WebAccessBase::requireAuthLevel(QHttpResponse *resp, const WebAccessUser &user, WebAccessUserLevel level) const
{
    if (m_auth && user.level < level)
    {
        m_auth->sendUnauthorizedResponse(resp);
        return false;
    }
    return true;
}

WebAccessBase::CommonRequestResult WebAccessBase::handleCommonHTTPRequest(const QHttpRequest *req, QHttpResponse *resp,
                                                                          const WebAccessUser &user,
                                                                          const QString &reqUrl,
                                                                          QString &content)
{
    if (reqUrl == "/qlcplusWS")
    {
        if (acceptWebSocket(resp, user))
            return CommonRequestResult::Handled;
        return CommonRequestResult::ContentReady;
    }
    else if (reqUrl == "/loadProject")
    {
        if (!requireAuthLevel(resp, user, SUPER_ADMIN_LEVEL))
            return CommonRequestResult::Handled;
        QByteArray projectXML = extractProjectXml(req);

        qDebug() << "Workspace XML received. Content-Length:" << req->headers().value("content-length") << projectXML.size();
        sendProjectLoadingResponse(resp);

        m_pendingProjectLoaded = false;

        handleProjectLoad(projectXML);

        return CommonRequestResult::Handled;
    }
    else if (reqUrl == "/loadFixture")
    {
        if (!requireAuthLevel(resp, user, SUPER_ADMIN_LEVEL))
            return CommonRequestResult::Handled;
        QByteArray fixtureXML = req->body();
        int fnamePos = fixtureXML.indexOf("filename=") + 10;
        QString fxName = fixtureXML.mid(fnamePos, fixtureXML.indexOf("\"", fnamePos) - fnamePos);

        fixtureXML.remove(0, fixtureXML.indexOf("\n\r\n") + 3);
        fixtureXML.truncate(fixtureXML.lastIndexOf("\n\r\n"));

        if (!storeFixtureDefinition(fxName, fixtureXML))
            return CommonRequestResult::Handled;

        QByteArray postReply =
                QString("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\n"
                        "<script>\n"
                        " alert(\"" + tr("Fixture stored and loaded") + "\");"
                        " window.location = \"/config\"\n"
                        "</script></head></html>").toUtf8();

        resp->setHeader("Content-Type", "text/html");
        resp->setHeader("Content-Length", QString::number(postReply.size()));
        resp->writeHead(200);
        resp->end(postReply);

        return CommonRequestResult::Handled;
    }
    else if (reqUrl == "/config")
    {
        if (!requireAuthLevel(resp, user, SUPER_ADMIN_LEVEL))
            return CommonRequestResult::Handled;
        content = WebAccessConfiguration::getHTML(m_doc, m_auth);
        return CommonRequestResult::ContentReady;
    }
    else if (reqUrl == "/simpleDesk")
    {
        if (!requireAuthLevel(resp, user, SIMPLE_DESK_AND_VC_LEVEL))
            return CommonRequestResult::Handled;
        content = WebAccessSimpleDesk::getHTML(m_doc, m_sd);
        return CommonRequestResult::ContentReady;
    }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    else if (reqUrl == "/system")
    {
        if (!requireAuthLevel(resp, user, SUPER_ADMIN_LEVEL))
            return CommonRequestResult::Handled;
        content = m_netConfig->getHTML();
        return CommonRequestResult::ContentReady;
    }
#endif
    else if (reqUrl.endsWith(".png"))
    {
        if (servePng(resp, reqUrl))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl.endsWith(".jpg") || reqUrl.endsWith(".jpeg"))
    {
        if (sendFile(resp, reqUrl, "image/jpg"))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl.endsWith(".bmp"))
    {
        if (sendFile(resp, reqUrl, "image/bmp"))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl.endsWith(".svg"))
    {
        if (sendFile(resp, reqUrl, "image/svg+xml"))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl.endsWith(".ico"))
    {
        if (serveWebFile(resp, reqUrl, "image/x-icon"))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl.endsWith(".css"))
    {
        if (serveWebFile(resp, reqUrl, "text/css"))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl.endsWith(".js"))
    {
        if (serveWebFile(resp, reqUrl, "text/javascript"))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl.endsWith(".html"))
    {
        if (serveWebFile(resp, reqUrl, "text/html"))
            return CommonRequestResult::Handled;
    }
    else if (reqUrl != "/")
    {
        sendNotFound(resp);
        return CommonRequestResult::Handled;
    }

    return CommonRequestResult::NotHandled;
}

bool WebAccessBase::handleCommonWebSocketCommand(QHttpConnection *conn, const WebAccessUser *user,
                                                 const QStringList &cmdList, const QString &logTag,
                                                 bool logWarning)
{
    if (cmdList.isEmpty())
        return false;

    auto logUnsupported = [&](const QString &cmd) {
        if (logWarning)
            qWarning() << logTag << "Command" << cmd << "not supported!";
        else
            qDebug() << logTag << "Command" << cmd << "not supported!";
    };

    if (cmdList[0] == "QLC+IO")
    {
        if (m_auth && user && user->level < SUPER_ADMIN_LEVEL)
            return true;

        if (cmdList.count() < 3)
            return true;

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
            if (inPatch != nullptr)
            {
                m_doc->inputOutputMap()->setInputPatch(universe, inPatch->pluginName(), "", inPatch->input(), cmdList[3]);
                m_doc->inputOutputMap()->saveDefaults();
            }
        }
        else if (cmdList[1] == "PASSTHROUGH")
        {
            quint32 uniIdx = cmdList[2].toUInt();
            m_doc->inputOutputMap()->setUniversePassthrough(uniIdx, cmdList[3] == "true");
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
        {
            logUnsupported(cmdList[1]);
        }

        return true;
    }
    else if (cmdList[0] == "QLC+AUTH")
    {
        if (!m_auth)
            return true;

        if (user && user->level < SUPER_ADMIN_LEVEL)
            return true;

        if (cmdList.count() < 2)
            return true;

        if (cmdList.at(1) == "ADD_USER")
        {
            QString username = cmdList.at(2);
            QString password = cmdList.at(3);
            int level = cmdList.at(4).toInt();
            if (username.isEmpty() || password.isEmpty())
            {
                QString wsMessage = QString("ALERT|" + tr("Username and password are required fields."));
                if (conn)
                    conn->webSocketWrite(wsMessage);
                return true;
            }
            if (level <= 0)
            {
                QString wsMessage = QString("ALERT|" + tr("User level has to be a positive integer."));
                if (conn)
                    conn->webSocketWrite(wsMessage);
                return true;
            }

            m_auth->addUser(username, password, (WebAccessUserLevel)level);
        }
        else if (cmdList.at(1) == "DEL_USER")
        {
            QString username = cmdList.at(2);
            if (!username.isEmpty())
                m_auth->deleteUser(username);
        }
        else if (cmdList.at(1) == "SET_USER_LEVEL")
        {
            QString username = cmdList.at(2);
            int level = cmdList.at(3).toInt();
            if (username.isEmpty())
            {
                QString wsMessage = QString("ALERT|" + tr("Username is required."));
                if (conn)
                    conn->webSocketWrite(wsMessage);
                return true;
            }
            if (level <= 0)
            {
                QString wsMessage = QString("ALERT|" + tr("User level has to be a positive integer."));
                if (conn)
                    conn->webSocketWrite(wsMessage);
                return true;
            }

            m_auth->setUserLevel(username, (WebAccessUserLevel)level);
        }
        else
        {
            logUnsupported(cmdList[1]);
        }

        if (!m_auth->savePasswordsFile())
        {
            QString wsMessage = QString("ALERT|" + tr("Error while saving passwords file."));
            if (conn)
                conn->webSocketWrite(wsMessage);
            return true;
        }

        return true;
    }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    else if (cmdList[0] == "QLC+SYS")
    {
        if (m_auth && user && user->level < SUPER_ADMIN_LEVEL)
            return true;

        if (cmdList.count() < 2)
            return true;

        if (cmdList.at(1) == "NETWORK")
        {
            QString wsMessage;
            if (m_netConfig->updateNetworkSettings(cmdList))
                wsMessage = QString("ALERT|" + tr("Network configuration changed. Reboot to apply the changes."));
            else
                wsMessage = QString("ALERT|" + tr("An error occurred while updating the network configuration."));

            if (conn)
                conn->webSocketWrite(wsMessage);
            return true;
        }
        else if (cmdList.at(1) == "HOTSPOT")
        {
            QString wsMessage;
            if (cmdList.count() < 5)
                return true;

            bool enable = cmdList.at(2).toInt();

            if (enable)
            {
                if (m_netConfig->createWiFiHotspot(cmdList.at(3), cmdList.at(4)))
                    wsMessage = QString("ALERT|" + tr("Wi-Fi hotspot successfully activated."));
                else
                    wsMessage = QString("ALERT|" + tr("An error occurred while creating a Wi-Fi hotspot."));
            }
            else
            {
                m_netConfig->deleteWiFiHotspot();
                wsMessage = QString("ALERT|" + tr("Wi-Fi hotspot successfully deactivated."));
            }

            if (conn)
                conn->webSocketWrite(wsMessage);
            return true;
        }
        else if (cmdList.at(1) == "AUTOSTART")
        {
            if (cmdList.count() < 3)
                return true;

            QString asName = QString("%1/%2/%3").arg(getenv("HOME")).arg(USERQLCPLUSDIR).arg(AUTOSTART_PROJECT_NAME);
            if (cmdList.at(2) == "none")
                QFile::remove(asName);
            else
                handleAutostartProject(asName);
            QString wsMessage = QString("ALERT|" + tr("Autostart configuration changed"));
            if (conn)
                conn->webSocketWrite(wsMessage);
            return true;
        }
        else if (cmdList.at(1) == "REBOOT")
        {
            QProcess *rebootProcess = new QProcess();
            rebootProcess->start("sudo", QStringList() << "shutdown" << "-r" << "now");
            return true;
        }
        else if (cmdList.at(1) == "HALT")
        {
            QProcess *haltProcess = new QProcess();
            haltProcess->start("sudo", QStringList() << "shutdown" << "-h" << "now");
            return true;
        }
    }
#endif

    return false;
}

void WebAccessBase::handleAutostartProject(const QString &path)
{
    Q_UNUSED(path)
}
