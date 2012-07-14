//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ioctl.c
//
//  This file implements the OEM's IO Control (IOCTL) functions and declares
//  global variables used by the IOCTL component.
//
//-----------------------------------------------------------------------------
#include <bsp.h>
#include <bsp_drivers.h>			// CS&ZHL MAY-30-2012: supporting board state
#include <usbkitl.h>
//#ifdef	EM9280
#include <em9280_oal.h>
//#endif	//EM9280

//-----------------------------------------------------------------------------
// External Functions
extern VOID ResetChip();

//-----------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregPOWER;
extern PVOID pv_HWregDIGCTL;		// CS&ZHL APR-9-2012: read CPU Info

//-----------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlPlatformType/OEM
//
//  Platform Type/OEM
//
LPCWSTR g_oalIoCtlPlatformType = IOCTL_PLATFORM_TYPE;
LPCWSTR g_oalIoCtlPlatformOEM  = IOCTL_PLATFORM_OEM;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlProcessorVendor/Name/Core
//
//  Processor information
//
LPCWSTR g_oalIoCtlProcessorVendor = IOCTL_PROCESSOR_VENDOR;
LPCWSTR g_oalIoCtlProcessorName   = IOCTL_PROCESSOR_NAME;
LPCWSTR g_oalIoCtlProcessorCore   = IOCTL_PROCESSOR_CORE;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlInstructionSet
//
//  Processor instruction set identifier
//
UINT32 g_oalIoCtlInstructionSet = IOCTL_PROCESSOR_INSTRUCTION_SET;
UINT32 g_oalIoCtlClockSpeed     = IOCTL_PROCESSOR_CLOCK_SPEED;

//
// CS&ZHL MAR-8-2012: Built Time Stamp
//
static char		g_BuiltTimeStamp[64];
static DWORD	g_dwBuiltTimeStampLength = 0;

//
// CS&ZHL FEB-28-2012: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
CRITICAL_SECTION	g_oalNfcMutex;

extern BOOL OALFMD_Access(VOID* pInpBuffer, UINT32 inpSize);
#endif	//NAND_PDD	

//---------------------------------------------------------------------------------
// OAL operations for EM9280 only
#ifdef	EM9280
#ifndef	UUT
//
// CS&ZHL MAR-8-2012: supporting multiple SPI ports of UART(HT45B0F)
//
CRITICAL_SECTION	g_oalSpiMutex;

extern void OALSSP0SpiInit(void);
extern BOOL OALSPI_Access(VOID* pInpBuffer, UINT32 inpSize);
#endif	//UUT

//
// CS&ZHL MAR-18-2012: supporting I2C bus for RTC and GPIOX
//
CRITICAL_SECTION	g_oalI2cMutex;

extern void OALGpioI2cInit(void);
extern BOOL OALI2C_Access(VOID* pInpBuffer, UINT32 inpSize);
#endif	//EM9280
//
//---------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//
//  This function is a placeholder by BSQR.  The ADS board used a PMIC implementation.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlHalReboot(UINT32 code, VOID *pInpBuffer,
                       UINT32 inpSize, VOID *pOutBuffer,
                       UINT32 outSize, UINT32 *pOutSize)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // If the board design supports software-controllable hardware reset logic,
    // it should be used.  This routine can be overidden in the specific
    // platform code to control board-level reset logic, should it exist.
    //

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));

    // Reset Chip
    ResetChip();
    // Wait for reset...
    //
    for(;;);

#if 0    // Should never get to this point...
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));

    return TRUE;
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalPostInit
//
//  This function is the next OAL routine called by the kernel after OEMInit and
//  provides a context for initializing other aspects of the device prior to
//  general boot.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlHalPostInit(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

	//
	// CS&ZHL JUN-24-2011: init time stamp
	//
	memset(g_BuiltTimeStamp, 0x20, 64);						// set all in space
	memcpy(g_BuiltTimeStamp, 	"Emtronix Built at ", 18);
	memcpy(&g_BuiltTimeStamp[18], __DATE__, 11);			// "Jun 23 2011"
	memcpy(&g_BuiltTimeStamp[29], " ", 1);					// " "
	memcpy(&g_BuiltTimeStamp[30], __TIME__, 8);				// "12:44:02"
	g_BuiltTimeStamp[38] = '\0';
	g_dwBuiltTimeStampLength = 38;


	//
	// CS&ZHL FEB-28-2012: supporting multiple partitions of NandFlash
	//
