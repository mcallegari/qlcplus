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
    \class QWebSocketFrame
    The class QWebSocketFrame is responsible for reading, validating and
    interpreting frames from a WebSocket.
    It reads data from a QIODevice, validates it against RFC 6455, and parses it into a
    frame (data, control).
    Whenever an error is detected, isValid() returns false.

    \note The QWebSocketFrame class does not look at valid sequences of frames.
    It processes frames one at a time.
    \note It is the QWebSocketDataProcessor that takes the sequence into account.

    \sa QWebSocketDataProcessor
    \internal
 */

#include "qwebsocketframe_p.h"
#include "qwebsocketprotocol_p.h"

#include <QtCore/QtEndian>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
void QWebSocketFrame::setMaxAllowedFrameSize(quint64 maxAllowedFrameSize)
{
    if (maxAllowedFrameSize <= maxFrameSize())
        m_maxAllowedFrameSize = maxAllowedFrameSize;
}

/*!
    \internal
 */
quint64 QWebSocketFrame::maxAllowedFrameSize() const
{
    return m_maxAllowedFrameSize;
}

/*!
    \internal
 */
quint64 QWebSocketFrame::maxFrameSize()
{
    return MAX_FRAME_SIZE_IN_BYTES;
}

/*!
    \internal
 */
QWebSocketProtocol::CloseCode QWebSocketFrame::closeCode() const
{
    return isDone() ? m_closeCode : QWebSocketProtocol::CloseCodeGoingAway;
}

/*!
    \internal
 */
QString QWebSocketFrame::closeReason() const
{
    return isDone() ? m_closeReason : tr("Waiting for more data from socket.");
}

/*!
    \internal
 */
bool QWebSocketFrame::isFinalFrame() const
{
    return m_isFinalFrame;
}

/*!
    \internal
 */
bool QWebSocketFrame::isControlFrame() const
{
    return (m_opCode & 0x08) == 0x08;
}

/*!
    \internal
 */
bool QWebSocketFrame::isDataFrame() const
{
    return !isControlFrame();
}

/*!
    \internal
 */
bool QWebSocketFrame::isContinuationFrame() const
{
    return isDataFrame() && (m_opCode == QWebSocketProtocol::OpCodeContinue);
}

/*!
    \internal
 */
bool QWebSocketFrame::hasMask() const
{
    return m_mask != 0;
}

/*!
    \internal
 */
quint32 QWebSocketFrame::mask() const
{
    return m_mask;
}

/*!
    \internal
 */
QWebSocketProtocol::OpCode QWebSocketFrame::opCode() const
{
    return m_opCode;
}

/*!
    \internal
 */
QByteArray QWebSocketFrame::payload() const
{
    return m_payload;
}

/*!
    Resets all member variables, and invalidates the object.

    \internal
 */
void QWebSocketFrame::clear()
{
    m_closeCode = QWebSocketProtocol::CloseCodeNormal;
    m_closeReason.clear();
    m_isFinalFrame = true;
    m_mask = 0;
    m_rsv1 = false;
    m_rsv2 = false;
    m_rsv3 = false;
    m_opCode = QWebSocketProtocol::OpCodeReservedC;
    m_length = 0;
    m_payload.clear();
    m_isValid = false;
    m_processingState = PS_READ_HEADER;
}

/*!
    \internal
 */
bool QWebSocketFrame::isValid() const
{
    return isDone() && m_isValid;
}

/*!
    \internal
 */
bool QWebSocketFrame::isDone() const
{
    return m_processingState == PS_DISPATCH_RESULT;
}

/*!
    \internal
 */
