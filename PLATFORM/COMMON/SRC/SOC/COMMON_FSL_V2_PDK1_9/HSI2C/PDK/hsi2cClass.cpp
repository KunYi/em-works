//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  HSI2CClass.c
//
//  This file contains the main I2C protocol engine class.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4127 4201)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <linklist.h>
#include <windev.h>
#include <ceddk.h>
#include <stdlib.h>
#pragma warning(pop)

#include "common_ddk.h"
#include "common_macros.h"
#include "common_hsi2c.h"
#include "hsi2cbus.h"
#include "hsi2cclass.h"

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines 
//------------------------------------------------------------------------------
#define I2CFSDIVTABSIZE   (sizeof(wI2CFSClockRateDivider)/sizeof(wI2CFSClockRateDivider[0]))
#define I2CHSDIVTABSIZE   (sizeof(wI2CHSClockRateDivider)/sizeof(wI2CHSClockRateDivider[0]))

#define I2CFS_MAXDIVIDER   3840
#define I2CFS_MINDIVIDER   16

#define I2CHS_MAXDIVIDER   3584
#define I2CHS_MINDIVIDER   16

#define  HSI2C_TRANSMIT_WAIT          (I2CFS_MAXDIVIDER*9)
#define  HSI2C_THREADPRIORITY         100
#define  HSI2C_WAITTIMEOUT            100000  
//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables 
//------------------------------------------------------------------------------
static const WORD wI2CFSClockRateDivider[] = {
    16,  18,  20,  22,  32,  36,  40,  44,   64,   72,  80,  88,  128,  144,  160,  176,
    256, 288, 320, 352, 512, 576, 640, 704, 1024, 1152, 1280, 1408, 2048, 2304, 2560, 2816,
    24,  26,  28,  30,  48,  52,  56,  60,   96,   104,   112,   120,   192,   208,  224,  240,
    384, 416, 448, 480, 768, 832, 896, 960,  1536,  1664,  1792, 1920, 3072, 3328, 3584, 3840
};
static const WORD wI2CHSClockRateDivider[] = {
    16,  18,  20,  22,  28,  32,  36,  40,   56,   64,  72,  80,  112,  128,  144,  160,
    224, 256, 288, 320, 448, 512, 576, 640, 896,  1024, 1152, 1280, 1792, 2048, 2304, 2560,
    24,  26,  28,  30,  44,  48,  52,  56,   88,   96,   104,   112,   176,   192,  208,  224,
    352, 384, 416, 448, 704, 768, 832, 896,  1408,  1536,  1664, 1792, 2816, 3072, 3328, 3584
};

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------
void HSI2CClass::DUMP()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("__________________________________________________________\r\n")));
    DEBUGMSG(ZONE_FUNCTION,(_T("HIMADR=%x\r\n"),INREG16(&pHSI2CReg->HIMADR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HICR=%x\r\n"),INREG16(&pHSI2CReg->HICR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HISR=%x\r\n"),INREG16(&pHSI2CReg->HISR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HIIMR=%x\r\n"),INREG16(&pHSI2CReg->HIIMR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HIFSFDR=%x\r\n"),INREG16(&pHSI2CReg->HIFSFDR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HITFR=%x\r\n"),INREG16(&pHSI2CReg->HITFR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HIRFR=%x\r\n"),INREG16(&pHSI2CReg->HIRFR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HITDCR=%x\r\n"),INREG16(&pHSI2CReg->HITDCR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("HIRDCR=%x\r\n"),INREG16(&pHSI2CReg->HIRDCR)));
    DEBUGMSG(ZONE_FUNCTION,(_T("__________________________________________________________\r\n")));
}
//-----------------------------------------------------------------------------
//
// Function: HSI2CWaitForRestartStatus
//
// This function will return until HSI2C restart bit status or waiting timeout
//
// Parameters:
//      status: 1: in progress
//              0: idle  
// Returns:
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CWaitForRestartStatus(DWORD status, DWORD timout = HSI2C_WAITTIMEOUT)
{
    DWORD count = 0;
        
    while (EXTREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_RSTA),
                    I2C_HICR_RSTA_LSH) != status)
    {
        // Intentional polling loop.
        if ( count > timout )
        {
            DEBUGMSG(ZONE_ERROR, (_T("Wait for repeat start status timeout, status=%d\r\n"),status));
            return FALSE;
        }
        count++;
        StallExecution(10);
    }
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: HSI2CWaitForBusStatus
//
// This function will return until HSI2C bus is ready or waiting timeout
//
// Parameters:
//      status: I2C_HISR_IBB_BUSY
//              I2C_I2SR_IBB_IDLE  
// Returns:
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CWaitForBusStatus(DWORD status, DWORD timout = HSI2C_WAITTIMEOUT)
{
    DWORD count = 0;
        
    while (EXTREG16(&pHSI2CReg->HISR, CSP_BITFMASK(I2C_HISR_IBB),
                    I2C_HISR_IBB_LSH) != status)
    {
        // Intentional polling loop.
        if ( count > timout )
        {
            DEBUGMSG(ZONE_ERROR, (_T("Wait for bus status timeout, status=%d\r\n"),status));
            return FALSE;
        }
        count++;
        StallExecution(10);
    }
    
    return TRUE;
}
BOOL HSI2CClass::HSI2CWaitAndClearByteTransfer(DWORD polling = 1, DWORD timeout = HSI2C_WAITTIMEOUT)
{
    DWORD count =0;
    DWORD i = CeGetThreadPriority(GetCurrentThread());
    CeSetThreadPriority(GetCurrentThread(), HSI2C_THREADPRIORITY);

    if(polling)
    {
        while (EXTREG16(&pHSI2CReg->HISR, CSP_BITFMASK(I2C_HISR_BTD),
                        I2C_HISR_BTD_LSH) != I2C_HISR_BTD_COMPLETE)
        {
            // Internal polling loop.
            if ( count > timeout )
            {
                DEBUGMSG(ZONE_ERROR, (_T("Wait for byte transfer timeout!\r\n")));
                return FALSE;
            }
            count++;
            StallExecution(10);
        }
        OUTREG16(&pHSI2CReg->HISR, 1 << I2C_HISR_BTD_LSH);
    }
    
    CeSetThreadPriority(GetCurrentThread(), i);
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: HSI2CCheckAckStatus
//
// This function will check HSI2C ack/nack
//
// Parameters:
//      None
//
// Returns:
//      TRUE   ACK
//      FALSE  NACK
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CCheckAckStatus()
{
    if(EXTREG16(&pHSI2CReg->HISR, CSP_BITFMASK(I2C_HISR_RXAK),
                    I2C_HISR_RXAK_LSH) == I2C_HISR_RXAK_NO_ACK_DETECT)
    {
        INSREG16(&pHSI2CReg->HISR, CSP_BITFMASK(I2C_HISR_RXAK),
                 CSP_BITFVAL(I2C_HISR_RXAK, I2C_HISR_RXAK_NO_ACK_DETECT));
        return FALSE;
    } 
    
    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: HSI2CCheckArbitLost
//
// This function will check HSI2C arbitration lost
//
// Parameters:
//      None
//
// Returns:
//      TRUE   Not Lost
//      FALSE  Lost
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CCheckArbitLost()
{
    if (EXTREG16(&pHSI2CReg->HISR, CSP_BITFMASK(I2C_HISR_IAL),
                 I2C_HISR_IAL_LSH) == I2C_HISR_IAL_LOST)
    {
        DEBUGMSG(ZONE_ERROR,(_T("arbit lost!\r\n")));
        // Clear IAL bit (we are already put into Stop)
        INSREG16(&pHSI2CReg->HISR, CSP_BITFMASK(I2C_HISR_IAL),
                 CSP_BITFVAL(I2C_HISR_IAL, I2C_HISR_IAL_NOT_LOST));

        return FALSE;
    }
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CWaitForTDC
//
// This function will not return until HSI2C TDC is set or timeout
//
// Parameters:
//      None
//
// Returns:
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CWaitForTDC(DWORD polling = 0, DWORD timeout = HSI2C_WAITTIMEOUT)
{
    BOOL  bRet = TRUE;
    DWORD count =0;
    DWORD i = CeGetThreadPriority(GetCurrentThread());
    CeSetThreadPriority(GetCurrentThread(), HSI2C_THREADPRIORITY);

    if(polling)
    {
        while (EXTREG16(&pHSI2CReg->HISR, CSP_BITFMASK(I2C_HISR_TDC),
                        I2C_HISR_TDC_LSH) != I2C_HISR_TDC_ZERO)
        {
            // Internal polling loop.
            if ( count > timeout )
            {
                DEBUGMSG(ZONE_ERROR, (_T("Wait for TDC timeout!\r\n")));
                bRet = FALSE;
                goto out;
            }
            count++;
            StallExecution(10);
        }
        OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
    }
    else
    {
        // Enable intr, arbit, ack/nack, Transmit Data Empty
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_AL), CSP_BITFVAL(I2C_HIIMR_AL, I2C_HIIMR_AL_UNMASK));
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_TDC), CSP_BITFVAL(I2C_HIIMR_TDC, I2C_HIIMR_TDC_UNMASK));
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_RXAK), CSP_BITFVAL(I2C_HIIMR_RXAK, I2C_HIIMR_RXAK_UNMASK));
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_ENABLE));
        ResetEvent(hInterrupted);
        InterruptDone(dwSysIntr);
        count = WaitForSingleObject(hInterrupted, timeout);
        if(count == WAIT_OBJECT_0)
        {
            if(!HSI2CCheckArbitLost())
            {
                DEBUGMSG(ZONE_ERROR, (_T("arbit lost in wait for TDC\r\n")));
                bRet = FALSE;
            }
            if(!HSI2CCheckAckStatus())
            {
                DEBUGMSG(ZONE_ERROR, (_T("ack not received in wait for TDC\r\n")));
                bRet = FALSE;
            }
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, (_T("WAIT_TIMEOUT in wait for TDC\r\n")));
            bRet = FALSE;
        }
        OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
        OUTREG16(&pHSI2CReg->HIIMR, 0xFFFF);
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_DISABLE));
    }
