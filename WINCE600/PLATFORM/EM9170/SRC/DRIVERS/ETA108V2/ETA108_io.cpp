// ETA108.cpp : Defines the entry point for the DLL application.
//

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "ETA108Class.h"

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types
#define ETA108_IOCTL_START           CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 4030, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define ETA108_IOCTL_SETUP           CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 4031, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define ETA108_IOCTL_STOP			 CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 4032, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define ETA108_IOCTL_WAITEDATAREADY  CTL_CODE(FILE_DEVICE_BUS_EXTENDER, 4033, METHOD_BUFFERED, FILE_ANY_ACCESS) 


//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

BOOL WINAPI ETA108_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
	// Remove-W4: Warning C4100 workaround
	UNREFERENCED_PARAMETER(lpvReserved);

	switch (dwReason) 
	{
	case DLL_PROCESS_ATTACH:
		DEBUGMSG(1, (TEXT("ETA108_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
		DisableThreadLibraryCalls((HMODULE) hInstDll);
		break;

	case DLL_PROCESS_DETACH:
		DEBUGMSG(1, (TEXT("ETA108_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
		break;
	}
	// return TRUE for success
	return TRUE;
}

DWORD ADS_Init(LPCTSTR pContext)
{
//	DWORD dwDataSize;
//	LONG regError;
	HKEY hKey;
	eta108Class *pETA108 = NULL;

	RETAILMSG( 1, (TEXT("ETA108V2_Init +\r\n")));
	
	pETA108 = new eta108Class();
	if( pETA108 == NULL )
	{
		return 0;
	}

	// try to open active device registry key for this context
	hKey = OpenDeviceKey(pContext);
	if (hKey == NULL)
	{
		DEBUGMSG(1, (TEXT("ETA108_Init:  OpenDeviceKey failed!!!\r\n")));
		return 0;
	}

// 	try to load CSPI index from registry data
// 		dwDataSize = sizeof(DWORD);
// 		regError = RegQueryValueEx(
// 			hKey,                       // handle to currently open key
// 			//REG_DEVINDEX_VAL_NAME,      // string containing value to query
// 			TEXT("CSPIChannel"),
// 			NULL,                       // reserved, set to NULL
// 			NULL,                       // type not required, set to NULL
// 			(LPBYTE)(&pETA108->m_dwCSPIChannle),      // pointer to buffer receiving value
// 			&dwDataSize);               // pointer to buffer size
// 	
// 		// check for errors during RegQueryValueEx
// 		if (regError != ERROR_SUCCESS)
// 		{ 
// 			RegCloseKey(hKey);
// 			DEBUGMSG(ZONE_ERROR, (TEXT("ETA108_Init:  RegQueryValueEx failed!!!\r\n")));
// 			return 0;
// 		}
// 	
// 		// try to load PWM index from registry data
// 		dwDataSize = sizeof(DWORD);
// 		regError = RegQueryValueEx(
// 			hKey,                       // handle to currently open key
// 			TEXT("PWMChannel"),
// 			NULL,                       // reserved, set to NULL
// 			NULL,                       // type not required, set to NULL
// 			(LPBYTE)(&pETA108->m_dwPWMChannle),      // pointer to buffer receiving value
// 			&dwDataSize);    
// 	
// 		// check for errors during RegQueryValueEx
// 		if (regError != ERROR_SUCCESS)
// 		{
// 			RegCloseKey(hKey);
// 			DEBUGMSG(ZONE_ERROR, (TEXT("ETA108_Init:  RegQueryValueEx failed!!!\r\n")));
// 			return 0;
// 		}
// 	
// 		// try to load CSPI DMA buffer size from registry data
// 		dwDataSize = sizeof(DWORD);
// 		regError = RegQueryValueEx(
// 			hKey,                       // handle to currently open key
// 			TEXT("DMABufSize"),
// 			NULL,                       // reserved, set to NULL
// 			NULL,                       // type not required, set to NULL
// 			(LPBYTE)(&pETA108->m_dwDMABufSize ),      // pointer to buffer receiving value
// 			&dwDataSize);    
// 	
// 		// check for errors during RegQueryValueEx
// 		if (regError != ERROR_SUCCESS)
// 		{
// 			RegCloseKey(hKey);
// 			DEBUGMSG(ZONE_ERROR, (TEXT("ETA108_Init:  RegQueryValueEx failed!!!\r\n")));
// 			return 0;
// 		}
// 	
// 		// try to load buffer Size of Continuous Sampling from registry data
// 		dwDataSize = sizeof(DWORD);
// 		regError = RegQueryValueEx(
// 			hKey,                       // handle to currently open key
// 			TEXT("MultDMABufSize"),
// 			NULL,                       // reserved, set to NULL
// 			NULL,                       // type not required, set to NULL
// 			(LPBYTE)(&pETA108->m_dwMultDmaBufSize ),      // pointer to buffer receiving value
// 			&dwDataSize);    
// 	
// 		// check for errors during RegQueryValueEx
// 		if (regError != ERROR_SUCCESS)
// 		{
// 			RegCloseKey(hKey);
// 			DEBUGMSG(ZONE_ERROR, (TEXT("ETA108_Init:  RegQueryValueEx failed!!!\r\n")));
// 			return 0;
// 		}

	// close handle to open key
	RegCloseKey(hKey);

	//ETA108 initialize
 	if( !pETA108->ETA108Initialize())
 	{
 		RETAILMSG(1, (TEXT("ETA108_Init:  CspiInitialize failed!!!\r\n")));
 		return 0;
 	}
 	
 	RETAILMSG(1, (TEXT("ETA108_Init: pETA108=0x%x\r\n"), pETA108));
	return (DWORD)pETA108;
}

BOOL ADS_Deinit(DWORD hDeviceContext)
{
	eta108Class *pETA108 = (eta108Class *)hDeviceContext;

	if( pETA108 )
	{
		pETA108->ETA108Release();
		delete pETA108;
		pETA108 = NULL;
	}
	RETAILMSG( 1, (TEXT("ETA108_Init -\r\n")));
	return TRUE;
}

DWORD ADS_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{

	//RETAILMSG(1, (TEXT("ADS_Open: hDeviceContext=0x%x\r\n"), hDeviceContext));
	// Remove-W4: Warning C4100 workaround
	UNREFERENCED_PARAMETER(AccessCode);
	UNREFERENCED_PARAMETER(ShareMode);
	//UNREFERENCED_PARAMETER(hDeviceContext);

	
	eta108Class *pETA108 = (eta108Class *)hDeviceContext;
	if(!pETA108->ETA108Open())
	{
		return FALSE;
	}
	return hDeviceContext;
	
}

BOOL ADS_Close(DWORD hOpenContext)
{
	eta108Class *pETA108 = (eta108Class *)hOpenContext;
	
	if( pETA108 )
	{
		return pETA108->ETA108Close();
	}
	return FALSE;
}

void ADS_PowerDown(DWORD hDeviceContext)
{
	// Remove-W4: Warning C4100 workaround
	UNREFERENCED_PARAMETER(hDeviceContext);
}

void ADS_PowerUp(void)
{
}

DWORD ADS_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	eta108Class *pETA108 = (eta108Class *)hOpenContext;

	if( pETA108 )
	{
		return pETA108->ETA108Read( pBuffer, Count );
	}
	return FALSE;
}

DWORD ADS_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD dwNumBytes)
{
	// Remove-W4: Warning C4100 workaround
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(pBuffer);
	UNREFERENCED_PARAMETER(dwNumBytes);
	return DWORD(-1);
}

DWORD ADS_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
	// Remove-W4: Warning C4100 workaround
	UNREFERENCED_PARAMETER( hOpenContext );
	UNREFERENCED_PARAMETER(Amount);
	UNREFERENCED_PARAMETER(Type);

	return (DWORD)-1;
}

BOOL ADS_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
				   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
				   PDWORD pdwActualOut)
{
	BOOL bRet = FALSE;
	eta108Class *pETA108 = (eta108Class *)hOpenContext;

	if( pETA108 )
	{
		switch( dwCode )
		{
		case ETA108_IOCTL_START:
			bRet = pETA108->ETA108Start( );
			break;

		case ETA108_IOCTL_SETUP:
			if( pBufIn && dwLenIn == sizeof(ADS_CONFIG ))
			{
				bRet = pETA108->ETA108Setup( (PADS_CONFIG)pBufIn, pBufOut, dwLenOut, pdwActualOut );
			}
			break;

		case ETA108_IOCTL_STOP:
			bRet = pETA108->ETA108Stop( );
			break;

		case ETA108_IOCTL_WAITEDATAREADY:
			if( pBufIn && dwLenIn == sizeof(DWORD))
			{
				bRet = pETA108->WateDataReady( (DWORD)(*(DWORD *)pBufIn) );
			}
			break;

		default:bRet = FALSE;
		}
	}
	return bRet;
}