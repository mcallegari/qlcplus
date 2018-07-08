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

    qDebug() << "HTTP connection created !";
}

QHttpConnection::~QHttpConnection()
{
    delete m_socket;
    m_socket = 0;

    free(m_parser);
    m_parser = 0;

    delete m_parserSettings;
    m_parserSettings = 0;

    if (m_isWebSocket == true)
        Q_EMIT webSocketConnectionClose(this);

    qDebug() << "HTTP connection destroyed !";
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
        QByteArray arr = m_socket->readAll();
        if (m_isWebSocket)
            webSocketRead(arr);
        else
            http_parser_execute(m_parser, m_parserSettings, arr.constData(), arr.size());
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    url.setQuery(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_QUERY));
#else
    if (HAS_URL_FIELD(urlInfo, UF_QUERY)) {
        url.setEncodedQuery(QByteArray(urlData + urlInfo.field_data[UF_QUERY].off,
                                       urlInfo.field_data[UF_QUERY].len));
    }
#endif
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

QHttpConnection *QHttpConnection::enableWebSocket(bool enable)
{
    m_isWebSocket = enable;
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(5000);

    connect(m_pollTimer, SIGNAL(timeout()),
            this, SLOT(slotWebSocketPollTimeout()));

    m_pollTimer->start();
    return this;
}

void QHttpConnection::slotWebSocketPollTimeout()
{
    webSocketWrite(Ping, QByteArray());
}

void QHttpConnection::webSocketWrite(WebSocketOpCode opCode, QByteArray data)
{
    qDebug() << "[webSocketWrite] data size:" << data.size();
    if (data.size() < 126)
        data.prepend(quint8(data.size()));
    else
    {
        data.prepend(quint8(data.size() & 0x00FF));
        data.prepend(quint8(data.size() >> 8));
        data.prepend(0x7E);
    }

    data.prepend(0x80 + quint8(opCode));

    if (m_socket)
        m_socket->write(data);
}

/**
     Here's the RFC 6455 Framing specs. The table of the law

      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
 */

void QHttpConnection::webSocketRead(QByteArray data)
{
    if (data.size() < 2)
        return;

    int dataPos = 0;

    if (((data.at(dataPos) >> 4) & 0x07) != 0)
    {
        qWarning() << "Wrong WebSocket RSV bits. Discard.";
        return;
    }

    qDebug() << "[webSocketRead] total data length:" << data.size();

    while (dataPos < data.size())
    {
        int opCode = data.at(dataPos) & 0x0F;
        dataPos++;

        bool masked = (data.at(dataPos) >> 7) ? true : false;
        int dataLen = data.at(dataPos) & 0x7F;
        dataPos++;

        if (dataLen == 126)
        {
            dataLen = (data.at(dataPos) << 8) + data.at(dataPos + 1);
            dataPos+=2;
        }
        else if (dataLen == 127)
        {
            // TODO: 64bit length...really ?
            dataPos+=8;
        }

        quint8 mask[4];
        if (masked == true)
        {
            mask[0] = quint8(data.at(dataPos));
            mask[1] = quint8(data.at(dataPos + 1));
            mask[2] = quint8(data.at(dataPos + 2));
            mask[3] = quint8(data.at(dataPos + 3));
            dataPos+=4;
        }

        qDebug() << "[webSocketRead] opCode:" << QString("0x%1").arg(opCode, 2, 16, QChar('0'));

        if (opCode == TextFrame)
        {
            int lengthCounter = dataLen;
            qDebug() << "[webSocketRead] Text frame length:" << dataLen;
            // if the payload is masked, then unmask
            if (masked == true)
            {
                int i = 0;
                char *cData = data.data() + dataPos;
                while (lengthCounter-- > 0)
                    *cData++ ^= mask[i++ % 4];
            }

            Q_EMIT webSocketDataReady(this, QString(data.mid(dataPos, dataLen)));
        }
        else if (opCode == ConnectionClose)
        {
            qDebug() << "[webSocketRead] Connection closed by the client";
            Q_EMIT webSocketConnectionClose(this);
        }
        dataPos += dataLen;
    }
}


/// @endcond
