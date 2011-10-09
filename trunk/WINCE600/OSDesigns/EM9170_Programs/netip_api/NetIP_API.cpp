#include "stdafx.h"
#include <halether.h>
#include "Ndis.h"
#include <winbase.h>
#include <Iphlpapi.h>

#include "netip_api.h"

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))


void IPAddrToStr (LPTSTR szStr, DWORD IPAddr)
{
	wsprintf(szStr, TEXT("%d.%d.%d.%d"),
			  (IPAddr >> 24) & 0xFF, (IPAddr >> 16) & 0xFF,
			  (IPAddr >> 8) & 0xFF,  IPAddr & 0xFF);
}

BOOL StringToIPAddr(TCHAR *IPAddressString, DWORD *IPAddressValue) 
{
	TCHAR	*pStr = IPAddressString;
	PUCHAR	AddressPtr = (PUCHAR)IPAddressValue;
	int		i;
	int		Value;

	// Parse the four pieces of the address.
	for (i=0; *pStr && (i < 4); i++) 
	{
		Value = 0;
		while (*pStr && TEXT('.') != *pStr) 
		{
			if ((*pStr < TEXT('0')) || (*pStr > TEXT('9'))) 
			{
				return FALSE;
			}
			Value *= 10;
			Value += *pStr - TEXT('0');
			pStr++;
		}
		if (Value > 255) 
		{
			return FALSE;
		}
		AddressPtr[i] = Value;
		if (TEXT('.') == *pStr) 
		{
			pStr++;
		}
	}

	// Did we get all of the pieces?
	if (i != 4) 
	{
		return FALSE;
	}

	*IPAddressValue = net_long(*IPAddressValue);
	
	return TRUE;
}

VOID GetMultiSZAddr (HKEY hKey, LPTSTR szValueName, LPDWORD pAddr1, LPDWORD pAddr2)
{
	TCHAR	szTemp[256];
	LPTSTR	szPtr;
	LONG	hRes;
	DWORD	dwSize, dwType;

	dwSize = sizeof(szTemp);
	hRes = RegQueryValueEx (hKey, szValueName, 0, &dwType, (LPBYTE)szTemp,
							&dwSize); 
	if ((hRes == ERROR_SUCCESS) &&
		((dwType == REG_SZ) || (dwType == REG_MULTI_SZ))) 
	{
		if (pAddr1 && szTemp[0]) 
		{
			StringToIPAddr (szTemp, pAddr1);
		}
		if (pAddr2 && szTemp[0] && (dwType == REG_MULTI_SZ)) 
		{
			szPtr = szTemp;
			while (*szPtr) 
			{
				szPtr++;
			}
			// Move past the null.
			szPtr++;
			if (*szPtr) 
			{
				StringToIPAddr (szPtr, pAddr2);
			}
		}
	} 
	
}

BOOL DoNdisIOControl(DWORD dwCommand, LPVOID pInBuffer,
                                  DWORD cbInBuffer, LPVOID pOutBuffer,
                                  DWORD * pcbOutBuffer)
{
	HANDLE hNdis;
	BOOL fResult = FALSE;

	hNdis = CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                        0, NULL);

	if (INVALID_HANDLE_VALUE != hNdis)
	{
		fResult = DeviceIoControl(hNdis, dwCommand, pInBuffer, cbInBuffer,
                                  pOutBuffer, (pcbOutBuffer ? *pcbOutBuffer : 0),
                                  pcbOutBuffer, NULL);
		CloseHandle(hNdis);
	}

	return fResult;
}


