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
/**************************************************************************
** Copyright 2000-2003 Intel Corporation. All Rights Reserved.
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

Module Name:  pmuioctl.c

Abstract:
 Contains  IOCTL PMU Functions and Interfaces

**************************************************************************/

#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <bulverde_base_regs.h>
#include <bulverde_intr.h>
#include <xllp_clkmgr.h>
#include <xllp_cpdvm.h>
#include <pmuioctl.h>
#include <pmu.h>         

// CP14/15 Register Access Routines
// (defined in assembly)
//
extern unsigned long ReadPMUReg(unsigned long Regno);
extern void WritePMUReg(unsigned long Regno, unsigned long Value);


ReleaseCCFCallback PVTuneReleaseCCF;
ReleasePMUCallback PVTuneReleasePMU;
PMUInterruptCallback PVTuneInterrupt;
    
static unsigned long savedCCCR;
static unsigned long savedCLKCFG;

BOOL bPMURunning = FALSE;

PPMURegInfo     pPMURegBuf;
PPMUCCFInfo     pPMUCCFBuf;
PCPUIdInfo      pCPUIdBuf;

unsigned long dCCFreqLock = 0;      // Core Clock Frequency lock

BOOL OALIoCtlPMUCCFCall(
  UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
  UINT32 outSize, UINT32 *pOutSize)
{             
    DWORD len;
    BOOL retval=FALSE;
    unsigned long LMult,NMult,CLKCFGReg,CLKCFGMask;
    volatile XLLP_CLKMGR_T  *v_pCLKReg=(volatile XLLP_CLKMGR_T*)OALPAtoVA(BULVERDE_BASE_REG_PA_CLKMGR, FALSE);      

    if ((pInpBuffer == NULL) || (inpSize != sizeof(PMUCCFInfo)))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    pPMUCCFBuf = (PPMUCCFInfo) pInpBuffer;
    pPMUCCFBuf->curFreq = 0;
    
    // Check PMU CCF control code
    //
    switch (pPMUCCFBuf->subcode)
    {
    
        case PMU_CCF_GETCURRENT:
      
            //
            // Extract the L and N fields from the
            // Core Clock Status register (CCSR)
            //
            
            LMult = v_pCLKReg->ccsr & 0x0000001F;
            NMult = (v_pCLKReg->ccsr & 0x00000380) >> 7;

            //
            // Compute the current frequency
            // Deal in KHz for PMU
            // NMult = (N * 10)/2
            //
            pPMUCCFBuf->curFreq = (LMult * CRYSTAL_KHZDIVTEN);

            //
            // Read CCCKCFG to determine if we're in turbo mode
            //
            CLKCFGReg = XllpXSC1ReadCLKCFG();

            if (CLKCFGReg & 0x01)
            {
                //
                // Turbo mode set
                //
                pPMUCCFBuf->curFreq *= (NMult*5);
            }
            else
            {
                pPMUCCFBuf->curFreq *= 10;
            }


            if (pPMUCCFBuf->curFreq != 0)
            {
                //
                // Return frequency in output buffer
                //
                len = sizeof(DWORD);
                if ((outSize == len) && (pOutBuffer != NULL) && (pOutSize != NULL))
                {
                    memcpy(pOutBuffer,&(pPMUCCFBuf->curFreq),len);
                    *pOutSize = len;
                    retval = TRUE;
                }
                else
                {
                    NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
                }
             }
             else
             {
                NKSetLastError(ERROR_NOT_SUPPORTED);
             }


        break;


        case PMU_CCF_SETLOCK:
            //
            // Save the current CCCR setting
            //
            savedCCCR = v_pCLKReg->ccsr;

            //
            // Check CPDIS bit.  If we are running in 13MHz mode,
            // other frequency changes are invalid.
            //
            if (savedCCCR & 0x80000000)
            {
                NKSetLastError(ERROR_INVALID_FUNCTION);
                break;
            }

            //
            // Read Clock Configuration CP14, r6 to get
            // curret bus mode and turbo settings
            //
            savedCLKCFG = XllpXSC1ReadCLKCFG();

            //
            // For new frequency, set F and preserve B setting
            //
            CLKCFGMask = 0x2 | (savedCLKCFG & 0x8);

            //
            // No Turbo Mode; Set N for a turbo multiplier of 1 (== Run Mode);
            //
            NMult = 2;

            //
            // Determine the Run mode multiplier (L) for the new Freq
            // PMU frequencies are in KHz.
            //
            LMult = pPMUCCFBuf->newFreq / CRYSTAL_KHZ;

            //
            // For frequencies about 15, must use normal Bus mode
            //
            if (LMult > 15)
            {
                CLKCFGMask = 0x2;     // B= 0 (Normal), F=1
            }

            //
            // Save (already translated) callback address
            //
            PVTuneReleaseCCF = (ReleaseCCFCallback) pPMUCCFBuf->pCallback;

            //
            // Disable all interrupts
            //
            INTERRUPTS_OFF();

            //
            // Disallow Mode change if PMU is already locked.
            //
            if (dCCFreqLock)
            {
                INTERRUPTS_ON();
                NKSetLastError(ERROR_INVALID_FUNCTION);
                break;
            }

            //
            // Setup Lock
            //
            dCCFreqLock++;

            //
            // Set up the new frequency multipliers, N and L
            // CPDIS, PPDIS = 0, enable after freq. change.
            //
            // Check for CPDIS bit is done above and if set
            // frequency change is not allowed.
            //
            v_pCLKReg->cccr =  (NMult << 7) | LMult;

            //
            // Notify frequency sensitive drivers to disable themselves
            //    - Display driver
            //

            //
            // Program the CCLKCFG (CP14, reg6) for a frequency change
            // Parameter is mask to use for CLKCFG register
            //
            XllpXSC1FreqChange(CLKCFGMask);


            //
            // TBD: Resume frequency sensitive drivers
            //


            INTERRUPTS_ON();

            retval = TRUE;


            break;


        case PMU_CCF_UNLOCK:

            //
            // Restore the prior CCCR settings and run mode
            // thereby instantiating another frequency change.
            //

            //
            // Disable all interrupts
            //
            INTERRUPTS_OFF();

            if (dCCFreqLock == 0)
            {
                //
                // Currently no lock taken out
                // so just return
                //
                INTERRUPTS_ON();
                retval = TRUE;
                break;
            }


            //
            // Restore prior CCCR contents
            // (Zero reserved bits)
            //
            v_pCLKReg->cccr = savedCCCR & ~XLLP_CCCR_RESERVED_BITS;

            //
            // Notify frequency sensitive drivers to disable themselves
            //    - Display driver (LCD frequency (K) is derived from L
            //          L = 2-7.   K=1
            //          L = 8-15,  K=4
            //          L = 16-31, K=8
            //

            //
            // Program the CLKCFG (CP14, reg6) for a frequency change
            // Parameter is the saved clock configuration settings to
            // restore proper run and bus modes.
            //
            XllpXSC1FreqChange(savedCLKCFG);

            //
            // TBS: Resume frequency sensitive drivers
            //

            //
            // Unlock Freq. Change
            //
            dCCFreqLock--;

            INTERRUPTS_ON();

            PVTuneReleaseCCF = NULL;

            retval = TRUE;

            break;
        default:
            NKDbgPrintfW(TEXT("Unsupported IOCTL called: %X\r\n"), code);
            NKSetLastError(ERROR_NOT_SUPPORTED);
            retval=FALSE;
    }   // End switch on pPMUCCFBuf->subcode

    return retval;
}