out:    
    CeSetThreadPriority(GetCurrentThread(), i);
    return bRet;
}
//-----------------------------------------------------------------------------
//
// Function: HSI2CWaitForRDC
//
// This function will not return until HSI2C RDC is set or timeout
//
// Parameters:
//      None
//
// Returns:
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CWaitForRDC(DWORD polling = 0, DWORD timeout = HSI2C_WAITTIMEOUT)
{
    BOOL  bRet = TRUE;
    DWORD count =0;
    DWORD i = CeGetThreadPriority(GetCurrentThread());
    CeSetThreadPriority(GetCurrentThread(), HSI2C_THREADPRIORITY);

    if(polling)
    {
        while (EXTREG16(&pHSI2CReg->HIRDCR, CSP_BITFMASK(I2C_HIRDCR_RDCC),
                        I2C_HIRDCR_RDCC_LSH) != 0)
        {
            // Internal polling loop.
            if ( count > timeout )
            {
                DEBUGMSG(ZONE_ERROR, (_T("Wait for RDC timeout!\r\n")));
                bRet = FALSE;
                goto out;
            }
            count++;
            StallExecution(10);
        }
    }
    else
    {
        // Enable intr, arbit, ack/nack, Transmit Data Empty
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_AL), CSP_BITFVAL(I2C_HIIMR_AL, I2C_HIIMR_AL_UNMASK));
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_RDC), CSP_BITFVAL(I2C_HIIMR_RDC, I2C_HIIMR_RDC_UNMASK));
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_RXAK), CSP_BITFVAL(I2C_HIIMR_RXAK, I2C_HIIMR_RXAK_UNMASK));
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_ENABLE));
        ResetEvent(hInterrupted);
        InterruptDone(dwSysIntr);
        count = WaitForSingleObject(hInterrupted, timeout);
        if(count == WAIT_OBJECT_0)
        {
            if(!HSI2CCheckArbitLost())
            {
                DEBUGMSG(ZONE_ERROR,(_T("arbit lost in wait for RDC\r\n")));
                bRet = FALSE;
            }
            if(!HSI2CCheckAckStatus())
            {
                DEBUGMSG(ZONE_ERROR,(_T("ack not received in wait for RDC\r\n")));
                bRet = FALSE;
            }
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, (_T("WAIT_TIMEOUT\r\n")));
            bRet = FALSE;
        }
        OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
        OUTREG16(&pHSI2CReg->HIIMR, 0xFFFF);
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_DISABLE));
    }
out:    
    CeSetThreadPriority(GetCurrentThread(), i);
    return bRet;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CWaitForTDE
//
// This function will not return until HSI2C TDE is set or timeout
//
// Parameters:
//      None
//
// Returns:
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CWaitForTDE(DWORD polling = 0, DWORD timeout = HSI2C_WAITTIMEOUT)
{
    BOOL  bRet = TRUE;
    DWORD count =0;
    DWORD i = CeGetThreadPriority(GetCurrentThread());
    CeSetThreadPriority(GetCurrentThread(), HSI2C_THREADPRIORITY);

    if(polling)
    {
        DWORD currentTFC = EXTREG16(&pHSI2CReg->HITFR, CSP_BITFMASK(I2C_HITFR_TFC), I2C_HITFR_TFC_LSH); 
        while (currentTFC != 0)
        {
            // Internal polling loop.
            if ( count > timeout )
            {
                DEBUGMSG(ZONE_ERROR, (_T("Wait for TDE timeout!\r\n")));
                bRet = FALSE;
                goto out;
            }
            count++;
            StallExecution(10);
            currentTFC = EXTREG16(&pHSI2CReg->HITFR, CSP_BITFMASK(I2C_HITFR_TFC), I2C_HITFR_TFC_LSH);
        }
    }
    else
    {
        // Setting up Tx FIFO Register threshold    
        INSREG16(&pHSI2CReg->HITFR, CSP_BITFMASK(I2C_HITFR_TFWM), 0);

        // Enable intr, arbit, ack/nack, Transmit Data Empty
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_AL), CSP_BITFVAL(I2C_HIIMR_AL, I2C_HIIMR_AL_UNMASK));
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_TDE), CSP_BITFVAL(I2C_HIIMR_TDE, I2C_HIIMR_TDE_UNMASK));
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_RXAK), CSP_BITFVAL(I2C_HIIMR_RXAK, I2C_HIIMR_RXAK_UNMASK));
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_ENABLE));

        ResetEvent(hInterrupted);
        InterruptDone(dwSysIntr);
        count = WaitForSingleObject(hInterrupted, timeout);
        if(count == WAIT_OBJECT_0)
        {
            if(!HSI2CCheckArbitLost())
            {
                DEBUGMSG(ZONE_ERROR, (_T("arbit lost in wait for TDC\r\n")));
                bRet = FALSE;
            }
            if(!HSI2CCheckAckStatus())
            {
                DEBUGMSG(ZONE_ERROR, (_T("ack not received in wait for TDC\r\n")));
                bRet = FALSE;
            }
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, (_T("WAIT_TIMEOUT in wait for TDC\r\n")));
            bRet = FALSE;
        }

        // Clear BTD bit
        OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
        OUTREG16(&pHSI2CReg->HIIMR, 0xFFFF);
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_DISABLE));
    }
