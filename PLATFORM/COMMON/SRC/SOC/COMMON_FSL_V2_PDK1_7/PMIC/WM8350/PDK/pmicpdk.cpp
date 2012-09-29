//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmicpdk.cpp
/// @brief  Common WM8350 PMIC functions.
///
/// This file contains the PMIC platform-specific functions that provide control
/// over the Power Management IC.
///
/// @version $Id: pmicpdk.cpp 700 2007-07-06 03:43:14Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4115)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#include <nkintr.h>
#include <cmnintrin.h>
#include <Devload.h>
#include <ceddk.h>

#include "pmic_status.h"
#include "cspiutil.h"
#include "pmi2cutil.h"
#include "pmic_ioctl.h"
#include "pmic_lla.h"
#include "pmic_adc.h"
#include "pmic_connectivity.h"
#include "pmic_convity_priv.h"
#include "WMPmic.h"
#include "WM8350.h"
#include "WM8350Util.h"
#include "i2cbus.h"

//-----------------------------------------------------------------------------
// External Functions
extern "C"  BOOL BSPPmicInitControlIF();
extern "C"  BOOL BSPPmicBlockControlIF();    // Only needed by
extern "C"  VOID BSPPmicUnblockControlIF();
extern "C"  int BSPPmicGetSpiPort(void);
extern "C"  UINT32 BSPPmicGetSpiFreqOut();
extern "C"  DWORD BSPPmicGetIrq(void);
extern "C"  BOOL BSPPmicGetWakeupGPIOConfig(WM_GPIO_CONFIG *pGPIOConfig);
extern "C"  BOOL BSPPmicGetPWRDYGPIOConfig(WM_GPIO_CONFIG *pGPIOConfig);
extern "C"  BOOL BSPPmicGetHibernateGPIOConfig(WM_GPIO_CONFIG *pGPIOConfig);
extern "C"  int BSPPmicGetI2CAddr();
extern "C"  void PmicConfigPolling( BOOL bPoll );
extern "C"  BOOL BSPPmicControlWrite( UINT32 regaddr, UINT32 regval );
extern "C"  BOOL BSPPmicControlRead( UINT32 regaddr, UINT32* pRegval );
extern "C"  BOOL BSPPmicInit();
extern "C"  BOOL BSPPmicClearIrq(void);
extern "C"  BOOL BSPPmicDeinit(void);
extern "C"  BOOL BSPPmicGetIrqStatus(UINT32 *status);
extern "C"  VOID BSPPmicPowerNotifySuspend(void);
extern "C"  VOID BSPPmicPowerNotifyResume(void);
extern "C"  VOID BSPPmicReadIntrStatus(void);
extern "C"  VOID BSPPmicSignalOALRTC(void);
extern "C"  BOOL PMICIoctlIntEnable(UINT32 IntID, BOOL EnableFlag);
extern "C"  BOOL SetRegister( UINT32 regAddr, UINT32 regval, UINT32 mask );
extern "C"  BOOL GetRegister( UINT32 regAddr, UINT32* pRegval );
extern "C"  BOOL PmicI2CProcessPackets( I2C_PACKET packets[], INT32 numPackets );
int PMICTests( WM_DEVICE_HANDLE hDevice );

//-----------------------------------------------------------------------------
// External Variables
extern "C" BOOL g_bPmicUsePolling;      // Main configuration variable, set in pmicpdk.c
extern "C" volatile BOOL g_bUsePolling; // Current state variable
extern "C" WM_DEVICE_HANDLE g_hWMDevice;


//-----------------------------------------------------------------------------
// Defines
#define PMIC_ALL_BITS                   0xFFFF

#define ADC_OPERATION_TIMEOUT           5000

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4
#define ZONEID_INTR            5
#define ZONEID_IOCTL           6

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)
#define ZONEMASK_INTR          (1 << ZONEID_INTR)
#define ZONEMASK_IOCTL         (1 << ZONEID_IOCTL)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)
#define ZONE_INTR              DEBUGZONE(ZONEID_INTR)
#define ZONE_IOCTL             DEBUGZONE(ZONEID_IOCTL)

DBGPARAM dpCurSettings = {
    _T("PMICPDK"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT("Interrupts"), TEXT("IOCTL"), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN // ulZoneMask
};

//
// Define this to TRUE to run some basic sanity tests.
//
#   define RUN_PMIC_TESTS               FALSE

#else
#   define RUN_PMIC_TESTS               FALSE
#endif  // DEBUG

// ### TEMP
#define PREPARE_ADS_BOARD               FALSE

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Local Variables
static CRITICAL_SECTION pmicCs;
static CRITICAL_SECTION adcCs;
static CRITICAL_SECTION intRegCs;
static HANDLE hIntrEventPmic;
static HANDLE hPmicISTThread;
static DWORD dwSysIntrPmic;
static BOOL bISTTerminate;
static volatile BOOL g_bAdcPolled = FALSE;
static CEDEVICE_POWER_STATE g_dxCurrent = D0;

//-----------------------------------------------------------------------------
// Local Functions
static BOOL InitializePMIC();
static VOID CleanupPMIC();
static WM_STATUS PMICHibernateSetup();
static BOOL PmicIsrThreadProc();
static DWORD WINAPI PmicPowerNotificationThread(LPVOID lpParam);
extern "C" BOOL PMICIoctlIntEnable(UINT32 IntID, BOOL EnableFlag);

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the PMIC control module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER( lpvReserved );

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DisableThreadLibraryCalls((HMODULE) hInstDll);

            DEBUGMSG(ZONE_INIT,
                (_T("***** DLL PROCESS ATTACH TO PMIC *****\r\n")));

            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    // Return TRUE for success
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: InitializePMIC
//
// Initializes the PMIC module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if the PMIC is initialized or FALSE if an error occurred during
//      initialization.  The reason for the failure can be retrieved by the
//      GetLastError().
//
//-----------------------------------------------------------------------------
static BOOL
InitializePMIC()
{
    BOOL        initializedPMIC = FALSE;
    WM_STATUS   status;
    DWORD       irq;

    DEBUGMSG(ZONE_INIT || ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    // Create critical sections
    InitializeCriticalSection( &pmicCs );
    InitializeCriticalSection( &adcCs );
    InitializeCriticalSection( &intRegCs );

    // Initialize control interface
    initializedPMIC = BSPPmicInitControlIF();
    if ( !initializedPMIC )
        goto Error;

    // Get a handel to our PMIC device.
    status = WMPmicOpenDevice( &g_hWMDevice );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s(): WMOpenPmicDevice failed!\r\n"),
            __WFUNCTION__));
        goto Error;
    }

