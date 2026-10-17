/*
  Q Light Controller Plus
  qlcrdmservice.h

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

#ifndef QLCRDMSERVICE_H
#define QLCRDMSERVICE_H

#include <QtPlugin>
#include <QMetaType>
#include <QObject>
#include <QStringList>
#include <QString>
#include <QList>

class QLCIOPlugin;

/** @addtogroup engine Engine
 * @{
 */

/** The id under which the RDM service plugin registers itself */
#define QLCRDM_SERVICE_ID "rdm"

/** A patched output line which can carry RDM frames */
struct QLCRDMLine
{
    quint32 universe;       //! QLC+ universe id
    quint32 line;           //! output line within the I/O plugin
    QLCIOPlugin *plugin;    //! the I/O plugin owning the physical link
};

/** A device found by the RDM discovery */
struct QLCRDMDevice
{
    QString uid;
    QString manufacturer;
    QString model;
    quint32 universe;
    quint32 pluginLine;
    quint16 address;
    quint16 channels;
    QString personality;        //! description of the active DMX personality
    quint8 personalityIndex;    //! index of the active DMX personality
    quint8 personalityCount;    //! number of DMX personalities
    QString ipAddress;          //! network address of the device (Art-Net only), empty if unknown
};

/**
 * RDM specific wiring, implemented by the engine (Doc) and NOT by
 * QLCServiceHost. This is the only way the RDM service reaches the hardware:
 * through the already loaded I/O plugins, which own the serial port / UDP
 * socket the RDM frames have to share with the normal DMX output.
 */
class QLCRDMTransportProvider
{
public:
    virtual ~QLCRDMTransportProvider() { }

    /** Patched output lines whose I/O plugin supports QLCIOPlugin::RDM */
    virtual QList<QLCRDMLine> lines() = 0;

    /** The I/O plugin to be used for the given universe and output line,
     *  or nullptr if not found / not RDM capable */
    virtual QLCIOPlugin *transport(quint32 universe, quint32 line) = 0;
};

/**
 * The RDM feature interface, implemented by the RDM service plugin
 * and consumed by the user interface.
 *
 * This is a plain interface (no QObject inheritance, no Q_OBJECT) on purpose:
 * the header is compiled both into QLC+ and into the closed plugin, and a
 * moc'd class would exist twice with different meta objects. The interface
 * is identified by its IID, which carries the interface version, so that
 *   qobject_cast<QLCRDMService*>(obj)
 * safely fails for a plugin built against a different version.
 *
 * The object implementing the interface must also be a QObject and must
 * emit the following signals (consumers connect with the string based
 * SIGNAL() syntax on the QObject):
 *
 *   void deviceFound(QLCRDMDevice device);   // a device was discovered/updated
 *   void deviceInfoReady(QString html);      // full device info, as HTML
 *   void pidResultReady(QString text);       // result of a single PID read/write
 *   void message(QString title, QString text); // something to tell the user
 *   void busyChanged(bool busy);             // requests started / all ended
 *
 * Both QLC+ and the plugin must call qRegisterMetaType<QLCRDMDevice>()
 * before using deviceFound across threads.
 */
class QLCRDMService
{
public:
    /** Kind of the argument passed to readPid/writePid */
    enum PidDataType
    {
        ByteArg = 0,
        ShortArg,
        LongArg,
        ArrayArg
    };

    virtual ~QLCRDMService() { }

    /** Set the object used to reach the I/O plugins.
     *  Called by the engine right after the service has been loaded. */
    virtual void setTransportProvider(QLCRDMTransportProvider *provider) = 0;

    /** Run a discovery on all the RDM capable lines */
    virtual void discoverAll() = 0;

    /** Set the DMX start address of a discovered device. The address is
     *  only sent to the device by executeOperations() */
    virtual void requestDMXAddress(const QString &uid, int address) = 0;

    /** Assign the DMX start addresses to the given devices, in the given order: the first
     *  one gets startAddress, the next ones follow with gap free channels in between.
     *  Addresses are assigned per universe, since every device belongs to one.
     *  The addresses are only sent to the devices by executeOperations().
     *  deviceFound is emitted for every device that has been given a new address. */
    virtual void autoPatch(const QStringList &orderedUIDs, int startAddress, int gap) = 0;

    /** Discard the pending operations (e.g. addresses not sent yet) of the given
     *  devices, restoring their original values. deviceFound is emitted for every
     *  device that has been restored */
    virtual void cancelOperations(const QStringList &uids) = 0;

    /** Send to the devices all the pending operations (see requestDMXAddress).
     *  deviceFound is emitted for every device that has been updated */
    virtual void executeOperations() = 0;

    /** Download all the information of a previously discovered device */
    virtual void queryDevice(const QString &uid) = 0;

    /** Read a single PID. @param args user text, comma separated */
    virtual void readPid(const QString &uid, const QString &pid,
                         int dataType, const QString &args) = 0;

    /** Write a single PID. @param args user text, comma separated */
    virtual void writePid(const QString &uid, const QString &pid,
                          int dataType, const QString &args) = 0;
};

#define QLCRDMService_iid "org.qlcplus.QLCRDMService/1"

Q_DECLARE_INTERFACE(QLCRDMService, QLCRDMService_iid)
Q_DECLARE_METATYPE(QLCRDMDevice)

/** @} */

#endif
