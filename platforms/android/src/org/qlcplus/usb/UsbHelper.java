/*
  Q Light Controller Plus
  UsbHelper.java

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

package org.qlcplus.usb;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.os.Build;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * USB Host API helper shared by the I/O plugins (see plugins/interfaces/android/androidusb.cpp).
 * Android apps cannot access /dev/hidraw* or /dev/bus/usb/* directly, so devices are
 * enumerated, granted and opened here, and the native side performs I/O on the
 * connection fd. A device path is "<UsbDevice name>:<interface id>".
 */
public class UsbHelper
{
    private static final String ACTION_USB_PERMISSION = "org.qlcplus.usb.USB_PERMISSION";

    private static final Map<String, UsbDeviceConnection> s_connections = new HashMap<>();
    private static final Set<String> s_permissionAsked = new HashSet<>();
    private static final List<Listener> s_listeners = new ArrayList<>();
    private static boolean s_receiverRegistered = false;

    /** A native listener: @callback is a native function pointer invoked with @context */
    private static class Listener
    {
        long callback;
        long context;
        int[] ids;
    }

    /**
     * Implemented identically by every plugin library (androidusb.cpp): it calls
     * the native @callback function pointer, so it doesn't matter which library
     * registered it last.
     */
    private static native void dispatch(long callback, long context, int vid, int pid, boolean added);