#if RUN_PMIC_TESTS
    PMICTests( g_hWMDevice );
#endif // TEST_COMMS

#if PREPARE_ADS_BOARD
    //
    // Set up the WM8350 ADS daughtercard to power the ADS board.
    //
    {
        WM_STATUS status;
        status = WM8350PowerMX32ADSBoard( g_hWMDevice );
        if ( !WM_SUCCESS( status ) )
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s(): WM8350PowerMX32ADSBoard failed!\r\n"),
                __WFUNCTION__));
            goto Error;
        }

        //
        // Breakpoint to allow switching to the primary I2C interface.
        //
        // On the WM8350 daughtercard, make sure J11 is A:2-3 & B:2-3.  To power the
        // board from the WM8350, set C: 2-3 as well, otherwise leave unconnected.
        // Also make sure J17-J26 are connected if (and only if) powering off the WM8350.
        //
        DebugBreak();
    }

#if RUN_PMIC_TESTS
    PMICTests( g_hWMDevice );
#endif // TEST_COMMS


    //
    // Check we can still talk to the board.
    //
    status = WMDumpRegs( g_hWMDevice );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMDumpRegs failed: 0x%X - did you switch to the primary interface?\r\n"),
                  __WFUNCTION__,
                  status
                ));

        //
        // Breakpoint: you have got here because talking to the board failed.  Check the
        // jumper settings above.
        //
        DebugBreak();
        status = WMDumpRegs( g_hWMDevice );
        if ( !WM_SUCCESS( status ) )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s(): WMDumpRegs still failed: 0x%X\r\n"),
                      __WFUNCTION__,
                      status
                    ));
            goto Error;
        }
    }
#endif // PREPARE_ADS_BOARD

    // Initialize interrupt on PMIC.
    // Mask all interrupts
    {
        WM8350_INTERRUPT_SET interrupts = WM8350_ALL_INTERRUPTS;
        status = WMIntDisableSet( g_hWMDevice, interrupts.set );
        if ( !WM_SUCCESS( status ) )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s(): WMIntDisableSet failed: 0x%X\r\n"),
                      __WFUNCTION__,
                      status
                    ));
            goto Error;
        }

        // Clear all interrupts
        status = WMIntClearAll( g_hWMDevice );
        if ( !WM_SUCCESS( status ) )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s(): WMIntClearAll failed: 0x%X\r\n"),
                      __WFUNCTION__,
                      status
                    ));
            goto Error;
        }
    }

    // Initialize platform-specific configuration
    BSPPmicInit();

    // create event for PMIC interrupt signaling
    //      pEventAttributes = NULL (must be NULL)
    //      bManualReset = FALSE => resets automatically to nonsignaled
    //                              state after waiting thread released
    //      bInitialState = FALSE => initial state is non-signaled
    //      lpName = NULL => object created without a name
    hIntrEventPmic = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Check if CreateEvent failed
    if (hIntrEventPmic == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s(): CreateEvent failed!\r\n"),
            __WFUNCTION__));
        goto Error;
    }

    irq = BSPPmicGetIrq();

    // Call the OAL to translate the IRQ into a SysIntr value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &dwSysIntrPmic, sizeof(DWORD), NULL))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT
            ("ERROR: Failed to obtain sysintr value for PMIC interrupt.\r\n")));
        goto Error;
    }

    // Register PMIC interrupt
    if (!InterruptInitialize(dwSysIntrPmic, hIntrEventPmic, NULL, 0))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s(): InterruptInitialize failed!\r\n")
            __WFUNCTION__));
        goto Error;
    }

    // Ask the OAL to enable our interrupt to wake the system from suspend
    KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &dwSysIntrPmic,
        sizeof(dwSysIntrPmic), NULL, 0, NULL);

    bISTTerminate = FALSE;

    hPmicISTThread = CreateThread(NULL, 0,
                                  (LPTHREAD_START_ROUTINE)PmicIsrThreadProc,
                                  0, 0, NULL);
    if (!hPmicISTThread)
    {
        DEBUGMSG(ZONE_ERROR,
            (TEXT("PmicIsrThreadStart: CreateThread failed\r\n")));
        goto Error;
    }


    // Create event for blocking on ADC conversions to complete
    {
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, _T("EVENT_ADCDONE"));

        // Check if CreateEvent failed
        if ( !hEvent )
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s(): CreateEvent failed!\r\n"),
                __WFUNCTION__));
            goto Error;
        }
        status = WMIntRegister( g_hWMDevice,
                                WM8350_INT_AUXADC_DATARDY,
                                hEvent
                              );
    }

    // Unmask ADC complete interrupt
    status = WMIntEnable( g_hWMDevice, WM8350_INT_AUXADC_DATARDY );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMIntEnable (ADC DATARDY) failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto Error;
    }

    //
    // Unmask alarm interrupts
    //
    status = WMIntEnable( g_hWMDevice, WM8350_INT_RTC_ALM );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMIntEnable (RTC_ALM) failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto Error;
    }

    //
    // Unmask watchdog interrupts.  Note this doesn't actually enable the
    // watchdog, just makes sure we can handle it if it gets enabled.
    //
    status = WMIntEnable( g_hWMDevice, WM8350_INT_SYS_WDOG_TO );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMIntEnable (RTC_ALM) failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto Error;
    }

    //
    // Set the PMIC up so that it can enter and exit hibernate
    //
    status = PMICHibernateSetup();
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): PMICHibernateSetup failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto Error;
    }

    DEBUGMSG(ZONE_INIT || ZONE_FUNC, (TEXT("-%s returning %d\r\n"),
        __WFUNCTION__, initializedPMIC));

    return initializedPMIC;

Error:
    CleanupPMIC();

    return FALSE;

}