BOOL OALIoCtlPMUConfigCall(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{
    DWORD len;
    BOOL retval = FALSE;
   
    if ((pInpBuffer == NULL) || (inpSize != sizeof(PMURegInfo)))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    //
    // Check PMU control code
    //
    switch (*(LPDWORD)pInpBuffer)
    {
        case PMU_ALLOCATE:
            //
            // A user has allocated the PMU.
            // Register the ReleasePMU callback with the kernel.
            // (Address translated in API (Pmudll))
            //
            pPMURegBuf = (PPMURegInfo)pInpBuffer;
            PVTuneReleasePMU = (ReleasePMUCallback) pPMURegBuf->pCallback;
            return TRUE;
            break;

        case PMU_RELEASE:
            //
            // A user has released the PMU.
            // Unregister the ReleasePMU callback.
            //
            PVTuneReleasePMU = NULL;
            return TRUE;
            break;

        case PMU_ENABLE_IRQ:
            pPMURegBuf = (PPMURegInfo)pInpBuffer;
            PVTuneInterrupt = (PMUInterruptCallback) pPMURegBuf->pCallback;
            bPMURunning = TRUE;

            //Enable PMU IRQ
            //VTune now uses an installable ISR
            /*{
                UINT32 irq = IRQ_PMU;
                OALIntrEnableIrqs( 1, &irq);
            }*/

            return TRUE;
            break;

        case PMU_ENABLE_FIQ:
            NKSetLastError(ERROR_INVALID_FUNCTION);
            break;

        case PMU_DISABLE_IRQ:
            PVTuneInterrupt = NULL;
            //Disable PMU IRQ
            //VTune now uses an installable ISR
            /*{
                UINT32 irq = IRQ_PMU;
                OALIntrDisableIrqs( 1, &irq);
            }*/
            bPMURunning = FALSE;
            return TRUE;
            break;

        case PMU_DISABLE_FIQ:
            NKSetLastError(ERROR_INVALID_FUNCTION);
            break;

        case PMU_READ_REG:
            //
            // Read from the specified PMU register
            //      2nd DWORD has register number
            //      Register value (DWORD) written to output buffer
            //
            pPMURegBuf = (PPMURegInfo)pInpBuffer;

            if (pPMURegBuf->PMUReg > MAXPMUREG)
            {
                NKSetLastError(ERROR_INVALID_PARAMETER);
                break;
            }

            //
            // Read the CP14 PMU register
            //
            pPMURegBuf->PMUValue = ReadPMUReg(pPMURegBuf->PMUReg);

            //
            // Return results
            //
            len = sizeof(DWORD);
            if ((outSize == len) && (pOutBuffer != NULL) && (pOutSize != NULL))
            {
                memcpy(pOutBuffer,&(pPMURegBuf->PMUValue),len);
                *pOutSize = len;
                retval = TRUE;
            }
            else
            {
                NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
            break;
        case PMU_WRITE_REG:
            //
            // Write to the specified PMU register
            //      2nd DWORD has register number
            //      3rd DWORD is value to write to register
            //
            pPMURegBuf = (PPMURegInfo)pInpBuffer;

            if (pPMURegBuf->PMUReg > MAXPMUREG)
            {
                NKSetLastError(ERROR_INVALID_PARAMETER);
                break;
            }

            //
            // Write to the CP14 PMU register and return
            //
            WritePMUReg (pPMURegBuf->PMUReg, pPMURegBuf->PMUValue);

            return TRUE;
            break;

        case PMU_OEM_INFO:
            //
            // Obtain the OEM information used for the
            //      SYSINTR_PMU interrupt ID
            //      VTune's PMU driver global area
            //
            pPMURegBuf = (PPMURegInfo)pInpBuffer;
            pPMURegBuf->OEMData.sysintrID = OALIntrTranslateIrq(IRQ_PMU);       //SYSINTR_PMU

            // Return results
            //
            len = (sizeof(OEMInfo));
            if ((outSize >= len) && (pOutBuffer != NULL) && (pOutSize != NULL))
            {
                memcpy(pOutBuffer,&(pPMURegBuf->OEMData),len);
                *pOutSize = len;
                retval = TRUE;
            }
            else
            {
                NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
            }

            return TRUE;
            break;
        default:
            NKDbgPrintfW(TEXT("Unsupported IOCTL called: %X\r\n"), code);
            NKSetLastError(ERROR_NOT_SUPPORTED);
            retval=FALSE;

    }  // end switch
    
    
    return retval;
}


BOOL OALIoCtlGetCPUIdCall(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{
    BOOL retval=FALSE;
    DWORD len;
    if ((pInpBuffer == NULL) || (inpSize != sizeof(CPUIdInfo)))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pCPUIdBuf = (PCPUIdInfo) pInpBuffer;

    // Read the CP15 R0 Register to get the ID information
    //
    pCPUIdBuf->CPUId = XSC1GetCPUId();

    // Return results
    //
    len = sizeof(DWORD);
    if ((outSize == len) && (pOutBuffer != NULL) && (pOutSize != NULL))
    {
        memcpy(pOutBuffer,&(pCPUIdBuf->CPUId),len);
        *pOutSize = len;
        retval = TRUE;
    }
    else
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
    }
    return retval;
}
