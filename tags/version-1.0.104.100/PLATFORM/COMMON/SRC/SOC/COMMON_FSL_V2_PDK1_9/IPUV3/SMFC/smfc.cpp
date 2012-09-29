//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  smfc.cpp
//
//  IPUv3 Sensor Multi FIFO Controller functions
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "idmac.h"
#include "cm.h"
#include "cpmem.h"
#include "smfc.h"

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
static HANDLE g_hIPUBase = NULL;
PCSP_IPU_SMFC_REGS g_pIPUV3_SMFC;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: SMFCRegsInit
//
// This function initializes the structures needed to access
// the IPUv3 Sensor Multi FIFO Controller.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL SMFCRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    if (g_pIPUV3_SMFC == NULL)
    {
        //  *** Use IPU_BASE driver to retrieve IPU Base Address ***

        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        dwIPUBaseAddr = IPUV3BaseGetBaseAddr(g_hIPUBase);
        if (dwIPUBaseAddr == -1)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        // Map CPMEM memory region entries
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_SMFC_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_SMFC = (PCSP_IPU_SMFC_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_SMFC_REGS), 
                        FALSE); 

        // Check if virtual mapping failed
        if (g_pIPUV3_SMFC == NULL)
        {
            ERRORMSG(TRUE,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
        memset(g_pIPUV3_SMFC,0,sizeof(PCSP_IPU_SMFC_REGS));
    }

    rc = TRUE;

cleanUp:

    // If initialization failed, be sure to clean up
    if (!rc) SMFCRegsCleanup();

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  SMFCRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 Sensor Multi FIFO Controller.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void SMFCRegsCleanup(void)
{
    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        ERRORMSG(TRUE,
            (_T("DI Cleanup: Failed closing IPU Base handle!\r\n")));
    }
    
    // Unmap peripheral registers
    if (g_pIPUV3_SMFC)
    {
        MmUnmapIoSpace(g_pIPUV3_SMFC, sizeof(PCSP_IPU_SMFC_REGS));
        g_pIPUV3_SMFC = NULL;
    }
}


//-----------------------------------------------------------------------------
//
// Function: SMFCRegsEnable
//
// Enable the SMFC.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SMFCRegsEnable(void)
{
    // Call to IPU Base to turn on SMFC_EN in IPU_CONF reg.
    // This will also turn on IPU clocks if no other IPU
    // modules have already turned them on.
    if(!IPUV3EnableModule(g_hIPUBase, IPU_SUBMODULE_SMFC, IPU_DRIVER_OTHER))
    {
        ERRORMSG(TRUE,
            (TEXT("%s: Failed to enable SMFC!\r\n"), __WFUNCTION__));
    }

}

//-----------------------------------------------------------------------------
//
// Function: SMFCRegsDisable
//
// Disable the SMFC.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void SMFCRegsDisable(void)
{
    if(!IPUV3DisableModule(g_hIPUBase, IPU_SUBMODULE_SMFC, IPU_DRIVER_OTHER))
    {
        ERRORMSG (TRUE,
            (TEXT("%s: Failed to disable SMFC!\r\n"), __WFUNCTION__));
    }
}