//-----------------------------------------------------------------------------
//
// Function: CleanupPMIC
//
// This function unmaps the registry space for the PMIC registers.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID
CleanupPMIC()
{

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s\r\n"), __WFUNCTION__));

    PmicCspiRelease(BSPPmicGetSpiPort());

    // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntrPmic, sizeof(DWORD),
                    NULL, 0, NULL);
    dwSysIntrPmic = (DWORD) SYSINTR_UNDEFINED;

    // If PMIC no longer needs service
    // Clear the gpio interrupt flag(w1c)
    // Deinitialize platform-specific configuration
    BSPPmicDeinit();

    // kill the IST thread
    if (hPmicISTThread)
    {
        bISTTerminate = TRUE;
        CloseHandle(hPmicISTThread);
        hPmicISTThread = NULL;
    }

    // Close interrupt event handle
    if (hIntrEventPmic)
    {
        CloseHandle(hIntrEventPmic);
        hIntrEventPmic = NULL;
    }

    // Close our device handle
    WMPmicCloseDevice( g_hWMDevice );
    g_hWMDevice = WM_HANDLE_INVALID;

    // Delete critical sections
    DeleteCriticalSection(&pmicCs);
    DeleteCriticalSection(&adcCs);
    DeleteCriticalSection(&intRegCs);

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s\r\n"), __WFUNCTION__));
}

//-----------------------------------------------------------------------------
//
// Function: PMICHibernateSetup
//
// This function sets up the PMIC signals for HIBERNATE and \WAKEUP
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static WM_STATUS PMICHibernateSetup()
{
    WM_STATUS status;
    WM_GPIO_CONFIG wakeupConfig, hibernateConfig, pwrdyConfig;


    //
    // Configure GPIO as the PIMC ready signal.
    //
    if(BSPPmicGetPWRDYGPIOConfig( &pwrdyConfig ) == TRUE)
    {
        status = WMGPConfigFunction( g_hWMDevice, pwrdyConfig );
        if ( !WM_SUCCESS( status ) )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s(): WMGPConfigFunction failed: 0x%X\r\n"),
                      __WFUNCTION__,
                      status
                    ));
            goto done;
        }
    }


    //
    // Configure GPIO as the wakeup source.
    //
    if(BSPPmicGetWakeupGPIOConfig( &wakeupConfig ) == TRUE)
    {
    status = WMGPConfigFunction( g_hWMDevice, wakeupConfig );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMGPConfigFunction failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto done;
    }
    }

#if WM_PMIC_HIBERNATE
    //
    // Configure Hibernate mode
    //
    status = WMPmicConfigureHibernate( g_hWMDevice );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMPmicConfigureHibernate failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto done;
    }

    //
    // Configure GPIO as the hibernate source.
    //
    if( BSPPmicGetHibernateGPIOConfig( &hibernateConfig ) == TRUE)
    {
    status = WMGPConfigFunction( g_hWMDevice, hibernateConfig );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMGPConfigFunction failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto done;
    }
    }

#else
    //
    // Configure GPIO as GPIO.
    //
    if( BSPPmicGetHibernateGPIOConfig( &hibernateConfig ) == TRUE)
    {
    hibernateConfig.function = 0;

    status = WMGPConfigFunction( g_hWMDevice, hibernateConfig );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s(): WMGPConfigFunction failed: 0x%X\r\n"),
                  __WFUNCTION__,
                  status
                ));
        goto done;
    }
    }
#endif /* WM_PMIC_HIBERNATE */

done:
    return status;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityOpHandler
//
// This function does the general operations (Reset and Set Mode) for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//               inParam->PARAMS.ifMode contains the desired mode in case of
//               OP_SET_MODE.
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityOpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                 PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityRs232OpHandler
//
// This function does the config setting in RS232 mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityRs232OpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                      PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityCea936OpHandler
//
// This function does the config setting in CEA-936 mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityCea936OpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                       PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbSetSpeedHandler
//
// This function does the speed setting in USB mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbSetSpeedHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                          PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbSetPowerSourceHandler
//
// This function does the power specific setting in USB mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbSetPowerSourceHandler(
                                                 PMIC_PARAM_CONVITY_OP *inParam,
                                                 PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbSetXcvrHandler
//
// This function does the Transceiver specific setting in USB mode for
// connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbSetXcvrHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                         PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgBeginHnpHandler
//
// This function does the settings for beginning the HNP protocol in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgBeginHnpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                             PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgEndHnpHandler
//
// This function does the settings for ending the HNP protocol in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgEndHnpHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                           PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgSetConfigHandler
//
// This function does the settings for USB OTG in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgSetConfigHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                              PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicIoctlConvityUsbOtgClearConfigHandler
//
// This function clears the settings for USB OTG in USB mode
// for connectivity interface.
//
// Parameters:
//      inParam
//          [in] PMIC_PARAM_CONVITY_OP contains op code and specific information
//
//      result
//          [out] PMIC_STATUS
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//------------------------------------------------------------------------------
static BOOL PmicIoctlConvityUsbOtgClearConfigHandler(PMIC_PARAM_CONVITY_OP *inParam,
                                                     PMIC_STATUS *result)
{
    UNREFERENCED_PARAMETER( inParam );

    *result = PMIC_NOT_SUPPORTED;

    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: PmicADCGetSettings
//
// This function gets the adc configuration information.
//
// Parameters:
//      channel
//           [in] channels to be configured.
//      group
//           [out] group setting.
//      chann
//           [out] channel setting in ADC1.
//      ena
//           [out] bits to be enabled in ADC0.
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
//static PMIC_STATUS PmicADCGetSettings(UINT16 channel, UINT32 *pGroup,
//                                      UINT32 *pChann, UINT32 *pEna)
//{
//    UNREFERENCED_PARAMETER( channel );
//    UNREFERENCED_PARAMETER( pGroup );
//    UNREFERENCED_PARAMETER( pChann );
//    UNREFERENCED_PARAMETER( pEna );
//
//    // ### TBD
//    return PMIC_NOT_SUPPORTED;
//}

//-----------------------------------------------------------------------------
//
// Function: PmicADCConvert
//
// This function triggers a new set of ADC conversions and waits for the
// conversions to complete.
//
// Parameters:
//      mode
//
// Returns:
//      Status.
//
//-----------------------------------------------------------------------------
//static PMIC_STATUS PmicADCConvert(void)
//{
//    // ### TBD
//    return PMIC_NOT_SUPPORTED;
//}

//-----------------------------------------------------------------------------
//
// Function: PmicADCDisable
//
// This function is just a helper function that disables ADC conversions
//
// Parameters:
//
// Returns:
//
//-----------------------------------------------------------------------------
//static VOID PmicADCDisable(void)
//{
//    // ### TBD
//}

//-----------------------------------------------------------------------------
//
// Function: PmicReadEightValues
//
//      This function is a helper function that reads eight values at a time
// from the ADC. This method uses the auto increment feature with two offset
// pointers. This is done to optimize the ADC read. Uses 4 CSPI transactions
// compared to 16.
//
// Parameters:
//      pReturnVal
//          [out]   This points to the eight returned UINT16 values.
//
// Returns:
//
//-----------------------------------------------------------------------------
//static VOID PmicReadEightValues( UINT16* pReturnVal )
//{
//    UNREFERENCED_PARAMETER( pReturnVal );
//    // ### TBD
//}

//-----------------------------------------------------------------------------
//
// Function: PMICAdcSetMode
//
// This function sets the mode for the PMIC ADC.
//
// Parameters:
//      mode
//          [in] Requested ADC.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
//static BOOL PMICAdcSetMode(PMIC_TOUCH_MODE mode)
//{
//    UNREFERENCED_PARAMETER( mode );
//    // ### TBD
//    return FALSE;
//}

//-----------------------------------------------------------------------------
//
// Function: PMICIoctlAdcSetMode
//
// This function sets the mode for the PMIC ADC.
//
// Parameters:
//      pBufIn
//          [in] Points to requested PMIC_TOUCH_MODE.
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] Unused.
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL PMICIoctlAdcSetMode(PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut,
                         DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER( pBufIn );
    UNREFERENCED_PARAMETER( dwLenIn );
    UNREFERENCED_PARAMETER( pBufOut );
    UNREFERENCED_PARAMETER( dwLenOut );

    // ### TBD
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PMICIoctlIntEnable
//
// This function enables the selected PMIC interrupt
//
// Parameters:
//      IntID
//          [in] Interrupt ID.
//
//      EnableFlag
//          [in] Flag set to TRUE = enable, FALSE = disable
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
extern "C"  BOOL PMICIoctlIntEnable(UINT32 IntID, BOOL EnableFlag)
{
    WM_STATUS status;

    EnterCriticalSection(&intRegCs);

    if ( EnableFlag )
        status = WMIntEnable( g_hWMDevice, IntID );
    else
        status = WMIntDisable( g_hWMDevice, IntID );

    LeaveCriticalSection(&intRegCs);

    if ( WM_SUCCESS( status ) )
        return TRUE;
    else
        return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PMICIoctlAdcTouchRead
//
// This function places the PMIC in touch position mode and acquires
// touch samples.
//
// Parameters:
//      pBufIn
//          [in] Unused.
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] Points to data buffer to hold touch samples.
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
static BOOL PMICIoctlAdcTouchRead(PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut,
                      DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER( pBufIn );
    UNREFERENCED_PARAMETER( dwLenIn );
    UNREFERENCED_PARAMETER( pBufOut );
    UNREFERENCED_PARAMETER( dwLenOut );

    // Not supported on WM8350
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCGetHandsetCurrent
//
// This function gets handset battery current measurement values.
//
// Parameters:
//      pBufIn
//          [in] Points to requested ADC converter mode:
//               ADC_8CHAN_1X  or  ADC_1CHAN_8X.
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] pointer to the handset battery current measurement
//               value(s).
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
static BOOL PmicIoctlADCGetHandsetCurrent(PBYTE pBufIn, DWORD dwLenIn,
                        PBYTE pBufOut, DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER( pBufIn );
    UNREFERENCED_PARAMETER( dwLenIn );
    UNREFERENCED_PARAMETER( pBufOut );
    UNREFERENCED_PARAMETER( dwLenOut );

    //
    // Not sure what this is and whether it maps on to any of the WM8350
    // ADC inputs.  Better handled through generic code, probably.
    //
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCGetMultipleChannelsSamples
//
// This function gets one sample for multiple channels.
//
// Parameters:
//      channels
//          [in] selected  channels (up to 16 channels).
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//          [out] pointer to  the sampled values (up to 16 sampled values).
//               The returned results will be ordered according to the channel
//               selected.
//      dwLenOut
//          [out] Unused.
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL PmicIoctlADCGetMultipleChannelsSamples(PBYTE pBufIn, DWORD dwLenIn,
                                        PBYTE pBufOut, DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER( pBufIn );
    UNREFERENCED_PARAMETER( dwLenIn );
    UNREFERENCED_PARAMETER( pBufOut );
    UNREFERENCED_PARAMETER( dwLenOut );

    // ### TBD
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCGetSingleChannelSample
//
// This function gets one channel sample. It can be a single sample or multiple
// depending on the bMultipleSampleFlag.
//
// Parameters:
//      pBufIn
//          [in] pointer to the selected channel
//
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//         [out] pointer to  the sampled values
//
//      dwLenOut
//          [out] Unused.
//
//      bMultipleSampleFlag
//          [in] Flag to chose between 1x and 8x samples ... if TRUE then 8x
//
// Returns:
//      TRUE if the successful, FALSE otherwise.
//-----------------------------------------------------------------------------
BOOL
PmicIoctlADCGetSingleChannelSample(PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut,
                                        DWORD dwLenOut, BOOL bMultipleSampleFlag)
{
    UNREFERENCED_PARAMETER( pBufIn );
    UNREFERENCED_PARAMETER( dwLenIn );
    UNREFERENCED_PARAMETER( pBufOut );
    UNREFERENCED_PARAMETER( dwLenOut );
    UNREFERENCED_PARAMETER( bMultipleSampleFlag );

    // ### TBD
    return FALSE;
}


//-----------------------------------------------------------------------------
//
// Function: PmicIoctlADCSetComparatorThresholds
//
// This function sets WHIGH and WLOW for automatic ADC result comparators.
//
// Parameters:
//      pBufIn
//          [in] pointer to threshold array
//                   threshold[0] = whigh
//                   threshold[1] = wlow
//      dwLenIn
//          [in] Unused.
//
//      pBufOut
//         [out] Unused
//
//      dwLenOut
//          [out] Unused.
//
// Returns:
//              Status.
//-----------------------------------------------------------------------------
BOOL PmicIoctlADCSetComparatorThresholds(PBYTE pBufIn, DWORD dwLenIn,
                        PBYTE pBufOut, DWORD dwLenOut)
{
    UNREFERENCED_PARAMETER( pBufIn );
    UNREFERENCED_PARAMETER( dwLenIn );
    UNREFERENCED_PARAMETER( pBufOut );
    UNREFERENCED_PARAMETER( dwLenOut );

    // ### TBD
    return FALSE;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.  This function will retrieve the handle to the
// device context from the registry and then initialize the PMI module by
// turning off all of the peripherals that are not being used.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD
PMI_Init(LPCTSTR pContext)
{
    DWORD rc = 0;

#ifndef DEBUG
    UNREFERENCED_PARAMETER( pContext );
#endif

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("+%s(%s)\r\n"), __WFUNCTION__, pContext));

    // Initialize the PMIC driver
    if (!InitializePMIC())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s:  Failed to initialize PMIC!!!\r\n"),
            __WFUNCTION__));
        goto cleanUp;
    }

    rc = 1;