#ifdef NAND_PDD
    OALMSG(1, (L"OALIoCtlHalPostInit::InitializeCriticalSection(&g_oalNfcMutex)\r\n"));
	InitializeCriticalSection(&g_oalNfcMutex);
#endif	//NAND_PDD

#ifdef	EM9280
#ifndef	UUT
	//
	// CS&ZHL MAR-8-2012: supporting multiple SPI ports of UART(HT45B0F)
	//
    OALMSG(1, (L"OALIoCtlHalPostInit::InitializeCriticalSection(&g_oalSpiMutex)\r\n"));
	InitializeCriticalSection(&g_oalSpiMutex);
	OALSSP0SpiInit();		// init SPI port
#endif	//UUT

	//
	// CS&ZHL MAR-18-2012: supporting GPIO based I2C for RTC and GPIOX
	//
    OALMSG(1, (L"OALIoCtlHalPostInit::InitializeCriticalSection(&g_oalI2cMutex)\r\n"));
	InitializeCriticalSection(&g_oalI2cMutex);
	OALGpioI2cInit();		// init I2C port
#endif	//EM9280

	return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalPresuspend
//
//  This function implements IOCTL_HAL_PRESUSPEND which provides the OAL
//  the time needed to prepare for a suspend operation. Any preparation is
//  completed while the system is still in threaded mode.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlHalPresuspend(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // Do nothing for now.
    //
    return(TRUE);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlPowerOffEnable
//
//  This function implements IOCTL_HAL_PRESUSPEND which provides the OAL
//  the time needed to prepare for a suspend operation. Any preparation is
//  completed while the system is still in threaded mode.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlPowerOffEnable(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // Do nothing for now.
    //
    if(code == IOCTL_HAL_POWER_OFF_ENABLE)
    {
        BF_WR(POWER_RESET,UNLOCK,BV_POWER_RESET_UNLOCK__KEY);

        //Chip power off
        HW_POWER_RESET_WR((BV_POWER_RESET_UNLOCK__KEY << BP_POWER_RESET_UNLOCK) | BM_POWER_RESET_PWD);
        for(;;) 
        { 
            OALMSG(OAL_ERROR, (L"Error the code should never been executed!! \r\n"));
        };

    }
    return(TRUE);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlQueryBoardId
//
//  This function implements IOCTL_HAL_QUERY_BOARD_ID and is used to query the
//  board id of the EVK.  The result returned by this IOCTL may be used to 
//  conditionally execute code for a specific board revision of the EVK.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlQueryBoardId (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    if (lpBytesReturned) 
    {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) 
    {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(DWORD) > nOutBufSize) 
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {

            // Return board ID that was was read during OEMInit
            //*((PDWORD) lpOutBuf) = (DWORD) g_dwBoardID;

            if (lpBytesReturned) 
            {
                *lpBytesReturned = sizeof (DWORD);
            }

        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) 
    {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlQueryBootMode
//
//  This function implements IOCTL_HAL_QUERY_BOOT_MODE and is used to query the
//  boot mode.  (NAND/SDMMC/SPI)
//
//------------------------------------------------------------------------------
BOOL OALIoCtlQueryBootMode (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    if (lpBytesReturned) 
    {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) 
    {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(BootMode) > nOutBufSize) 
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {
            PVOID pv_HWregRTC = OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);

            if(pv_HWregRTC)
            {
                switch(HW_RTC_PERSISTENT3_RD())
                {
                    case BOOT_MODE_SDMMC:
                        ((PBootMode)lpOutBuf)->media = SDMMCBoot;
                        break;   
                    case BOOT_MODE_SPI:
                        ((PBootMode)lpOutBuf)->media = SPIBoot;
                        break;   
                    default:
                        ((PBootMode)lpOutBuf)->media = NandBoot;
                        break;  
                }
                
                if(HW_RTC_PERSISTENT1_RD() & BM_PERSIST1_NAND_SECONDARY_BOOT)
                {
                    ((PBootMode)lpOutBuf)->source = Second;
                }
                else
                {
                    ((PBootMode)lpOutBuf)->source = First;
                }
                
                if (lpBytesReturned) 
                {
                    *lpBytesReturned = sizeof (BootMode);
                }
            }
            else
                dwErr = ERROR_INVALID_ACCESS;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) 
    {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlSetBootSource
//
//  This function implements IOCTL_HAL_SET_BOOT_SOURCE and is used to set the
//  boot source.  (First/Second)
//
//------------------------------------------------------------------------------
BOOL OALIoCtlSetBootSource (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    if (lpBytesReturned) 
    {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) 
    {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(BootMode) > nOutBufSize) 
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {
            PVOID pv_HWregRTC = OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);

            if(pv_HWregRTC)
            {
                if(((PBootMode)lpOutBuf)->source == First)
                {
                    HW_RTC_PERSISTENT1_WR(0);
                }
                else
                {
                    HW_RTC_PERSISTENT1_WR(BM_PERSIST1_NAND_SECONDARY_BOOT);
                }
                
                if (lpBytesReturned) 
                {
                    *lpBytesReturned = sizeof (BootMode);
                }
            }
            else
                dwErr = ERROR_INVALID_ACCESS;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) 
    {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlQueryUpdateSig
//
//  This function implements IOCTL_HAL_QUERY_UPDATE_SIG and is used to query the
//  update signature.  
//
//------------------------------------------------------------------------------
BOOL OALIoCtlQueryUpdateSig (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    if (lpBytesReturned) 
    {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) 
    {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(DWORD) > nOutBufSize) 
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {
            PVOID pv_HWregRTC = OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);

            if(pv_HWregRTC)
            {
                *(DWORD*)lpOutBuf = HW_RTC_PERSISTENT2_RD();
                
                if (lpBytesReturned) 
                {
                    *lpBytesReturned = sizeof (DWORD);
                }
            }
            else
                dwErr = ERROR_INVALID_ACCESS;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) 
    {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlSetUpdateSig
//
//  This function implements IOCTL_HAL_Set_UPDATE_SIG and is used to query the
//  update signature.  
//
//------------------------------------------------------------------------------
BOOL OALIoCtlSetUpdateSig (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    if (lpBytesReturned) 
    {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) 
    {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(DWORD) > nOutBufSize) 
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {
            PVOID pv_HWregRTC = OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);
            
            if(pv_HWregRTC)
            {
                HW_RTC_PERSISTENT2_WR(*(DWORD*)lpOutBuf);
                
                if (lpBytesReturned) 
                {
                    *lpBytesReturned = sizeof (DWORD);
                }
            }
            else
                dwErr = ERROR_INVALID_ACCESS;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) 
    {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}

//
// CS&ZHL FEB-28-2012: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
BOOL OALIoCtlHalNandfmdAccess(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL bResult;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	//OALMSG(1, (L"->OALIoCtlHalNandfmdAccess\r\n"));

	EnterCriticalSection(&g_oalNfcMutex);
	bResult = OALFMD_Access(pInpBuffer, inpSize);
	LeaveCriticalSection(&g_oalNfcMutex);

	return bResult;
}
#endif	//NAND_PDD

//----------------------------------------------------------------------------
// EM9280 IOCTL routines
//----------------------------------------------------------------------------
#ifdef  EM9280
#ifndef UUT

// CS&ZHL MAR-8-2012: access GPIO based SPI port
BOOL OALIoCtlHalSpiAccess(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL bResult;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	//OALMSG(1, (L"->OALIoCtlHalSpiAccess\r\n"));

	EnterCriticalSection(&g_oalSpiMutex);
	bResult = OALSPI_Access(pInpBuffer, inpSize);
	LeaveCriticalSection(&g_oalSpiMutex);

	return bResult;
}
#endif	//UUT

// CS&ZHL MAY-18-2012: access GPIO based I2C bus
BOOL OALIoCtlHalI2cAccess(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL bResult;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	//OALMSG(1, (L"->OALIoCtlHalI2cAccess\r\n"));

	EnterCriticalSection(&g_oalI2cMutex);
	bResult = OALI2C_Access(pInpBuffer, inpSize);
	LeaveCriticalSection(&g_oalI2cMutex);

	return bResult;
}
#endif	//EM9280

//
// CS&ZHL APR-06-2012: read various info about EM9280
//
BOOL OALIoCtlHalBoardInfoRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
										VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL	bResult = FALSE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	/*
	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalBoardInfoRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}
exit:
	*/

	return bResult;
}


BOOL OALIoCtlHalTimeStampRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
										VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL	bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize < (g_dwBuiltTimeStampLength + 1)))
	{
		OALMSG(1, (L"OALIoCtlHalTimeStampRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	// copy built timestamp string to output buffer
	memcpy(pOutBuffer, g_BuiltTimeStamp, g_dwBuiltTimeStampLength);
	
	if(pOutSize != NULL)
	{
		*pOutSize = g_dwBuiltTimeStampLength;
	}

exit:
	return bResult;
}

BOOL OALIoCtlHalVendorIDRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
										VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL	bResult = FALSE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	/*
	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalVendorIDRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}

exit:
	*/

	return bResult;
}

BOOL OALIoCtlHalCustomerIDRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
											VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL	bResult = FALSE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	/*
	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalVendorIDRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}

exit:
	*/

	return bResult;
}

BOOL OALIoCtlHalBoardStateRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
											VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	DWORD		dwBoardState = 0;
	BOOL		bResult = TRUE;
    BSP_ARGS	*pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;

#define	EM9280_BOARD_STATE_DBGSL		(1 << 7)

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalBoardStateRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	// get DBGSLn state
	if(pBspArgs->bDebugFlag)
	{
		dwBoardState |= EM9280_BOARD_STATE_DBGSL;
	}

	//copy board state info to output buffer
	memcpy(pOutBuffer, &dwBoardState, sizeof(DWORD));
	
	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}

