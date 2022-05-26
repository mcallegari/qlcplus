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

#define WS_CLOSE_CODE(n) (char[]{(n/0x100, n & 0xff})

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

    if (m_isWebSocket == true)
        Q_EMIT webSocketConnectionClose(this);

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

QHttpConnection *QHttpConnection::enableWebSocket(bool enable)
{
    m_isWebSocket = enable;
    m_websocket_state = WebSocketState::init;
    m_websocket_state_param = 0;
    m_websocket_opCode = 0;
    m_websocket_masked = false;
    for(int i=0; i<4; i++) m_websocket_mask[i] = 0;

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(5000);

    connect(m_pollTimer, SIGNAL(timeout()),
            this, SLOT(slotWebSocketPollTimeout()));

    m_pollTimer->start();
    return this;
}

void QHttpConnection::slotWebSocketPollTimeout()
{
    webSocketWrite(Ping, QByteArray::fromRawData({}, 0));
}

void QHttpConnection::webSocketWrite(WebSocketOpCode opCode, QByteArray data)
{
    QByteArray header;
    header.append(0x80 + quint8(opCode));

    qDebug() << "[webSocketWrite] data size:" << data.size();
    // qDebug() << "[webSocketWrite] opCode:" << QString("0x%1").arg(m_websocket_opCode, 2, 16, QChar('0')) << data.toHex();
    if (data.size() < 126)
    {
        header.append(quint8(data.size()));
    }
    else if(data.size() < 65536)
    {
        header.append(0x7E);
        header.append(quint8(data.size() >> 8));
        header.append(quint8(data.size() & 0xFF));
    }
    else
    {
        header.append(0x7F);
        header.append(quint8(0));
        header.append(quint8(0));
        header.append(quint8(0));
        header.append(quint8(0));
        header.append(quint8((data.size() >> 24) & 0xFF));
        header.append(quint8((data.size() >> 16) & 0xFF));
        header.append(quint8((data.size() >> 8) & 0xFF));
        header.append(quint8(data.size() & 0xFF));
    }

    if (m_socket)
    {
        m_socket->write(header);
        m_socket->write(data);
    }
}

