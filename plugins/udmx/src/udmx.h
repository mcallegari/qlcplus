/*
  Q Light Controller
  udmx.h

  Copyright (c)	Lutz Hillebrand
                Heikki Junnila

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

#ifndef UDMX_H
#define UDMX_H

#include <QStringList>
#include <QList>

#include "qlcioplugin.h"

struct libusb_device;
class UDMXDevice;

class UDMX : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~UDMX();

    /** @reimp */
    void init() override;

    /** @reimp */
    QString name() override;

    /** @reimp */
    int capabilities() const override;

    /** @reimp */
    QString pluginInfo() override;

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output, quint32 universe) override;

    /** @reimp */
    void closeOutput(quint32 output, quint32 universe) override;

    /** @reimp */
    QStringList outputs() override;

    /** @reimp */
    QString outputInfo(quint32 output) override;

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged) override;

private:
    /** Attempt to find all uDMX devices */
    void rescanDevices();

    /** Get a UDMXDevice entry by its usbdev struct */
    UDMXDevice* device(libusb_device *usbdev);

private:
    struct libusb_context* m_ctx;

    /** List of available devices */
    QList <UDMXDevice*> m_devices;

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure() override;

    /** @reimp */
    bool canConfigure() override;
};

#endif
