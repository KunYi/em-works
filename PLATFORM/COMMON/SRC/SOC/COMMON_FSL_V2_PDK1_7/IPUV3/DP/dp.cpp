//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dp.cpp
//
//  IPUv3 CSP-level DP functions
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#include <math.h>
#pragma warning(pop)


#include "common_macros.h"
#include "dp_priv.h"
#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "ipu_common.h"
#include "dp.h"


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
PCSP_IPU_DP_REGS g_pIPUV3_SRM = NULL;
PCSP_IPU_DP_REGS g_pIPUV3_DP = NULL;
dpCSCConfigData g_CSCConfigure[3] ;   //the global CSC parameter
static UINT32 g_dp_com[3];  //the global DP_COM register data, all DP_COM access will through it.
static PCSP_IPU_COMMON_REGS g_pIPUV3_COMMON;
static HANDLE g_hIPUBase;
static UINT32 g_iSYNC_X = 0;
static UINT32 g_iSYNC_Y = 0;
CRITICAL_SECTION g_csDPCOM;

//Y = ((CSC_A0*R >> 4) + (CSC_A1*G >> 4) + (CSC_A2*B >> 4) + (CSC_B0 << 2) + (1 << (3 - (2-CSC_S0)))) >> (4 - (2-CSC_S0)) 
//Cb = ((CSC_A3*R >> 4) + (CSC_A4*G >> 4) + (CSC_A5*B >> 4) + (CSC_B1 << 2) + (-255 << (3 - (2-CSC_S1)))) >> (4 -(2-CSC_S1)) 
//Cr = ((CSC_A6*R >> 4) + (CSC_A7*G >> 4) + (CSC_A8*B >> 4) + (CSC_B2 << 2) + (-255 << (3 - (2-CSC_S2)))) >> (4 -(2-CSC_S2)) 
//
//B 13+1bit
//A 9+1bit
//S 2bit
// Color conversion coefficient table
//
// RGB to YUV
static const UINT16 dp_rgb2yuv_tbl[4][16] =
{
   // A0       A1       A2      A3       A4       A5       A6      A7       A8       B0        B1        B2       S0   S1   S2   sta_mode
    {0x04D, 0x096, 0x01D, 0x3D5, 0x3AB, 0x080, 0x080, 0x395, 0x3EB, 0x0000, 0x0200, 0x0200, 0x2, 0x2, 0x2, 0 },  // A1
    {0x042, 0x081, 0x019, 0x3DA, 0x3B6, 0x070, 0x070, 0x3A2, 0x3EE, 0x0040, 0x0200, 0x0200, 0x2, 0x2, 0x2, 1 },  // A0
    {0x036, 0x0B7, 0x012, 0x3E3, 0x39D, 0x080, 0x080, 0x38C, 0x3F4, 0x0000, 0x0200, 0x0200, 0x2, 0x2, 0x2, 0 },  // B1
    {0x02F, 0x09D, 0x010, 0x3E6, 0x3A9, 0x070, 0x070, 0x39A, 0x3F6, 0x0040, 0x0200, 0x0200, 0x2, 0x2, 0x2, 1 },  // B0
};

// YUV to RGB
static const UINT16 dp_yuv2rgb_tbl[4][16] =
{
   // A0       A1        A2       A3        A4        A5        A6        A7        A8        B0         B1          B2         S0   S1   S2   sta_mode
//    {0x095, 0x000, 0x0CC, 0x095, 0x3CE, 0x398, 0x095, 0x0FF, 0x000, 0x3E42, 0x010A, 0x3DD6, 0x1, 0x1, 0x1, 0 },  // A1
    {0x095, 0x000, 0x0CC, 0x095, 0x3CE, 0x398, 0x095, 0x0FF, 0x000, 0x3E68, 0x0134, 0x3E02, 0x1, 0x1, 0x1, 0 },  // A1
    {0x04A, 0x066, 0x000, 0x04A, 0x3CC, 0x3E7, 0x04A, 0x000, 0x081, 0x3E42, 0x010F, 0x3DD6, 0x2, 0x2, 0x2, 1 },  // A0
    {0x080, 0x000, 0x0CA, 0x080, 0x3E8, 0x3C4, 0x080, 0x0ED, 0x000, 0x3E6D, 0x00A8, 0x3E25, 0x1, 0x1, 0x1, 0 },  // B1
    {0x04A, 0x073, 0x000, 0x04A, 0x3DE, 0x3F2, 0x04A, 0x000, 0x087, 0x3E10, 0x009A, 0x3DBE, 0x2, 0x2, 0x2, 1 },  // B0
};

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
BOOL DPGetGammaCoeffs(dpGamma Gamma, pDpGammaCoeffs pGammaCoeffs);