cleanUp:
    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("-%s() returning 1\r\n"), __WFUNCTION__));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Deinit
//
// The Device Manager calls this function as a result of a call to the
// DeactivateDevice() function.  This function will return any resources
// allocated while using this driver.
//
// Parameters:
//      hDeviceContext
//          [in] The handle to the context.
//
// Returns:
//      TRUE
//
//-----------------------------------------------------------------------------
BOOL
PMI_Deinit(DWORD hDeviceContext)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER( hDeviceContext );
#endif

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("+%s(%d)\r\n"), __WFUNCTION__, hDeviceContext));

    CleanupPMIC();

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("-%s() returning %d\r\n"), __WFUNCTION__, FALSE));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: PMI_Open
//
// Called when an application attempts to establish a connection to this driver
// This function will verify that a trusted application has made the request
// and deny access to all non-trusted applications.
//
// Parameters:
//      hDeviceContext
//          [in] Ignored
//      AccessCode
//          [in] Ignored
//      ShareMode
//          [in] Ignored
//
// Returns:
//      Returns 0 if the calling application is not trusted and 1 if it is
//      trusted.
//
//-----------------------------------------------------------------------------
DWORD
PMI_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER( hDeviceContext );
    UNREFERENCED_PARAMETER( AccessCode );
    UNREFERENCED_PARAMETER( ShareMode );
#endif

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d, %d, %d)\r\n"), __WFUNCTION__,
        hDeviceContext, AccessCode, ShareMode));


    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() returning 1\r\n"), __WFUNCTION__));

    return 1;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Close
