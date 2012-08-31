// ETA503Set.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
    // TODO: Place code here.
 	// TODO: Place code here.
	char    str[100];
	DWORD	dwSetting;
	int		i1;
	HKEY	hKey;
	LPCTSTR	hSubKeyIRQ1 = _T("Drivers\\BuiltIn\\IRQ1");
	TCHAR   szSubKeyETA503[80];

	DWORD	dwTrueInstall;
	DWORD	dwType = REG_DWORD;
	DWORD	dwBufLen = sizeof(DWORD);
	int     nExSerNum = 4;
	int     nExSerNo = 6;

	printf("ETA503 Driver Configuration\r\n");

	//
	// setup setting value = 0, or 1
	//
	dwSetting = 1;
	i1 = wcslen( lpCmdLine );
	if( i1 > 0 )
	{
		wcstombs( str, lpCmdLine, i1 );
		sscanf(str, "%d", &dwSetting);
		if( dwSetting > 0 )
		{
			dwSetting = 1;
		}
	}

	//
	// if dwSetting == 1, disable IRQ1 function
	//
	if( dwSetting )
	{
		//
		// CS&ZHL JLY-01-2009: open IRQ1 registry & disable IRQ1
		//
		if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, hSubKeyIRQ1, 0, 0, &hKey) != ERROR_SUCCESS)
		{
			RETAILMSG(1, (L"Can not open registry!\r\n"));
			return -1;
		}
		dwTrueInstall = 0;
		dwType = REG_DWORD;
		dwBufLen = sizeof(DWORD);
		RegSetValueEx(hKey, _T("TrueInstall"), 0, dwType, (BYTE*)&dwTrueInstall, dwBufLen);

		RegCloseKey(hKey);		// close registry
	}

	//
	// CS&ZHL JLY-01-2009: open ETA503 registry
	//
	for( i1=0; i1<nExSerNum; i1++ )
	{
		_stprintf( szSubKeyETA503, _T("Drivers\\BuiltIn\\Serial%d"),  i1 + nExSerNo );
		if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, (LPCTSTR)szSubKeyETA503, 0, 0, &hKey) != ERROR_SUCCESS)
		{
			RETAILMSG(1, (L"Can not open registry!\r\n"));
			return -1;
		}

		//
		// CS&ZHL JLY-01-2009: read registry item
		//
		if(RegQueryValueEx(hKey, _T("TrueInstall"), NULL, &dwType, (LPBYTE)&dwTrueInstall, &dwBufLen) != ERROR_SUCCESS)
		{
			RETAILMSG(1, (L"Can not read registry!\r\n"));
			RegCloseKey(hKey);
			return -1;
		}

		//
		// set new value if necessary
		//
		if(dwTrueInstall != dwSetting )
		{
			dwTrueInstall = dwSetting;
			RegSetValueEx(hKey, _T("TrueInstall"), 0, dwType, (BYTE*)&dwTrueInstall, dwBufLen);
			printf("\r\nPlease reboot the system to let the settings active!\r\n");
		}
		RegCloseKey(hKey);		// close registry
	}
	
	return 0;
}



