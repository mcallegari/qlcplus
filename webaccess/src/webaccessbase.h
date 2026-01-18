/*
  Q Light Controller Plus
  webaccessbase.h

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

#ifndef WEBACCESSBASE_H
#define WEBACCESSBASE_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>

#include "webaccessauth.h"

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
class WebAccessNetwork;
#endif

class VirtualConsole;
class SimpleDesk;
class Doc;

class QHttpServer;
class QHttpRequest;
class QHttpResponse;
class QHttpConnection;

class WebAccessBase : public QObject
{
    Q_OBJECT
public:
    virtual ~WebAccessBase();

protected:
    explicit WebAccessBase(Doc *doc, VirtualConsole *vcInstance, SimpleDesk *sdInstance,
                           int portNumber, bool enableAuth, const QString &passwdFile,
                           QObject *parent = nullptr);

    bool sendFile(QHttpResponse *response, QString filename, QString contentType) const;
    void sendWebSocketMessage(const QString &message);
    virtual QString webFilePath(const QString &relativePath) const;
    bool servePng(QHttpResponse *resp, const QString &reqUrl) const;
    bool serveWebFile(QHttpResponse *resp, const QString &reqUrl, const QString &contentType) const;
    bool authenticateRequest(QHttpRequest *req, QHttpResponse *resp, WebAccessUser &user);
    bool acceptWebSocket(QHttpResponse *resp, const WebAccessUser &user);
    QByteArray extractProjectXml(const QHttpRequest *req) const;
    void sendProjectLoadingResponse(QHttpResponse *resp) const;
    void sendNotFound(QHttpResponse *resp) const;
    bool requireAuthLevel(QHttpResponse *resp, const WebAccessUser &user, WebAccessUserLevel level) const;
    bool handleCommonWebSocketCommand(QHttpConnection *conn, WebAccessUser *user,
                                      const QStringList &cmdList, const QString &logTag,
                                      bool logWarning);
    virtual void handleAutostartProject(const QString &path);

protected slots:
    virtual void slotHandleHTTPRequest(QHttpRequest *req, QHttpResponse *resp) = 0;
    virtual void slotHandleWebSocketRequest(QHttpConnection *conn, QString data) = 0;
    virtual void slotHandleWebSocketClose(QHttpConnection *conn) = 0;
    virtual void slotFunctionStarted(quint32 fid) = 0;
    virtual void slotFunctionStopped(quint32 fid) = 0;

protected:
    Doc *m_doc;
    VirtualConsole *m_vc;
    SimpleDesk *m_sd;
    WebAccessAuth *m_auth;
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    WebAccessNetwork *m_netConfig;
#endif
    QHttpServer *m_httpServer;
    QList<QHttpConnection *> m_webSocketsList;
    bool m_pendingProjectLoaded;
};

#endif // WEBACCESSBASE_H
