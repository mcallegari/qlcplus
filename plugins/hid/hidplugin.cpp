/*
  Q Light Controller
  hidplugin.cpp

  Copyright (c) Heikki Junnila

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

#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QDir>

#include "configurehid.h"
#include "hiddmxdevice.h"
#include "hidjsdevice.h"
#include "hidapi.h"
#include "hidplugin.h"

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
  #include "hidlinuxjoystick.h"
#elif defined(WIN32) || defined (Q_OS_WIN)
  #include "win32/hidwindowsjoystick.h"
#elif defined (__APPLE__) || defined(Q_OS_MACOS)
  #include "hidosxjoystick.h"
#endif

/*****************************************************************************
 * HID Initialization
 *****************************************************************************/

void HIDPlugin::init()
{
    rescanDevices();
}

HIDPlugin::~HIDPlugin()
{
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();
}

QString HIDPlugin::name()
{
    return QString("HID");
}

int HIDPlugin::capabilities() const
{
    return QLCIOPlugin::Input | QLCIOPlugin::Output;
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

bool HIDPlugin::openInput(quint32 input, quint32 universe)
{
    HIDDevice* dev = device(input);
    if (dev != NULL)
    {
        connect(dev, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        addToMap(universe, input, Input);
        return dev->openInput();
    }
    else
        qDebug() << name() << "has no input number:" << input;
    return false;
}

void HIDPlugin::closeInput(quint32 input, quint32 universe)
{
    HIDDevice* dev = device(input);
    if (dev != NULL)
    {
        removeFromMap(input, universe, Input);
        dev->closeInput();
        disconnect(dev, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                   this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
    }
    else
        qDebug() << name() << "has no input number:" << input;
}

QStringList HIDPlugin::inputs()
{
    QStringList list;

    QListIterator <HIDDevice*> it(m_devices);
    while (it.hasNext() == true)
    {
        HIDDevice* dev = it.next();
        if (dev->hasInput())
            list << dev->name();
    }

    return list;
}

QString HIDPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides support for HID-based joysticks, gamepads and some USB DMX adapters.");
    str += QString("</P>");

    return str;
}

QString HIDPlugin::inputInfo(quint32 input)
{
    QString str;

    if (input != QLCIOPlugin::invalidLine())
    {
        /* A specific input line selected. Display its information if
           available. */
        HIDDevice* dev = device(input);
        if (dev != NULL)
            str += dev->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/*********************************************************************
 * Outputs
 *********************************************************************/
bool HIDPlugin::openOutput(quint32 output, quint32 universe)
{
    HIDDevice* dev = deviceOutput(output);
    if (dev != NULL)
    {
        addToMap(universe, output, Output);
        return dev->openOutput();
    }
    else
        qDebug() << name() << "has no output number:" << output;
    return false;
}

void HIDPlugin::closeOutput(quint32 output, quint32 universe)
{
    HIDDevice* dev = deviceOutput(output);
    if (dev != NULL)
    {
        removeFromMap(output, universe, Output);
        dev->closeOutput();
    }
    else
        qDebug() << name() << "has no output number:" << output;
}

QStringList HIDPlugin::outputs()
{
    QStringList list;

    QListIterator <HIDDevice*> it(m_devices);
    while (it.hasNext() == true)
    {
        HIDDevice* dev = it.next();
        if (dev->hasOutput())
            list << dev->name();
    }

    return list;
}

QString HIDPlugin::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine())
    {
        /* A specific output line selected. Display its information if
           available. */
        HIDDevice* dev = deviceOutput(output);
        if (dev != NULL)
            str += dev->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void HIDPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (output != QLCIOPlugin::invalidLine())
    {
        HIDDevice *dev = deviceOutput(output);
        if (dev != NULL)
            dev->outputDMX(data);
    }
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void HIDPlugin::configure()
{
    ConfigureHID conf(NULL, this);
    conf.exec();
}

bool HIDPlugin::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void HIDPlugin::rescanDevices()
{
    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QList <HIDDevice*> destroyList(m_devices);
    int devCount = m_devices.count();

    struct hid_device_info *devs, *cur_dev;
    quint32 line = 0;

    devs = hid_enumerate(0x0, 0x0);

    cur_dev = devs;

    while (cur_dev)
    {
        //qDebug() << "[HID Device found] path:" << QString(cur_dev->path) << ", name:" << QString::fromWCharArray(cur_dev->product_string);

        HIDDevice* dev = device(QString(cur_dev->path));
        if (dev != NULL)
        {
            /** Device already exists, delete from remove list */
            destroyList.removeAll(dev);
        }
        else if ((cur_dev->vendor_id == HID_DMX_INTERFACE_VENDOR_ID
                && cur_dev->product_id == HID_DMX_INTERFACE_PRODUCT_ID) ||
                (cur_dev->vendor_id == HID_DMX_INTERFACE_VENDOR_ID_2
                && cur_dev->product_id == HID_DMX_INTERFACE_PRODUCT_ID_2) ||
                (cur_dev->vendor_id == HID_DMX_INTERFACE_VENDOR_ID_3
                && cur_dev->product_id == HID_DMX_INTERFACE_PRODUCT_ID_3) ||
                (cur_dev->vendor_id == HID_DMX_INTERFACE_VENDOR_ID_4
                && cur_dev->product_id == HID_DMX_INTERFACE_PRODUCT_ID_4))
        {
            /* Device is a USB DMX Interface, add it */
            dev = new HIDDMXDevice(this, line++,
                                   QString::fromWCharArray(cur_dev->manufacturer_string) + " " +
                                   QString::fromWCharArray(cur_dev->product_string) + " " +
                                   "(" + QString::fromWCharArray(cur_dev->serial_number) + ")",
                                   QString(cur_dev->path));
            addDevice(dev);
        }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
        else if (QString(cur_dev->path).contains("js"))
        {
            dev = new HIDLinuxJoystick(this, line++, cur_dev);
#elif defined(WIN32) || defined (Q_OS_WIN)
        else if (HIDWindowsJoystick::isJoystick(cur_dev->vendor_id, cur_dev->product_id) == true)
        {
            dev = new HIDWindowsJoystick(this, line++, cur_dev);
#elif defined (__APPLE__) || defined(Q_OS_MACOS)
        else if (HIDOSXJoystick::isJoystick(cur_dev->usage) == true)
        {
            dev = new HIDOSXJoystick(this, line++, cur_dev);
#endif
            addDevice(dev);
        }

        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);

    /* Destroy those devices that were no longer found. */
    while (destroyList.isEmpty() == false)
    {
        HIDDevice* dev = destroyList.takeFirst();
        m_devices.removeAll(dev);
        delete dev;
    }

    if (m_devices.count() != devCount)
        emit configurationChanged();
}

HIDDevice* HIDPlugin::device(const QString& path)
{
    QListIterator <HIDDevice*> it(m_devices);

    while (it.hasNext() == true)
    {
        HIDDevice* dev = it.next();
        if (dev->path() == path)
            return dev;
    }

    return NULL;
}

HIDDevice* HIDPlugin::device(quint32 index)
{
    if (index < quint32(m_devices.count()))
        return m_devices.at(index);
    else
        return NULL;
}

HIDDevice* HIDPlugin::deviceOutput(quint32 index)
{
    QListIterator <HIDDevice*> it(m_devices);
    quint32 pos = 0;
    while (it.hasNext() == true)
    {
        HIDDevice* dev = it.next();
        if (dev->hasOutput())
        {
            if (pos == index)
                return dev;
            else
                pos++;
        }
    }
    return NULL;
}

void HIDPlugin::addDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    m_devices.append(device);
    emit deviceAdded(device);

    emit configurationChanged();
}

void HIDPlugin::removeDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    m_devices.removeAll(device);

    emit deviceRemoved(device);
    delete device;

    emit configurationChanged();
}

/*****************************************************************************
 * Hotplug
 *****************************************************************************/

void HIDPlugin::slotDeviceAdded(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);

    rescanDevices();
}

void HIDPlugin::slotDeviceRemoved(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);

    rescanDevices();
}
