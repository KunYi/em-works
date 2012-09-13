// wstartup.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include <pwindbas.h>
#include <winioctl.h>
#include "pkfuncs.h"
#include "netip_api.h"
#include "bsp_drivers.h"

#ifdef EM9170
#define  DEVICE_NAME	_T("EM9170")
#endif

#ifdef EM9280
#define  DEVICE_NAME	_T("EM9280")
#endif

#ifdef EM9283
#define  DEVICE_NAME	_T("EM9283")
#endif

TCHAR	HostIPStr[20], EXE_NAME[80], EXE_PAR[80];
int			HostPort;
int			StorePercent;
int			UpdateFlag=0;

//
// autotest function for manufacture
//  return = TRUE: autotest launched
//         = FALSE: no autotest
//
BOOL AutoEmtronixTest( );

void  SysReboot( )
{
	if( !KernelIoControl( IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL ) )
	{
		RETAILMSG(1, (TEXT("IOCTL_HAL_REBOOT Fail!\r\n" ) ));
	}
}

extern void IPAddrToStr (LPTSTR szStr, DWORD IPAddr);

// return = TRUE   when the DEBUG jumper is ON
//          = FALSE  when the DEBUG jumper is OFF
BOOL Debug_State( )
{	
	DWORD							dwValue=0;
	DWORD							dwOutBufSize=sizeof(DWORD);
	DWORD							dwReturnBytes=0;

	KernelIoControl(IOCTL_HAL_BOARD_STATE_READ,  NULL, 0, (LPVOID)&dwValue, dwOutBufSize, &dwReturnBytes);
	
	if( dwValue & 0x00000080 )
	{
		return	  TRUE;					// in debug mode
	}

	return FALSE;						// in run mode
	//return TRUE;						// only for test APR-09-2012
}


BOOL GetCFGValue( char* CFGBuffer, int Buflen, char *pKeyName, char *pItemName, TCHAR* pStr )
{
	int   i1, i2, len1, len2;

	len1 = strlen( pKeyName );
	len2 = strlen( pItemName );
   
	for( i1=0; i1<Buflen; i1++ )
	{
		if( strncmp( &CFGBuffer[i1], pKeyName, len1 ) == 0 )
		{
			i1 += len1;
			break;
		}
	}
	if( i1==Buflen )    return  FALSE;
	for( ; i1<Buflen; i1++ )
	{
		if( strncmp( &CFGBuffer[i1], pItemName, len2 )== 0 )
		{
			i1+= len2;
			break;
		}
	}
	if( i1==Buflen )    return FALSE;
	int Flg=0;
	for( i2=0; i1<Buflen; i1++)
	{
		if( (CFGBuffer[i1]==0x22) &&(Flg==0) )
		{
			Flg = 1;
			continue;
		}
		if( (CFGBuffer[i1]==0x22)&&(Flg>0) ) 
			break;
		pStr[i2] = CFGBuffer[i1];
		i2++;
	}
	pStr[i2] = 0;	
	return TRUE;	
}

