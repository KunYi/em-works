//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#include <windows.h>

#include <ras.h>

#include "ndis.h"
#include "nuiouser.h"

#include <svsutil.hxx>

#include <bt_api.h>
#include <bt_buffer.h>
#include <bt_ddi.h>

#include "btenum.hxx"

#include "bthid.h"

#define HID_CLIENT_REGKEY_SZ       _T("Software\\Microsoft\\Bluetooth\\Hid\\Hid_Class")
#define HID_REGKEY_SZ              _T("Software\\Microsoft\\Bluetooth\\Hid\\Instance")

// RAS
typedef DWORD (RASAPI * p_RasSetEntryDialParams) (LPWSTR lpszPhoneBook, LPRASDIALPARAMS lpRasDialParams, BOOL fRemovePassword);
typedef DWORD (RASAPI * p_RasGetEntryProperties) (LPWSTR lpszPhoneBook, LPWSTR szEntry, LPRASENTRY lpEntry, LPDWORD lpdwEntrySize, LPBYTE lpb, LPDWORD lpdwSize);
typedef DWORD (RASAPI * p_RasSetEntryProperties) (LPWSTR lpszPhoneBook, LPWSTR szEntry, LPRASENTRY lpEntry, DWORD dwEntrySize, LPBYTE lpb, DWORD dwSize);
typedef DWORD (RASAPI * p_RasDeleteEntry) (LPWSTR lpszPhonebook, LPWSTR lpszEntry);

static LPWSTR gszKeyNames[BTENUM_CLASSES + 1] = {L"modem", L"printer", L"lan_access", L"file_trans", L"OBEX", L"Headset", L"ActiveSync", L"HID", L"PAN", L"Handsfree", L"UNKNOWN"};
static int    gimtu[BTENUM_CLASSES + 1]       = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
static int    gauth[BTENUM_CLASSES + 1];
static int    gencr[BTENUM_CLASSES + 1];
static int    gidefault_mtu = 0;
static int    gidefault_auth = 0;
static int    gidefault_encr = 0;

static WCHAR gszAdapterName[50];

static p_RasGetEntryProperties g_pfnRasGetEntryProperties;
static p_RasSetEntryProperties g_pfnRasSetEntryProperties;
static p_RasSetEntryDialParams g_pfnRasSetEntryDialParams;
static p_RasDeleteEntry g_pfnRasDeleteEntry;

static int GetBA (WCHAR *pp, BT_ADDR *pba) {
    *pba = 0;

    while (*pp == ' ')
        ++pp;

    for (int i = 0 ; i < 12 ; ++i, ++pp) {
        if (! iswxdigit (*pp))
            return FALSE;

        int c = *pp;
        if (c >= 'a')
            c = c - 'a' + 0xa;
        else if (c >= 'A')
            c = c - 'A' + 0xa;
        else c = c - '0';

        if ((c < 0) || (c > 16))
            return FALSE;

        *pba = *pba * 16 + c;
    }

    if ((*pp != ' ') && (*pp != '\0'))
        return FALSE;

    return TRUE;
}

static BOOL HexStringToDword(WCHAR *lpsz, DWORD &Value, int cDigits, WCHAR chDelim) {
    Value = 0;
    for (int Count = 0; Count < cDigits; Count++, lpsz++) {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }

    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}

static int StrToGUID (WCHAR *szStr, GUID *pguid) {
    WCHAR *lpsz = szStr;
    DWORD dw;

    if (*lpsz++ != '{')
        return FALSE;

    if (!HexStringToDword(lpsz, pguid->Data1, sizeof(DWORD)*2, '-'))
        return FALSE;

    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
        return FALSE;

    pguid->Data2 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
        return FALSE;

    pguid->Data3 = (WORD)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[0] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, '-'))
        return FALSE;

    pguid->Data4[1] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[2] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[3] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[4] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[5] = (BYTE)dw;

    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[6] = (BYTE)dw;
    if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
        return FALSE;

    pguid->Data4[7] = (BYTE)dw;

    if (*lpsz++ != '}')
        return FALSE;

    if (*lpsz++ != '\0')
        return FALSE;

    return TRUE;
}

static int SignalPAN (void) {
    HANDLE hEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE, BTH_NAMEDEVENT_PAN_REFRESH);
    if (! hEvent)
        return FALSE;

    SetEvent (hEvent);
    CloseHandle (hEvent);

    return TRUE;
}

