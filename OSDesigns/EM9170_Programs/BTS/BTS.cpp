// BTS.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <winioctl.h>
#include "bsp_drivers.h"

extern "C" __declspec(dllimport) BOOL KernelIoControl(DWORD   dwIoControlCode,
													  LPVOID  pInBuffer,
													  DWORD	  InSize,
													  LPVOID  pOutBuffer,
													  DWORD   OutSize,
													  LPDWORD pReturnBytes);

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
    // TODO: Place code here.
	char	StampString[128];
	DWORD	dwOutBufSize;
	DWORD	dwReturnBytes;
	BOOL	bRet;

	dwOutBufSize = 128;
	bRet = KernelIoControl(IOCTL_HAL_TIMESTAMP_READ, 
						NULL,    
						0, 
						(LPVOID)StampString,
						dwOutBufSize, 
						&dwReturnBytes);


	if(!bRet)
	{
		printf("Kernel Built Stamp is unavailable\r\n");
	}
	else
	{
		StampString[dwReturnBytes] = '\0';
		printf("%s\r\n", StampString);
	}

    return 0;
}



