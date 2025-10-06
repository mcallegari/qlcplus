/*
  Q Light Controller Plus
  usbdmxlegacy.h

  Adds support for the "USB/DMX Interface" (usbdmx.com) custom protocol
  as documented in "USB/DMX Interface - Command Specification (V 1.4)".

  Copyright (C) 2025

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt
*/

#ifndef USBDMXLEGACY_H
#define USBDMXLEGACY_H

#include "dmxusbwidget.h"

/**
 * Implements the usbdmx.com interface protocol (Ben Suffolk, v1.4 PDF).
 * The device is FTDI-based and maintains its own DMX timing once TX is ON.
 * We only push per-channel updates and set the "last TX channel" bound.
 */
class UsbdmxLegacy : public DMXUSBWidget
{
public:
    UsbdmxLegacy(DMXInterface *iface, quint32 outputLine, QObject *parent = nullptr);
    ~UsbdmxLegacy();

    /** @reimp */
    DMXUSBWidget::Type type() const override;

    /*************************************************************************
     * Open & Close
     *************************************************************************/
    bool open(quint32 line=0, bool input=false) override;
    bool close(quint32 line=0, bool input=false) override;

    /*************************************************************************
     * Outputs
     *************************************************************************/
    bool writeUniverse(quint32 universe, quint32 output,
                       const QByteArray& data, bool dataChanged) override;

    /*************************************************************************
     * Info
     *************************************************************************/
    QString additionalInfo() const override;

private:
    // usbdmx.com command helpers
    bool cmdTxOn();
    bool cmdTxOff();
    bool cmdSetLastChannel(int lastIdx);        // argument in [0..511]
    bool cmdSetChannelValue(int idx, uchar val);// idx in [0..511]

    inline uchar lo(int idx) const { return static_cast<uchar>(idx & 0xFF); }
    inline bool hiBit(int idx) const { return (idx & 0x100) != 0; }
};

#endif // USBDMXLEGACY_H