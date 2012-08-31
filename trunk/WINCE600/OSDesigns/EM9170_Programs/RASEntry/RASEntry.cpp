// RASEntry.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
// RASEntry.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <ras.h>
#include <raserror.h>
#include <tapi.h>
#include "unimodem.h"

#define _TEOF		WEOF

#define _FILENAM  "RasEntry"

/*		The enumerated constants in "LabelValue" and the corresponding array  */
/* "Labels" are used to parse the RAS configuration file for matching strings.*/

enum {
	VAL_NAME,
	VAL_COUNTRYCODE,
	VAL_AREACODE,
	VAL_PHONE,
	VAL_SPECIFICIPADDR,
	VAL_IPADDR,
	VAL_SPECIFICNAMESERVERS,
	VAL_DNSADDR,
	VAL_ALTDNSADDR,
	VAL_WINSADDR,
	VAL_ALTWINSADDR,
	VAL_DEVICETYPE,
	VAL_DEVICENAME,
	VAL_IPHEADERCOMPRESSION,
	VAL_SWCOMPRESSION,
	VAL_USECOUNTRYANDAREACODES,
	VAL_REQUIREENCRYPTEDPW,
	VAL_REQUIREMSENCRYPTEDPW,
	VAL_REQUIREDATAENCRYPTION,
	VAL_DIALASLOCALCALL,
	VAL_FRAMING,
	VAL_FRAMESIZE,
	VAL_SCRIPT,
	VAL_USERNAME,
	VAL_PASSWORD,
	VAL_DOMAIN,
    VAL_BAUDRATE,
	VAL_ATTACH,
	VAL_MAX
}  LabelValue;

TCHAR	*Labels[] = {
	TEXT("Name"),
	TEXT("CountryCode"),
	TEXT("AreaCode"),
	TEXT("Phone"),
	TEXT("SpecificIpAddr"),
	TEXT("IpAddr"),
	TEXT("SpecificNameServers"),
	TEXT("DnsAddr"),
	TEXT("AltDnsAddr"),
	TEXT("WinsAddr"),
	TEXT("AltWinsAddr"),
	TEXT("DeviceType"),
	TEXT("DeviceName"),
	TEXT("IpHeaderCompression"),
	TEXT("SwCompression"),
	TEXT("UseCountryAndAreaCodes"),
	TEXT("RequireEncryptedPw"),
	TEXT("RequireMsEncryptedPw"),
	TEXT("RequireDataEncryption"),
	TEXT("DialAsLocalCall"),
	TEXT("Framing"),
	TEXT("FrameSize"),
	TEXT("Script"),
	TEXT("UserName"),
	TEXT("Password"),
	TEXT("Domain"), 
	TEXT("BaudRate"),
	TEXT("AttachATCmd")
};

#define LOG_EXCEPTION          0				  /* Output verbosity levels */
#define LOG_FAIL               2
#define LOG_WARNING            3
#define LOG_ABORT              4
#define LOG_SKIP               6
#define LOG_NOT_IMPLEMENTED    8
#define LOG_PASS              10
#define LOG_DETAIL            12
#define LOG_COMMENT           14
#define LOG_MAX_VERBOSITY     15

#define MAX_LINE			 256
#define MAXENTRIES            25      /* Maximum number of phonebook entries */

HANDLE		hFile;
DWORD		dwDebugFlag = LOG_PASS;
RASENTRY    RasEntry;
BOOL		g_fUseDevConfig;
BOOL		g_fDevName;
LPVARSTRING g_lpDevConfig;
DWORD		g_dwDevConfigLen;

BOOL SetBaudRate(TCHAR *s);


//------------------------------------------------------------------------
//	Program functions - prototypes:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TCHAR GetFileChar(void);
TCHAR *GetFileLine(TCHAR *s, int cch);
void Usage(void);
RASIPADDR *GetRasIpAddress(LPTSTR lpszIpString, RASIPADDR *lpRasIpAddr);
void LogDebugData(DWORD dwVerbosity, LPCTSTR szFormat, ...);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Functions:
    GetFileChar and GetFileLine

