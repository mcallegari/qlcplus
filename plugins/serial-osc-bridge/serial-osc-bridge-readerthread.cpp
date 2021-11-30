/*
  Q Light Controller Plus

  serial-osc-bridge-settings.cpp

  Copyright (c) House Gordon Software Company LTD

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


// GNU_SOURCE before <string.h> is needed for memmem(3) function
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QUdpSocket>

#include "serial-osc-bridge-utils.h"
#include "serial-osc-bridge-readerthread.h"
#include "serial-osc-bridge-plugin.h"

#define DEBUG_PREFIX "[SerialOSCBridge-ReaderThread] "


/*
The Serial-OSC-Bridge Reader Thread is started the moment the plugin
is connected to a universe (see OpenInput function in the main plugin code ).

It has three main states, corresponding to the three member functions:
   THREAD_STATUS_WAIT_FOR_SETTINGS => ReadThread::wait_for_settings()
   THREAD_STATUS_WAIT_FOR_DEVICE   => ReadThread::wait_for_serial_device()
   THREAD_STATUS_PROCESS_INPUT     => ReadThread::process_input()

Each of the three function runs a loop doing its thing,
until either there's a request to terminate the thread (isInterruptionRequested())
or if the settings changed and we need to go back to the first state "Wait_for_settings"
(based on the m_settings_changed variable).

The main thread function ReadThread::run() is in charge of switching
between the states (or existing).

This not-so-clean implementation is due to two main factors:
1. There is no clean way to know when the QLC infrastructure
   finished loading all the plugin settings (and, the order in
   which the settings are loaded is not defined).

   If there was a notification (QT Signal, function call, etc.)
   of "all settings were loaded" - the would be a natural place
   to start the thread, with all the settings already known.
   But there's no such notification.

   So this thread starts early, and in a loop waits until all required settings
   have been set (critical settings which are required to continue are
   USB VendorID, Product ID, and baud rate). Once we got those, we can
   continue to the next start of trying to detect the requested USB device.

2. This plugin supports dynamic insertion/removal of the USB device,
   without causing any problem to the rest of QLC.
   It will simply act as if the OSC input has gone silent.
   When the USB device is removed, the thread will go back to the "wait for serial device" state.
   When the USB device is re-inserted, the thread will continue processing data.
*/

ReadThread::ReadThread(QObject *parent)
    : QThread(parent)
    , thread_status(THREAD_STATUS_INITIALIZING)
    , wait_for_settings_loop_count(0)
    , input_bytes_count(0)
    , valid_frames_count(0)
    , valid_frames_bytes_count(0)
    , buffer_overflow_count(0)
    , m_input_device(NULL)
    , m_first_frame (false)
{
  m_input_data.reserve(m_settings.max_input_buffer_size);
}

ReadThread::~ReadThread()
{
}

void ReadThread::stop_request()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO;
  requestInterruption();
}

void ReadThread::changeSettings(const SerialOscBridgeSettings& new_settings)
{
  QMutexLocker locker(&m_mutex);
  qDebug() << DEBUG_PREFIX << "Settings Changed" ;
  m_new_settings = new_settings ;
  m_settings_changed = 1;
}

QSerialPort* ReadThread::OpenSerialPort(const QString& portName)
{
  QSerialPort *s = new QSerialPort;

  s->setPortName(portName);
  s->setBaudRate(m_settings.baudrate);
  s->setDataBits(QSerialPort::Data8);
  s->setParity(QSerialPort::NoParity);
  s->setStopBits(QSerialPort::OneStop);
  s->setFlowControl(QSerialPort::NoFlowControl);
  bool ok = s->open(QIODevice::ReadOnly);

  if (!ok) {
    qWarning() << DEBUG_PREFIX "Failed to open serial device " << portName << " error: " << s->error();
    delete s;
    return NULL;
  }

  qDebug() << DEBUG_PREFIX << "Opened Serial Device " << portName ;
  return s;
}

void ReadThread::wait_for_settings()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [START]" ;

  thread_status = THREAD_STATUS_WAIT_FOR_SETTINGS;
  wait_for_settings_loop_count = 0;

  while ( !isInterruptionRequested() )
    {
      {
	QMutexLocker locker(&m_mutex);
	// This is the only place were "m_settings_pending" is accessed
	// by the reader thread, and this is done with the mutex locked.
	//
	// The rest of the functions in the read use "m_settings"
	// which will not be changed directly by the main (plugin) thread.
	if (m_new_settings.input_values_ready()) {
	  m_settings = m_new_settings;
	  m_settings_changed = 0;
	  break;
	}
      }
      wait_for_settings_loop_count++;

      // arbitrary delay, to avoid busy-loop and 100% cpu for no good benefit.
      // Longer values would mean slightly slower UI respond (if the user close the plugin
      // and triggers thread cancellation during this sleep).
      //
      // This sleep is shorter than in 'wait_for_serial_device' as
      // the configuration phase should be quick and we want to be responsive.
      msleep(100);
    }

  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [END]" ;
}

