//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  prp_io.c
//
//  This module provides wrapper functions for accessing
//  prpclass.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"


#include "IpuBuffer.h"
#include "IPU_base.h"
#include "IPU_common.h"
#include "tpm.h"
#include "dp.h"
#include "prp.h"
#include "prpclass.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PrPOpenHandle
//
// This method creates a handle to the PrP stream driver.
//
// Parameters:
//      None
//
// Returns:
//      Handle to PrP driver, which is set in this method.
//      Returns NULL if failure.
//
//------------------------------------------------------------------------------
HANDLE PrPOpenHandle(void)
{
    PrpClass* pPrp = new PrpClass();

    return (HANDLE)pPrp;
}


//------------------------------------------------------------------------------
//
// Function: PrPCloseHandle
//
// This method closes a handle to the PrP stream driver.
//
// Parameters:
//      hDeviceContext
//          [in/out] Handle to close.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrPCloseHandle(HANDLE hDeviceContext)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    
    // if we don't have handle to PrP driver
    if (pPrp != NULL)
    {
        delete pPrp;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PrPBufferStatus
//
// This method gets the first module status of Pre-processor processing chain.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
//      pStatus
//          [out] Pointer to status.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrPBufferStatus(HANDLE hDeviceContext, icBufferStatus * pStatus)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    BOOL bRet = FALSE;
    bRet = pPrp->PrpBufferStatus(pStatus);
    return bRet;
}

//------------------------------------------------------------------------------
//
// Function: PrPConfigure
//
// This method configures the Pre-processor using parameters passed
// in by the caller.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
//      pPrpConfigData
//          [in] Pointer to PrP configuration data structure.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrPConfigure(HANDLE hDeviceContext, pPrpConfigData pConfigData)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    BOOL bRet = FALSE;
    bRet = pPrp->PrpConfigure(pConfigData);

    return bRet;

}


//------------------------------------------------------------------------------
//
// Function: PrPStart
//
// This method starts the Pre-processor task.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PrPStart(HANDLE hDeviceContext)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;

    pPrp->PrpStartChannel();

    return;
}

//------------------------------------------------------------------------------
//
// Function: PrPStop
//
// This method stops the Pre-processor.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void PrPStop(HANDLE hDeviceContext)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    
    pPrp->PrpStopChannel();

    return;

}


//------------------------------------------------------------------------------
//
// Function: PrPAddInputBuffer
//
// This method allows the caller to add input buffer.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
//      PhysicalBuf
//          [in] the address of input buffer.
//
//      bWait
//          [in]   TRUE:  The function won't return until the first module finished processing.
//                  FALSE:The function returns immediately once the buffer is filled.
//
//      IntType
//          [in] determine which interrupt should be enabled, 
//               only FIRSTMODULE_INTERRUPT available yet
//               FIRSTMODULE_INTERRUPT: first module of the whole chain
//               FRAME_INTERRUPT:           last module of the whole chain
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrPAddInputBuffer(HANDLE hDeviceContext, UINT32 PhysicalBuf, BOOL bWait, UINT8 IntType)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    BOOL bRet = FALSE;
    
    bRet = pPrp->PrpAddInputBuffer((UINT32 *)PhysicalBuf, bWait, IntType);

    return bRet;

}

//------------------------------------------------------------------------------
//
// Function: PrPAddInputCombBuffer
//
// This method allows the caller to add input the second buffer for combine.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
//      PhysicalBuf
//          [in] the address of input buffer for combine.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrPAddInputCombBuffer(HANDLE hDeviceContext, UINT32 PhysicalBuf)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    BOOL bRet = FALSE;
    
    bRet = pPrp->PrpAddInputCombBuffer((UINT32 *)PhysicalBuf);

    return bRet;

}

//------------------------------------------------------------------------------
//
// Function: PrPIsBusy
//
// This method allows the caller to query the status of prp module
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
// Returns:
//      TRUE if busy.
//      FALSE if idle.
//
//------------------------------------------------------------------------------
BOOL PrPIsBusy(HANDLE hDeviceContext)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    
    if(pPrp->PrpIsBusy())
        return TRUE;
    if(pPrp->IRTIsBusy())
        return TRUE;

    return FALSE;
}


//------------------------------------------------------------------------------
//
// Function: PrPClearBuffers
//
// This method allows the caller to clear the input and output buffer 
// queues in the Post-processing driver.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to PrP driver.
//
// Returns:
//      TRUE if success.
//      FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL PrPClearBuffers(HANDLE hDeviceContext)
{
    PrpClass * pPrp = (PrpClass*) hDeviceContext;
    BOOL bRet = FALSE;
    
    bRet = pPrp->PrpClearBuffers();

    return bRet;
}