out:    
    CeSetThreadPriority(GetCurrentThread(), i);
    return bRet;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CWaitForRDF
//
// This function will not return until HSI2C RDF is set or timeout
//
// Parameters:
//      None
//
// Returns:
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::HSI2CWaitForRDF(DWORD bytes, DWORD polling = 0, DWORD timeout = HSI2C_WAITTIMEOUT)
{
    BOOL  bRet = TRUE;
    DWORD count =0;
    DWORD i = CeGetThreadPriority(GetCurrentThread());
    CeSetThreadPriority(GetCurrentThread(), HSI2C_THREADPRIORITY);

    if(polling)
    {
        DWORD currentRFC = EXTREG16(&pHSI2CReg->HIRFR, CSP_BITFMASK(I2C_HIRFR_RFC), I2C_HIRFR_RFC_LSH);
    
        while (currentRFC < bytes)
        {
            // Internal polling loop.
            if ( count > timeout )
            {
                DEBUGMSG(ZONE_ERROR, (_T("Wait for RDF timeout!\r\n")));
                bRet = FALSE;
                goto out;
            }
            count++;
            currentRFC = EXTREG16(&pHSI2CReg->HIRFR, CSP_BITFMASK(I2C_HIRFR_RFC), I2C_HIRFR_RFC_LSH);
        }
    }
    else
    {
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_AL), CSP_BITFVAL(I2C_HIIMR_AL, I2C_HIIMR_AL_UNMASK));
        INSREG16(&pHSI2CReg->HIIMR, CSP_BITFMASK(I2C_HIIMR_RDF), CSP_BITFVAL(I2C_HIIMR_RDF, I2C_HIIMR_RDF_UNMASK));
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_ENABLE));
        ResetEvent(hInterrupted);
        InterruptDone(dwSysIntr);
        count = WaitForSingleObject(hInterrupted, timeout);
        if(count == WAIT_OBJECT_0)
        {
            if(!HSI2CCheckArbitLost())
            {
                DEBUGMSG(ZONE_ERROR,(_T("arbit lost in wait for RDF\r\n")));
                bRet = FALSE;
            }
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, (_T("WAIT_TIMEOUT\r\n")));
            bRet = FALSE;
        }

        OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
        OUTREG16(&pHSI2CReg->HIIMR, 0xFFFF);
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IIEN), CSP_BITFVAL(I2C_HICR_IIEN, I2C_HICR_IIEN_DISABLE));
    }
out:    
    CeSetThreadPriority(GetCurrentThread(), i);
    return bRet;
}

//------------------------------------------------------------------------------
//
// Function: DeinitChannelDMA
//
//  This function de-initializes the DMA channel for output.
//
// Parameters:
//        None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
VOID HSI2CClass::DeInitChannelDMA(void)
{    
   if (m_dmaChanRx != 0)
   {
       DDKSdmaCloseChan(m_dmaChanRx);
       m_dmaChanRx = 0;
   }
   if (m_dmaChanTx != 0)
   {
       DDKSdmaCloseChan(m_dmaChanTx);
       m_dmaChanTx = 0;
   }
}
//------------------------------------------------------------------------------
//
// Function: InitChannelDMA
//
//  This function initializes the DMA channel for output.
//
// Parameters:
//  Index
//      HSI2C device Index
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL HSI2CClass::InitChannelDMA(UINT32 Index)
{
    UINT8 dmaChannelPriority[2];
    BOOL rc = FALSE;
    Index = Index;

    DEBUGMSG(ZONE_FUNCTION,(_T("HSI2C::InitChannelDMA+\r\n")));

    // Check if DMA buffer has been allocated
    if (!PhysDMABufferAddr.LowPart || !pVirtDMABufferAddr)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA: Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    dmaChannelPriority[0] = SDMA_CHNPRI_CHNPRI_HIGHEST-3;
    dmaChannelPriority[1] = SDMA_CHNPRI_CHNPRI_HIGHEST-2;
    
    // Open virtual DMA channels 
    m_dmaChanRx = DDKSdmaOpenChan(m_dmaReqRx, dmaChannelPriority[1]); 
    if (!m_dmaChanRx)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): SdmaOpenChan for input failed.\r\n")));
        goto cleanUp;
    }
    DEBUGMSG(ZONE_FUNCTION,(_T("Channel Allocated(Rx) : %d\r\n"),m_dmaChanRx));
    
    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(m_dmaChanRx, 1))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaAllocChain for input failed.\r\n")));
        goto cleanUp;
    }  

    // Initialize the chain and set the watermark level     
    if (!DDKSdmaInitChain(m_dmaChanRx, 2))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Rx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    // Open virtual DMA channels 
    m_dmaChanTx = DDKSdmaOpenChan(m_dmaReqTx, dmaChannelPriority[0]); 
    if (!m_dmaChanTx)
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): SdmaOpenChan(Rx) for input failed.\r\n")));
        goto cleanUp;
    }
    DEBUGMSG(ZONE_FUNCTION,(_T("Channel Allocated(Tx) : %d\r\n"),m_dmaChanTx));
    
    // Allocate DMA chain buffer
    if (!DDKSdmaAllocChain(m_dmaChanTx, 1))
    {
         DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaAllocChain for input failed.\r\n")));
         goto cleanUp;
    }  

    // Initialize the chain and set the watermark level 
    if (!DDKSdmaInitChain(m_dmaChanTx, 2))
    {
        DEBUGMSG(ZONE_ERROR, (_T("ERROR:InitChannelDMA(Tx): DDKSdmaInitChain failed.\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
   if (!rc)
   {
      DeInitChannelDMA();
   }
   DEBUGMSG(ZONE_FUNCTION,(_T("HSI2C::InitChannelDMA-\r\n")));
   return rc;
}
//------------------------------------------------------------------------------
//
// Function: UnmapDMABuffers
//
//  This function unmaps the DMA buffers previously mapped with the
//  MapDMABuffers function.
//
// Parameters:
//        None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
VOID HSI2CClass::UnMapDMABuffers(void)
{
    DMA_ADAPTER_OBJECT Adapter;
    DEBUGMSG(ZONE_FUNCTION,(_T("HSI2C::UnmapDMABuffers+\r\n")));

    if(pVirtDMABufferAddr)
    {
        memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
        // Logical address parameter is ignored
        PhysDMABufferAddr.QuadPart = 0;
        HalFreeCommonBuffer(&Adapter, 0, PhysDMABufferAddr, (PVOID)pVirtDMABufferAddr, FALSE);
    }

}
//------------------------------------------------------------------------------
//
// Function: MapDMABuffers
//
// Allocate and map DMA buffer 
//
// Parameters:
//        None
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL HSI2CClass::MapDMABuffers(void)
{
   DMA_ADAPTER_OBJECT Adapter;
   DEBUGMSG(ZONE_FUNCTION,(_T("HSI2C::MapDMABuffers+\r\n")));
      
   pVirtDMABufferAddr = NULL;
   
   memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
   Adapter.InterfaceType = Internal;
   Adapter.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

   // Allocate a block of virtual memory (physically contiguous) for the DMA buffers.
   pVirtDMABufferAddr = (PBYTE)HalAllocateCommonBuffer(&Adapter, (0x1000)
                                , &(PhysDMABufferAddr), FALSE);

   if (pVirtDMABufferAddr == NULL)
   {
      DEBUGMSG(ZONE_ERROR, (TEXT("HSI2C: MapDMABuffers() - Failed to allocate DMA buffer.\r\n")));
      return(FALSE);
   }

   DEBUGMSG(ZONE_FUNCTION,(_T("HSI2C::MapDMABuffers-\r\n")));
   return(TRUE);
}
//------------------------------------------------------------------------------
//
// Function: DeInitDMA
//
//  Performs DMA channel de-intialization
//
// Parameters:
//      None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
VOID HSI2CClass::DeInitDMA(void) 
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HSI2C: DeInitDMA+\r\n")));
    
    // De-Initialize the output DMA
    DeInitChannelDMA();
    
    // UnMap the DMA buffers
    UnMapDMABuffers();
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HSI2C::InitDMA-\r\n")));
}
//------------------------------------------------------------------------------
//
// Function: InitDMA
//
//  Performs DMA channel intialization
//
// Parameters:
//  Index
//      HSI2C device Index
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL HSI2CClass::InitDMA(UINT32 Index) 
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("HSI2C: InitDMA+\r\n")));
    Index = Index;

    m_dmaReqTx = HSI2CGetDMATxReq(Index); 
    m_dmaReqRx = HSI2CGetDMARxReq(Index); 

    // Map the DMA buffers into driver's virtual address space
    if (!MapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("HSI2C:InitDMA() - Failed to map DMA buffers.\r\n")));
        return FALSE;
    }

    // Initialize the output DMA
    if (!InitChannelDMA(Index))
    {
        UnMapDMABuffers();
        DEBUGMSG(ZONE_ERROR, (TEXT("HSI2C:InitDMA() - Failed to initialize output DMA.\r\n")));
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("HSI2C::InitDMA-\r\n")));
    return TRUE ; 
}

