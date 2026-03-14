/*
  Q Light Controller
  ftd2xx-android-interface.cpp

  Copyright (C) Massimo Callegari

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

#include "ftd2xx-interface.h"

#include <QJniEnvironment>
#include <QJniObject>
#include <QDebug>

namespace
{
const char *D2XX_MANAGER_CLASS = "com/ftdi/j2xx/D2xxManager";
const char *QT_NATIVE_CLASS = "org/qtproject/qt/android/QtNative";

bool clearJniException(const char *where)
{
    QJniEnvironment env;
    if (!env->ExceptionCheck())
        return false;

    qWarning() << where << "raised a JNI exception";
    env->ExceptionDescribe();
    env->ExceptionClear();
    return true;
}

QJniObject androidContext()
{
    QJniObject context = QJniObject::callStaticObjectMethod(
        QT_NATIVE_CLASS, "context", "()Landroid/content/Context;");
    clearJniException("QtNative.context");
    return context;
}

QJniObject d2xxManager(const QJniObject &context)
{
    if (!context.isValid())
        return QJniObject();

    QJniObject manager = QJniObject::callStaticObjectMethod(
        D2XX_MANAGER_CLASS,
        "getInstance",
        "(Landroid/content/Context;)Lcom/ftdi/j2xx/D2xxManager;",
        context.object<jobject>());
    clearJniException("D2xxManager.getInstance");
    return manager;
}

jbyteArray toJByteArray(QJniEnvironment &env, const QByteArray &data)
{
    jbyteArray array = env->NewByteArray(data.size());
    if (array == NULL)
        return NULL;

    env->SetByteArrayRegion(array, 0, data.size(), reinterpret_cast<const jbyte*>(data.constData()));
    return array;
}
}

FTD2XXInterface::FTD2XXInterface(const QString& serial, const QString& name, const QString& vendor,
                                 quint16 VID, quint16 PID, quint32 id)
    : DMXInterface(serial, name, vendor, VID, PID, id)
    , m_handle(NULL)
{
}

FTD2XXInterface::~FTD2XXInterface()
{
    if (isOpen())
        close();
}

DMXInterface::Type FTD2XXInterface::type() const
{
    return DMXInterface::FTD2xx;
}

QString FTD2XXInterface::typeString() const
{
    return "FTD2xx";
}

QList<DMXInterface *> FTD2XXInterface::interfaces(QList<DMXInterface *> discoveredList)
{
    QList<DMXInterface *> interfacesList;

    QJniObject context = androidContext();
    QJniObject manager = d2xxManager(context);
    if (!context.isValid() || !manager.isValid())
    {
        qWarning() << Q_FUNC_INFO << "Cannot access Android context or D2XX manager";
        return interfacesList;
    }

    jint num = manager.callMethod<jint>("createDeviceInfoList",
                                        "(Landroid/content/Context;)I",
                                        context.object<jobject>());
    clearJniException("D2xxManager.createDeviceInfoList");
    if (num <= 0)
        return interfacesList;

    for (jint i = 0; i < num; ++i)
    {
        QJniObject info = manager.callObjectMethod(
            "getDeviceInfoListDetail",
            "(I)Lcom/ftdi/j2xx/D2xxManager$FtDeviceInfoListNode;",
            i);
        clearJniException("D2xxManager.getDeviceInfoListDetail");
        if (!info.isValid())
            continue;

        QString serial = info.getObjectField<jstring>("serialNumber").toString();
        QString name = info.getObjectField<jstring>("description").toString();
        jint usbId = info.getField<jint>("id");
        quint16 vid = quint16((usbId >> 16) & 0xFFFF);
        quint16 pid = quint16(usbId & 0xFFFF);

        if (serial.isEmpty())
            serial = QString("ANDROID-%1").arg(i);
        if (name.isEmpty())
            name = QString("FTDI Device %1").arg(i);
        if (vid == 0)
            vid = DMXInterface::FTDIVID;
        if (pid == 0)
            pid = DMXInterface::FTDIPID;

        if (validInterface(vid, pid) == false)
            continue;

        QString vendor = "FTDI";

        bool found = false;
        for (int c = 0; c < discoveredList.count(); c++)
        {
            if (discoveredList.at(c)->checkInfo(serial, name, vendor))
            {
                found = true;
                break;
            }
        }

        if (!found)
            interfacesList << new FTD2XXInterface(serial, name, vendor, vid, pid, i);
    }

    return interfacesList;
}

bool FTD2XXInterface::open()
{
    if (isOpen())
        return true;

    QJniObject context = androidContext();
    QJniObject manager = d2xxManager(context);
    if (!context.isValid() || !manager.isValid())
        return false;

    QJniObject device = manager.callObjectMethod(
        "openByIndex",
        "(Landroid/content/Context;I)Lcom/ftdi/j2xx/FT_Device;",
        context.object<jobject>(), jint(id()));
    clearJniException("D2xxManager.openByIndex");
    if (!device.isValid())
    {
        qWarning() << Q_FUNC_INFO << "Failed to open D2XX device with index" << id();
        return false;
    }

    if (!device.callMethod<jboolean>("isOpen"))
    {
        clearJniException("FT_Device.isOpen");
        qWarning() << Q_FUNC_INFO << "D2XX device is not open. USB permission may be missing.";
        return false;
    }

    jbyte latency = device.callMethod<jbyte>("getLatencyTimer");
    if (clearJniException("FT_Device.getLatencyTimer"))
        m_defaultLatency = 16;
    else
        m_defaultLatency = static_cast<unsigned char>(latency);

    m_handle = new QJniObject(device);
    return true;
}

bool FTD2XXInterface::openByPID(const int PID)
{
    Q_UNUSED(PID)
    return open();
}

bool FTD2XXInterface::close()
{
    if (m_handle == NULL)
        return true;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    if (device->isValid())
        device->callMethod<void>("close");
    clearJniException("FT_Device.close");

    delete device;
    m_handle = NULL;
    return true;
}

bool FTD2XXInterface::isOpen() const
{
    if (m_handle == NULL)
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    if (!device->isValid())
        return false;

    bool isOpen = device->callMethod<jboolean>("isOpen");
    clearJniException("FT_Device.isOpen");
    return isOpen;
}

bool FTD2XXInterface::reset()
{
    if (!isOpen())
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = device->callMethod<jboolean>("resetDevice");
    clearJniException("FT_Device.resetDevice");
    return ok;
}

bool FTD2XXInterface::setLineProperties()
{
    if (!isOpen())
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = device->callMethod<jboolean>(
        "setDataCharacteristics", "(BBB)Z",
        jbyte(8), jbyte(2), jbyte(0));
    clearJniException("FT_Device.setDataCharacteristics");
    return ok;
}

bool FTD2XXInterface::setBaudRate()
{
    if (!isOpen())
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = device->callMethod<jboolean>("setBaudRate", "(I)Z", jint(250000));
    clearJniException("FT_Device.setBaudRate");
    return ok;
}

bool FTD2XXInterface::setFlowControl()
{
    if (!isOpen())
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = device->callMethod<jboolean>(
        "setFlowControl", "(SBB)Z",
        jshort(0), jbyte(0), jbyte(0));
    clearJniException("FT_Device.setFlowControl");
    return ok;
}

bool FTD2XXInterface::setLowLatency(bool lowLatency)
{
    if (!isOpen())
        return false;

    unsigned char latency = lowLatency ? 1 : m_defaultLatency;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = device->callMethod<jboolean>("setLatencyTimer", "(B)Z", jbyte(latency));
    clearJniException("FT_Device.setLatencyTimer");
    return ok;
}

bool FTD2XXInterface::clearRts()
{
    if (!isOpen())
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = device->callMethod<jboolean>("clrRts");
    clearJniException("FT_Device.clrRts");
    return ok;
}

bool FTD2XXInterface::purgeBuffers()
{
    if (!isOpen())
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = device->callMethod<jboolean>("purge", "(B)Z", jbyte(0x01 | 0x02));
    clearJniException("FT_Device.purge");
    return ok;
}

bool FTD2XXInterface::setBreak(bool on)
{
    if (!isOpen())
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    bool ok = on ? device->callMethod<jboolean>("setBreakOn")
                 : device->callMethod<jboolean>("setBreakOff");
    clearJniException(on ? "FT_Device.setBreakOn" : "FT_Device.setBreakOff");
    return ok;
}

bool FTD2XXInterface::write(const QByteArray& data)
{
    if (!isOpen())
        return false;

    QJniEnvironment env;
    jbyteArray jdata = toJByteArray(env, data);
    if (jdata == NULL)
        return false;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    jint written = device->callMethod<jint>("write", "([B)I", jdata);
    clearJniException("FT_Device.write");
    env->DeleteLocalRef(jdata);

    return (written == data.size());
}

QByteArray FTD2XXInterface::read(int size)
{
    QByteArray result;
    if (!isOpen() || size <= 0)
        return result;

    QJniEnvironment env;
    jbyteArray jbuffer = env->NewByteArray(size);
    if (jbuffer == NULL)
        return result;

    QJniObject *device = static_cast<QJniObject*>(m_handle);
    jint readBytes = device->callMethod<jint>("read", "([BI)I", jbuffer, jint(size));
    clearJniException("FT_Device.read");

    if (readBytes > 0)
    {
        result.resize(readBytes);
        env->GetByteArrayRegion(jbuffer, 0, readBytes, reinterpret_cast<jbyte*>(result.data()));
    }

    env->DeleteLocalRef(jbuffer);
    return result;
}

uchar FTD2XXInterface::readByte(bool* ok)
{
    QByteArray bytes = read(1);
    if (bytes.size() == 1)
    {
        if (ok != NULL)
            *ok = true;
        return uchar(bytes.at(0));
    }

    if (ok != NULL)
        *ok = false;
    return 0;
}
