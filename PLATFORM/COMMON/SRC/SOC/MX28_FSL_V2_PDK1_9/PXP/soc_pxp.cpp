//------------------------------------------------------------------------------
//
//  Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  soc_pxp.cpp
//
//  Provides SoC-specific configuration routines for
//  the pxp driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"
#include "common_pxp.h"
#include "common_regspxp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function:  CSPPXPGetBaseAddr
//
// This function returns the base address for the PXP module.
//
// Parameters:
//      None.
//
// Returns:
//      PXP Base address.
//
//-----------------------------------------------------------------------------
DWORD CSPPXPGetBaseAddr()
{
    return CSP_BASE_REG_PA_PXP;
}

//-----------------------------------------------------------------------------
//
// Function:  PXPGetIRQ
//
// This function returns the IRQ number for the PXP Interrupt.
//
// Parameters:
//      None.
//
// Returns:
//      PXP IRQ number.
//
//-----------------------------------------------------------------------------
DWORD PXPGetIRQ(void)
{
    return IRQ_PXP;
}

//-----------------------------------------------------------------------------
//
// Function: PXPIntrEnable
//
// This function is for enable or disable PXP interrupt.
//
// Parameters:
//      pPxp_registers
//          [in] Pointer to PXP double buffer registers address.
//      Enable
//          [in] TRUE if enable, FALSE if disable
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PXPIntrEnable(pPxp_registers pPxpRegsVirtAddr, BOOL Enable)
{
    ((hw_pxp_ctrl_t *)&pPxpRegsVirtAddr->ctrl)->B.IRQ_ENABLE = Enable;
    ((hw_pxp_ctrl_t *)&(pPxpRegsVirtAddr+1)->ctrl)->B.IRQ_ENABLE = Enable;
    ((hw_pxp_ctrl_t *)&pPxpRegsVirtAddr->ctrl)->B.NEXT_IRQ_ENABLE = Enable;
    ((hw_pxp_ctrl_t *)&(pPxpRegsVirtAddr+1)->ctrl)->B.NEXT_IRQ_ENABLE = Enable;    
}


//-----------------------------------------------------------------------------
//
// Function: PXPSetBlockSizeValue
//
// This function set block size value for PXP. Also according to block size, 
// relative exponent value will be returned.
//
// Parameters:
//      pPxpRegsVirtAddr
//          [in] Pointer to PXP double buffer registers address.
//
//      BlockSize
//          [in] Pointer to block size value
//
//      BlockExp
//          [out] Pointer to exponent part of block size
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PXPSetBlockSizeValue(pPxp_registers pPxpRegsVirtAddr, UINT32 *BlockSize, UINT32 *BlockExp)
{
    switch(*BlockSize)
    {
    case 8:
        ((hw_pxp_ctrl_t *)&pPxpRegsVirtAddr->ctrl)->B.BLOCK_SIZE = 0;
        ((hw_pxp_ctrl_t *)&(pPxpRegsVirtAddr+1)->ctrl)->B.BLOCK_SIZE = 0;
        *BlockExp = 3;
        break;
    case 16:        
        ((hw_pxp_ctrl_t *)&pPxpRegsVirtAddr->ctrl)->B.BLOCK_SIZE = 1;
        ((hw_pxp_ctrl_t *)&(pPxpRegsVirtAddr+1)->ctrl)->B.BLOCK_SIZE = 1;
        *BlockExp = 4;
        break;
    default:
        ((hw_pxp_ctrl_t *)&pPxpRegsVirtAddr->ctrl)->B.BLOCK_SIZE = 0;
        ((hw_pxp_ctrl_t *)&(pPxpRegsVirtAddr+1)->ctrl)->B.BLOCK_SIZE = 0;
        *BlockSize = 8;
        *BlockExp = 3;
        RETAILMSG(1,(_T("MX28 PXP doesn't support %d block size, will set 8 as default!\r\n"), *BlockSize));
    }        
}

//-----------------------------------------------------------------------------
//
// Function: PXPSetScaleFactor
//
// This function set PXP scale factor.
//
// Parameters:
//      pPxpRegsVirtAddr
//          [in] Pointer to PXP double buffer registers address.
//      XScale
//          [in] X scaling factor
//      YScale
//          [in] Y scaling factor
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PXPSetScaleFactor(pPxp_registers pPxpRegsVirtAddr,UINT32 XScale,UINT32 YScale)
{
    ((hw_pxp_s0scale_mx28_t *)&pPxpRegsVirtAddr->s0scale)->B.XSCALE = XScale;
    ((hw_pxp_s0scale_mx28_t *)&pPxpRegsVirtAddr->s0scale)->B.YSCALE = YScale;
}

//-----------------------------------------------------------------------------
//
// Function: PXPOperateComplete
//
// This function is called in PXP operation completion interrupt routine 
// while PXP is waiting for signal indicating next operation setting can be 
// loaded in normal operation routine.
//
// Parameters:
//      hPxpContinueEvent
//          [in] Signal event indicating next operation setting can be loaded
//               in normal operation routine
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PXPOperateComplete(HANDLE hPxpContinueEvent)
{
    UNREFERENCED_PARAMETER(hPxpContinueEvent);  //MX28 PXP should not set continue event here.
}

//-----------------------------------------------------------------------------
//
// Function: PXPLoadComplete
//
// This function is called in PXP registers load completion interrupt routine 
// while PXP is waiting for signal indicating next operation setting can be 
// loaded.
//
// Parameters:
//      hPxpContinueEvent
//          [in] Signal event indicating next operation setting can be loaded 
//               to continue normal operation routine.
//
//      bProcessPending
//          [in] Indicate whether PXP has next operation waiting for being set
//               in PXP for process in normal operation routine.
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PXPLoadComplete(HANDLE hPxpContinueEvent, BOOL bProcessPending)
{
    if(bProcessPending)    
        SetEvent(hPxpContinueEvent);
}

//-----------------------------------------------------------------------------
//
// Function: PXPWaitforLoadComplete
//
// This function is called in resume process routine to wait for signal 
// indicating next operation setting can be loaded.
// Parameters:
//      hPxpResumeLoadEvent
//          [in] Signal event indicating next operation setting can be loaded
//               in resume process routine
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PXPWaitforLoadComplete(HANDLE hPxpResumeLoadEvent)
{
    WaitForSingleObject(hPxpResumeLoadEvent,1);
}

//-----------------------------------------------------------------------------
//
// Function: PXPGetDownScaleFactorExp
//
// This function returns PXP down scaling factor exponent.
// Parameters:
//      pDownScaleExp
//          [in/out] Address of PXP down scaling factor exponent. 
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PXPGetDownScaleFactorExp(UINT32 *pDownScaleExp)
{
    *pDownScaleExp = 2;
}

