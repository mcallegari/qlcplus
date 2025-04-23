/*
 * Copyright 2011-2014 Nikhil Marathe <nsm.nikhil@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "qhttpconnection.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QDebug>

#include "http_parser.h"
#include "qhttprequest.h"
#include "qhttpresponse.h"

/// @cond nodoc

QHttpConnection::QHttpConnection(QTcpSocket *socket, QObject *parent)
    : QObject(parent),
      m_socket(socket),
      m_parser(0),
      m_parserSettings(0),
      m_request(0),
      m_transmitLen(0),
      m_transmitPos(0),
      m_postPending(false),
      m_isWebSocket(false),
      m_pollTimer(NULL)
{
    m_parser = (http_parser *)malloc(sizeof(http_parser));
    http_parser_init(m_parser, HTTP_REQUEST);

    m_parserSettings = new http_parser_settings();
    m_parserSettings->on_message_begin = MessageBegin;
    m_parserSettings->on_url = Url;
    m_parserSettings->on_header_field = HeaderField;
    m_parserSettings->on_header_value = HeaderValue;
    m_parserSettings->on_headers_complete = HeadersComplete;
    m_parserSettings->on_body = Body;
    m_parserSettings->on_message_complete = MessageComplete;

    m_parser->data = this;

    connect(socket, SIGNAL(readyRead()), this, SLOT(parseRequest()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateWriteCount(qint64)));

    qDebug() << "HTTP connection created!";
}

QHttpConnection::~QHttpConnection()
{
    delete m_socket;
    m_socket = 0;

    free(m_parser);
    m_parser = 0;

    delete m_parserSettings;
    m_parserSettings = 0;

    qDebug() << "HTTP connection destroyed!";
}

void QHttpConnection::socketDisconnected()
{
    deleteLater();
    invalidateRequest();
}

void QHttpConnection::invalidateRequest()
{
    if (m_request && !m_request->successful())
    {
        Q_EMIT m_request->end();
    }

    m_request = NULL;
}

void QHttpConnection::updateWriteCount(qint64 count)
{
    if (m_isWebSocket == false)
    {
        //Q_ASSERT(m_transmitPos + count <= m_transmitLen);
        if (m_transmitPos + count > m_transmitLen)
            return;

        m_transmitPos += count;

        if (m_transmitPos == m_transmitLen)
        {
            m_transmitLen = 0;
            m_transmitPos = 0;
            Q_EMIT allBytesWritten();
        }
    }
}

void QHttpConnection::parseRequest()
{
    Q_ASSERT(m_parser);

    while (m_socket->bytesAvailable())
    {
        m_socket->startTransaction();
        QByteArray data = m_socket->readAll();
        http_parser_execute(m_parser, m_parserSettings, data.constData(), data.size());

        if (!m_isWebSocket)
        {
            m_socket->commitTransaction();
        }
        else
        {
            // interrupt here otherwise this will loop forever
            // since the transaction is rolled back
            // on websocket handover
            break;
        }
    }
}

void QHttpConnection::write(const QByteArray &data)
{
    m_socket->write(data);
    m_transmitLen += data.size();
}

void QHttpConnection::flush()
{
    m_socket->flush();
}

void QHttpConnection::waitForBytesWritten()
{
    m_socket->waitForBytesWritten();
}

void QHttpConnection::responseDone()
{
    QHttpResponse *response = qobject_cast<QHttpResponse *>(QObject::sender());
    if (response->m_last && m_isWebSocket == false)
        m_socket->disconnectFromHost();
}

/* URL Utilities */
#define HAS_URL_FIELD(info, field) (info.field_set &(1 << (field)))

#define GET_FIELD(data, info, field)                                                               \
    QString::fromLatin1(data + info.field_data[field].off, info.field_data[field].len)

#define CHECK_AND_GET_FIELD(data, info, field)                                                     \
    (HAS_URL_FIELD(info, field) ? GET_FIELD(data, info, field) : QString())