//-----------------------------------------------------------------------------
//
// Function: CalculateClkRateDiv
//
// This function will, on obtaining the frequency, determines the nearest clock
// rate divider needed to step the I2C Bus up/down. DO NOT CALL THIS FUNCTION
// directly. The recommended method is to use SDK function I2CSetFrequency().
//
// Parameters:
//      dwFrequency
//          [in] Contains the desired clock frequency of the slave device.
//
// Returns:
//      Returns an index to the array wI2CClockRateDivider. The content in the
//      index holds the nearest clock rate divider available.
//
//-----------------------------------------------------------------------------
WORD HSI2CClass::CalculateClkRateDiv(DWORD dwFrequency)
{
    INT iFirstRd, iSecondRd;
    WORD wEstDivider;
    DWORD freq;
    BYTE byCRDALen;
    const WORD *point;
    
    EnterCriticalSection(&gcsHSI2CDataLock);
    
    dwFreq = dwFrequency;
    if(m_bHSMode)
    {
        byCRDALen = I2CHSDIVTABSIZE;
        point = wI2CHSClockRateDivider;
    }
    else
    {
        byCRDALen = I2CFSDIVTABSIZE;
        point = wI2CFSClockRateDivider;
    }

    // Obtain an estimate of the divider required
    BSPHSI2CGetModuleClock(m_iModuleIndex, &freq);
    wEstDivider = (WORD)(freq / dwFrequency);

    // Tolerance control, the look for frequency shall never exceed target frequency +1%
    if ((freq-(dwFrequency*wEstDivider)) * 100 > dwFrequency)
    {
        ++wEstDivider;
    }
    // RETAILMSG(TRUE,(_T("wEstDivider=0x%x,freq=0x%x,dwFrequency=0x%x\r\n"),wEstDivider,freq,dwFrequency));
    // Search for the nearest divider in the first half of the array
    for (iFirstRd = 0; iFirstRd < (byCRDALen/2-1); iFirstRd++)
    {
        // Once found a divider greater than the estimate, stop
        if (wEstDivider <= *(point + iFirstRd))
            break;
    }
    if (wEstDivider == *(point + iFirstRd))
    {
        // If the estimated divider matched one of the array entries, no need
        // to search further
        wEstDivider = (WORD)iFirstRd;
    }
    else
    {
        // Going to second round
        for (iSecondRd = (byCRDALen/2); iSecondRd < (byCRDALen-1); iSecondRd++)
        {
            // Again, if a greater entry is found, stop
            if (wEstDivider <= *(point + iSecondRd))
                break;
        }
        if (wEstDivider == *(point + iSecondRd))
        {
            // If the estimated divider is found in the second round, stop
            wEstDivider = (WORD)iSecondRd;
        }
        else
        {
            // Search for the nearest divider among the 2 portion of the array
            if ((*(point + iFirstRd) > wEstDivider) && (*(point + iSecondRd) > wEstDivider))
            {
                if ((*(point + iFirstRd) - wEstDivider) < (*(point + iSecondRd) - wEstDivider))
                    wEstDivider = (WORD)iFirstRd;
                else
                    wEstDivider = (WORD)iSecondRd;
            }
            else
            {
                if (*(point + iSecondRd) > wEstDivider)
                {
                    wEstDivider = (WORD)iSecondRd;
                }
                else
                {
                    // Less than setting, use wI2CClockRateDivider[31] as default
                    wEstDivider = (WORD)iSecondRd;
                }
            }
        }
    }
    LeaveCriticalSection(&gcsHSI2CDataLock);
    // Obtain the nearest clock rate divider
    return wEstDivider;   
}
//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::HSI2CClass
//
// This HSI2CClass constructor creates all the mutexes, events and heaps required
// for the subsequent use of I2C bus interface. It will also allocate the
// interrupt id from the HAL and binds the interrupt event to the interrupt id.
// This constructor has a built-in mechanism to prevent concurrent execution and a
// multiple interrupt binding procedure.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------

