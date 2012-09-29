
/*++

Module Name:  
    usbInstl.h

Abstract:  
    USB driver installation class.
    
Notes: 

--*/
#ifndef __USBINSTL_H_
#define __USBINSTL_H_

#include <usbdi.h>
#include <devload.h>
#define DEFAULT_USB_DRIVER TEXT("USBD.DLL")

class UsbClientDrv;
#define MAX_SERVICE_ENTRY 5
#define MAX_USB_CLIENT_DRIVER 128
class USBDriverClass {
public:
    USBDriverClass(LPCTSTR lpDrvName, DWORD dwZone );
    USBDriverClass(DWORD dwZone  );
    virtual ~USBDriverClass() { FreeLibrary(hInst);};
    BOOL Init() { return(!UsbDriverClassError);};
 
//access function
    VOID GetUSBDVersion(LPDWORD lpdwMajorVersion, LPDWORD lpdwMinorVersion);
    BOOL RegisterClientDriverID(LPCWSTR szUniqueDriverId);
    BOOL UnRegisterClientDriverID(LPCWSTR szUniqueDriverId);
    BOOL RegisterClientSettings(LPCWSTR lpszDriverLibFile,
                            LPCWSTR lpszUniqueDriverId,LPCUSB_DRIVER_SETTINGS lpDriverSettings);
    BOOL UnRegisterClientSettings(LPCWSTR lpszUniqueDriverId, LPCWSTR szReserved,
                              LPCUSB_DRIVER_SETTINGS lpDriverSettings);
    HKEY OpenClientRegistryKey(LPCWSTR szUniqueDriverId);
    BOOL SetDefaultDriverRegistry(LPCWSTR szUniqueDriverId, int nCount, REGINI *pReg, BOOL fOverWrite = TRUE );
protected:
    DWORD   m_sDebugZone ;
private:
    BOOL CreateUsbAccessHandle(HINSTANCE hInst);
    LPOPEN_CLIENT_REGISTRY_KEY          lpOpenClientRegistyKey;
    LPREGISTER_CLIENT_DRIVER_ID         lpRegisterClientDriverID;
    LPUN_REGISTER_CLIENT_DRIVER_ID      lpUnRegisterClientDriverID;
    LPREGISTER_CLIENT_SETTINGS          lpRegisterClientSettings;
    LPUN_REGISTER_CLIENT_SETTINGS       lpUnRegisterClientSettings;
    LPGET_USBD_VERSION                  lpGetUSBDVersion;
    BOOL UsbDriverClassError;
    HINSTANCE hInst;
};

#endif
