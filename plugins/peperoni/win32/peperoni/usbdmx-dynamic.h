/*
  usbdmx-dynamic.h - include file for run-time dynamic loading of
  the USBDMX.DLL. This demo can handle the situation where the dll
  is not present.

  This file is provided as is to allow an easy start with the
  usbdmx driver and dll.

  Copyright (c) Lighting Solutions, Dr. Jan Menzel

  In case of trouble please contact driver@lighting-solutions.de or
  call +49/40/600877-51.

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

#ifndef _USBDMX_DYNAMIC_H
#define _USBDMX_DYNAMIC_H

#include <windows.h>

/**
 * DLL version
 */
#define USBDMX_DLL_VERSION USHORT(0x0403)

/**
 * MACRO to verify dll version
 */
#define USBDMX_DLL_VERSION_CHECK(a) ((a)->version() >= USBDMX_DLL_VERSION)

/****************************************************************************
 * Type definitions for all functions exported by the USBDMX.DLL
 ****************************************************************************/

/**
 * define the basic type of exported functions
 */
#define USBDMX_TYPE WINAPI *

/**
 * usbdmx_version(): returns the version number (16bit, 4 digits BCD)
 * Current version is USBDMX_DLL_VERSION. Use the Macro
 * USBDMX_DLL_VERSION_CHECK() to compare the dll version number with
 * the header files version.
 */
typedef	USHORT	(USBDMX_TYPE USBDMX_TYPE_VERSION) (VOID);

/**
 * usbdmx_open(): open device number <device>, where 0 is the first
 * and unlimit devices are supported. The function returnes
 * STATUS_INVALID_HANDLE_VALUE if <device> is not supported. Use the
 * returned handle to access the device later on. One device can be
 * opened an unlimited number of times.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_OPEN) (IN USHORT, OUT PHANDLE);

/**
 * usbdmx_close(): close the device identified by the given handle.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_CLOSE) (IN HANDLE);

/**
 * usbdmx_device_id(): read the device id of the device
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_DEVICE_ID) (IN HANDLE, OUT PUSHORT);

/**
 * usbdmx_is_XXX(): identify the device identified by the given handle.
 * Each function returns TRUE if the device matches.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_IS_XSWITCH) (IN HANDLE);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_IS_RODIN1) (IN HANDLE);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_IS_RODIN2) (IN HANDLE);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_IS_RODINT) (IN HANDLE);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_IS_USBDMX21) (IN HANDLE);

/**
 * usbdmx_product_get(): read the product string from the device.
 * size specifies the maximum size of the buffer pointed to by <string>
 * (unit bytes).
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_PRODUCT_GET) (IN HANDLE, OUT PWCHAR, IN USHORT);

/**
 * usbdmx_device_version(): Read the the device version of a device.
 * the device version is one of the values within the USBs configuration
 * descriptor (BcdDevice). pversion is only valid if the function returns
 * TRUE.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_DEVICE_VERSION) (IN HANDLE, OUT PUSHORT);

/**
 * USBDMX_TX(): transmit a frame using the new protocol on bulk endpoints
 *
 * INPUTs:	h			- handle to the device, returned by usbdmx_open()
 *			universe	- addressed universe
 *			slots		- number of bytes to be transmitted, as well as sizeof(buffer)
 *						  for DMX512: buffer[0] == startcode, slots <= 513
 *			buffer		- data to be transmitted,  !!! sizeof(buffer) >= slots !!!
 *			config		- configuration of the transmitter, see below for possible values
 *			time		- time value in s, depending on config, either timeout or delay
 *			time_break	- break time in s (can be zero, to not transmitt a break)
 *			time_mab	- Mark-after-Break time (can be zero)
 * OUTPUTs:	ptimestamp	- timestamp of this frame in ms, does overrun
 *			pstatus		- status of this transmission, see below for possible values
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX)
(IN  HANDLE  /* h          */, IN  UCHAR  /* universe */, IN USHORT /* slots */,
 IN  PUCHAR  /* buffer     */, IN  UCHAR  /* config   */, IN FLOAT  /* time  */,
 IN  FLOAT   /* time_break */, IN  FLOAT  /* time_mab */,
 OUT PUSHORT /* ptimestamp */, OUT PUCHAR /* pstatus  */);
/**
 * values of config (to be ored together)
 */
#define USBDMX_BULK_CONFIG_DELAY	(0x01)	// delay frame by time
#define USBDMX_BULK_CONFIG_BLOCK	(0x02)	// block while frame is not transmitting (timeout given by time)
#define USBDMX_BULK_CONFIG_RX		(0x04)	// switch to RX after having transmitted this frame
#define USBDMX_BULK_CONFIG_NORETX	(0x08)	// do not retransmit this frame
#define USBDMX_BULK_CONFIG_TXIRQ	(0x40)	// send data using transmitter IRQ instead of timer