static void DeactivatePanDevice (BTDEV *pbtDevice) {
    WCHAR szRegKey[_MAX_PATH];
    StringCchPrintf (STRING_AND_COUNTOF(szRegKey), L"COMM\\%s\\Associations\\%04x%08x", gszAdapterName, GET_NAP(pbtDevice->b), GET_SAP(pbtDevice->b));
    RegDeleteKey (HKEY_LOCAL_MACHINE, szRegKey);

    HANDLE h = CreateFile(
                    NDISUIO_DEVICE_NAME,                                //    Object name.
                    0x00,                                    //    Desired access.
                    0x00,                                            //    Share Mode.
                    NULL,                                            //    Security Attr
                    OPEN_EXISTING,                                    //    Creation Disposition.
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,    //    Flag and Attributes..
                    (HANDLE)INVALID_HANDLE_VALUE); 
    if (INVALID_HANDLE_VALUE != h) {
        DWORD dwWritten = 0;
        
        struct {
            NDISUIO_SET_OID SetOid;
            unsigned char   uca[64];
        } NdisUioSetOid;

        NdisUioSetOid.SetOid.Oid = OID_PAN_DISCONNECT;
        NdisUioSetOid.SetOid.ptcDeviceName = L"BTPAN1";
        memcpy(&NdisUioSetOid.SetOid.Data, &pbtDevice->b, sizeof(BD_ADDR));

        DeviceIoControl(
            h,
            IOCTL_NDISUIO_SET_OID_VALUE,
            &NdisUioSetOid,
            sizeof(NdisUioSetOid),
            NULL,
            0,
            &dwWritten,
            NULL);

        CloseHandle(h);
    }
}

static int ActivatePanDevice (BTDEV *pbtDevice, int fFirstTime) {
    WCHAR szRegKey[_MAX_PATH];
    StringCchPrintf (STRING_AND_COUNTOF(szRegKey), L"COMM\\%s\\Associations\\%04x%08x", gszAdapterName, GET_NAP(pbtDevice->b), GET_SAP(pbtDevice->b));

    DWORD dwDisp;
    HKEY hk;
    if (ERROR_SUCCESS == RegCreateKeyEx (HKEY_LOCAL_MACHINE, szRegKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, &dwDisp)) {
        WCHAR szAddr[40];
        StringCchPrintf (STRING_AND_COUNTOF(szAddr), L"%04x%08x", GET_NAP(pbtDevice->b), GET_SAP(pbtDevice->b));

        RegSetValueEx (hk, L"Address", 0, REG_SZ, (BYTE *)szAddr, sizeof(WCHAR) * (wcslen(szAddr) + 1));

        WCHAR szServiceId[60];
        StringCchPrintf (STRING_AND_COUNTOF(szServiceId), L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                            pbtDevice->service_id.Data1, pbtDevice->service_id.Data2, pbtDevice->service_id.Data3,
                            pbtDevice->service_id.Data4[0], pbtDevice->service_id.Data4[1], pbtDevice->service_id.Data4[2], pbtDevice->service_id.Data4[3], 
                            pbtDevice->service_id.Data4[4], pbtDevice->service_id.Data4[5], pbtDevice->service_id.Data4[6], pbtDevice->service_id.Data4[7]); 
        RegSetValueEx (hk, L"ServiceId", 0, REG_SZ, (BYTE*)szServiceId, (wcslen(szServiceId) + 1) * sizeof(WCHAR));

        WCHAR szSSID[50];
        StringCchPrintf (STRING_AND_COUNTOF(szSSID), L"PAN@%04x%08x", GET_NAP(pbtDevice->b), GET_SAP(pbtDevice->b));
        RegSetValueEx (hk, L"SSID", 0, REG_SZ, (BYTE*)szSSID, (wcslen(szSSID) + 1) * sizeof(WCHAR));

        DWORD dw = 1;
        RegSetValueEx (hk, L"Priority", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));

        RegCloseKey (hk);

        HANDLE h = CreateFile(
                        NDISUIO_DEVICE_NAME,                                //    Object name.
                        0x00,                                    //    Desired access.
                        0x00,                                            //    Share Mode.
                        NULL,                                            //    Security Attr
                        OPEN_EXISTING,                                    //    Creation Disposition.
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,    //    Flag and Attributes..
                        (HANDLE)INVALID_HANDLE_VALUE); 
        if (INVALID_HANDLE_VALUE != h) {
            DWORD dwWritten;

            struct {
                NDISUIO_SET_OID SetOid;
                unsigned char   uca[64];
            } NdisUioSetOid;

            // --- Set Authentication for the connection
            
            NdisUioSetOid.SetOid.Oid = OID_PAN_AUTHENTICATE;
            NdisUioSetOid.SetOid.ptcDeviceName = L"BTPAN1";
            *(PDWORD)NdisUioSetOid.SetOid.Data = pbtDevice->fAuth;

            dwWritten = 0;
            if (! DeviceIoControl(
                        h,
                        IOCTL_NDISUIO_SET_OID_VALUE,
                        &NdisUioSetOid,
                        sizeof(NdisUioSetOid),
                        NULL,
                        0,
                        &dwWritten,
                        NULL)) {
                CloseHandle(h);                
                return FALSE;                         
            }

            // --- Set Encryption for the connection

            NdisUioSetOid.SetOid.Oid = OID_PAN_ENCRYPT;
            NdisUioSetOid.SetOid.ptcDeviceName = L"BTPAN1";
            *(PDWORD)NdisUioSetOid.SetOid.Data = pbtDevice->fEncrypt;

            dwWritten = 0;
            if (! DeviceIoControl(
                        h,
                        IOCTL_NDISUIO_SET_OID_VALUE,
                        &NdisUioSetOid,
                        sizeof(NdisUioSetOid),
                        NULL,
                        0,
                        &dwWritten,
                        NULL)) {
                CloseHandle(h);                
                return FALSE; 
            }
            

            // --- Connect

            NdisUioSetOid.SetOid.Oid = OID_PAN_CONNECT;
            NdisUioSetOid.SetOid.ptcDeviceName = L"BTPAN1";
            memcpy(&NdisUioSetOid.SetOid.Data, &pbtDevice->b, sizeof(BD_ADDR));
            memcpy((PBYTE)NdisUioSetOid.SetOid.Data + sizeof(BD_ADDR), &pbtDevice->service_id, sizeof(GUID));

            dwWritten = 0;
            if (! DeviceIoControl(
                        h,
                        IOCTL_NDISUIO_SET_OID_VALUE,
                        &NdisUioSetOid,
                        sizeof(NdisUioSetOid),
                        NULL,
                        0,
                        &dwWritten,
                        NULL)) {
                CloseHandle(h);                
                return FALSE; 
            }

            CloseHandle(h);
            return TRUE;
        }    
    }

    return FALSE;
}

