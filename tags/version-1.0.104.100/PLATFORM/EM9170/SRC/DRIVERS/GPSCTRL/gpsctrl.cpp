//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
// GPSCtl.cpp : Defines the entry point for the DLL application.
//
#pragma warning(disable: 4100 4201)
#include "GPSCtrl.h"
#include <pm.h>
#include <GlobalLocate/CriticalSection.h>
#include <GlobalLocate/GenericIoctl.h>
#include <GlobalLocate/Gll/glgpsapi.h>
#include "Log/DebugZones.h"
#include "IGPSCtlPdd.h"

using namespace GlobalLocate;
bool isOpen = false;

CEDEVICE_POWER_STATE g_dxCurrent = D0;


BOOL
DllEntry(
              HINSTANCE   hinstDll,             /*@parm Instance pointer. */
              DWORD   dwReason,                 /*@parm Reason routine is called. */
              LPVOID  lpReserved                /*@parm system parameter. */
              )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER(hinstDll);
        DEBUGMSG (ZONE_FUNCTION, (TEXT("serial port process attach\r\n")));
        DisableThreadLibraryCalls((HMODULE) hinstDll);
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return(TRUE);
}

GPS_API DWORD GPS_Init(LPCTSTR pContext, LPCVOID lpvBusContext)
{
    DWORD result(0);

    DEBUGMSG( ZONE_FUNCTION, (TEXT("+GPS_Init\r\n")));
    result = initHw(pContext);

    disableAsicPowerOn();
    DEBUGMSG( ZONE_FUNCTION, (TEXT("-GPS_Init\r\n")));
    return result;
}

GPS_API BOOL GPS_Deinit(DWORD context)
{
    BOOL result(FALSE);

    DEBUGMSG( ZONE_FUNCTION, (TEXT("+GPS_Deinit\r\n")));

    result = deinitHw(context);
    DEBUGMSG( ZONE_FUNCTION, (TEXT("-GPS_Deinit\r\n")));
    return result;
}