//------------------------------------------------------------------------------
//
// Function: DPRegsInit
//
// This function allocates the data structures required for interaction
// with the IPUv3 DP hardware.
//
//
// Returns:
//      TRUE if successful.
//------------------------------------------------------------------------------
BOOL DPRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_SRM == NULL)
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

        // Map TPM memory region entries
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_SRM_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_SRM = (PCSP_IPU_DP_REGS) MmMapIoSpace(phyAddr, 
                                sizeof(CSP_IPU_DP_REGS), FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_SRM == NULL)
        {
            DEBUGMSG(1,
                (_T("Init:  SRM reg MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        // Map TPM memory region entries
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_DP_REGS_OFFSET;
        
        // Map peripheral physical address to virtual address
        g_pIPUV3_DP = (PCSP_IPU_DP_REGS) MmMapIoSpace(phyAddr, 
                                sizeof(CSP_IPU_DP_REGS), FALSE);
        
        // Check if virtual mapping failed
        if (g_pIPUV3_DP == NULL)
        {
            DEBUGMSG(1,
                (_T("Init:  DP reg MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }


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
    memset(&g_CSCConfigure, 0, sizeof(dpCSCConfigData)*3);
    g_dp_com[0]=0;
    g_dp_com[1]=0;
    g_dp_com[2]=0;
    //Initialize all SRM registers
    memset(g_pIPUV3_SRM,0,sizeof(CSP_IPU_DP_REGS));
    InitializeCriticalSection(&g_csDPCOM);
    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) DPRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  DPRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 DP hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void DPRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPUV3_DP)
    {
        MmUnmapIoSpace(g_pIPUV3_DP, sizeof(PCSP_IPU_DP_REGS));
        g_pIPUV3_DP = NULL;
    }    

    if (g_pIPUV3_SRM)
    {
        MmUnmapIoSpace(g_pIPUV3_SRM, sizeof(PCSP_IPU_DP_REGS));
        g_pIPUV3_SRM = NULL;
    }

    
    if (g_pIPUV3_COMMON)
    {
        MmUnmapIoSpace(g_pIPUV3_COMMON, sizeof(PCSP_IPU_DP_REGS));
        g_pIPUV3_COMMON = NULL;
    }
    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        RETAILMSG(1,
            (_T("DP Cleanup: Failed closing IPU Base handle!\r\n")));
    }


    IPU_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: DPEnable
//
// Enable the Display processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPEnable(void)
{


    DEBUGMSG (1,
        (TEXT("%s: Enabling DP!\r\n"), __WFUNCTION__));

    if(!IPUV3EnableModule(g_hIPUBase, IPU_SUBMODULE_DP, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (1,
            (TEXT("%s: Failed to enable DP!\r\n"), __WFUNCTION__));
    }

}

//-----------------------------------------------------------------------------
//
// Function: DPDisable
//
// Disable the Display processor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPDisable(void)
{
    DEBUGMSG (1,
        (TEXT("%s: Disabling DP!\r\n"), __WFUNCTION__));

    if(!IPUV3DisableModule(g_hIPUBase, IPU_SUBMODULE_DP, IPU_DRIVER_OTHER))
    {
        DEBUGMSG (1,
            (TEXT("%s: Failed to disable DP!\r\n"), __WFUNCTION__));
    }
}

//-----------------------------------------------------------------------------
//
// Function:  DPGetGammaCoeffs
//
// This function translates the gamma value to right gamma register coefficients. 
//
// Parameters:
//      Gamma
//          [in] The structure of gamma setting.
//
//      pGammaCoeffs
//          [out] pointer to gamma coefficients structure.
//
// Returns:
//      TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL DPGetGammaCoeffs(dpGamma Gamma, pDpGammaCoeffs pGammaCoeffs)
{
    //Algorithm from ElvisISP10 17C2.PDF  
    UINT16 k,ck[17],rk[16];
    const UINT16 tk[17] = {0, 2, 4, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384};
    double fStart, fHeight, fGammacurve[17], fScurve[17];


    //Gamma.fValue = 0.6F;   // nominal 0.5 - nominal gamma, different from actual, because of s-curve

    if(Gamma.fValue <= 0)
        return FALSE;

    if((Gamma.fWidth < 1 )||(Gamma.fWidth > 380 ))
    {
        Gamma.fWidth = 32.;  //nominal 32 - must be in {1,380} how narrow is s-curve
    }
    
    if((Gamma.fCenter< 1 )||(Gamma.fCenter > 380 ))
    {
        Gamma.fCenter = 32.;  // nominal 32 - must be in {1,380} where is s-curve centered
    }
    
    for (k = 0; k < 17; k++) {
        fScurve[k] = (256.F/3.14159F)*((float) atan((tk[k]-Gamma.fCenter)/Gamma.fWidth));
        fGammacurve[k] = pow((tk[k])/384.0, 1.0/Gamma.fValue)*255.0;
    }
    
    fStart = fScurve[0];
    fHeight = fScurve[16]-fStart;
    for (k = 0; k < 17; k++) {
        fScurve[k] = (256.F/fHeight)*(fScurve[k]-fStart);   //remap the s-curve region
        ck[k]=( (UINT16) (fScurve[k] * fGammacurve[k]/256.F + 0.5)) << 1;  
    }

    rk[0] = ck[1]-ck[0];
    pGammaCoeffs->Const[0]=0;
    pGammaCoeffs->Slope[0]=(UINT8)(rk[0]<<4);

    for(k = 1; k<5; k++)
    {
        rk[k] = ck[k+1]-ck[k];
        pGammaCoeffs->Const[k] = ck[k] - rk [k];
        pGammaCoeffs->Slope[k] = (UINT8)(rk[k] << (4-k));
    }

    for(k= 5; k<16; k++)
    {
        rk[k] = ck[k+1]-ck[k];
        pGammaCoeffs->Const[k] = ck[k];
        pGammaCoeffs->Slope[k] = (UINT8)(rk[k]>>1);
    }

    //avoid data overflow
    for(k = 0; k<16; k++)
    {
        pGammaCoeffs->Const[k] /= 2;
        pGammaCoeffs->Const[k] &= 0x1ff;
    }
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  DPGammaConfigure
//
// This function configure the gamma setting for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      pGammaConfigData
//          [in] pointer to gamma configure structure.
//
// Returns:
//      TRUE if successful.
//
//-----------------------------------------------------------------------------
BOOL DPGammaConfigure(DP_CH_TYPE Channel, pDpGammaConfigData pGammaConfigData)
{
    UINT16 iRegOffset, i;
    dpGammaCoeffs GammaCoeffs;


    UINT32 oldVal, newVal, iMask, iBitval;

    //According to channel type, set the right acess register offset and corresponding SRM mode.
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC0:
            iRegOffset = IPU_DP_COM_CONF_ASYNC0_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC1:
            iRegOffset = IPU_DP_COM_CONF_ASYNC1_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        default:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;                
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
    }
    iRegOffset = iRegOffset/4;

    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
            oldVal, newVal) != oldVal);
    
    if(pGammaConfigData->Gamma.fValue <= 0)
    {
        // negative gamma value is invalid value
        EnterCriticalSection(&g_csDPCOM);
        INSREG32BF((&g_dp_com[Channel]), 
            IPU_DP_COM_CONF_DP_GAMMA_EN,
            IPU_DP_COM_CONF_DP_GAMMA_EN_DISABLE);
        ERRORMSG(1,(TEXT("%s: Invalid gamma value. \r\n"), __WFUNCTION__));
        OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
        LeaveCriticalSection(&g_csDPCOM);
        return FALSE;
    }
    else if(pGammaConfigData->Gamma.fValue == 1)
    {
        //When Gamma value equal to 1, it means no Gamma correction needed,
        //disable it.
        EnterCriticalSection(&g_csDPCOM);
        INSREG32BF((&g_dp_com[Channel]), 
            IPU_DP_COM_CONF_DP_GAMMA_EN,
            IPU_DP_COM_CONF_DP_GAMMA_EN_DISABLE);
        OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
        LeaveCriticalSection(&g_csDPCOM);    
        return TRUE;
    }

    //Get the gamma coefficents according to gamma value
    DPGetGammaCoeffs(pGammaConfigData->Gamma, &GammaCoeffs);
    
    for(i = 0;i<8;i++)
    {
        OUTREG32((&g_pIPUV3_SRM->DP_GAMMA_C_SYNC[i]+iRegOffset),
        ((UINT32)GammaCoeffs.Const[2*i+1]<<16) |((UINT32)GammaCoeffs.Const[2*i]&0x0000ffff) );
    }
    for(i = 0;i<4;i++)
    {
        OUTREG32((&g_pIPUV3_SRM->DP_GAMMA_S_SYNC[i]+iRegOffset),
        (UINT32)(GammaCoeffs.Slope[4*i+3]<<24) |(UINT32)(GammaCoeffs.Slope[4*i+2]<<16) 
        |(UINT32)(GammaCoeffs.Slope[4*i+3]<<8) |(UINT32)(GammaCoeffs.Slope[4*i]) );
    }

    //configure gamma YUV mode
    EnterCriticalSection(&g_csDPCOM);
    if(TRUE == pGammaConfigData->bYUVmode)
    {
         INSREG32BF((&g_dp_com[Channel]), 
            IPU_DP_COM_CONF_DP_GAMMA_YUV_EN,
            IPU_DP_COM_CONF_DP_GAMMA_YUV_EN_ENABLE);           
    }
    else
    {
        INSREG32BF((&g_dp_com[Channel]), 
            IPU_DP_COM_CONF_DP_GAMMA_YUV_EN,
            IPU_DP_COM_CONF_DP_GAMMA_YUV_EN_DISABLE);        
    }
     
    INSREG32BF((&g_dp_com[Channel]), 
        IPU_DP_COM_CONF_DP_GAMMA_EN,
        IPU_DP_COM_CONF_DP_GAMMA_EN_ENABLE);
    OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
    LeaveCriticalSection(&g_csDPCOM);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  DPCSCGetCoeffs
//
// This function returns the CSC(color space conversion) coefficients of the specified DP 
// channel. It is designed for DLS function which needs to adjust csc value frame by frame. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      pDpCSCCoeffs
//          [out] pointer to CSC coefficients structure.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPCSCGetCoeffs(DP_CH_TYPE Channel, pDpCSCConfigData pCSCConfigData)
{
    memcpy(pCSCConfigData,&g_CSCConfigure[Channel],sizeof(dpCSCConfigData));
    return;
}

//-----------------------------------------------------------------------------
//
// Function:  DPCSCConfigure
//
// This function configures the CSC coefficients for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      pDpCSCConfigData
//          [in] pointer to CSC configure data structure.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPCSCConfigure(DP_CH_TYPE Channel, pDpCSCConfigData pCSCConfigData)
{
    UINT16 iRegOffset, i;
    
    UINT32 oldVal, newVal, iMask, iBitval;

    //According to channel type, set the right acess register offset and corresponding SRM mode.
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC0:
            iRegOffset = IPU_DP_COM_CONF_ASYNC0_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC1:
            iRegOffset = IPU_DP_COM_CONF_ASYNC1_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        default:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;                
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
    }
    iRegOffset = iRegOffset/4;

    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
            oldVal, newVal) != oldVal);


    memcpy(&g_CSCConfigure[Channel],pCSCConfigData,sizeof(dpCSCConfigData));
    if(g_CSCConfigure[Channel].CSCPosition == DP_CSC_DISABLE)
    {
        // Disable CSC in register
        EnterCriticalSection(&g_csDPCOM);
        INSREG32BF((&g_dp_com[Channel]), IPU_DP_COM_CONF_DP_CSC_DEF,
            IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_DISABLE);
        OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
        LeaveCriticalSection(&g_csDPCOM);    
        return;
    } 
    
    // Set up CSC for display-processing
    if (g_CSCConfigure[Channel].CSCEquation != CSCNoOp)
    {
        switch (g_CSCConfigure[Channel].CSCEquation)
        {
            case CSCR2Y_A1:
            case CSCR2Y_A0:
            case CSCR2Y_B0:
            case CSCR2Y_B1:
                for(i = 0 ; i<9 ;i++)
                    g_CSCConfigure[Channel].CSCCoeffs.A[i] = dp_rgb2yuv_tbl[pCSCConfigData->CSCEquation][i];
                for(i = 0 ; i<3 ;i++)
                    g_CSCConfigure[Channel].CSCCoeffs.B[i] = dp_rgb2yuv_tbl[pCSCConfigData->CSCEquation][i+9];
                for(i = 0 ; i<3 ;i++)
                    g_CSCConfigure[Channel].CSCCoeffs.S[i] = dp_rgb2yuv_tbl[pCSCConfigData->CSCEquation][i+12];

                g_CSCConfigure[Channel].CSCCoeffs.sta_mode = dp_rgb2yuv_tbl[pCSCConfigData->CSCEquation][15];
                
                break;

            case CSCY2R_A1:
            case CSCY2R_A0:
            case CSCY2R_B0:
            case CSCY2R_B1:
                for(i = 0 ; i<9 ;i++)
                    g_CSCConfigure[Channel].CSCCoeffs.A[i] = dp_yuv2rgb_tbl[pCSCConfigData->CSCEquation - 4][i];
                for(i = 0 ; i<3 ;i++)
                    g_CSCConfigure[Channel].CSCCoeffs.B[i] = dp_yuv2rgb_tbl[pCSCConfigData->CSCEquation - 4][i+9];
                for(i = 0 ; i<3 ;i++)
                    g_CSCConfigure[Channel].CSCCoeffs.S[i] = dp_yuv2rgb_tbl[pCSCConfigData->CSCEquation - 4][i+12];

                g_CSCConfigure[Channel].CSCCoeffs.sta_mode= dp_yuv2rgb_tbl[pCSCConfigData->CSCEquation - 4][15];
                break;

            case CSCCustom:
                for(i = 0 ; i<9 ;i++)
                    g_CSCConfigure[Channel].CSCCoeffs.A[i] = pCSCConfigData->CSCCoeffs.A[i] & 0x3ff;
                for(i = 0 ; i<3 ;i++)
                {
                    g_CSCConfigure[Channel].CSCCoeffs.B[i] = pCSCConfigData->CSCCoeffs.B[i] & 0x3fff ;
                    g_CSCConfigure[Channel].CSCCoeffs.S[i] = pCSCConfigData->CSCCoeffs.S[i] & 0x3;
                }
                g_CSCConfigure[Channel].CSCCoeffs.sta_mode= pCSCConfigData->CSCCoeffs.sta_mode & 0x1;
                break;

            default:
                DEBUGMSG(1,
                    (TEXT("%s: Invalid PP CSC equation. \r\n"), __WFUNCTION__));
                break;
        }

        // Set up the csc parameter
        for(i = 0;i<4;i++)
        {
            OUTREG32((&g_pIPUV3_SRM->DP_CSCA_SYNC[i]+iRegOffset),
            (UINT32)(g_CSCConfigure[Channel].CSCCoeffs.A[2*i+1]<<16) 
                    |g_CSCConfigure[Channel].CSCCoeffs.A[2*i] );
        }
        OUTREG32((&g_pIPUV3_SRM->DP_CSC_SYNC_0+iRegOffset),
        (UINT32)(g_CSCConfigure[Channel].CSCCoeffs.S[0]<<30) |
                (g_CSCConfigure[Channel].CSCCoeffs.B[0]<<16) |
                 g_CSCConfigure[Channel].CSCCoeffs.A[8] );
        OUTREG32((&g_pIPUV3_SRM->DP_CSC_SYNC_1+iRegOffset),
        (UINT32)(g_CSCConfigure[Channel].CSCCoeffs.S[2]<<30) |
                (g_CSCConfigure[Channel].CSCCoeffs.B[2]<<16) |
                (g_CSCConfigure[Channel].CSCCoeffs.S[1]<<14) |
                (g_CSCConfigure[Channel].CSCCoeffs.B[1]) );


        EnterCriticalSection(&g_csDPCOM);
        INSREG32BF((&g_dp_com[Channel]), 
            IPU_DP_COM_CONF_DP_CSC_YUV_SAT_MODE,
            g_CSCConfigure[Channel].CSCCoeffs.sta_mode);       

        //For sync panel's foreground , we should do enable FG CSC together with FG enable
        //If FG CSC is enabled before FG enable, enabled FG CSC will cause whole DP stall. Not so robust IC design.
        if((Channel!=DP_CHANNEL_SYNC)
            ||(g_CSCConfigure[Channel].CSCPosition==IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_DISABLE)
            ||(g_CSCConfigure[Channel].CSCPosition==IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_ENABLE_BEFORE_COMBINING_BG))
        {

            INSREG32BF((&g_dp_com[Channel]), IPU_DP_COM_CONF_DP_CSC_DEF,
                g_CSCConfigure[Channel].CSCPosition);
        }
        
        //configure gamut mapping
        if(TRUE == g_CSCConfigure[Channel].bGamutEnable)
        {
            INSREG32BF((&g_dp_com[Channel]), 
                IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN,
                IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN_ENABLE);           
        }
        else
        {
            INSREG32BF((&g_dp_com[Channel]), 
                IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN,
                IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN_DISABLE);        
        }
        OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
        LeaveCriticalSection(&g_csDPCOM);
    }
    else
    {
        // Disable CSC in register
        EnterCriticalSection(&g_csDPCOM);
        INSREG32BF((&g_dp_com[Channel]), IPU_DP_COM_CONF_DP_CSC_DEF,
            IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_DISABLE);
        OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
        LeaveCriticalSection(&g_csDPCOM); 
    }    
    return;
}

//-----------------------------------------------------------------------------
//
// Function:  DPCursorEnable
//
// This function enable or disable the cursor for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      enable
//          [in] The flag for enable or disable cursor.
//                  TRUE:   enable        
//                  FALSE:  disable        
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPCursorEnable(DP_CH_TYPE Channel, BOOL enable)    
{
    static UINT8 iCoc[3]={0,0,0};  //for store the cursor operation cotroll
    UINT8 iTempCoc;
    UINT16 iRegOffset;
    UINT32 oldVal, newVal, iMask, iBitval;

    //According to channel type, set the right acess register offset and corresponding SRM mode.
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC0:
            iRegOffset = IPU_DP_COM_CONF_ASYNC0_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC1:
            iRegOffset = IPU_DP_COM_CONF_ASYNC1_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        default:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;                
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
    }
    iRegOffset = iRegOffset/4;

    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
            oldVal, newVal) != oldVal);


    if(TRUE == enable)
    {
        EnterCriticalSection(&g_csDPCOM);
        if(iCoc[Channel] != 0) //check if the cursor is initialized
        {
            INSREG32BF((&g_dp_com[Channel]), 
                IPU_DP_COM_CONF_DP_COC, iCoc[Channel]);
        }
        else
        {
            DEBUGMSG(1,
                    (TEXT("%s: Cursor isn't initialized yet. \r\n"), __WFUNCTION__));                
            INSREG32BF((&g_dp_com[Channel]), 
                        IPU_DP_COM_CONF_DP_COC, 0);                
        }
        OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
        LeaveCriticalSection(&g_csDPCOM); 

    }
    else
    {
        EnterCriticalSection(&g_csDPCOM);
        iTempCoc = (UINT8)EXTREG32((&g_dp_com[Channel]), 
                            CSP_BITFMASK(IPU_DP_COM_CONF_DP_COC), 
                            IPU_DP_COM_CONF_DP_COC_LSH);
        if(iTempCoc != 0) //avoid disable cursor two times
        {
            iCoc[Channel]=iTempCoc;
            INSREG32BF((&g_dp_com[Channel]), 
                    IPU_DP_COM_CONF_DP_COC, 0);  
        }
        OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
        LeaveCriticalSection(&g_csDPCOM); 

    }
    return;
}