static void BthEnumInit (void) {
    static int fInited = FALSE;

    if (fInited)
        return;

    fInited = TRUE;

    HINSTANCE hCoreDll = LoadLibrary (L"coredll.dll");

    if(hCoreDll) {
        g_pfnRasGetEntryProperties = (p_RasGetEntryProperties)GetProcAddress (hCoreDll, L"RasGetEntryProperties");
        g_pfnRasSetEntryProperties = (p_RasSetEntryProperties)GetProcAddress (hCoreDll, L"RasSetEntryProperties");
        g_pfnRasSetEntryDialParams = (p_RasSetEntryDialParams)GetProcAddress (hCoreDll, L"RasSetEntryDialParams");
        g_pfnRasDeleteEntry        = (p_RasDeleteEntry)       GetProcAddress (hCoreDll, L"RasDeleteEntry");
    }

    if ((! g_pfnRasGetEntryProperties) || (! g_pfnRasSetEntryProperties) || (! g_pfnRasSetEntryDialParams) || (! g_pfnRasDeleteEntry)) {
        g_pfnRasGetEntryProperties = NULL;
        g_pfnRasSetEntryProperties = NULL;
        g_pfnRasSetEntryDialParams = NULL;
        g_pfnRasDeleteEntry        = NULL;
    }

    memset (gauth, 0, sizeof(gauth));
    memset (gencr, 0, sizeof(gencr));

    HKEY hk;
    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\device", 0, KEY_READ, &hk))
        return;

    int iNumDevices = 0;
    DWORD dwSize = sizeof (gidefault_mtu);

    DWORD dwType;   
    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"DefaultMtu", NULL, &dwType, (BYTE *)&gidefault_mtu, &dwSize)) ||
            (dwType != REG_DWORD) || (dwSize != sizeof(gidefault_mtu)))
        gidefault_mtu = 0;

    dwSize = sizeof (gidefault_auth);

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"DefaultAuth", NULL, &dwType, (BYTE *)&gidefault_auth, &dwSize)) ||
            (dwType != REG_DWORD) || (dwSize != sizeof(gidefault_auth)))
        gidefault_auth = 0;

    dwSize = sizeof (gidefault_encr);

    if ((ERROR_SUCCESS != RegQueryValueEx (hk, L"DefaultEncrypt", NULL, &dwType, (BYTE *)&gidefault_encr, &dwSize)) ||
            (dwType != REG_DWORD) || (dwSize != sizeof(gidefault_encr)))
        gidefault_encr = 0;

    for (int i = 0; i < BTENUM_CLASSES + 1; ++i) {
        HKEY hkdetail;
        if (ERROR_SUCCESS == RegOpenKeyEx (hk, gszKeyNames[i], 0,  KEY_ALL_ACCESS , &hkdetail)) {
            dwSize = sizeof (gimtu[i]);

            if ((ERROR_SUCCESS != RegQueryValueEx (hkdetail, L"DefaultMtu", NULL, &dwType, (BYTE *)&gimtu[i], &dwSize)) ||
                (dwType != REG_DWORD) || (dwSize != sizeof(gimtu[i])))
                gimtu[i] = -1;

            dwSize = sizeof (gauth[i]);
            if ((ERROR_SUCCESS != RegQueryValueEx (hkdetail, L"DefaultAuth", NULL, &dwType, (BYTE *)&gauth[i], &dwSize)) ||
                (dwType != REG_DWORD) || (dwSize != sizeof(gauth[i])))
                gauth[i] = 0;

            if (gauth[i])
                gauth[i] = TRUE;

            dwSize = sizeof (gencr[i]);
            if ((ERROR_SUCCESS != RegQueryValueEx (hkdetail, L"DefaultEncrypt", NULL, &dwType, (BYTE *)&gencr[i], &dwSize)) ||
                (dwType != REG_DWORD) || (dwSize != sizeof(gencr[i])))
                gencr[i] = 0;

            if (gencr[i])
                gencr[i] = TRUE;

            if (i == BTENUM_DEVICE_PAN) {
                dwSize = sizeof (gszAdapterName);
                if ((ERROR_SUCCESS != RegQueryValueEx (hkdetail, L"AdapterName", NULL, &dwType, (BYTE *)gszAdapterName, &dwSize)) ||
                    (dwType != REG_SZ) || (dwSize >= sizeof(gszAdapterName)))
                    gszAdapterName[0] = '\0';
            }

            RegCloseKey (hkdetail); 
        }
    }

    RegCloseKey (hk);
}