HSI2CClass::HSI2CClass(UINT32 index)
{
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass +\r\n")));

    m_iModuleIndex = index;
    m_bDMAMode = FALSE;     //HW workaround. SDMA conflict with ESDHC? TBD
    
    // I2C Bus Critical Section
    {
        InitializeCriticalSection(&gcsHSI2CBusLock);
        InitializeCriticalSection(&gcsHSI2CDataLock);
    }
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass:InitializeCriticalSection(): Creating I2C Bus Critical Section! \r\n")));
    
    // Mapped I2C Register Base Physical Address -> Virtual Address +
    {
        PHYSICAL_ADDRESS phyAddr;

        // Copy I2C physical address
        phyAddr.QuadPart = HSI2CGetBaseRegAddr(m_iModuleIndex);
        // Map to virtual address
        pHSI2CReg = (PCSP_HSI2C_REG) MmMapIoSpace(phyAddr, sizeof(CSP_HSI2C_REG), FALSE);

        // If mapping fails, fatal error, quit
        if (pHSI2CReg == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("HSI2CClass::HSI2CClass:MmMapIoSpace(): Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = HSI2C_ERR_PA_VA_MISSING;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass:MmMapIoSpace(): pHSI2CReg=0x%x \r\n"), pHSI2CReg));

    // Configure IOMUX for I2C pins
    if (!BSPHSI2CIOMUXConfig(m_iModuleIndex))
    {
        DEBUGMSG(ZONE_ERROR, 
            (TEXT("%s: Error configuring IOMUX for I2C.\r\n"), __WFUNCTION__));
        return;
    }
    BSPHSI2CEnableClock(m_iModuleIndex, TRUE);
    
    // Create Hardware Interrupt Occurrence Event
    {
        hInterrupted = CreateEvent(NULL, FALSE, FALSE, NULL);
        // Able to create or obtain the event?
        if (hInterrupted == NULL)
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("HSI2CClass::HSI2CClass:CreateEvent(): Interrupt Occurrence Event (hardware) Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = HSI2C_ERR_EOPS_CREATE;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass:CreateEvent(): hInterrupted=0x%x \r\n"), hInterrupted));

    // Map IRQ -> System Interrupt ID
    {
        // Copy our I2C IRQ Number
        DWORD dwIrq = HSI2CGetIRQ(m_iModuleIndex);

        // Get kernel to translate IRQ -> System Interrupt ID
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(DWORD), &dwSysIntr, sizeof(DWORD), NULL))
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("HSI2CClass::HSI2CClass:KernelIoControl(): IRQ -> SysIntr Failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = HSI2C_ERR_IRQ_SYSINTR_MISSING;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass:KernelIoControl(): dwSysIntr=0x%x \r\n"), dwSysIntr));

    // Link hInterrupted -> I2C Interrupt Pin
    {
        if (!InterruptInitialize(dwSysIntr, hInterrupted, NULL, 0))
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("HSI2CClass::HSI2CClass:Interruptinitialize(): Linking failed! ErrCode=%d \r\n"), GetLastError()));
            iResult = HSI2C_ERR_INT_INIT;
            return;
        }
    }
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass:Interruptinitialize(): Linking passed! \r\n")));

    // Disable I2C Module Initially
    {
       OUTREG16(&pHSI2CReg->HICR, 0);
       Sleep(10);
    }
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass:I2C_WORD_OUT(): I2CR=0x0 \r\n")));
    
    if(m_bDMAMode)
    {
        // Initial DMA
        InitDMA(m_iModuleIndex);
    }
    
    // Initialize I2C Mode
    byLastMode = HSI2C_MASTER_MODE;
    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass: Default to Master Mode \r\n")));

    // Initialize I2C default address and clock.
    byLastSelfAddr= (BYTE)EXTREG16BF(&pHSI2CReg->HISADR, I2C_SADR_ADR);
    wLastClkRate= EXTREG16BF(&pHSI2CReg->HIFSFDR, I2C_FSFDR_IC);
    
    // Disable I2C Clock
    BSPHSI2CEnableClock(m_iModuleIndex, FALSE); 
       
    // Default Mode is interrupt
    m_dxCurrent = D0;
    m_bHSMode = 0;
    
    // After this point, all initialization routines completed
    iResult = HSI2C_NO_ERROR;

    DEBUGMSG(ZONE_INIT, (TEXT("HSI2CClass::HSI2CClass -\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::~HSI2CClass
//
// This HSI2CClass destructor releases all the mutexes, events and heaps created.
// It will also attempt to terminate the worker thread. It has built-in 
// mechanism to determine whether it is safe to unbind the interrupt event from
// the interrupt id. This is to facilitate situations where multiple
// processes have obtained the same file handle to the I2C Bus Interface.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
    
HSI2CClass::~HSI2CClass(void)
{
    DEBUGMSG(ZONE_DEINIT, (TEXT("HSI2CClass::~HSI2CClass +\r\n")));

    // Release the interrupt resources
    InterruptDisable(dwSysIntr);
    DEBUGMSG(ZONE_DEINIT, (TEXT("HSI2CClass::~HSI2CClass: Release System Interrupt \r\n")));

    // Release the interrupt
    DEBUGMSG(ZONE_DEINIT, (TEXT("HSI2CClass::~HSI2CClass(): Releasing dwSysIntr = 0x%x \r\n"), dwSysIntr));
    if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntr, sizeof(DWORD), NULL, 0, NULL))
        DEBUGMSG(ZONE_DEINIT | ZONE_ERROR, (TEXT("ERROR: Failed to release dwSysIntr.\r\n")));                

    DEBUGMSG(ZONE_DEINIT, (TEXT("HSI2CClass::~HSI2CClass:DeleteCriticalSection(): Deleting I2C_IOControl Critical Section! \r\n")));

    // I2C Bus Critical Section
    {
        DeleteCriticalSection(&gcsHSI2CBusLock);
        DeleteCriticalSection(&gcsHSI2CDataLock);
    }

    // Release Interrupt Occurrence Event -
    {   
        if (hInterrupted != NULL)
            CloseHandle(hInterrupted);

        DEBUGMSG(ZONE_DEINIT, (TEXT("HSI2CClass::~HSI2CClass: Release Interrupt Occurence Event \r\n")));
    }
    
    // de init DMA
    DeInitDMA();
    
    // Release I2C Register Base Mapped Virtual Memory -
    {
        if (pHSI2CReg != NULL)
            MmUnmapIoSpace((LPVOID) pHSI2CReg, sizeof(CSP_HSI2C_REG));

        DEBUGMSG(ZONE_DEINIT, (TEXT("HSI2CClass::~HSI2CClass: Release I2C Register Base Mapped Virtual Memory \r\n")));
    }

    DEBUGMSG(ZONE_DEINIT, (TEXT("HSI2CClass::~HSI2CClass -\r\n")));
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::Reset
//
// This method performs a software reset on HSI2C internal register (HICR). It is
// important to take note that the fields of the HSI2CClass are not modified in
// any way.
//
// Parameters:
//  None.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------

VOID HSI2CClass::Reset(void)
{
    EnterCriticalSection(&gcsHSI2CBusLock);
    OUTREG16(&pHSI2CReg->HICR, 0x0);
    iResult = HSI2C_NO_ERROR;
    LeaveCriticalSection(&gcsHSI2CBusLock);
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::GenerateStop
//
// This method generates a stop bit on the I2C bus
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID HSI2CClass::GenerateStop()
{
    if(!HSI2CWaitForBusStatus(I2C_HISR_IBB_BUSY, 1))
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("%s, bus not busy\r\n"),__FUNCTION__));
        return;
    }
    
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MSTA),
             CSP_BITFVAL(I2C_HICR_MSTA, I2C_HICR_MSTA_SLAVE));
             
    if(!HSI2CWaitForBusStatus(I2C_HISR_IBB_IDLE))
    {
        DEBUGMSG(ZONE_FUNCTION, (_T("%s, bus not idle after stop\r\n"),__FUNCTION__));
        return;
    }
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::WritePacket
//
// This method performs a write operation with the data in one HSI2C_PACKET.
//
// Parameters:
//      pI2CPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//      bFirst
//          [in] Signifies if this is the first packet to be transmitted.
//
//      bLast
//          [in] Signifies if this is the last packet to be transmitted.
//
// Returns:  
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::WritePacketDMA(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast)
{
    if(bFirst)
    {
        MasterModeInit(FALSE);
    }
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_TXAK), CSP_BITFVAL(I2C_HICR_TXAK, I2C_HICR_TXAK_ACK_SEND));
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MTX), CSP_BITFVAL(I2C_HICR_MTX, I2C_HICR_MTX_TRANSMIT));
    
    if(bLast)
    {
        OUTREG16(&pHSI2CReg->HITDCR, CSP_BITFVAL(I2C_HITDCR_TDCE, I2C_HITDCR_TDCE_ENABLE) | packets->wLen);
    }
    else
    {
        OUTREG16(&pHSI2CReg->HITDCR, CSP_BITFVAL(I2C_HITDCR_TDCR, I2C_HITDCR_TDCR_RESTART) | CSP_BITFVAL(I2C_HITDCR_TDCE, I2C_HITDCR_TDCE_ENABLE) | packets->wLen);
    }
    
    INSREG16(&pHSI2CReg->HITFR, CSP_BITFMASK(I2C_HITFR_TFEN), CSP_BITFVAL(I2C_HITFR_TFEN, I2C_HITFR_TFEN_ENABLE));
    INSREG16(&pHSI2CReg->HITFR, CSP_BITFMASK(I2C_HITFR_TFLSH), CSP_BITFVAL(I2C_HITFR_TFLSH, I2C_HITFR_TFLSH_RESET));
    
    // configure TX DMA
    DDKSdmaClearBufDescStatus(m_dmaChanTx,0);
    DDKSdmaSetBufDesc(m_dmaChanTx, 0, 
        DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP,
        PhysDMABufferAddr.LowPart,
        0, DDK_DMA_ACCESS_8BIT, (UINT16)packets->wLen); // set the count in bytes.

    memcpy(pVirtDMABufferAddr, packets->pbyBuf, packets->wLen);
    
    // start Tx DMA channel
    DDKSdmaStartChan(m_dmaChanTx);
    
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_DMATX), CSP_BITFVAL(I2C_HICR_DMATX, I2C_HICR_DMATX_ENABLE));
    
    if(bFirst)
    {
        if(!GenerateStart(packets->byAddr, bFirst))
        {
            DEBUGMSG(ZONE_ERROR, (_T("%s generate start failed\r\n"),__FUNCTION__));
            (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
            return FALSE;
        }
    }
    
    if(!HSI2CWaitForTDC())
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s HSI2CWaitForTDC failed\r\n"),__FUNCTION__));
        (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
        DDKSdmaStopChan(m_dmaChanTx, TRUE);
        return FALSE;
    }
    
    DDKSdmaStopChan(m_dmaChanTx, TRUE);
    
    if (!HSI2CCheckArbitLost())
    {
        (*(packets->lpiResult)) = HSI2C_ERR_ARBITRATION_LOST;
        return FALSE;
    }
    
    if(!HSI2CCheckAckStatus())
    {
        (*(packets->lpiResult)) = HSI2C_ERR_NO_ACK_ISSUED;
        GenerateStop();
        return FALSE;
    } 
    
    return TRUE;
}

