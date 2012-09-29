//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cm.cpp
//
//  IPU CSP-level Control Module (IPU Common) functions
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
#include "cm_priv.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "cm.h"


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
static PCSP_IPU_COMMON_REGS g_pIPUV3_COMMON;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: CMRegsInit
//
// This function allocates the data structures required for interaction
// with the IPUv3 CM registers.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL CMRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_COMMON == NULL)
    {
        //  *** Use IPU_BASE driver to retrieve IPU Base Address ***

        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        dwIPUBaseAddr = IPUV3BaseGetBaseAddr(g_hIPUBase);
        if (dwIPUBaseAddr == -1)
        {
            RETAILMSG (1,
                (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        //********************************************
        // Create pointer to IPU Common registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_IPU_COMMON_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_COMMON = (PCSP_IPU_COMMON_REGS) MmMapIoSpace(phyAddr,
                                    sizeof(CSP_IPU_COMMON_REGS), FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_COMMON == NULL)
        {
            DEBUGMSG(1,
                (_T("Init:  COMMON reg MmMapIoSpace failed!\r\n")));
             goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) CMRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  CMRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 CM registers.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void CMRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        RETAILMSG(1,
            (_T("DI Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    // Unmap peripheral registers
    if (g_pIPUV3_COMMON)
    {
        MmUnmapIoSpace(g_pIPUV3_COMMON, sizeof(CSP_IPU_COMMON_REGS));
        g_pIPUV3_COMMON = NULL;
    }

    IPU_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: CMSetProcFlow
//
// This function sets the IPU_PROC_FLOW registers.
//
// Parameters:
//      procFlowType
//          [in] Identifies the PROC_FLOW value to modify.
//
//      procFlowVal
//          [in] New value to program for PROC_FLOW specified
//          in procFlowID.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CMSetProcFlow(IPU_PROC_FLOW procFlowType, DWORD procFlowVal)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // We will use Interlocked API to synchronize access to 
    // IPU_FS_PROC_FLOW1, IPU_FS_PROC_FLOW2, IPU_FS_PROC_FLOW3 registers.

    switch (procFlowType)
    {
        case IPU_PROC_FLOW_PRPENC_ROT_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_PRPENC_ROT_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_PRPENC_ROT_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_ALT_ISP_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_ALT_ISP_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_ALT_ISP_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRPVF_ROT_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_PRPVF_ROT_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_PRPVF_ROT_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PP_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_PP_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_PP_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PP_ROT_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_PP_ROT_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_PP_ROT_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_ISP_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_ISP_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_ISP_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRP_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_PRP_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_PRP_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_ENC_IN_VALID:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_ENC_IN_VALID);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_ENC_IN_VALID, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_VF_IN_VALID:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_VF_IN_VALID);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_VF_IN_VALID, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_VDI_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW1_VDI_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW1_VDI_SRC_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRPENC_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PRP_ENC_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PRP_ENC_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRPVF_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PRPVF_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PRPVF_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRPVF_ROT_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PRPVF_ROT_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PRPVF_ROT_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PP_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PP_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PP_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PP_ROT_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PP_ROT_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PP_ROT_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRPENC_ROT_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PRPENC_ROT_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PRPENC_ROT_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRP_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PRP_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PRP_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_PRP_ALT_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW2_PRP_ALT_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW2_PRP_ALT_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_SMFC0_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW3 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW3_SMFC0_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW3_SMFC0_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_SMFC1_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW3 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW3_SMFC1_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW3_SMFC1_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_SMFC2_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW3 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW3_SMFC2_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW3_SMFC2_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_PROC_FLOW_SMFC3_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW3 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW3_SMFC3_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW3_SMFC3_DEST_SEL, procFlowVal);

            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3,
                        oldVal, newVal) != oldVal);
            break;
#ifdef IPUv3EX
        case IPU_PROC_FLOW_EXT_SRC1_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW3 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW3_EXT_SRC1_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW3_EXT_SRC1_DEST_SEL, procFlowVal);
        
            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3,
                        oldVal, newVal) != oldVal);
            break;1
        case IPU_PROC_FLOW_EXT_SRC2_DEST:
            // Compute bitmask and shifted bit value for IPU_FS_PROC_FLOW3 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_PROC_FLOW3_EXT_SRC2_DEST_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_PROC_FLOW3_EXT_SRC2_DEST_SEL, procFlowVal);
        
            // Use interlocked function to set proc flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_PROC_FLOW3,
                        oldVal, newVal) != oldVal);
            break;
