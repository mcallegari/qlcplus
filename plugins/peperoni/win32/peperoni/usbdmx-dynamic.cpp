/*
 * USBDMX_DYNAMIC.C - module to dynamically load the usbdmx.dll,
 * if present.
 *
 * This file is provided as is to allow an easy start with the
 * usbdmx driver and dll.
 *
 * In case of trouble please contact driver@lighting-solutions.de or
 * call +49/40/600877-51.
 */

#include <windows.h>
#include "usbdmx-dynamic.h"

/* global variables used by this module */
struct		usbdmx_functions usbdmx;
DWORD		used = 0;
HINSTANCE	hLib = 0;

/*
 * USBDMX_INIT()
 *
 * Load the usbdmx.DLL and returns all functions in struct usbdmx.
 * USBDMX_INIT() increments a "used" counter to track the number of
 * useres. Make shure to call usbdmx_release if the DLL is not used
 * anymore.
 * If the DLL was not found, NULL is returned.
 */
struct usbdmx_functions *usbdmx_init(VOID)
{
    // DLL alreday loaded?
    if (used)
    {
        used++;			// increment used counter
        return &usbdmx;	// return usbdmx_functions structure
    }

    // load the DLL
    hLib = LoadLibrary(TEXT("usbdmx.dll"));
    if (!hLib)
        return NULL;

