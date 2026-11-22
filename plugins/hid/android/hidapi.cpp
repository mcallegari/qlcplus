/*
  Q Light Controller Plus
  hidapi.cpp (Android backend)

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

/*
 * Android apps cannot open /dev/hidraw*. Devices are enumerated and opened
 * through AndroidUSB (Java USB Host API), then reports are exchanged on the
 * interrupt endpoints when available, otherwise with SET_REPORT/GET_REPORT
 * control transfers.
 */

#include <QDebug>

#include <linux/usb/ch9.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "hidapi.h"
#include "hidandroid.h"
#include "qlcandroidusb.h"
#include "hiddmxdevice.h"

#define HID_IO_TIMEOUT_MS   1000
#define HID_MAX_PACKET      1024

#define HID_REPORT_TYPE_OUTPUT  0x02
#define HID_REPORT_TYPE_FEATURE 0x03
#define HID_REQ_GET_REPORT      0x01
#define HID_REQ_SET_REPORT      0x09

struct hid_device_
{
    AndroidUSB::Handle handle;
    bool blocking;
    QString path;
};

/* Devices for which USB permission is requested on enumeration */
static const QVector<int> s_wantedIds =
{
    HID_DMX_INTERFACE_VENDOR_ID,   HID_DMX_INTERFACE_PRODUCT_ID,
    HID_DMX_INTERFACE_VENDOR_ID_2, HID_DMX_INTERFACE_PRODUCT_ID_2,
    HID_DMX_INTERFACE_VENDOR_ID_3, HID_DMX_INTERFACE_PRODUCT_ID_3,
    HID_DMX_INTERFACE_VENDOR_ID_4, HID_DMX_INTERFACE_PRODUCT_ID_4
};

void hid_android_set_listener(QObject *listener)
{
    AndroidUSB::addListener(listener, s_wantedIds);
}

static wchar_t *toWide(const QString &str)
{
    wchar_t *wstr = (wchar_t *)calloc(str.length() + 1, sizeof(wchar_t));
    str.toWCharArray(wstr);
    return wstr;
}

static int reportTransfer(hid_device *dev, bool in, quint8 reportType, unsigned char *data, size_t length)
{
    if (dev == NULL || data == NULL || length == 0)
        return -1;

    /* Unnumbered reports: the leading 0x00 is not sent on the wire */
    const unsigned char reportId = data[0];
    unsigned char *buf = reportId == 0 ? data + 1 : data;
    size_t len = reportId == 0 ? length - 1 : length;

    int res = AndroidUSB::controlTransfer(dev->handle,
                    USB_TYPE_CLASS | USB_RECIP_INTERFACE | (in ? USB_DIR_IN : USB_DIR_OUT),
                    in ? HID_REQ_GET_REPORT : HID_REQ_SET_REPORT,
                    (reportType << 8) | reportId, dev->handle.interfaceNumber,
                    buf, len, HID_IO_TIMEOUT_MS);
    if (res < 0)
        return -1;

    return reportId == 0 ? res + 1 : res;
}

int HID_API_EXPORT hid_init(void)
{
    return 0;
}

int HID_API_EXPORT hid_exit(void)
{
    return 0;
}

struct hid_device_info HID_API_EXPORT *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
    struct hid_device_info *root = NULL;
    struct hid_device_info *last = NULL;

    const QList<AndroidUSB::DeviceInfo> devices = AndroidUSB::enumerate(USB_CLASS_HID, s_wantedIds);
    for (const AndroidUSB::DeviceInfo &dev : devices)
    {
        if ((vendor_id != 0 && dev.vendorID != vendor_id) ||
            (product_id != 0 && dev.productID != product_id))
            continue;

        struct hid_device_info *info = (struct hid_device_info *)calloc(1, sizeof(struct hid_device_info));
        info->path = strdup(dev.path.toUtf8().constData());
        info->vendor_id = dev.vendorID;
        info->product_id = dev.productID;
        info->interface_number = dev.interfaceNumber;
        info->manufacturer_string = toWide(dev.manufacturer);
        info->product_string = toWide(dev.product);
        info->serial_number = toWide(dev.serial);

        if (last)
            last->next = info;
        else
            root = info;
        last = info;
    }

    return root;
}

