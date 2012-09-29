//------------------------------------------------------------------------------
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS 
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//------------------------------------------------------------------------------
#pragma warning(disable: 4100 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <bsp.h>
#include "common_nandfmd.h"
#include "csp.h"

//
// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
//-----------------------------------------------------------------------------
// External Functions
//extern BOOL BSPNAND_SetClock(BOOL bEnabled);
//extern VOID BSPNAND_ConfigIOMUX(DWORD CsNum);
//extern VOID* BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size);
//extern BOOL BSP_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
//    PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned);

//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------
// Defines
#define MAX_GPMI_CLK_FREQUENCY_kHZ (120000)


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
static BOOL						g_bNandfmdInitialized = FALSE;
static FlashInfo				g_OalFlashInfo;


//-----------------------------------------------------------------------------
//
//  Function:  BSPNAND_SetClock
//
//  This enables/disable clocks for the NANDFC.
//
//  Parameters:
//     bEnabled
//          [in] - enable/disable clock.  
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void BSPNAND_SetClock(BOOL bEnabled)
{
    BOOL rc = TRUE;
    UINT32 frequency , rootfreq, u32Div;
    static BOOL bInit = FALSE;

    if(!bInit){
        // Bump GPMI_CLK frequency up to the maximum.
        frequency = MAX_GPMI_CLK_FREQUENCY_kHZ;
        //status = DDKClockSetGpmiClk(&frequency, TRUE);

        DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_GPMI, &rootfreq);

        u32Div = rootfreq / (frequency*1000);
        if(u32Div != 0)
            rc = DDKClockConfigBaud(DDK_CLOCK_SIGNAL_GPMI, DDK_CLOCK_BAUD_SOURCE_REF_GPMI, u32Div );
        if (rc != TRUE)
        {
            //return rc;
            return;
        }    
        bInit = TRUE;
    }
    
    if (bEnabled)
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, FALSE);
    }
    else
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, TRUE);
    }

    
    //return rc;
    return;
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPNAND_RemapRegister
//
//  This functions remaps certain registers for high level use.  
//
//  Parameters:
//      PhyAddr
//          [in] - physical address that needs to be remapped.  
//
//      size
//          [in] - mapping size
//  Returns:
//      Pointer to the remapped address.
//
//-----------------------------------------------------------------------------
VOID* BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size)
{
    //PHYSICAL_ADDRESS phyAddr;
    
    //phyAddr.QuadPart = PhyAddr;
    //return MmMapIoSpace(phyAddr, size, FALSE);

	UNREFERENCED_PARAMETER(size);
    return OALPAtoUA(PhyAddr);
}

VOID BSPNAND_UnmapRegister(PVOID VirtAddr, DWORD size)
{
    UNREFERENCED_PARAMETER(size);
    VirtAddr = NULL;
}

