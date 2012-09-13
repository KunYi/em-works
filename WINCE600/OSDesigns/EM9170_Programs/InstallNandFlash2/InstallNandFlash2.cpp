// InstallNandFlash2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
    // TODO: Place code here.
	HANDLE hDevHandle;
	TCHAR pszDriverPath[] = TEXT("\\Drivers\\BlockDevice\\NAND_Flash2");

	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	//Sleep(5000);
	RETAILMSG(1, (TEXT("Install NandFlash2...\r\n")));
	hDevHandle = ActivateDevice(pszDriverPath, 0);
	if(hDevHandle == NULL)
	{
		RETAILMSG(1, (TEXT("ActivateDevice failed, error = %d.\r\n"), GetLastError()));
	}
	else
	{
		RETAILMSG(1, (TEXT("ActivateDevice success.\r\n")));
		//CS&ZHL AUG-11-2011: registry -> "Launch129"="InstallNandFlash2.exe"
		SignalStarted( 129 );
	}

	return 0;
}



