/*
  Q Light Controller Plus

  serial-osc-bridge-plugin.cpp

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

#include <QtGlobal>
#include <QDebug>
#include <QSerialPortInfo>

#include "serial-osc-bridge-utils.h"
#include "serial-osc-bridge-configuration.h"
#include "serial-osc-bridge-readerthread.h"
#include "serial-osc-bridge-plugin.h"

// This version is just for internal debugging of the pluging, no meaning
// outside debug messages.
#define VERSION "11"
#define DEBUG_PREFIX "[SerialOSCBridge] "

/*****************************************************************************
 * Initialization
 *****************************************************************************/
SerialOscBridgePlugin::SerialOscBridgePlugin()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " version " << VERSION;

  m_settings.ResetToDefaults();
}

SerialOscBridgePlugin::~SerialOscBridgePlugin()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO ;
}

void SerialOscBridgePlugin::init()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO ;
  m_readerThread = NULL;
  m_inputUniverse = UINT_MAX;
}

QString SerialOscBridgePlugin::name()
{
  return QString("Serial-to-OSC Bridge");
}

int SerialOscBridgePlugin::capabilities() const
{
  return QLCIOPlugin::Input;
}


QString SerialOscBridgePlugin::pluginInfo()
{
  QString str;

  str += QString("<HTML>");
  str += QString("<HEAD>");
  str += QString("<TITLE>%1</TITLE>").arg(name());
  str += QString("</HEAD>");
  str += QString("<BODY>");

  str += QString("<P>");
  str += QString("<H3>%1</H3>").arg(name());
  str += tr("This plugin provides a bridge between a Serial line (typicaly, an Android over USB) input and an OSC input");
  str += QString("</P>");
  str += "<small>Note: this page is not automatically updated. Click on another input then back on this one to refresh</small>";
  str += "<br/>";

  return str;
}

bool SerialOscBridgePlugin::openInput(quint32 input, quint32 universe)
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO  << " input=" << input << " universe=" << universe;

  if (input != 0)
    return false;

  m_inputUniverse = universe;

  addToMap(universe, input, Input);

  /*
    The reader thread is started automatically when the plugin is connected
    to a universe, NOT waiting for valid user configuration.

    If/When the user changes configuration later,
    the thread will be notified and it will handle re-opening the
    serial device.
  */
  Q_ASSERT(m_readerThread == NULL);
  m_readerThread = new ReadThread();
  m_readerThread->changeSettings(m_settings);
  m_readerThread->start();

  return true;
}

void SerialOscBridgePlugin::closeInput(quint32 input, quint32 universe)
{
    qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << " input=" << input << " universe=" << universe;

    if (input != 0)
        return;

    m_inputUniverse = UINT_MAX;

    removeFromMap(universe, input, Input);

    Q_ASSERT(m_readerThread != NULL);
    qDebug() << DEBUG_PREFIX << "Requesting Thread Interruption";
    m_readerThread->stop_request();
    qDebug() << DEBUG_PREFIX << "Waiting for Reader thread to stop";
    m_readerThread->wait();
    qDebug() << DEBUG_PREFIX "Thread is done";
    delete m_readerThread;

    m_readerThread = NULL;
}

QStringList SerialOscBridgePlugin::inputs()
{
  /*
    Note about the input implementation:
    This plugin DOES NOT enumerate USB devices upon program startup
    and displays one input per USB device (unlike USB-DMX).

    Instead, we display one generic input device,
    and the USB/serial-device associate happens dynamically
    when the plugin is active.

    This allows inserion/removal of the USB device (e.g. arduino)
    whlie the plugin is running without causing any problems.

    Which device is used is determined by the user by
    setting the USB VendorID/ProductID in the configuration dialog.
  */
  QStringList list;
  list << QString("1: Serial-OSC-Bridge");
  return list;
}