//-----------------------------------------------------------------------------
//
// CS&ZHL MAY-13-2011: provide Init / Read / Write / Erase 
//
//-----------------------------------------------------------------------------
BOOL OALFMD_Access(VOID* pInpBuffer, UINT32 inpSize)
{
	PFmdAccessInfo	pInfo = NULL;
	BOOL			bRet = FALSE;
	DWORD			dwStatus;
	DWORD			dwReturnBytes = 0;

	//OALMSG(1, (L"->OALFMD_Access..\r\n"));

	if((pInpBuffer == NULL) || (inpSize != sizeof(FmdAccessInfo)))
		return FALSE;

	pInfo = (PFmdAccessInfo)pInpBuffer;

	switch(pInfo->dwAccessCode)
	{
	case FMD_ACCESS_CODE_HWINIT:
		//OALMSG(1, (L"->OALFMD_Access->FMD_ACCESS_CODE_HWINIT\r\n"));
		if(!g_bNandfmdInitialized)
		{
			DWORD	dwType = 0;			// -> vendor authentication

			BSPNAND_SetClock(TRUE);		//enable nfc clock

			// init NandFlash
			//OALMSG(1, (L"->OALFMD_Access->FMD_Init(...)\r\n"));
			bRet = (BOOL)FMD_Init(NULL, NULL, NULL);
			if(!bRet)
			{
				RETAILMSG(1, (TEXT("OALFMD_Access: Init failed\r\n")));
				BSPNAND_SetClock(FALSE);		//stop clock to NFC
				break;
			}

			// get NandFlash Info
			//OALMSG(1, (L"->OALFMD_Access->FMD_GetInfo(...)\r\n"));
			bRet = FMD_GetInfo(&g_OalFlashInfo);
			if(!bRet)
			{
				RETAILMSG(1, (TEXT("OALFMD_Access: Get Flash Info failed\r\n")));
				BSPNAND_SetClock(FALSE);		//stop clock to NFC
				break;
			}

			// CS&ZHL APR-9-2012: do vendor security check
			bRet = FMD_OEMIoControl(IOCTL_DISK_AUTHENTICATION,
									(PBYTE)&dwType, sizeof(DWORD),
									(PBYTE)&dwStatus, sizeof(DWORD),
									&dwReturnBytes);
			if(bRet && (dwReturnBytes == sizeof(DWORD)) && (dwStatus == 1))
			{
				RETAILMSG(1, (TEXT("DISK_VENDOR_AUTHENTICATION passed\r\n")));
			}
			else
			{
				RETAILMSG(1, (TEXT("DISK_VENDOR_AUTHENTICATION failed: %d, %d status = %d\r\n"), bRet, dwReturnBytes, dwStatus));
				RETAILMSG(1, (TEXT("spin forever\r\n")));
				for( ; ; );
			}
			// end of CS&ZHL APR-9-2012: do security check

			g_bNandfmdInitialized = TRUE;
		}
		else
		{
			OALMSG(1, (L"->OALFMD_Access->FMD_ACCESS_CODE_HWINIT done already\r\n"));
			bRet = TRUE;
		}
		break;

	case FMD_ACCESS_CODE_READSECTOR:
		if(g_bNandfmdInitialized)
		{
			bRet = FMD_ReadSector((SECTOR_ADDR)pInfo->dwStartSector, (LPBYTE)pInfo->pMData, (PSectorInfo)pInfo->pSData, pInfo->dwSectorNum);
		}
		break;

	case FMD_ACCESS_CODE_WRITESECTOR:
		if(g_bNandfmdInitialized)
		{
			bRet = FMD_WriteSector((SECTOR_ADDR)pInfo->dwStartSector, (LPBYTE)pInfo->pMData, (PSectorInfo)pInfo->pSData, pInfo->dwSectorNum);
		}
		break;

	case FMD_ACCESS_CODE_ERASEBLOCK:
		if(g_bNandfmdInitialized)
		{
			//bRet = FMD_EraseBlock((BLOCK_ID)(SECTOR_TO_BLOCK(pInfo->dwStartSector)));
			bRet = FMD_EraseBlock((BLOCK_ID)(pInfo->dwStartSector / g_OalFlashInfo.wSectorsPerBlock));
		}
		break;

	case FMD_ACCESS_CODE_GETINFO:
		//OALMSG(1, (L"->OALFMD_Access::FMD_ACCESS_CODE_GETINFO\r\n"));
		if(g_bNandfmdInitialized)
		{
			DWORD		dwLen = 0;
			
			if(pInfo->pSData != NULL)
			{
				memcpy(&dwLen, pInfo->pSData, sizeof(dwLen));
			}

			if((pInfo->pMData != NULL) && (dwLen == sizeof(FlashInfo)))
			{
				memcpy(pInfo->pMData, &g_OalFlashInfo, sizeof(FlashInfo));
				bRet = TRUE;
				//OALMSG(1, (L"->OALFMD_Access::FMD_ACCESS_CODE_GETINFO done!\r\n"));
			}
		}
		break;

	case FMD_ACCESS_CODE_GETSTATUS:
		if(g_bNandfmdInitialized)
		{
			if(pInfo->pMData != NULL)
			{
				dwStatus = FMD_GetBlockStatus((BLOCK_ID)(pInfo->dwStartSector / g_OalFlashInfo.wSectorsPerBlock));
				memcpy(pInfo->pMData, &dwStatus, sizeof(dwStatus));
				bRet = TRUE;
			}
		}
		break;

	case FMD_ACCESS_CODE_SETSTATUS:
		if(g_bNandfmdInitialized)
		{
			if(pInfo->pMData != NULL)
			{
				memcpy(&dwStatus, pInfo->pMData, sizeof(dwStatus));
				bRet = FMD_SetBlockStatus((BLOCK_ID)(pInfo->dwStartSector / g_OalFlashInfo.wSectorsPerBlock), dwStatus);
			}
		}
		break;

	//
	// CS&ZHL SEP-18-2012: supporting UserID write and verify
	//
	case FMD_ACCESS_CODE_WRITEUID:			// pMData ->UserID buffer; *((PDWORD)pSData) = sizeof(UserID buffer)
		if(g_bNandfmdInitialized)
		{
			NANDWrtImgInfo	WriteInfo;
			PBYTE			pImageBuf;
			DWORD			dwBufSize;
			
			// setup write image type
			WriteInfo.dwImgType = IMAGE_UID;
			WriteInfo.dwIndex = 0;
			WriteInfo.dwImgSizeUnit = g_OalFlashInfo.wSectorsPerBlock * g_OalFlashInfo.wDataBytesPerSector;

			// setup userID buffer
			pImageBuf = (PBYTE)pInfo->pMData;
			dwBufSize = *((DWORD*)pInfo->pSData);
			// write UserID in buffer
			bRet = FMD_OEMIoControl(IOCTL_DISK_VENDOR_WRITE_IMAGE,
									(PBYTE)&WriteInfo, sizeof(NANDWrtImgInfo),
									(PBYTE)pImageBuf, dwBufSize,
									NULL);
			if(!bRet)
			{
				RETAILMSG(1, (TEXT("Write UserID failed\r\n")));
			}
		}
		break;

	case FMD_ACCESS_CODE_VERIFYUID:			// pMData ->UserID buffer; *((PDWORD)pSData) = sizeof(UserID buffer)
		if(g_bNandfmdInitialized)
		{
			PBYTE	pImageBuf;
			DWORD	dwBufSize;

			// setup userID buffer
			pImageBuf = (PBYTE)pInfo->pMData;
			dwBufSize = *((DWORD*)pInfo->pSData);
			if(dwBufSize <= sizeof(DWORD))
			{
				bRet = FALSE;
				RETAILMSG(1, (TEXT("Invalid UserID: spin forever\r\n")));
				for( ; ; );
			}

			// verify UserID in buffer
			bRet = FMD_OEMIoControl(IOCTL_DISK_AUTHENTICATION,
									(PBYTE)pImageBuf, dwBufSize,
									(PBYTE)&dwStatus, sizeof(DWORD),
									&dwReturnBytes);
			if(bRet && (dwReturnBytes == sizeof(DWORD)) && (dwStatus == 1))
			{
				RETAILMSG(1, (TEXT("User Authentication passed\r\n")));
			}
			else
			{
				RETAILMSG(1, (TEXT("User Authentication failed: spin forever\r\n")));
				for( ; ; );
			}
		}
		break;

	default:
		RETAILMSG(1, (TEXT("OALFMD_Access: not support access code = %d.\r\n"), pInfo->dwAccessCode));
	}
	
	return bRet;
}

#endif		//NAND_PDD