BOOL GetAdapterIPFromFile( PNETWORK_ADPT_INFO pAdptInfo, LPTSTR FileName )    //, LPTSTR szAdapterName )
{
	BOOL			bResult;
	TCHAR		ValueStr[MAX_PATH];
	HANDLE		fHandle;
    CHAR			Buffer[2000], str[100];
	DWORD		nBytes;
	int				i1;

	RETAILMSG( 1, (TEXT("\r\nFileName:%s" ), FileName  ) );	
		
	fHandle = CreateFile( FileName, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    RETAILMSG( 1, (TEXT("\r\nCreatFile %p" ), fHandle  ) );	
	if( fHandle==INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}
	bResult = ReadFile( fHandle, Buffer, 1000, &nBytes, NULL );
    RETAILMSG( 1, (TEXT("\r\nFile bResult=%d nBytes=%d" ), bResult, nBytes  ) );	
	if( bResult )
	{
		if( GetCFGValue( Buffer, (int)nBytes, "[LOCAL_MACHINE]", "DHCP=", ValueStr ) )
		{
			if( wcscmp( ValueStr, TEXT("EnableDHCP") )==0 )
			{
				pAdptInfo->fUseDHCP = 1;
				RETAILMSG( 1, (TEXT("\r\nDHCP Enable" ) ) );	
			}
			else
			{
				pAdptInfo->fUseDHCP = 0;
				RETAILMSG( 1, (TEXT("\r\nDHCP Disable" ) ) );	
			}
		}
		else
		{
			pAdptInfo->fUseDHCP = 0;
			RETAILMSG( 1, (TEXT("\r\nDHCP Disable" ) ) );	
		}

		if( GetCFGValue( Buffer, (int)nBytes, "[LOCAL_MACHINE]", "DefaultGateway=", ValueStr ) )
			StringToIPAddr( ValueStr, &pAdptInfo->Gateway );
		if( GetCFGValue( Buffer, (int)nBytes, "[LOCAL_MACHINE]", "IPAddress=", ValueStr ) )
			StringToIPAddr( ValueStr, &pAdptInfo->IPAddr );
		if( GetCFGValue( Buffer, (int)nBytes, "[LOCAL_MACHINE]", "SubnetMask=", ValueStr ) )
			StringToIPAddr( ValueStr, &pAdptInfo->SubnetMask );

		if( GetCFGValue( Buffer, (int)nBytes, "[LOCAL_MACHINE]", "DNS=", ValueStr ) )
			StringToIPAddr( ValueStr, &pAdptInfo->DNSAddr );

		if( !GetCFGValue( Buffer, (int)nBytes, "[HOST_MACHINE]", "IPAddress=", HostIPStr ) )
		{
			_tcscpy( HostIPStr, TEXT("192.168.201.131") );
		}

		HostPort = 9300;
		if( GetCFGValue( Buffer, (int)nBytes, "[HOST_MACHINE]", "port=", ValueStr ) )
		{
			i1 = wcslen( ValueStr);
			wcstombs( str, ValueStr, i1 );
			str[i1] = 0;
			HostPort = atoi( str );
		}

		// add 2007/4/12 for SetSystemMemerySize: from EM9XXX
		StorePercent = 100;
		if( GetCFGValue( Buffer, (int)nBytes, "[SYSTEM]", "Store=", ValueStr ) )
		{
			i1 = wcslen( ValueStr);
			wcstombs( str, ValueStr, i1 );
			str[i1] = 0;
			StorePercent = atoi( str );
		}

	}
	CloseHandle( fHandle );

	return TRUE;
}

BOOL GetEXENameFromFile( LPTSTR FileName )    //, LPTSTR szAdapterName )
{
	BOOL			fRetVal = FALSE, bResult;
	TCHAR		ValueStr[MAX_PATH];
	HANDLE		fHandle;
    CHAR			Buffer[2000], str[100];
	DWORD		nBytes;
	int				i1;
	
	fHandle = CreateFile( FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
	if( fHandle!=INVALID_HANDLE_VALUE )
	{
		fRetVal = TRUE;	
		bResult = ReadFile( fHandle, Buffer, 1000, &nBytes, NULL );
	    RETAILMSG( 1, (TEXT("\r\nFile bResult=%d nBytes=%d" ), bResult, nBytes  ) );	
		if( bResult )
		{
			if( !GetCFGValue( Buffer, (int)nBytes, "[USER_EXE]", "Name=", EXE_NAME ) )
			{
				_tcscpy( EXE_NAME, TEXT("") );
			}
			if( !GetCFGValue( Buffer, (int)nBytes, "[USER_EXE]", "Parameters=", EXE_PAR ) )
			{
				_tcscpy( EXE_PAR, TEXT("") );
			}

			// add 2009/2/10 for check if start update program
			UpdateFlag = 0;
			if( GetCFGValue( Buffer, (int)nBytes, "[USER_EXE]", "Update=", ValueStr ) )
			{
				if( wcscmp( ValueStr, TEXT("ON") )==0 )
				{
					UpdateFlag = 1;
				}
			}

			// add 2007/4/12 for SetSystemMemerySize: from EM9XXX
			StorePercent = 100;
			if( GetCFGValue( Buffer, (int)nBytes, "[SYSTEM]", "Store=", ValueStr ) )
			{
				i1 = wcslen( ValueStr);
				wcstombs( str, ValueStr, i1 );
				str[i1] = 0;
				StorePercent = atoi( str );
			}

		}
		CloseHandle( fHandle );
	}

	return fRetVal;
}


BOOL  SetAdapterIPProperties ( LPTSTR szAdapterName, LPTSTR FileName )
{
	BOOL								fRetVal = FALSE, bRes;
	TCHAR							szTemp[256];
	NETWORK_ADPT_INFO		AdptInfo, OldAdptInfo;
	DWORD							Len;

    if (szAdapterName == NULL)
        return FALSE;

	// Initialize the adapter Info.
	memset ((char *)&AdptInfo, 0, sizeof(AdptInfo));

	bRes = GetNetWorkAdapterInfo( szAdapterName, &AdptInfo );

	IPAddrToStr (szTemp, AdptInfo.IPAddr);
	Len = _tcslen(szTemp)+1;
	szTemp[Len++] = TEXT('\0');
	RETAILMSG( 1, (TEXT("\r\n IPAddr: %s" ), ( BYTE *)szTemp ));
	
	IPAddrToStr (szTemp, AdptInfo.SubnetMask);
	Len = _tcslen(szTemp)+1;
	szTemp[Len++] = TEXT('\0');
	RETAILMSG( 1, (TEXT("    SunnetMask: %s" ), ( BYTE *)szTemp ));

	IPAddrToStr (szTemp, AdptInfo.Gateway);
	Len = _tcslen(szTemp)+1;
	szTemp[Len++] = TEXT('\0');
	RETAILMSG( 1, (TEXT("    Gateway: %s\r\n" ), ( BYTE *)szTemp ));

	// Save copy of Adptinfo to detect changes
	memcpy(&OldAdptInfo, &AdptInfo, sizeof (AdptInfo));

	bRes = GetAdapterIPFromFile( &AdptInfo, FileName );

	//
	// CS&ZHL:2008-06-27 compare oldAdpInfo with adptInfo, if result is same: return FALSE
	//
	if( memcmp( &OldAdptInfo, &AdptInfo, sizeof( AdptInfo ) )==0 )
	{
		RETAILMSG( 1, (TEXT("\r\n IP Parameters aren't changed, need not set AdapterIPProperties\r\n" ) ));
		return FALSE;
	}

	if ( bRes ) 
	{
		fRetVal = SetNetWorkAdapterInfo( szAdapterName, &AdptInfo );
	} 
	
	return fRetVal;
}


BOOL SetMemoryDivision( int Percent )
{
 	BOOL  nDone;
	DWORD dwStorePages, dwRamPages, dwPageSize, nResult;

	if( (Percent>90)||(Percent<5) )		
		return FALSE;
	nDone = GetSystemMemoryDivision( &dwStorePages, &dwRamPages, &dwPageSize ); 
	if( !nDone )
	{
		MessageBox ( NULL, TEXT("GetSystemMemoryDivision Call Fail"), NULL, MB_OK ); 
		return FALSE; 
	}

	// calculate new storepage
	dwStorePages = ((dwStorePages+dwRamPages)*Percent)/100;
	nResult = SetSystemMemoryDivision( dwStorePages );
	if( nResult != SYSMEM_CHANGED )
	{
		MessageBox ( NULL, TEXT("SetSystemMemoryDivision Call Fail"), NULL, MB_OK ); 
		return FALSE; 
	}

	return TRUE;
}

BOOL FileIsExist( LPTSTR lpFileName )
{
	HANDLE  hFile;

	hFile = CreateFile( lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile==INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}
	CloseHandle( hFile );

	return TRUE;
}

//
// CS&ZHL 2011-04-06: add autotest function for manufacture
//       return=TRUE: autotest launched
//            =FALSE: no autotest
//
BOOL AutoEmtronixTest( )
{
	BOOL			bRet=FALSE, b;
	HKEY			hKey;
	DWORD		dwCount;
	DWORD		dwType = REG_DWORD;
	DWORD		dwBufLen = sizeof(DWORD);
	LPCTSTR	hSubKey = _T("Emtronix");
	TCHAR       szFileName[80];

	PROCESS_INFORMATION procInfo;

	//
	// CS&ZHL Sep-08-2009: open registry
	//
	if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, hSubKey, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		RETAILMSG(1, (L"Can not open registry!\r\n"));
		return bRet;
	}

	//
	// CS&ZHL Sep-08-2009: read registry item
	//
	if(RegQueryValueEx(hKey, _T("Count"), NULL, &dwType, (LPBYTE)&dwCount, &dwBufLen) != ERROR_SUCCESS)
	{
		RETAILMSG(1, (L"Can not read registry!\r\n"));
		RegCloseKey(hKey);
		return bRet;
	}

	if( dwCount )
	{
		RegCloseKey(hKey);
		return bRet;
	}

	_tcscpy( szFileName, TEXT( "\\USBDisk\\autotest.txt" ) );
		
    if( !FileIsExist( szFileName )  )
	{
		RETAILMSG(1, (L"%s file isnot exist!\r\n", szFileName));
		RegCloseKey(hKey);
		return bRet;
	}
	
	if( !GetEXENameFromFile( szFileName ) )
	{
		RETAILMSG(1, (L"%s file error!\r\n", szFileName));
		RegCloseKey(hKey);
		return bRet;
	
	}

	b = CreateProcess
	( 
		EXE_NAME,				  // LPCWSTR lpszImageName, 
		EXE_PAR,                  // LPCWSTR lpszCmdLine, 
		NULL,                     // LPSECURITY_ATTRIBUTES lpsaProcess, 
		NULL,                     // LPSECURITY_ATTRIBUTES lpsaThread, 
		FALSE,                    // BOOL fInheritHandles, 
		0,						  // DWORD fdwCreate, 
		NULL,                     // LPVOID lpvEnvironment, 
		NULL,                     // LPWSTR lpszCurDir, 
		NULL,                     // LPSTARTUPINFOW lpsiStartInfo, 
		&procInfo                 // LPPROCESS_INFORMATION lppiProcInfo
	); 

	if( b )
	{
		CloseHandle(procInfo.hThread);		
		CloseHandle(procInfo.hProcess);
		bRet = TRUE;
		dwCount = 1;
		RegSetValueEx(hKey, _T("Count"), 0, dwType, (BYTE*)&dwCount, dwBufLen);
	}
	else
	{
		RETAILMSG(1, (L"CreateProcess fail!\r\n"));
	}
	RegCloseKey(hKey);

	return bRet;
}


BOOL SetCountOfStartFail( DWORD dwCount )
{
	HKEY			hKey;
	DWORD		dwType = REG_DWORD;
	DWORD		dwBufLen = sizeof(DWORD);
	LPCTSTR	hSubKey = _T("Emtronix");

	// open registry
	if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, hSubKey, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		RETAILMSG(1, (L"Can not open registry!\r\n"));
		return FALSE;
	}

	RegSetValueEx(hKey, _T("CountOfStartFail"), 0, dwType, (BYTE*)&dwCount, dwBufLen);
	RegCloseKey(hKey);

	return TRUE;
}

