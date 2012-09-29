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
extern VOID* BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size);
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

extern PDDK_CLK_CONFIG			g_pDdkClkConfig;
extern PVOID                    pv_HWregCLKCTRL;


//-----------------------------------------------------------------------------
//
// Function: ClockUpdateRoot
//
// Updates references to the root clock of a specified clock node.
//
// Parameters:
//      index
//           [in] Index of clock node for which the root reference will be
//                updated.
//
//      root
//           [in] Root for the specified clock node.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID NFCClockUpdateRoot(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_BAUD_SOURCE root)
{
    DDK_CLOCK_BAUD_SOURCE oldRoot;
    UINT32 nodeRefCnt;
    oldRoot = g_pDdkClkConfig->root[index];
    nodeRefCnt = g_pDdkClkConfig->clockRefCount[index];
    
    // Check if clock node has open references.
    if (nodeRefCnt != 0)
    {
        // Reduce old root clock references using the amount of 
        // open references for the specified clock node.  Make
        // sure ref count does not go negative.
        if (g_pDdkClkConfig->rootRefCount[oldRoot] >= nodeRefCnt)
        {
            g_pDdkClkConfig->rootRefCount[oldRoot] -= nodeRefCnt;
        }
        else
        {
            ERRORMSG(TRUE, (_T("ClockUpdateRoot:  Node references exceed root references.\r\n")));
            g_pDdkClkConfig->rootRefCount[oldRoot] = 0;
        }
        
        // Transfer clock references to new clock root.
        g_pDdkClkConfig->rootRefCount[root] += nodeRefCnt;
    }

    g_pDdkClkConfig->root[index] = root;
}

//-----------------------------------------------------------------------------
//
//  Function: ClockRootEnable
//
//  This function enables the specified root clock.
//
//  Parameters:
//      root
//           [in] Index for referencing the root clock
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID NFCClockRootGating(DDK_CLOCK_BAUD_SOURCE root, BOOL bClkGate)
{
    switch(root)
    {

    case DDK_CLOCK_BAUD_SOURCE_PLL1: 
        if(bClkGate) 
        {
            HW_CLKCTRL_PLL1CTRL0_CLR(BM_CLKCTRL_PLL1CTRL0_POWER);
            //RETAILMSG(1, (_T("PLL1 Disabled!\r\n")));
        }
        else
        {
            HW_CLKCTRL_PLL1CTRL0_SET(BM_CLKCTRL_PLL1CTRL0_POWER);
            // Sleep(1);     // Powerup/PowerDown will call this function and cause low priority error. omment this out 
            //RETAILMSG(1, (_T("PLL1 Enabled!\r\n")));
        }        
        break;

    case DDK_CLOCK_BAUD_SOURCE_PLL2: 
        if(bClkGate) 
        {
            HW_CLKCTRL_PLL2CTRL0_SET(BM_CLKCTRL_PLL2CTRL0_CLKGATE);
            HW_CLKCTRL_PLL2CTRL0_CLR(BM_CLKCTRL_PLL2CTRL0_POWER);
            //RETAILMSG(1, (_T("PLL2 Disabled!\r\n")));
        }
        else
        {       
            HW_CLKCTRL_PLL2CTRL0_SET(BM_CLKCTRL_PLL2CTRL0_POWER);
            // Sleep(1);     // Powerup/PowerDown will call this function and cause low priority error. omment this out 
            HW_CLKCTRL_PLL2CTRL0_CLR(BM_CLKCTRL_PLL2CTRL0_CLKGATE);
            //RETAILMSG(1, (_T("PLL2 Enabled!\r\n")));
        }        
        break;
       
    case DDK_CLOCK_BAUD_SOURCE_REF_IO0: 
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEIO0);
            //RETAILMSG(1, (_T("REF_IO0 Disabled!\r\n")));
        }
        else
        {
            HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO0);
            //RETAILMSG(1, (_T("REF_IO0 Enabled!\r\n")));
        }        
        break;
        
    case DDK_CLOCK_BAUD_SOURCE_REF_IO1: 
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC0_SET(BM_CLKCTRL_FRAC0_CLKGATEIO1);
            //RETAILMSG(1, (_T("REF_IO1 Disabled!\r\n")));
        }
        else
        {
            HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATEIO1);
            //RETAILMSG(1, (_T("REF_IO1 Enabled!\r\n")));
        }        
        break;
             
    case DDK_CLOCK_BAUD_SOURCE_REF_GPMI:
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEGPMI);
            //RETAILMSG(1, (_T("REF_GPMI Disabled!\r\n")));            
        }
        else
        {
            HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEGPMI);
            //RETAILMSG(1, (_T("REF_GPMI Enabled!\r\n")));            
        }        
        break;
        
    case DDK_CLOCK_BAUD_SOURCE_REF_HSADC:
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEHSADC);
            //RETAILMSG(1, (_T("REF_HSADC Disabled!\r\n")));            
        }
        else
        {
            HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEHSADC);
            //RETAILMSG(1, (_T("REF_HSADC Enabled!\r\n")));            
        }        
        break;

    case DDK_CLOCK_BAUD_SOURCE_REF_PIX:         
        if(bClkGate) 
        {
            HW_CLKCTRL_FRAC1_SET(BM_CLKCTRL_FRAC1_CLKGATEPIX);
            //RETAILMSG(1, (_T("REF_PIX Disabled!\r\n")));            
        }    
        else
        {
            HW_CLKCTRL_FRAC1_CLR(BM_CLKCTRL_FRAC1_CLKGATEPIX);
            //RETAILMSG(1, (_T("REF_PIX Enabled!\r\n")));             
        }
        break; 

    }
    
}

