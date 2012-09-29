//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspiutils_oal.c
//
//  CSPI support for shared access between OAL, KITL, and drivers.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include <bsp.h>

//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_bOalPostInit;
static BOOL g_bOalCspiMapped;

//-----------------------------------------------------------------------------
// Local Variables
static CRITICAL_SECTION g_oalCspiCS;
static LPCRITICAL_SECTION g_pOalCspiCS = NULL;
static BOOL g_bEdbgActive;


//-----------------------------------------------------------------------------
//
//  Function:  CSPIRequest
//
//  Acquires exclusive access to shared CSPI port.
//
//  Parameters:
//      pCSPI
//          [in] Pointer to CSPI registers.
//
//      controlReg
//          [in] CSPI CONTROLREG value to be programmed.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID CSPIRequest(PCSP_CSPI_REG pCSPI, UINT32 controlReg)
{
    UINT32 cpiEn;
    BOOL bIntrEnable;
    
    // Skip synchronization if kernel initialzation is not complete
    if (g_bOalPostInit)
    {
        // Make sure critical section has been mapped
        if (g_pOalCspiCS == NULL)
        {
            g_pOalCspiCS = &g_oalCspiCS;
            InitializeCriticalSection(g_pOalCspiCS);
        }

        // Take critical section protecting OAL CSPI access
        EnterCriticalSection(g_pOalCspiCS);
        
        // If KITL EDBG is active, we need to gain exclusive access to CSPI
        // by temporarily disabling interrupts
        g_bEdbgActive = KITLIoctl(IOCTL_EDBG_IS_STARTED, NULL, 0, NULL, 0, NULL);
        if (g_bEdbgActive) 
        {
            // Wait for CSPI to be released by other clients
            do
            {
                // We should never attempt this if interrupts are disabled...
                bIntrEnable = INTERRUPTS_ENABLE (FALSE);
                DEBUGCHK(bIntrEnable);
                cpiEn = EXTREG32BF(&pCSPI->CONREG, CSPI_CONREG_EN);

                // If the CSPI is enabled, KITL must currently be using it.
                // Reenable interrupts and try again after sleeping.
                if (cpiEn == CSPI_CONREG_EN_ENABLE)
                {
                    INTERRUPTS_ENABLE (TRUE);
                    NKSleep(1);
                }
            } while (cpiEn == CSPI_CONREG_EN_ENABLE);

        }
    }

    
    // At this point, we have met one of the following conditions:
    //
    //  1) Kernel is not still not initialized, so we have
    //     exclusive access to CSPI
    //
    //  2) Kernel is initialized, but KITL EDBG is not active.  We
    //     have acquired CSPI critical section for exclusive
    //     access.
    //
    //  3) Kernel is initialized and KITL EDBG is active.  We have
    //     acquired the CSPI critical section and we have
    //     disabled interrupts to prevent KITL from accessing
    //     the CSPI.
    OUTREG32(&pCSPI->CONREG, controlReg);
    }


//-----------------------------------------------------------------------------
//
//  Function:  CSPIRelease
//
//  Releases exclusive access to shared CSPI port.
//
//  Parameters:
//      pCSPI
//          [in] Pointer to CSPI registers.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID CSPIRelease(PCSP_CSPI_REG pCSPI)
{
    // We give up exclusive access by clearing the EN bit
    INSREG32BF(&pCSPI->CONREG, CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE);

    if (g_bOalPostInit)
    {
        // If KITL EDBG was active, we temporarily disabled interrupts so
        // we need to enable them again here
        if (g_bEdbgActive)
        {
            INTERRUPTS_ENABLE (TRUE);
        }
        // Release the CSPI critical section
        if (g_pOalCspiCS != NULL)
        {
            LeaveCriticalSection(g_pOalCspiCS);
        }                
    }    
}