/**
     <https://www.rfc-editor.org/rfc/rfc6455.html#section-5.2>
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

void QHttpConnection::webSocketRead(QByteArray &data)
{
    qDebug() << "[webSocketRead] total data length:" << data.size();

    for(int dataPos = 0; dataPos < data.size(); dataPos++)
    {
        uchar dataAt = data.at(dataPos) & 0xFF;
        switch(m_websocket_state){
            case WebSocketState::init:
                m_websocket_fin = (dataAt & 0x80) ? true : false;
                m_websocket_opCode = dataAt & 0x0F;
                if (((dataAt >> 4) & 0x07) != 0)
                {
                    qWarning() << "[webSocketRead] Wrong WebSocket RSV bits. Abort.";
                    webSocketClose(1002);
                    return;
                }
                if (!m_websocket_fin && (m_websocket_opCode & 8))
                {
                    qWarning() << "[webSocketRead] Fragmented control frame. Abort.";
                    webSocketClose(1002);
                    return;
                }
                if((m_websocket_opCode & 0x7) > 2){
                    qWarning() << "[webSocketRead] Unknown opCode. Abort.";
                    webSocketClose(1002);
                    return;
                }
                if(!m_websocket_fin)
                {
                    qWarning() << "[webSocketRead] Unsupported fragmented frame. Abort.";
                    webSocketClose(1003);
                    return;
                }
                if(m_websocket_opCode & 7)
                {
                    m_websocket_payload_buffer.resize(0);
                }
                m_websocket_state = WebSocketState::length;
                m_websocket_state_param = 0;
                break;

            case WebSocketState::length:
                m_websocket_masked = (dataAt & 0x80) ? true : false;
                m_websocket_dataLen = dataAt & 0x7F;
                // Determine the state of the next byte
                if(m_websocket_dataLen >= 126)
                {
                    if(m_websocket_opCode & 0x8){
                        // Control frames must not be this long
                        webSocketClose(1002);
                        return;
                    }
                    m_websocket_state = WebSocketState::length_extended;
                    m_websocket_state_param = (m_websocket_dataLen == 127) ? 8 : 2;
                    m_websocket_dataLen = 0;
                }
                else
                    // webSocketClose(1003);
                    // return;
                    goto transition_mask;

                break;

            case WebSocketState::length_extended:
                if(m_websocket_dataLen > (UINT_MAX>>8)){
                    // If this is going to overflow m_websocket_dataLen, abort
                    m_websocket_state = WebSocketState::sink;
                    Q_EMIT webSocketConnectionClose(this);
                    return;
                }
                m_websocket_dataLen = m_websocket_dataLen*0x100 + dataAt;
                // If at end, go to next state (whatever that is)
                if(--m_websocket_state_param > 0)
                    break;

                // fall through: goto transition_mask
            
                transition_mask:
                // A few states jump here when the next byte is a mask,
                // or would be if the mask were enabled. If the mask is disabled,
                // this transitions through to parsing the payload.
                if(m_websocket_masked)
                    m_websocket_state = WebSocketState::mask;
                else
                    goto transition_payload;
                break;

            case WebSocketState::mask:
                m_websocket_mask[m_websocket_state_param] = dataAt;
                if(++m_websocket_state_param < 4)
                    break;

                // fall through: goto transition_payload

                transition_payload:
                // Multiple states jump here in anticipation of the next byte
                // being a payload or a new frame.
                m_websocket_state = WebSocketState::payload;
                m_websocket_state_param = 0;
                // Determine the state of the next byte.
                // If the payload size is zero, this will be a new frame,
                // so call webSocketParsePayload, which will also set m_websocket_state
                qDebug() << "[webSocketRead] Text frame length:" << m_websocket_dataLen;
                if(0 == m_websocket_dataLen)
                {
                    m_websocket_payload.resize(0);
                    webSocketParsePayload();
                }
                else
                {
                    m_websocket_payload.resize(m_websocket_dataLen);
                    m_websocket_payload.fill(0);
                }
                break;

            case WebSocketState::payload:
                // if the payload is masked, then unmask
                if (m_websocket_masked == true)
                    m_websocket_payload[m_websocket_state_param] = dataAt ^ m_websocket_mask[m_websocket_state_param % 4];
                else
                    m_websocket_payload[m_websocket_state_param] = dataAt;

                Q_ASSERT(m_websocket_state_param < m_websocket_dataLen);
                if(++m_websocket_state_param == m_websocket_dataLen)
                    webSocketParsePayload();
                break;

            case WebSocketState::closed:
                // We have sent a closed frame, and are waiting for the client to receive it
                return;

            case WebSocketState::sink:
                // Connection has been closed to a protocol error, ignore additional bytes
                return;

            default:
                Q_UNREACHABLE();
                break;
        }
    }
}

void QHttpConnection::webSocketParsePayload()
{
    // This is the last byte of this frame, the next byte will be a new frame.
    m_websocket_state = WebSocketState::init;
    qDebug() << "[webSocketRead] opCode:" << QString("0x%1").arg(m_websocket_opCode, 2, 16, QChar('0'));

    if(m_websocket_opCode < 8){
        m_websocket_payload_buffer.append(m_websocket_payload);
    }

    if (m_websocket_opCode == ConnectionClose)
    {
        qDebug() << "[webSocketRead] Connection closed by the client";
        webSocketClose((m_websocket_payload[0]<<8) & m_websocket_payload[1]);
        return;
    }
    else if(m_websocket_opCode == Ping)
    {
        webSocketWrite(Pong, m_websocket_payload);
    }

    if(!m_websocket_fin)
        return;

    if (m_websocket_opCode == TextFrame)
    {
        qDebug() << "[webSocketRead] Text frame length:" << m_websocket_dataLen << m_websocket_payload_buffer;
        Q_EMIT webSocketDataReady(this, QString(m_websocket_payload));
        // webSocketWrite(TextFrame, QString(m_websocket_payload_buffer).toUtf8());
    }
    else if (m_websocket_opCode == BinaryFrame)
    {
        qDebug() << "[webSocketRead] Binary frame length:" << m_websocket_dataLen << m_websocket_payload_buffer;
        Q_EMIT webSocketDataReady(this, QString(m_websocket_payload));
        // webSocketWrite(BinaryFrame, m_websocket_payload_buffer);
    }
    else
    {
        qWarning() << "[webSocketRead] Unknown opCode:" << QString::number(m_websocket_opCode, 0x100);
    }
}

void QHttpConnection::webSocketClose(quint16 error_code){
    if(!m_websocket_closing)
    {
        QByteArray response(2, (char)0);
        response[0] = error_code >> 8;
        response[1] = error_code & 0xff;
        m_websocket_state = WebSocketState::sink;
        webSocketWrite(ConnectionClose, response);
        m_socket->flush();
    }
    m_websocket_closing = true;
    m_socket->close();
    Q_EMIT webSocketConnectionClose(this);
}


/// @endcond
