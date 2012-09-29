//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  tpm.h
//
//  Task Parameter Memory interface definitions
//
//-----------------------------------------------------------------------------

#ifndef __TPM_H__
#define __TPM_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types

// CSC equation coefficients
typedef struct CSCCoeffsStruct
{
    UINT16 C00;
    UINT16 C01;
    UINT16 C02;
    UINT16 C10;
    UINT16 C11;
    UINT16 C12;
    UINT16 C20;
    UINT16 C21;
    UINT16 C22;
    UINT16 A0;
    UINT16 A1;
    UINT16 A2;
    UINT16 Scale;
} icCSCCoeffs, *pIcCSCCoeffs;

typedef enum TPM_CSC_MATRIX_ENUM
{
    TPM_CHANNEL_ENC_CSC1_MATRIX1,    // Pre-processor encoding, CSC equation 1
    TPM_CHANNEL_VF_CSC1_MATRIX1,     // Pre-Processor viewfinding, CSC equation 1
    TPM_CHANNEL_VF_CSC1_MATRIX2,     // Pre-Processor viewfinding, CSC equation 1
    TPM_CHANNEL_PP_CSC1_MATRIX1,     // Post-Processor, CSC equation 1
    TPM_CHANNEL_PP_CSC1_MATRIX2,     // Post-Processor, CSC equation 1
} TPM_CSC_MATRIX;

typedef struct TPMConfigDataStruct
{
    icCSCCoeffs           cscCoeffData;
    TPM_CSC_MATRIX        tpmMatrix;
} TPMConfigData, *pTPMConfigData;

//------------------------------------------------------------------------------
// Functions

BOOL TPMRegsInit();
void TPMRegsCleanup();
void TPMWrite(TPMConfigData *pTPMData);

#ifdef __cplusplus
}
#endif

#endif //__TPM_H__