//-----------------------------------------------------------------------------
//
// Function:  DPCursorConfigure
//
// This function configures the cursor for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      coc
//          [in] the type of cursor operation control
//
//      iCursorColor
//          [in] the color mapping of cursor
//                0x00RRGGBB
//
//      bBlinkEnable
//          [in] Enable or disable Cursor blink function (only avaiable for synchronous channel)
//                
//      iBlinkRate
//          [in] The cursor blink rate, unit for this setting is frame(0~255)
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPCursorConfigure(DP_CH_TYPE Channel, DP_COC coc, UINT32 iCursorColor, 
                                            BOOL bBlinkEnable, UINT8 iBlinkRate)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(bBlinkEnable);
    UNREFERENCED_PARAMETER(iBlinkRate);


    UINT16 iRegOffset;
    UINT32 oldVal, newVal, iMask, iBitval;

    //According to channel type, set the right acess register offset and corresponding SRM mode.
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC0:
            iRegOffset = IPU_DP_COM_CONF_ASYNC0_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC1:
            iRegOffset = IPU_DP_COM_CONF_ASYNC1_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        default:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;                
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
    }
    iRegOffset = iRegOffset/4;

    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
            oldVal, newVal) != oldVal);

    EnterCriticalSection(&g_csDPCOM);
    INSREG32BF((&g_dp_com[Channel]),IPU_DP_COM_CONF_DP_COC, coc); 
    OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
    LeaveCriticalSection(&g_csDPCOM); 

    DPCursorEnable(Channel, FALSE); //for store the coc in DPCursorEnable()
    
    OUTREG32((&g_pIPUV3_SRM->DP_CUR_MAP_SYNC+iRegOffset),
        iCursorColor);

    // TODO: call DC function
    if(Channel ==DP_CHANNEL_SYNC)
    {
        //DCCursorBlinkConfig(bBlinkEnable, iBlinkRate);
    }
    return;
}

