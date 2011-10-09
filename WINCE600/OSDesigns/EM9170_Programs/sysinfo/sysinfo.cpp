// sysinfo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

TCHAR	szWorkPathName[40];

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
 	// TODO: Place code here.
	int					i1;
	int					nCount = 10;		//10s
	DWORD			dwStartTick, dwIdleSt;
	DWORD			dwStopTick, dwIdleEd;
	DWORD			PercentIdle;
	BOOL				bResult;
	char				str[100];
	unsigned long	ul1, ul2;

    ULARGE_INTEGER  uCaller, uTotalSize, uFreeSize;
	MEMORYSTATUS    mMemory;

	printf("EM91670 System Info V1.0\n");
	printf("   Emtronix(c) 2011\n");

	_tcscpy( szWorkPathName, _T("NandFlash") );

	i1 = wcslen( lpCmdLine );
	if( i1 > 0 )
	{
		wcstombs( str, lpCmdLine, i1 );
		sscanf(str, "%d", &nCount);
		if( nCount > 3600 )
		{
			nCount = 3600;
		}
	}

	while(nCount)
	{
		nCount -= 2;
		if( nCount < 0 )
			nCount = 0;
		dwStartTick = GetTickCount();
		dwIdleSt = GetIdleTime();
		Sleep( 2000 );
		dwStopTick = GetTickCount();
		dwIdleEd = GetIdleTime();
		PercentIdle = ((100*(dwIdleEd - dwIdleSt)) / (dwStopTick - dwStartTick));

		bResult = GetDiskFreeSpaceEx( szWorkPathName, &uCaller, &uTotalSize,  &uFreeSize);
		ul1 = (unsigned long)uTotalSize.QuadPart;
		ul2 = (unsigned long)uFreeSize.QuadPart;
		mMemory.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus( &mMemory );
		i1 = wcslen( szWorkPathName );
		wcstombs( str, szWorkPathName, i1 );
		str[i1] = 0;
		printf( "\r\n%s=%ld::%ld TotalPhys=0x%x AvailPhys=0x%x CPU %d%%\r\n", 
			     str, ul1, ul2, mMemory.dwTotalPhys, mMemory.dwAvailPhys, (100 - PercentIdle) );
	}

	return 0;
}