void ReadThread::debug_print_serialportinfo(const QSerialPortInfo &pi)
{
  QString s;
  QTextStream o(&s);
  o << pi.portName();
  o << " (" << pi.description() << ")";

  if (pi.hasVendorIdentifier())
    o << " Vendor-ID " << int_to_hex(pi.vendorIdentifier());
  else
    o << " No-Vendor-ID" ;

  if (pi.hasProductIdentifier())
    o << " Product-ID " << int_to_hex(pi.productIdentifier());
  else
    o << " No-Product-ID" ;

  qDebug() << DEBUG_PREFIX " Found Serial Device: "  << s;
}

QList<QSerialPortInfo> ReadThread::getDetectedSerialPorts()
{
  QList<QSerialPortInfo> copy;
  {
    // Make a deep-copy of the list, (hopefully?) working-around QT's implicit sharing.
    // Then we return the data (in the calling main/plugin thread),
    // and it can do whatever it wants with it.
    QMutexLocker locker(&m_mutex);
    copy = m_detected_serial_ports ;
  }
  return copy;
}

void ReadThread::setDetectedSerialPorts(const QList<QSerialPortInfo>& ports)
{
  QMutexLocker locker(&m_mutex);
  m_detected_serial_ports = ports;
}

void ReadThread::wait_for_serial_device()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [START]" ;

  Q_ASSERT(m_input_device == NULL);

  thread_status = THREAD_STATUS_WAIT_FOR_DEVICE;

  // Reset counts
  input_bytes_count = 0;
  valid_frames_count = 0;
  valid_frames_bytes_count = 0;
  buffer_overflow_count = 0;

  while ( !isInterruptionRequested() && (m_settings_changed==0) )
    {
      QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

      setDetectedSerialPorts (ports);

      foreach(QSerialPortInfo pi , ports) {
	debug_print_serialportinfo(pi);

	bool found_device = false;

	//
	// Two methods of detection: by USB VID:PID or by filename.
	//
	if (m_settings.input_type == InputType_USB_Device) {
	  if (!pi.hasVendorIdentifier() || !pi.hasProductIdentifier())
	    continue ;

	  if (pi.vendorIdentifier() == m_settings.vendor_id
	      && pi.productIdentifier() == m_settings.product_id) {
	    found_device = true;
	  }
	}
	else if (m_settings.input_type == InputType_Serial_Device) {
	  if (pi.systemLocation() == m_settings.serial_device_filename) {
	    found_device = true;
	  }
	}

	// Found the device we were looking for, try to open it
	if (found_device) {
	  m_input_device = OpenSerialPort(pi.portName());

	  //If opened successfully, stop scanning the rest of the serial ports.
	  if (m_input_device)
	    break;
	}

      }

      // If a device was found, break from the auto-detection loop and move on
      // to input processing.
      if (m_input_device)
	break;

      // arbitrary delay, to avoid busy-loop and 100% cpu for no good benefit.
      // Longer values would mean slightly slower UI respond (if the user close the plugin
      // and triggers thread cancellation during this sleep).
      msleep (300);
    }

  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [END]" ;
}

void ReadThread::debug_print_buffer(const char* data, size_t len)
{
    QByteArray ba;
    ba.append(data,len);
    qDebug() << "=== " << ba.toHex(' ');

    QString s;
    QTextStream o(&s);
    o.setFieldAlignment(QTextStream::AlignLeft);
    o.setFieldWidth(3);
    for (char c : ba) {
      if (::isprint(c))
	o << c;
      else
	o << '.';
    }
    qDebug() << DEBUG_PREFIX << s ;
}

void ReadThread::process_frame(const char* data, size_t len)
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [START]" << " frame length " << len << " bytes" ;

  debug_print_buffer(data,len);

  QUdpSocket sock;
  qint64 i = sock.writeDatagram(data, len,
				m_settings.osc_server_ip,
				m_settings.osc_server_base_udp_port + m_settings.osc_universe_num);
  qDebug() << DEBUG_PREFIX "=== UDP Frame Sent: " << i ;

  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [END]" ;
}