    // load all functions of the DLL
    if ((usbdmx.version		= (USBDMX_TYPE_VERSION)		GetProcAddress(hLib, ("usbdmx_version"))) &&
            (usbdmx.open		= (USBDMX_TYPE_OPEN)		GetProcAddress(hLib, ("usbdmx_open"))) &&
            (usbdmx.close		= (USBDMX_TYPE_CLOSE)		GetProcAddress(hLib, ("usbdmx_close"))) &&
            (usbdmx.device_id	= (USBDMX_TYPE_DEVICE_ID)	GetProcAddress(hLib, ("usbdmx_device_id"))) &&
            (usbdmx.is_xswitch	= (USBDMX_TYPE_IS_XSWITCH)	GetProcAddress(hLib, ("usbdmx_is_xswitch"))) &&
            (usbdmx.is_rodin1	= (USBDMX_TYPE_IS_RODIN1)	GetProcAddress(hLib, ("usbdmx_is_rodin1"))) &&
            (usbdmx.is_rodin2	= (USBDMX_TYPE_IS_RODIN2)	GetProcAddress(hLib, ("usbdmx_is_rodin2"))) &&
            (usbdmx.is_rodint	= (USBDMX_TYPE_IS_RODINT)	GetProcAddress(hLib, ("usbdmx_is_rodint"))) &&
            (usbdmx.is_usbdmx21	= (USBDMX_TYPE_IS_USBDMX21)	GetProcAddress(hLib, ("usbdmx_is_usbdmx21"))) &&
            (usbdmx.product_get	= (USBDMX_TYPE_PRODUCT_GET)	GetProcAddress(hLib, ("usbdmx_product_get"))) &&
            (usbdmx.device_version	= (USBDMX_TYPE_DEVICE_VERSION)	GetProcAddress(hLib, ("usbdmx_device_version"))) &&
            (usbdmx.tx		= (USBDMX_TYPE_TX)		GetProcAddress(hLib, ("usbdmx_tx"))) &&
            (usbdmx.rx		= (USBDMX_TYPE_RX)		GetProcAddress(hLib, ("usbdmx_rx"))) &&
            (usbdmx.tx_set		= (USBDMX_TYPE_TX_SET)		GetProcAddress(hLib, ("usbdmx_tx_set"))) &&
            (usbdmx.tx_set_blocking	= (USBDMX_TYPE_TX_SET_BLOCKING)	GetProcAddress(hLib, ("usbdmx_tx_set_blocking"))) &&
            (usbdmx.tx2_set		= (USBDMX_TYPE_TX2_SET)		GetProcAddress(hLib, ("usbdmx_tx2_set"))) &&
            (usbdmx.tx2_set_blocking= (USBDMX_TYPE_TX2_SET_BLOCKING)GetProcAddress(hLib, ("usbdmx_tx2_set_blocking"))) &&
            (usbdmx.tx_get		= (USBDMX_TYPE_TX_GET)		GetProcAddress(hLib, ("usbdmx_tx_get"))) &&
            (usbdmx.tx_get_blocking	= (USBDMX_TYPE_TX_GET_BLOCKING)	GetProcAddress(hLib, ("usbdmx_tx_get_blocking"))) &&
            (usbdmx.tx2_get		= (USBDMX_TYPE_TX2_GET)		GetProcAddress(hLib, ("usbdmx_tx2_get"))) &&
            (usbdmx.tx2_get_blocking= (USBDMX_TYPE_TX2_GET_BLOCKING)GetProcAddress(hLib, ("usbdmx_tx2_get_blocking"))) &&
            (usbdmx.rx_set		= (USBDMX_TYPE_RX_SET)		GetProcAddress(hLib, ("usbdmx_rx_set"))) &&
            (usbdmx.rx_set_blocking	= (USBDMX_TYPE_RX_SET_BLOCKING)	GetProcAddress(hLib, ("usbdmx_rx_set_blocking"))) &&
            (usbdmx.rx_get		= (USBDMX_TYPE_RX_GET)		GetProcAddress(hLib, ("usbdmx_rx_get"))) &&
            (usbdmx.rx_get_blocking	= (USBDMX_TYPE_RX_GET_BLOCKING)	GetProcAddress(hLib, ("usbdmx_rx_get_blocking"))) &&
            (usbdmx.tx_frames	= (USBDMX_TYPE_TX_FRAMES)	GetProcAddress(hLib, ("usbdmx_tx_frames"))) &&
            (usbdmx.rx_frames	= (USBDMX_TYPE_RX_FRAMES)	GetProcAddress(hLib, ("usbdmx_rx_frames"))) &&
            (usbdmx.tx_startcode_set= (USBDMX_TYPE_TX_STARTCODE_SET)GetProcAddress(hLib, ("usbdmx_tx_startcode_set"))) &&
            (usbdmx.tx_startcode_get= (USBDMX_TYPE_TX_STARTCODE_GET)GetProcAddress(hLib, ("usbdmx_tx_startcode_get"))) &&
            (usbdmx.rx_startcode_set= (USBDMX_TYPE_RX_STARTCODE_SET)GetProcAddress(hLib, ("usbdmx_rx_startcode_set"))) &&
            (usbdmx.rx_startcode_get= (USBDMX_TYPE_RX_STARTCODE_GET)GetProcAddress(hLib, ("usbdmx_rx_startcode_get"))) &&
            (usbdmx.tx_slots_set	= (USBDMX_TYPE_TX_SLOTS_SET)	GetProcAddress(hLib, ("usbdmx_tx_slots_set"))) &&
            (usbdmx.tx_slots_get	= (USBDMX_TYPE_TX_SLOTS_GET)	GetProcAddress(hLib, ("usbdmx_tx_slots_get"))) &&
            (usbdmx.rx_slots_get	= (USBDMX_TYPE_RX_SLOTS_GET)	GetProcAddress(hLib, ("usbdmx_rx_slots_get"))) &&
            (usbdmx.tx_timing_set	= (USBDMX_TYPE_TX_TIMING_SET)	GetProcAddress(hLib, ("usbdmx_tx_timing_set"))) &&
            (usbdmx.tx_timing_get	= (USBDMX_TYPE_TX_TIMING_GET)	GetProcAddress(hLib, ("usbdmx_tx_timing_get"))) &&
            (usbdmx.id_led_set	= (USBDMX_TYPE_ID_LED_SET)	GetProcAddress(hLib, ("usbdmx_id_led_set"))) &&
            (usbdmx.id_led_get	= (USBDMX_TYPE_ID_LED_GET)	GetProcAddress(hLib, ("usbdmx_id_led_get"))))
    {
        // Library loaded successfully
        return &usbdmx;
    }
    else
    {
        return NULL;
    }
}

/*
 * USBDMX_RELEASE()
 *
 * decrements the "used" counter and releases the DLL if not used anymore.
 * After calling USBDMX_RELEASE() the pointer returned by USBDMX_INIT()
 * is not valid anymore.
 */
VOID usbdmx_release(VOID)
{
    used--;	// decrement used counter

    // is the dll still used?
    if (used)
        return;	// finish

    // release dll
    FreeLibrary(hLib);
}
