// BTS.cpp : Defines the entry point for the console application.
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

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
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