//
// Called when an application attempts to close a connection to this driver.
// This function does nothing.
//
// Parameters:
//      hOpenContext
//          [in] Ignored
//
// Returns:
//      TRUE
//
//-----------------------------------------------------------------------------
BOOL
PMI_Close(DWORD hOpenContext)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER( hOpenContext );
#endif

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d)\r\n"), __WFUNCTION__, hOpenContext));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() returning TRUE\r\n"), __WFUNCTION__));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_IOControl
//
// Called when an application calls the DeviceIoControl() function.  This
// function operates differently based upon the IOCTL that is passed to it.
// The following table describes the expected values associated with each
// IOCTL implemented by this function.
//
// dwCode                      pBufIn         pBufOut         Description
// --------------------------- -------------- --------------- -----------------
//
// Parameters:
//      hOpenContext
//          [in] Ignored
//
//      dwCode
//          [in] The IOCTL requested.
//
//      pBufIn
//          [in] Input buffer.
//
//      dwLenIn
//          [in] Length of the input buffer.
//
//      pBufOut
//          [out] Output buffer.
//
//      dwLenOut
//          [out] The length of the output buffer.
//
//      pdwActualOut
//          [out] Size of output buffer returned to application.
//
// Returns:
//      TRUE if the IOCTL is handled. FALSE if the IOCTL was not recognized or
//      an error occurred while processing the IOCTL
//
//-----------------------------------------------------------------------------
BOOL
PMI_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    BOOL result = FALSE;
    UINT32 temp;
    PMIC_PARAM_LLA_WRITE_REG* pwr;
    PMIC_PARAM_INT_REGISTER* pir;

    UNREFERENCED_PARAMETER( hOpenContext );
    //DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d, 0x%X)\r\n"), __WFUNCTION__, hOpenContext, dwCode));

    switch (dwCode)
    {
    case PMIC_IOCTL_LLA_READ_REG:
        //DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_LLA_READ_REG\r\n")));
        temp = *pBufIn;

        EnterCriticalSection(&adcCs);
        GetRegister( temp, (UINT32*)pBufOut );
        LeaveCriticalSection(&adcCs);

        //DEBUGMSG( ZONE_INFO, (
        //          _T("PMI_IOControl PMIC_IOCTL_LLA_READ_REG: R%02XH = 0x%04X\r\n"),
        //          temp,
        //          *((UINT32*) pBufOut)
        //        ));

        result = TRUE;
        break;

    case PMIC_IOCTL_LLA_WRITE_REG:
        pwr = (PMIC_PARAM_LLA_WRITE_REG*)pBufIn;

        DEBUGMSG( ZONE_INFO, (
                  _T("PMI_IOControl PMIC_IOCTL_LLA_WRITE_REG: R%02Xh => 0x%04X/0x%04X\r\n"),
                  pwr->addr,
                  pwr->data,
                  pwr->mask
                ));

        if ( pwr->data != (pwr->data & pwr->mask) )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("%s: data and mask don't match: 0x%04X/0x%04X\r\n"),
                      pwr->data,
                      pwr->mask
                    ));
        }

        EnterCriticalSection(&adcCs);
        SetRegister( pwr->addr,
                     (pwr->data & PMIC_ALL_BITS),
                     (pwr->mask & PMIC_ALL_BITS)
                   );
        LeaveCriticalSection(&adcCs);

        result = TRUE;
        break;

    case PMIC_IOCTL_LLA_BLOCK_CONTROL_IF:
        // Completely blocks access to the control interface.  Use with caution.
        DEBUGMSG( ZONE_INFO, (
                  _T("PMI_IOControl PMIC_IOCTL_LLA_BLOCK_CONTROL_IF\r\n")
                ));

        result = BSPPmicBlockControlIF();

        break;

    case PMIC_IOCTL_LLA_UNBLOCK_CONTROL_IF:
        // Releases access after PMIC_IOCTL_LLA_BLOCK_CONTROL_IF.
        DEBUGMSG( ZONE_INFO, (
                  _T("PMI_IOControl PMIC_IOCTL_LLA_BLOCK_CONTROL_IF\r\n")
                ));

        BSPPmicUnblockControlIF();

        result = TRUE;

        break;

    case PMIC_IOCTL_LLA_INT_REGISTER:
    {
        HANDLE      hEvent;
        WM_STATUS   status;
        LPTSTR      eventName;

        pir = (PMIC_PARAM_INT_REGISTER*)pBufIn;

        DEBUGMSG( ZONE_INFO, (
                  _T("PMI_IOControl PMIC_IOCTL_LLA_INT_REGISTER: %d\r\n"),
                  pir->int_id
                ));

        eventName = (LPTSTR) MapCallerPtr(pir->event_name, MAX_PATH);

        if (eventName == NULL)
        {
            DEBUGMSG(ZONE_ERROR,
                    (_T("MapCallerPtr failed for event_name.\r\n")));
            result = FALSE;
            break;
        }

        hEvent = CreateEvent(NULL, FALSE, FALSE, eventName);
        if ( !hEvent )
        {
            DEBUGMSG( ZONE_ERROR, (
                      _T("CreateEvent failed for event_name '%s'.\r\n"),
                      eventName
                    ));
            result = FALSE;
            break;
        }

        // Register for the event
        status = WMIntRegister( g_hWMDevice, pir->int_id, hEvent );
        if ( WM_SUCCESS( status ) )
        {
            // Unmask the interrupt
            EnterCriticalSection(&intRegCs);
            status = WMIntEnable( g_hWMDevice, pir->int_id );
            LeaveCriticalSection(&intRegCs);
        }

        if ( !WM_SUCCESS( status ) )
        {
            CloseHandle( hEvent );
            DEBUGMSG( ZONE_ERROR, (
                      _T("WMIntRegister/WMIntEnable failed for ID 0x%X: 0x%X.\r\n"),
                      pir->int_id,
                      status
                    ));
            result = FALSE;
            break;
        }

        result = TRUE;

        break;
    }

    case PMIC_IOCTL_LLA_INT_DEREGISTER:
    {
        temp = *pBufIn;

        DEBUGMSG( ZONE_FUNC, (
                  _T("PMI_IOControl PMIC_IOCTL_LLA_INT_DEREGISTER: %d\r\n"),
                  temp
                ));

        WM_STATUS status;

        // "temp" is the IntId
        status = WMIntDeregister( g_hWMDevice, temp );
        if ( WM_SUCCESS( status ) )
            result = TRUE;
        break;
    }

    case PMIC_IOCTL_LLA_INT_COMPLETE:
        DEBUGMSG(ZONE_FUNC,
                    (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_COMPLETE\r\n")));
        // Intentional fall through no break needed

    case PMIC_IOCTL_LLA_INT_ENABLE:
        DEBUGMSG(ZONE_FUNC,
                    (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_ENABLE\r\n")));
        result = PMICIoctlIntEnable(*pBufIn, TRUE);
        break;

    case PMIC_IOCTL_LLA_INT_DISABLE:
        DEBUGMSG(ZONE_FUNC,
                    (_T("PMI_IOControl PMIC_IOCTL_LLA_INT_DISABLE\r\n")));
        result = PMICIoctlIntEnable(*pBufIn, FALSE);
        break;

    case PMIC_IOCTL_ADC_TOUCH_READ:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_ADC_TOUCH_READ\r\n")));
        result = PMICIoctlAdcTouchRead(pBufIn, dwLenIn, pBufOut, dwLenOut);
        break;

    case PMIC_IOCTL_ADC_SET_MODE:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_ADC_SET_MODE\r\n")));
        result = PMICIoctlAdcSetMode(pBufIn, dwLenIn, pBufOut, dwLenOut);
        break;

    case PMIC_IOCTL_ADC_GET_HS_CURRENT:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_ADC_GET_HS_CURRENT\r\n")));
        result = PmicIoctlADCGetHandsetCurrent(pBufIn, dwLenIn, pBufOut,
                    dwLenOut);
        break;

    case PMIC_IOCTL_ADC_GET_MUL_CH_SPL:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_ADC_GET_MUL_CH_SPL\r\n")));
        result = PmicIoctlADCGetMultipleChannelsSamples(pBufIn, dwLenIn,
                    pBufOut, dwLenOut);
        break;

    case PMIC_IOCTL_ADC_GET_SGL_CH_8SPL:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_ADC_GET_SGL_CH_8SPL\r\n")));
        result = PmicIoctlADCGetSingleChannelSample(pBufIn, dwLenIn, pBufOut,
                    dwLenOut, TRUE);
        break;

    case PMIC_IOCTL_ADC_GET_SGL_CH_1SPL:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_ADC_GET_SGL_CH_1SPL\r\n")));
        result = PmicIoctlADCGetSingleChannelSample(pBufIn, dwLenIn, pBufOut,
                    dwLenOut, FALSE);
        break;

    case PMIC_IOCTL_ADC_SET_CMPTR_TRHLD:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_ADC_SET_CMPTR_TRHLD\r\n")));
        result = PmicIoctlADCSetComparatorThresholds(pBufIn, dwLenIn, pBufOut,
                    dwLenOut);
        break;

    case PMIC_IOCTL_CONVT_OP:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_OP\r\n")));
        result = PmicIoctlConvityOpHandler((PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_RS232_OP:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_RS232_OP\r\n")));
        result = PmicIoctlConvityRs232OpHandler((PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_CEA936_OP:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_CEA936_OP\r\n")));
        result = PmicIoctlConvityCea936OpHandler((PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETSPEED:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_USB_SETSPEED\r\n")));
        result = PmicIoctlConvityUsbSetSpeedHandler(
                    (PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETPWR:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_USB_SETPWR\r\n")));
        result = PmicIoctlConvityUsbSetPowerSourceHandler(
                    (PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETXCVR:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_USB_SETXCVR\r\n")));
        result = PmicIoctlConvityUsbSetXcvrHandler(
                    (PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_BGNHNP:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_USB_BGNHNP\r\n")));
        result = PmicIoctlConvityUsbOtgBeginHnpHandler(
                    (PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_ENDHNP:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_USB_ENDHNP\r\n")));
        result = PmicIoctlConvityUsbOtgEndHnpHandler(
                    (PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_SETCFG:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_USB_SETCFG\r\n")));
        result = PmicIoctlConvityUsbOtgSetConfigHandler(
                    (PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;

    case PMIC_IOCTL_CONVT_USB_CLRCFG:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl PMIC_IOCTL_CONVT_USB_CLRCFG\r\n")));
        result = PmicIoctlConvityUsbOtgClearConfigHandler(
                    (PMIC_PARAM_CONVITY_OP*)pBufIn,
                    (PMIC_STATUS*)pBufOut);
        break;


    case IOCTL_POWER_CAPABILITIES:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl IOCTL_POWER_CAPABILITIES\r\n")));
        // Tell the power manager about ourselves.
        if (pBufOut != NULL
            && dwLenOut >= sizeof(POWER_CAPABILITIES)
            && pdwActualOut != NULL)
        {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(*ppc));
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D4);
                *pdwActualOut = sizeof(*ppc);
                result = TRUE;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_CAPABILITIES\r\n")));
            }
        }

        break;


    case IOCTL_POWER_SET:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl IOCTL_POWER_SET\r\n")));
        if(pBufOut != NULL
            && dwLenOut == sizeof(CEDEVICE_POWER_STATE)
            && pdwActualOut != NULL)
        {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try
            {
                CEDEVICE_POWER_STATE dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                if(VALID_DX(dx))
                {
                    // Any request that is not D0 becomes a D4 request
                    if(dx != D0)
                    {
                        dx = D4;
                    }

                    *(PCEDEVICE_POWER_STATE) pBufOut = dx;
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                    EnterCriticalSection(&adcCs);
                    EnterCriticalSection(&pmicCs);

                    // If we are turning off
                    if (dx == D4)
                    {
                        // Force polled CSPI communication to avoid
                        // synchronization issues with suspending or powering off
                        PmicConfigPolling(TRUE);

                        // Set global flag to force subsequent ADC conversions
                        // into polled mode
                        g_bAdcPolled = TRUE;

                        BSPPmicPowerNotifySuspend();
                    }

                    // Else we are powering on
                    else
                    {
                        // Restore previous CSPI transfer mode
                        PmicConfigPolling(g_bPmicUsePolling);

                        // Set global flag to force subsequent ADC conversions
                        // into polled mode
                        g_bAdcPolled = FALSE;

                        BSPPmicPowerNotifyResume();
                    }

                    LeaveCriticalSection(&pmicCs);
                    LeaveCriticalSection(&adcCs);

                    result = TRUE;
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_SET\r\n")));
            }
        }
        break;

    case IOCTL_POWER_GET:
        DEBUGMSG(ZONE_FUNC, (_T("PMI_IOControl IOCTL_POWER_GET\r\n")));
        if(pBufOut != NULL
            && dwLenOut == sizeof(CEDEVICE_POWER_STATE)
            && pdwActualOut != NULL) {
            // Just return our current Dx value
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try
            {
                *(PCEDEVICE_POWER_STATE) pBufOut = g_dxCurrent;
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                result = TRUE;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE, (_T("Exception in DVFC IOCTL_POWER_SET\r\n")));
            }
        }
        break;

    case I2C_IOCTL_SET_SLAVE_MODE:
    {
        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:SET_SLAVE_MODE +\r\n")));
        result = TRUE;
        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:SET_SLAVE_MODE -\r\n")));
        break;
    }
    case I2C_IOCTL_SET_MASTER_MODE:
    {
        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:SET_MASTER_MODE +\r\n")));
        result = TRUE;
        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:SET_MASTER_MODE +\r\n")));
        break;
    }
    case I2C_IOCTL_IS_MASTER:
    {
        if (dwLenOut != sizeof(BOOL))
            return FALSE;

        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:IS_MASTER +\r\n")));
        PBOOL pbIsMaster = (PBOOL) MapCallerPtr(pBufOut, sizeof(BOOL));
        *pbIsMaster = TRUE;
        result = TRUE;
        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:IS_MASTER - Val=0x%x\r\n"), *pbIsMaster));
        break;
    }
    case I2C_IOCTL_IS_SLAVE:
    {
        if (dwLenOut != sizeof(BOOL))
            return FALSE;

        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:IS_SLAVE +\r\n")));
        PBOOL pbIsSlave = (PBOOL) MapCallerPtr(pBufOut, sizeof(BOOL));
        *pbIsSlave = FALSE;
        result = TRUE;
        DEBUGMSG (ZONE_FUNC, (TEXT("I2C_IOControl:IS_SLAVE - Val=0x%x\r\n"), *pbIsSlave));
        break;
    }
    case I2C_IOCTL_TRANSFER:
    {
        DEBUGMSG (ZONE_IOCTL, (TEXT("I2C_IOControl:I2C_IOCTL_TRANSFER +\r\n")));
        I2C_TRANSFER_BLOCK *pXferBlock = (I2C_TRANSFER_BLOCK *) pBufIn;
        I2C_PACKET *pPackets = (I2C_PACKET *) MapCallerPtr(pXferBlock->pI2CPackets, sizeof(I2C_PACKET) * pXferBlock->iNumPackets);

        // Map pointers for each packet in the array
        for (int i = 0; i < pXferBlock->iNumPackets; i++)
        {
            pPackets[i].pbyBuf = (PBYTE) MapCallerPtr(pPackets[i].pbyBuf, pPackets[i].wLen);
        }

        result = PmicI2CProcessPackets(pPackets, pXferBlock->iNumPackets);
        DEBUGMSG (ZONE_IOCTL, (TEXT("I2C_IOControl:I2C_IOCTL_TRANSFER -\r\n")));
        break;
    }

    default:
        DEBUGMSG(ZONE_ERROR,
            (TEXT("%s: Unrecognized IOCTL 0x%X\r\n"), __WFUNCTION__, dwCode));
        result = FALSE;
        break;
    }

    //DEBUGMSG(ZONE_FUNC,
    //            (TEXT("-%s(0x%X) returning %d\r\n"), __WFUNCTION__, dwCode, result));

    return result;
}


//-----------------------------------------------------------------------------
//
// Function: PMI_PowerUp
//
// This function restores power to a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PMI_PowerUp(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER( hDeviceContext );
}


//-----------------------------------------------------------------------------
//
// Function: PMI_PowerDown
//
// This function suspends power to the device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PMI_PowerDown(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER( hDeviceContext );

    PmicI2CPowerDown();
    PmicCspiPowerDown();
}


//-----------------------------------------------------------------------------
//
//  Function: SetRegister
//
//  Sets a PMIC register.
//
//  Parameters:
//      regAddr     Register index
//      regval      New value
//      mask        Mask of valid bits
//
//  Returns:
//      TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL SetRegister( UINT32 regAddr, UINT32 regval, UINT32 mask )
{
    UINT32  content;
    BOOL    retval;

    // If it is not updating the whole register (16 bits), we need to read
    // the register first.  Then we set the bits and write it back.
    EnterCriticalSection( &pmicCs );
    if ((mask & PMIC_ALL_BITS) ^ PMIC_ALL_BITS)
    {
        GetRegister( regAddr, &content );
        content &= ~mask;
        content |= regval & mask;
    }
    else
    {
        content = regval;
    }

    retval = BSPPmicControlWrite( regAddr, content );

    LeaveCriticalSection( &pmicCs );

    return retval;
}


//-----------------------------------------------------------------------------
//
//  Function: GetRegister
//
//  Reads a PMIC register.
//
//  Parameters:
//      regAddr     Register index
//      pRegval     Variable to receive register value
//
//  Returns:
//      TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------
BOOL GetRegister( UINT32 regAddr, UINT32* pRegval )
{
    return BSPPmicControlRead( regAddr, pRegval );
}


//-----------------------------------------------------------------------------
//
//  Function: PmicConfigPolling
//
//  Configures whether to use polling for the control interface.
//
//  Parameters:
//      bPoll   Whether to use polling - TRUE = polled, FALSE = interrupts
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void PmicConfigPolling( BOOL bPoll )
{
    g_bUsePolling = bPoll;
}


//-----------------------------------------------------------------------------
//
// Function: PmicIsrThreadProc
//
// ISR Thread Process that launches IST loop.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//-----------------------------------------------------------------------------
static BOOL
PmicIsrThreadProc()
{
    UINT32 pmicInt;
    WM_STATUS               status;
    WM8350_INTERRUPT_SET    interrupts;


    // Set thread priority.  Make sure this priority is higher (lower value)
    // than any PMIC client driver (touch, battery, audio, etc.) so that the
    // interrupt handling is properly sequenced.  In addition, make sure this
    // priority lower (higher value) than the CspiIntrServThread since
    // this thread will block on CSPI transfers.
    CeSetThreadPriority(GetCurrentThread(), 98);

    while(!bISTTerminate)
    {
        if (WaitForSingleObject(hIntrEventPmic, INFINITE) == WAIT_OBJECT_0)
        {
            DEBUGMSG( ZONE_INTR, (_T("%s: Interrupt received\r\n"), __WFUNCTION__ ));

            // Keep processing interrupts PMIC interrupt request is deasserted
            do
            {
                EnterCriticalSection(&intRegCs);

                //
                // Query the current interrupt status.  Note this clears and
                // disables all interrupts.
                //
                status = WMIntGetStatusAndMask( g_hWMDevice, interrupts.set );
                if ( !WM_SUCCESS( status ) )
                {
                    DEBUGMSG( ZONE_ERROR, (
                              _T("%s: WMIntGetStatus failed: 0x%X\r\n"),
                              status
                            ));
                }

                //
                // Don't mask the alarm interrupt
                //
                status = WMIntEnable( g_hWMDevice, WM8350_INT_RTC_ALM );
                if ( !WM_SUCCESS( status ) )
                {
                    DEBUGMSG( ZONE_ERROR, (
                              _T("%s: WMIntEnable (TOD) failed: 0x%X\r\n"),
                              status
                            ));
                }

                //
                // ... or the watchdog interrupt
                //
                status = WMIntEnable( g_hWMDevice, WM8350_INT_SYS_WDOG_TO );
                if ( !WM_SUCCESS( status ) )
                {
                    DEBUGMSG( ZONE_ERROR, (
                              _T("%s: WMIntEnable (TOD) failed: 0x%X\r\n"),
                              status
                            ));
                }

                LeaveCriticalSection(&intRegCs);

                DEBUGMSG( ZONE_INTR, (
                          _T("PMIC_INT:  Active Interrupts[0:3] = 0x%04X 0x%04X 0x%04X 0x%04X\r\n"),
                          interrupts.set[0],
                          interrupts.set[1],
                          interrupts.set[2],
                          interrupts.set[3]
                        ));
                DEBUGMSG( ZONE_INTR, (
                          _T("PMIC_INT:  Active Interrupts[4:7] = 0x%04X 0x%04X 0x%04X 0x%04X\r\n"),
                          interrupts.set[4],
                          interrupts.set[5],
                          interrupts.set[6],
                          interrupts.set[7]
                        ));

                // Service all active interrupts.

                //  check for TOD alarm
                if ( interrupts.ints.RTC_ALM )
                {
                    BSPPmicSignalOALRTC();
                }

                // Now signal registered events
                // Note:  Rescheduling may occur and could result
                // in more interrupts pending.
                status = WMIntProcessInterrupts( g_hWMDevice, interrupts.set );
                if ( !WM_SUCCESS( status ) )
                {
                    DEBUGMSG( ZONE_ERROR, (
                              _T("%s: WMIntProcessInterrupts failed: 0x%X\r\n"),
                              status
                            ));
                }

                BSPPmicClearIrq();
                if (!BSPPmicGetIrqStatus(&pmicInt))
                {
                    ERRORMSG(1, (_T("DDKGpioReadIntrPin failed!\r\n")));
                    pmicInt = 0;
                }
            } while (pmicInt);
            InterruptDone(dwSysIntrPmic);
        }
    }

    ExitThread(TRUE);

    return TRUE;
}