exit:
	return bResult;
}

BOOL OALIoCtlHalWatchdogGet(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	PWATCHDOG_INFO	pWDT;
	BOOL			bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(WATCHDOG_INFO)))
	{
		OALMSG(1, (L"OALIoCtlHalWatchdogGet: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	if((pfnOEMRefreshWatchDog != NULL) && (dwOEMWatchDogPeriod != 0))
	{
		//
		// call OEMInitWatchDogTimer(DWORD dwWatchdogPeriod) before
		//
		pfnOEMRefreshWatchDog( );		//refresh WDT first

		// transfer kernel wdt info out
		pWDT = (PWATCHDOG_INFO)pOutBuffer;
		pWDT->pfnKickWatchDog = pfnOEMRefreshWatchDog;
		pWDT->dwWatchDogPeriod = dwOEMWatchDogPeriod;
		pWDT->dwWatchDogThreadPriority = 100;						//dwNKWatchDogThreadPriority;

		//pfnOEMRefreshWatchDog = NULL;
		dwOEMWatchDogPeriod = 0;

		if(pOutSize != NULL)
		{
			*pOutSize = sizeof(WATCHDOG_INFO);
		}
	}

exit:
	return bResult;
}



BOOL OALIoCtlHalGetCPUInfo(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
	PIMX_CPU_INFO	pCPUInfo;
	DWORD			dwTmp[4];

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(IMX_CPU_INFO)))
	{
		OALMSG(1, (L"OALIoCtlHalGetCPUInfo::invalid parameters!\r\n"));
		return FALSE;
	}

	pCPUInfo = (PIMX_CPU_INFO)pOutBuffer;

	dwTmp[0] = *((volatile DWORD*)(HW_DIGCTL_FSL_ADDR + 0x00));		// -> (pv_HWregDIGCTL + 0x300)
	dwTmp[1] = *((volatile DWORD*)(HW_DIGCTL_FSL_ADDR + 0x04));		// -> (pv_HWregDIGCTL + 0x304)
	dwTmp[2] = *((volatile DWORD*)(HW_DIGCTL_FSL_ADDR + 0x08));		// -> (pv_HWregDIGCTL + 0x308)
	pCPUInfo->dwStringLen = 3 * sizeof(DWORD);
	memcpy(pCPUInfo->FSLString, dwTmp, pCPUInfo->dwStringLen);
	pCPUInfo->FSLString[pCPUInfo->dwStringLen] = '\0';
	
	pCPUInfo->dwChipID = HW_DIGCTL_CHIPID_RD();						// -> (pv_HWregDIGCTL + 0x310)

	if(pOutSize)
	{
		*pOutSize = sizeof(IMX_CPU_INFO);
	}
	//OALMSG(1, (L"OALIoCtlHalGetCPUInfo::%s 0x%08x\r\n", pCPUInfo->FSLString, pCPUInfo->dwChipID));

	return TRUE;
}

//------------------------------------------------------------------------------
//
//  Global: g_oalIoCtlTable[]
//
//  IOCTL handler table. This table includes the IOCTL code/handler pairs
//  defined in the IOCTL configuration file. This global array is exported
//  via oal_ioctl.h and is used by the OAL IOCTL component.
//
const OAL_IOCTL_HANDLER g_oalIoCtlTable[] = {
#include "ioctl_tab.h"
};

//------------------------------------------------------------------------------