int BthEnumDevices (void *pContext, BthEnumCallback pCallback) {
    BthEnumInit ();

    HKEY hk;
    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"software\\microsoft\\bluetooth\\device", 0, KEY_READ, &hk))
        return 0;

    int iNumDevices = 0;

    for (int i = 0; i < BTENUM_CLASSES + 1; ++i) {
        HKEY hkdetail;
        if (ERROR_SUCCESS == RegOpenKeyEx (hk, gszKeyNames[i], 0,  KEY_ENUMERATE_SUB_KEYS , &hkdetail)) {
            WCHAR szTemp1[50];
            szTemp1[0] = '\0';

            DWORD dwSizeKey = sizeof(szTemp1)/sizeof(WCHAR);

            int iContinue = TRUE;
            for (int j = 0; iContinue && (ERROR_SUCCESS == RegEnumKeyEx(hkdetail, j, szTemp1, &dwSizeKey, NULL, NULL, 0, NULL)); ++j) {

                BT_ADDR b;

                HKEY hkdevice;
                if (GetBA(szTemp1, &b) && (ERROR_SUCCESS == RegOpenKeyEx (hkdetail, szTemp1, 0, KEY_READ, &hkdevice))) {
                    BTDEV bdev(b, i < BTENUM_CLASSES ? i : -1);

                    DWORD dwSize = sizeof(bdev.szDeviceName);
                    DWORD dwType = 0;

                    if ((ERROR_SUCCESS != RegQueryValueEx (hkdevice, L"name", NULL, &dwType, (BYTE *)bdev.szDeviceName, &dwSize)) ||
                        (dwType != REG_SZ) || (dwSize > sizeof(bdev.szDeviceName)))
                        bdev.szDeviceName[0] = '\0';

                    dwSize = sizeof(bdev.szPortName);

                    if ((ERROR_SUCCESS != RegQueryValueEx (hkdevice, L"port_name", NULL, &dwType, (BYTE *)bdev.szPortName, &dwSize)) ||
                        (dwType != REG_SZ) || (dwSize > sizeof(bdev.szPortName)))
                        bdev.szPortName[0] = '\0';

                    DWORD dw = 0;
                    dwSize = sizeof(dw);
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"channel", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
                        (dwType == REG_DWORD) && (dwSize == sizeof(dw)) && dw)
                        bdev.ucChannel = (unsigned char)dw;                         

                    dwSize = sizeof(dw);
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"hid_subclass", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
                        (dwType == REG_DWORD) && (dwSize == sizeof(dw)))
                        bdev.dwHidDevClass = dw;

                    dwSize = sizeof(dw);
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"handle", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
                        (dwType == REG_DWORD) && (dwSize == sizeof(dw)))
                        bdev.hDevHandle = (HANDLE)dw;

                    dwSize = sizeof(dw);
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"active", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
                        (dwType == REG_DWORD) && (dwSize == sizeof(dw)) && dw)
                        bdev.fActive = TRUE;

                    dwSize = sizeof(dw);
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"auth", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
                        (dwType == REG_DWORD) && (dwSize == sizeof(dw)))
                        bdev.fAuth = (dw != 0);

                    dwSize = sizeof(dw);
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"encrypt", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
                        (dwType == REG_DWORD) && (dwSize == sizeof(dw)))
                        bdev.fEncrypt = (dw != 0);

                    dwSize = sizeof(dw);
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"mtu", NULL, &dwType, (BYTE *)&dw, &dwSize)) &&
                        (dwType == REG_DWORD) && (dwSize == sizeof(dw)))
                        bdev.imtu = dw;

                    WCHAR szServiceId[64];
                    dwSize = sizeof(szServiceId);
                    GUID g;
                    if ((ERROR_SUCCESS == RegQueryValueEx (hkdevice, L"service_id", NULL, &dwType, (BYTE *)szServiceId, &dwSize)) &&
                        (dwType == REG_SZ) && (dwSize < sizeof(szServiceId)) && StrToGUID(szServiceId, &g))
                        bdev.service_id = g;

                    dwType = 0;
                    dwSize = 0;

                    DWORD dwRes = RegQueryValueEx (hkdevice, L"sdp_record", NULL, &dwType, NULL, &dwSize);

                    if ((dwRes == ERROR_SUCCESS) && (dwType == REG_BINARY)) {
                        unsigned char *psdp = (unsigned char *)LocalAlloc(LMEM_FIXED, dwSize);
                        if (psdp)
                            dwRes = RegQueryValueEx (hkdevice, L"sdp_record", NULL, &dwType, psdp, &dwSize);

                        if (dwRes == ERROR_SUCCESS) {
                            bdev.psdp = psdp;
                            bdev.csdp = dwSize;
                        } else
                            LocalFree (psdp);
                    }

                    bdev.fTrusted = TRUE;
                    RegCloseKey (hkdevice);

                    ++iNumDevices;
                    iContinue = pCallback (pContext, &bdev);
                }

                dwSizeKey = sizeof(szTemp1)/sizeof(WCHAR);
            }
            RegCloseKey (hkdetail); 
        }
    }

    RegCloseKey (hk);

    return 0;
}

