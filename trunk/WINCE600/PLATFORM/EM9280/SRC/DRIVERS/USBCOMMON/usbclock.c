//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
// File: usbclock.c
//     Maintains shared access to USB peripheral block clock gating
// Functions:
//     USBClockDisable(BOOL fEnabled)
//         Either enable or disable the USB clock, for this user.  Only if no
//         other user requires the clock will it actually be gated off.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "common_usbcommon.h"
#include "mx28_usbname.h"

HANDLE m_hUSBClockGatingHandle = NULL;
BSP_USB_CLOCK_GATING  *pUSBClkGating = NULL;
BOOL BSPUSBClockSwitch(BOOL fOn);
extern WORD BSPGetUSBControllerType(void);

static HANDLE h_USBOTGClockOpenState = NULL;
static HANDLE h_USBH1HClockOpenState = NULL;

// #ifdef DEBUG
// #define ZONE_FUNCTION         DEBUGZONE(3)
// #endif

//------------------------------------------------------------------------------
// Function: USBClockSet
//
// Description: This function enables or disables the USB clock by calling the
//              DDK clock related interfaces. Configure the clock source for USB
//              as PER CLK and prediv = 0, postdiv = 4(The actual divider will
//              be (0+1)*(4+1)= 5 ).
//
// Parameters:
//     fEnabled
//         [IN] If TRUE, configure the boud rate for USB module and enable the
//              clock for USB. If FALSE, disable the clock for USB module.
// Returns:
//    TRUE - operation success
//    FALSE - operation failed
//
//------------------------------------------------------------------------------
static BOOL USBClockSet(BOOL fEnabled)
{
    BOOL rc = FALSE;
    PVOID pv_HWregDIGCTL = NULL;
    PHYSICAL_ADDRESS phyAddr;

    if (pv_HWregDIGCTL == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_DIGCTL;
        pv_HWregDIGCTL = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregDIGCTL == NULL)
        {
            RETAILMSG(1, (TEXT("InitializeTransceiver::MmMapIoSpace failed for pv_HWregDIGCTL\r\n")));
        }
    }

    if (fEnabled)
    {
        if (BSPGetUSBControllerType() == USB_SEL_OTG)
        {
            if (
                    (DDKClockGetGatingMode(DDK_CLOCK_GATE_UTMI0_CLK480M_CLK) == TRUE) || 
                    (WaitForSingleObject(h_USBOTGClockOpenState, 0) == WAIT_TIMEOUT) // there is possibility that clock is auto
                                                                                     // resumed by HW, in this state "h_USBOTGClockOpenState"
                                                                                     // is non-signaled
               )
            {
                SetEvent(h_USBOTGClockOpenState);
                DDKClockSetGatingMode(DDK_CLOCK_GATE_UTMI0_CLK480M_CLK, FALSE);        
            }

            if (HW_DIGCTL_CTRL.B.USB0_CLKGATE)
            {
                HW_DIGCTL_CTRL.B.USB0_CLKGATE = 0;
            }
        }
        else if (BSPGetUSBControllerType() == USB_SEL_H1)
        {
            if (
                    (DDKClockGetGatingMode(DDK_CLOCK_GATE_UTMI1_CLK480M_CLK) == TRUE) || 
                    (WaitForSingleObject(h_USBH1HClockOpenState, 0) == WAIT_TIMEOUT) // there is possibility that clock is auto              
                                                                                     // resumed by HW, in this state "h_USBH1HClockOpenState"
                                                                                     // is non-signaled                                      
               )
            {
                SetEvent(h_USBH1HClockOpenState);
                DDKClockSetGatingMode(DDK_CLOCK_GATE_UTMI1_CLK480M_CLK, FALSE);
            }

            if (HW_DIGCTL_CTRL.B.USB1_CLKGATE)
            {
                HW_DIGCTL_CTRL.B.USB1_CLKGATE = 0;
            }
        }
    }
    else
    {
        // In MX233 there are 2 usb clock path, "DDK_CLOCK_GATE_UTMI_CLK480M"
        // and "HW_DIGCTL_CTRL_USB_CLKGATE", here we only control the first path
        // when doing power down operation
        //
        // The reason why we don't set "HW_DIGCTL_CTRL_USB_CLKGATE" is that
        // when it is 1, no USB Interrupt will be generated to wakeup the 
        // module. Fortunately it should have little impact to power consumption
        if (BSPGetUSBControllerType() == USB_SEL_OTG)
        {
            if (DDKClockGetGatingMode(DDK_CLOCK_GATE_UTMI0_CLK480M_CLK) == FALSE)
            {
                ResetEvent(h_USBOTGClockOpenState);
                DDKClockSetGatingMode(DDK_CLOCK_GATE_UTMI0_CLK480M_CLK, TRUE);
            }

            if (!HW_DIGCTL_CTRL.B.USB0_CLKGATE)
            {
                HW_DIGCTL_CTRL.B.USB0_CLKGATE = 1;
            }
        }
        else if (BSPGetUSBControllerType() == USB_SEL_H1)
        {
            if (DDKClockGetGatingMode(DDK_CLOCK_GATE_UTMI1_CLK480M_CLK) == FALSE)
            {
                ResetEvent(h_USBH1HClockOpenState);
                DDKClockSetGatingMode(DDK_CLOCK_GATE_UTMI1_CLK480M_CLK, TRUE);
            }
            if (!HW_DIGCTL_CTRL.B.USB1_CLKGATE)
            {
                HW_DIGCTL_CTRL.B.USB1_CLKGATE = 1;
            }
        }
    }

    if (pv_HWregDIGCTL) MmUnmapIoSpace((PVOID)pv_HWregDIGCTL, 0x1000);

    rc = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBClock %s return %d\r\n"), (fEnabled ? TEXT("TRUE") : TEXT("FALSE")), rc));

    return rc;
}