GPS_API BOOL GPS_IOControl(DWORD context, DWORD code,
                               PBYTE pBufIn, DWORD lenIn,
                               PBYTE pBufOut, DWORD lenOut,
                               PDWORD pActualOut)
{
    BOOL result(FALSE);
    DEBUGMSG(ZONE_IOCTL | ZONE_FUNCTION, (TEXT("+GPS_IOControl: code 0x%X\r\n"), code));
 
    switch (code)
    {
    case Gpsct::Hal::Generic::Ioctl::GPS_CHIPSET_CONTROL:
    {
        DEBUGMSG(ZONE_IOCTL, (TEXT("GPS_IOControl: GPS_CHIPSET_CONTROL\r\n")));

        Gpsct::Hal::Generic::Ioctl::GPS_GPIO_CONTROL* pGpioControlIn = 
                reinterpret_cast<Gpsct::Hal::Generic::Ioctl::GPS_GPIO_CONTROL*>(pBufIn);
        Gpsct::Hal::Generic::Ioctl::GPS_GPIO_CONTROL* pGpioControlOut = 
                reinterpret_cast<Gpsct::Hal::Generic::Ioctl::GPS_GPIO_CONTROL*>(pBufOut);


        if (pGpioControlIn && pGpioControlOut) // query availability
        {
            if (GLPIO_ASIC_RESET == pGpioControlIn->id || GLPIO_ASIC_STDBY == pGpioControlIn->id)
            {
                result = TRUE;
            }
            else
            {
                SetLastError(ERROR_SUCCESS);
                result = FALSE;
            }
        }
        else if (pGpioControlIn) // setting gpio
        {
            switch (pGpioControlIn->id)
            {
            case GLPIO_ASIC_RESET:
                pGpioControlIn->value ? enableAsicReset(): disableAsicReset();
                result = TRUE;
                break;
            case GLPIO_ASIC_STDBY:
                pGpioControlIn->value ? enableAsicPowerOn(): disableAsicPowerOn();
                result = TRUE;
                break;
            default:
                SetLastError(ERROR_INVALID_PARAMETER);
                result = FALSE;
                break;
            }
        }
        else if (pGpioControlOut) // getting gpio
        {
            switch (pGpioControlOut->id)
            {
            case GLPIO_ASIC_RESET:
                pGpioControlOut->value = 1;
                result = TRUE;
                break;
            case GLPIO_ASIC_STDBY:
                pGpioControlOut->value = 1;
                result = TRUE;
                break;
            default:
                SetLastError(ERROR_INVALID_PARAMETER);
                result = FALSE;
                break;
            }
            if(pActualOut)
            {
                *pActualOut = sizeof(Gpsct::Hal::Generic::Ioctl::GPS_GPIO_CONTROL);
            }
        }
    }
        break;        
#if !SUPPORT_POWER_YYY_ENTRY_POINTS
    case IOCTL_POWER_CAPABILITIES:
        // Tell the power manager about ourselves.
        if (pBufOut != NULL && lenOut >= sizeof(POWER_CAPABILITIES) && pActualOut != NULL) 
        {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try 
            {
                PPOWER_CAPABILITIES ppc = reinterpret_cast<PPOWER_CAPABILITIES>(pBufOut);
                ZeroMemory(ppc, sizeof(*ppc));
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D3) | DX_MASK(D4);
                *pActualOut = sizeof(*ppc);
                result = TRUE;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                ERRORMSG(TRUE, (_T("Exception in serial IOCTL_POWER_CAPABILITIES\r\n")));
            }
        }
        else
        {   SetLastError(ERROR_INVALID_PARAMETER);
        }
        break;        
        
    case IOCTL_POWER_SET: 
        if(pBufOut != NULL && lenOut == sizeof(CEDEVICE_POWER_STATE) && pActualOut != NULL) 
        {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try 
            {
                CEDEVICE_POWER_STATE dx = *reinterpret_cast<PCEDEVICE_POWER_STATE>(pBufOut);
                if(VALID_DX(dx)) 
                {
                    //BOOL bPMFlags = FALSE;
                    // Any request that is not D0 becomes a D4 request
                    if(dx != D0) 
                    {   dx = D4;
                    }
                    
                    *reinterpret_cast<PCEDEVICE_POWER_STATE>(pBufOut) = dx;
                    *pActualOut = sizeof(CEDEVICE_POWER_STATE);
                    if(dx != D0)
                    {
                        disableAsicPowerOn();
                    }
                    else
                    {
                        enableAsicPowerOn();
                    }  
                    g_dxCurrent = dx;
                    result = TRUE;
                }
                else
                {   SetLastError(ERROR_BAD_ARGUMENTS);
                }
            } 
            __except(EXCEPTION_EXECUTE_HANDLER) 
            {
                ERRORMSG(TRUE, (_T("Exception in serial IOCTL_POWER_SET\r\n")));
            }
        }
        else
        {   SetLastError(ERROR_INVALID_PARAMETER);
        }
        break;
        
    case IOCTL_POWER_GET: 
        if(pBufOut != NULL && lenOut == sizeof(CEDEVICE_POWER_STATE) && pActualOut != NULL)
        {
            // Just return our current Dx value
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try 
            {
                *reinterpret_cast<PCEDEVICE_POWER_STATE>(pBufOut) = g_dxCurrent;
                *pActualOut = sizeof(CEDEVICE_POWER_STATE);
                result = TRUE;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) 
            {
                ERRORMSG(TRUE, (_T("Exception in serial IOCTL_POWER_SET\r\n")));
            }
        }
        else
        {   SetLastError(ERROR_INVALID_PARAMETER);
        }
        break;
#endif // !SUPPORT_POWER_YYY_ENTRY_POINTS
    default:
        DEBUGMSG(ZONE_IOCTL | ZONE_WARN, (TEXT("GPS_IOControl: No matching IOCTL\r\n")));
        SetLastError(ERROR_BAD_ARGUMENTS);
        break;
    }

    DEBUGMSG(ZONE_IOCTL | ZONE_FUNCTION, (TEXT("-GPS_IOControl\r\n")));
    return result;
}

#if SUPPORT_POWER_YYY_ENTRY_POINTS

GPS_API void GPS_PowerDown(DWORD context)
{

}

GPS_API void GPS_PowerUp(DWORD context)
{

}

#endif // SUPPORT_POWER_YYY_ENTRY_POINTS

GPS_API DWORD GPS_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    DEBUGMSG(ZONE_OPEN| ZONE_FUNCTION, (TEXT("+GPS_Open\r\n")));
    DWORD result(0);
    // TODO: better analysis of shareMode
    if (!isOpen)
    {
        g_dxCurrent = D0;

        isOpen = true;
        result = 1; /// some context value to be used in future calls into driver
    }
    DEBUGMSG(ZONE_OPEN | ZONE_FUNCTION, (TEXT("-GPS_Open\r\n")));
    return result;
}

GPS_API BOOL GPS_Close(DWORD context)
{
    DEBUGMSG(ZONE_OPEN | ZONE_FUNCTION, (TEXT("+GPS_Close\r\n")));
    BOOL result(true);

    isOpen = false;

    disableAsicPowerOn();

    DEBUGMSG(ZONE_OPEN | ZONE_FUNCTION, (TEXT("-GPS_Close\r\n")));

    return result;
}

GPS_API DWORD GPS_Read(DWORD context, LPVOID pBuf, DWORD len)
{
    return 1;
}

GPS_API DWORD GPS_Seek(DWORD context, long pos, DWORD type)
{
    return 1;
}

GPS_API DWORD GPS_Write(DWORD context, LPCVOID pBufIn, DWORD lenIn)
{
    return 1;
}
