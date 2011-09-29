//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/* 
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/

#include <windows.h>
#include <xllp_ac97.h>
#include <bulverde.h>
#include <ceddk.h>
#include "aclink.h"

static volatile BULVERDE_AC97_REG   *g_pAc97Regs  = NULL;
static volatile BULVERDE_OST_REG    *g_pOSTRegs   = NULL;
static volatile BULVERDE_INTR_REG   *g_pICRegs    = NULL;
static volatile BULVERDE_GPIO_REG   *g_pGPIORegs  = NULL;
static volatile BULVERDE_CLKMGR_REG *g_pClockRegs = NULL;

static XLLP_AC97_CONTEXT_T g_AC97CtxStruct;

static BOOL g_IsAC97Configured = FALSE;


BOOL InitializeACLink(BOOL InPowerHandler, UINT8 DevId)
{

    // Allocate AC link control resources.
    //
    if (!AllocateACLinkResources(DevId))
    {
        return(FALSE);
    }

    // Data structure used by XLLP routines.
    //
    g_AC97CtxStruct.pGpioReg          = (P_XLLP_GPIO_T)   g_pGPIORegs;
    g_AC97CtxStruct.pClockReg         = (P_XLLP_CLKMGR_T) g_pClockRegs;
    g_AC97CtxStruct.pAc97Reg          = (P_XLLP_AC97_T)   g_pAc97Regs;
    g_AC97CtxStruct.pOstRegs          = (P_XLLP_OST_T)    g_pOSTRegs;
    g_AC97CtxStruct.pIntcReg          = (P_XLLP_INTC_T)   g_pICRegs;
    g_AC97CtxStruct.maxSetupTimeOutUs = 1000;
    g_AC97CtxStruct.useSecondaryCodec = 0;

    XllpOstDelayMicroSeconds((P_XLLP_OST_T) g_pOSTRegs, 1);

    // Configure the AC97 controller.
    //
    if (!ConfigureAC97Control())
    {
        return(FALSE);
    }

    return(TRUE);
}

BOOL DeInitializeACLink(BOOL InPowerHandler, UINT8 DevId)
{
    if (!DeAllocateACLinkResources(DevId))
    {
        return(FALSE);
    }

    return(TRUE);
}

BOOL AllocateACLinkResources(UINT8 DevId)
{
    PHYSICAL_ADDRESS RegPA;

    if (g_pICRegs == NULL)
    {
        RegPA.QuadPart = BULVERDE_BASE_REG_PA_INTC;
        g_pICRegs = (volatile BULVERDE_INTR_REG *) MmMapIoSpace(RegPA, 0x400, FALSE);
    }

    if (g_pAc97Regs == NULL)
    {
        RegPA.QuadPart = BULVERDE_BASE_REG_PA_AC97;
        g_pAc97Regs = (volatile BULVERDE_AC97_REG *) MmMapIoSpace(RegPA, 0x400, FALSE);
    }

    if (g_pClockRegs == NULL)
    {
        RegPA.QuadPart = BULVERDE_BASE_REG_PA_CLKMGR;
        g_pClockRegs = (volatile BULVERDE_CLKMGR_REG *) MmMapIoSpace(RegPA, 0x400, FALSE);
    }

    if (g_pGPIORegs == NULL)
    {
        RegPA.QuadPart = BULVERDE_BASE_REG_PA_GPIO;
        g_pGPIORegs = (volatile BULVERDE_GPIO_REG *) MmMapIoSpace(RegPA, 0x400, FALSE);
    }

    if (g_pOSTRegs == NULL)
    {
        RegPA.QuadPart = BULVERDE_BASE_REG_PA_OST;
        g_pOSTRegs = (volatile BULVERDE_OST_REG *) MmMapIoSpace(RegPA, 0x400, FALSE);
    }

    if (hACLinkControlMutex == NULL)
    {
        hACLinkControlMutex = CreateMutex(NULL, FALSE, ACLINK_MUTEX_NAME);
    }

    if (!g_pICRegs || !g_pAc97Regs || !g_pClockRegs || !g_pGPIORegs || !g_pOSTRegs || !hACLinkControlMutex)
    {
        DEBUGMSG(TRUE, (TEXT("ERROR:  Failed to allocate AC Link resources.\r\n")));
        DeAllocateACLinkResources(DevId);
        return(FALSE);
    }

    return(TRUE);
}