    private static final BroadcastReceiver s_receiver = new BroadcastReceiver()
    {
        @Override
        @SuppressWarnings("deprecation")
        public void onReceive(Context context, Intent intent)
        {
            UsbDevice dev = Build.VERSION.SDK_INT >= 33 ?
                    intent.getParcelableExtra(UsbManager.EXTRA_DEVICE, UsbDevice.class) :
                    intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (dev == null)
                return;

            String action = intent.getAction();
            boolean added = true;

            if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action))
            {
                synchronized (UsbHelper.class)
                {
                    s_permissionAsked.remove(dev.getDeviceName());
                }
                added = false;
            }
            else if (ACTION_USB_PERMISSION.equals(action) &&
                     intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false) == false)
            {
                return;
            }

            List<Listener> listeners;
            synchronized (UsbHelper.class)
            {
                listeners = new ArrayList<>(s_listeners);
            }

            for (Listener l : listeners)
            {
                if (matches(l.ids, dev))
                    dispatch(l.callback, l.context, dev.getVendorId(), dev.getProductId(), added);
            }
        }
    };

    /** Return true if @dev matches one of the VID/PID pairs in @ids */
    private static boolean matches(int[] ids, UsbDevice dev)
    {
        for (int i = 0; ids != null && i + 1 < ids.length; i += 2)
        {
            if (dev.getVendorId() == ids[i] && dev.getProductId() == ids[i + 1])
                return true;
        }
        return false;
    }

    private static synchronized void init(Context ctx)
    {
        if (s_receiverRegistered)
            return;

        IntentFilter filter = new IntentFilter();
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        filter.addAction(ACTION_USB_PERMISSION);

        Context appCtx = ctx.getApplicationContext();
        if (Build.VERSION.SDK_INT >= 33)
            appCtx.registerReceiver(s_receiver, filter, Context.RECEIVER_NOT_EXPORTED);
        else
            appCtx.registerReceiver(s_receiver, filter);

        s_receiverRegistered = true;
    }

    /** Be notified of attach/detach/permission events of devices matching the VID/PID pairs in @ids */
    public static synchronized void addListener(Context ctx, long callback, long context, int[] ids)
    {
        init(ctx);

        Listener l = new Listener();
        l.callback = callback;
        l.context = context;
        l.ids = ids;
        s_listeners.add(l);
    }

    private static String sanitize(String s)
    {
        if (s == null)
            return "";
        return s.replace('\t', ' ').replace('\n', ' ');
    }

    /**
     * Return one line per accessible interface of class @ifaceClass (-1 = any)
     * of the devices matching the VID/PID pairs in @ids:
     * path \t vid \t pid \t interface \t manufacturer \t product \t serial
     * Permission is requested (once per attach) for matching devices,
     * which are skipped until permission is granted.
     */
    public static synchronized String enumerate(Context ctx, int ifaceClass, int[] ids)
    {
        UsbManager mgr = (UsbManager)ctx.getSystemService(Context.USB_SERVICE);
        StringBuilder sb = new StringBuilder();

        if (mgr == null)
            return "";

        init(ctx);

        for (UsbDevice dev : mgr.getDeviceList().values())
        {
            if (matches(ids, dev) == false)
                continue;

            if (mgr.hasPermission(dev) == false)
            {
                if (s_permissionAsked.add(dev.getDeviceName()))
                {
                    Intent intent = new Intent(ACTION_USB_PERMISSION).setPackage(ctx.getPackageName());
                    int flags = Build.VERSION.SDK_INT >= 31 ? PendingIntent.FLAG_MUTABLE : 0;
                    mgr.requestPermission(dev, PendingIntent.getBroadcast(ctx, 0, intent, flags));
                }
                continue;
            }

            String serial = "";
            try { serial = dev.getSerialNumber(); } catch (SecurityException e) { }

            for (int i = 0; i < dev.getInterfaceCount(); i++)
            {
                UsbInterface intf = dev.getInterface(i);
                if (ifaceClass >= 0 && intf.getInterfaceClass() != ifaceClass)
                    continue;

                sb.append(dev.getDeviceName()).append(':').append(intf.getId()).append('\t')
                  .append(dev.getVendorId()).append('\t')
                  .append(dev.getProductId()).append('\t')
                  .append(intf.getId()).append('\t')
                  .append(sanitize(dev.getManufacturerName())).append('\t')
                  .append(sanitize(dev.getProductName())).append('\t')
                  .append(sanitize(serial)).append('\n');
            }
        }

        return sb.toString();
    }

    /**
     * Open and claim the interface identified by @path.
     * Return { fd, epIn, epOut, inMaxPacket, outMaxPacket, interface, interfaceCount, bcdDevice,
     * commInterface } (endpoint address 0 = not present, commInterface -1 = not a CDC data
     * interface) or null on failure.
     * Endpoints are the first bulk or interrupt IN/OUT ones of the interface.
     */
    public static synchronized int[] open(Context ctx, String path)
    {
        UsbManager mgr = (UsbManager)ctx.getSystemService(Context.USB_SERVICE);
        int sep = path.lastIndexOf(':');
        if (mgr == null || sep < 0)
            return null;

        UsbDevice dev = mgr.getDeviceList().get(path.substring(0, sep));
        if (dev == null || mgr.hasPermission(dev) == false)
            return null;

        int ifaceId = Integer.parseInt(path.substring(sep + 1));
        UsbInterface intf = null;
        for (int i = 0; i < dev.getInterfaceCount(); i++)
        {
            if (dev.getInterface(i).getId() == ifaceId)
            {
                intf = dev.getInterface(i);
                break;
            }
        }
        if (intf == null)
            return null;

        UsbDeviceConnection conn = mgr.openDevice(dev);
        if (conn == null)
            return null;

        // force = true detaches the kernel driver
        if (conn.claimInterface(intf, true) == false)
        {
            conn.close();
            return null;
        }

        // CDC data interface: claim its communication interface too, the one
        // receiving the class requests (line coding, control lines, break)
        int commIfaceId = -1;
        if (intf.getInterfaceClass() == UsbConstants.USB_CLASS_CDC_DATA)
        {
            UsbInterface comm = null;
            for (int i = 0; i < dev.getInterfaceCount(); i++)
            {
                UsbInterface ui = dev.getInterface(i);
                if (ui.getInterfaceClass() == UsbConstants.USB_CLASS_COMM && ui.getId() < ifaceId &&
                    (comm == null || ui.getId() > comm.getId()))
                    comm = ui;
            }

            if (comm != null && conn.claimInterface(comm, true))
                commIfaceId = comm.getId();
        }

        int epIn = 0, epOut = 0, inMax = 0, outMax = 0;
        for (int i = 0; i < intf.getEndpointCount(); i++)
        {
            UsbEndpoint ep = intf.getEndpoint(i);
            if (ep.getType() != UsbConstants.USB_ENDPOINT_XFER_INT &&
                ep.getType() != UsbConstants.USB_ENDPOINT_XFER_BULK)
                continue;

            if (ep.getDirection() == UsbConstants.USB_DIR_IN && epIn == 0)
            {
                epIn = ep.getAddress();
                inMax = ep.getMaxPacketSize();
            }
            else if (ep.getDirection() == UsbConstants.USB_DIR_OUT && epOut == 0)
            {
                epOut = ep.getAddress();
                outMax = ep.getMaxPacketSize();
            }
        }

        // bcdDevice from the raw device descriptor (bytes 12-13)
        int bcdDevice = 0;
        byte[] desc = conn.getRawDescriptors();
        if (desc != null && desc.length >= 14)
            bcdDevice = (desc[12] & 0xFF) | ((desc[13] & 0xFF) << 8);

        UsbDeviceConnection old = s_connections.put(path, conn);
        if (old != null)
            old.close();

        return new int[] { conn.getFileDescriptor(), epIn, epOut, inMax, outMax,
                           ifaceId, dev.getInterfaceCount(), bcdDevice, commIfaceId };
    }

    public static synchronized void close(String path)
    {
        UsbDeviceConnection conn = s_connections.remove(path);
        if (conn != null)
            conn.close();
    }
}