#endif
    }
}

//------------------------------------------------------------------------------
//
// Function: CMSetDispFlow
//
// This function sets the IPU_DISP_FLOW registers.
//
// Parameters:
//      dispFlowType
//          [in] Identifies the DISP_FLOW value to modify.
//
//      dispFlowVal
//          [in] New value to program for DISP_FLOW specified
//          in dispFlowID.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CMSetDispFlow(IPU_DISP_FLOW dispFlowType, DWORD dispFlowVal)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    // We will use Interlocked API to synchronize access to 
    // IPU_FS_DISP_FLOW1 and IPU_FS_DISP_FLOW2 registers.

    switch (dispFlowType)
    {
        case IPU_DISP_FLOW_DP_SYNC0_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW1_DP_SYNC0_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW1_DP_SYNC0_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DP_SYNC1_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW1_DP_SYNC1_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW1_DP_SYNC1_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DP_ASYNC0_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW1_DP_ASYNC0_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW1_DP_ASYNC0_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DP_ASYNC1_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW1_DP_ASYNC1_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW1_DP_ASYNC1_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DC2_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW1_DC2_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW1_DC2_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DC1_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW1 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW1_DC1_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW1_DC1_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW1,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DP_ASYNC1_ALT_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW2_DP_ASYNC1_ALT_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW2_DP_ASYNC1_ALT_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DP_ASYNC0_ALT_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW2_DP_ASYNC0_ALT_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW2_DP_ASYNC0_ALT_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
        case IPU_DISP_FLOW_DC2_ALT_SRC:
            // Compute bitmask and shifted bit value for IPU_FS_DISP_FLOW2 register
            iMask = CSP_BITFMASK(IPU_IPU_FS_DISP_FLOW2_DC2_ALT_SRC_SEL);
            iBitval = CSP_BITFVAL(IPU_IPU_FS_DISP_FLOW2_DC2_ALT_SRC_SEL, dispFlowVal);

            // Use interlocked function to set disp flow bits.
            do
            {
                oldVal = INREG32(&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW2);
                newVal = (oldVal & (~iMask)) | iBitval;
            } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_FS_DISP_FLOW2,
                        oldVal, newVal) != oldVal);
            break;
    }
}
//------------------------------------------------------------------------------
//
// Function: CMResetIPUMemories
//
// This function resets internal IPU memories, based on flags
// passed in to function call.
//
// Parameters:
//      dwIPUMemoriesFlags
//          Flags that identify which IPU internal memories
//          should be reset.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void CMResetIPUMemories(DWORD dwIPUMemoriesFlags)
{
    // Set memories for reset
    OUTREG32(&g_pIPUV3_COMMON->IPU_MEM_RST, dwIPUMemoriesFlags);

    // Begin reset process
    INSREG32BF(&g_pIPUV3_COMMON->IPU_MEM_RST, IPU_IPU_MEM_RST_RST_MEM_START,
        IPU_IPU_MEM_RST_RST_MEM_START_RESET_MEMORY);

    // Wait for reset to complete (bit will clear)
    do
    {
        Sleep(1);
    } while (EXTREG32BF(&g_pIPUV3_COMMON->IPU_MEM_RST, IPU_IPU_MEM_RST_RST_MEM_START));
}

//------------------------------------------------------------------------------
//
// Function: CMSetMCU_T
//
// This function sets the MCU_T.
//
// Parameters:
//      mcu_t_val
//          Value for MCU_T.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void CMSetMCU_T(DWORD mcu_t_val)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_MCU_T);
    iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_MCU_T, mcu_t_val);
    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
        oldVal, newVal) != oldVal);
}