BOOL  GetNetWorkAdaptersName( PNETWORK_ADPTS_NAME pAdptsName )
{
	int					i1, i2;
	PIP_ADAPTER_INFO    pAdapterInfo = NULL, pNextInfo = NULL;
	ULONG               ulSizeAdapterInfo = 0;
	DWORD               dwStatus;
	
	// Find out how big our buffer needs to be to hold the data
	dwStatus = GetAdaptersInfo(pAdapterInfo, &ulSizeAdapterInfo);
	if(dwStatus == ERROR_BUFFER_OVERFLOW) 
	{
		// Allocate a buffer of the appropriate size
		if (!(pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulSizeAdapterInfo))) 
		{
			//printf("\n Insufficient Memory ");
			RETAILMSG( 1, ( TEXT( "\r\n Insufficient Memory " ) ) );
			return FALSE;
		}

		// Obtain the Adapter Info
		dwStatus = GetAdaptersInfo(pAdapterInfo, &ulSizeAdapterInfo);
	}
		
	pNextInfo = pAdapterInfo;
	
	for( i2=0; i2<MAXNUMOFADPT; i2++ )
	{
		if( pNextInfo==NULL )
			break;
		i1 = strlen(pNextInfo->AdapterName);
		mbstowcs( pAdptsName->szAdapterName[i2], pNextInfo->AdapterName, i1 );
		pAdptsName->szAdapterName[i2][i1] = 0;
		pNextInfo = pNextInfo->Next;
	}

	pAdptsName->NumOfAdapters = (DWORD)i2;

	if( pAdapterInfo )
		free( pAdapterInfo );

	return TRUE;
}


//
// Get the current registry data.
//
BOOL  GetNetWorkAdapterInfo( LPTSTR szAdapterName, PNETWORK_ADPT_INFO pAdapterInfo )
{
	BOOL				fRetVal = TRUE;
	TCHAR				szTemp[256];
	HKEY				hKey;
	LONG				hRes;
	DWORD				dwDisp = 0;
	DWORD				dwSize = 0;
	DWORD				dwType = 0;

    if (szAdapterName == NULL)
        return FALSE;

	// Initialize the adapter Info.
	memset ((char *)pAdapterInfo, 0, sizeof(NETWORK_ADPT_INFO));

	pAdapterInfo->szAdapterName = szAdapterName;

	// Get the current registry data.
	_tcscpy (szTemp, _T("Comm\\"));
	_tcscat (szTemp, szAdapterName);
	hKey = NULL;
	hRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, 0, &hKey);
	
	RETAILMSG( 1, (TEXT("\r\nRegOpenKeyEx %s %d" ), szTemp, hRes  ) );	

	if(ERROR_SUCCESS != hRes) 
		return FALSE;

	pAdapterInfo->szDisplayName[0] = TEXT('\0');
	dwSize = sizeof(pAdapterInfo->szDisplayName);
	RegQueryValueEx (hKey, TEXT("DisplayName"), NULL, NULL,
					 (LPBYTE)pAdapterInfo->szDisplayName,
					 &dwSize);
	RegCloseKey (hKey);
	
	
	// Get the current registry data.
	_tcscpy (szTemp, _T("Comm\\"));
	_tcscat (szTemp, szAdapterName);
	_tcscat (szTemp, _T("\\Parms\\TcpIp"));

	hKey = NULL;
	hRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, 0, &hKey);
	if (ERROR_SUCCESS == hRes) 
	{
		// Read the saved data in.
		dwSize = sizeof(DWORD);
		hRes = RegQueryValueEx (hKey, TEXT("EnableDHCP"), 0, &dwType,
								(LPBYTE)&pAdapterInfo->fUseDHCP,
								&dwSize);
		if ((ERROR_SUCCESS == hRes) && (!pAdapterInfo->fUseDHCP)) 
		{
			GetMultiSZAddr (hKey, TEXT("IpAddress"),
							&pAdapterInfo->IPAddr, NULL);
			GetMultiSZAddr (hKey, TEXT("Subnetmask"),
							&pAdapterInfo->SubnetMask, NULL);
			GetMultiSZAddr (hKey, TEXT("DefaultGateway"),
							&pAdapterInfo->Gateway, NULL);
		}
		GetMultiSZAddr (hKey, TEXT("DNS"), &pAdapterInfo->DNSAddr,
						&pAdapterInfo->DNSAltAddr);
		GetMultiSZAddr (hKey, TEXT("WINS"), &pAdapterInfo->WINSAddr,
						&pAdapterInfo->WINSAltAddr);			
	} 
	else
	{
		fRetVal = FALSE;
	}

	if (hKey)
		RegCloseKey (hKey);

	return fRetVal;
}


