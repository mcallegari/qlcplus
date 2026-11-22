/*
  Q Light Controller Plus
  qlcandroidusb.cpp

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

#include <QtCore/qcoreapplication_platform.h>
#include <QJniEnvironment>
#include <QJniObject>
#include <QStringList>
#include <QPointer>
#include <QDebug>

#include <linux/usbdevice_fs.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "qlcandroidusb.h"

#define USB_JAVA_CLASS "org/qlcplus/usb/UsbHelper"

typedef void (*ListenerCallback)(jlong context, jint vid, jint pid, jboolean added);

/* Registered by every plugin library on the same Java class: since it only
 * calls the function pointer it receives, any library's copy will do */
static void dispatch(JNIEnv *, jclass, jlong callback, jlong context, jint vid, jint pid, jboolean added)
{
    ListenerCallback cb = reinterpret_cast<ListenerCallback>(static_cast<intptr_t>(callback));
    if (cb != NULL)
        cb(context, vid, pid, added);
}

/* Called on the Android main thread */
static void notifyListener(jlong context, jint vid, jint pid, jboolean added)
{
    QPointer<QObject> *listener = reinterpret_cast<QPointer<QObject> *>(static_cast<intptr_t>(context));
    if (listener == NULL || listener->isNull())
        return;

    QMetaObject::invokeMethod(listener->data(), added ? "slotDeviceAdded" : "slotDeviceRemoved",
                              Qt::QueuedConnection, Q_ARG(uint, uint(vid)), Q_ARG(uint, uint(pid)));
}

static QJniObject appContext()
{
    return QNativeInterface::QAndroidApplication::context();
}

static jintArray toJIntArray(QJniEnvironment &env, const QVector<int> &ids)
{
    jintArray array = env->NewIntArray(ids.count());
    QVector<jint> values(ids.begin(), ids.end());
    env->SetIntArrayRegion(array, 0, values.count(), values.constData());
    return array;
}

bool AndroidUSB::addListener(QObject *listener, const QVector<int> &ids)
{
    static bool nativesRegistered = false;
    QJniEnvironment env;

    if (nativesRegistered == false)
    {
        if (!env.registerNativeMethods(USB_JAVA_CLASS, {
                { "dispatch", "(JJIIZ)V", reinterpret_cast<void *>(dispatch) } }))
        {
            qWarning() << "[AndroidUSB] Cannot register native methods on" << USB_JAVA_CLASS;
            return false;
        }
        nativesRegistered = true;
    }

    // intentionally never deleted: Java may call back until the application quits
    QPointer<QObject> *context = new QPointer<QObject>(listener);
    jintArray jids = toJIntArray(env, ids);

    QJniObject::callStaticMethod<void>(USB_JAVA_CLASS, "addListener", "(Landroid/content/Context;JJ[I)V",
                                       appContext().object(),
                                       jlong(reinterpret_cast<intptr_t>(&notifyListener)),
                                       jlong(reinterpret_cast<intptr_t>(context)), jids);
    env->DeleteLocalRef(jids);

    return env.checkAndClearExceptions() == false;
}

QList<AndroidUSB::DeviceInfo> AndroidUSB::enumerate(int interfaceClass, const QVector<int> &ids)
{
    QList<DeviceInfo> list;
    QJniEnvironment env;
    jintArray jids = toJIntArray(env, ids);

    QJniObject res = QJniObject::callStaticObjectMethod(USB_JAVA_CLASS, "enumerate",
                            "(Landroid/content/Context;I[I)Ljava/lang/String;",
                            appContext().object(), jint(interfaceClass), jids);
    env->DeleteLocalRef(jids);

    if (env.checkAndClearExceptions() || res.isValid() == false)
        return list;

    const QStringList lines = res.toString().split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        QStringList fields = line.split('\t');
        if (fields.count() < 7)
            continue;

        DeviceInfo info;
        info.path = fields.at(0);
        info.vendorID = fields.at(1).toUShort();
        info.productID = fields.at(2).toUShort();
        info.interfaceNumber = fields.at(3).toInt();
        info.manufacturer = fields.at(4);
        info.product = fields.at(5);
        info.serial = fields.at(6);
        list.append(info);
    }

    return list;
}

bool AndroidUSB::open(const QString &path, Handle &handle)
{
    QJniEnvironment env;
    QJniObject jpath = QJniObject::fromString(path);

    QJniObject res = QJniObject::callStaticObjectMethod(USB_JAVA_CLASS, "open",
                            "(Landroid/content/Context;Ljava/lang/String;)[I",
                            appContext().object(), jpath.object<jstring>());

    if (env.checkAndClearExceptions() || res.isValid() == false)
    {
        qWarning() << "[AndroidUSB] Unable to open" << path;
        return false;
    }

    jintArray array = res.object<jintArray>();
    if (env->GetArrayLength(array) < 9)
        return false;

    jint values[9];
    env->GetIntArrayRegion(array, 0, 9, values);

    handle.fd = values[0];
    handle.epIn = values[1];
    handle.epOut = values[2];
    handle.inMaxPacket = values[3];
    handle.outMaxPacket = values[4];
    handle.interfaceNumber = values[5];
    handle.interfaceCount = values[6];
    handle.bcdDevice = values[7];
    handle.commInterface = values[8];

    return true;
}

void AndroidUSB::close(const QString &path)
{
    QJniObject jpath = QJniObject::fromString(path);
    QJniObject::callStaticMethod<void>(USB_JAVA_CLASS, "close", "(Ljava/lang/String;)V",
                                       jpath.object<jstring>());
    QJniEnvironment env;
    env.checkAndClearExceptions();
}

int AndroidUSB::bulkTransfer(const Handle &handle, int endpoint, void *data, int length, int timeoutMs)
{
    struct usbdevfs_bulktransfer xfer;
    xfer.ep = endpoint;
    xfer.len = length;
    xfer.timeout = timeoutMs;
    xfer.data = data;

    return ioctl(handle.fd, USBDEVFS_BULK, &xfer);
}

int AndroidUSB::controlTransfer(const Handle &handle, quint8 requestType, quint8 request,
                                quint16 value, quint16 index, void *data, quint16 length, int timeoutMs)
{
    struct usbdevfs_ctrltransfer ctrl;
    ctrl.bRequestType = requestType;
    ctrl.bRequest = request;
    ctrl.wValue = value;
    ctrl.wIndex = index;
    ctrl.wLength = length;
    ctrl.timeout = timeoutMs;
    ctrl.data = data;

    return ioctl(handle.fd, USBDEVFS_CONTROL, &ctrl);
}
