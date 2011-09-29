//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File:  mx35_asrc.cpp
//
// Provides mx35-specific configuration routines for
// the ASRC
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include "csp.h"
#include "asrc_base.h"

//-----------------------------------------------------------------------------
//
// Function:  ASRCGetBaseRegAddr
//
// This function returns the physical base address for the
// ASRC registers based on the device index requested.
//
// Returns:
//      Physical base address for ASRC registers, 
//
//-----------------------------------------------------------------------------

UINT32 AsrcGetBaseRegAddr(void)
{
    return CSP_BASE_REG_PA_ASRC;
}

//-----------------------------------------------------------------------------
//
// Function:  ASRCGetIRQ
//
// This function returns the IRQ number for the
// I2C based on the device index requested.

// Returns:
//      IRQ number for ASRC, 
//
//-----------------------------------------------------------------------------
UINT32 AsrcGetIRQ(void)
{
     return IRQ_ASRC;
}


//-----------------------------------------------------------------------------
//
// Function:  AsrcGetInputDMAReq
//
// This function returns the dma request  for the input pair
// based on the device index requested.
//
// Returns:
//      dma request 
//
//-----------------------------------------------------------------------------
UINT8 AsrcGetInputDMAReq(ASRC_PAIR_INDEX index)
{
    UINT8 req = 0;
    switch (index )
    {
        case ASRC_PAIR_A:
               req = DDK_DMA_REQ_ASRC_RXA;
                break;
        case ASRC_PAIR_B:
               req = DDK_DMA_REQ_ASRC_RXB;
                break;               
        case ASRC_PAIR_C:
               req = DDK_DMA_REQ_ASRC_RXC;
                break;
        default:
                break;
    }
    return req;
}

//-----------------------------------------------------------------------------
//
// Function:  AsrcGetOuputP2PDMAReq
//
// This function returns the dma request  for the output pair
// based on the device index requested.
//
// Returns:
//      dma request 
//
//-----------------------------------------------------------------------------

UINT8 AsrcGetOutputP2PDMAReq(ASRC_PAIR_INDEX index)
{
    UINT8 req = 0;
    switch (index )
    {
        case ASRC_PAIR_A:
               req = DDK_DMA_REQ_ASRC_TXA_2_ESAI_TX;
                break;
        case ASRC_PAIR_B:
               req = DDK_DMA_REQ_ASRC_TXB_2_ESAI_TX;
                break;               
        case ASRC_PAIR_C:
               req = DDK_DMA_REQ_ASRC_TXC_2_ESAI_TX;
                break;
        default:
                break;
    }
    return req;
}



//-----------------------------------------------------------------------------
//
// Function:  AsrcGetOuputDMAReq
//
// This function returns the dma request  for the output pair
// based on the device index requested.
//
// Returns:
//      dma request 
//
//-----------------------------------------------------------------------------

UINT8 AsrcGetOutputDMAReq(ASRC_PAIR_INDEX index)
{
    UINT8 req = 0;
    switch (index )
    {
        case ASRC_PAIR_A:
               req = DDK_DMA_REQ_ASRC_TXA;
                break;
        case ASRC_PAIR_B:
               req = DDK_DMA_REQ_ASRC_TXB;
                break;               
        case ASRC_PAIR_C:
               req = DDK_DMA_REQ_ASRC_TXC;
                break;
        default:
                break;
    }
    return req;
}

//-----------------------------------------------------------------------------
//
// Function:  AsrcGetInputDMAPriority
//
// This function returns the dma priority  for the input dma
// based on the device index requested.
//
// Returns:
//      dma priority 
//
//-----------------------------------------------------------------------------
UINT8 AsrcGetInputDMAPriority(ASRC_PAIR_INDEX index)
{
    UINT8 pri = 0;
    switch (index )
    {
        case ASRC_PAIR_A:
               pri = (SDMA_CHNPRI_CHNPRI_HIGHEST-3);
                break;
        case ASRC_PAIR_B:
               pri = (SDMA_CHNPRI_CHNPRI_HIGHEST-3);
                break;               
        case ASRC_PAIR_C:
               pri = (SDMA_CHNPRI_CHNPRI_HIGHEST-3);
                break;
        default:
                break;
    }
    return pri;
}

//-----------------------------------------------------------------------------
//
// Function:  AsrcGetOutputDMAPriority
//
// This function returns the dma priority  for the output dma
// based on the device index requested.
//
// Returns:
//      dma priority 
//
//-----------------------------------------------------------------------------

UINT8 AsrcGetOutputDMAPriority(ASRC_PAIR_INDEX index)
{
    UINT8 pri = 0;
    switch (index )
    {
        case ASRC_PAIR_A:
               pri = (SDMA_CHNPRI_CHNPRI_HIGHEST-3);
                break;
        case ASRC_PAIR_B:
               pri = (SDMA_CHNPRI_CHNPRI_HIGHEST-3);
                break;               
        case ASRC_PAIR_C:
               pri = (SDMA_CHNPRI_CHNPRI_HIGHEST-3);
                break;
        default:
                break;
    }
    return pri;
}

UINT32 AsrcGetSDMABaseIrq(void)
{
    return IRQ_SDMA_CH0;
}


BOOL AsrcEnableClock(void)
{
    //RETAILMSG(TRUE, (_T("ASRC Enable Clock\r\n ")));
    
    DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_ASRC, DDK_CLOCK_GATE_MODE_ENABLED_ALL);

    return TRUE;
}

UINT32 AsrcGetP2PInfo(PASRC_PAIR_CONTEXT pPairContext)
{
    UINT32 info=0;
    //PS: 8  PA: 9  SPDIF: 10
    
    //DP/SP SPBA: 11~12, HWE/LWC: 28:29, CONT:31
    info = (UINT32)(3<<11) | (3<<28)|(1<<31);

    //LWML :0~7
    info |= pPairContext->deviceWML;
    //HWML: 14:23
    info |= (pPairContext->outputWaterMark * pPairContext->pairChnNum) << 14;
    // N:24~27
    info |= (pPairContext->pairChnNum) << 24;

    return info;
}

UINT32 AsrcGetVersion(void)
{
    UINT32 dwRev = 0;
    
    KernelIoControl(IOCTL_HAL_QUERY_SI_VERSION, NULL, 0, 
        &dwRev, sizeof(dwRev), NULL);

    if(dwRev == DDK_SI_REV_TO2){
        return ASRC_VERSION_2;
    }else{
        return ASRC_VERSION_1;
    }
}
