/*
  Q Light Controller Plus

  serial-osc-bridge-configuration.cpp

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

#include <QFileInfo>
#include <QMessageBox>

#include "serial-osc-bridge-utils.h"
#include "serial-osc-bridge-configuration.h"
#include "serial-osc-bridge-plugin.h"
/*****************************************************************************
 * Initialization
 *****************************************************************************/

SerialOscBridgeConfiguration::SerialOscBridgeConfiguration(SerialOscBridgePlugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    SerialOscBridgeSettings s = plugin->getSettings();

    /* Fill Controls from the current plugin values */
    if (s.vendor_id)
      input_usb_vendor_id->setText( int_to_hex(s.vendor_id) );
    if (s.product_id)
      input_usb_product_id->setText( int_to_hex(s.product_id) );

    if (s.baudrate)
      input_baudrate->setText( int_to_str(s.baudrate) );

    input_serial_device_filename->setText( s.serial_device_filename ) ;

    output_osc_universe_number->setValue (s.osc_universe_num);

    frame_marker_byte_1->setText ( byte_to_hex(s.frame_marker[0]) );
    frame_marker_byte_2->setText ( byte_to_hex(s.frame_marker[1]) );
    frame_marker_byte_3->setText ( byte_to_hex(s.frame_marker[2]) );
    frame_marker_byte_4->setText ( byte_to_hex(s.frame_marker[3]) );

    max_input_buffer_length->setText( int_to_str(s.max_input_buffer_size) );
    osc_target_host_ip->setText( s.osc_server_ip.toString() ) ;
    osc_target_base_udp_port->setText ( int_to_str(s.osc_server_base_udp_port) ) ;

    input_type_auto_detect_usb->setChecked(false);
    input_type_serial_device_file->setChecked(false);
    switch(s.input_type)
      {
      case InputType_USB_Device:
	input_type_auto_detect_usb->setChecked(true);
	break;
      case InputType_Serial_Device:
	input_type_serial_device_file->setChecked(true);
	break;
      case InputType_None:
	//Leave both un-checked, and force the user to select one.
	break;
      }

}

SerialOscBridgeConfiguration::~SerialOscBridgeConfiguration()
{
    /** Cleanup the allocated resources, if any */
}



/*****************************************************************************
 * Dialog actions
 *****************************************************************************/
void SerialOscBridgeConfiguration::showValueAlert(QString control_name, QString control_value_name, QString value)
{
  QMessageBox::critical(this,
			tr("Invalid Hex Value"),
			tr("'%1' is not a valid %2 value for '%3'.\nPlease fix it before confirming.")\
			    .arg(value)\
			    .arg(control_value_name)\
			    .arg(control_name));
}