QString SerialOscBridgePlugin::inputInfo(quint32 input)
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO ;

  /*
    Provide as much useful and helpful information as possible
    to the user (life is already complicated enough when mixing
    software and hardware...
  */

  QString str;

  if (input != QLCIOPlugin::invalidLine())
    str += QString("<H3>%1</H3>").arg(inputs()[input]);


  /*
    If the reader-thread isn't running, the plugin is not
    active and there's not much to tell the user.
  */
  if (m_readerThread==NULL) {
    str += "STATUS: <b>Inactive</b>";
    str += QString("</BODY>");
    str += QString("</HTML>");
    return str;
  }


  // Reader Thread Status (conceptually, from the user pserspective, it's the same as the plugin status)
  // The reader-thread can be in one of several states.
  // Show the user in which start we are in.

  if (m_readerThread->thread_status == THREAD_STATUS_INITIALIZING) {
    // Very short state, happens when the thread just started. Shouldn't stay in this for too long
    str += "STATUS: <b>Initializing...</b>";

  } else if (m_readerThread->thread_status == THREAD_STATUS_WAIT_FOR_SETTINGS) {
    // QLC+ plugin initialization is finicky:
    // Settings (e.g. loaded from saved workspace files) are trickled in one-by-one,
    // and there's no programmatic way to know when all of them were loaded.
    // As such, use a bit of heuristics:
    //

    // If the "wait for settings" loop lasted LESS then 10 interations (equivalent
    // to about 1 seconds, due to 100ms delay) - assume we're still OK, and just waiting
    // for more settings to arrive.
    //
    // If it's more than 10 iterations, assume something is wrong with the configuration,
    // e.g. some values are missing (for example, the "vendor_id" is set but "product_id" is missing).
    if (m_readerThread->wait_for_settings_loop_count<=10) {
      str += "STATUS: <b>Waiting for configuration update</b>";
    } else {
      str += "STATUS: <b>Invalid configuration, please re-check</b>";
    }

  } else if (m_readerThread->thread_status == THREAD_STATUS_WAIT_FOR_DEVICE) {
    //Things can pretty complicated with USB autodetection, provide the user
    //as much details as possible for troubleshooting...

    if (m_settings.input_type == InputType_USB_Device) {
      str += "STATUS: <b>Waiting for USB Serial Device</b>";
      str += " (Expecting VID:PID <code>";
      str += int_to_hex(m_settings.vendor_id);
      str += ":" ;
      str += int_to_hex(m_settings.product_id);
      str += ")";
    } else if (m_settings.input_type == InputType_Serial_Device) {
      str += "STATUS: <b>Waiting for Serial Device File</b>";
      str += " (Expecting file <code>" + m_settings.serial_device_filename + "</code>)";
    } else {
      Q_ASSERT(!"Should not happen");
    }
    str += "<br/><br/>";

    //
    // Show currently detected Serial port devices in a <table>
    //
    str += "<style>" \
      "table.ports {" \
      "  padding: 0px; " \
      "  margin: 0px; " \
      "  border-collapse: collapse;" \
      "  border: 1px solid black;" \
      "}" \
      "table.ports th,td {" \
      "  text-align: left;" \
      "  padding: 2px 10px;" \
      "}" \
      "</style>";
    str += "Detected Serial Devices:<br/>" \
      "<table class='ports'>"	\
      "<thead>" \
      "<tr>" \
      "<th>Device</th>" \
      "<th>VID:PID</th>" \
      "<th>Sys.File</th>" \
      "<th>Desc.</th>" \
      "</tr>" \
      "</thead>" \
      "<tbody>";
    // Now show the currently detected serial devices
    QList<QSerialPortInfo> ports = m_readerThread->getDetectedSerialPorts();
    for (QSerialPortInfo spi : ports) {
      str += "<tr>";
      str += "<td>" + spi.portName() + "</td>";
      str += "<td>" + int_to_hex(spi.vendorIdentifier()) + ":" + int_to_hex(spi.productIdentifier()) + "</td>";
      str += "<td>" + spi.systemLocation() + "</td>";
      str += "<td>" + spi.description() + "</td>";
      str += "</tr>";
    }
    str += "</tbody></table>";

  } else if (m_readerThread->thread_status == THREAD_STATUS_PROCESS_INPUT) {
    //
    // The reader thread is properly configured, and the requested USB/serial device
    // was found, and was opened successfully.
    //
    // But it's too soon to rest on our laurels:
    // Any USB/Serial device can be opened, it doesn't mean
    // that it sends correct OSC buffers.
    // And even if it is an arduino sending proper OSC buffers (and OSC end-of-frame markers),
    // It could still have bugs (e.g. when developing new arduino code).
    //
    // So show the user some statistics about how much data was received
    // (indicating if nothing is sent over the USB/serial line),
    // and how many valid OSC frames were detected.
    //
    //
    str += "STATUS: <b>Processing Serial Device Input</b>";

    //
    // Show some helpful statistics
    //
    str += "<style>" \
      "table.stats {" \
      "  padding: 0px; " \
      "  margin: 0px; " \
      "  border-collapse: collapse;" \
      "  border: 1px solid black;" \
      "}" \
      "table.stats th,td {" \
      "  text-align: left;" \
      "  padding: 2px 10px;" \
      "}" \
      "</style>";
    str += "<table class='stats'>"	\
      "<tbody>";
    str += "<tr>" \
      "<th>Valid Frames</th>" \
      "<td>" + int_to_str_group_sep(m_readerThread->valid_frames_count) + "</td>" \
      "</tr>" ;
    str += "<tr>" \
      "<th>Valid Frames Bytes</th>" \
      "<td>" + int_to_data_human_sizes(m_readerThread->valid_frames_count*4 +
				       m_readerThread->valid_frames_bytes_count) + "</td>" \
      "</tr>" ;
    str += "<tr>" \
      "<th>Total Input Bytes</th>" \
      "<td>" + int_to_data_human_sizes(m_readerThread->input_bytes_count) + "</td>" \
      "</tr>" ;
    str += "<tr>" \
      "<th>Buffer overflows</th>" \
      "<td>" + int_to_str_group_sep(m_readerThread->buffer_overflow_count) + "</td>" \
      "</tr>" ;

    //
    // Try to give the user a useful hint
    //
    str += "<tr>"				    \
      "<th>Input Health Heuristics</th>" \
      "<td>";

    unsigned long int valid_bytes = m_readerThread->valid_frames_count*4 + m_readerThread->valid_frames_bytes_count;
    unsigned long int valid_ratio =
      (m_readerThread->input_bytes_count>0)
      ? ((valid_bytes*100) / m_readerThread->input_bytes_count)
      : (0);

    if (m_readerThread->input_bytes_count < 0) {
      str += "BAD: no data received from device";
    } else if (m_readerThread->input_bytes_count < 2000) {
      str += "TOO SOON: not enough input data to decide";
    } else if (valid_ratio>90) {
      str += "GOOD: input contains mostly valid frames (" + int_to_str(valid_ratio) + "% valid)";
    } else {
      str += "MIXED: input contains mixed of valid and invalid data (" + int_to_str(valid_ratio) + "% valid)";
    }
    str += "</td></tr></tbody></table>";

  } else if (m_readerThread->thread_status == THREAD_STATUS_EXITED) {
    str += "terminated";

  } else {
    Q_ASSERT(!"Should not happen");
  }

  str += QString("</BODY>");
  str += QString("</HTML>");

  return str;
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