BOOL  SetNetWorkAdapterInfo( LPTSTR szAdapterName, PNETWORK_ADPT_INFO pAdapterInfo )
{
	BOOL				fRetVal = FALSE;
	TCHAR				szTemp[256];
	HKEY				hKey;
	LONG				hRes;
	NETWORK_ADPT_INFO	AdptInfo;
	DWORD				dwDisp = 0;
	DWORD				dwSize = 0;
	DWORD				dwType = 0;

	DWORD				Len;

    if (szAdapterName == NULL)
        return FALSE;

	// Initialize the adapter Info.
	memset ((char *)&AdptInfo, 0, sizeof(AdptInfo));

	// set new paramters
	memcpy( &AdptInfo, pAdapterInfo, sizeof(AdptInfo));
	AdptInfo.szAdapterName = szAdapterName;

	/*
	// Get the current registry data.
	bRes = GetNetWorkAdapterInfo( szAdapterName, &AdptInfo );

	if( bRes==FALSE )
			return FALSE;

	// Save copy of Adptinfo to detect changes
	memcpy(&OldAdptInfo, &AdptInfo, sizeof(AdptInfo));

	// set new paramters
	memcpy( &AdptInfo, pAdapterInfo, sizeof(AdptInfo));

	//
	// CS&ZHL:2008-06-27 compare oldAdpInfo with adptInfo, if result is same: return TRUE
	//
	if( memcmp( &OldAdptInfo, &AdptInfo, sizeof( AdptInfo ) )==0 )
	{
		RETAILMSG( 1, (TEXT("\r\n IP Parameters aren't changed, need not set AdapterIPProperties\r\n" ) ));
		return TRUE;
	}
	*/

	hKey = NULL;
	if (hKey == NULL) 
	{
		// Create it.
		_tcscpy (szTemp, TEXT("Comm\\"));
		_tcscat (szTemp, szAdapterName);
		_tcscat (szTemp, TEXT("\\Parms\\TcpIp"));
		hRes = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, NULL,
							   REG_OPTION_NON_VOLATILE, 0, NULL,
							   &hKey, &dwDisp);
		if (hRes != ERROR_SUCCESS) 
		{
		}
	}

	// Just in case the create failed.
	if (hKey != NULL) 
	{
		fRetVal = TRUE;

		// I'd like 1 to mean TRUE.
		if( AdptInfo.fUseDHCP ) 
		{
			AdptInfo.fUseDHCP = 1;
			AdptInfo.IPAddr = 0;
			AdptInfo.SubnetMask = 0;
			AdptInfo.Gateway = 0;
			AdptInfo.DNSAddr = 0;
		}
		RegSetValueEx (hKey, TEXT("EnableDHCP"), 0, REG_DWORD,
					   (const BYTE *)&(AdptInfo.fUseDHCP), sizeof(DWORD));

		if( !AdptInfo.fUseDHCP )
		{
			IPAddrToStr (szTemp, AdptInfo.IPAddr);
			Len = _tcslen(szTemp)+1;
			szTemp[Len++] = TEXT('\0');
			RegSetValueEx (hKey, TEXT("IpAddress"), 0, REG_MULTI_SZ,
						   (const BYTE *)szTemp, sizeof(TCHAR)*Len);

			RETAILMSG( 1, (TEXT("\r\n IPAddr: %s" ), ( BYTE *)szTemp ));
			
			IPAddrToStr (szTemp, AdptInfo.SubnetMask);
			Len = _tcslen(szTemp)+1;
			szTemp[Len++] = TEXT('\0');
			RegSetValueEx (hKey, TEXT("Subnetmask"), 0, REG_MULTI_SZ,
						   (const BYTE *)szTemp, sizeof(TCHAR)*Len);
			
			RETAILMSG( 1, (TEXT("\r\n SunnetMask: %s" ), ( BYTE *)szTemp ));

			//if( AdptInfo.Gateway!=0 )
			{
				IPAddrToStr (szTemp, AdptInfo.Gateway);
				Len = _tcslen(szTemp)+1;
				szTemp[Len++] = TEXT('\0');
				RegSetValueEx (hKey, TEXT("DefaultGateway"), 0, REG_MULTI_SZ,
						   (const BYTE *)szTemp, sizeof(TCHAR)*Len);
			
				RETAILMSG( 1, (TEXT("\r\n Gateway: %s\r\n" ), ( BYTE *)szTemp ));

			}

			Len = 0;
			if (AdptInfo.DNSAddr) 
			{
				IPAddrToStr (szTemp, AdptInfo.DNSAddr);
				Len = _tcslen(szTemp)+1;
				if (AdptInfo.DNSAltAddr) 
				{
					IPAddrToStr (szTemp+Len, AdptInfo.DNSAltAddr);
					Len += _tcslen(szTemp+Len)+1; // allow for NULL character added by IPAddrToStr
				}
				szTemp[Len++] = TEXT('\0');
			} 
			else if (AdptInfo.DNSAltAddr) 
			{
				IPAddrToStr (szTemp, AdptInfo.DNSAltAddr);
				Len = _tcslen(szTemp)+1;
				szTemp[Len++] = TEXT('\0');
			}
			if (Len) 
			{
				RegSetValueEx (hKey, TEXT("DNS"), 0, REG_MULTI_SZ,
							   (const BYTE *)szTemp, sizeof(TCHAR)*Len);
				RETAILMSG( 1, (TEXT("DNS: %s\r\n" ), (BYTE *)szTemp ) );	
			} 
			else 
			{
				RegSetValueEx (hKey, TEXT("DNS"), 0, REG_MULTI_SZ,
							   (const BYTE *)TEXT(""), sizeof(TCHAR));
			}
			
			
			Len = 0;
			if (AdptInfo.WINSAddr) 
			{
				IPAddrToStr (szTemp, AdptInfo.WINSAddr);
				Len = _tcslen(szTemp)+1;
				if (AdptInfo.WINSAltAddr) 
				{
					IPAddrToStr (szTemp+Len, AdptInfo.WINSAltAddr);
					Len += _tcslen(szTemp+Len)+1; // allow for NULL character added by IPAddrToStr
				}
				szTemp[Len++] = TEXT('\0');
			} 
			else if (AdptInfo.WINSAltAddr) 
			{
				IPAddrToStr (szTemp, AdptInfo.WINSAltAddr);
				Len = _tcslen(szTemp)+1;
				szTemp[Len++] = TEXT('\0');
			}
			if (Len) 
			{
				RegSetValueEx (hKey, TEXT("WINS"), 0, REG_MULTI_SZ,
							   (const BYTE *)szTemp, sizeof(TCHAR)*Len);
			} 
			else 
			{
				RegSetValueEx (hKey, TEXT("WINS"), 0, REG_MULTI_SZ,
							   (const BYTE *)TEXT(""), sizeof(TCHAR));
			}	
		}
	}
	
	
	if (hKey)
		RegCloseKey (hKey);

    if( fRetVal )
	{
		TCHAR multiSz[257];
     

		// because we building a multisz we need to leave room for the double null at the end so copy one less than the buffer.
		StringCchCopy(multiSz, (sizeof(multiSz) / sizeof(TCHAR))-1, szAdapterName);
		multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

		//hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		DoNdisIOControl(IOCTL_NDIS_REBIND_ADAPTER, multiSz,
						  (_tcslen(multiSz)+2) * sizeof(TCHAR),
						  NULL, NULL);
    }


	return fRetVal;
}
