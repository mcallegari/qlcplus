/*
  Q Light Controller Plus
  hidosxjoystick.cpp

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
#include <QDebug>

#include "hidosxjoystick.h"

HIDOSXJoystick::HIDOSXJoystick(HIDPlugin *parent, quint32 line, struct hid_device_info *info)
    : HIDJsDevice(parent, line, info)
{
    init();
}

bool HIDOSXJoystick::isJoystick(unsigned short usage)
{
    if (usage == kHIDUsage_GD_Joystick || usage == kHIDUsage_GD_GamePad ||
        usage == kHIDUsage_GD_MultiAxisController || usage == kHIDUsage_GD_Hatswitch)
        return true;

    return false;
}

static int32_t get_int_property(IOHIDDeviceRef device, CFStringRef key)
{
    CFTypeRef ref;
    int32_t value;

    ref = IOHIDDeviceGetProperty(device, key);
    if (ref) {
        if (CFGetTypeID(ref) == CFNumberGetTypeID()) {
            CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value);
            return value;
        }
    }
    return 0;
}

void HIDOSXJoystick::init()
{
    m_HIDManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    IOHIDManagerOpen(m_HIDManager, kIOHIDOptionsTypeNone);
    /* Get a list of the Devices */
    IOHIDManagerSetDeviceMatching(m_HIDManager, NULL);
    CFSetRef device_set = IOHIDManagerCopyDevices(m_HIDManager);

    /* Convert the list into a C array so we can iterate easily. */
    CFIndex num_devices = CFSetGetCount(device_set);
    IOHIDDeviceRef *device_array = (IOHIDDeviceRef *)calloc(num_devices, sizeof(IOHIDDeviceRef));
    CFSetGetValues(device_set, (const void **) device_array);

    m_IOKitDevice = 0;
    m_axesNumber = 0;
    m_buttonsNumber = 0;

    for (int d = 0; d < num_devices; d++)
    {
        unsigned short vid, pid;

        vid = get_int_property(device_array[d], CFSTR(kIOHIDVendorIDKey));
        pid = get_int_property(device_array[d], CFSTR(kIOHIDProductIDKey));
        if (m_dev_info->vendor_id == vid && m_dev_info->product_id == pid)
        {
            // store the reference of the IOKit HID device
            m_IOKitDevice = device_array[d];
            break;
        }
    }

    if (m_IOKitDevice == 0)
        return;

    // to return all elements for a device
    CFArrayRef elementsArray = IOHIDDeviceCopyMatchingElements(m_IOKitDevice, NULL, kIOHIDOptionsTypeNone);
    if (elementsArray)
    {
        CFIndex count = CFArrayGetCount(elementsArray);

        //qDebug() << "Device" << QString::fromWCharArray(m_dev_info->product_string) << "has elements:" << count;

        for (int i = 0; i < count; i++)
        {
            IOHIDElementRef elem = (IOHIDElementRef)CFArrayGetValueAtIndex(elementsArray, i);
            IOHIDElementType elemType = IOHIDElementGetType(elem);
            unsigned int usage = IOHIDElementGetUsage(elem);
            unsigned int usagePage = IOHIDElementGetUsagePage(elem);

            if (elemType == kIOHIDElementTypeInput_Misc ||
               elemType == kIOHIDElementTypeInput_Button ||
               elemType == kIOHIDElementTypeInput_Axis ||
               elemType == kIOHIDElementTypeInput_ScanCodes)
            {
                switch(usagePage)
                {
                    case kHIDPage_GenericDesktop:
                        switch(usage)
                        {
                            case kHIDUsage_GD_X:
                            case kHIDUsage_GD_Y:
                            case kHIDUsage_GD_Z:
                            case kHIDUsage_GD_Rx:
                            case kHIDUsage_GD_Ry:
                            case kHIDUsage_GD_Rz:
                            case kHIDUsage_GD_Slider:
                            case kHIDUsage_GD_Dial:
                            case kHIDUsage_GD_Wheel:
                            {
                                HIDInfo axis;
                                axis.IOHIDRef = elem;
                                axis.value = 0;
                                m_axesValues.append(axis);
                                m_axesNumber++;
                            }
                            break;
                            default:
                            break;
                        }
                    break;
                    case kHIDPage_Button:
                    {
                        HIDInfo button;
                        button.IOHIDRef = elem;
                        button.value = 0;
                        m_buttonsValues.append(button);
                        m_buttonsNumber++;
                    }
                    break;
                    default:
                    break;
                }
            }
        }
        CFRelease(elementsArray);
    }
    free(device_array);
    CFRelease(device_set);
}

bool HIDOSXJoystick::readEvent()
{
    IOHIDValueRef valueRef;
    int value;

    if (m_IOKitDevice == 0)
        return false;

    for (int b = 0; b < m_buttonsNumber; b++)
    {
        IOHIDDeviceGetValue(m_IOKitDevice, m_buttonsValues[b].IOHIDRef, &valueRef);
        value = IOHIDValueGetIntegerValue(valueRef);

        if (value != m_buttonsValues[b].value)
        {
            uchar val;
            if (value != 0)
                val = UCHAR_MAX;
            else
                val = 0;

            emit valueChanged(UINT_MAX, m_line, m_axesNumber + b, val);
            m_buttonsValues[b].value = value;
        }
    }
    for (int a = 0; a < m_axesNumber; a++)
    {
        IOHIDDeviceGetValue(m_IOKitDevice, m_axesValues[a].IOHIDRef, &valueRef);
        value = IOHIDValueGetIntegerValue(valueRef);

        if (value != m_axesValues[a].value)
        {
            emit valueChanged(UINT_MAX, m_line, a, uchar(value));
            m_axesValues[a].value = value;
        }
    }

    return true;
}