bool SerialOscBridgePlugin::canConfigure()
{
  return true;
}

void SerialOscBridgePlugin::configure()
{
  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO ;
  SerialOscBridgeConfiguration conf(this);
  if (conf.exec() == QDialog::Accepted)
    {
      applySettings(conf.getNewSettings());
    }
}

void SerialOscBridgePlugin::applySettings(const SerialOscBridgeSettings& new_settings)
{
  const QLCIOPlugin::Capability cap = QLCIOPlugin::Input;
  quint32 universe = m_inputUniverse ; // TODO: How to find out which universe we're in?
  quint32 line = 0 ; // This plugin support only one line

  // Input
  setParameter(universe, line, cap, SOSCB_INPUT_TYPE, new_settings.input_type);
  setParameter(universe, line, cap, SOSCB_VENDOR_ID,  new_settings.vendor_id);
  setParameter(universe, line, cap, SOSCB_PRODUCT_ID, new_settings.product_id);
  setParameter(universe, line, cap, SOSCB_SERIAL_DEVICE, new_settings.serial_device_filename);
  setParameter(universe, line, cap, SOSCB_BAUDRATE, new_settings.baudrate);

  // Output
  setParameter(universe, line, cap, SOSCB_OSC_UNIVERSE_NUM, new_settings.osc_universe_num);

  // Advanced
  setParameter(universe, line, cap, SOSCB_FRAME_MARKER_1, new_settings.frame_marker[0]);
  setParameter(universe, line, cap, SOSCB_FRAME_MARKER_2, new_settings.frame_marker[1]);
  setParameter(universe, line, cap, SOSCB_FRAME_MARKER_3, new_settings.frame_marker[2]);
  setParameter(universe, line, cap, SOSCB_FRAME_MARKER_4, new_settings.frame_marker[3]);
  setParameter(universe, line, cap, SOSCB_OSC_HOST_IP, new_settings.osc_server_ip.toString());
  setParameter(universe, line, cap, SOSCB_OSC_BASE_UDP_PORT, new_settings.osc_server_base_udp_port);
  setParameter(universe, line, cap, SOSCB_MAX_INPUT_BUFFER_SIZE, new_settings.max_input_buffer_size);
}