int BthEnumUpdate (BTDEV* bt) {
    BthEnumInit ();

    HKEY hkdevice;
    DWORD dwDisp;

    WCHAR szKey[2*_MAX_PATH];
    StringCchPrintf (STRING_AND_COUNTOF(szKey), L"software\\microsoft\\bluetooth\\device\\%s\\%04x%08x", bt->iDeviceClass != -1 ? gszKeyNames[bt->iDeviceClass] : L"UNKNOWN", GET_NAP(bt->b), GET_SAP(bt->b));
    if(ERROR_SUCCESS == RegCreateKeyEx (HKEY_LOCAL_MACHINE, szKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkdevice, &dwDisp)){
        RegSetValueEx (hkdevice, L"name", 0, REG_SZ, (BYTE *)bt->szDeviceName, (wcslen (bt->szDeviceName) + 1) * sizeof(WCHAR));
        RegSetValueEx (hkdevice, L"port_name", 0, REG_SZ, (BYTE *)bt->szPortName, (wcslen (bt->szPortName) + 1) * sizeof(WCHAR));
        DWORD dw = bt->ucChannel;
        RegSetValueEx (hkdevice, L"channel", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
        dw = bt->dwHidDevClass;
        RegSetValueEx (hkdevice, L"hid_subclass", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
        dw = bt->imtu;
        RegSetValueEx (hkdevice, L"mtu", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
        dw = bt->fActive;
        RegSetValueEx (hkdevice, L"active", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw)); 
        dw = bt->fAuth;
        RegSetValueEx (hkdevice, L"auth", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw)); 
        dw = bt->fEncrypt;
        RegSetValueEx (hkdevice, L"encrypt", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
        dw = (DWORD)bt->hDevHandle;
        RegSetValueEx (hkdevice, L"handle", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

        WCHAR szServiceId[60];
        StringCchPrintf (STRING_AND_COUNTOF(szServiceId), L"{%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x}",
                            bt->service_id.Data1, bt->service_id.Data2, bt->service_id.Data3,
                            bt->service_id.Data4[0], bt->service_id.Data4[1], bt->service_id.Data4[2], bt->service_id.Data4[3], 
                            bt->service_id.Data4[4], bt->service_id.Data4[5], bt->service_id.Data4[6], bt->service_id.Data4[7]); 
        RegSetValueEx (hkdevice, L"service_id", 0, REG_SZ, (BYTE*)szServiceId, (wcslen(szServiceId) + 1) * sizeof(WCHAR));

        if (bt->psdp)
            RegSetValueEx (hkdevice, L"sdp_record", 0, REG_BINARY, bt->psdp, bt->csdp);
        else
            RegDeleteValue (hkdevice, L"sdp_record");

        RegCloseKey (hkdevice);

        RegFlushKey (HKEY_LOCAL_MACHINE);

        return TRUE;
    }

    return FALSE;
}

int BthEnumRemove (BTDEV* bt) {
    BthEnumInit ();

    WCHAR szKey[2*_MAX_PATH];
    StringCchPrintf (STRING_AND_COUNTOF(szKey), L"software\\microsoft\\bluetooth\\device\\%s\\%04x%08x", bt->iDeviceClass != -1 ? gszKeyNames[bt->iDeviceClass] : L"UNKNOWN", GET_NAP(bt->b), GET_SAP(bt->b));
    RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);

    return TRUE;
}

static int DisconnectFromHidDevice(BTDEV *pbt) {
    int iRet = TRUE;

    ASSERT(pbt->hDevHandle);

    HANDLE hFile = CreateFile (L"BHI0:", 0, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    iRet = DeviceIoControl (hFile, BTHHID_IOCTL_HIDDisconnect, &pbt->b, sizeof(pbt->b), NULL, 0, NULL, NULL);

    CloseHandle (hFile);

    return iRet;
}

static int ConnectToHidDevice (BTDEV *pbt, BOOL fFirstTime) {
    int iRet = TRUE;
    
    pbt->hDevHandle = ActivateDeviceEx(HID_REGKEY_SZ, NULL, 0, HID_CLIENT_REGKEY_SZ);
    if (pbt->hDevHandle == NULL)
        return FALSE;

    if (fFirstTime) {
        HANDLE hFile = CreateFile (L"BHI0:", 0, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return FALSE;

        iRet = DeviceIoControl (hFile, BTHHID_IOCTL_HIDConnect, &pbt->b, sizeof(pbt->b), NULL, 0, NULL, NULL);

        CloseHandle (hFile);
    }
    
    return iRet;
}

static void DeleteRegForCSRWPP (BTDEV* pbtDevice){

   HKEY  regKey;

   if (BTENUM_DEVICE_OBEX_FTP == pbtDevice->iDeviceClass || 
       BTENUM_DEVICE_OBEX_OPP == pbtDevice->iDeviceClass)
   {
      WCHAR deviceAddr[128];

      wsprintf (deviceAddr, L"Software\\Microsoft\\Bluetooth\\device\\%04x%08x", GET_NAP(pbtDevice->b),
         GET_SAP(pbtDevice->b));
             
      if (ERROR_SUCCESS != RegDeleteKey(HKEY_LOCAL_MACHINE, deviceAddr))
      {
         NKDbgPrintfW(L"<DeleteRegForCSRWPP> Registry Delete - Failure\n");            
      }
   }
   else if (BTENUM_DEVICE_HEADSET == pbtDevice->iDeviceClass)
   {
      if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"\\SOFTWARE\\CSR\\Modules\\AVM", 0, 0, 
          &regKey))
      {
         WCHAR deviceAddr[40];
         WCHAR enquiredAddr [40];
         DWORD dwType;
         DWORD dwSize = 0;
         DWORD retVal;
         
         wsprintf (deviceAddr, L"%04x%08x", GET_NAP(pbtDevice->b), GET_SAP(pbtDevice->b));

         dwSize = (wcslen(deviceAddr) + 1) * sizeof(WCHAR);

         retVal = RegQueryValueEx(regKey, L"bluetoothAddress", NULL, &dwType, (BYTE *)enquiredAddr,
                  &dwSize);

         if (ERROR_SUCCESS == retVal)
         {
            if (wcsicmp (deviceAddr, enquiredAddr) == 0)
            {
               retVal = RegDeleteValue (regKey, L"bluetoothAddress");
               if (ERROR_SUCCESS == retVal)
               {
                  HKEY key;
                  
                  if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"\\Drivers\\BuiltIn\\WaveDev", 0, 0,
                      &key))
                  {
                     DWORD val = 0;
                     
                     RegSetValueEx(key, (LPCTSTR)TEXT("streamOn"), 0, REG_DWORD, (LPBYTE)&val , sizeof(DWORD));
                     RegCloseKey(key);
                  }
                  else
                  {
                     NKDbgPrintfW(L"<DeleteRegForCSRWPP> WaveDev Registry open - Failure\n");                        
                  }
               }
               else
               {
                  NKDbgPrintfW(L"<DeleteRegForCSRWPP> BTAddress Reg Delete Failure. Error code %d \r\n",
                                retVal); 
               }
            }
            else
            {
               NKDbgPrintfW (L"<DeleteRegForCSRWPP> BT device not the active headset\r\n");               
            }
         }
         else
         {
            NKDbgPrintfW (L"<DeleteRegForCSRWPP> Error Code %d size of reg value %d\r\n", retVal, dwSize);
         }

         RegCloseKey(regKey);   
      }
      else
      {
         NKDbgPrintfW(L"<DeleteRegForCSRWPP> AVM Registry open - Failure\n");
      }  
   }
}

