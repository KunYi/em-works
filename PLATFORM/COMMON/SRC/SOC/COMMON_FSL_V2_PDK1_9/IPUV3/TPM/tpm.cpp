//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  tpm.cpp
//
//  IPU Task Parameter Memory access functions
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

#include "tpm.h"
#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"

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
PCSP_IPU_MEM_TPM   g_pIPUV3_TPM;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: TPMRegsInit
//
// This function initializes the structures needed to access
// the IPUv3 Task Parameter Memory.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL TPMRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    HANDLE hIPUBase = NULL;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_TPM == NULL)
    {
        //  *** Use IPU_BASE driver to retrieve IPU Base Address ***

        // First, create handle to IPU_BASE driver
        hIPUBase = IPUV3BaseOpenHandle();
        if (hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        dwIPUBaseAddr = IPUV3BaseGetBaseAddr(hIPUBase);
        if (dwIPUBaseAddr == -1)
        {
            RETAILMSG (1,
                (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        // Map TPM memory region entries
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_TPM_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_TPM = (PCSP_IPU_MEM_TPM) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_MEM_TPM),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_TPM == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    if (!IPUV3BaseCloseHandle(hIPUBase))
    {
        RETAILMSG(1,
            (_T("TPM Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    // If initialization failed, be sure to clean up
    if (!rc) TPMRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  TPMRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 Task Parameter Memory.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void TPMRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPUV3_TPM)
    {
        MmUnmapIoSpace(g_pIPUV3_TPM, sizeof(PCSP_IPU_MEM_TPM));
        g_pIPUV3_TPM = NULL;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: TPMWrite
//
// This function configures the Task Parameter Memory with color space
// conversion configuration data for the specified IPU DMA channel.
//
// Parameters:
//      dwChannel
//          [in] The IDMAC channel being configured.
//
//      pCPMEMData
//          [in] Pointer to CPMEM_CONFIG_DATA structure containing
//          data to configure IDMAC channel.
//
//      bInterleaved
//          [in] TRUE if interleaved mode, FALSE if planar mode.
//
// Returns:
//      None.
//------------------------------------------------------------------------------
void TPMWrite(TPMConfigData *pTPMData)
{
    IPU_FUNCTION_ENTRY();

    if (pTPMData->tpmMatrix == TPM_CHANNEL_ENC_CSC1_MATRIX1)
    {
        OUTREG32(&g_pIPUV3_TPM->enc_csc1_matrix1.tpm_word1.coeffs_word1,
            CSP_BITFVAL(TPM_C22, pTPMData->cscCoeffData.C22) |
            CSP_BITFVAL(TPM_C11, pTPMData->cscCoeffData.C11) |
            CSP_BITFVAL(TPM_C00, pTPMData->cscCoeffData.C00) |
            CSP_BITFVAL(TPM_A0_LOW, pTPMData->cscCoeffData.A0));

        OUTREG32(&g_pIPUV3_TPM->enc_csc1_matrix1.tpm_word1.coeffs_word2,
            CSP_BITFVAL(TPM_A0_HIGH, pTPMData->cscCoeffData.A0>>TPM_A0_LOW_WID) |
            CSP_BITFVAL(TPM_SCALE, pTPMData->cscCoeffData.Scale) |
            CSP_BITFVAL(TPM_SAT_MODE, 0));

        OUTREG32(&g_pIPUV3_TPM->enc_csc1_matrix1.tpm_word2.coeffs_word1,
            CSP_BITFVAL(TPM_C20, pTPMData->cscCoeffData.C20) |
            CSP_BITFVAL(TPM_C10, pTPMData->cscCoeffData.C10) |
            CSP_BITFVAL(TPM_C01, pTPMData->cscCoeffData.C01) |
            CSP_BITFVAL(TPM_A1_LOW, pTPMData->cscCoeffData.A1));

        OUTREG32(&g_pIPUV3_TPM->enc_csc1_matrix1.tpm_word2.coeffs_word2,
            CSP_BITFVAL(TPM_A1_HIGH, pTPMData->cscCoeffData.A1>>TPM_A1_LOW_WID));

        OUTREG32(&g_pIPUV3_TPM->enc_csc1_matrix1.tpm_word3.coeffs_word1,
            CSP_BITFVAL(TPM_C21, pTPMData->cscCoeffData.C21) |
            CSP_BITFVAL(TPM_C12, pTPMData->cscCoeffData.C12) |
            CSP_BITFVAL(TPM_C02, pTPMData->cscCoeffData.C02) |
            CSP_BITFVAL(TPM_A2_LOW, pTPMData->cscCoeffData.A2));

        OUTREG32(&g_pIPUV3_TPM->enc_csc1_matrix1.tpm_word3.coeffs_word2,
            CSP_BITFVAL(TPM_A2_HIGH, pTPMData->cscCoeffData.A2>>TPM_A2_LOW_WID));
    }

    else if (pTPMData->tpmMatrix == TPM_CHANNEL_VF_CSC1_MATRIX1)
    {
        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix1.tpm_word1.coeffs_word1,
            CSP_BITFVAL(TPM_C22, pTPMData->cscCoeffData.C22) |
            CSP_BITFVAL(TPM_C11, pTPMData->cscCoeffData.C11) |
            CSP_BITFVAL(TPM_C00, pTPMData->cscCoeffData.C00) |
            CSP_BITFVAL(TPM_A0_LOW, pTPMData->cscCoeffData.A0));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix1.tpm_word1.coeffs_word2,
            CSP_BITFVAL(TPM_A0_HIGH, pTPMData->cscCoeffData.A0>>TPM_A0_LOW_WID) |
            CSP_BITFVAL(TPM_SCALE, pTPMData->cscCoeffData.Scale) |
            CSP_BITFVAL(TPM_SAT_MODE, 0));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix1.tpm_word2.coeffs_word1,
            CSP_BITFVAL(TPM_C20, pTPMData->cscCoeffData.C20) |
            CSP_BITFVAL(TPM_C10, pTPMData->cscCoeffData.C10) |
            CSP_BITFVAL(TPM_C01, pTPMData->cscCoeffData.C01) |
            CSP_BITFVAL(TPM_A1_LOW, pTPMData->cscCoeffData.A1));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix1.tpm_word2.coeffs_word2,
            CSP_BITFVAL(TPM_A1_HIGH, pTPMData->cscCoeffData.A1>>TPM_A1_LOW_WID));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix1.tpm_word3.coeffs_word1,
            CSP_BITFVAL(TPM_C21, pTPMData->cscCoeffData.C21) |
            CSP_BITFVAL(TPM_C12, pTPMData->cscCoeffData.C12) |
            CSP_BITFVAL(TPM_C02, pTPMData->cscCoeffData.C02) |
            CSP_BITFVAL(TPM_A2_LOW, pTPMData->cscCoeffData.A2));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix1.tpm_word3.coeffs_word2,
            CSP_BITFVAL(TPM_A2_HIGH, pTPMData->cscCoeffData.A2>>TPM_A2_LOW_WID));
    }

    else if (pTPMData->tpmMatrix == TPM_CHANNEL_VF_CSC1_MATRIX2)
    {
        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix2.tpm_word1.coeffs_word1,
            CSP_BITFVAL(TPM_C22, pTPMData->cscCoeffData.C22) |
            CSP_BITFVAL(TPM_C11, pTPMData->cscCoeffData.C11) |
            CSP_BITFVAL(TPM_C00, pTPMData->cscCoeffData.C00) |
            CSP_BITFVAL(TPM_A0_LOW, pTPMData->cscCoeffData.A0));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix2.tpm_word1.coeffs_word2,
            CSP_BITFVAL(TPM_A0_HIGH, pTPMData->cscCoeffData.A0>>TPM_A0_LOW_WID) |
            CSP_BITFVAL(TPM_SCALE, pTPMData->cscCoeffData.Scale) |
            CSP_BITFVAL(TPM_SAT_MODE, 0));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix2.tpm_word2.coeffs_word1,
            CSP_BITFVAL(TPM_C20, pTPMData->cscCoeffData.C20) |
            CSP_BITFVAL(TPM_C10, pTPMData->cscCoeffData.C10) |
            CSP_BITFVAL(TPM_C01, pTPMData->cscCoeffData.C01) |
            CSP_BITFVAL(TPM_A1_LOW, pTPMData->cscCoeffData.A1));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix2.tpm_word2.coeffs_word2,
            CSP_BITFVAL(TPM_A1_HIGH, pTPMData->cscCoeffData.A1>>TPM_A1_LOW_WID));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix2.tpm_word3.coeffs_word1,
            CSP_BITFVAL(TPM_C21, pTPMData->cscCoeffData.C21) |
            CSP_BITFVAL(TPM_C12, pTPMData->cscCoeffData.C12) |
            CSP_BITFVAL(TPM_C02, pTPMData->cscCoeffData.C02) |
            CSP_BITFVAL(TPM_A2_LOW, pTPMData->cscCoeffData.A2));

        OUTREG32(&g_pIPUV3_TPM->vf_csc1_matrix2.tpm_word3.coeffs_word2,
            CSP_BITFVAL(TPM_A2_HIGH, pTPMData->cscCoeffData.A2>>TPM_A2_LOW_WID));
    }
    else if (pTPMData->tpmMatrix == TPM_CHANNEL_PP_CSC1_MATRIX1)
    {
        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix1.tpm_word1.coeffs_word1,
            CSP_BITFVAL(TPM_C22, pTPMData->cscCoeffData.C22) |
            CSP_BITFVAL(TPM_C11, pTPMData->cscCoeffData.C11) |
            CSP_BITFVAL(TPM_C00, pTPMData->cscCoeffData.C00) |
            CSP_BITFVAL(TPM_A0_LOW, pTPMData->cscCoeffData.A0));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix1.tpm_word1.coeffs_word2,
            CSP_BITFVAL(TPM_A0_HIGH, pTPMData->cscCoeffData.A0>>TPM_A0_LOW_WID) |
            CSP_BITFVAL(TPM_SCALE, pTPMData->cscCoeffData.Scale) |
            CSP_BITFVAL(TPM_SAT_MODE, 0));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix1.tpm_word2.coeffs_word1,
            CSP_BITFVAL(TPM_C20, pTPMData->cscCoeffData.C20) |
            CSP_BITFVAL(TPM_C10, pTPMData->cscCoeffData.C10) |
            CSP_BITFVAL(TPM_C01, pTPMData->cscCoeffData.C01) |
            CSP_BITFVAL(TPM_A1_LOW, pTPMData->cscCoeffData.A1));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix1.tpm_word2.coeffs_word2,
            CSP_BITFVAL(TPM_A1_HIGH, pTPMData->cscCoeffData.A1>>TPM_A1_LOW_WID));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix1.tpm_word3.coeffs_word1,
            CSP_BITFVAL(TPM_C21, pTPMData->cscCoeffData.C21) |
            CSP_BITFVAL(TPM_C12, pTPMData->cscCoeffData.C12) |
            CSP_BITFVAL(TPM_C02, pTPMData->cscCoeffData.C02) |
            CSP_BITFVAL(TPM_A2_LOW, pTPMData->cscCoeffData.A2));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix1.tpm_word3.coeffs_word2,
            CSP_BITFVAL(TPM_A2_HIGH, pTPMData->cscCoeffData.A2>>TPM_A2_LOW_WID));
    }

    else if (pTPMData->tpmMatrix == TPM_CHANNEL_PP_CSC1_MATRIX2)
    {
        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix2.tpm_word1.coeffs_word1,
            CSP_BITFVAL(TPM_C22, pTPMData->cscCoeffData.C22) |
            CSP_BITFVAL(TPM_C11, pTPMData->cscCoeffData.C11) |
            CSP_BITFVAL(TPM_C00, pTPMData->cscCoeffData.C00) |
            CSP_BITFVAL(TPM_A0_LOW, pTPMData->cscCoeffData.A0));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix2.tpm_word1.coeffs_word2,
            CSP_BITFVAL(TPM_A0_HIGH, pTPMData->cscCoeffData.A0>>TPM_A0_LOW_WID) |
            CSP_BITFVAL(TPM_SCALE, pTPMData->cscCoeffData.Scale) |
            CSP_BITFVAL(TPM_SAT_MODE, 0));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix2.tpm_word2.coeffs_word1,
            CSP_BITFVAL(TPM_C20, pTPMData->cscCoeffData.C20) |
            CSP_BITFVAL(TPM_C10, pTPMData->cscCoeffData.C10) |
            CSP_BITFVAL(TPM_C01, pTPMData->cscCoeffData.C01) |
            CSP_BITFVAL(TPM_A1_LOW, pTPMData->cscCoeffData.A1));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix2.tpm_word2.coeffs_word2,
            CSP_BITFVAL(TPM_A1_HIGH, pTPMData->cscCoeffData.A1>>TPM_A1_LOW_WID));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix2.tpm_word3.coeffs_word1,
            CSP_BITFVAL(TPM_C21, pTPMData->cscCoeffData.C21) |
            CSP_BITFVAL(TPM_C12, pTPMData->cscCoeffData.C12) |
            CSP_BITFVAL(TPM_C02, pTPMData->cscCoeffData.C02) |
            CSP_BITFVAL(TPM_A2_LOW, pTPMData->cscCoeffData.A2));

        OUTREG32(&g_pIPUV3_TPM->pp_csc1_matrix2.tpm_word3.coeffs_word2,
            CSP_BITFVAL(TPM_A2_HIGH, pTPMData->cscCoeffData.A2>>TPM_A2_LOW_WID));
    }

    IPU_FUNCTION_EXIT();
}
