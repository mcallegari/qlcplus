/*
  Q Light Controller Plus
  hiddmxdevice.h

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

#ifndef HIDDMXDEVICE_H
#define HIDDMXDEVICE_H

#include <QObject>
#include <QMutex>
#include <QAtomicPointer>
#include <QWaitCondition>

#include "hiddevice.h"
#include "hidapi.h"

// Vendor IDs provided by http://www.linux-usb.org/usb.ids

#define HID_DMX_INTERFACE_VENDOR_ID    0x04B4  // Cypress Semiconductor Corp.
#define HID_DMX_INTERFACE_PRODUCT_ID   0x0F1F  // Digital Enlightenment USB DMX
#define HID_DMX_INTERFACE_VENDOR_ID_2  0x16C0  // Van Ooijen Technische Informatica
#define HID_DMX_INTERFACE_PRODUCT_ID_2 0x088B  // FX5
#define HID_DMX_INTERFACE_VENDOR_ID_3  0x16D0  // MCS
#define HID_DMX_INTERFACE_PRODUCT_ID_3 0x0830  // DMXControl Projects e.V. Nodle U1
#define HID_DMX_INTERFACE_VENDOR_ID_4  0x16D0  // MCS
#define HID_DMX_INTERFACE_PRODUCT_ID_4 0x0833  // DMXControl Projects e.V. Nodle R4S

#define HID_DMX_READ_TIMEOUT 100

/* RDM specific */
#define HID_DMX_RDM_CHUNK        31     // RDM bytes per report
#define HID_DMX_RDM_MAX_CHUNKS   17
#define HID_DMX_RDM_INDEX_BASE   204    // report byte 0 = 204 + chunk index
#define HID_DMX_RDM_TIMEOUT      100    // ms to wait for a reply
#define HID_DMX_RDM_KEEPALIVE    250    // ms between keep alive requests

class HIDPlugin;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDDMXDevice final : public HIDDevice
{
    Q_OBJECT

public:
    HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString& name, const QString& path);
    virtual ~HIDDMXDevice();

protected:
    /** Initialize the device, find out its capabilities etc. */
    void init();

    /** @reimp */
    bool hasInput() const override { return true; }

    /** @reimp */
    bool hasOutput() const override { return true; }

    /** @reimp */
    bool hasMergerMode() const override { return true; /*DE, FX5, and Nodle have a merger mode*/ }

    /*********************************************************************
     * File operations
     *********************************************************************/
public:

    /** @reimp */
    bool isMergerModeEnabled() const override;

    /** @reimp */
    void enableMergerMode(bool mergerModeEnabled) override;

    /** @reimp */
    bool openInput() override;

    /** @reimp */
    void closeInput() override;

    /** @reimp */
    bool openOutput() override;

    /** @reimp */
    void closeOutput() override;

    /** @reimp */
    bool readEvent() override;

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    /** @reimp */
    QString infoText() const override;

    /*********************************************************************
     * Input data
     *********************************************************************/
public:
    /** @reimp */
    void feedBack(quint32 channel, uchar value) override;

private:
    /** @reimp */
    void run() override;

    /** Process a 33 bytes DMX input report: [chunk index][32 channel values] */
    void handleInputReport(const unsigned char *buffer);

    /*********************************************************************
     * Output data
     *********************************************************************/
public:
    /** @reimp */
    void outputDMX(const QByteArray &data, bool forceWrite = false) override;

private:
    /** Serialize the writes coming from the DMX output, the mode
     *  changes and the RDM thread (hidapi shares state per handle) */
    QMutex m_writeMutex;

    /*********************************************************************
     * RDM
     *
     * Report: [report ID 0][204 + chunk index][32 bytes]. The RDM frame
     * (start code to checksum) is split in 31 bytes chunks, each one
     * encoded to 32 bytes by the codec installed by the RDM service.
     * Replies use the same format and are read with GET_REPORT: 0xFF
     * means no data. A discovery reply comes back raw (preamble included)
     * in chunk 0. After RDM traffic, the dongle reboots if the host stays
     * quiet for about a second, so the thread keeps polling it.
     *********************************************************************/
public:
    /** @reimp */
    bool hasRDM() const override { return m_rdm && m_rdmCodec.loadAcquire() != nullptr; }

    /** @reimp */
    bool sendRDMFrame(quint32 universe, quint32 line, int type, const QByteArray &frame) override;

    /** @reimp */
    void setRDMCodec(QLCRDMCodec *codec) override;

private:
    struct RDMRequest
    {
        quint32 universe;
        quint32 line;
        int type;
        QByteArray frame;
    };

    /** Write a complete RDM frame as encoded chunks */
    void writeRDMFrame(QLCRDMCodec *codec, const QByteArray &frame);

    /** GET_REPORT one input report. Returns the chunk index and its
     *  decoded bytes in data (if codec is set), or -1 if the dongle has no data */
    int readRDMChunk(QLCRDMCodec *codec, QByteArray &data);

    /** Send a queued request and wait for its reply. Device thread only */
    void processRDMRequest(const RDMRequest &request);

    /** true if the dongle supports RDM */
    bool m_rdm;

    /** The RDM payload codec, set by the RDM service */
    QAtomicPointer<QLCRDMCodec> m_rdmCodec;

    /** true once RDM has been used: the dongle needs keep alive requests */
    bool m_rdmActive;

    QList<RDMRequest> m_rdmQueue;
    QMutex m_rdmMutex;
    QWaitCondition m_rdmCondition;

     /*********************************************************************
     * FX5 - specific functions and device handle
     *********************************************************************/
private:
    /** Interface mode specification */
    enum DMXmode
    {
        DMX_MODE_NONE   = 1 << 0,
        DMX_MODE_OUTPUT = 1 << 1,
        DMX_MODE_INPUT  = 1 << 2,
        DMX_MODE_MERGER = 1 << 3
    };

    /** mode selection function */
    void updateMode();

    /** The device current open mode */
    int m_mode;

    /** Last universe data that has been received */
    QByteArray m_dmx_in_cmp;

    /** Last universe data that has been output */
    QByteArray m_dmx_cmp;

    /** device handle for the interface */
    hid_device *m_handle;
};

#endif