//-----------------------------------------------------------------------------
//
// Function: SMFCSetCSIMap
//
// This function map CSI frames to SMFC IDMAC channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel number:0~3.
//      csi_sel
//          [in] The selected CSI module:CSI0,CSI1.
//      csi_FrameId
//          [in] The CSI frame ID:0~3.CSI moduel(Two interface: CSI0,CSI1) can 
//               handle up four stream of data(four input sensor at most).
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL SMFCSetCSIMap(DWORD dwChannel,CSI_SELECT csi_sel,CSI_SELECT_FRAME csi_FrameId)
{
    UINT8 iMapId;

    if(csi_sel == CSI_SELECT_CSI0)
        iMapId = (UINT8)csi_FrameId;//0~3 for CSI0
    else
        iMapId = (UINT8)csi_FrameId + 4;//4~7 for CSI1
    
    switch (dwChannel)
    {
        case IDMAC_CH_SMFC_CH0:
        
            // Set up SMFC_MAP:channel0
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_MAP, 
                       IPU_SMFC_MAP_CH0, iMapId);  
            break;
            
        case IDMAC_CH_SMFC_CH1:
             
            // Set up SMFC_MAP:channel1
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_MAP, 
                       IPU_SMFC_MAP_CH1, iMapId);                 
            break;

        case IDMAC_CH_SMFC_CH2:
            
            // Set up SMFC_MAP:channel2
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_MAP, 
                       IPU_SMFC_MAP_CH2, iMapId);    
            break;            

        case IDMAC_CH_SMFC_CH3:
           
            // Set up SMFC_MAP:channel3
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_MAP, 
                       IPU_SMFC_MAP_CH3, iMapId);    
            break;

        default:
            ERRORMSG (TRUE,
            (TEXT("%s: Error channel for SMFC_MAP!\r\n"), __WFUNCTION__));
            break;
     }
 
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: SMFCSetBurstSize
//
// This function set Burst Size to SMFC IDMAC channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel number:0~3.
//      iBpp
//          [in] The BPP parameters in the IDMAC's CPMEM:BitsPerPixel.  
//      iPFS
//          [in] The PFS parameters in the IDMAC's CPMEM:PixelFormat. 
//      iNPB
//          [in] The NPB parameters in the IDMAC's CPMEM:PixelBurst. 
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL SMFCSetBurstSize(DWORD dwChannel,UINT8 iBpp, UINT8 iPFS, UINT8 iNPB)
{
    UINT8 iBurstSize;
    
    if (iPFS == IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC)//PFS
    {
        if((iBpp == CPMEM_BPP_16)||(iBpp == CPMEM_BPP_8))//BPP
            iBurstSize = (iNPB & 0x70) >> 4;//NPB[6:4]
        else
            iBurstSize = (iNPB & 0x7c) >> 2;//NPB[6:2]
    }
    else
        iBurstSize = (iNPB & 0x7c) >> 2;//NPB[6:2]
        
    switch (dwChannel)
    {
        case IDMAC_CH_SMFC_CH0:
        
            // Set up SMFC_BS:CSI0-channel0
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_BS, 
                       IPU_SMFC_BS_CH0, iBurstSize);    
            break;
            
        case IDMAC_CH_SMFC_CH1:
             
             // Set up SMFC_BS:CSI0-channel1
             INSREG32BF(&g_pIPUV3_SMFC->SMFC_BS, 
                        IPU_SMFC_BS_CH1, iBurstSize);                 
             break;

        case IDMAC_CH_SMFC_CH2:
            
            // Set up SMFC_BS:CSI0-channel2
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_BS, 
                       IPU_SMFC_BS_CH2, iBurstSize);    
            break;            

        case IDMAC_CH_SMFC_CH3:
           
           // Set up SMFC_BS:CSI0-channel3
           INSREG32BF(&g_pIPUV3_SMFC->SMFC_BS, 
                      IPU_SMFC_BS_CH3, iBurstSize);    
           break;

        default:
            ERRORMSG (TRUE,
            (TEXT("%s: Error channel for SMFC_MAP!\r\n"), __WFUNCTION__));
            break;
     }
 
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SMFCSetWMC
//
// This function set watermark level to SMFC IDMAC channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel number:0~3.
//      iLevelSize
//          [in] The watermark size level:0~7
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL SMFCSetWMC(DWORD dwChannel,UINT8 iLevelSize)
{    
    switch (dwChannel)
    {
        case IDMAC_CH_SMFC_CH0:
        
            // Set up SMFC_WMC:channel0
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                       IPU_SMFC_WMC_SET_CH0, iLevelSize);  
            break;
            
        case IDMAC_CH_SMFC_CH1:
             
            // Set up SMFC_WMC:channel1
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                       IPU_SMFC_WMC_SET_CH1, iLevelSize);                
            break;

        case IDMAC_CH_SMFC_CH2:
            
            // Set up SMFC_WMC:channel2
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                       IPU_SMFC_WMC_SET_CH2, iLevelSize);    
            break;            

        case IDMAC_CH_SMFC_CH3:
           
            // Set up SMFC_WMC:channel3
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                      IPU_SMFC_WMC_SET_CH3, iLevelSize);    
            break;

        default:
            ERRORMSG (TRUE,
            (TEXT("%s: Error channel for SMFC_MAP!\r\n"), __WFUNCTION__));
            break;
     }
 
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SMFCClearWMC
//
// This function clear watermark level to SMFC IDMAC channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel number:0~3.
//      iLevelSize
//          [in] The watermark size level:0~7
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL SMFCClearWMC(DWORD dwChannel,UINT8 iLevelSize)
{    
    switch (dwChannel)
    {
        case IDMAC_CH_SMFC_CH0:
        
            // Set up SMFC_WMC:channel0
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                       IPU_SMFC_WMC_CLR_CH0, iLevelSize);  
            break;
            
        case IDMAC_CH_SMFC_CH1:
             
            // Set up SMFC_WMC:channel1
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                       IPU_SMFC_WMC_CLR_CH1, iLevelSize);                 
            break;

        case IDMAC_CH_SMFC_CH2:
            
            // Set up SMFC_WMC:channel2
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                       IPU_SMFC_WMC_CLR_CH2, iLevelSize);    
            break;            

        case IDMAC_CH_SMFC_CH3:
           
            // Set up SMFC_WMC:channel3
            INSREG32BF(&g_pIPUV3_SMFC->SMFC_WMC, 
                       IPU_SMFC_WMC_CLR_CH3, iLevelSize);    
            break;

        default:
            ERRORMSG (TRUE,
            (TEXT("%s: Error channel for SMFC_MAP!\r\n"), __WFUNCTION__));
            break;
     }
 
    return TRUE;
}

void SMFCDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_SMFC->SMFC_MAP;
    RETAILMSG (1, (TEXT("\n\nSMFC Registers\r\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_SMFC_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\r\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}