Description:
    These functions are called to read, respectively, a single character or a
	line from a RAS configuration file previously opened for reading.

-------------------------------------------------------------------*/

TCHAR GetFileChar(void)
{
	unsigned char ch;
	ULONG cnt;
    BOOL bRet;
	bRet = ReadFile( hFile, &ch, 1, &cnt, 0);
	if (!bRet || (0 == cnt))
		return _TEOF;
	return (TCHAR)ch;
}


TCHAR *GetFileLine(TCHAR *s, int cch)
{
	TCHAR ch;
	TCHAR *pch;

	pch = s;
	while ((ch = GetFileChar()) != _TEOF)
	{
		if (ch == TEXT('\r')|| ch == TEXT('\n'))
		{
			if (pch == s)
				continue;
			break;
		}
		else
		{
			//PREFAST_SUPPRESS(394, "False positive since we pass in param cch as buffer size and do overrun check below");
			*pch++ = ch;
		}

		if (pch - s == (cch - 1))
			break;
	}

	if ((_TEOF == ch) && (pch == s))
		return (TCHAR *)_TEOF;

	*pch = TEXT('\0');
	return s;
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    GetRasIpAddress

Description:
    This function reads a string containing an IP address in dotted-decimal
	format and returns it in a RASIPADDR structure; if an error occurs, NULL
	is returned.  Bytes in the structure are ordered in big-endian format for
	NT, in little-endian format for CE.

-------------------------------------------------------------------*/

RASIPADDR *GetRasIpAddress(LPTSTR lpszIpString, RASIPADDR *lpRasIpAddr)
{
	unsigned a, b, c, d;

	//LogDebugData(LOG_MAX_VERBOSITY, TEXT("Reading IP address from \"%s\""),
	 // lpszIpString);
	if (_stscanf(lpszIpString, TEXT("%u.%u.%u.%u"), &a, &b, &c, &d) < 4)
		return NULL;
#ifdef UNDER_CE
	lpRasIpAddr->a = d;
	lpRasIpAddr->b = c;
	lpRasIpAddr->c = b;
	lpRasIpAddr->d = a;
#else
	lpRasIpAddr->a = a;
	lpRasIpAddr->b = b;
	lpRasIpAddr->c = c;
	lpRasIpAddr->d = d;
#endif
	return lpRasIpAddr;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    TapiCallback

Description:
    Stub because lineInitialize requires a callback function.
-------------------------------------------------------------------*/
void CALLBACK
TapiCallback(
    DWORD               hDevice,
    DWORD               dwMessage,
    DWORD               dwInstance,
    DWORD               dwParam1,
    DWORD               dwParam2,
    DWORD               dwParam3
    )
{
    return;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    FindDeviceID

Description:
    Given the device name, find the corresponding TAPI device ID.
    Set pfUnimodem to TRUE if the device is a unimodem device.
    
    Returns the TAPI device ID or -1 for failure.
-------------------------------------------------------------------*/
DWORD FindDeviceID(TCHAR * szDeviceName, BOOL * pfUnimodem)
{
    DWORD dwDeviceID;
    DWORD dwNumDevices;
    DWORD rc;
    HLINEAPP hLineApp;
    LPLINEDEVCAPS lpDevCaps;
    DWORD dwCapsLen;
    LPTSTR lpszTSP;

    //LogDebugData(LOG_DETAIL, TEXT(_FILENAM) TEXT(" - FindDeviceID: %s"), szDeviceName);

    rc = lineInitialize(
            &hLineApp,
            (HINSTANCE)1, // hInstance is unused
            TapiCallback,
            TEXT("RASENTRY"),
            &dwNumDevices
            );
    if (rc) 
	{
        //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - FindDeviceID - lineInitialize failed 0x%x"), rc);
        return -1;
    }

    dwCapsLen = sizeof(LINEDEVCAPS);

    //
    // Loop looking for a TAPI device whose line name matches szDeviceName
    //
    for (dwDeviceID = 0; dwDeviceID < dwNumDevices; dwDeviceID++) 
	{
        lpDevCaps = (LPLINEDEVCAPS)LocalAlloc(LPTR, dwCapsLen);
        if (NULL == lpDevCaps) 
		{
            //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - FindDeviceID - LocalAlloc failed"));
            goto fdi_fail;
        }
        lpDevCaps->dwTotalSize = dwCapsLen;
        rc = lineGetDevCaps(
                hLineApp,
                dwDeviceID,
                TAPI_CURRENT_VERSION,
                0,
                lpDevCaps
                );
        if (rc) 
		{
            LocalFree(lpDevCaps);
            continue;
        }

        //
        // Make sure we get the full devcaps info
        //
        if (lpDevCaps->dwTotalSize < lpDevCaps->dwNeededSize) 
		{
            dwCapsLen = lpDevCaps->dwNeededSize;
            LocalFree(lpDevCaps);
            lpDevCaps = (LPLINEDEVCAPS)LocalAlloc(LPTR, dwCapsLen);
            if (NULL == lpDevCaps) 
			{
                //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - FindDeviceID - LocalAlloc failed."));
                goto fdi_fail;
            }
            lpDevCaps->dwTotalSize = dwCapsLen;
            rc = lineGetDevCaps(
                    hLineApp,
                    dwDeviceID,
                    TAPI_CURRENT_VERSION,
                    0,
                    lpDevCaps
                    );
            if ((rc) || (lpDevCaps->dwTotalSize < lpDevCaps->dwNeededSize)) 
			{
                LocalFree(lpDevCaps);
                continue;
            }
        }

        //
        // Compare the TAPI device's line name.
        //
		if (!_tcscmp((LPTSTR) ((char *)lpDevCaps + lpDevCaps->dwLineNameOffset), szDeviceName)) 
		{
            //
            // It's a match. Now see if it's a unimodem device
            //
            if (pfUnimodem) {
                *pfUnimodem = FALSE;
                if (lpDevCaps->dwProviderInfoSize) 
				{
                    lpszTSP = (LPTSTR)((DWORD)lpDevCaps + lpDevCaps->dwProviderInfoOffset);
                    //LogDebugData(LOG_DETAIL, TEXT(_FILENAM) TEXT(" - FindDeviceID - TSP for %d is %s"), dwDeviceID, lpszTSP);
                    if (!_tcscmp(lpszTSP, TEXT("UNIMODEM"))) 
					{
                        *pfUnimodem = TRUE;
                    }
                }
            }
            lineShutdown(hLineApp);
            LocalFree(lpDevCaps);
            return dwDeviceID;
        }

    }   // for dwDeviceID

fdi_fail:
    //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - FindDeviceID - %s is not a UNIMODEM device"), szDeviceName);
    lineShutdown(hLineApp);
    return -1;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    SetBaudRate

Description:
    Convert the string form of the baud rate setting to an int and then
    use the lineDevSpecific interface to unimodem to edit the private
    structure.
-------------------------------------------------------------------*/
BOOL SetBaudRate(TCHAR * szBaud)
{
    BOOL fUnimodem;
    DWORD dwDeviceID;
    DWORD dwNumDevices;
    DWORD rc;
    HLINEAPP hLineApp;
    HLINE hLine;
    LPVARSTRING lpDevConfig;
    DWORD dwLen;
    UNIMDM_CHG_DEVCFG UCD;

    //
    // Get device ID of the TAPI device whose line name matches the RASENTRY szDeviceName
    //
    dwDeviceID = FindDeviceID(RasEntry.szDeviceName, &fUnimodem);
    if (dwDeviceID == -1) {
        return FALSE;
    }

    //
    // This interface is only supported by unimodem
    //
    if (fUnimodem == FALSE) 
	{
        return FALSE;
    }

    rc = lineInitialize(
            &hLineApp,
            (HINSTANCE)1, // hInstance is unused
            TapiCallback,
            TEXT("RASENTRY"),
            &dwNumDevices
            );
    if (rc) 
	{
        //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - SetBaudRate - lineInitialize failed 0x%x"), rc);
        return FALSE;
    }

    lpDevConfig = NULL;

    //
    // Open the TAPI device
    //
    rc = lineOpen(
            hLineApp,
            dwDeviceID,
            &hLine,
            TAPI_CURRENT_VERSION,
            0,
            0,
            LINECALLPRIVILEGE_MONITOR,
            LINEMEDIAMODE_DATAMODEM,
            NULL
            );
    if (rc) 
	{
        //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - SetBaudRate - lineOpen(%d) failed 0x%x"), dwDeviceID, rc);
        goto gbr_fail;
    }

    //
    // Find out how big its DevConfig is and get it.
    //
    dwLen = sizeof(VARSTRING);
    lpDevConfig = (LPVARSTRING)LocalAlloc(LPTR, dwLen);
    if (NULL == lpDevConfig) 
	{
        //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - SetBaudRate - LocalAlloc failed"));
        goto gbr_fail;
    }

    lpDevConfig->dwTotalSize = dwLen;
    rc = lineGetDevConfig(
            dwDeviceID,
            lpDevConfig,
            TEXT("comm/datamodem")
            );
    if (rc) 
	{
        //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - SetBaudRate - lineGetDevConfig(%d) failed 0x%x"), dwDeviceID, rc);
        goto gbr_fail;
    }

    if (lpDevConfig->dwTotalSize < lpDevConfig->dwNeededSize) 
	{
        dwLen = lpDevConfig->dwNeededSize;
        LocalFree(lpDevConfig);
        lpDevConfig = (LPVARSTRING)LocalAlloc(LPTR, dwLen);
        if (NULL == lpDevConfig) 
		{
            //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - SetBaudRate - LocalAlloc failed."));
            goto gbr_fail;
        }

        lpDevConfig->dwTotalSize = dwLen;
        rc = lineGetDevConfig(
                dwDeviceID,
                lpDevConfig,
                TEXT("comm/datamodem")
                );
        if (rc) 
		{
            //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - SetBaudRate - lineGetDevConfig(%d) failed 0x%x."), dwDeviceID, rc);
            goto gbr_fail;
        }
    }

    //
    // Use the lineDevSpecific interface to unimodem to edit the baud rate
    // in the devconfig.
    //
    UCD.dwCommand = UNIMDM_CMD_CHG_DEVCFG;
    UCD.lpszDeviceClass = TEXT("comm/datamodem");
    UCD.lpDevConfig = lpDevConfig;
    UCD.dwOption = UNIMDM_OPT_BAUDRATE;

    UCD.dwValue  = _tcstol(szBaud, (TCHAR)0, 10);
	//UCD.dwValue  = 57600;


    rc = lineDevSpecific(
            hLine,
            0,
            NULL,
            &UCD,
            sizeof(UCD)
            );
    if (!(rc & 0x80000000)) 
	{   // Valid async ID ?
        lineShutdown(hLineApp);
        g_lpDevConfig = lpDevConfig;  // gets freed at end of main()
        g_dwDevConfigLen = lpDevConfig->dwNeededSize;
        return TRUE;
    }

    //
    // Use the lineDevSpecific interface to unimodem to edit the baud rate
    // in the devconfig.
    //
    UCD.dwCommand = UNIMDM_CMD_CHG_DEVCFG;
    UCD.lpszDeviceClass = TEXT("comm/datamodem");
    UCD.lpDevConfig = lpDevConfig;
    UCD.dwOption = UNIMDM_OPT_DIALMOD;

	UCD.dwValue  = (DWORD)TEXT("+CGATT=1" );


    rc = lineDevSpecific(
            hLine,
            0,
            NULL,
            &UCD,
            sizeof(UCD)
            );
    if (!(rc & 0x80000000)) 
	{   // Valid async ID ?
        lineShutdown(hLineApp);
        g_lpDevConfig = lpDevConfig;  // gets freed at end of main()
        g_dwDevConfigLen = lpDevConfig->dwNeededSize;
        return TRUE;
    }

    //LogDebugData(LOG_WARNING, TEXT(_FILENAM) TEXT(" - SetBaudRate - lineDevSpecific(%d) failed 0x%x."), dwDeviceID, rc);

gbr_fail:
    if (lpDevConfig) 
	{
        LocalFree(lpDevConfig);
    }
    lineShutdown(hLineApp);
    return FALSE;
}


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	DWORD					cb;
	RASDIALPARAMS	RasDialParams;

	TCHAR				szName[RAS_MaxEntryName + 1];
	TCHAR				szLine[MAX_LINE];
	TCHAR				szFileName[MAX_LINE];
	DWORD				dwLineNum = 0;
	TCHAR				*szStart, *szEnd;
	char					FileName[MAX_LINE];
	TCHAR				szAttachCmd[RAS_MaxEntryName + 1];

	DWORD				dwLabel;
	int						i1;

    // This will create the default entries if the key does not exist.
    RasEnumEntries (NULL, NULL, NULL, &cb, NULL);


	/*	
	// list current entries
	LPRASENTRYNAME lpRasEntries;	// Pointer to the RasEntries.
	int            cEntries;			// Number of Entries found
	
	cb = MAXENTRIES * sizeof(RASENTRYNAME);
	lpRasEntries = (LPRASENTRYNAME)LocalAlloc(LPTR, cb);
	if (RasEnumEntries (NULL, NULL, lpRasEntries, &cb, &cEntries))
		printf("Unable to read RAS phonebook\n");
	else
	{
		for (i = 0; i < cEntries; i++)
			printf("Entry #%d: \"%s\""), i, lpRasEntries[i].szEntryName);
	}
	return 0;*/

	
	// Get a default RasEntry
	RasEntry.dwSize = sizeof(RASENTRY);
	cb = sizeof(RASENTRY);
	RasGetEntryProperties (NULL, TEXT(""), &RasEntry, &cb, NULL, NULL);
	memset((char *)&RasDialParams, 0, sizeof(RasDialParams));
	szName[0] = TEXT('\0');
	szFileName[0] = TEXT('\0');

#ifdef UNDER_CE
    g_fUseDevConfig = FALSE;
    g_fDevName = FALSE;
    g_lpDevConfig = NULL;
#endif

	_tcscpy( szAttachCmd, TEXT("+CGDCONT=1,\"IP\",\"CMNET\""));
	_tcscpy( szFileName, TEXT("\\Windows\\GPRSEntry.txt") );
	i1 = wcslen( lpCmdLine );
	if( i1 > 0 )
	{
		_tcscpy( szFileName, lpCmdLine );
	}
	i1 = wcslen( szFileName );
	wcstombs( FileName, szFileName, i1 );
	printf( "FileName:%s\n", FileName);

	hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		printf( "Open File %s fail!\n" );
		return -1;
	}

	while ((TCHAR *)_TEOF != GetFileLine(szLine, MAX_LINE))
	{
		dwLineNum++;
		//LogDebugData(LOG_DETAIL, TEXT("Line %d: '%s'"), dwLineNum, szLine);

		for (szStart = szLine; _istspace(*szStart); szStart++)
			;
		
		// Allow comment lines
		if ((TEXT(';') == *szStart) || (TEXT('#') == *szStart))
			continue;
		
		for (szEnd = szStart; *szEnd && !_istspace(*szEnd) &&  (TEXT('=') != *szEnd); szEnd++)
			;
		if (szEnd)
		{
			//PREFAST_SUPPRESS(394, "False positive since szEnd is not past the end of the buffer");
			*szEnd = TEXT('\0');
		}
		else
		{
			//LogDebugData(LOG_ABORT, TEXT("%d : Invalid input line '%s'"), dwLineNum, szLine);
			continue;
		}
		for (dwLabel = 0; dwLabel != VAL_MAX; dwLabel++)
		{
			if (!_tcsicmp (Labels[dwLabel], szStart))
				break;
		}
		szStart = szEnd + 1;

		switch (dwLabel)
		{
			case VAL_NAME:
				_tcsncpy(szName, szStart, RAS_MaxEntryName);
				break;
			case VAL_COUNTRYCODE:
				RasEntry.dwCountryCode = _ttol(szStart);
				break;
			case VAL_AREACODE:
				_tcsncpy (RasEntry.szAreaCode, szStart, RAS_MaxAreaCode);
				break;
			case VAL_PHONE:
				_tcsncpy (RasEntry.szLocalPhoneNumber, szStart,
				  RAS_MaxPhoneNumber);
				break;
			case VAL_SPECIFICIPADDR:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_SpecificIpAddr;
				else
					RasEntry.dwfOptions &= ~(RASEO_SpecificIpAddr);
				break;
			case VAL_IPADDR:
				GetRasIpAddress(szStart, &(RasEntry.ipaddr));
				break;
			case VAL_SPECIFICNAMESERVERS:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_SpecificNameServers;
				else
					RasEntry.dwfOptions &= ~(RASEO_SpecificNameServers);
				break;
			case VAL_DNSADDR:
				GetRasIpAddress(szStart, &(RasEntry.ipaddrDns));
				break;
			case VAL_ALTDNSADDR:
				GetRasIpAddress(szStart, &(RasEntry.ipaddrDnsAlt));
				break;
			case VAL_WINSADDR:
				GetRasIpAddress(szStart, &(RasEntry.ipaddrWins));
				break;
			case VAL_ALTWINSADDR:
				GetRasIpAddress(szStart, &(RasEntry.ipaddrWinsAlt));
				break;
			case VAL_DEVICETYPE:
				_tcsncpy (RasEntry.szDeviceType, szStart, RAS_MaxDeviceType + 1);
				break;
			case VAL_DEVICENAME:
				_tcsncpy (RasEntry.szDeviceName, szStart, RAS_MaxDeviceName + 1);
#ifdef UNDER_CE
                g_fDevName = TRUE;
#endif
				break;
			case VAL_IPHEADERCOMPRESSION:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_IpHeaderCompression;
				else
					RasEntry.dwfOptions &= ~(RASEO_IpHeaderCompression);
				break;
			case VAL_SWCOMPRESSION:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_SwCompression;
				else
					RasEntry.dwfOptions &= ~(RASEO_SwCompression);
				break;
			case VAL_USECOUNTRYANDAREACODES:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_UseCountryAndAreaCodes;
				else
					RasEntry.dwfOptions &= ~(RASEO_UseCountryAndAreaCodes);
				break;
			case VAL_REQUIREENCRYPTEDPW:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_RequireEncryptedPw;
				else
					RasEntry.dwfOptions &= ~(RASEO_RequireEncryptedPw);
				break;
			case VAL_REQUIREMSENCRYPTEDPW:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_RequireMsEncryptedPw;
				else
					RasEntry.dwfOptions &= ~(RASEO_RequireMsEncryptedPw);
				break;
			case VAL_REQUIREDATAENCRYPTION:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_RequireDataEncryption;
				else
					RasEntry.dwfOptions &= ~(RASEO_RequireDataEncryption);
				break;
#ifdef UNDER_CE
			case VAL_DIALASLOCALCALL:
				if ((*szStart == 'y') || (*szStart == 'Y'))
					RasEntry.dwfOptions |= RASEO_DialAsLocalCall;
				else
					RasEntry.dwfOptions &= ~(RASEO_DialAsLocalCall);
				break;
#endif
			case VAL_FRAMING :
				if ((*szStart == 's') || (*szStart == 'S'))
					RasEntry.dwFramingProtocol = RASFP_Slip;
				break;
			case VAL_FRAMESIZE:
				RasEntry.dwFrameSize = _ttol(szStart);
				break;
			case VAL_SCRIPT :
			    _tcsncpy (RasEntry.szScript, szStart, MAX_PATH);
				break;
			case VAL_USERNAME:
				_tcsncpy (RasDialParams.szUserName, szStart, UNLEN);
				break;
			case VAL_PASSWORD:
				_tcsncpy (RasDialParams.szPassword, szStart, PWLEN);
				break;
			case VAL_DOMAIN:
			    _tcsncpy (RasDialParams.szDomain, szStart, DNLEN);
				break;
#ifdef UNDER_CE
            case VAL_BAUDRATE:
                if (g_fDevName) 
				{
                    g_fUseDevConfig = SetBaudRate(szStart);
                }
                break;
			case VAL_ATTACH:
				_tcscpy( szAttachCmd, szStart );
				break;
#endif                
			default :
				//LogDebugData(LOG_PASS,
				  //TEXT("Line %d: Unable to copy '%s' (data ='%s')"),
					//	   dwLineNum, szLine, szStart);
				break;
		}
	}

	if (szName[0] == TEXT('\0'))
	{
		//LogDebugData(LOG_ABORT, TEXT("Error: connectoid name required"));
		CloseHandle (hFile);
		return -2;
	}

#ifdef UNDER_CE
    if (FALSE == g_fUseDevConfig) {
        g_lpDevConfig = NULL;
        g_dwDevConfigLen = 0;
    }
//增加GPRS拨号需要的附加设置
//有的SIM卡默认不为CMNET!
////////////////////////////////
	LPBYTE patcmd;//tanzf add
	patcmd=(LPBYTE)g_lpDevConfig + g_lpDevConfig->dwStringOffset+0x16;       //tanzf add
	//wcscpy((unsigned short *)(patcmd),TEXT("+CGDCONT=1,\"IP\",\"CMNET\""));//tanzf add
	wcscpy((unsigned short *)(patcmd), szAttachCmd );// 2011-9-23 modify by zhl
////////////////////////////////
	if (RasSetEntryProperties(NULL, szName, &RasEntry,
							  sizeof(RasEntry), (g_lpDevConfig) ?
							  ((LPBYTE)g_lpDevConfig + g_lpDevConfig->dwStringOffset) : NULL,
							  (g_lpDevConfig) ? g_lpDevConfig->dwStringSize : 0))
#else
	if (RasSetEntryProperties(NULL, szName, &RasEntry, sizeof(RasEntry) + 1000,
	  NULL, 0))
#endif
		//LogDebugData(LOG_ABORT, TEXT("Error %d from \"RasSetEntryProperties\""),
		  //GetLastError());
	//else
		//LogDebugData(LOG_PASS, TEXT("RasEntry '%s' created"), szName);

#ifdef UNDER_CE
    if (g_fUseDevConfig) {
        LocalFree(g_lpDevConfig);
    }
#endif

	RasDialParams.dwSize = sizeof(RASDIALPARAMS);
	_tcscpy (RasDialParams.szEntryName, szName);
	_tcscpy( RasDialParams.szPhoneNumber, _T("*99***1#") );
	_tcscpy( RasDialParams.szUserName, _T("") );
	_tcscpy( RasDialParams.szPassword, _T("") );
	DWORD dwError;
	dwError = RasSetEntryDialParams(NULL, &RasDialParams, FALSE);
	
	CloseHandle (hFile);

	printf( "Set RASEntry OK!\n" );
	return 0;
}