void ReadThread::process_data(const char* data, size_t len)
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [START]" << " input data len: " << len ;

  input_bytes_count += len ;

  m_input_data.append(data,len);
  qDebug() << DEBUG_PREFIX "Accumulated Data = " << m_input_data.size();

  //qDebug() << "=== " << m_input_data.toHex(' ');

  if (m_input_data.size() > m_settings.max_input_buffer_size) {
    buffer_overflow_count++;

    qWarning() << DEBUG_PREFIX "data overflow!" ;

    // Frame overflow, meaning the 'frame marker' was not found while bytes
    // are contiguously accumulating.
    // Either the serial input stream is not really OSC-Bridge compatible,
    // or someone is sending VERY large streams.
    // In either case, discard some data (arbitrary size)
    // and continue.
    m_input_data.remove(0,m_settings.max_input_buffer_size/2);

    // Since data is discarded, the next frame to be detected
    // is certainly truncated, so discard it as if it's the first frame.
    m_first_frame = true ;
  }

  // Search for frames in the buffer (could be more than one)
  while (1) {

    //The arduino is expected to send the special 'frame marker' 4 bytes
    //between each OSC frame.
    //NOTE: almost every moderm libc has efficient memmem(3) implementation
    //      for searching 4 bytes in a buffer.
    const char* haystack = m_input_data.data();
    char* marker = (char*)memmem(haystack, m_input_data.size(),
				 m_settings.frame_marker, 4);

    if (!marker)
      break;

    ptrdiff_t pos = marker - haystack ;
    qDebug() << DEBUG_PREFIX "Found Frame-Marker at offset " << pos;
    if (m_first_frame) {
      // Discard the first frame, it is likely truncated (we missed the first few bytes).
      m_first_frame = false;
      qDebug() << DEBUG_PREFIX "Discarding first frame" ;
    } else {
      qDebug() << DEBUG_PREFIX "Processing new frame" ;
      valid_frames_count++;
      valid_frames_bytes_count += pos;
      process_frame(haystack, pos);
    }

    // Now remove the data up to and including
    // the frame marker - anything after that is the next frame.
    m_input_data.remove(0, pos+4);
  }


  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [END]" << "Remaining data " << m_input_data.size() << " bytes";
}

void ReadThread::process_input()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [START]" ;

  Q_ASSERT(m_input_device != NULL);

  thread_status = THREAD_STATUS_PROCESS_INPUT;

  m_first_frame = true;

  while ( !isInterruptionRequested() && (m_settings_changed==0) )
    {
      qDebug() << DEBUG_PREFIX "Waiting for Serial Input..." ;
      if (m_input_device->waitForReadyRead(500)) {

	#define MAX_BUF_SIZE 1024
	char buf[MAX_BUF_SIZE];
	qint64 i ;

	// Don't use QIODevice->readAll() -> it doesn't not provide a way to detect errors.
	i = m_input_device->read(buf, MAX_BUF_SIZE);

	if (i==-1) {
	  // Error
	  qWarning() << DEBUG_PREFIX "I/O error in input device, stop processing";
	  break;

	} else if (i>0) {
	  // Got Some Data
	  qDebug() << DEBUG_PREFIX "Got " << i << " bytes from serial input:";
	  process_data(buf, i);

	} else {
	  qWarning() << DEBUG_PREFIX " Strange: got no data after data-ready indication...";
	}
      } else {
	// If no data was ready, check if it was a simple timeout, or an I/O error
	if ((m_input_device->error() != QSerialPort::NoError) &&
	    (m_input_device->error() != QSerialPort::TimeoutError) ) {
	  qDebug() << DEBUG_PREFIX " I/O error " << m_input_device->error();
	  break;
	}
      }
    }

  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [END]" ;
}


void ReadThread::close_device()
{
  if (!m_input_device)
    return;

  qDebug() << DEBUG_PREFIX "Closing input device";
  m_input_device->close();
  delete m_input_device;
  m_input_device = NULL;
}


void ReadThread::run()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [Thread Created and Running]" ;

  while ( !isInterruptionRequested() )
    {
      wait_for_settings();

      if (!isInterruptionRequested() && (m_settings_changed==0) )
	wait_for_serial_device();

      if (!isInterruptionRequested() && (m_settings_changed==0) )
	process_input();

      close_device();
    }

  thread_status = THREAD_STATUS_EXITED;

  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " [Thread Function Terminated]" ;
}