void HID_API_EXPORT hid_free_enumeration(struct hid_device_info *devs)
{
    while (devs)
    {
        struct hid_device_info *next = devs->next;
        free(devs->path);
        free(devs->serial_number);
        free(devs->manufacturer_string);
        free(devs->product_string);
        free(devs);
        devs = next;
    }
}

hid_device * hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
    struct hid_device_info *devs = hid_enumerate(vendor_id, product_id);
    hid_device *handle = NULL;

    for (struct hid_device_info *cur = devs; cur && handle == NULL; cur = cur->next)
    {
        if (serial_number == NULL || wcscmp(serial_number, cur->serial_number) == 0)
            handle = hid_open_path(cur->path);
    }

    hid_free_enumeration(devs);
    return handle;
}

hid_device * HID_API_EXPORT hid_open_path(const char *path)
{
    if (path == NULL)
        return NULL;

    hid_device *dev = new hid_device;
    dev->path = QString::fromUtf8(path);
    dev->blocking = true;

    if (AndroidUSB::open(dev->path, dev->handle) == false)
    {
        delete dev;
        return NULL;
    }

    if (dev->handle.inMaxPacket <= 0 || dev->handle.inMaxPacket > HID_MAX_PACKET)
        dev->handle.inMaxPacket = 64;

    return dev;
}

int HID_API_EXPORT hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
    if (dev == NULL || data == NULL || length == 0)
        return -1;

    if (dev->handle.epOut == 0)
        return reportTransfer(dev, false, HID_REPORT_TYPE_OUTPUT, (unsigned char *)data, length);

    /* Unnumbered reports: the leading 0x00 is not sent on the wire */
    const unsigned char reportId = data[0];
    const unsigned char *buf = reportId == 0 ? data + 1 : data;
    size_t len = reportId == 0 ? length - 1 : length;

    int res = AndroidUSB::bulkTransfer(dev->handle, dev->handle.epOut, (void *)buf, len, HID_IO_TIMEOUT_MS);
    if (res < 0)
        return -1;

    return reportId == 0 ? res + 1 : res;
}

int HID_API_EXPORT hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
    if (dev == NULL || data == NULL || dev->handle.epIn == 0)
        return -1;

    unsigned char buf[HID_MAX_PACKET];
    /* usbdevfs: 0 means wait forever */
    int timeout = milliseconds < 0 ? 0 : (milliseconds == 0 ? 1 : milliseconds);

    int res = AndroidUSB::bulkTransfer(dev->handle, dev->handle.epIn, buf, dev->handle.inMaxPacket, timeout);
    if (res < 0)
        return (errno == ETIMEDOUT) ? 0 : -1;

    res = qMin(res, int(length));
    memcpy(data, buf, res);

    return res;
}

int HID_API_EXPORT hid_read(hid_device *dev, unsigned char *data, size_t length)
{
    if (dev == NULL)
        return -1;

    return hid_read_timeout(dev, data, length, dev->blocking ? -1 : 0);
}

int HID_API_EXPORT hid_set_nonblocking(hid_device *dev, int nonblock)
{
    if (dev == NULL)
        return -1;

    dev->blocking = !nonblock;
    return 0;
}

int HID_API_EXPORT hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
    return reportTransfer(dev, false, HID_REPORT_TYPE_FEATURE, (unsigned char *)data, length);
}

int HID_API_EXPORT hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
    return reportTransfer(dev, true, HID_REPORT_TYPE_FEATURE, data, length);
}

void HID_API_EXPORT hid_close(hid_device *dev)
{
    if (dev == NULL)
        return;

    AndroidUSB::close(dev->path);
    delete dev;
}

int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *, wchar_t *, size_t)
{
    return -1;
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *, wchar_t *, size_t)
{
    return -1;
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *, wchar_t *, size_t)
{
    return -1;
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *, int, wchar_t *, size_t)
{
    return -1;
}

HID_API_EXPORT const wchar_t * HID_API_CALL hid_error(hid_device *)
{
    return NULL;
}