QUrl createUrl(const char *urlData, const http_parser_url &urlInfo)
{
    QUrl url;
    url.setScheme(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_SCHEMA));
    url.setHost(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_HOST));
    // Port is dealt with separately since it is available as an integer.
    url.setPath(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_PATH));
    url.setQuery(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_QUERY));
    url.setFragment(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_FRAGMENT));
    url.setUserInfo(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_USERINFO));

    if (HAS_URL_FIELD(urlInfo, UF_PORT))
        url.setPort(urlInfo.port);

    return url;
}

#undef CHECK_AND_SET_FIELD
#undef GET_FIELD
#undef HAS_URL_FIELD

/********************
 * Static Callbacks *
 *******************/

int QHttpConnection::MessageBegin(http_parser *parser)
{
    QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
    theConnection->m_currentHeaders.clear();
    theConnection->m_currentUrl.clear();
    theConnection->m_currentUrl.reserve(128);

    // The QHttpRequest should not be parented to this, since it's memory
    // management is the responsibility of the user of the library.
    theConnection->m_request = new QHttpRequest(theConnection);

    // Invalidate the request when it is deleted to prevent keep-alive requests
    // from calling a signal on a deleted object.
    connect(theConnection->m_request, SIGNAL(destroyed(QObject*)), theConnection, SLOT(invalidateRequest()));

    return 0;
}

int QHttpConnection::HeadersComplete(http_parser *parser)
{
    QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
    Q_ASSERT(theConnection->m_request);

    /** set method **/
    theConnection->m_request->setMethod(static_cast<QHttpRequest::HttpMethod>(parser->method));

    /** set version **/
    theConnection->m_request->setVersion(
        QString("%1.%2").arg(parser->http_major).arg(parser->http_minor));

    /** get parsed url **/
    struct http_parser_url urlInfo;
    int r = http_parser_parse_url(theConnection->m_currentUrl.constData(),
                                  theConnection->m_currentUrl.size(),
                                  parser->method == HTTP_CONNECT, &urlInfo);
    Q_ASSERT(r == 0);
    Q_UNUSED(r);

    theConnection->m_request->setUrl(createUrl(theConnection->m_currentUrl.constData(), urlInfo));

    // Insert last remaining header
    theConnection->m_currentHeaders[theConnection->m_currentHeaderField.toLower()] =
        theConnection->m_currentHeaderValue;
    theConnection->m_request->setHeaders(theConnection->m_currentHeaders);

    /** set client information **/
    theConnection->m_request->m_remoteAddress = theConnection->m_socket->peerAddress().toString();
    theConnection->m_request->m_remotePort = theConnection->m_socket->peerPort();

    QHttpResponse *response = new QHttpResponse(theConnection);
    if (parser->http_major < 1 || parser->http_minor < 1)
        response->m_keepAlive = false;

    connect(theConnection, SIGNAL(destroyed()), response, SLOT(connectionClosed()));
    connect(response, SIGNAL(done()), theConnection, SLOT(responseDone()));

    if (theConnection->m_request->method() == QHttpRequest::HTTP_POST)
    {
        // on a POST, do not emit the newRequest signal until the transfer has
        // been completed
        theConnection->m_postPending = true;
    }
    else
    {
        // we are good to go!
        Q_EMIT theConnection->newRequest(theConnection->m_request, response);
    }

    return 0;
}

int QHttpConnection::MessageComplete(http_parser *parser)
{
    // TODO: do cleanup and prepare for next request
    QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
    Q_ASSERT(theConnection->m_request);

    theConnection->m_request->setSuccessful(true);
    Q_EMIT theConnection->m_request->end();
    if (theConnection->m_postPending == true)
    {
        theConnection->m_postPending = false;
        QHttpResponse *response = new QHttpResponse(theConnection);
        Q_EMIT theConnection->newRequest(theConnection->m_request, response);
    }
    return 0;
}