void SerialOscBridgePlugin::setParameter(quint32 universe, quint32 line, Capability type,
                             QString name, QVariant value)
{
  bool unset = false;
  SerialOscBridgeSettings defaults;
  defaults.ResetToDefaults();

  qDebug() << DEBUG_PREFIX << Q_FUNC_INFO << "Universe" << universe << "line" << line
	   << "name" << name << "value" << value ;

  if ( name == SOSCB_INPUT_TYPE ) {
    // Be more careful (than naive type-casting), don't blindly accept any value
    int i = value.toInt();
    if ( i == 1 ) {
      m_settings.input_type = InputType_USB_Device ;
    } else if ( i == 2 ) {
      m_settings.input_type = InputType_Serial_Device;
    } else {
      qFatal("=== Invalid InputType parameter for SerialOscBridge: %s", qPrintable(value.toString()));
    }
    unset = ( m_settings.input_type == defaults.input_type ) ;

  } else if ( name == SOSCB_VENDOR_ID ) {
    m_settings.vendor_id = value.toInt();
    unset = ( m_settings.vendor_id == defaults.vendor_id ) ;

  } else if ( name == SOSCB_PRODUCT_ID ) {
    m_settings.product_id = value.toInt();
    unset = ( m_settings.product_id == defaults.product_id ) ;

  } else if ( name == SOSCB_SERIAL_DEVICE ) {
    m_settings.serial_device_filename = value.toString();
    unset = ( m_settings.serial_device_filename == defaults.serial_device_filename ) ;

  } else if ( name == SOSCB_BAUDRATE ) {
    m_settings.baudrate = value.toInt();
    unset = ( m_settings.baudrate == defaults.baudrate ) ;

  } else if ( name == SOSCB_OSC_UNIVERSE_NUM ) {
    m_settings.osc_universe_num = value.toInt();
    unset = ( m_settings.osc_universe_num == defaults.osc_universe_num ) ;

  } else if ( name == SOSCB_FRAME_MARKER_1 ) {
    int i = value.toInt();
    Q_ASSERT(i>=0 && i<=255);
    m_settings.frame_marker[0] = i;
    unset = ( m_settings.frame_marker[0] == defaults.frame_marker[0] ) ;

  } else if ( name == SOSCB_FRAME_MARKER_2 ) {
    int i = value.toInt();
    Q_ASSERT(i>=0 && i<=255);
    m_settings.frame_marker[1] = i;
    unset = ( m_settings.frame_marker[1] == defaults.frame_marker[1] ) ;

  } else if ( name == SOSCB_FRAME_MARKER_3 ) {
    int i = value.toInt();
    Q_ASSERT(i>=0 && i<=255);
    m_settings.frame_marker[2] = i;
    unset = ( m_settings.frame_marker[2] == defaults.frame_marker[2] ) ;

  } else if ( name == SOSCB_FRAME_MARKER_4 ) {
    int i = value.toInt();
    Q_ASSERT(i>=0 && i<=255);
    m_settings.frame_marker[3] = i;
    unset = ( m_settings.frame_marker[3] == defaults.frame_marker[3] ) ;

  } else if ( name == SOSCB_OSC_HOST_IP ) {
    QString s = value.toString();
    QHostAddress newHostAddress(s);
    Q_ASSERT( s.isEmpty() || !newHostAddress.isNull() );
    m_settings.osc_server_ip = newHostAddress ;
    unset = ( m_settings.osc_server_ip == defaults.osc_server_ip ) ;

  } else if ( name == SOSCB_OSC_BASE_UDP_PORT ) {
    int i = value.toInt();
    Q_ASSERT(i>=0 && i<=65535);
    m_settings.osc_server_base_udp_port = i ;
    unset = ( m_settings.osc_server_base_udp_port == defaults.osc_server_base_udp_port ) ;

  } else if ( name == SOSCB_MAX_INPUT_BUFFER_SIZE ) {
    int i = value.toInt();
    Q_ASSERT(i>=0 && i<=65535);
    m_settings.max_input_buffer_size  = i ;
    unset = ( m_settings.max_input_buffer_size == defaults.max_input_buffer_size ) ;
  } else {
    qWarning() << Q_FUNC_INFO << name << "is not a valid Serial-OSC-Bridge parameter";
    return;
  }

  if (unset) {
    qDebug() << DEBUG_PREFIX "UNSET" << name ;
    QLCIOPlugin::unSetParameter(universe, line, type, name);
  } else {
    qDebug() << DEBUG_PREFIX "SET" << name ;
    QLCIOPlugin::setParameter(universe, line, type, name, value);
  }

  if (m_readerThread != NULL) {
    m_readerThread->changeSettings(m_settings);
  }
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(serialoscbridgeplugin, SerialOscBridgePlugin)
#endif