BOOL HSI2CClass::WritePacketFIFO(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast)
{
    int num = 0;
    
    if(bFirst)
    {
        MasterModeInit(FALSE);
    }
    else
    {
        OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
    }
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_TXAK), CSP_BITFVAL(I2C_HICR_TXAK, I2C_HICR_TXAK_ACK_SEND));
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MTX), CSP_BITFVAL(I2C_HICR_MTX, I2C_HICR_MTX_TRANSMIT));
    
    if(bFirst)
    {
        OUTREG16(&pHSI2CReg->HITDR, packets->pbyBuf[num]);
        num++;
    }
    if(!GenerateStart(packets->byAddr, bFirst))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s generate start failed\r\n"),__WFUNCTION__));
        (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
        return FALSE;
    }
    
    for (int i = num; i < packets->wLen; i++)
    {
        OUTREG16(&pHSI2CReg->HITDR, packets->pbyBuf[i]);
    }
    
    if(!HSI2CWaitForTDE())
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s HSI2CWaitForTDC failed\r\n"),__WFUNCTION__));
        (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
        GenerateStop();
        return FALSE;
    }
    
    if (!HSI2CCheckArbitLost())
    {
        (*(packets->lpiResult)) = HSI2C_ERR_ARBITRATION_LOST;
        GenerateStop();
        return FALSE;
    }
    
    if(!HSI2CCheckAckStatus())
    {
        (*(packets->lpiResult)) = HSI2C_ERR_NO_ACK_ISSUED;
        GenerateStop();
        return FALSE;
    }
    if(bLast)
    {
        GenerateStop();
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::MasterModeInit
//
// This method performs initialize for master mode transfer.
//
// Parameters:
//      BitMode
//          [in] indicates 7/10 bit mode.
//
// Returns:  
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::MasterModeInit(BOOL BitMode)
{
    EnterCriticalSection(&gcsHSI2CDataLock);
    BitMode = BitMode;

    // Reset I2CR
    HSI2CWaitForBusStatus(I2C_HISR_IBB_IDLE);
    OUTREG16(&pHSI2CReg->HICR, 0x0);
    
    if(m_bHSMode)
    {
        // Configure data sampling rate
        OUTREG16(&pHSI2CReg->HIHSFDR, CSP_BITFVAL(I2C_HSFDR_IC, wLastClkRate));
    }
    else
    {
        // Configure data sampling rate
        OUTREG16(&pHSI2CReg->HIFSFDR, CSP_BITFVAL(I2C_FSFDR_IC, wLastClkRate));
    }
    // Configure self address
    INSREG16(&pHSI2CReg->HISADR, CSP_BITFMASK(I2C_SADR_ADR), CSP_BITFVAL(I2C_SADR_ADR, 0x30));
    
    // Enable I2C
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_IEN), CSP_BITFVAL(I2C_HICR_IEN, I2C_HICR_IEN_ENABLE));
    
    // Configure 7/10 bit mode
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_ADDR), CSP_BITFVAL(I2C_HICR_ADDR, I2C_HICR_ADDR_7BIT));

    // Configure FS/HS mode
    if(m_bHSMode)
    {
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_HSM), CSP_BITFVAL(I2C_HICR_HSM, I2C_HICR_HSM_ENABLE));
    }
    else
    {
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_HSM), CSP_BITFVAL(I2C_HICR_HSM, I2C_HICR_HSM_DISABLE));
    }

    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MST), CSP_BITFVAL(I2C_HICR_MST, 0x1));     //???
    // Clear status register
    OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
    
    INSREG16(&pHSI2CReg->HITFR, CSP_BITFMASK(I2C_HITFR_TFEN), CSP_BITFVAL(I2C_HITFR_TFEN, I2C_HITFR_TFEN_ENABLE));
    INSREG16(&pHSI2CReg->HIRFR, CSP_BITFMASK(I2C_HIRFR_RFEN), CSP_BITFVAL(I2C_HIRFR_RFEN, I2C_HIRFR_RFEN_ENABLE));
    
    
    LeaveCriticalSection(&gcsHSI2CDataLock);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: I2CClass::GenerateStart