int QHttpConnection::Url(http_parser *parser, const char *at, size_t length)
{
    QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
    Q_ASSERT(theConnection->m_request);

    theConnection->m_currentUrl.append(at, length);
    return 0;
}

int QHttpConnection::HeaderField(http_parser *parser, const char *at, size_t length)
{
    QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
    Q_ASSERT(theConnection->m_request);

    // insert the header we parsed previously
    // into the header map
    if (!theConnection->m_currentHeaderField.isEmpty() &&
        !theConnection->m_currentHeaderValue.isEmpty()) {
        // header names are always lower-cased
        theConnection->m_currentHeaders[theConnection->m_currentHeaderField.toLower()] =
            theConnection->m_currentHeaderValue;
        // clear header value. this sets up a nice
        // feedback loop where the next time
        // HeaderValue is called, it can simply append
        theConnection->m_currentHeaderField = QString();
        theConnection->m_currentHeaderValue = QString();
    }

    QString fieldSuffix = QString::fromLatin1(at, length);
    theConnection->m_currentHeaderField += fieldSuffix;
    return 0;
}

int QHttpConnection::HeaderValue(http_parser *parser, const char *at, size_t length)
{
    QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
    Q_ASSERT(theConnection->m_request);

    QString valueSuffix = QString::fromLatin1(at, length);
    theConnection->m_currentHeaderValue += valueSuffix;
    return 0;
}

int QHttpConnection::Body(http_parser *parser, const char *at, size_t length)
{
    QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
    Q_ASSERT(theConnection->m_request);

    Q_EMIT theConnection->m_request->data(QByteArray(at, length));
    return 0;
}

/*************************************************************************
 * WebSocket methods
 *************************************************************************/

QHttpConnection *QHttpConnection::enableWebSocket()
{
    m_isWebSocket = true;

    disconnect(m_socket, SIGNAL(readyRead()), this, SLOT(parseRequest()));

    m_websocketServer = new QWebSocketServer("QLC+WSServer", QWebSocketServer::NonSecureMode);
    m_socket->disconnect();
    //m_socket->setParent(m_websocketServer);
    m_socket->rollbackTransaction();
    m_websocketServer->handleConnection(m_socket);
    //emit m_socket->readyRead();

    connect(m_websocketServer, SIGNAL(newConnection()),
            this, SLOT(slotWebSocketNewConnection()));

    return this;
}

void QHttpConnection::slotWebSocketNewConnection()
{
    qDebug() << "[WS] New connection";
    if (m_websocketServer->hasPendingConnections())
    {
        m_webSocket = m_websocketServer->nextPendingConnection();

        connect(m_webSocket, SIGNAL(textMessageReceived(const QString&)),
                this, SLOT(slotWebSocketTextMessage(const QString&)));
        connect(m_webSocket, SIGNAL(disconnected()),
                this, SLOT(slotWebSocketDisconnected()));

        // activate ping to WS
        m_pollTimer = new QTimer(this);
        m_pollTimer->setInterval(5000);

        connect(m_pollTimer, SIGNAL(timeout()),
                this, SLOT(slotWebSocketPollTimeout()));

        m_pollTimer->start();
    }
}

void QHttpConnection::slotWebSocketDisconnected()
{
    Q_EMIT webSocketConnectionClose(this);
}

void QHttpConnection::slotWebSocketTextMessage(const QString &message)
{
    //qDebug() << "[WS] message received:" << message;
    Q_EMIT webSocketDataReady(this, message);
}

void QHttpConnection::slotWebSocketPollTimeout()
{
    if (m_webSocket)
        m_webSocket->ping();
}

void QHttpConnection::webSocketWrite(const QString &message)
{
    qDebug() << "[webSocketWrite] message length:" << message.size() << "message:" << message;

    if (m_webSocket)
        m_webSocket->sendTextMessage(message);
}

/// @endcond
