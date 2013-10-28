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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  off.c
//
//  This file provides the capabilities to suspend the system and controlling
//  wake sources.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <cmnintrin.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------
extern UINT32 g_oalIrqTranslate[OAL_INTR_IRQ_MAXIMUM];
extern UINT32 g_oalGpioTranslate[DDK_GPIO_PORT3+1][GPIO_INTR_SOURCES_MAX];
extern ULARGE_INTEGER g_oalIrqMask[OAL_INTR_IRQ_MAXIMUM];
extern UINT32 g_oalGpioMask[DDK_GPIO_PORT3+1][OAL_INTR_IRQ_MAXIMUM];
extern PCSP_AVIC_REGS g_pAVIC;
extern PCSP_GPIO_REGS g_pGPIO1;
extern PCSP_GPIO_REGS g_pGPIO2;
extern PCSP_GPIO_REGS g_pGPIO3;
extern PCSP_CCM_REGS  g_pCCM;
extern PCSP_EPIT_REG g_pEPIT;

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------
static ULARGE_INTEGER g_avicWakeMask = {0, 0};
static UINT32 g_gpioWakeMask[DDK_GPIO_PORT3+1];
static UINT32 g_WakeSource;

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
// Function: OEMPowerOff
//
// Called when the system is to transition to it's lowest power mode.  This
// function stores off some of the important registers (not really needed for
// DSM since registers will not lose state in DSM).
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void OEMPowerOff()
{
    UINT32 registerStore[6];
    UINT32 irq;
    UINT32 line;
    BOOL PowerState;

    // Reset the wake source global
    g_WakeSource = SYSWAKE_UNKNOWN;

    // If any of the GPIO interrupts are wakes sources, add the corresponding
    // AVIC interrupt to the wake mask
    if (g_gpioWakeMask[DDK_GPIO_PORT1])
    {
        g_avicWakeMask.QuadPart |= CSP_IRQMASK(IRQ_GPIO1);
    }

    if (g_gpioWakeMask[DDK_GPIO_PORT2])
    {
        g_avicWakeMask.QuadPart |= CSP_IRQMASK(IRQ_GPIO2);
    }

    if (g_gpioWakeMask[DDK_GPIO_PORT3])
    {
        g_avicWakeMask.QuadPart |= CSP_IRQMASK(IRQ_GPIO3);
    }

    // If no wake sources are registered, we should not allow power off
    if (!g_avicWakeMask.QuadPart)
    {
        OALMSG(OAL_ERROR, (_T("ERROR: OEMPowerOff has no wake sources!\r\n")));
        return;
    }

    // Switch off power for KITL device
    PowerState = FALSE;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

    // Save state of enabled interrupts
    registerStore[0] = INREG32(&g_pAVIC->INTENABLEH);
    registerStore[1] = INREG32(&g_pAVIC->INTENABLEL);
    registerStore[2] = INREG32(&g_pGPIO1->IMR);
    registerStore[3] = INREG32(&g_pGPIO2->IMR);
    registerStore[4] = INREG32(&g_pGPIO3->IMR);

    // Disable all GPIO interrups except for desired wake sources
    OUTREG32(&g_pGPIO1->IMR, g_gpioWakeMask[DDK_GPIO_PORT1]);
    OUTREG32(&g_pGPIO2->IMR, g_gpioWakeMask[DDK_GPIO_PORT2]);
    OUTREG32(&g_pGPIO3->IMR, g_gpioWakeMask[DDK_GPIO_PORT3]);

    // Disable all interrupts except for desired wake sources
    OUTREG32(&g_pAVIC->INTENABLEH, g_avicWakeMask.HighPart);
    OUTREG32(&g_pAVIC->INTENABLEL, g_avicWakeMask.LowPart);

    // Place the system in suspend state and wait for an interrupt.
    OALMSG(OAL_POWER, (_T("INFO: OEMPowerOff entering suspend.  INTENABLEH = 0x%x, INTENABLEL = 0x%x\r\n"), 
        INREG32(&g_pAVIC->INTENABLEH), INREG32(&g_pAVIC->INTENABLEL)));

    // Disable the OS tick timer (EPIT1) while we are powered off.  This is 
    // necessary since the EPIT can be clocked from CKIL and will continue
    // to run in some low-power modes.
    INSREG32BF(&g_pEPIT->CR, EPIT_CR_EN, EPIT_CR_EN_DISABLE);

    // Allow platform code to finish the power off sequence
    BSPPowerOff();

    INSREG32BF(&g_pEPIT->CR, EPIT_CR_EN, EPIT_CR_EN_ENABLE);

    // Get interrupt source
    irq = EXTREG32BF(&g_pAVIC->NIVECSR, AVIC_NIVECSR_NIVECTOR);

    OALMSG(OAL_POWER, (_T("INFO: OEMPowerOff leaving suspend.  NIVECSR = 0x%x\r\n"), irq));

    // If valid wake interrupt is pending
    if (irq < AVIC_IRQ_SOURCES_MAX)
    {
        if (irq == IRQ_GPIO1)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO1->ISR) 
                                      & INREG32(&g_pGPIO1->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            { 
                irq = g_oalGpioTranslate[0][31 - line];
            }

        } // GPIO1 special case

        else if (irq == IRQ_GPIO2)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO2->ISR) 
                                      & INREG32(&g_pGPIO2->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            { 
                irq = g_oalGpioTranslate[1][31 - line];
            }

        } // GPIO2 special case

        else if (irq == IRQ_GPIO3)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO3->ISR) 
                                      & INREG32(&g_pGPIO3->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            { 
                irq = g_oalGpioTranslate[2][31 - line];
            }
        } // GPIO3 special case

        else
        {
            irq = g_oalIrqTranslate[irq];
        }

        // Map the irq to a SYSINTR
        g_WakeSource = OALIntrTranslateIrq(irq);
    }

    // Restore state of enabled interrupts
    OUTREG32(&g_pAVIC->INTENABLEH, registerStore[0]);
    OUTREG32(&g_pAVIC->INTENABLEL, registerStore[1]);

    // Restore state of enabled GPIO interrupts
    OUTREG32(&g_pGPIO1->IMR, registerStore[2]);
    OUTREG32(&g_pGPIO2->IMR, registerStore[3]);
    OUTREG32(&g_pGPIO3->IMR, registerStore[4]);

    // Remove GPIO as wake source
    g_avicWakeMask.QuadPart &= (~(CSP_IRQMASK(IRQ_GPIO1) | 
        CSP_IRQMASK(IRQ_GPIO2) | CSP_IRQMASK(IRQ_GPIO3)));

    // Switch on power for KITL device
    PowerState = TRUE;
    KITLIoctl (IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

    // Do platform dependent power on actions
    BSPPowerOn();

}