int BthEnumDeactivate (BTDEV* pbtDevice) {
    BthEnumInit ();

    pbtDevice->fActive = FALSE;

    if (pbtDevice->iDeviceClass == BTENUM_DEVICE_HID) {
        DisconnectFromHidDevice(pbtDevice);
    }

    if (pbtDevice->hDevHandle) {
        DeactivateDevice (pbtDevice->hDevHandle);
        pbtDevice->hDevHandle = NULL;
    }
   
   /* Do CSR WPP specific registry changes */
   DeleteRegForCSRWPP (pbtDevice);
   
    if(pbtDevice->iDeviceClass == BTENUM_DEVICE_PRINTER) {
        HKEY hk;
        if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"Printers\\Ports", 0, KEY_ALL_ACCESS, &hk))
            return FALSE;

        for (int i = 1; i < 10 ; ++i) {
            WCHAR szPortName[10];
            StringCchPrintf (STRING_AND_COUNTOF(szPortName), L"port%d", i);

            WCHAR szString[256];
            DWORD dwSize = sizeof(szString);
            DWORD dwType;
            if (ERROR_SUCCESS == RegQueryValueEx (hk, szPortName, NULL, &dwType, (BYTE *)szString, &dwSize)) {
                if ((dwType == REG_SZ) && (wcsicmp (szString, pbtDevice->szPortName) == 0)) {
                    RegDeleteValue (hk, szPortName);
                    break;
                }
            }
        }

        RegCloseKey (hk);
    } else if ((pbtDevice->iDeviceClass == BTENUM_DEVICE_ASYNC) && g_pfnRasDeleteEntry)
        g_pfnRasDeleteEntry (NULL, BTENUM_RAS_NAME);
    else if (pbtDevice->iDeviceClass == BTENUM_DEVICE_PAN) {
        DeactivatePanDevice (pbtDevice);
    }
    pbtDevice->szPortName[0] = '\0';

    return TRUE;
}