//-----------------------------------------------------------------------------
//
// Function: ClockSetGatingMode
//
// Sets the clock gating mode of the peripheral.
//
// Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      bClkGate
//           [in] Requested clock gating mode for the peripheral.
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise 
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL NFCClockSetGating(DDK_CLOCK_GATE_INDEX index, BOOL bClkGate)
{
    BOOL rc = TRUE;
    switch(index)
    {
        case DDK_CLOCK_GATE_UART_CLK: 
            if(bClkGate) 
            {
                HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_UART_CLK_GATE);
            }
            else
            {
                HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_UART_CLK_GATE);
            }
            break;
            
        case DDK_CLOCK_GATE_PWM24M_CLK:   
            if(bClkGate) 
            {
                HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
            }
            else
            {
                HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_PWM_CLK24M_GATE);
            }
            break;
            
        case DDK_CLOCK_GATE_TIMROT32K_CLK:
            if(bClkGate) 
            {
                HW_CLKCTRL_XTAL_SET(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
            }
            else
            {
                HW_CLKCTRL_XTAL_CLR(BM_CLKCTRL_XTAL_TIMROT_CLK32K_GATE);
            }
            break;

        case DDK_CLOCK_GATE_UTMI0_CLK480M_CLK:
            if( bClkGate )
            {
                HW_CLKCTRL_PLL0CTRL0_CLR(BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS);
            }
            else
            {
                HW_CLKCTRL_PLL0CTRL0_SET(BM_CLKCTRL_PLL0CTRL0_EN_USB_CLKS);
            }
            break;

        case DDK_CLOCK_GATE_UTMI1_CLK480M_CLK:
            if( bClkGate )
            {
                HW_CLKCTRL_PLL1CTRL0_CLR(BM_CLKCTRL_PLL1CTRL0_EN_USB_CLKS);
            }
            else
            {
                HW_CLKCTRL_PLL1CTRL0_SET(BM_CLKCTRL_PLL1CTRL0_EN_USB_CLKS);
            }
            break;

        case DDK_CLOCK_GATE_HSADC_CLK:
           //Do nothing here
            break;
    
        case DDK_CLOCK_GATE_SSP0_CLK:  
            HW_CLKCTRL_SSP0.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SSP1_CLK:  
            HW_CLKCTRL_SSP1.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SSP2_CLK:  
            HW_CLKCTRL_SSP2.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SSP3_CLK:  
            HW_CLKCTRL_SSP3.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_GPMI_CLK :  
            HW_CLKCTRL_GPMI.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SPDIF_CLK:  
            HW_CLKCTRL_SPDIF.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SAIF0_CLK:       
            HW_CLKCTRL_SAIF0.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_SAIF1_CLK:       
            HW_CLKCTRL_SAIF1.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_DIS_LCDIF_CLK  :        
            HW_CLKCTRL_DIS_LCDIF.B.CLKGATE = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_ETM_CLK:  
            HW_CLKCTRL_ETM.B.CLKGATE = bClkGate ? 1 : 0;
            break;   
            
        case DDK_CLOCK_GATE_ENET_CLK  :  
            HW_CLKCTRL_ENET.B.SLEEP = bClkGate ? 1 : 0;
            break;
            
        case DDK_CLOCK_GATE_FLEXCAN0_CLK  :  
            HW_CLKCTRL_FLEXCAN.B.STOP_CAN0 = bClkGate ? 1 : 0;
            break;   
            
        case DDK_CLOCK_GATE_FLEXCAN1_CLK  :  
           HW_CLKCTRL_FLEXCAN.B.STOP_CAN1 = bClkGate ? 1 : 0;
           break; 
           
        default:
            rc = FALSE;
            break; 
    }
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: DDKClockConfigBaud
//
// Configures the input source clock and dividers for the specified
// CCM serial peripheral baud clock output.
//
// Parameters:
//      sig
//          [in] Clock signal to configure.
//
//      src
//          [in] Selects the input clock source.
//
//      u32Div
//          [in] Specifies the value programmed into the baud clock divider.
//
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL NFCClockConfigBaud( UINT32 u32Div )
{
    BOOL rc = TRUE;
    BOOL bclkgate = FALSE;
	UINT32 srcFreq;

    // Check divider range
    if(u32Div == 0)
    {
        rc = FALSE;
        ERRORMSG(1, (_T("DDKClockConfigBaud: Divide by zero\r\n")));            
        return rc;
    }

	// Return if busy with another divider change.
    // Only change DIV_FRAC_EN and DIV when CLKGATE = 0        
    if(HW_CLKCTRL_GPMI.B.BUSY )
    {
        rc = FALSE;
		return rc;
    }
    // Change Clock source
    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);

	NFCClockUpdateRoot(DDK_CLOCK_GATE_GPMI_CLK, DDK_CLOCK_BAUD_SOURCE_REF_GPMI );   
    // Only change DIV_FRAC_EN and DIV when CLKGATE = 0
    if((HW_CLKCTRL_GPMI_RD() & BM_CLKCTRL_GPMI_CLKGATE) != 0)
    {
        HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_ETM_CLKGATE); 
        bclkgate = TRUE;
    }
    // Change divider
    // Always use integer divide for SSP clock
    HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_DIV_FRAC_EN);
    // Set divider
    HW_CLKCTRL_GPMI.B.DIV = u32Div;
    //waiting Clock transfering to new divider
    //while(HW_CLKCTRL_GPMI.B.BUSY);
    //restore clock gating states
    if(bclkgate)
    {
        HW_CLKCTRL_GPMI_SET(BM_CLKCTRL_GPMI_CLKGATE);
    }

    if(rc)
    {
		//BSPClockUpdateFreq(index, src, u32Div);
        srcFreq = g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_GPMI];
		g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_GPMI] = srcFreq / (u32Div);
    }
    else
    {
        RETAILMSG(1, (_T(" NFCClockConfigBaud ERROR, clock ERROR \r\n") ));        
    } 
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: DDKClockSetGatingMode
//
// Sets the clock gating mode of the peripheral.
//
// Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      bClkGate
//           [in] Requested clock gating mode for the peripheral.
//           TRUE:   Clock Gating
//           FALSE:  Clock Enable
//
// Returns:
//      Returns TRUE if the clock gating mode was set successfully, otherwise 
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL NFCClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, BOOL bClkGate)
{
    DDK_CLOCK_BAUD_SOURCE root;

    if (bClkGate == TRUE)
    {
        switch (g_pDdkClkConfig->clockRefCount[index])
        {
            case 0:
                break;
    
            case 1:
                // Remove reference to root clock
                root = g_pDdkClkConfig->root[index];
                switch(g_pDdkClkConfig->rootRefCount[root])
                {
                case 0:
                    break;
    
                case 1:
                    NFCClockRootGating(root, TRUE);
                    // Fall through
    
                default:
                    --g_pDdkClkConfig->rootRefCount[root];
                    break;
                }
                            
                // Disable the clock
                NFCClockSetGating(index, TRUE);
    
                // Fall through
                
            default:
                --g_pDdkClkConfig->clockRefCount[index];
                break;
        }

    
    }
    else
    {        
  
        switch (g_pDdkClkConfig->clockRefCount[index])
        {
        case 0:
            // Add reference to root clock
            root = g_pDdkClkConfig->root[index];
            switch(g_pDdkClkConfig->rootRefCount[root])
            {
            case 0:
                NFCClockRootGating(root, FALSE);
                // Fall through

            default:
                ++g_pDdkClkConfig->rootRefCount[root];
                break;
            }

            // Enable the clock
            NFCClockSetGating(index, FALSE);   
            
            // Fall through
            
        default:
            // Reference count cannot be incremented since some drivers
            // will enable clock multiple times but only disable once.  Force
            // reference count to 1 since clock nodes managed directly by drivers
            // should not be shared and accurate reference count is not
            // required.
            g_pDdkClkConfig->clockRefCount[index] = 1;
            break;
        }
    }
    
	return TRUE;

}

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
//
//  Function:  NFCSetClock
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
	UINT32 index = DDK_CLOCK_SIGNAL_REF_GPMI;		// in iMX287

	BOOL rc = TRUE;
    UINT32 frequency , rootfreq, u32Div;
    static BOOL bInit = FALSE;
	BOOL	bInterruptEnable;

	g_pDdkClkConfig = (PDDK_CLK_CONFIG) IMAGE_WINCE_DDKCLK_RAM_UA_START;
    pv_HWregCLKCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);

	bInterruptEnable = INTERRUPTS_ENABLE(FALSE);		// disable interrupt
	if(!bInit){
        // Bump GPMI_CLK frequency up to the maximum.
        frequency = MAX_GPMI_CLK_FREQUENCY_kHZ;
        //status = DDKClockSetGpmiClk(&frequency, TRUE);
        //DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_GPMI, &rootfreq);
		rootfreq = g_pDdkClkConfig->clockFreq[index];

        u32Div = rootfreq / (frequency*1000) + 1;
        if(u32Div != 0)
            //rc = DDKClockConfigBaud(DDK_CLOCK_SIGNAL_GPMI, DDK_CLOCK_BAUD_SOURCE_REF_GPMI, u32Div );
            rc = NFCClockConfigBaud( u32Div );
        if (rc != TRUE)
        {
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
	INTERRUPTS_ENABLE(bInterruptEnable);					// restore interrupt

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
	PFmdAccessInfo	        pInfo = NULL;
	BOOL					bRet = FALSE;
	DWORD					dwStatus;

	OALMSG(1, (L"->OALFMD_Access..\r\n"));

	if((pInpBuffer == NULL) || (inpSize != sizeof(FmdAccessInfo)))
		return FALSE;

	pInfo = (PFmdAccessInfo)pInpBuffer;

	switch(pInfo->dwAccessCode)
	{
		case FMD_ACCESS_CODE_HWINIT:
			OALMSG(1, (L"->OALFMD_Access->FMD_ACCESS_CODE_HWINIT\r\n"));
			if(!g_bNandfmdInitialized)
			{
				BSPNAND_SetClock(TRUE);		//enable nfc clock

				// init NandFlash
				OALMSG(1, (L"->OALFMD_Access->FMD_Init(...)\r\n"));
				bRet = (BOOL)FMD_Init(NULL, NULL, NULL);
				if(!bRet)
				{
					RETAILMSG(1, (TEXT("OALFMD_Access: Init failed\r\n")));
					BSPNAND_SetClock(FALSE);		//stop clock to NFC
					break;
				}

				// get NandFlash Info
				OALMSG(1, (L"->OALFMD_Access->FMD_GetInfo(...)\r\n"));
				bRet = FMD_GetInfo(&g_OalFlashInfo);
				if(!bRet)
				{
					RETAILMSG(1, (TEXT("OALFMD_Access: Get Flash Info failed\r\n")));
					BSPNAND_SetClock(FALSE);		//stop clock to NFC
					break;
				}

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

		default:
			RETAILMSG(1, (TEXT("OALFMD_Access: not support access code = %d.\r\n"), pInfo->dwAccessCode));
	}
	
	return bRet;
}

#endif		//NAND_PDD