//------------------------------------------------------------------------------
//
// Function: CMSetPathIC2DP
//
// This function sets path of PRP viewfinder output channel to DP.
//
// Parameters:
//      bIsDMFC:
//         TRUE if use DMFC direct path, FALSE if use IDMAC normal path.
//
//      bIsSYNC
//         TRUE if is synchronous path, FALSE if is asynchronous path.
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void CMSetPathIC2DP(BOOL bIsDMFC, BOOL bIsSync)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    iMask = CSP_BITFMASK(IPU_IPU_CONF_IC_DMFC_SEL)
           |CSP_BITFMASK(IPU_IPU_CONF_IC_DMFC_SYNC);
    iBitval = CSP_BITFVAL(IPU_IPU_CONF_IC_DMFC_SEL, bIsDMFC)
             |CSP_BITFVAL(IPU_IPU_CONF_IC_DMFC_SYNC, bIsSync);
    
    // Use interlocked function to enable DMFC direct path.
    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_CONF,
                oldVal, newVal) != oldVal);

}


//-----------------------------------------------------------------------------
//
// Function:  CMGetProcTaskStatus
//
// This function retrieves the IPU processing task status.
//
// Parameters:
//      procTask
//          [in] IPU_PROC_TASK enumeration value to select the processing
//          task to query the status of.
//
// Returns:
//      DWORD value representing the processing task status
//
//      Possible values for all tasks except MEM2PRP are:
//          IPU_PROC_TASK_STAT_IDLE
//          IPU_PROC_TASK_STAT_ACTIVE
//          IPU_PROC_TASK_STAT_WAIT4READY
//      Possible values for the MEM2PRP task are:
//          IPU_PROC_TASK_STAT_IDLE
//          IPU_PROC_TASK_STAT_BOTH_ACTIVE
//          IPU_PROC_TASK_STAT_ENC_ACTIVE
//          IPU_PROC_TASK_STAT_VF_ACTIVE
//          IPU_PROC_TASK_STAT_BOTH_PAUSE
//-----------------------------------------------------------------------------
DWORD CMGetProcTaskStatus(IPU_PROC_TASK procTask)
{
    DWORD status;

    switch (procTask)
    {
        case IPU_PROC_TASK_ENC:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_ENC_TSTAT);
            break;
        case IPU_PROC_TASK_VF:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_VF_TSTAT);
            break;
        case IPU_PROC_TASK_PP:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_PP_TSTAT);
            break;
        case IPU_PROC_TASK_ENC_ROT:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_ENC_ROT_TSTAT);
            break;
        case IPU_PROC_TASK_VF_ROT:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_VF_ROT_TSTAT);
            break;
        case IPU_PROC_TASK_PP_ROT:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_PP_ROT_TSTAT);
            break;
        case IPU_PROC_TASK_MEM2PRP:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_MEM2PRP_TSTAT);
            break;
        case IPU_PROC_TASK_CSI2MEM_SMFC0:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC0_TSTAT);
            break;
        case IPU_PROC_TASK_CSI2MEM_SMFC1:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC1_TSTAT);
            break;
        case IPU_PROC_TASK_CSI2MEM_SMFC2:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC2_TSTAT);
            break;
        case IPU_PROC_TASK_CSI2MEM_SMFC3:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_PROC_TASKS_STAT,
                    IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC3_TSTAT);
            break;
        default:
            status = 0;
            break;
    }

    return status;

}