//
// This method performs generates an I2C START signal to a slave device.
//
// Parameters:
//      Flag
//          [in] 1: start
//               0: repeat start
// Returns:  
//      TRUE   Success
//      FALSE  Error or timeout.
//
//-----------------------------------------------------------------------------

BOOL HSI2CClass::GenerateStart(BYTE SlaveAddr, BOOL flag)
{
    if(flag)
    {
        //it's a start, bus status should be idle
        if(!HSI2CWaitForBusStatus(I2C_HISR_IBB_IDLE))
        {
            return FALSE;
        }
    }
    else
    {
        //it's a repeat start, bus status should be busy
        if(!HSI2CWaitForBusStatus(I2C_HISR_IBB_BUSY))
        {
            return FALSE;
        }
    }
    if(flag)
    {
        // Configure slave to be addressed
        INSREG16(&pHSI2CReg->HIMADR, CSP_BITFMASK(I2C_MADR_ADR), CSP_BITFVAL(I2C_MADR_ADR, SlaveAddr));
    
        // start, Grant Bus Master
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MSTA),
                 CSP_BITFVAL(I2C_HICR_MSTA, I2C_HICR_MSTA_MASTER));
    }
    else
    {
        if(!HSI2CWaitForRestartStatus(I2C_HICR_RSTA_WITHHOLD))
        {
            return FALSE;   
        }
        // Configure slave to be addressed
        INSREG16(&pHSI2CReg->HIMADR, CSP_BITFMASK(I2C_MADR_ADR), CSP_BITFVAL(I2C_MADR_ADR, SlaveAddr));
    
        // repeat start, Grant Bus Master
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_RSTA),
             CSP_BITFVAL(I2C_HICR_RSTA, I2C_HICR_RSTA_GENERATE));
             
        if(!HSI2CWaitForRestartStatus(I2C_HICR_RSTA_WITHHOLD))
        {
            return FALSE;   
        }
    }
    
    // Ensure the START operation is success
    if (!HSI2CCheckArbitLost())
    {
        DEBUGMSG(ZONE_ERROR, (_T("IAL detect in GenerateStart\r\n")));
        return FALSE;
    }
    // Ensure the START operation is success
    if (EXTREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MSTA),
                 I2C_HICR_MSTA_LSH) != I2C_HICR_MSTA_MASTER)
    {
        DEBUGMSG(ZONE_ERROR, (_T("Master mode set failure\r\n")));
        return FALSE;
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::ReadPacket
//
// This method performs a read operation with the data in one HSI2C_PACKET.
//
// Parameters:
//      pI2CPkt
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      bFirst
//          [in] Signifies if this is the first packet to be transmitted.
//
//      bLast
//          [in] Signifies if this is the last packet to be transmitted.
//
// Returns:  
//      TRUE   Success.
//      FALSE  Arbitration lost or timeout error.
//
//-----------------------------------------------------------------------------

BOOL HSI2CClass::ReadPacketDMA(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast)
{
    if(bFirst)
    {
        MasterModeInit(FALSE);
    }
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MTX), CSP_BITFVAL(I2C_HICR_MTX, I2C_HICR_MTX_RECEIVE));
    
    if (packets->wLen == 1)
    {
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_TXAK), CSP_BITFVAL(I2C_HICR_TXAK, I2C_HICR_TXAK_NO_ACK_SEND));
    }
    else
    {
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_TXAK), CSP_BITFVAL(I2C_HICR_TXAK, I2C_HICR_TXAK_ACK_SEND));
    }
    
    if(bLast)
    {
        OUTREG16(&pHSI2CReg->HIRDCR, CSP_BITFVAL(I2C_HIRDCR_RDCE, I2C_HIRDCR_RDCE_ENABLE) | packets->wLen);
    }
    else
    {
        OUTREG16(&pHSI2CReg->HIRDCR, CSP_BITFVAL(I2C_HIRDCR_RDCR, I2C_HIRDCR_RDCR_RESTART) | CSP_BITFVAL(I2C_HIRDCR_RDCE, I2C_HIRDCR_RDCE_ENABLE) | packets->wLen);
    }
    
    INSREG16(&pHSI2CReg->HIRFR, CSP_BITFMASK(I2C_HIRFR_RFEN), CSP_BITFVAL(I2C_HIRFR_RFEN, I2C_HIRFR_RFEN_ENABLE));
    INSREG16(&pHSI2CReg->HIRFR, CSP_BITFMASK(I2C_HIRFR_RFLSH), CSP_BITFVAL(I2C_HIRFR_RFLSH, I2C_HIRFR_RFLSH_RESET));
    
    // configure RX DMA
    DDKSdmaClearBufDescStatus(m_dmaChanRx,0);
    DDKSdmaSetBufDesc(m_dmaChanRx, 0, 
        DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_WRAP,
        PhysDMABufferAddr.LowPart,
        0, DDK_DMA_ACCESS_8BIT, (UINT16)packets->wLen); // set the count in bytes.

    // start Rx DMA channel
    DDKSdmaStartChan(m_dmaChanRx);
    
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_DMARX), CSP_BITFVAL(I2C_HICR_DMARX, I2C_HICR_DMARX_ENABLE));
    
    if(bFirst)
    {
        if(!GenerateStart(packets->byAddr, bFirst))
        {
            DEBUGMSG(ZONE_ERROR, (_T("%s generate start failed\r\n"),__FUNCTION__));
            (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
            return FALSE;
        }
    }
    
    if(!HSI2CWaitForRDC())
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s HSI2CWaitForRDC failed\r\n"),__FUNCTION__));
        (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
        DDKSdmaStopChan(m_dmaChanRx, TRUE);
        return FALSE;
    }
    {
        //Also check DMA status here
        UINT32 RxStatus = 0;
        UINT32 Counter = 0;
        DDKSdmaGetBufDescStatus(m_dmaChanRx, 0, &RxStatus);
        while ((RxStatus & DDK_DMA_FLAGS_BUSY))
        {
            DDKSdmaGetBufDescStatus(m_dmaChanRx, 0, &RxStatus);
            Counter++;
            if(Counter > 1000)
                break;
        }
        if(Counter > 1000)
        {
            (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
            DDKSdmaStopChan(m_dmaChanRx, TRUE);
            return FALSE;
        }
    }
    DDKSdmaStopChan(m_dmaChanRx, TRUE);
    memcpy(packets->pbyBuf, pVirtDMABufferAddr, packets->wLen);
    
    if (!HSI2CCheckArbitLost())
    {
        (*(packets->lpiResult)) = HSI2C_ERR_ARBITRATION_LOST;
        return FALSE;
    }
    
    if(!HSI2CCheckAckStatus())
    {
        (*(packets->lpiResult)) = HSI2C_ERR_NO_ACK_ISSUED;
        GenerateStop();
        return FALSE;
    } 
    
    return TRUE;
}

BOOL HSI2CClass::ReadPacketFIFO(PHSI2C_PACKET packets, BOOL bFirst, BOOL bLast)
{
    int i = 0;
    
    if(bFirst)
    {
        MasterModeInit(FALSE);
    }
    else
    {
        OUTREG16(&pHSI2CReg->HISR, 0xFFFF);
    }
    INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_MTX), CSP_BITFVAL(I2C_HICR_MTX, I2C_HICR_MTX_RECEIVE));
    
    if (packets->wLen == 1)
    {
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_TXAK), CSP_BITFVAL(I2C_HICR_TXAK, I2C_HICR_TXAK_NO_ACK_SEND));
    }
    else
    {
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_TXAK), CSP_BITFVAL(I2C_HICR_TXAK, I2C_HICR_TXAK_ACK_SEND));
    }
    
    INSREG16(&pHSI2CReg->HIRFR, CSP_BITFMASK(I2C_HIRFR_RFWM), CSP_BITFVAL(I2C_HIRFR_RFWM, packets->wLen - 1));
        
    if(!GenerateStart(packets->byAddr, bFirst))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s generate start failed\r\n"),__FUNCTION__));
        (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
        return FALSE;
    }
    
    if(!HSI2CWaitForRDF(packets->wLen))
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s HSI2CWaitForRDC failed\r\n"),__FUNCTION__));
        (*(packets->lpiResult)) = HSI2C_ERR_STATEMENT_CORRUPT;
        return FALSE;
    }
    
    if (packets->wLen != 1)
    {
        INSREG16(&pHSI2CReg->HICR, CSP_BITFMASK(I2C_HICR_TXAK), CSP_BITFVAL(I2C_HICR_TXAK, I2C_HICR_TXAK_NO_ACK_SEND));
        OUTREG16(&pHSI2CReg->HISR, 1 << I2C_HISR_BTD_LSH);
    
        for (i = 0; i < packets->wLen - 1; i++)
        {
            packets->pbyBuf[i] = (BYTE) INREG16(&pHSI2CReg->HIRDR);
        }
    
        if(!HSI2CWaitAndClearByteTransfer())
        {
            //RETAILMSG(TRUE,(_T("HSI2CWaitAndClearByteTransfer timout!\r\n")));
        }
    }
    packets->pbyBuf[i] = (BYTE) INREG16(&pHSI2CReg->HIRDR);
    
    if (!HSI2CCheckArbitLost())
    {
        (*(packets->lpiResult)) = HSI2C_ERR_ARBITRATION_LOST;
        return FALSE;
    }
    if(bLast)
    {
        GenerateStop();
    }
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: HSI2CClass::ProcessPackets
//
// This is the main engine that transmits or receives data from I2C Bus
// Interface. This engine implements the complete I2C Bus Protocol which allows
// the calling process to interact with all I2C-compliant slave device. This
// method has built-in mechanism to prevent concurrent execution.
//
// Parameters:
//      pPacket 
//          [in] Contains all the necessary information to transmit/
//          receive from the slave device.
//
//      dwNumPackets
//          [in] Number of packets to be processed.
//
// Returns:  
//      TRUE   Success.
//      FALSE  Failed due to any one of the following conditions:
//               - failed to enable the I2C clock
//               - GenerateStart() call returned an error
//               - timed out waiting for interrupt notification
//               - arbitration lost
//               - transfer operation timed out
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::ProcessPackets(HSI2C_PACKET packets[], INT32 numPackets)
{  
    BOOL retVal = TRUE;
    DWORD nsToStall;
    // Must gain ownership to bus lock mutex
    EnterCriticalSection(&gcsHSI2CBusLock);
    BSPHSI2CEnableClock(m_iModuleIndex, TRUE);
    
    for (int i = 0; i < numPackets; i++)
    {
        // Is I2C in master mode?
        if (byLastMode == HSI2C_MASTER_MODE)
        {
            if (packets[i].wLen > 0x8)
            {
                // len is bigger than 0x8, not support recently
                retVal = FALSE;
                *(packets[i].lpiResult) = HSI2C_ERR_INVALID_BUFSIZE;
                SetLastError(ERROR_INVALID_PARAMETER);
                goto __exit;
            }
            if ((packets[i].byRW & HSI2C_METHOD_MASK) == HSI2C_RW_WRITE)
            {
                //Note: high speed i2c seems to have repeat start problem.
                //   temporily, we generate start and stop for every
                //   transfer
                
                if(m_bDMAMode)
                {
                    if (!WritePacketDMA(&packets[i], 1, 1))
                    {
                        retVal = FALSE;
                        goto __exit;
                    }
                }
                else
                {
                    if (!WritePacketFIFO(&packets[i], i == 0, (i + 1 == numPackets)))
                    //if (!WritePacketFIFO(&packets[i], 1, 1))
                    {
                        retVal = FALSE;
                        goto __exit;
                    }
                }
            }
            else
            {
                if(m_bDMAMode)
                {
                    if (!ReadPacketDMA(&packets[i], 1, 1))
                    {
                        retVal = FALSE;
                        goto __exit;
                    }
                }
                else
                {
                    if (!ReadPacketFIFO(&packets[i], i == 0, (i + 1 == numPackets)))
                    //if (!ReadPacketFIFO(&packets[i], 1, 1))
                    {
                        retVal = FALSE;
                        goto __exit;
                    }
                }
            }

            *(packets[i].lpiResult) = HSI2C_NO_ERROR;
        }
        nsToStall = 1000000 * 10 / dwFreq; //stall 10 clk cycles for safty
        StallExecution(nsToStall);
    }
__exit:

    if(!HSI2CWaitForBusStatus(I2C_HISR_IBB_IDLE))
    {
        //RETAILMSG(TRUE, (_T("disable HSI2C module error\r\n")));
    }
    
    // Disable I2C Module
    OUTREG16(&pHSI2CReg->HICR, 0);

    // Disable I2C Clock
    BSPHSI2CEnableClock(m_iModuleIndex, FALSE);
    
    // On completion, release bus lock mutex
    LeaveCriticalSection(&gcsHSI2CBusLock);

    return retVal;   
}