/**
 * USBDMX_RX(): receive a frame using the new protocol on bulk endpoints
 *
 * INPUTs:	h		- handle to the device, returned by usbdmx_open()
 *		universe	- addressed universe
 *		slots_set	- number of bytes to receive, as well as sizeof(buffer)
 *				  for DMX512: buffer[0] == startcode, slots_set <= 513
 *		buffer		- data to be transmitted,  !!! sizeof(buffer) >= slots !!!
 *		timeout		- timeout for receiving the total frame in s,
 *		timeout_rx	- timeout between two slots used to detect premature end of frames
 * OUTPUTs:	pslots_get	- number of slots actually received, *pslots_get <= slots_set
 * 		ptimestamp	- timestamp of this frame in ms, does overrun
 *		pstatus		- status of the reception, see below for possible values
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX)
(IN  HANDLE  /* h          */, IN  UCHAR   /* universe   */, IN  USHORT /* slots_set  */,
 IN  PUCHAR  /* buffer     */, IN  FLOAT   /* timeout    */, IN  FLOAT  /* timeout_rx */,
 OUT PUSHORT /* pslots_get */, OUT PUSHORT /* ptimestamp */, OUT PUCHAR /* pstatus    */);

/**
 * values of *pstatus
 */
#define USBDMX_BULK_STATUS_OK			(0x00)
#define USBDMX_BULK_STATUS_TIMEOUT		(0x01)	// request timed out
#define USBDMX_BULK_STATUS_TX_START_FAILED	(0x02)	// delayed start failed
#define USBDMX_BULK_STATUS_UNIVERSE_WRONG	(0x03)	// wrong universe addressed\tabularnewline
#define USBDMX_BULK_STATUS_RX_OLD_FRAME		(0x10)	// old frame not read
#define USBDMX_BULK_STATUS_RX_TIMEOUT		(0x20)	// receiver finished with timeout (ored with others)
#define USBDMX_BULK_STATUS_RX_NO_BREAK		(0x40)	// frame without break received (ored with others)
#define USBDMX_BULK_STATUS_RX_FRAMEERROR	(0x80)	// frame finished with frame error (ored with others)

/**
 * macro to check, it the return status is ok
 */
#define USBDMX_BULK_STATUS_IS_OK(s) (s == USBDMX_BULK_STATUS_OK)

/**
 * usbdmx_tx_XXX(): Write or read the interfaces transmitter buffer.
 * usbdmx_tx2_XXX() addresses the buffer of the second transmitter, which
 * is only valid for usbdmx21 devices. The XXX_blocking() functions return
 * if the current frame has been transmitted compleately. They operate
 * the interface synronices with the transmitter. All other functions
 * operate asynchronously, they return immediately.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_SET) (IN HANDLE, IN PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_SET_BLOCKING) (IN HANDLE, IN PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX2_SET) (IN HANDLE, IN PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX2_SET_BLOCKING) (IN HANDLE, IN PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_GET) (IN HANDLE, OUT PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_GET_BLOCKING) (IN HANDLE, OUT PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX2_GET) (IN HANDLE, OUT PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX2_GET_BLOCKING) (IN HANDLE, OUT PUCHAR, IN USHORT);

/**
 * usbdmx_rx_XXX(): Write or read the interfaces receiver buffer.
 * The XXX_blocking() functions return if the current frame has been
 * transmitted compleately. They operate the interface synronices with
 * the transmitter. All other functions operate asynchronously, they
 * return immediately.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_SET) (IN HANDLE, IN PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_SET_BLOCKING) (IN HANDLE, IN PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_GET) (IN HANDLE, OUT PUCHAR, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_GET_BLOCKING) (IN HANDLE, OUT PUCHAR, IN USHORT);

/**
 * usbdmx_[rx|tx]_frames(): return a 32bit frame counter from the device.
 * Each device counts the transmitted/received frames. The framecounter
 * can overflow.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_FRAMES) (IN HANDLE, OUT PDWORD);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_FRAMES) (IN HANDLE, OUT PDWORD);

/**
 * usbdmx_[rx|tx]_startcode_[set|get](): read/set the startcode of the
 * transmitter/receiver. The receiver only accepts frames with the
 * given startcode, all other are ignored. According to DMX512A
 * specification the startcode should be 0.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_STARTCODE_SET) (IN HANDLE, IN UCHAR);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_STARTCODE_GET) (IN HANDLE, OUT PUCHAR);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_STARTCODE_SET) (IN HANDLE, IN UCHAR);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_STARTCODE_GET) (IN HANDLE, OUT PUCHAR);

/**
 * usbdmx_tx_slots_[set|get](): read/set the number of slots transmitted
 * per frame. Numbers above 512 or below 24 are not allowed.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_SLOTS_SET) (IN HANDLE, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_SLOTS_GET) (IN HANDLE, OUT PUSHORT);

/**
 * usbdmx_rx_slots_get(): read the number of slots received within the
 * last frames.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_RX_SLOTS_GET) (IN HANDLE, OUT PUSHORT);

/**
 * usbdmx_tx_timing_XXX(): read/set the timing values of the transmitter.
 * each value is returned/set in seconds.
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_TIMING_SET) (IN HANDLE, IN FLOAT, IN FLOAT, IN FLOAT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_TX_TIMING_GET) (IN HANDLE, OUT PFLOAT, OUT PFLOAT, OUT PFLOAT);

/**
 * usbdmx_id_led_XXX(): get/set the "id-led", the way the TX-led is handled:
 * id == 0:    the TX led is equal to the GL (good-link, USB-stauts) value
 * id == 0xff: the TX led shows just the transmitter state (active/deactivated)
 * other:      the TX led blinks the given number of times and then pauses
 */
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_ID_LED_SET) (IN HANDLE, IN USHORT);
typedef	BOOL	(USBDMX_TYPE USBDMX_TYPE_ID_LED_GET) (IN HANDLE, OUT PUSHORT);

