// AudioSet.cpp : Defines the entry point for the application.
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
	LPCTSTR	hSubKey = _T("Drivers\\BuiltIn\\Audio");
	DWORD	dwTrueInstall;
	DWORD	dwType = REG_DWORD;
	DWORD	dwBufLen = sizeof(DWORD);

	printf("Audio Driver Configuration\r\n");

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
	// CS&ZHL JLY-01-2009: open registry
	//
	if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, hSubKey, 0, 0, &hKey) != ERROR_SUCCESS)
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

    return 0;
}