//-----------------------------------------------------------------------------
//
// Function:  DPCursorPosition
//
// This function setup the position of cursor for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      pDpCursorPos
//          [in] Pointer to cursor position structure
//              ->iWidth:   the width of cursor (0~31)
//              ->iHeight:  the height of cursor (0~31)
//              ->iXpos:    the x value of cursor upper left corner (0~2047) 
//              ->iYpos:    the y value of cursor upper left corner (0~2047) 
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPCursorPosition(DP_CH_TYPE Channel, pDpCursorPos pCursorPos)    
{
    UINT16 iRegOffset;
    UINT32 oldVal, newVal, iMask, iBitval;

    //According to channel type, set the right acess register offset and corresponding SRM mode.
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC0:
            iRegOffset = IPU_DP_COM_CONF_ASYNC0_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC1:
            iRegOffset = IPU_DP_COM_CONF_ASYNC1_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        default:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;                
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
    }
    iRegOffset = iRegOffset/4;

    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
            oldVal, newVal) != oldVal);


    //avoid the data overflow
    pCursorPos->iHeight &= 0x1f;
    pCursorPos->iWidth &= 0x1f;
    pCursorPos->iXpos &= 0x7ff;
    pCursorPos->iYpos &= 0x7ff;
    
    OUTREG32((&g_pIPUV3_SRM->DP_CUR_POS_SYNC+iRegOffset),
        (UINT32)(pCursorPos->iWidth<<27)
        |(pCursorPos->iXpos<<16)
        |(pCursorPos->iHeight<<11)
        |(pCursorPos->iYpos));    
    
    return;
}

