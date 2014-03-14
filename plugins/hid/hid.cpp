/*
  Q Light Controller
  hid.cpp

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
#include "hidfx5device.h"
#include "hidjsdevice.h"
#include "hidapi.h"
#include "hid.h"

/*****************************************************************************
 * HIDInputEvent
 *****************************************************************************/

static const QEvent::Type _HIDInputEventType = static_cast<QEvent::Type>
        (QEvent::registerEventType());

HIDInputEvent::HIDInputEvent(HIDDevice* device, quint32 input,
                             quint32 channel, uchar value,
                             bool alive) : QEvent(_HIDInputEventType)
{
    m_device = device;
    m_input = input;
    m_channel = channel;
    m_value = value;
    m_alive = alive;
}

HIDInputEvent::~HIDInputEvent()
{
}

/*****************************************************************************
 * HID Initialization
 *****************************************************************************/

void HID::init()
{
    rescanDevices();
}

HID::~HID()
{
    while (m_devices.isEmpty() == false)
        delete m_devices.takeFirst();
}

QString HID::name()
{
    return QString("HID");
}

int HID::capabilities() const
{
    return QLCIOPlugin::Input | QLCIOPlugin::Output;
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

void HID::openInput(quint32 input)
{
    HIDDevice* dev = device(input);
    if (dev != NULL)
    {
        dev->openInput();
        connect(dev, SIGNAL(valueChanged(quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,uchar)));
    }
    else
        qDebug() << name() << "has no input number:" << input;
}

void HID::closeInput(quint32 input)
{
    HIDDevice* dev = device(input);
    if (dev != NULL)
    {
        dev->closeInput();
        disconnect(dev, SIGNAL(valueChanged(quint32,quint32,uchar)),
                   this, SIGNAL(valueChanged(quint32,quint32,uchar)));
    }
    else
        qDebug() << name() << "has no input number:" << input;
}

QStringList HID::inputs()
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

void HID::customEvent(QEvent* event)
{
    if (event->type() == _HIDInputEventType)
    {
        HIDInputEvent* e = static_cast<HIDInputEvent*> (event);
        if (e != NULL && e->m_alive == true)
            emit valueChanged(e->m_input, e->m_channel, e->m_value);
        else
            removeDevice(e->m_device);

        event->accept();
    }
}

QString HID::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides support for HID-based joysticks and the FX5 USB DMX adapter.");
    str += QString("</P>");

    return str;
}

QString HID::inputInfo(quint32 input)
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
void HID::openOutput(quint32 output)
{
    HIDDevice* dev = device(output);
    if (dev != NULL)
        dev->openOutput();
    else
        qDebug() << name() << "has no output number:" << output;
}

void HID::closeOutput(quint32 output)
{
    HIDDevice* dev = device(output);
    if (dev != NULL)
        dev->closeOutput();
    else
        qDebug() << name() << "has no output number:" << output;
}

QStringList HID::outputs()
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

QString HID::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine())
    {
        /* A specific output line selected. Display its information if
           available. */
        HIDDevice* dev = device(output);
        if (dev != NULL)
            str += dev->infoText();
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void HID::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe);

    if (output != QLCIOPlugin::invalidLine())
    {
        HIDDevice* dev = device(output);
        if (dev != NULL)
            dev->outputDMX(data);
    }
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void HID::configure()
{
    ConfigureHID conf(NULL, this);
    conf.exec();
}

bool HID::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void HID::rescanDevices()
{
    /* Treat all devices as dead first, until we find them again. Those
       that aren't found, get destroyed at the end of this function. */
    QList <HIDDevice*> destroyList(m_devices);

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
        else if((cur_dev->vendor_id == FX5_DMX_INTERFACE_VENDOR_ID
                && cur_dev->product_id == FX5_DMX_INTERFACE_PRODUCT_ID) ||
                (cur_dev->vendor_id == FX5_DMX_INTERFACE_VENDOR_ID_2
                && cur_dev->product_id == FX5_DMX_INTERFACE_PRODUCT_ID_2))
        {
            /* Device is a FX5 / Digital Enlightenment USB DMX Interface, add it */
            dev = new HIDFX5Device(this, line++,
                                   QString::fromWCharArray(cur_dev->manufacturer_string) + " " +
                                   QString::fromWCharArray(cur_dev->product_string),
                                   QString(cur_dev->path));
            addDevice(dev);
        }
#if !defined (__APPLE__) && !defined(Q_OS_MACX)
        else
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
            if (QString(cur_dev->path).contains("js"))
#elif defined(WIN32) || defined (Q_OS_WIN)
            if(HIDJsDevice::isJoystick(cur_dev->vendor_id, cur_dev->product_id) == true)
#endif
        {
            dev = new HIDJsDevice(this, line++,
                                  QString::fromWCharArray(cur_dev->manufacturer_string) + " " +
                                  QString::fromWCharArray(cur_dev->product_string),
                                  QString(cur_dev->path));
            addDevice(dev);
        }
#endif
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
    
    emit configurationChanged();
}

HIDDevice* HID::device(const QString& path)
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

HIDDevice* HID::device(quint32 index)
{
    if (index < quint32(m_devices.count()))
        return m_devices.at(index);
    else
        return NULL;
}

void HID::addDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    m_devices.append(device);
    emit deviceAdded(device);

    emit configurationChanged();
}

void HID::removeDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    m_devices.removeAll(device);

    emit deviceRemoved(device);
    delete device;

    emit configurationChanged();
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(hid, HID)
#endif
