/*
  Q Light Controller
  hpmprivate-iokit.cpp

  Copyright (C) Heikki Junnila

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

#include <QDebug>

#include "hpmprivate-iokit.h"
#include "hotplugmonitor.h"

/****************************************************************************
 * Static callback functions for IOKit
 ****************************************************************************/

void onHPMPrivateRawDeviceAdded(void* refCon, io_iterator_t iterator)
{
    HPMPrivate* self = (HPMPrivate*) refCon;
    Q_ASSERT(self != NULL);
    self->deviceAdded(iterator);
}

void onHPMPrivateRawDeviceRemoved(void* refCon, io_iterator_t iterator)
{
    HPMPrivate* self = (HPMPrivate*) refCon;
    Q_ASSERT(self != NULL);
    self->deviceRemoved(iterator);
}

/****************************************************************************
 * HPMPrivate implementation
 ****************************************************************************/

HPMPrivate::HPMPrivate(HotPlugMonitor* parent)
    : QThread(parent)
    , m_run(false)
    , loop(NULL)
{
}

HPMPrivate::~HPMPrivate()
{
    if (isRunning() == true)
        stop();
    loop = NULL;
}

void HPMPrivate::stop()
{
    CFRunLoopStop(loop);
    while (isRunning() == true)
        usleep(10);
}

void HPMPrivate::extractVidPid(io_service_t usbDevice, UInt16* vid, UInt16* pid)
{
    Q_ASSERT(vid != NULL);
    Q_ASSERT(pid != NULL);

    CFNumberRef number;

    number = (CFNumberRef) IORegistryEntryCreateCFProperty(usbDevice, CFSTR(kUSBVendorID),
                                                           kCFAllocatorDefault, 0);
    CFNumberGetValue(number, kCFNumberSInt16Type, vid);
    CFRelease(number);

    number = (CFNumberRef) IORegistryEntryCreateCFProperty(usbDevice, CFSTR(kUSBProductID),
                                                           kCFAllocatorDefault, 0);
    CFNumberGetValue(number, kCFNumberSInt16Type, pid);
    CFRelease(number);
}

void HPMPrivate::deviceAdded(io_iterator_t iterator)
{
    io_service_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator)) != 0)
    {
        UInt16 vid = 0, pid = 0;
        extractVidPid(usbDevice, &vid, &pid);
        HotPlugMonitor* hpm = qobject_cast<HotPlugMonitor*> (parent());
        Q_ASSERT(hpm != NULL);
        hpm->emitDeviceAdded(vid, pid);
        IOObjectRelease(usbDevice);
    }
}

void HPMPrivate::deviceRemoved(io_iterator_t iterator)
{
    io_service_t usbDevice;
    while ((usbDevice = IOIteratorNext(iterator)))
    {
        UInt16 vid = 0, pid = 0;
        extractVidPid(usbDevice, &vid, &pid);
        HotPlugMonitor* hpm = qobject_cast<HotPlugMonitor*> (parent());
        Q_ASSERT(hpm != NULL);
        hpm->emitDeviceRemoved(vid, pid);
        IOObjectRelease(usbDevice);
    }
}

void HPMPrivate::run()
{
    mach_port_t masterPort = 0;
    IONotificationPortRef notifyPort = 0;
    io_iterator_t rawAddedIter = 0;
    io_iterator_t rawRemovedIter = 0;

    // Create an IOMainPort for accessing IOKit
    kern_return_t kr = IOMainPort(MACH_PORT_NULL, &masterPort);
    if (kr || !masterPort)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create a master I/O Kit port" << kr;
        return;
    }

    // Create a new dictionary for matching device classes
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
    if (!matchingDict)
    {
        qWarning() << Q_FUNC_INFO << "Unable to create a USB matching dictionary";
        mach_port_deallocate(mach_task_self(), masterPort);
        return;
    }

    // Take an extra reference because IOServiceAddMatchingNotification consumes one
    matchingDict = (CFMutableDictionaryRef) CFRetain(matchingDict);

    // Store the thread's run loop context
    loop = CFRunLoopGetCurrent();
    // New notification port
    notifyPort = IONotificationPortCreate(masterPort);

    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notifyPort);
    CFRunLoopAddSource(loop, runLoopSource, kCFRunLoopDefaultMode);

    // Listen to device add notifications
    kr = IOServiceAddMatchingNotification(notifyPort,
                                          kIOFirstMatchNotification,
                                          matchingDict,
                                          onHPMPrivateRawDeviceAdded,
                                          (void*) this,
                                          &rawAddedIter);
    if (kr != kIOReturnSuccess)
        qFatal("Unable to add notification for device additions");

    // Iterate over set of matching devices to access already-present devices
    // and to arm the notification.
    onHPMPrivateRawDeviceAdded(this, rawAddedIter);

    // Listen to device removal notifications
    kr = IOServiceAddMatchingNotification(notifyPort,
                                          kIOTerminatedNotification,
                                          matchingDict,
                                          onHPMPrivateRawDeviceRemoved,
                                          (void*) this,
                                          &rawRemovedIter);
    if (kr != kIOReturnSuccess)
        qFatal("Unable to add notification for device termination");

    // Iterate over set of matching devices to release each one and to
    // arm the notification.
    onHPMPrivateRawDeviceRemoved(this, rawRemovedIter);

    // No longer needed
    mach_port_deallocate(mach_task_self(), masterPort);
    masterPort = 0;

    // Start the run loop inside this thread. The thread "stops" here.
    CFRunLoopRun();

    // Destroy the notification port when the thread exits
    IONotificationPortDestroy(notifyPort);
    notifyPort = 0;
}