//-----------------------------------------------------------------------------
//
// Function:  DPPartialPlanePosition
//
// This function setup the position of partial plane for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      x
//          [in] the x value of partial plane upper left corner based on full plane (0~2047) 
//
//      Y
//          [in] the y value of partial plane upper left corner based on full plane (0~2047) 
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPPartialPlanePosition(DP_CH_TYPE Channel, UINT32 x, UINT32 y)    
{
    UINT16 iRegOffset;
    UINT32 oldVal, newVal, iMask, iBitval;

    //According to channel type, set the right acess register offset and corresponding SRM mode.
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            g_iSYNC_X = x;
            g_iSYNC_Y = y;
            break;
        case DP_CHANNEL_ASYNC0:
            iRegOffset = IPU_DP_COM_CONF_ASYNC0_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC1:
            iRegOffset = IPU_DP_COM_CONF_ASYNC1_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        default:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;                
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
    }
    iRegOffset = iRegOffset/4;

    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
            oldVal, newVal) != oldVal);

    OUTREG32((&g_pIPUV3_SRM->DP_FG_POS_SYNC+iRegOffset), 
        (UINT32)(x<<16)|y);
    return;
}

//-----------------------------------------------------------------------------
//
// Function:  DPGraphicWindowEnable
//
// This function enables or disables graphic window for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      enable
//          [in] The flag for enable or disable graphic window.
//                  TRUE:   enable        
//                  FALSE:  disable        
//
// Returns:
//      True if sucessful.
//
//-----------------------------------------------------------------------------
BOOL DPGraphicWindowEnable(DP_CH_TYPE Channel, BOOL enable)    
{
    UINT32 oldVal, newVal, iMask, iBitval;

    if(TRUE == enable)
    {
        //Enable graphic window
        switch(Channel)
        {
            case DP_CHANNEL_SYNC:
                //For restore the FG position.
                OUTREG32((&g_pIPUV3_SRM->DP_FG_POS_SYNC),(UINT32)(g_iSYNC_X<<16)|g_iSYNC_Y);  

                //For sync panel, we enabled the FG CSC together with FG enable.
                EnterCriticalSection(&g_csDPCOM);
                INSREG32BF((&g_dp_com[DP_CHANNEL_SYNC]), 
                                IPU_DP_COM_CONF_DP_FG_EN, 
                                IPU_DP_COM_CONF_DP_FG_EN_ENABLE);
                INSREG32BF((&g_dp_com[DP_CHANNEL_SYNC]), IPU_DP_COM_CONF_DP_CSC_DEF,
                    g_CSCConfigure[DP_CHANNEL_SYNC].CSCPosition);
                OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC),g_dp_com[DP_CHANNEL_SYNC]);
                LeaveCriticalSection(&g_csDPCOM);

                iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
                iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                        IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
                        oldVal, newVal) != oldVal);
                break;
            case DP_CHANNEL_ASYNC0:
                iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0);
                iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0, 
                        IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0_ENABLE);
                if(EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_GEN, IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1))
                {
                    iMask |= CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW);
                    iBitval |= CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW, 
                                IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW);
                }
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
                        oldVal, newVal) != oldVal);
                        
                break;
            case DP_CHANNEL_ASYNC1:
                iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1);
                iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1, 
                        IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1_ENABLE);
                if(EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_GEN, IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0))
                {
                    iMask |= CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW);
                    iBitval |= CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW, 
                                IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW);
                }
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
                        oldVal, newVal) != oldVal);
                break;
            default:
                break;
        }        
    }
    else
    {
        //Disable graphic window
        switch(Channel)
        {
            case DP_CHANNEL_SYNC:
                //Disable FG
                EnterCriticalSection(&g_csDPCOM);
                INSREG32BF((&g_dp_com[DP_CHANNEL_SYNC]), 
                                IPU_DP_COM_CONF_DP_FG_EN, 
                                IPU_DP_COM_CONF_DP_FG_EN_DISABLE);
                OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC),g_dp_com[DP_CHANNEL_SYNC]);
                LeaveCriticalSection(&g_csDPCOM);
                
                //the Fg position must be back to (0,0)
                //Otherwise, the DMFC will have data tail which may cause DP stall.
                OUTREG32(&g_pIPUV3_SRM->DP_FG_POS_SYNC,0); 
                              
                iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
                iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                        IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
                        oldVal, newVal) != oldVal);

                break;
            case DP_CHANNEL_ASYNC0:
                //Disable async0 flow, meanwhile change async flow from double flow to single flow.
                //Afterr disable async0 flow, dobule async flow is impossible.
                iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0)
                    |CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW);
                iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0, 
                                IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0_DISABLE)
                            |CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW, 
                                IPU_IPU_DISP_GEN_DP_ASYNC_SINGLE_FLOW);

                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
                        oldVal, newVal) != oldVal);
                break;
            case DP_CHANNEL_ASYNC1:
                //Disable async1 flow, meanwhile change async flow from double flow to single flow.
                //Afterr disable async1 flow, dobule async flow is impossible.
                iMask = CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1)
                    |CSP_BITFMASK(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW);
                iBitval = CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1, 
                                IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0_DISABLE)
                            |CSP_BITFVAL(IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW, 
                                IPU_IPU_DISP_GEN_DP_ASYNC_SINGLE_FLOW);
                do
                {
                    oldVal = INREG32(&g_pIPUV3_COMMON->IPU_DISP_GEN);
                    newVal = (oldVal & (~iMask)) | iBitval;
                } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_DISP_GEN,
                        oldVal, newVal) != oldVal);
                break;
            default:
                break;
        }    
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  DPGraphicWindowConfigure
//
// This function configures graphic window for the specified DP channel. 
// The features are colorkey, partial plane type, global apha setting and FG position.
//
// Parameters:
//      Channel
//          [in] The DP channel being configured.
//
//      pDpGraphicWindowConfigData
//          [in] Pointer to graphic window configuration structure
//              ->alpha:    
//                      global alpha value of graphic window, 0(opaque)~255(transparent)
//              ->colorKey:
//                      the color key value of graphic window, the specific color will be transparent
//                      0x00RRGGBB
//              ->bPartialPlane:
//                      TRUE:   set partial plane as graphic window
//                      FALSE: set full plane as graphic window
//              ->bGlobalAlpha:
//                      TRUE:   global alpha 
//                      FALSE: local alpha,use the alpha data from alpha channel
//              ->iXpos:
//                      the x value of partial plane upper left corner based on full plane (0~2047) 
//              ->iYpos:
//                      the y value of partial plane upper left corner based on full plane (0~2047) 
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DPGraphicWindowConfigure(DP_CH_TYPE Channel, pDpGraphicWindowConfigData pGraphicWindowconfigData)    
{
    UINT32 graphWindCtrl;
    UINT16 iRegOffset;
    UINT32 oldVal, newVal, iMask, iBitval;

    //According to channel type, set the right acess register offset and corresponding SRM mode.
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC0:
            iRegOffset = IPU_DP_COM_CONF_ASYNC0_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        case DP_CHANNEL_ASYNC1:
            iRegOffset = IPU_DP_COM_CONF_ASYNC1_OFFSET;
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
        default:
            iRegOffset = IPU_DP_COM_CONF_SYNC_OFFSET;                
            iMask = CSP_BITFMASK(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE);
            iBitval = CSP_BITFVAL(IPU_IPU_SRM_PRI2_DP_S_SRM_MODE, 
                    IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME);
            break;
    }
    iRegOffset = iRegOffset/4;

    do
    {
        oldVal = INREG32(&g_pIPUV3_COMMON->IPU_SRM_PRI2);
        newVal = (oldVal & (~iMask)) | iBitval;
    } while ((UINT32) InterlockedTestExchange((LPLONG)&g_pIPUV3_COMMON->IPU_SRM_PRI2,
            oldVal, newVal) != oldVal);


    // Build value for graphics window control and write it to register
    graphWindCtrl = (UINT32)(pGraphicWindowconfigData->alpha << 24) 
                          | (pGraphicWindowconfigData->colorKey & 0x00FFFFFF);
    
    OUTREG32((&g_pIPUV3_SRM->DP_GRAPH_WIND_CTRL_SYNC+iRegOffset), graphWindCtrl);
    
    EnterCriticalSection(&g_csDPCOM);
    INSREG32BF((&g_dp_com[Channel]), 
                    IPU_DP_COM_CONF_DP_GWSEL, 
                    pGraphicWindowconfigData->bPartialPlane);
    
    INSREG32BF((&g_dp_com[Channel]), 
                    IPU_DP_COM_CONF_DP_GWAM, 
                    pGraphicWindowconfigData->bGlobalAlpha);

    //if colorkey equal to 0xFFFFFFFF, it means the colorkey should be disabled.
    if (pGraphicWindowconfigData->colorKey != 0xFFFFFFFF)
    {
        INSREG32BF((&g_dp_com[Channel]), 
                    IPU_DP_COM_CONF_DP_GWCKE, 
                    IPU_DP_COM_CONF_DP_GWCKE_ENABLE);
    }
    else
    {
        INSREG32BF((&g_dp_com[Channel]), 
                    IPU_DP_COM_CONF_DP_GWCKE, 
                    IPU_DP_COM_CONF_DP_GWCKE_DISABLE);
    }
    OUTREG32((&g_pIPUV3_SRM->DP_COM_CONF_SYNC+iRegOffset),g_dp_com[Channel]);
    LeaveCriticalSection(&g_csDPCOM); 

    //Setup partial plane position.
    DPPartialPlanePosition(Channel,
        pGraphicWindowconfigData->iXpos,
        pGraphicWindowconfigData->iYpos);
    return;
}