/****************************************************************************
 * Structure declaration for USBDMX.DLL functions
 ****************************************************************************/

struct usbdmx_functions
{
    /**
     * usbdmx_version(): returns the version number (16bit, 4 digits BCD)
     * Current version is USBDMX_DLL_VERSION. Use the Macro
     * USBDMX_DLL_VERSION_CHECK() compare dll's and header files version.
     */
    USBDMX_TYPE_VERSION version;

    /**
     * usbdmx_open(): open device number <device>, where 0 is the first
     * and unlimit devices are supported. The function returnes
     * STATUS_INVALID_HANDLE_VALUE if <device> is not supported. Use the
     * returned handle to access the device later on. One device can be
     * opened an unlimited number of times.
     */
    USBDMX_TYPE_OPEN open;

    /**
     * usbdmx_close(): close the device identified by the given handle.
     */
    USBDMX_TYPE_CLOSE close;

    /**
     * usbdmx_device_id(): read the device id of the device
     */
    USBDMX_TYPE_DEVICE_ID device_id;

    /**
     * usbdmx_is_XXX(): identify the device identified by the given handle.
     * Each function returns TRUE if the device matches.
     */
    USBDMX_TYPE_IS_XSWITCH is_xswitch;
    USBDMX_TYPE_IS_RODIN1 is_rodin1;
    USBDMX_TYPE_IS_RODIN2 is_rodin2;
    USBDMX_TYPE_IS_RODINT is_rodint;
    USBDMX_TYPE_IS_USBDMX21 is_usbdmx21;

    /**
     * usbdmx_product_get(): read the product string from the device.
     * size specifies the maximum size of the buffer pointed to by
     * <string> (unit bytes).
     */
    USBDMX_TYPE_PRODUCT_GET product_get;

    /**
     * usbdmx_device_version(): Read the the device version of a device.
     * the device version is one of the values within the USBs
     * configuration descriptor (BcdDevice). pversion is only valid if the
     * function returns TRUE.
     */
    USBDMX_TYPE_DEVICE_VERSION device_version;

    /**
     * USBDMX_TX(): transmit a frame using the new protocol on bulk
     * 		endpoints
     *
     * INPUTs:	h		- handle to the device, returned by usbdmx_open()
     *		universe	- addressed universe
     *		slots		- number of bytes to be transmitted, as well as sizeof(buffer)
     *				  for DMX512: buffer[0] == startcode, slots <= 513
     *		buffer		- data to be transmitted,  !!! sizeof(buffer) >= slots !!!
     *		config		- configuration of the transmitter, see below for possible values
     *		time		- time value in s, depending on config, either timeout or delay
     *		time_break	- break time in s (can be zero, to not transmitt a break)
     *		time_mab	- Mark-after-Break time (can be zero)
     * OUTPUTs:	ptimestamp	- timestamp of this frame in ms, does overrun
     *		pstatus		- status of this transmission, see below for possible values
     */
    USBDMX_TYPE_TX tx;

    /**
     * USBDMX_RX(): receive a frame using the new protocol on bulk endpoints
     *
     * INPUTs:	h		- handle to the device, returned by usbdmx_open()
     *		universe	- addressed universe
     *		slots_set	- number of bytes to receive, as well as sizeof(buffer)
     *				  for DMX512: buffer[0] == startcode, slots_set <= 513
     *		buffer		- data to be transmitted,  !!! sizeof(buffer) >= slots !!!
     *		timeout		- timeout for receiving the total frame in s,
     *		timeout_rx	- timeout between two slots used to detect premature end of frames
     * OUTPUTs:	pslots_get	- number of slots actually received, *pslots_get <= slots_set
     *              ptimestamp	- timestamp of this frame in ms, does overrun
     *		pstatus		- status of the reception, see below for possible values
     */
    USBDMX_TYPE_RX rx;

