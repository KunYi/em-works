//------------------------------------------------------------------------------
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS 
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//------------------------------------------------------------------------------

//
// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
#pragma warning(disable: 4100 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <bsp.h>
#include "common_nandfmd.h"
#include "csp.h"

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


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
static BOOL						g_bNandfmdInitialized = FALSE;
static FlashInfo					g_OalFlashInfo;
extern PCSP_CRM_REGS	g_pCRM;								// CS&ZHL MAY-16-2011: for nfc_clock setup

//-----------------------------------------------------------------------------
//
//  Function:  NFCSetClock
//
//  This enables/disable clocks for the NANDFC.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL NFCSetClock(BOOL bEnabled)
{
	UINT32 index = DDK_CLOCK_GATE_INDEX_PER_NFC;		// = 8 in iMX257
	UINT32 mode = (UINT32)bEnabled;									// = 0, 1
	BOOL	bInterruptEnable;

	if(g_pCRM == NULL)
	{
		// Map peripheral physical address to virtual address
		g_pCRM = (PCSP_CRM_REGS)OALPAtoUA(CSP_BASE_REG_PA_CRM);
	}

    // Check if we are already at the required state 
    if ((UINT32) EXTREG32(&g_pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)], CRM_CGR_MASK(index), CRM_CGR_SHIFT(index)) == mode)
    {
        return TRUE;
        //return;
    }

	bInterruptEnable = INTERRUPTS_ENABLE(FALSE);		// disable interrupt
    INSREG32(&g_pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)], CRM_CGR_MASK(index), CRM_CGR_VAL(index, mode));
	INTERRUPTS_ENABLE(bInterruptEnable);					// restore interrupt

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPNAND_ConfigIOMUX
//
//  This functions config certain pin for nfc use.  
//
//  Parameters:
//      CsNum
//          [in] - how many cs are used defines how this function works.  
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPNAND_ConfigIOMUX(DWORD CsNum)
{
    UNREFERENCED_PARAMETER(CsNum);
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

	return	OALPAtoUA(PhyAddr);
}


//-----------------------------------------------------------------------------
//
// CS&ZHL MAY-13-2011: provide Init / Read / Write / Erase 
//
//-----------------------------------------------------------------------------
BOOL OALFMD_Access(VOID* pInpBuffer, UINT32 inpSize)
{
	PFmdAccessInfo	pInfo = NULL;
	BOOL					bRet = FALSE;
	DWORD					dwStatus;

	//OALMSG(1, (L"->OALFMD_Access\r\n"));

	if((pInpBuffer == NULL) || (inpSize != sizeof(FmdAccessInfo)))
		return FALSE;

	pInfo = (PFmdAccessInfo)pInpBuffer;

	switch(pInfo->dwAccessCode)
	{
		case FMD_ACCESS_CODE_HWINIT:
			//OALMSG(1, (L"->OALFMD_Access->FMD_ACCESS_CODE_HWINIT\r\n"));
			if(!g_bNandfmdInitialized)
			{
				NFCSetClock(TRUE);		//enable nfc clock

				// init NandFlash
				//OALMSG(1, (L"->OALFMD_Access->FMD_Init(...)\r\n"));
				bRet = (BOOL)FMD_Init(NULL, NULL, NULL);
				if(!bRet)
				{
					RETAILMSG(1, (TEXT("OALFMD_Access: Init failed\r\n")));
					NFCSetClock(FALSE);		//stop clock to NFC
					break;
				}

				// get NandFlash Info
				//OALMSG(1, (L"->OALFMD_Access->FMD_GetInfo(...)\r\n"));
				bRet = FMD_GetInfo(&g_OalFlashInfo);
				if(!bRet)
				{
					RETAILMSG(1, (TEXT("OALFMD_Access: Get Flash Info failed\r\n")));
					NFCSetClock(FALSE);		//stop clock to NFC
					break;
				}

				g_bNandfmdInitialized = TRUE;
			}
			else
			{
				//OALMSG(1, (L"->OALFMD_Access->FMD_ACCESS_CODE_HWINIT done already\r\n"));
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

		default:
			RETAILMSG(1, (TEXT("OALFMD_Access: not support access code = %d.\r\n"), pInfo->dwAccessCode));
	}
	
	return bRet;
}
#endif		//NAND_PDD