//-----------------------------------------------------------------------------
//
// Function:  DPIsBusy
//
// This function checks the dp module status for the specified DP channel. 
//
// Parameters:
//      Channel
//          [in] The DP channel being checked.
//
// Returns:
//      TRUE if busy.
//
//-----------------------------------------------------------------------------
BOOL DPIsBusy(DP_CH_TYPE Channel)
{
    UINT32 status;
    BOOL ret=FALSE;
    switch(Channel)
    {
        case DP_CHANNEL_SYNC:
            break;
        case DP_CHANNEL_ASYNC0:
            status= EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                                IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_CUR_FLOW);
            if(status==1)
            {
                ret= FALSE;
                break;
            }
            status= EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                                IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_TSTAT);
            if((status&0x3)!=0)
                ret= TRUE;
            else
                ret= FALSE;
            break;
        case DP_CHANNEL_ASYNC1:
            status= EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                                IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_CUR_FLOW);
            if(status==0)
            {
                ret= FALSE;
                break;
            }
            status= EXTREG32BF(&g_pIPUV3_COMMON->IPU_DISP_TASKS_STAT,
                                IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_TSTAT);
            if((status&0x3)!=0)
                ret= TRUE;
            else
                ret= FALSE;

            break;
        default:
            break;
    }
    return ret;
}
//-----------------------------------------------------------------------------
//
// Function:  DPColorKeyConv_A1
//
// This function is used for convert colorkey between two color space, and it always uses 
// formula A1
//
// Parameters:
//      OrgColorKey
//          [in] The original colorkey
//
//      bRGB2YUV
//          [in] TRUE if convert colorkey from RGB space to YUV space.
//                FALSE if convert colorkey from YUV space to RGB space.
//
// Returns:
//      the converted colorkey.
//
//-----------------------------------------------------------------------------
UINT32 DPColorKeyConv_A1(UINT32 OrgColorKey,BOOL bRGB2YUV)
{
    INT32 R,G,B;
    UINT32 Y,U,V,i,dstColorKey;
    UINT16 Coef_tbl[15];
    INT32 sig[12];
    B= OrgColorKey&0x0000ff;
    G= (OrgColorKey>>8)&0x0000ff;
    R= (OrgColorKey>>16)&0x0000ff;
    for(i= 0;i<9;i++)
    {
        if(bRGB2YUV)
            Coef_tbl[i]= dp_rgb2yuv_tbl[0][i];
        else
            Coef_tbl[i]= dp_yuv2rgb_tbl[0][i];
        if(Coef_tbl[i]>0x0200) // Convert nagtive value
        {
            Coef_tbl[i]=((~Coef_tbl[i])+1)&0x1ff;
            sig[i]=-1;
        }
        else
        {
            sig[i]=1;
        }
    }
    for(i= 9;i<12;i++)
    {
        if(bRGB2YUV)
            Coef_tbl[i]= dp_rgb2yuv_tbl[0][i];
        else
            Coef_tbl[i]= dp_yuv2rgb_tbl[0][i];
        if(Coef_tbl[i]>0x2000) // Convert nagtive value
        {
            Coef_tbl[i]=((~Coef_tbl[i])+1)&0x1fff;
            sig[i]=-1;
        }
        else
        {
            sig[i]=1;
        }
    }  
    for(i= 12;i<15;i++)
    {
        if(bRGB2YUV)
            Coef_tbl[i]= dp_rgb2yuv_tbl[0][i];
        else
            Coef_tbl[i]= dp_yuv2rgb_tbl[0][i];
    }  
    Y= ((Coef_tbl[0]*R >> 4)*sig[0] 
        + (Coef_tbl[1]*G >> 4)*sig[1] 
        + (Coef_tbl[2]*B >> 4)*sig[2] 
        + (Coef_tbl[9] << 2)*sig[9] 
        + (1 << (1 + Coef_tbl[12]))) 
        >> (2 + Coef_tbl[12]); 
    U= ((Coef_tbl[3]*R >> 4)*sig[3] 
        + (Coef_tbl[4]*G >> 4)*sig[4] 
        + (Coef_tbl[5]*B >> 4)*sig[5] 
        + (Coef_tbl[10] << 2)*sig[10] 
        + (1 << (1 + Coef_tbl[13]))) 
        >> (2 + Coef_tbl[13]); 
    V= ((Coef_tbl[6]*R >> 4)*sig[6] 
        + (Coef_tbl[7]*G >> 4)*sig[7] 
        + (Coef_tbl[8]*B >> 4)*sig[8] 
        + (Coef_tbl[11] << 2)*sig[11] 
        + (1 << (1 + Coef_tbl[14]))) 
        >> (2 + Coef_tbl[14]); 
    dstColorKey = (Y<<16) + (U<<8)+ V;
    return dstColorKey;
}


void DPDumpRegs()
{
    int i;
    UINT32*ptr = (UINT32 *)&g_pIPUV3_DP->DP_COM_CONF_SYNC;
    RETAILMSG (1, (TEXT("\n\nDP sync Registers\n")));
    for (i = 0; i < 24; i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }

    ptr = (UINT32 *)&g_pIPUV3_DP->DP_COM_CONF_ASYNC1;
    RETAILMSG (1, (TEXT("\n\nDP debug Registers\n")));
    RETAILMSG (1, (TEXT("Address %08x  %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1)));
}