//-----------------------------------------------------------------------------
//
// Function:  CMGetDispTaskStatus
//
// This function retrieves the IPU display task status.
//
// Parameters:
//      dispTask
//          [in] IPU_DISP_TASK enumeration value to select the display
//          task to query the status of.
//
// Returns:
//      DWORD value representing the display task status
//
//      Possible values for DP_ASYNC and DC_ASYNCH2:
//          IPU_DISP_TASK_STAT_IDLE
//          IPU_DISP_TASK_STAT_PRIM_ACTIVE
//          IPU_DISP_TASK_STAT_ALT_ACTIVE
//          IPU_DISP_TASK_STAT_UPDATE_PARAM
//          IPU_DISP_TASK_STAT_PAUSE
//      Possible values for DC_ASYNC1:
//          IPU_DISP_TASK_STAT_IDLE
//          IPU_DISP_TASK_STAT_ACTIVE
//          IPU_DISP_TASK_STAT_WAIT4READY
//      Possible values for DC_ASYNC2_CUR_FLOW and DP_ASYNC_CUR_FLOW:
//          IPU_DISP_TASK_STAT_CUR_FLOW_ALTERNATE
//          IPU_DISP_TASK_STAT_CUR_FLOW_MAIN
//-----------------------------------------------------------------------------
DWORD CMGetDispTaskStatus(IPU_DISP_TASK dispTask)
{
    DWORD status;

    switch (dispTask)
    {
        case IPU_DISP_TASK_DP_ASYNC:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                    IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_TSTAT);
            break;
        case IPU_DISP_TASK_DP_ASYNC_CUR_FLOW:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                    IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_CUR_FLOW);
            break;
        case IPU_DISP_TASK_DC_ASYNC1:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                    IPU_IPU_DISP_TASKS_STAT_DC_ASYNC1_TSTAT);
            break;
        case IPU_DISP_TASK_DC_ASYNC2:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                    IPU_IPU_DISP_TASKS_STAT_DC_ASYNC2_TSTAT);
            break;
        case IPU_DISP_TASK_DC_ASYNC2_CUR_FLOW:
            status = EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                    IPU_IPU_DISP_TASKS_STAT_DC_ASYNC2_CUR_FLOW);
            break;
        default:
            status = 0;
            break;
    }

    return status;
}


//-----------------------------------------------------------------------------
//
// Function:  CMDPFlowClearIntStatus
//
// This function sets the right bit in the IPU_INT_STAT_15 register to clear 
// the interrupt status for the specified DP flow interrupt.
//
// Parameters:
//      IntrType
//          [in] Selects the type of interrupt to clear.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CMDPFlowClearIntStatus(DP_INTR_TYPE IntrType)
{
    UINT32 oldVal, newVal, iMask, iBitval;
    
    switch (IntrType)
    {
        case DP_INTR_TYPE_SF_START:
            iMask = CSP_BITFMASK(IPU_DP_SF_START);
            iBitval = CSP_BITFVAL(IPU_DP_SF_START, 1); // 1 to clear
            break;
        case DP_INTR_TYPE_SF_END:
            iMask = CSP_BITFMASK(IPU_DP_SF_END);
            iBitval = CSP_BITFVAL(IPU_DP_SF_END, 1); // 1 to clear
            break;
        case DP_INTR_TYPE_ASF_START:
            iMask = CSP_BITFMASK(IPU_DP_ASF_START);
            iBitval = CSP_BITFVAL(IPU_DP_ASF_START, 1); // 1 to clear
            break;
        case DP_INTR_TYPE_ASF_END:
            iMask = CSP_BITFMASK(IPU_DP_ASF_END);
            iBitval = CSP_BITFVAL(IPU_DP_ASF_END, 1); // 1 to clear
            break;
        case DP_INTR_TYPE_SF_BRAKE:
            iMask = CSP_BITFMASK(IPU_DP_SF_BRAKE);
            iBitval = CSP_BITFVAL(IPU_DP_SF_BRAKE, 1); // 1 to clear
            break;
        case DP_INTR_TYPE_ASF_BRAKE:
            iMask = CSP_BITFMASK(IPU_DP_SF_BRAKE);
            iBitval = CSP_BITFVAL(IPU_DP_SF_BRAKE, 1); // 1 to clear
            break;
        default:
            // Error - Unsupported DP Channel
            return;

    }

    // set IPU_INTR_STAT_15 with new value
    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_15);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_15,
            oldVal, newVal) != oldVal);

    return;
}


