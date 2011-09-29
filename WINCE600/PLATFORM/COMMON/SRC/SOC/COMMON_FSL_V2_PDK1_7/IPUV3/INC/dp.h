//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dp.h
//
//  Display processor interface definitions
//
//-----------------------------------------------------------------------------

#ifndef __DP_H__
#define __DP_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types
typedef enum {
    DP_CHANNEL_SYNC = 0,
    DP_CHANNEL_ASYNC0,
    DP_CHANNEL_ASYNC1
} DP_CH_TYPE;

typedef enum {
    DP_CSC_DISABLE = 0,
    DP_CSC_BOTH = 1,
    DP_CSC_BG = 2,
    DP_CSC_FG = 3,
} DP_CSC_POS;

typedef enum {
    DP_COC_DISABLE =0,
    DP_COC_FULL,
    REVERSED,
    DP_COC_AND,
    RESERVED,
    DP_COC_OR,
    DP_COC_XOR,
} DP_COC;

// Display processor CSC equation coefficients
// These should be set when the CSCCustom CSC equation is selected
typedef struct dpCSCCoeffsStruct
{
    UINT16 A[9];
    UINT16 B[3];
    UINT16 S[3];
    UINT16 sta_mode;
} dpCSCCoeffs, *pDpCSCCoeffs;

typedef struct dpCSCConfigDataStruct
{
    CSCEQUATION CSCEquation;
    dpCSCCoeffs CSCCoeffs;
    DP_CSC_POS CSCPosition;
    BOOL bGamutEnable;
} dpCSCConfigData, *pDpCSCConfigData;

typedef struct dpGammaDataStruct
{
    float fValue;
    float fCenter;
    float fWidth;
} dpGamma, *pDpGamma;


typedef struct dpGammaConfigDataStruct
{
    dpGamma Gamma;
    BOOL bYUVmode;  //the dp output color mode
} dpGammaConfigData, *pDpGammaConfigData;

typedef struct dpGammaCoeffsStruct
{
   INT16 Const[16];
   UINT8 Slope[16];
} dpGammaCoeffs, *pDpGammaCoeffs;

typedef struct dpCursorPosStruct
{
    UINT8 iWidth;
    UINT8 iHeight;
    UINT16 iXpos;
    UINT16 iYpos;
} dpCursorPos, *pDpCursorPos;

typedef struct dpGraphicWindowConfigDataStruct
{
    UINT8 alpha;
    UINT32 colorKey;
    BOOL bPartialPlane;
    BOOL bGlobalAlpha;
    UINT16 iXpos;
    UINT16 iYpos;    
} dpGraphicWindowConfigData, *pDpGraphicWindowConfigData;


//------------------------------------------------------------------------------
// Functions
//DP Functions
BOOL DPRegsInit(void);
void DPRegsCleanup(void);
void DPEnable(void);
void DPDisable(void);
BOOL DPGammaConfigure(DP_CH_TYPE Channel, pDpGammaConfigData pGammaConfigData);

void DPCSCGetCoeffs(DP_CH_TYPE Channel, pDpCSCConfigData pCSCConfigData);
void DPCSCConfigure(DP_CH_TYPE Channel, pDpCSCConfigData pCSCConfigData);

void DPCursorEnable(DP_CH_TYPE Channel, BOOL enable);    
void DPCursorConfigure(DP_CH_TYPE Channel, DP_COC coc, UINT32 iCursorColor, BOOL bBlinkEnable, UINT8 iBlinkRate);
void DPCursorPosition(DP_CH_TYPE Channel, pDpCursorPos pCursorPos);

void DPPartialPlanePosition(DP_CH_TYPE Channel, UINT32 x, UINT32 y);    
BOOL DPGraphicWindowEnable(DP_CH_TYPE Channel, BOOL enable);    
void DPGraphicWindowConfigure(DP_CH_TYPE Channel, pDpGraphicWindowConfigData pGraphicWindowconfigData);
BOOL DPIsBusy(DP_CH_TYPE Channel);
UINT32 DPColorKeyConv_A1(UINT32 OrgColorKey,BOOL bRGB2YUV);

// Debug helper function
void DPDumpRegs();

#ifdef __cplusplus
}
#endif

#endif //__DP_H__