void QWebSocketFrame::readFrame(QIODevice *pIoDevice)
{
    while (true)
    {
        switch (m_processingState) {
        case PS_READ_HEADER:
            m_processingState = readFrameHeader(pIoDevice);
            if (m_processingState == PS_WAIT_FOR_MORE_DATA) {
                m_processingState = PS_READ_HEADER;
                return;
            }
            break;

        case PS_READ_PAYLOAD_LENGTH:
            m_processingState = readFramePayloadLength(pIoDevice);
            if (m_processingState == PS_WAIT_FOR_MORE_DATA) {
                m_processingState = PS_READ_PAYLOAD_LENGTH;
                return;
            }
            break;

        case PS_READ_MASK:
            m_processingState = readFrameMask(pIoDevice);
            if (m_processingState == PS_WAIT_FOR_MORE_DATA) {
                m_processingState = PS_READ_MASK;
                return;
            }
            break;

        case PS_READ_PAYLOAD:
            m_processingState = readFramePayload(pIoDevice);
            if (m_processingState == PS_WAIT_FOR_MORE_DATA) {
                m_processingState = PS_READ_PAYLOAD;
                return;
            }
            break;

        case PS_DISPATCH_RESULT:
            return;

        default:
            Q_UNREACHABLE();
            return;
        }
    }
}

/*!
    \internal
 */
QWebSocketFrame::ProcessingState QWebSocketFrame::readFrameHeader(QIODevice *pIoDevice)
{
    if (Q_LIKELY(pIoDevice->bytesAvailable() >= 2)) {
        // FIN, RSV1-3, Opcode
        char header[2] = {0};
        if (Q_UNLIKELY(pIoDevice->read(header, 2) < 2)) {
            setError(QWebSocketProtocol::CloseCodeGoingAway,
                     tr("Error occurred while reading header from the network: %1")
                        .arg(pIoDevice->errorString()));
            return PS_DISPATCH_RESULT;
        }
        m_isFinalFrame = (header[0] & 0x80) != 0;
        m_rsv1 = (header[0] & 0x40);
        m_rsv2 = (header[0] & 0x20);
        m_rsv3 = (header[0] & 0x10);
        m_opCode = static_cast<QWebSocketProtocol::OpCode>(header[0] & 0x0F);

        // Mask
        // Use zero as mask value to mean there's no mask to read.
        // When the mask value is read, it over-writes this non-zero value.
        m_mask = header[1] & 0x80;
        // PayloadLength
        m_length = (header[1] & 0x7F);

        if (!checkValidity())
            return PS_DISPATCH_RESULT;

        switch (m_length) {
        case 126:
        case 127:
            return PS_READ_PAYLOAD_LENGTH;
        default:
            return hasMask() ? PS_READ_MASK : PS_READ_PAYLOAD;
        }
    }
    return PS_WAIT_FOR_MORE_DATA;
}

/*!
    \internal
 */