//-----------------------------------------------------------------------------
//
// Function:  CMDPFlowIntCntrl
//
// This function sets the right bit in the IPU_INT_CTRL_15 register to 
// enable/disable the interrupt control for the specified DP flow interrupt.
//
// Parameters:
//      IntrType
//          [in] Selects the type of interrupt to enable/disable.
//
//      enable
//          [in]   TRUE:  enable interrupt
//                  FALSE:disable interrupt
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CMDPFlowIntCntrl(DP_INTR_TYPE IntrType, BOOL enable)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    switch (IntrType)
    {
        case DP_INTR_TYPE_SF_START:
            iMask = CSP_BITFMASK(IPU_DP_SF_START);
            iBitval = CSP_BITFVAL(IPU_DP_SF_START, enable);
            break;
        case DP_INTR_TYPE_SF_END:
            iMask = CSP_BITFMASK(IPU_DP_SF_END);
            iBitval = CSP_BITFVAL(IPU_DP_SF_END, enable);
            break;
        case DP_INTR_TYPE_ASF_START:
            iMask = CSP_BITFMASK(IPU_DP_ASF_START);
            iBitval = CSP_BITFVAL(IPU_DP_ASF_START, enable);
            break;
        case DP_INTR_TYPE_ASF_END:
            iMask = CSP_BITFMASK(IPU_DP_ASF_END);
            iBitval = CSP_BITFVAL(IPU_DP_ASF_END, enable);
            break;
        case DP_INTR_TYPE_SF_BRAKE:
            iMask = CSP_BITFMASK(IPU_DP_SF_BRAKE);
            iBitval = CSP_BITFVAL(IPU_DP_SF_BRAKE, enable);
            break;
        case DP_INTR_TYPE_ASF_BRAKE:
            iMask = CSP_BITFMASK(IPU_DP_SF_BRAKE);
            iBitval = CSP_BITFVAL(IPU_DP_SF_BRAKE, enable);
            break;
        default:
            // Error - Unsupported DP Channel
            return;

    }

    // set IPU_INTR_CTRL_15 with new value
    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_15);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_15,
            oldVal, newVal) != oldVal);

    return;
}


//------------------------------------------------------------------------------
//
// Function: CMDCFrameCompleteClearIntStatus
//
// This function sets the bit in the IPU_INT_STAT_15 register to clear 
// the DC Frame Complete interrupt status for the specified DC channel.
//
// Parameters:
//      dcChan
//          [in] DC channel number whose interrupt status will be cleared.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CMDCFrameCompleteClearIntStatus(DC_CHANNEL dcChan)
{
    UINT32 oldVal, newVal, iMask, iBitval;
    
    switch (dcChan)
    {
        case DC_CHANNEL_0:
            iMask = CSP_BITFMASK(IPU_DC_FC_0);
            iBitval = CSP_BITFVAL(IPU_DC_FC_0, 1); // 1 to clear
            break;
        case DC_CHANNEL_1:
            iMask = CSP_BITFMASK(IPU_DC_FC_1);
            iBitval = CSP_BITFVAL(IPU_DC_FC_1, 1); // 1 to clear
            break;
        case DC_CHANNEL_2:
            iMask = CSP_BITFMASK(IPU_DC_FC_2);
            iBitval = CSP_BITFVAL(IPU_DC_FC_2, 1); // 1 to clear
            break;
        case DC_CHANNEL_3:
            iMask = CSP_BITFMASK(IPU_DC_FC_3);
            iBitval = CSP_BITFVAL(IPU_DC_FC_3, 1); // 1 to clear
            break;
        case DC_CHANNEL_4:
            iMask = CSP_BITFMASK(IPU_DC_FC_4);
            iBitval = CSP_BITFVAL(IPU_DC_FC_4, 1); // 1 to clear
            break;
        case DC_CHANNEL_6:
            iMask = CSP_BITFMASK(IPU_DC_FC_6);
            iBitval = CSP_BITFVAL(IPU_DC_FC_6, 1); // 1 to clear
            break;
        default:
            // Error - Unsupported DC Channel
            return;
    }

    // set IPU_INTR_STAT_15 with new value
    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_STAT_15);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_STAT_15,
            oldVal, newVal) != oldVal);

    return;
}