static DWORD FindFreeBluetoothPort (BTDEV *pbt) {
    for (int i = 2 ; i < 10 ; ++i) {
        WCHAR szPortName[20];
        StringCchPrintf (STRING_AND_COUNTOF(szPortName), BTENUM_PORT_NAME L"%d:", i);

        HANDLE hFile = CreateFile (szPortName, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            return i;
        CloseHandle (hFile);
    }

    return 10;
}

static DWORD GetBluetoothPort (BTDEV *pbt) {
    if ((wcslen (pbt->szPortName) == 5) && (pbt->szPortName[4] == ':') &&
        (pbt->szPortName[3] >= '0') && (pbt->szPortName[3] <= '9'))
        return pbt->szPortName[3] - '0';

    return 10;
}

static int ActivateBluetoothPort (BTDEV *pbt, DWORD dwIndex) {
    PORTEMUPortParams pp;

    memset (&pp, 0, sizeof(pp));
    pp.device  = pbt->b;
    pp.channel = pbt->ucChannel;
    pp.imtu    = pbt->imtu;

    if (pbt->fAuth)
        pp.uiportflags |= RFCOMM_PORT_FLAGS_AUTHENTICATE;

    if (pbt->fEncrypt)
        pp.uiportflags |= RFCOMM_PORT_FLAGS_ENCRYPT;

    WCHAR szKeyName[_MAX_PATH];

    StringCchPrintf (STRING_AND_COUNTOF(szKeyName), L"software\\microsoft\\bluetooth\\device\\ports\\%s", pbt->szDeviceName);

    HKEY hk;
    DWORD dwDisp = 0;

    if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, 0, KEY_WRITE, NULL, &hk, &dwDisp))
        return FALSE;

    RegSetValueEx (hk, L"dll", 0, REG_SZ, (BYTE *)L"btd.dll", sizeof(L"btd.dll"));
    RegSetValueEx (hk, L"prefix", 0, REG_SZ, (BYTE *)BTENUM_PORT_NAME, sizeof(BTENUM_PORT_NAME));

    RegSetValueEx (hk, L"index", 0, REG_DWORD, (BYTE *)&dwIndex, sizeof(dwIndex));

    DWORD dw = (DWORD) &pp;
    RegSetValueEx (hk, L"context", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

    if ((pbt->iDeviceClass == BTENUM_DEVICE_MODEM) || (pbt->iDeviceClass == BTENUM_DEVICE_LAP) || (pbt->iDeviceClass == BTENUM_DEVICE_ASYNC)) {
        HKEY hk2;

        if (ERROR_SUCCESS != RegCreateKeyEx (hk, L"unimodem", 0, NULL, 0, KEY_WRITE, NULL, &hk2, &dwDisp)) {
            RegCloseKey (hk);
            RegDeleteKey (HKEY_LOCAL_MACHINE, szKeyName);
            return FALSE;
        }

        RegSetValueEx (hk2, L"friendlyname", 0, REG_SZ, (BYTE *)pbt->szDeviceName, (wcslen (pbt->szDeviceName) + 1) * sizeof(WCHAR));
        RegSetValueEx (hk2, L"tsp", 0, REG_SZ, (BYTE *)L"unimodem.dll", sizeof(L"unimodem.dll"));
        dw = pbt->iDeviceClass == BTENUM_DEVICE_MODEM ? 1 : 0;
        RegSetValueEx (hk2, L"devicetype", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));

        RegCloseKey (hk2);
    }

    RegCloseKey (hk);

    pbt->hDevHandle = ActivateDevice (szKeyName, 0);

    return pbt->hDevHandle != NULL;
}