void SerialOscBridgeConfiguration::accept()
{
  SerialOscBridgeSettings set;
  QString s;
  bool ok;
  int i;

  if (input_type_auto_detect_usb->isChecked())
    set.input_type = InputType_USB_Device;
  else if (input_type_serial_device_file->isChecked())
    set.input_type = InputType_Serial_Device;
  else {
    //Shouldn't happen
    QMessageBox::critical(this,
			  tr("Missing Input Type"),
			  tr("Please select input type (USB/Serial) to continue"));
    return;
  }

  //
  // USB Vendor-ID
  //
  s = input_usb_vendor_id->text();
  if ( (set.input_type != InputType_USB_Device) && s.isEmpty() ) {
    // Allow Empty Vendor ID if not using USB input.
    i = 0 ;
  } else {
    // If not empty, require it to be valid 4-digit hex (even if not using USB input)
    i = s.toInt(&ok, 16);
    if (!ok || (s.size()!=4) || (i==0) || i>0xFFFF) {
      showValueAlert("Vendor-ID", "4-digit hex", s);
      return;
    }
  }
  set.vendor_id = i ;


  //
  // USB Product ID
  //
  s = input_usb_product_id->text();
  if ( (set.input_type != InputType_USB_Device) && s.isEmpty() ) {
    // Allow Empty Product ID if not using USB input.
    i = 0 ;
  } else {
    // If not empty, require it to be valid 4-digit hex (even if not using USB input)
    i = s.toInt(&ok, 16);
    if (!ok || (s.size()!=4) || (i==0) || i>0xFFFF) {
      showValueAlert("Product-ID", "4-digit hex", s);
      return;
    }
  }
  set.product_id = i;

  //
  // Serial Device Name
  //
  s = input_serial_device_filename->text();
  if ( (set.input_type != InputType_Serial_Device) && s.isEmpty() ) {
    // Allow Empty serial device if not using serial input
  } else {
    // If not empty, it must be a valid filename
    if (!valid_filename(s)) {
      showValueAlert("Serial Device filename", "filename", s);
      return ;
    }
  }
  set.serial_device_filename = s;


  //
  // Baud rate
  //
  s = input_baudrate->text();
  i = s.toInt(&ok, 10);
  if (!ok || !valid_baudrate(i)) {
    showValueAlert("USB Device Baudrate", "numeric baudrate", s);
    return;
  }
  set.baudrate = i ;


  //
  // Output: Universe Number where OSC input is used.
  // This is USER-FACING, so first universe is 1 (not zero).
  s = output_osc_universe_number->text();
  i = s.toInt(&ok, 10);
  if (!ok || i==0) {
      showValueAlert("OSC Target Universe Number", "numeric", s);
      return ;
  }
  set.osc_universe_num = i;


  //
  // Advanced: max input buffer length
  //
  s = max_input_buffer_length->text();
  i = s.toInt(&ok, 10);
  if (!ok || i==0) {
      showValueAlert("Max Input buffer length", "numeric", s);
      return ;
  }
  set.max_input_buffer_size = i;

  //
  // Advanced: OSC Base UDP Port
  //
  s = osc_target_base_udp_port->text();
  i = s.toInt(&ok, 10);
  if (!ok || i==0 || i>65535) {
      showValueAlert("OSC Base UDP Port", "numeric", s);
      return ;
  }
  set.osc_server_base_udp_port = i ;

  //
  // Advanced: OSC Host IP
  //
  s = osc_target_host_ip->text();
  QHostAddress newHostAddress(s);
  if (s.isEmpty() || newHostAddress.isNull()) {
      showValueAlert("OSC Target Host IP", "IP", s);
      return ;
  }
  set.osc_server_ip = newHostAddress;

  //
  // Advanced: The 4 bytes frame marker
  //
  s = frame_marker_byte_1->text();
  i = s.toInt(&ok, 16);
  if (!ok || (s.size()!=2) || i<0 || i>0xFF) {
    showValueAlert("Frame Marker (Byte 1)", "2-digit hex", s);
    return;
  }
  set.frame_marker[0] = i;

  s = frame_marker_byte_2->text();
  i = s.toInt(&ok, 16);
  if (!ok || (s.size()!=2) || i<0 || i>0xFF) {
    showValueAlert("Frame Marker (Byte 2)", "2-digit hex", s);
    return;
  }
  set.frame_marker[1] = i;

  s = frame_marker_byte_3->text();
  i = s.toInt(&ok, 16);
  if (!ok || (s.size()!=2) || i<0 || i>0xFF) {
    showValueAlert("Frame Marker (Byte 3)", "2-digit hex", s);
    return;
  }
  set.frame_marker[2] = i;

  s = frame_marker_byte_4->text();
  i = s.toInt(&ok, 16);
  if (!ok || (s.size()!=2) || i<0 || i>0xFF) {
    showValueAlert("Frame Marker (Byte 4)", "2-digit hex", s);
    return;
  }
  set.frame_marker[3] = i;

  // Save it for the plugin class to read it
  ui_settings = set;

  QDialog::accept();
}

int SerialOscBridgeConfiguration::exec()
{
    return QDialog::exec();
}