//----------------------------------------------------------------
// 
//  Function:  DumpUsbClocks
//
//  Dump external registers, such as clock, power, iomux, etc
//
//  Parameter:
//
//  Return
//     NULL
//
//----------------------------------------------------------------
void DumpUsbClocks(void)
{
    PVOID pv_HWregDIGCTL = NULL;
    PHYSICAL_ADDRESS phyAddr;
    BOOL bDDKGating = FALSE;
    BOOL bDigCtlGating = FALSE;

    if (pv_HWregDIGCTL == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_DIGCTL;
        pv_HWregDIGCTL = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregDIGCTL == NULL)
        {
            RETAILMSG(1, (TEXT("InitializeTransceiver::MmMapIoSpace failed for pv_HWregDIGCTL\r\n")));
        }
    }
    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        bDDKGating = DDKClockGetGatingMode(DDK_CLOCK_GATE_UTMI0_CLK480M_CLK);
        bDigCtlGating = HW_DIGCTL_CTRL.B.USB0_CLKGATE;
        
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        bDDKGating = DDKClockGetGatingMode(DDK_CLOCK_GATE_UTMI1_CLK480M_CLK);
        bDigCtlGating = HW_DIGCTL_CTRL.B.USB1_CLKGATE;
    }
    RETAILMSG(1, (L"\t[%s] : DDK gating %s, Dig gating %s\r\n", 
                            BSPGetUSBControllerType() == USB_SEL_OTG ? L"OTG" : L"H1H",
                            bDDKGating ? L"TRUE" : L"FALSE",
                            bDigCtlGating ? L"TRUE" : L"FALSE"));
    
    if (pv_HWregDIGCTL) MmUnmapIoSpace((PVOID)pv_HWregDIGCTL, 0x1000);
}

//------------------------------------------------------------------------------
// Function: USBClockInit
//
// Description: This funtion initializes and starts the USB Core Clock
//
// Parameter:
//     NULL.
//
// Return:
//     TRUE - success to start the clock
//     FALSE - fail to start the clock
//
//------------------------------------------------------------------------------
BOOL USBClockInit(void)
{
    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        h_USBOTGClockOpenState = CreateEvent(NULL, TRUE, FALSE, L"OTGClockOpenState");
    }
    if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        h_USBH1HClockOpenState = CreateEvent(NULL, TRUE, FALSE, L"H1HClockOpenState");
    }
    BSPUSBClockSwitch(TRUE);     // get sel internally, don't need to change parameter
    return USBClockSet(TRUE);
}

//------------------------------------------------------------------------------
// Function: USBClockGatingLock
//
// Descriptions: Use the parameter ClockGatingLock as a critical section
//               in controlling the access within the Lock/Unlock for multiple
//               USB drivers
//
// Parameters:
//     NULL
//
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
void USBClockGatingLock(void)
{
    do
    {
        // Wait until lock is released
        while (*((volatile UINT32 *)(&pUSBClkGating->ClockGatingLock))) ;
    } while (InterlockedTestExchange((LPLONG)&pUSBClkGating->ClockGatingLock, FALSE, TRUE) != FALSE);

}

//------------------------------------------------------------------------------
// Function: USBClockGatingUnlock
//
// Description: This function uses the parameter ClockGatingLock as a critical
//              section in controlling the access within the Lock/Unlock for
//              multiple USB drivers
//
// Parameter:
//     NULL
//
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
void USBClockGatingUnlock(void)
{
    pUSBClkGating->ClockGatingLock = FALSE;
}

