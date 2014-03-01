#ifdef __cplusplus
extern "C" { 
#endif

#include <stdint.h>

#define DWORD uint32_t
#ifdef WIN32
    #define USB_DMX_DLL __declspec(dllexport) __stdcall
    #define USB_DMX_CALLBACK __stdcall
#else
    #define USB_DMX_DLL
    #define USB_DMX_CALLBACK
#endif


// types for library functions
typedef unsigned char TDMXArray[512];
typedef char TSERIAL[16];
typedef TSERIAL TSERIALLIST[32];
typedef void USB_DMX_CALLBACK (THOSTDEVICECHANGEPROC) (void *);
typedef void USB_DMX_CALLBACK (THOSTINPUTCHANGEPROCBLOCK) (unsigned char blocknumber);


// define library functions
USB_DMX_DLL void GetAllConnectedInterfaces(TSERIALLIST* SerialList);
USB_DMX_DLL void GetAllOpenedInterfaces(TSERIALLIST* SerialList);
USB_DMX_DLL DWORD OpenLink(TSERIAL Serial, TDMXArray *DMXOutArray, TDMXArray *DMXInArray);
USB_DMX_DLL DWORD CloseLink (TSERIAL Serial);
USB_DMX_DLL DWORD CloseAllLinks (void);
USB_DMX_DLL DWORD RegisterInterfaceChangeNotification (THOSTDEVICECHANGEPROC Proc);
USB_DMX_DLL DWORD UnregisterInterfaceChangeNotification (void);
USB_DMX_DLL DWORD RegisterInputChangeNotification (THOSTDEVICECHANGEPROC Proc, void *additional);
USB_DMX_DLL DWORD UnregisterInputChangeNotification (void);

USB_DMX_DLL DWORD SetInterfaceMode (TSERIAL Serial, unsigned char Mode);
  // Modes:
  // 0: Do nothing - Standby
  // 1: DMX In -> DMX Out
  // 2: PC Out -> DMX Out
  // 3: DMX In + PC Out -> DMX Out
  // 4: DMX In -> PC In
  // 5: DMX In -> DMX Out & DMX In -> PC In
  // 6: PC Out -> DMX Out & DMX In -> PC In
  // 7: DMX In + PC Out -> DMX Out & DMX In -> PC In

USB_DMX_DLL DWORD GetDeviceVersion(TSERIAL Serial);
USB_DMX_DLL DWORD SetInterfaceAdvTxConfig(
    TSERIAL Serial, unsigned char Control, uint16_t Breaktime, uint16_t Marktime,
    uint16_t Interbytetime, uint16_t Interframetime, uint16_t Channelcount, uint16_t Startbyte
);
USB_DMX_DLL DWORD StoreInterfaceAdvTxConfig(TSERIAL Serial);
USB_DMX_DLL DWORD RegisterInputChangeBlockNotification(THOSTINPUTCHANGEPROCBLOCK Proc);
USB_DMX_DLL DWORD UnregisterInputChangeBlockNotification(void);

/// And the Functions from usbdmxsi.USB_DMX_DLL also

USB_DMX_DLL DWORD OpenInterface(TDMXArray * DMXOutArray, TDMXArray * DMXInArray, unsigned char Mode);
  // Modes:
  // 0: Do nothing - Standby
  // 1: DMX In -> DMX Out
  // 2: PC Out -> DMX Out
  // 3: DMX In + PC Out -> DMX Out
  // 4: DMX In -> PC In
  // 5: DMX In -> DMX Out & DMX In -> PC In
  // 6: PC Out -> DMX Out & DMX In -> PC In
  // 7: DMX In + PC Out -> DMX Out & DMX In -> PC In

USB_DMX_DLL DWORD CloseInterface(void);


#ifdef __cplusplus
}
#endif

