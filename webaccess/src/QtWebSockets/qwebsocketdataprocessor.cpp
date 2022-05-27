/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
/*!
    \class QWebSocketDataProcessor
    The class QWebSocketDataProcessor is responsible for reading, validating and
    interpreting data from a WebSocket.
    It reads data from a QIODevice, validates it against \l{RFC 6455}, and parses it into
    frames (data, control).
    It emits signals that correspond to the type of the frame: textFrameReceived(),
    binaryFrameReceived(), textMessageReceived(), binaryMessageReceived(), pingReceived(),
    pongReceived() and closeReceived().
    Whenever an error is detected, the errorEncountered() signal is emitted.
    QWebSocketDataProcessor also checks if a frame is allowed in a sequence of frames
    (e.g. a continuation frame cannot follow a final frame).
    This class is an internal class used by QWebSocketInternal for data processing and validation.

    \sa Frame()

    \internal
*/
#include "qwebsocketdataprocessor_p.h"
#include "qwebsocketprotocol.h"
#include "qwebsocketprotocol_p.h"
#include "qwebsocketframe_p.h"

#include <QtCore/QtEndian>
#include <QtCore/QTextCodec>
#include <QtCore/QTextDecoder>
#include <QtCore/QDebug>

#include <limits.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
QWebSocketDataProcessor::QWebSocketDataProcessor(QObject *parent) :
    QObject(parent),
    m_processingState(PS_READ_HEADER),
    m_isFinalFrame(false),
    m_isFragmented(false),
    m_opCode(QWebSocketProtocol::OpCodeClose),
    m_isControlFrame(false),
    m_hasMask(false),
    m_mask(0),
    m_binaryMessage(),
    m_textMessage(),
    m_payloadLength(0),
    m_pConverterState(nullptr),
    m_pTextCodec(QTextCodec::codecForName("UTF-8")),
    m_waitTimer(new QTimer(this))
{
    clear();
    // initialize the internal timeout timer
    m_waitTimer->setInterval(5000);
    m_waitTimer->setSingleShot(true);
    m_waitTimer->callOnTimeout(this, &QWebSocketDataProcessor::timeout);
}

/*!
    \internal
 */
QWebSocketDataProcessor::~QWebSocketDataProcessor()
{
    clear();
    if (m_pConverterState) {
        delete m_pConverterState;
        m_pConverterState = nullptr;
    }
}

void QWebSocketDataProcessor::setMaxAllowedFrameSize(quint64 maxAllowedFrameSize)
{
    frame.setMaxAllowedFrameSize(maxAllowedFrameSize);
}

quint64 QWebSocketDataProcessor::maxAllowedFrameSize() const
{
    return frame.maxAllowedFrameSize();
}

/*!
    \internal
 */
void QWebSocketDataProcessor::setMaxAllowedMessageSize(quint64 maxAllowedMessageSize)
{
    if (maxAllowedMessageSize <= maxMessageSize())
        m_maxAllowedMessageSize = maxAllowedMessageSize;
}

/*!
    \internal
 */
quint64 QWebSocketDataProcessor::maxAllowedMessageSize() const
{
    return m_maxAllowedMessageSize;
}

/*!
    \internal
 */
quint64 QWebSocketDataProcessor::maxMessageSize()
{
    return MAX_MESSAGE_SIZE_IN_BYTES;   //COV_NF_LINE
}

/*!
    \internal
 */
quint64 QWebSocketDataProcessor::maxFrameSize()
{
   return QWebSocketFrame::maxFrameSize();
}

/*!
    \internal

    Returns \c true if a complete websocket frame has been processed;
    otherwise returns \c false.
 */
bool QWebSocketDataProcessor::process(QIODevice *pIoDevice)
{
    bool isDone = false;

    while (!isDone) {
        frame.readFrame(pIoDevice);
        if (!frame.isDone()) {
            // waiting for more data available
            QObject::connect(pIoDevice, &QIODevice::readyRead,
                             m_waitTimer, &QTimer::stop, Qt::UniqueConnection);
            m_waitTimer->start();
            return false;
        } else if (Q_LIKELY(frame.isValid())) {
            if (frame.isControlFrame()) {
                isDone = processControlFrame(frame);
            } else {
                //we have a dataframe; opcode can be OC_CONTINUE, OC_TEXT or OC_BINARY
                if (Q_UNLIKELY(!m_isFragmented && frame.isContinuationFrame())) {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeProtocolError,
                                            tr("Received Continuation frame, while there is " \
                                               "nothing to continue."));
                    return true;
                }
                if (Q_UNLIKELY(m_isFragmented && frame.isDataFrame() &&
                               !frame.isContinuationFrame())) {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeProtocolError,
                                            tr("All data frames after the initial data frame " \
                                               "must have opcode 0 (continuation)."));
                    return true;
                }
                if (!frame.isContinuationFrame()) {
                    m_opCode = frame.opCode();
                    m_isFragmented = !frame.isFinalFrame();
                }
                quint64 messageLength = m_opCode == QWebSocketProtocol::OpCodeText
                        ? quint64(m_textMessage.length())
                        : quint64(m_binaryMessage.length());
                if (Q_UNLIKELY((messageLength + quint64(frame.payload().length())) >
                               maxAllowedMessageSize())) {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeTooMuchData,
                                            tr("Received message is too big."));
                    return true;
                }

                if (m_opCode == QWebSocketProtocol::OpCodeText) {
                    QString frameTxt = m_pTextCodec->toUnicode(frame.payload().constData(),
                                                               frame.payload().size(),
                                                               m_pConverterState);
                    bool failed = (m_pConverterState->invalidChars != 0)
                            || (frame.isFinalFrame() && (m_pConverterState->remainingChars != 0));
                    if (Q_UNLIKELY(failed)) {
                        clear();
                        Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeWrongDatatype,
                                                tr("Invalid UTF-8 code encountered."));
                        return true;
                    } else {
                        m_textMessage.append(frameTxt);
                        Q_EMIT textFrameReceived(frameTxt, frame.isFinalFrame());
                    }
                } else {
                    m_binaryMessage.append(frame.payload());
                    Q_EMIT binaryFrameReceived(frame.payload(), frame.isFinalFrame());
                }

                if (frame.isFinalFrame()) {
                    isDone = true;
                    if (m_opCode == QWebSocketProtocol::OpCodeText) {
                        const QString textMessage(m_textMessage);
                        clear();
                        Q_EMIT textMessageReceived(textMessage);
                    } else {
                        const QByteArray binaryMessage(m_binaryMessage);
                        clear();
                        Q_EMIT binaryMessageReceived(binaryMessage);
                    }
                }
            }
        } else {
            Q_EMIT errorEncountered(frame.closeCode(), frame.closeReason());
            clear();
            isDone = true;
        }
        frame.clear();
    }
    return true;
}