//-----------------------------------------------------------------------------
//
// Function: EnableSlave
//
// This method enable i2c slave from clock gating.
//
// Parameters:
//
// Returns:
//      TRUE:  enabled SLAVE mode
//      FALSE: enable fail 
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::EnableSlave(void)
{
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: DisableSlave
//
// This method disable i2c slave and gate its input clock.
//
// Parameters:
//
// Returns:
//      TRUE:  disabled SLAVE mode
//      FALSE: disable fail 
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::DisableSlave(void)
{
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: SetSlaveText
//
// This method set slave text in slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::SetSlaveText(PBYTE pBufIn, DWORD dwLen)
{
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLen);
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: GetSlaveText
//
// This method get slave text in slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::GetSlaveText(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut)
{
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLen);
    UNREFERENCED_PARAMETER(pdwActualOut);
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: SetSlaveSize
//
// This method set buffer size to the slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::SetSlaveSize(PBYTE pBufIn, DWORD dwLen)
{
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLen);
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: GetSlaveSize
//
// This method set buffer size to the slave interface buffer.
//
// Parameters:
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
BOOL HSI2CClass::GetSlaveSize(PBYTE pBufOut, DWORD dwLen, PDWORD pdwActualOut)
{
    UNREFERENCED_PARAMETER(pBufOut);
    UNREFERENCED_PARAMETER(dwLen);
    UNREFERENCED_PARAMETER(pdwActualOut);
    return FALSE;
}