DWORD GetCountOfStartFail( )
{
	HKEY			hKey;
	DWORD     dwCount;
	DWORD		dwType = REG_DWORD;
	DWORD		dwBufLen = sizeof(DWORD);
	LPCTSTR	hSubKey = _T("Emtronix");

	// open registry
	if(RegOpenKeyEx( HKEY_LOCAL_MACHINE, hSubKey, 0, 0, &hKey) != ERROR_SUCCESS)
	{
		RETAILMSG(1, (L"Can not open registry!\r\n"));
		return (DWORD)-1;
	}

	// read registry item
	if(RegQueryValueEx(hKey, _T("CountOfStartFail"), NULL, &dwType, (LPBYTE)&dwCount, &dwBufLen) != ERROR_SUCCESS)
	{
		RETAILMSG(1, (L"Can not read registry!\r\n"));
		RegCloseKey(hKey);
		return (DWORD)-1;
	}

	RegCloseKey(hKey);

	return dwCount;
}

// CS&ZHL FEB-23-2012: routines for supporting screen rotation
extern BOOL ScreenRotate( int NewAngleDegrees );
extern int GetRotateModeFromReg(VOID);

#define WAITNUM		8

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
    // TODO: Place code here.
	int								i1;
	BOOL							b, bResult;
	TCHAR							OldFileName[MAX_PATH], NewFileName[MAX_PATH];
	TCHAR							cmdline[MAX_PATH];
	ULONG							uIndex = 0;

	char							StampString[128];
	TCHAR							szStampString[128];
	DWORD							dwOutBufSize;
	DWORD							dwReturnBytes;
	DWORD							dwCountOfStartFail;
	//IMX_CPU_INFO					cpuInfo;

	PROCESS_INFORMATION	procInfo;
	NETWORK_ADPTS_NAME	AdaptersName;
	
	//
	// CS&ZHL FEB-23-2012: do screen rotation if required
	//
	i1 = GetRotateModeFromReg( );
    ScreenRotate( i1 );

	// step0: get CPU Info
	//dwOutBufSize = sizeof(cpuInfo);
	//bResult = KernelIoControl(IOCTL_HAL_CPU_INFO_READ, 
	//								NULL,    
	//								0, 
	//								(LPVOID)&cpuInfo,
	//								dwOutBufSize, 
	//								&dwReturnBytes);

	//if( bResult )
	//{
	//	for( i1=0; i1<cpuInfo.dwStringLen; i1++ )
	//	{
	//		szStampString[i1] = cpuInfo.FSLString[i1];
	//	}
	//	RETAILMSG(1,(TEXT("\r\n%s 0x%08x\r\n"), szStampString, cpuInfo.dwChipID ));
	//}


	// step1: get time stamp
	dwOutBufSize = 128;
	bResult = KernelIoControl(IOCTL_HAL_TIMESTAMP_READ, 
									NULL,    
									0, 
									(LPVOID)StampString,
									dwOutBufSize, 
									&dwReturnBytes);

	for( i1=0; i1<strlen(StampString); i1++ )
	{
		szStampString[i1] = StampString[i1];
	}
    szStampString[i1] = 0;
	if( bResult )
	{
		RETAILMSG(1,(TEXT("\r\n%s %s\r\nAdaptation performed by Emtronix (c)\r\n"), DEVICE_NAME, szStampString));
	}

	// step2: Get Network Adapter Name
    bResult = GetNetWorkAdaptersName( &AdaptersName );

	if( bResult )
		RETAILMSG( 1, (TEXT("\r\n AdapterName: %s" ), AdaptersName.szAdapterName[0]  ) );

	//_tcscpy( AdaptersName.szAdapterName[0], _T("FEC1") );

	bResult = Debug_State( );

	if( bResult )
	{
		RETAILMSG( 1, (TEXT("\r\n %s Debug Mode \r\n" ), DEVICE_NAME ) );
		// in debug state
		_tcscpy( OldFileName, TEXT( "\\USBDisk\\userinfo.txt") );
		for( i1=0; i1<WAITNUM; i1++ )
		{
			Sleep( 1000 );
			if( FileIsExist( OldFileName ) )
			{
				break;
			}
		}
		if( i1 < WAITNUM )
		{
			//
			// CS&ZHL Aug-25-2011: copy userinfo.txt to sysflash
			//
			_tcscpy( NewFileName, TEXT( "\\SysFlash\\userinfo.txt") );
			bResult = CopyFile( OldFileName, NewFileName, FALSE );
			RETAILMSG( 1, (TEXT("\r\n CopyFile: %d" ), bResult  ) );

			_tcscpy( NewFileName, TEXT( "\\NandFlash\\userinfo.txt") );
			bResult = CopyFile( OldFileName, NewFileName, FALSE );
		}

		_tcscpy( NewFileName, TEXT( "\\SysFlash\\userinfo.txt") );
		
#ifndef EM9283
		bResult = SetAdapterIPProperties ( AdaptersName.szAdapterName[0], NewFileName );
#endif

		RETAILMSG( 1, (TEXT("SetMemoryDivision : %d\r\n" ), StorePercent  ) );
		bResult = SetMemoryDivision( StorePercent );
		//
		// CS&ZHL 2011-04-13: add autotest function for manufacture
		//
		bResult = AutoEmtronixTest( );
		if( bResult )
		{
			return 0;
		}
	

		// for debug 
		/* CS&ZHL Sep-09-2011: 
		_stprintf( cmdline, _T("/s /t:tcpipc.dll /q /d:%s:%d"), HostIPStr, HostPort );
		// Create a process and run AutoRun.exe
		// CS&ZHL AUG-11-2011: 
		//[HKEY_LOCAL_MACHINE\init]
		//	"Launch130"="wstartup.exe"
		//	"Depend130"=hex:14,00,1e,00,81,00
		//
		SignalStarted( 130 );
		for(; ;)
		{
			memset(&procInfo, 0, sizeof(PROCESS_INFORMATION));
			b = CreateProcess
			( 
				_T("cemgrc.exe"),         // LPCWSTR lpszImageName, 
				cmdline,                  // LPCWSTR lpszCmdLine, 
				NULL,                     // LPSECURITY_ATTRIBUTES lpsaProcess, 
				NULL,                     // LPSECURITY_ATTRIBUTES lpsaThread, 
				FALSE,                    // BOOL fInheritHandles, 
				0,						  // DWORD fdwCreate, 
				NULL,                     // LPVOID lpvEnvironment, 
				NULL,                     // LPWSTR lpszCurDir, 
				NULL,                     // LPSTARTUPINFOW lpsiStartInfo, 
				&procInfo                 // LPPROCESS_INFORMATION lppiProcInfo
			); 

			if(!b)
				continue;

			CloseHandle(procInfo.hThread);
			WaitForSingleObject(procInfo.hProcess, INFINITE);
			CloseHandle(procInfo.hProcess);
		}
		*/
	}
	else
	{
		//
		//[HKEY_LOCAL_MACHINE\init]
		//	"Launch130"="wstartup.exe"
		//	"Depend130"=hex:14,00,1e,00,81,00
		//
		//SignalStarted( 130 );
		RETAILMSG( 1, (TEXT("\r\n %s Run Mode\r\n" ), DEVICE_NAME ) );

		dwCountOfStartFail = GetCountOfStartFail( );

		// in normal running state
		_tcscpy( NewFileName, TEXT( "\\SysFlash\\userinfo.txt") );
		if( !FileIsExist( NewFileName ) )
		{
			//
			// CS&ZHL AUG-25-2011: wait "\NanFlash" ready
			// 
			_tcscpy( NewFileName, TEXT( "\\NandFlash\\userinfo.txt") );
			for( i1=0; i1<WAITNUM; i1++ )
			{
				Sleep( 1000 );
				if( FileIsExist( NewFileName ) )
				{
					break;
				}
			}
			if( i1==WAITNUM )
			{
				return -1;
			}
		}

		// CS&ZHL 2011-06-29
		bResult = GetEXENameFromFile( NewFileName );

		// CS&ZHL 2009-02-24: check if need update files 
		if( UpdateFlag==1 )
		{
			_stprintf( cmdline, _T("%s %s"), EXE_NAME, EXE_PAR );
			_tcscpy( EXE_NAME, _T("Update.exe" ) );
			_tcscpy( EXE_PAR, cmdline );
		}

		bResult = SetMemoryDivision( StorePercent );

		//set i1 > 3
		if( wcslen( EXE_NAME ) > 4 )
		{
			for(i1 = 0; i1 < 3; i1++)
			{
				b = CreateProcess
				( 
					EXE_NAME,				  // LPCWSTR lpszImageName, 
					EXE_PAR,                  // LPCWSTR lpszCmdLine, 
					NULL,                     // LPSECURITY_ATTRIBUTES lpsaProcess, 
					NULL,                     // LPSECURITY_ATTRIBUTES lpsaThread, 
					FALSE,                    // BOOL fInheritHandles, 
					0,						  // DWORD fdwCreate, 
					NULL,                     // LPVOID lpvEnvironment, 
					NULL,                     // LPWSTR lpszCurDir, 
					NULL,                     // LPSTARTUPINFOW lpsiStartInfo, 
					&procInfo                 // LPPROCESS_INFORMATION lppiProcInfo
				); 

				if( b )
				{
					CloseHandle(procInfo.hThread);		
					CloseHandle(procInfo.hProcess);
					break;
				}

				Sleep( 5 );
			}
		}
		//
		// CS&ZHL OCT-20-2010: if create process fail for 3 times, then system reboot.
		//
		if( i1 >= 3 )
		{
			dwCountOfStartFail++;
			SetCountOfStartFail( dwCountOfStartFail );
			if(  dwCountOfStartFail < 5 )
			{
				RETAILMSG( 1, (TEXT("\r\nwstartup::Something wrong with userinfo %d, system reboot...\r\n"), i1));	
				SysReboot( );
			}
		}
		else
		{
			if(  dwCountOfStartFail !=0 )
			{
				dwCountOfStartFail = 0;
				SetCountOfStartFail( dwCountOfStartFail );
			}
		}
#ifndef EM9283
		bResult = SetAdapterIPProperties( AdaptersName.szAdapterName[0], NewFileName );
#endif
	} 
    return 0;
}

