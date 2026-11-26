/*
  Q Light Controller Plus
  hiddmxdevice.cpp

  Copyright (c) Massimo Callegari
                Florian Euchner
                Stefan Krupop

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

#include <errno.h>
#if !defined(WIN32) && !defined(Q_OS_WIN)
  #include <unistd.h>
#endif

#include <QApplication>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <QMessageBox>
#include <QByteArray>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "hiddmxdevice.h"
#include "qlcmacros.h"
#include "hidapi.h"
#include "hidplugin.h"

HIDDMXDevice::HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString &name, const QString& path)
    : HIDDevice(parent, line, name, path)
{
    m_capabilities = QLCIOPlugin::Output;
    m_mode = DMX_MODE_NONE;
    m_rdm = name.contains("RDM");
    m_rdmActive = false;
    init();
}

HIDDMXDevice::~HIDDMXDevice()
{
    closeInput();
    closeOutput();
    hid_close(m_handle);
}

void HIDDMXDevice::init()
{
    /* Device name */
    m_handle = hid_open_path(path().toUtf8().constData());

    if (!m_handle)
    {
        QMessageBox::warning(NULL, (tr("HID DMX Interface Error")),
            (tr("Unable to open %1. Make sure the udev rule is installed.").arg(name())),
             QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    /** Reset channels when opening the interface: */
    m_dmx_cmp.fill(0, 512);
    m_dmx_in_cmp.fill(0, 512);
    outputDMX(m_dmx_cmp, true);
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDDMXDevice::isMergerModeEnabled() const
{
    return (m_mode & DMX_MODE_MERGER);
}

void HIDDMXDevice::enableMergerMode(bool mergerModeEnabled)
{
    if (mergerModeEnabled)
        m_mode |= DMX_MODE_MERGER;
    else
        m_mode &= ~DMX_MODE_MERGER;
    updateMode();
}

bool HIDDMXDevice::openInput()
{
    m_mode |= DMX_MODE_INPUT;
    updateMode();

    return true;
}

void HIDDMXDevice::closeInput()
{
    m_mode &= ~DMX_MODE_INPUT;
    updateMode();
}

bool HIDDMXDevice::openOutput()
{
    m_mode |= DMX_MODE_OUTPUT;
    updateMode();

    return true;
}

void HIDDMXDevice::closeOutput()
{
    m_mode &= ~DMX_MODE_OUTPUT;
    updateMode();
}

bool HIDDMXDevice::readEvent()
{
    return true;
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDDMXDevice::infoText() const
{
    QString info;

    info += QString("<H3>%1</H3><P>").arg(m_name);

    return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDDMXDevice::feedBack(quint32 channel, uchar value)
{
    /* HID devices don't support feedback (yet) */
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void HIDDMXDevice::run()
{
    QElapsedTimer keepAlive;
    keepAlive.start();

    while (m_running == true)
    {
        RDMRequest request;
        bool hasRequest = false;

        m_rdmMutex.lock();
        if (m_running && m_rdmQueue.isEmpty() && (m_mode & DMX_MODE_INPUT) == 0)
            m_rdmCondition.wait(&m_rdmMutex, HID_DMX_RDM_KEEPALIVE / 2);
        if (m_rdmQueue.isEmpty() == false)
        {
            request = m_rdmQueue.takeFirst();
            hasRequest = true;
        }
        m_rdmMutex.unlock();

        if (hasRequest)
        {
            processRDMRequest(request);
            keepAlive.restart();
            continue;
        }

        /* after RDM traffic the dongle must be kept busy, or it reboots */
        if (m_rdmActive && keepAlive.elapsed() >= HID_DMX_RDM_KEEPALIVE)
        {
            QByteArray data;
            readRDMChunk(nullptr, data);
            keepAlive.restart();
        }

        if ((m_mode & DMX_MODE_INPUT) == 0)
            continue;

        unsigned char buffer[35];
        int size = hid_read_timeout(m_handle, buffer, 33, HID_DMX_READ_TIMEOUT);

        if (size == 33)
            handleInputReport(buffer);
    }
}

void HIDDMXDevice::handleInputReport(const unsigned char *buffer)
{
    /**
    * Protocol: 33 bytes in buffer[33]
    * [0]      = chunk, which is the offset by which the channel is calculated
    *            from, the nth chunk starts at address n * 32
    * [1]-[32] = channel values, where the nth value is the offset + n
    */
    if (buffer[0] >= 16)
        return;

    unsigned short startOff = buffer[0] * 32;
    for (int i = 0; i < 32; i++)
    {
        unsigned short channel = startOff + i;
        unsigned char value = buffer[i + 1];
        if ((unsigned char)m_dmx_in_cmp.at(channel) != value)
        {
            emit valueChanged(UINT_MAX, m_line, channel, value);
            m_dmx_in_cmp[channel] = value;
        }
    }
}

/*****************************************************************************
 * Output data
 *****************************************************************************/

void HIDDMXDevice::outputDMX(const QByteArray &universe, bool forceWrite)
{
    QMutexLocker locker(&m_writeMutex);

    for (int i = 0; i < 16; i++)
    {
        int startOff = i * 32;
        if (startOff >= universe.size())
            return;

        QByteArray chunk = universe.mid(startOff, 32);
        if (chunk.size() < 32)
            chunk.append(QByteArray(32 - chunk.size(), (char)0x0));

        if (forceWrite == true || chunk != m_dmx_cmp.mid(startOff, 32))
        {
            /** Save different data to m_dmx_cmp */
            m_dmx_cmp.replace(startOff, 32, chunk);

            chunk.prepend((char)i);
            chunk.prepend((char)0x0);

            /** Output new data */
            hid_write(m_handle, (const unsigned char *)chunk.data(), chunk.size());
        }
    }
}

/*****************************************************************************
 * RDM
 *****************************************************************************/

bool HIDDMXDevice::sendRDMFrame(quint32 universe, quint32 line, int type, const QByteArray &frame)
{
    if (hasRDM() == false || m_handle == NULL || (m_mode & DMX_MODE_OUTPUT) == 0 ||
        frame.isEmpty() || frame.size() > HID_DMX_RDM_CHUNK * HID_DMX_RDM_MAX_CHUNKS)
        return false;

    QMutexLocker locker(&m_rdmMutex);
    m_rdmQueue.append({ universe, line, type, frame });
    m_rdmCondition.wakeAll();

    return true;
}

void HIDDMXDevice::setRDMCodec(QLCRDMCodec *codec)
{
    m_rdmCodec.storeRelease(m_rdm ? codec : nullptr);

    /* the RDM requests are processed by the device thread */
    if (hasRDM() && (m_mode & DMX_MODE_OUTPUT) && isRunning() == false)
    {
        m_running = true;
        start();
    }
}

void HIDDMXDevice::writeRDMFrame(QLCRDMCodec *codec, const QByteArray &frame)
{
    QMutexLocker locker(&m_writeMutex);

    for (int index = 0; index * HID_DMX_RDM_CHUNK < frame.size(); index++)
    {
        QByteArray payload = frame.mid(index * HID_DMX_RDM_CHUNK, HID_DMX_RDM_CHUNK);
        payload.append(QByteArray(HID_DMX_RDM_CHUNK - payload.size(), char(0)));
        QByteArray data = codec->encode(payload);

        unsigned char buffer[34];
        memset(buffer, 0, sizeof(buffer));
        buffer[0] = 0; // report ID
        buffer[1] = uchar(HID_DMX_RDM_INDEX_BASE + index);
        memcpy(buffer + 2, data.constData(), qMin(data.size(), int(sizeof(buffer)) - 2));

        hid_write(m_handle, buffer, sizeof(buffer));
    }
}

int HIDDMXDevice::readRDMChunk(QLCRDMCodec *codec, QByteArray &data)
{
    unsigned char buffer[34];
    memset(buffer, 0, sizeof(buffer)); // report ID 0

    if (hid_get_input_report(m_handle, buffer, sizeof(buffer)) < int(sizeof(buffer)))
        return -1;

    // 0xFF = no data
    int index = buffer[1] - HID_DMX_RDM_INDEX_BASE;
    if (index < 0 || index >= HID_DMX_RDM_MAX_CHUNKS)
        return -1;

    if (codec == nullptr)
        return index;

    data = codec->decode(QByteArray((const char *)buffer + 2, sizeof(buffer) - 2));
    if (data.size() < HID_DMX_RDM_CHUNK)
        return -1;
    data.truncate(HID_DMX_RDM_CHUNK);

    return index;
}

void HIDDMXDevice::processRDMRequest(const RDMRequest &request)
{
    bool discovery = (request.type == QLCIOPlugin::RDMDiscovery);
    QLCRDMCodec *codec = m_rdmCodec.loadAcquire();
    QByteArray chunk;
    QByteArray reply;

    if (codec == nullptr)
    {
        emit rdmTimeout(request.universe, request.line, request.type);
        return;
    }

    m_rdmActive = true;

    /* the dongle sometimes loses a reply: retry the requests */
    for (int attempt = 0; attempt < (discovery ? 2 : 3) && reply.isEmpty(); attempt++)
    {
        /* drop the replies arrived too late for a previous request */
        for (int i = 0; i < HID_DMX_RDM_MAX_CHUNKS && readRDMChunk(nullptr, chunk) >= 0; i++) {}

        writeRDMFrame(codec, request.frame);

        QByteArray data;
        int received = 0; // bit mask of the chunks received
        QElapsedTimer timer;
        timer.start();

        while (timer.elapsed() < HID_DMX_RDM_TIMEOUT)
        {
            int index = readRDMChunk(codec, chunk);
            if (index < 0)
            {
                msleep(5);
                continue;
            }

            if (data.size() < (index + 1) * HID_DMX_RDM_CHUNK)
                data.resize((index + 1) * HID_DMX_RDM_CHUNK);
            data.replace(index * HID_DMX_RDM_CHUNK, HID_DMX_RDM_CHUNK, chunk);
            received |= (1 << index);

            if ((received & 1) == 0)
                continue;

            /* raw discovery reply: preamble, separator, encoded UID */
            if (uchar(data.at(0)) != 0xCC)
            {
                reply = data.left(HID_DMX_RDM_CHUNK);
                break;
            }

            int length = uchar(data.at(2)) + 2; // message length + checksum
            int mask = (1 << ((length + HID_DMX_RDM_CHUNK - 1) / HID_DMX_RDM_CHUNK)) - 1;
            if ((received & mask) != mask)
                continue;

            /* a late reply to a previous request: keep waiting for ours */
            if (discovery == false && data.at(15) != request.frame.at(15))
            {
                data.clear();
                received = 0;
                continue;
            }

            reply = data.left(length);
            break;
        }
    }

    if (reply.isEmpty())
        emit rdmTimeout(request.universe, request.line, request.type);
    else
        emit rdmFrameReceived(request.universe, request.line, reply);
}

/*****************************************************************************
 * HID DMX - specific functions / driver
 *****************************************************************************/

void HIDDMXDevice::updateMode()
{
    /**
    *  Send chosen mode to the HID DMX device
    */
    unsigned char driver_mode = 0;
    if (m_mode & DMX_MODE_OUTPUT)
        driver_mode += 2;
    if (m_mode & DMX_MODE_INPUT)
        driver_mode += 4;
    if (m_mode & DMX_MODE_MERGER)
        driver_mode += 1;

    unsigned char buffer[34];

    memset(buffer, 0, 34);
    buffer[1] = 16;
    buffer[2] = driver_mode;

    m_writeMutex.lock();
    hid_write(m_handle, buffer, 34);
    m_writeMutex.unlock();

    /**
    *  Start / stop the device thread based on whether the input is activated
    *  (DMX input polling) or the output of an RDM capable dongle (RDM requests)
    */
    if ((m_mode & DMX_MODE_INPUT) || (hasRDM() && (m_mode & DMX_MODE_OUTPUT)))
    {
        m_running = true;
        start();
    }
    else if (isRunning() == true)
    {
        m_rdmMutex.lock();
        m_running = false;
        m_rdmCondition.wakeAll();
        m_rdmMutex.unlock();
        wait();
        m_rdmQueue.clear();
        m_rdmActive = false;
    }
}