/*!
    \internal
 */
void QWebSocketDataProcessor::clear()
{
    m_processingState = PS_READ_HEADER;
    m_isFinalFrame = false;
    m_isFragmented = false;
    m_opCode = QWebSocketProtocol::OpCodeClose;
    m_hasMask = false;
    m_mask = 0;
    m_binaryMessage.clear();
    m_textMessage.clear();
    m_payloadLength = 0;
    frame.clear();
    if (m_pConverterState) {
        if ((m_pConverterState->remainingChars != 0) || (m_pConverterState->invalidChars != 0)) {
            delete m_pConverterState;
            m_pConverterState = nullptr;
        }
    }
    if (!m_pConverterState)
        m_pConverterState = new QTextCodec::ConverterState(QTextCodec::ConvertInvalidToNull |
                                                           QTextCodec::IgnoreHeader);
}

/*!
    \internal
 */
bool QWebSocketDataProcessor::processControlFrame(const QWebSocketFrame &frame)
{
    bool mustStopProcessing = true; //control frames never expect additional frames to be processed
    switch (frame.opCode()) {
    case QWebSocketProtocol::OpCodePing:
        Q_EMIT pingReceived(frame.payload());
        break;

    case QWebSocketProtocol::OpCodePong:
        Q_EMIT pongReceived(frame.payload());
        break;

    case QWebSocketProtocol::OpCodeClose:
    {
        quint16 closeCode = QWebSocketProtocol::CloseCodeNormal;
        QString closeReason;
        QByteArray payload = frame.payload();
        if (Q_UNLIKELY(payload.size() == 1)) {
            //size is either 0 (no close code and no reason)
            //or >= 2 (at least a close code of 2 bytes)
            closeCode = QWebSocketProtocol::CloseCodeProtocolError;
            closeReason = tr("Payload of close frame is too small.");
        } else if (Q_LIKELY(payload.size() > 1)) {
            //close frame can have a close code and reason
            closeCode = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(payload.constData()));
            if (Q_UNLIKELY(!QWebSocketProtocol::isCloseCodeValid(closeCode))) {
                closeCode = QWebSocketProtocol::CloseCodeProtocolError;
                closeReason = tr("Invalid close code %1 detected.").arg(closeCode);
            } else {
                if (payload.size() > 2) {
                    QTextCodec *tc = QTextCodec::codecForName(QByteArrayLiteral("UTF-8"));
                    QTextCodec::ConverterState state(QTextCodec::ConvertInvalidToNull);
                    closeReason = tc->toUnicode(payload.constData() + 2, payload.size() - 2, &state);
                    const bool failed = (state.invalidChars != 0) || (state.remainingChars != 0);
                    if (Q_UNLIKELY(failed)) {
                        closeCode = QWebSocketProtocol::CloseCodeWrongDatatype;
                        closeReason = tr("Invalid UTF-8 code encountered.");
                    }
                }
            }
        }
        Q_EMIT closeReceived(static_cast<QWebSocketProtocol::CloseCode>(closeCode), closeReason);
        break;
    }

    case QWebSocketProtocol::OpCodeContinue:
    case QWebSocketProtocol::OpCodeBinary:
    case QWebSocketProtocol::OpCodeText:
    case QWebSocketProtocol::OpCodeReserved3:
    case QWebSocketProtocol::OpCodeReserved4:
    case QWebSocketProtocol::OpCodeReserved5:
    case QWebSocketProtocol::OpCodeReserved6:
    case QWebSocketProtocol::OpCodeReserved7:
    case QWebSocketProtocol::OpCodeReservedC:
    case QWebSocketProtocol::OpCodeReservedB:
    case QWebSocketProtocol::OpCodeReservedD:
    case QWebSocketProtocol::OpCodeReservedE:
    case QWebSocketProtocol::OpCodeReservedF:
        //do nothing
        //case statements added to make C++ compiler happy
        break;

    default:
        Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeProtocolError,
                                tr("Invalid opcode detected: %1").arg(int(frame.opCode())));
        //do nothing
        break;
    }
    return mustStopProcessing;
}

/*!
    \internal
 */
void QWebSocketDataProcessor::timeout()
{
    clear();
    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeGoingAway,
                            tr("Timeout when reading data from socket."));
}

QT_END_NAMESPACE
