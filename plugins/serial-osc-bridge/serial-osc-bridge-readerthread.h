/*
  Q Light Controller Plus

 `serial-osc-bridge-readerthread.cpp

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

#ifndef SERIAL_OSC_BRIDGE_READERTHREAD_H
#define SERIAL_OSC_BRIDGE_READERTHREAD_H

#include <QThread>
#include <QAtomicInteger>
#include <QMutexLocker>
#include <QByteArray>
#include <QSerialPort>
#include <QHostAddress>

// If compilation fails regarding missing QSerialPort,
// install the add on with "apt-get install libqt5serialport5-dev"
#include <QSerialPort>

#include "serial-osc-bridge-settings.h"

enum SerialOscBridgeThreadStatus
  {
   THREAD_STATUS_INITIALIZING = 1,
   THREAD_STATUS_WAIT_FOR_SETTINGS = 2,
   THREAD_STATUS_WAIT_FOR_DEVICE = 3,
   THREAD_STATUS_PROCESS_INPUT = 4,
   THREAD_STATUS_EXITED =5
  };

class ReadThread : public QThread
{
  Q_OBJECT

public:
  ReadThread(QObject *parent = 0);
  /** Destructor */
  virtual ~ReadThread();

  void stop_request();

  void changeSettings(const SerialOscBridgeSettings& new_settings);

  QAtomicInteger<int> thread_status; // one of SerialOscBrdigeThreadStatus
  QAtomicInteger<int> wait_for_settings_loop_count;
  QAtomicInteger<int> input_bytes_count;
  QAtomicInteger<int> valid_frames_count;
  QAtomicInteger<int> valid_frames_bytes_count;
  QAtomicInteger<int> buffer_overflow_count;
  QList<QSerialPortInfo> getDetectedSerialPorts();

protected:
  void run();
  void wait_for_settings();
  void wait_for_serial_device();

  QSerialPort* OpenSerialPort(const QString& s);
  void close_device();

  void process_input();
  void process_data(const char* data, size_t len);
  void process_frame(const char* data, size_t len);


  void debug_print_buffer(const char* data, size_t len);
  void debug_print_serialportinfo(const QSerialPortInfo &spi);

  void setDetectedSerialPorts ( const QList<QSerialPortInfo>& ports ) ;

signals:
  void valueChanged(quint32 channel, uchar value);

private:
  mutable QMutex m_mutex;
  // atomic true/false flag indicating the new settings are
  // pending from another thread
  QAtomicInteger<int> m_settings_changed;
  SerialOscBridgeSettings m_new_settings;
  SerialOscBridgeSettings m_settings;

  QList<QSerialPortInfo> m_detected_serial_ports;

  QSerialPort *m_input_device;
  QByteArray m_input_data;

  bool m_first_frame ;
};

#endif