QWebSocketFrame::ProcessingState QWebSocketFrame::readFramePayloadLength(QIODevice *pIoDevice)
{
    // see http://tools.ietf.org/html/rfc6455#page-28 paragraph 5.2
    // in all cases, the minimal number of bytes MUST be used to encode the length,
    // for example, the length of a 124-byte-long string can't be encoded as the
    // sequence 126, 0, 124"
    switch (m_length) {
    case 126:
        if (Q_LIKELY(pIoDevice->bytesAvailable() >= 2)) {
            uchar length[2] = {0};
            if (Q_UNLIKELY(pIoDevice->read(reinterpret_cast<char *>(length), 2) < 2)) {
                setError(QWebSocketProtocol::CloseCodeGoingAway,
                         tr("Error occurred while reading from the network: %1")
                            .arg(pIoDevice->errorString()));
                return PS_DISPATCH_RESULT;
            }
            m_length = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
            if (Q_UNLIKELY(m_length < 126)) {

                setError(QWebSocketProtocol::CloseCodeProtocolError,
                            tr("Lengths smaller than 126 must be expressed as one byte."));
                return PS_DISPATCH_RESULT;
            }
            return hasMask() ? PS_READ_MASK : PS_READ_PAYLOAD;
        }
        break;
    case 127:
        if (Q_LIKELY(pIoDevice->bytesAvailable() >= 8)) {
            uchar length[8] = {0};
            if (Q_UNLIKELY(pIoDevice->read(reinterpret_cast<char *>(length), 8) < 8)) {
                setError(QWebSocketProtocol::CloseCodeAbnormalDisconnection,
                         tr("Something went wrong during reading from the network."));
                return PS_DISPATCH_RESULT;
            }
            // Most significant bit must be set to 0 as
            // per http://tools.ietf.org/html/rfc6455#section-5.2
            m_length = qFromBigEndian<quint64>(length);
            if (Q_UNLIKELY(m_length & (quint64(1) << 63))) {
                setError(QWebSocketProtocol::CloseCodeProtocolError,
                            tr("Highest bit of payload length is not 0."));
                return PS_DISPATCH_RESULT;
            }
            if (Q_UNLIKELY(m_length <= 0xFFFFu)) {
                setError(QWebSocketProtocol::CloseCodeProtocolError,
                            tr("Lengths smaller than 65536 (2^16) must be expressed as 2 bytes."));
                return PS_DISPATCH_RESULT;
            }
            return hasMask() ? PS_READ_MASK : PS_READ_PAYLOAD;
        }
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    return PS_WAIT_FOR_MORE_DATA;
}

/*!
    \internal
 */
QWebSocketFrame::ProcessingState QWebSocketFrame::readFrameMask(QIODevice *pIoDevice)
{
    if (Q_LIKELY(pIoDevice->bytesAvailable() >= 4)) {
        if (Q_UNLIKELY(pIoDevice->read(reinterpret_cast<char *>(&m_mask), sizeof(m_mask)) < 4)) {
            setError(QWebSocketProtocol::CloseCodeGoingAway,
                     tr("Error while reading from the network: %1.").arg(pIoDevice->errorString()));
            return PS_DISPATCH_RESULT;
        }
        m_mask = qFromBigEndian(m_mask);
        return PS_READ_PAYLOAD;
    }
    return PS_WAIT_FOR_MORE_DATA;
}

/*!
    \internal
 */
QWebSocketFrame::ProcessingState QWebSocketFrame::readFramePayload(QIODevice *pIoDevice)
{
    if (!m_length)
        return PS_DISPATCH_RESULT;

    if (Q_UNLIKELY(m_length > maxAllowedFrameSize())) {
        setError(QWebSocketProtocol::CloseCodeTooMuchData, tr("Maximum framesize exceeded."));
        return PS_DISPATCH_RESULT;
    }
    if (quint64(pIoDevice->bytesAvailable()) >= m_length) {
        m_payload = pIoDevice->read(int(m_length));
        // m_length can be safely cast to an integer,
        // because MAX_FRAME_SIZE_IN_BYTES = MAX_INT
        if (Q_UNLIKELY(m_payload.length() != int(m_length))) {
            // some error occurred; refer to the Qt documentation of QIODevice::read()
            setError(QWebSocketProtocol::CloseCodeAbnormalDisconnection,
                     tr("Some serious error occurred while reading from the network."));
        } else if (hasMask()) {
            QWebSocketProtocol::mask(&m_payload, mask());
        }
        return PS_DISPATCH_RESULT;
    }
    return PS_WAIT_FOR_MORE_DATA;
}

/*!
    \internal
 */
void QWebSocketFrame::setError(QWebSocketProtocol::CloseCode code, const QString &closeReason)
{
    clear();
    m_closeCode = code;
    m_closeReason = closeReason;
    m_isValid = false;
}

/*!
    \internal
 */
bool QWebSocketFrame::checkValidity()
{
    if (Q_UNLIKELY(m_rsv1 || m_rsv2 || m_rsv3)) {
        setError(QWebSocketProtocol::CloseCodeProtocolError, tr("Rsv field is non-zero"));
    } else if (Q_UNLIKELY(QWebSocketProtocol::isOpCodeReserved(m_opCode))) {
        setError(QWebSocketProtocol::CloseCodeProtocolError, tr("Used reserved opcode"));
    } else if (isControlFrame()) {
        if (Q_UNLIKELY(m_length > 125)) {
            setError(QWebSocketProtocol::CloseCodeProtocolError,
                     tr("Control frame is larger than 125 bytes"));
        } else if (Q_UNLIKELY(!m_isFinalFrame)) {
            setError(QWebSocketProtocol::CloseCodeProtocolError,
                     tr("Control frames cannot be fragmented"));
        } else {
            m_isValid = true;
        }
    } else {
        m_isValid = true;
    }
    return m_isValid;
}

QT_END_NAMESPACE