//------------------------------------------------------------------------------
//
// Function: CMDCFrameCompleteIntCntrl
//
// This function configures the bit in the IPU_INT_CTRL_15 register to 
// enable or disable the interrupt of specified DC channel.
//
// Parameters:
//      dcChan
//          [in] dc channel number to set interrupt control bit for
//
//      enable
//          [in] TRUE:  enable interrupt
//               FALSE: disable interrupt
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void CMDCFrameCompleteIntCntrl(DC_CHANNEL dcChan, BOOL enable)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    switch (dcChan)
    {
        case DC_CHANNEL_0:
            iMask = CSP_BITFMASK(IPU_DC_FC_0);
            iBitval = CSP_BITFVAL(IPU_DC_FC_0, enable);
            break;
        case DC_CHANNEL_1:
            iMask = CSP_BITFMASK(IPU_DC_FC_1);
            iBitval = CSP_BITFVAL(IPU_DC_FC_1, enable);
            break;
        case DC_CHANNEL_2:
            iMask = CSP_BITFMASK(IPU_DC_FC_2);
            iBitval = CSP_BITFVAL(IPU_DC_FC_2, enable);
            break;
        case DC_CHANNEL_3:
            iMask = CSP_BITFMASK(IPU_DC_FC_3);
            iBitval = CSP_BITFVAL(IPU_DC_FC_3, enable);
            break;
        case DC_CHANNEL_4:
            iMask = CSP_BITFMASK(IPU_DC_FC_4);
            iBitval = CSP_BITFVAL(IPU_DC_FC_4, enable);
            break;
        case DC_CHANNEL_6:
            iMask = CSP_BITFMASK(IPU_DC_FC_6);
            iBitval = CSP_BITFVAL(IPU_DC_FC_6, enable);
            break;
        default:
            // Error - Unsupported DC Channel
            return;
    }

    // set IPU_INTR_CTRL_15 with new value
    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_INT_CTRL_15);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_INT_CTRL_15,
            oldVal, newVal) != oldVal);

    return;
}

//------------------------------------------------------------------------------
//
// Function: CMStartDICounters
//
// This function starts the DI0/DI1 counters running.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void CMStartDICounters(DI_SELECT di_sel)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (di_sel == DI_SELECT_DI0)
    {
        iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE);
        iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE,
                    IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_START);
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
            oldVal, newVal) != oldVal);
    }
    else
    {
        iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE);
        iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE,
                    IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE_START);
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
            oldVal, newVal) != oldVal);
    }
}


//------------------------------------------------------------------------------
//
// Function: CMStopDICounters
//
// This function halts the DI0/DI1 counters.
//
// Parameters:
//      di_sel
//          [in] Selects either the DI0 or DI1 to configure
//
// Returns:
//      none.
//------------------------------------------------------------------------------
void CMStopDICounters(DI_SELECT di_sel)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (di_sel == DI_SELECT_DI0)
    {
        iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE);
        iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE,
                    IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_STOP);
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
            oldVal, newVal) != oldVal);
    }
    else
    {
        iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE);
        iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE,
                    IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_STOP);
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
            oldVal, newVal) != oldVal);
    }
}