BOOL DeAllocateACLinkResources(UINT8 DevId)
{

    if (g_pAc97Regs)
    {
        VirtualFree((void *)g_pAc97Regs, 0, MEM_RELEASE);
        g_pAc97Regs = NULL;
    }

    if (g_pICRegs)
    {
        VirtualFree((void *)g_pICRegs, 0, MEM_RELEASE);
        g_pICRegs = NULL;
    }

    if (g_pClockRegs)
    {
        VirtualFree((void *)g_pClockRegs, 0, MEM_RELEASE);
        g_pClockRegs = NULL;
    }

    if (g_pGPIORegs)
    {
        VirtualFree((void *)g_pGPIORegs, 0, MEM_RELEASE);
        g_pGPIORegs = NULL;
    }

    if (g_pOSTRegs)
    {
        VirtualFree((void *)g_pOSTRegs, 0, MEM_RELEASE);
        g_pOSTRegs = NULL;
    }

    if (hACLinkControlMutex)
    {
        CloseHandle(hACLinkControlMutex);
    }

    return(TRUE);
}

BOOL ConfigureAC97Control(void)
{
    if (g_IsAC97Configured)
    {
        return(TRUE);
    }

    if (XllpAc97Init(&g_AC97CtxStruct) != XLLP_AC97_NO_ERROR)
    {
        return(FALSE);
    }

    g_IsAC97Configured = TRUE;

    return(TRUE);
}

BOOL UnConfigureAC97Control(void)
{
    if (!g_IsAC97Configured)
    {
        return(TRUE);
    }

    if (XllpAc97DeInit(&g_AC97CtxStruct) != XLLP_AC97_NO_ERROR)
    {
        return(FALSE);
    }

    g_IsAC97Configured = FALSE;

    return(TRUE);
}

BOOL GetAC97Lock(void)
{   

    if (WaitForSingleObject(hACLinkControlMutex, 3000) == WAIT_OBJECT_0)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

BOOL ReleaseAC97Lock(void)
{

    if (!g_pAc97Regs) return(FALSE);

    g_pAc97Regs->car &= ~AC97CAR_CAIP;
    ReleaseMutex(hACLinkControlMutex);

    return(TRUE);
}

BOOL ReadAC97Raw(UINT8 Offset, UINT16 *pData, UINT8 DevId)
{

    if (XllpAc97Read((XLLP_UINT16_T)   Offset,
                     (XLLP_UINT16_T *) pData,
                     (P_XLLP_AC97_T)   g_pAc97Regs, 
                     (P_XLLP_OST_T)    g_pOSTRegs,
                     5000, 
                     XLLP_AC97_CODEC_PRIMARY) != XLLP_AC97_NO_ERROR)
    {
        return(FALSE);
    }

    return(TRUE);
}

BOOL ReadAC97(UINT8 Offset, UINT16 *pData, UINT8 DevId)
{
    BOOL retVal = FALSE;

    if (GetAC97Lock() == TRUE)
    {
        retVal = ReadAC97Raw(Offset, pData, DevId);
        ReleaseAC97Lock();
    }

    return(retVal);
}

BOOL WriteAC97Raw(UINT8 Offset, UINT16 Data, UINT8 DevId)
{

    if (XllpAc97Write((XLLP_UINT16_T) Offset,
                      (XLLP_UINT16_T) Data, 
                      (P_XLLP_AC97_T) g_pAc97Regs, 
                      (P_XLLP_OST_T)  g_pOSTRegs,
                      5000, 
                      XLLP_AC97_CODEC_PRIMARY) != XLLP_AC97_NO_ERROR)
    {
        return(FALSE);
    }

    return(TRUE);
}

BOOL WriteAC97(UINT8 Offset, UINT16 Data, UINT8 DevId)
{
    BOOL retVal = FALSE;

    if (GetAC97Lock() == TRUE)
    {
        retVal = WriteAC97Raw(Offset, Data, DevId);
        ReleaseAC97Lock();
    }

    return(retVal);
}


BOOL ColdResetAC97Control(void)
{

    if (XllpAc97ColdReset(&g_AC97CtxStruct) != XLLP_AC97_NO_ERROR)
    {
        return(FALSE);
    }

    return(TRUE);
}