//------------------------------------------------------------------------------
// Function: BSPUSBClockCreateFileMapping
//
// Description: This fuction is used to create the shared memory to be used
//              for controlling the stop/start of USB clock when multiple
//              USB controllers are running
//
// Parameter:
//     NULL
//
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
BOOL BSPUSBClockCreateFileMapping(void)
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    WORD sel = BSPGetUSBControllerType();
#else
    BSPGetUSBControllerType();
#endif

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Port(%d):BSPUSBCreateFileMapping\r\n"), sel));

    if (m_hUSBClockGatingHandle == NULL)
    {
        m_hUSBClockGatingHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
                                                    PAGE_READWRITE, 0, sizeof(BSP_USB_CLOCK_GATING), USBClockGatingName);

        if (m_hUSBClockGatingHandle == NULL)
        {
            RETAILMSG (1, (TEXT("Failure to Create File Mapping for USB\r\n")));
            return FALSE;
        }

        if (GetLastError() != ERROR_ALREADY_EXISTS) // you are the first one to create it
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Port(%d):First to create USB_CLOCK_GATING\r\n"), sel));
            pUSBClkGating = (BSP_USB_CLOCK_GATING *) MapViewOfFile(m_hUSBClockGatingHandle,
                                                                   FILE_MAP_ALL_ACCESS, 0, 0, 0);
            pUSBClkGating->ClockGatingMask = 0;
            pUSBClkGating->ClockGatingLock = FALSE;
        }
        else
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Port(%d) Open existing USB_CLOCK_GATING\r\n"), sel));
            pUSBClkGating = (BSP_USB_CLOCK_GATING *) MapViewOfFile(m_hUSBClockGatingHandle,
                                                                   FILE_MAP_ALL_ACCESS, 0, 0, 0);
        }

        DEBUGMSG(ZONE_FUNCTION, (TEXT("Port(%d):BSPUSBClockCreateFileMapping create success\r\n"), sel));
        return TRUE;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-Port(%d):BSPUSBClockCreateFileMapping handle exist\r\n"), sel));
    return TRUE;

}

//------------------------------------------------------------------------------
// Function: BSPUSBClockDeleteFileMapping
//
// Description: This function is used to delete the shared memory to be used
//              for controlling the stop/start of USB clock when multiple
//              USB controllers are running
//
// Parameters:
//     NULL
//
// Returns:
//     NULL
//
//------------------------------------------------------------------------------
void BSPUSBClockDeleteFileMapping(void)
{
    if (pUSBClkGating)
    {
        UnmapViewOfFile(pUSBClkGating);
        pUSBClkGating = NULL;
    }

    if (m_hUSBClockGatingHandle)
    {
        CloseHandle(m_hUSBClockGatingHandle);
        m_hUSBClockGatingHandle = NULL;
    }

}


//------------------------------------------------------------------------------
// Function: BSPUSBClockSwitch
//
// Description: This function is used to control multiple USB controllers from
//              accessing the same USB clock. ClockGatingMask is used to control
//              2 USB controllers (OTG and Host 2). If stop is request, it would
//              check against the ClockGatingMask and make sure it is 0 before
//              it actually stop the clock.
//              On other hands, whenever a start is request, it would start
//              the USB clock right away.
//              ClockGatingMask: Bit 0 => OTG, Bit 1 => Host 2
//
// Parameters:
//     fOn
//         [IN] TRUE means to start the USB clock, FALSE means to stop the USB
//              clock
//
// Returns:
//    TRUE: Success
//    FALSE: FAilure
//
//------------------------------------------------------------------------------
BOOL BSPUSBClockSwitch(BOOL fOn)
{
    DWORD dwMask = 0x0;
    BOOL fOK = FALSE;

    WORD sel = BSPGetUSBControllerType();

    // Since only OTG port support client mode.
    dwMask = 0x1 << sel;

    BSPUSBClockCreateFileMapping();

    USBClockGatingLock();
    if (fOn == FALSE) // close the clock
    {
        pUSBClkGating->ClockGatingMask &= ~dwMask;
        // In MX28, the clock to OTG and H1 are totally separate, so there is no need for us
        // to check the mask any more
        fOK = TRUE;
        if (fOK)
            fOK = USBClockSet(FALSE);
    }
    else // open the clock
    {
        pUSBClkGating->ClockGatingMask |= dwMask;
        fOK = TRUE;
        fOK = USBClockSet(TRUE);
    }
    USBClockGatingUnlock();

    DEBUGMSG(1, (TEXT("Port(%d) - USBClockCanClockGating(%d), return 0x%x, value 0x%x\r\n"),
                 sel, fOn, fOK, pUSBClkGating->ClockGatingMask));
    return fOK;
}