//------------------------------------------------------------------------------
//
// Function: CMSetSRMPriority
//
// This sets the SRM Priority .
//
// Parameters:
//      module
//          [in] IPU submodule to set the SRM priority for.
//
//      priority
//          [in] Priority level to configure.  Valid range is 0-7.
//
// Returns:
//      TRUE if successfully configured; FALSE if configuration failed.
//------------------------------------------------------------------------------
BOOL CMSetSRMPriority(IPU_SRM_PRI_MODULE module, DWORD priority)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (priority > 7)
    {
        RETAILMSG(1,
            (TEXT("%s: Invalid priority value...must be between 0 and 7!  Aborting configuration.\r\n"), __WFUNCTION__));
        return FALSE;
    }

    switch(module)
    {
        case IPU_SRM_PRI_MODULE_CSI1:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI1_CSI1_SRM_PRI);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI1_CSI1_SRM_PRI, 
                    priority);
            break;
        case IPU_SRM_PRI_MODULE_CSI0:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI1_CSI0_SRM_PRI);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI1_CSI0_SRM_PRI, 
                    priority);
            break;
        case IPU_SRM_PRI_MODULE_ISP:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI1_ISP_SRM_PRI);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI1_ISP_SRM_PRI, 
                    priority);
            break;
        case IPU_SRM_PRI_MODULE_DP:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_SRM_PRI);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_SRM_PRI,
                    priority);
            break;
        case IPU_SRM_PRI_MODULE_DC:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DC_SRM_PRI);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DC_SRM_PRI,
                    priority);
            break;
        case IPU_SRM_PRI_MODULE_DI0:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DI0_SRM_PRI);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DI0_SRM_PRI,
                    priority);
            break;
        case IPU_SRM_PRI_MODULE_DI1:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DI1_SRM_PRI);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DI1_SRM_PRI,
                    priority);
            break;
        default:
            RETAILMSG(1,
                (TEXT("%s: Invalid module...Aborting configuration.\r\n"), __WFUNCTION__));
            return FALSE;
    }

    if ((module == IPU_SRM_PRI_MODULE_CSI1) || (module == IPU_SRM_PRI_MODULE_CSI0) || (module == IPU_SRM_PRI_MODULE_ISP))
    {
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI1,
                oldVal, newVal) != oldVal);
    }
    else
    {
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
                oldVal, newVal) != oldVal);
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: CMSetSRMMode
//
// This sets the SRM Mode.
//
// Parameters:
//      module
//          [in] IPU submodule to set the SRM mode for.
//
//      mode
//          [in] SRM mode to configure.  Valid values are:
//                  IPU_IPU_SRM_PRI_SRM_MODE_MCU_ACCESS_RAM
//                  IPU_IPU_SRM_PRI_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME
//                  IPU_IPU_SRM_PRI_SRM_MODE_FSU_CONTROL_SWAP_CONTINUOUSLY
//                  IPU_IPU_SRM_PRI_SRM_MODE_MCU_CONTROL_UPDATE_NOW
//
// Returns:
//      TRUE if successfully configured; FALSE if configuration failed.
//------------------------------------------------------------------------------
BOOL CMSetSRMMode(IPU_SRM_MODE_MODULE module, DWORD mode)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    switch(module)
    {
        case IPU_SRM_MODE_MODULE_CSI1:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI1_CSI1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI1_CSI1_SRM_MODE, 
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_CSI0:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI1_CSI0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI1_CSI0_SRM_MODE, 
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_ISP:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI1_ISP_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI1_ISP_SRM_MODE, 
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_DP_SYNC:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE,
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_DP_ASYNC0:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE,
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_DP_ASYNC1:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE,
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_DC_2:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DC_2_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DC_2_SRM_MODE,
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_DC_6:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DC_6_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DC_6_SRM_MODE,
                    mode);
            break;
        case IPU_SRM_MODE_MODULE_DI1:
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DI1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DI1_SRM_MODE,
                    mode);
            break;
        default:
            RETAILMSG(1,
                (TEXT("%s: Invalid module...Aborting configuration.\r\n"), __WFUNCTION__));
            return FALSE;
    }

    if ((module == IPU_SRM_PRI_MODULE_CSI1) || (module == IPU_SRM_PRI_MODULE_CSI0) || (module == IPU_SRM_PRI_MODULE_ISP))
    {
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI1);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI1,
                oldVal, newVal) != oldVal);
    }
    else
    {
        do
        {
            oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
            newVal = (oldVal & (~iMask)) | iBitval;
        } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
                oldVal, newVal) != oldVal);
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: CMSetCSIDataSource
//
// This sets whether the CSI0/CSI1 data source is MIPI or parallel.
//
// Parameters:
//      csi_sel
//          [in] Selects between CSI0 and CSI1 for configuration.
//
//      source
//          [in] CSI source to configure.  Valid values are:
//                  IPU_IPU_CONF_CSI_DATA_SOURCE_PARALLEL
//                  IPU_IPU_CONF_CSI_DATA_SOURCE_MIPI
//
// Returns:
//      TRUE if successfully configured; FALSE if configuration failed.
//------------------------------------------------------------------------------
void CMSetCSIDataSource(CSI_SELECT csi_sel, DWORD source)
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if (csi_sel == CSI_SELECT_CSI0)
    {
        
        iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI0_DATA_SOURCE);
        iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI0_DATA_SOURCE, source);
    }
    else
    {
        iMask = CSP_BITFMASK(IPU_IPU_CONF_CSI1_DATA_SOURCE);
        iBitval = CSP_BITFVAL(IPU_IPU_CONF_CSI1_DATA_SOURCE, source);
    }

    // Use interlocked function to enable CSI data source.
    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_CONF);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_CONF,
                oldVal, newVal) != oldVal);
}


void CMDumpRegs()
{
    int i;
    UINT32* ptr = (UINT32 *)&g_pIPUV3_COMMON->IPU_CONF;
    RETAILMSG (1, (TEXT("\n\nIPU Common Registers\n")));
    for (i = 0; i <= (sizeof(CSP_IPU_COMMON_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}