//-----------------------------------------------------------------------------
//
// Function:     OALIoCtlHalEnableWake
//
// Called when a IOCTL_HAL_ENABLE_WAKE interrupt occurs.  This function will
// set the requested interrupt as a wake source.
//
// Parameters:
//      code
//           [in] IOCTL_HAL_ENABLE_WAKE.
//
//      pInpBuffer
//           [in] The interrupt that is being set as a wake source.
//
//      inpSize
//           [in] The size of a UINT32.
//
//      pOutBuffer
//           [out] Ignored.
//
//      outSize
//           [in] Ignored.
//
//      pOutSize
//           [out] Ignored.
//
// Returns:
//      TRUE if the IOCTL was successfully processed.  FALSE if an error
//      occurs while processing the IOCTL.
//
//-----------------------------------------------------------------------------
BOOL
OALIoCtlHalEnableWake(UINT32 code, VOID* pInpBuffer, UINT32 inpSize,
                      VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    UINT32 sysIntr;
    BOOL rc = FALSE;
    UINT32 count = 1;
    const UINT32 *irq;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_FUNC&&OAL_POWER, (_T("+OALIoCtlHalEnableWake\r\n")));

    if (pInpBuffer == NULL || inpSize < sizeof(UINT32)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_ENABLE_WAKE invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    sysIntr = *(UINT32*)pInpBuffer;

    // Retrieve list of irqs for the specified SYSINTR
    if (!OALIntrTranslateSysIntr(sysIntr, &count, &irq))
    {
        OALMSG(OAL_ERROR,
           (_T("ERROR: OALIntrTranslateSysIntr failed for IOCTL_HAL_ENABLE_WAKE,\r\n")));
        goto cleanUp;
    }

    g_avicWakeMask.QuadPart |= g_oalIrqMask[*irq].QuadPart;
    g_gpioWakeMask[DDK_GPIO_PORT1] |= g_oalGpioMask[DDK_GPIO_PORT1][*irq];
    g_gpioWakeMask[DDK_GPIO_PORT2] |= g_oalGpioMask[DDK_GPIO_PORT2][*irq];
    g_gpioWakeMask[DDK_GPIO_PORT3] |= g_oalGpioMask[DDK_GPIO_PORT3][*irq];

    rc = TRUE;

cleanUp:
    OALMSG(OAL_FUNC&&OAL_POWER, (_T("-OALIoCtlHalEnableWake (rc = %d)\r\n"), rc));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:     OALIoCtlHalDisableWake
//
// Called when a IOCTL_HAL_DISABLE_WAKE interrupt occurs.  This function will
// disable the requested interrupt as a wake source.
//
// Parameters:
//      code
//           [in] IOCTL_HAL_DISABLE_WAKE.
//
//      pInpBuffer
//           [in] The interrupt that is being set as a wake source.
//
//      inpSize
//           [in] The size of a UINT32.
//
//      pOutBuffer
//           [out] Ignored.
//
//      outSize
//           [in] Ignored.
//
//      pOutSize
//           [out] Ignored.
//
// Returns:
//      TRUE if the IOCTL was successfully processed.  FALSE if an error
//      occurs while processing the IOCTL.
//
//-----------------------------------------------------------------------------
BOOL
OALIoCtlHalDisableWake(UINT32 code, VOID* pInpBuffer, UINT32 inpSize,
                       VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    UINT32 sysIntr;
    BOOL rc = FALSE;
    UINT32 count = 1;
    const UINT32 *irq;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_FUNC&&OAL_POWER, (_T("+OALIoCtlHalDisableWake\r\n")));

    if (pInpBuffer == NULL || inpSize < sizeof(UINT32)) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_DISABLE_WAKE invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    sysIntr = *(UINT32*)pInpBuffer;

    // Retrieve list of irqs for the specified SYSINTR
    if (!OALIntrTranslateSysIntr(sysIntr, &count, &irq))
    {
        OALMSG(OAL_ERROR,
           (_T("ERROR: OALIntrTranslateSysIntr failed for IOCTL_HAL_DISABLE_WAKE,\r\n")));
        goto cleanUp;
    }

    g_avicWakeMask.QuadPart &= ~(g_oalIrqMask[*irq].QuadPart);
    g_gpioWakeMask[DDK_GPIO_PORT1] &= ~(g_oalGpioMask[DDK_GPIO_PORT1][*irq]);
    g_gpioWakeMask[DDK_GPIO_PORT2] &= ~(g_oalGpioMask[DDK_GPIO_PORT2][*irq]);
    g_gpioWakeMask[DDK_GPIO_PORT3] &= ~(g_oalGpioMask[DDK_GPIO_PORT3][*irq]);

    rc = TRUE;
cleanUp:
    OALMSG(OAL_FUNC&&OAL_POWER, (_T("-OALIoCtlHalDisableWake (rc = %d)\r\n"), rc));

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function: OALIoCtlHalGetWakeSource
//
// Called when a IOCTL_HAL_GET_WAKE_SOURCE interrupt occurs.  This function
// returns the interrupts that are the current wake sources.
//
// Parameters:
//      code
//           [in] IOCTL_HAL_GET_WAKE_SOURCE.
//
//      pInpBuffer
//           [in] Ignored
//
//      inpSize
//           [in] Ignored.
//
//      pOutBuffer
//           [out] The system interrupts that wake up the core.
//
//      outSize
//           [in] The size of pOutBuffer.  Must be the size of an UINT64.
//
//      pOutSize
//           [out] The actual size of pOutBuffer.
//
// Returns:
//      TRUE if the IOCTL was successfully processed, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL
OALIoCtlHalGetWakeSource(UINT32 code, VOID* pInpBuffer, UINT32 inpSize,
                         VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);

    OALMSG(OAL_FUNC&&OAL_POWER, (_T("+OALIoCtlHalGetWakeSource\r\n")));

    if ((pOutBuffer == NULL) || (outSize < sizeof(UINT32))) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_WAKE_SOURCE invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    *(UINT32*)pOutBuffer = g_WakeSource;

    if (pOutSize != NULL) {
        *pOutSize = sizeof(UINT32);
    }

    rc = TRUE;

cleanUp:

    OALMSG(OAL_FUNC&&OAL_POWER, (_T("-OALIoCtlHalGetWakeSource (rc = %d)\r\n"), rc));

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: OALClockSetGatingMode
//
//  This function provides the OAL a safe mechanism for setting the clock 
//  gating mode of peripherals.
//
//  Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      mode
//           [in] Requested clock gating mode for the peripheral.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
    BOOL fEnable;

    // CGR is a shared register so we must disable interrupts temporarily
    // for safe access
    fEnable = INTERRUPTS_ENABLE(FALSE);

    // Update the clock gating mode
    INSREG32(&g_pCCM->CGR[CCM_CGR_INDEX(index)], CCM_CGR_MASK(index),
             CCM_CGR_VAL(index, mode));

    INTERRUPTS_ENABLE(fEnable);
}


//-----------------------------------------------------------------------------
//
//  Function: OALClockEnableIIM
//
//  This function enables/disables module clocks for the IIM module. 
//
//  Parameters:
//      bClockEnable
//           [in] Set TRUE to enable IIM module clocks.  Set FALSE
//           to disable IIM module clocks.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID OALClockEnableIIM(BOOL bClockEnable)
{
    DDK_CLOCK_GATE_MODE mode = bClockEnable ? DDK_CLOCK_GATE_MODE_ENABLED_ALL :
        DDK_CLOCK_GATE_MODE_DISABLED;

    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_IIM, mode);
}
