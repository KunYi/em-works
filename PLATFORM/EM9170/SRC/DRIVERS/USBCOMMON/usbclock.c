//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
// File:      
//     USBClock.c
// Purpose:   
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
#include "common_usbname.h"

HANDLE m_hUSBClockGatingHandle = NULL;
BSP_USB_CLOCK_GATING  *pUSBClkGating = NULL;
BOOL BSPUSBClockSwitch(BOOL fOn);
extern WORD BSPGetUSBControllerType(void);

#ifdef DEBUG
#define ZONE_FUNCTION         DEBUGZONE(3)
#endif

//------------------------------------------------------------------------------
// Function: USBClockSet
//
// Description: This function enables or disables the USB clock by calling the
//              DDK clock related interfaces. Configure the clock source for USBPLL
//              with a 4 divider
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
    DDK_CLOCK_GATE_MODE currentMode;

    if (fEnabled)
    {
        // USB PLL clock value is 240Mhz. So we need a 4 divider to obtain the 60Mhz for USB clock
        if (!DDKClockConfigBaud(DDK_CLOCK_SIGNAL_USBCLK, DDK_CLOCK_BAUD_SOURCE_USBPLL, 4-1))
        {
            DEBUGMSG (ZONE_ERROR, (L"USBClockSet: DDKClockConfigBaud failed\r\n"));
            goto cleanUp;
        }

        DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_USBOTG, &currentMode);
        if (currentMode != DDK_CLOCK_GATE_MODE_ENABLED)
        {
            RETAILMSG(FALSE, (L"enable USBOTG clockgate\r\n"));
            if (!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_USBOTG, DDK_CLOCK_GATE_MODE_ENABLED))
            {
                DEBUGMSG (ZONE_ERROR, (L"USBClockSet: DDKClockSetGatingMode failed\r\n"));
                goto cleanUp;
            }
            DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_USBOTG, &currentMode);
            if (currentMode != DDK_CLOCK_GATE_MODE_ENABLED)
            {
                RETAILMSG(1, (L"Enable Fail, Need to check, currentMode %d\r\n", currentMode));
            }
        }
    }
    else
    {
        DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_USBOTG, &currentMode);
        if (currentMode != DDK_CLOCK_GATE_MODE_DISABLED)
        {
            RETAILMSG(FALSE, (L"disable USBOTG clockgate\r\n"));
            if (!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_USBOTG, DDK_CLOCK_GATE_MODE_DISABLED))
            {
                DEBUGMSG (ZONE_ERROR, (L"USBClockSet: DDKClockSetGatingMode failed\r\n"));
                goto cleanUp;
            }
            DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_USBOTG, &currentMode);
            if (currentMode != DDK_CLOCK_GATE_MODE_DISABLED)
            {
                RETAILMSG(1, (L"Disable Fail, Need to check, currentMode %d\r\n", currentMode));
            }
        }
    }

    rc = TRUE;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBClock %s return %d\r\n"),
                    (fEnabled? TEXT("TRUE"): TEXT("FALSE")), rc));    

cleanUp:

    return rc;
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
    return BSPUSBClockSwitch(TRUE);
    //return USBClockSet(TRUE);
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
        while (*((volatile UINT32 *)(&pUSBClkGating->ClockGatingLock)));
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
            DEBUGMSG (ZONE_ERROR, (TEXT("Failure to Create File Mapping for USB\r\n")));
            return FALSE;
        }

        pUSBClkGating = (BSP_USB_CLOCK_GATING *) MapViewOfFile(m_hUSBClockGatingHandle, 
                FILE_MAP_ALL_ACCESS, 0, 0, 0);

        if (GetLastError() != ERROR_ALREADY_EXISTS) // you are the first one to create it
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Port(%d):First to create USB_CLOCK_GATING\r\n"), sel));
            pUSBClkGating->ClockGatingMask = 0;
            pUSBClkGating->ClockGatingLock = FALSE;
        }
        else
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("Port(%d) Open existing USB_CLOCK_GATING\r\n"), sel));
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
        fOK = ((pUSBClkGating->ClockGatingMask == 0)? TRUE: FALSE);
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

    DEBUGMSG(ZONE_FUNCTION, (TEXT("Port(%d) - USBClockCanClockGating(%d), return 0x%x, value 0x%x\r\n"),
                     sel, fOn, fOK, pUSBClkGating->ClockGatingMask));
    return fOK;    
}