int BthEnumActivate (BTDEV *pbtDevice, int fFirstTime) {
    BthEnumInit ();

    pbtDevice->fActive = TRUE;

    //if it is an SPP-based device
    if ((pbtDevice->iDeviceClass == BTENUM_DEVICE_MODEM) ||
        (pbtDevice->iDeviceClass == BTENUM_DEVICE_LAP) ||
        (pbtDevice->iDeviceClass == BTENUM_DEVICE_PRINTER) ||
        (pbtDevice->iDeviceClass == BTENUM_DEVICE_ASYNC)) {

        DWORD dwIndex = fFirstTime ? FindFreeBluetoothPort (pbtDevice) : GetBluetoothPort (pbtDevice);
        if (dwIndex == 10) {
            pbtDevice->fActive = FALSE;
            return FALSE;
        }

        if (fFirstTime)
            wsprintf (pbtDevice->szPortName, BTENUM_PORT_NAME L"%d:", dwIndex);

        pbtDevice->fActive = ActivateBluetoothPort (pbtDevice, dwIndex);

        if (! pbtDevice->fActive)
            return FALSE;
    }

    //if printer
    if (pbtDevice->iDeviceClass == BTENUM_DEVICE_PRINTER) {
        HKEY hk;
        if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"Printers\\Ports", 0, KEY_ALL_ACCESS, &hk)) {
            pbtDevice->fActive = FALSE;

            DeactivateDevice (pbtDevice->hDevHandle);
            pbtDevice->hDevHandle = NULL;

            return FALSE;
        }

        //add the COM port mapping into the Printer\Ports key so that other applications can pick it up
        int iMinPort = 10;
        WCHAR szPortName[10];
        for (int i = 1; i < 10 ; ++i) {
            wsprintf (szPortName, L"port%d", i);
            WCHAR szString[_MAX_PATH];
            DWORD dwType;
            DWORD dwSize = sizeof(szString);
            if (ERROR_SUCCESS == RegQueryValueEx (hk, szPortName, NULL, &dwType, (BYTE *)szString, &dwSize)) {
                if ((dwType == REG_SZ) && (wcsicmp (szString, pbtDevice->szPortName) == 0)) {
                    RegCloseKey (hk);
                    return TRUE;
                }
            } else if (iMinPort > i)
                iMinPort = i;
        }

        if (iMinPort < 10) {
            wsprintf (szPortName, L"port%d", iMinPort);
            RegSetValueEx (hk, szPortName, 0, REG_SZ, (BYTE *)pbtDevice->szPortName, (wcslen (pbtDevice->szPortName) + 1) * sizeof(WCHAR));
        } else
            pbtDevice->fActive = FALSE;

        RegCloseKey (hk);

        RegFlushKey (HKEY_LOCAL_MACHINE);
    }
    else if ((pbtDevice->iDeviceClass == BTENUM_DEVICE_HEADSET) ||
            (pbtDevice->iDeviceClass == BTENUM_DEVICE_HANDSFREE)) {
        // Open or create key to save BT_ADDR.  This registry value is used by SCO driver.
        HKEY hk;
        DWORD dwDis = 0;
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Bluetooth\\AudioGateway\\Devices\\1", 0, NULL, 0, NULL, NULL, &hk, &dwDis)) {
            BD_ADDR b;
            b.NAP = GET_NAP(pbtDevice->b);
            b.SAP = GET_SAP(pbtDevice->b);
            RegSetValueEx(hk, L"Address", 0, REG_BINARY, (PBYTE)&b, sizeof(b));
            RegSetValueEx(hk, L"Service", 0, REG_BINARY, (PBYTE)&pbtDevice->service_id, sizeof(GUID));
            RegCloseKey (hk);

            RegFlushKey (HKEY_LOCAL_MACHINE);
        } else {
            pbtDevice->fActive = FALSE;
        }
    } else if ((pbtDevice->iDeviceClass==BTENUM_DEVICE_ASYNC) && g_pfnRasGetEntryProperties) {
        RASENTRY RasEntry;

        RasEntry.dwSize = sizeof(RASENTRY);
        DWORD cb = sizeof(RASENTRY);
        g_pfnRasGetEntryProperties (NULL, L"", &RasEntry, &cb, NULL, NULL);

        RasEntry.dwfOptions &= ~(RASEO_SpecificNameServers|RASEO_SpecificIpAddr|
                                    RASEO_IpHeaderCompression|RASEO_SwCompression|RASEO_UseCountryAndAreaCodes);

        wcscpy (RasEntry.szDeviceType, L"direct");
        wcsncpy (RasEntry.szDeviceName, pbtDevice->szDeviceName, sizeof(RasEntry.szDeviceName)/sizeof(RasEntry.szDeviceName[0])-1);
        RasEntry.szDeviceName[sizeof(RasEntry.szDeviceName)/sizeof(RasEntry.szDeviceName[0])-1] = '\0';

        g_pfnRasSetEntryProperties(NULL, BTENUM_RAS_NAME, &RasEntry, sizeof(RasEntry), NULL, 0);

        RASDIALPARAMS   RasDialParams;
        memset((char *)&RasDialParams, 0, sizeof(RasDialParams));

        RasDialParams.dwSize = sizeof(RASDIALPARAMS);
        wcscpy (RasDialParams.szEntryName, BTENUM_RAS_NAME);

        wcscpy (RasDialParams.szUserName, L"guest");
        wcscpy (RasDialParams.szPassword, L"guest");

        g_pfnRasSetEntryDialParams(NULL, &RasDialParams, FALSE);

        RegFlushKey (HKEY_LOCAL_MACHINE);
    } else if (pbtDevice->iDeviceClass == BTENUM_DEVICE_HID) {  // HID
            pbtDevice->fActive = ConnectToHidDevice (pbtDevice, fFirstTime);
    } else if (pbtDevice->iDeviceClass == BTENUM_DEVICE_PAN) {
            pbtDevice->fActive = ActivatePanDevice (pbtDevice, fFirstTime);
    }

    return pbtDevice->fActive;
}

int BthEnumGetClassDefaults (int iClass, BTDEV *pbt) {
    pbt->imtu     = gidefault_mtu;
    pbt->fAuth    = gidefault_auth;
    pbt->fEncrypt = gidefault_encr;

    if ((iClass >= 0) && (iClass < sizeof(gimtu)/sizeof(gimtu[0]))) {
        if (gimtu[iClass] >= 0)
            pbt->imtu = gimtu[iClass];

        pbt->fAuth = gauth[iClass];
        pbt->fEncrypt = gencr[iClass];
    }

    return TRUE;
}