    /**
     * usbdmx_tx_XXX(): Write or read the interfaces transmitter buffer.
     * usbdmx_tx2_XXX() addresses the buffer of the second transmitter,
     * which is only valid for usbdmx21 devices. The XXX_blocking()
     * functions return if the current frame has been transmitted
     * completely. They operate the interface in sync with the transmitter.
     * All other functions operate asynchronously, they return immediately.
     */
    USBDMX_TYPE_TX_SET tx_set;
    USBDMX_TYPE_TX_SET_BLOCKING tx_set_blocking;
    USBDMX_TYPE_TX2_SET tx2_set;
    USBDMX_TYPE_TX2_SET_BLOCKING tx2_set_blocking;
    USBDMX_TYPE_TX_GET tx_get;
    USBDMX_TYPE_TX_GET_BLOCKING tx_get_blocking;
    USBDMX_TYPE_TX2_GET tx2_get;
    USBDMX_TYPE_TX2_GET_BLOCKING tx2_get_blocking;

    /**
     * usbdmx_rx_XXX(): Write or read the interfaces receiver buffer.
     * The XXX_blocking() functions return if the current frame has been
     * transmitted compleately. They operate the interface synronices with
     * the transmitter. All other functions operate asynchronously, they
     * return immediately.
     */
    USBDMX_TYPE_RX_SET rx_set;
    USBDMX_TYPE_RX_SET_BLOCKING rx_set_blocking;
    USBDMX_TYPE_RX_GET rx_get;
    USBDMX_TYPE_RX_GET_BLOCKING rx_get_blocking;

    /**
     * usbdmx_[rx|tx]_frames(): return a 32bit frame counter from the
     * device. Each device counts the transmitted/received frames. The
     * frame counter can overflow.
     */
    USBDMX_TYPE_TX_FRAMES tx_frames;
    USBDMX_TYPE_RX_FRAMES rx_frames;

    /**
     * usbdmx_[rx|tx]_startcode_[set|get](): read/set the startcode of the
     * transmitter/receiver. The receiver only accepts frames with the
     * given startcode, all other are ignored. According to DMX512A
     * specification the startcode should be 0.
     */
    USBDMX_TYPE_TX_STARTCODE_SET tx_startcode_set;
    USBDMX_TYPE_TX_STARTCODE_GET tx_startcode_get;
    USBDMX_TYPE_RX_STARTCODE_SET rx_startcode_set;
    USBDMX_TYPE_RX_STARTCODE_GET rx_startcode_get;

    /**
     * usbdmx_tx_slots_[set|get](): read/set the number of slots x-mitted
     * per frame. Numbers above 512 or below 24 are not allowed.
     */
    USBDMX_TYPE_TX_SLOTS_SET tx_slots_set;
    USBDMX_TYPE_TX_SLOTS_GET tx_slots_get;

    /**
     * usbdmx_rx_slots_get(): read the number of slots received within the
     * last frames.
     */
    USBDMX_TYPE_RX_SLOTS_GET rx_slots_get;

    /**
     * usbdmx_tx_timing_XXX(): read/set the timing values of the x-mitter.
     * each value is returned/set in seconds.
     */
    USBDMX_TYPE_TX_TIMING_SET tx_timing_set;
    USBDMX_TYPE_TX_TIMING_GET tx_timing_get;

    /**
     * usbdmx_id_led_XXX(): get/set the "id-led", the way the TX-led is handled:
     * id == 0:    the TX led is equal to the GL (good-link, USB-stauts) value
     * id == 0xff: the TX led shows just the transmitter state (active/deactivated)
     * other:      the TX led blinks the given number of times and then pauses
     */
    USBDMX_TYPE_ID_LED_SET id_led_set;
    USBDMX_TYPE_ID_LED_GET id_led_get;
};

/*
 * USBDMX_INIT()
 *
 * Load the usbdmx.DLL and returns all functions in struct usbdmx.
 * USBDMX_INIT() increments a "used" counter to track the number of
 * users. Make shure to call usbdmx_release if the DLL is not used
 * anymore. If the DLL was not found, NULL is returned.
 */
struct usbdmx_functions* usbdmx_init(void);

/*
 * USBDMX_RELEASE()
 *
 * decrements the "used" counter and releases the DLL if not used anymore.
 * After calling USBDMX_RELEASE() the pointer returned by USBDMX_INIT()
 * is not valid anymore.
 */
VOID usbdmx_release(VOID);

#endif
