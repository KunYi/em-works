//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//
//------------------------------------------------------------------------------
//
//  File:  rotary.c
//
//   This file implements the device specific functions for rotary encoder.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include <csp.h>
#include <rotaryhw.h>

#ifdef DEBUG
#define DEFAULT_ZONE        0
DBGPARAM dpCurSettings = {
    TEXT("ROTARY"), {
        TEXT("Init"), TEXT("Read"), TEXT("Write"), TEXT("Functions"),
        TEXT("-"), TEXT("-"), TEXT("-"), TEXT("-"),
        TEXT("-"), TEXT("-"), TEXT("-"),TEXT("-"),
        TEXT("-"), TEXT(""),  TEXT("Warnings"), TEXT("Errors")
    },
    DEFAULT_ZONE
};

#define ZONE_READ           DEBUGZONE(1)
#define ZONE_WRITE          DEBUGZONE(2)
#define ZONE_FUNCTION       DEBUGZONE(3)
#define ZONE_INIT           DEBUGZONE(4)
#define ZONE_ERROR          DEBUGZONE(5)

#endif

static VOID Rot_Handler(); //thread handler function

 UINT32 RotaryGetState();
//------------------------------------------------------------------------------
//Global Variables
//------------------------------------------------------------------------------
//WINCE600
// Current Serial device power state.
CEDEVICE_POWER_STATE CurDx;
static HANDLE hRotaryThreadHandle=NULL;
static HANDLE hRotaryEV=NULL;
CRITICAL_SECTION RotaryCritSec;
static DWORD dwRotTimeOut=INFINITE;
//rotary setup structure.
ROT_SETUP RotSetup;
DWORD dwSysIntr;


//------------------------------------------------------------------------------
// Function: ROT_Init
//
// Initializes the rotary device
//
// Parameters:
//       Identifier
//          [IN] Device identifier.
// Returns:
//    Returns Handle to the device
//
//------------------------------------------------------------------------------
HANDLE ROT_Init(ULONG Identifier)
{
    PROTARYINFO pROTObj = NULL;
    DWORD dwIrq;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Identifier);

    DEBUGMSG (ZONE_INIT,(TEXT("ROTARY_Init: \r\n")));
    

    hRotaryEV = CreateEvent(NULL, FALSE, FALSE, NULL);

    pROTObj = (PROTARYINFO)LocalAlloc(LPTR,sizeof(ROTARYINFO));
    if (!pROTObj)
    {
        DEBUGMSG (ZONE_INIT,(TEXT("ROTARY_Init- ***Error***  Unable to allocate ROTARYINFO buffer.\r\n")));
        return NULL;
    }
    memset(pROTObj, 0, sizeof(ROTARYINFO));

    // Init the rotary decoder setup structure
    RotSetup.Relative    = ROT_COUNTER_RELATIVE_ON;
    RotSetup.PolA        = ROT_INPUT_POLARITY_NO_CHANGE;
    RotSetup.PolB        = ROT_INPUT_POLARITY_NO_CHANGE;
    RotSetup.SelectA     = ROT_SELECT_ROTARYA;
    RotSetup.SelectB     = ROT_SELECT_ROTARYB;
    RotSetup.Divider     = 0;
    RotSetup.Oversample  = 2;

    //initializing the critical section
    InitializeCriticalSection(&RotaryCritSec);

    // Init rotary decoder
    if (!RotaryInit(&RotSetup))
    {
        ERRORMSG(1, (TEXT("RotaryInit failed \r\n")));
        goto CleanUp;
    }    

    dwIrq=IRQ_TIMER2;

    // Get kernel to translate IRQ -> System Interrupt ID
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(DWORD), &dwSysIntr, sizeof(DWORD), NULL))
    {
        DEBUGMSG (ZONE_INIT,(TEXT("ROTARY_Init- ***Error***  IOCTL_HAL_REQUEST_SYSINTR.\r\n")));
          goto CleanUp;
    }

    if (!InterruptInitialize(dwSysIntr, hRotaryEV, NULL, 0))
    {
         DEBUGMSG (ZONE_INIT,(TEXT("ROTARY_Init- ***Error*** InterruptInitialize.\r\n")));
          goto CleanUp;
    }

    // Create thread
    hRotaryThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) Rot_Handler, NULL, 0, NULL);
    if ( !hRotaryThreadHandle){
        ERRORMSG(1, (TEXT("ROTARY error in create thread \r\n")));
    }

    DEBUGMSG (ZONE_INIT,(TEXT("ROTARY_Init:--\r\n")));

CleanUp:
    return pROTObj;
}
//------------------------------------------------------------------------------
// Function: Rot_Handler
//
//Thread handler function for polling the rotary wheel rotation
//
//  Parameters:
//         None
//
//  Returns:
//         None
//-----------------------------------------------------------------------------
VOID Rot_Handler()
{
    static INT16 RotaryCount = 0;
    static INT16 Temp=0;

    for(;;)
    {
        WaitForSingleObject(hRotaryEV, dwRotTimeOut);

        EnterCriticalSection(&RotaryCritSec);
        //Read the rotary count
        RotaryCount = RotaryGetUpdownCount();

        LeaveCriticalSection(&RotaryCritSec);

        if ( RotaryCount != 0 )
        {
            Temp = -((INT16)(RotaryCount*WHEEL_DELTA));

           
            //mouse_event API is used to send the mouse events to the application layer
            //In this context the wheel event is send to the application.The application can be
            //any standard WINCE applications.
            
            mouse_event(MOUSEEVENTF_WHEEL,0,0,(DWORD)Temp,0);
        }
        RotarySetTimer();
        InterruptDone(dwSysIntr);
       
    }
}
//-----------------------------------------------------------------------------
//
// Function: ROT_Open
//
//   This Device open function
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//      AccessCode
//          [IN] Requested access (combination of GENERIC_READ and GENERIC_WRITE) (ignored)
//      ShareMode
//          [IN] Requested share mode (combination of FILE_SHARE_READ and FILE_SHARE_WRITE) (ignored)
//
//
// Returns:
//     Returns Handle to the device
//-----------------------------------------------------------------------------
HANDLE ROT_Open(HANDLE pPortObj,
                DWORD AccessCode,
                DWORD ShareMode)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+ROTARY_Open\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    return pPortObj;
}
//-----------------------------------------------------------------------------
//
// Function: ROT_Close
//
//    This function closes the device initialized by the ROT_Init function.
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL ROT_Close(HANDLE pPortObj)
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("ROTARY_Close:\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPortObj);

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: ROT_Deinit
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//
// Returns:
//      Returns Handle to the device
//
//-----------------------------------------------------------------------------
BOOL ROT_Deinit(HANDLE pPortObj)
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("ROTARY_Deinit:\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPortObj);
    RotaryDeinit();

    // delete the critical section
    DeleteCriticalSection(&RotaryCritSec);

    //Close the thread handle if exists
    if(hRotaryThreadHandle)
        CloseHandle(hRotaryThreadHandle);

      // Release SYSINTR
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntr, sizeof(DWORD),
                NULL, 0, NULL);

    return TRUE;
}
//-----------------------------------------------------------------------------
//
// Function: ROT_Read
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//      pTargetBuffer
//          [IN] Pointer to the target Value returned from WAV_Open call (ignored).
//      BufferLength
//          [IN] Maximum length to read (ignored)
//
// Returns:
//     Returns 0 always. ROT_Read should never get called and does nothing.
//-----------------------------------------------------------------------------
ULONG ROT_Read( HANDLE pPortObj,
                PUCHAR pTargetBuffer,
                ULONG BufferLength)
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("ROTARY_Read\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPortObj);
    UNREFERENCED_PARAMETER(pTargetBuffer);
    UNREFERENCED_PARAMETER(BufferLength);

    return 0;
}

//-----------------------------------------------------------------------------
// Function: ROT_Write
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//      pSourceBytes
//          [IN] Pointer to the buffer containing data (ignored)
//      NumberOfBytes
//          [OUT] Maximum length to write (ignored)
//
// Returns:
//      Returns 0 always. ROT_Write should never get called and does
//          nothing.
//-----------------------------------------------------------------------------
ULONG ROT_Write(HANDLE pPortObj,
                PUCHAR pSourceBytes,
                ULONG NumberOfBytes)
{
    DEBUGMSG (ZONE_FUNCTION,(TEXT("ROTARY_Write:\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPortObj);
    UNREFERENCED_PARAMETER(pSourceBytes);
    UNREFERENCED_PARAMETER(NumberOfBytes);

    return 0;
}
//-----------------------------------------------------------------------------
//
// Function: ROT_Seek
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//      Position
//          [IN] Position to seek to (relative to type) (ignored)
//      Type
//          [IN] FILE_BEGIN, FILE_CURRENT, or FILE_END (ignored)
//
// Returns:
//       Returns -1 always. ROT_Seek should never get called and does nothing.
//-----------------------------------------------------------------------------
ULONG ROT_Seek( HANDLE pPortObj,
                LONG Position,
                DWORD Type)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("ROTARY_Seek:\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPortObj);
    UNREFERENCED_PARAMETER(Position);
    UNREFERENCED_PARAMETER(Type);

    return (ULONG)-1;
}
//------------------------------------------------------------------------------
// Function: ROT_PowerOn
//
// This routine performs poweron sequence.
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL ROT_PowerOn(HANDLE pPortObj)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("ROTARY_PowerUp:\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPortObj);
 
    return TRUE;
}
//------------------------------------------------------------------------------
// Function: ROT_PowerOff
//
// This routine performs powerdown sequence.
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL ROT_PowerOff(HANDLE pPortObj)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("ROTARY_PowerDown:\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPortObj);
  
    return TRUE;
}

//------------------------------------------------------------------------------
// Function: ROT_IOControl
//
// This function sends a command to a device.wince600
//
// Parameters:
//      pPortObj
//          [IN] Handle to the open context of the device.
//      Ioctl
//          [IN] I/O control operation to perform.
//      pInBuf
//          [IN] Pointer to the buffer containing data to transfer to the device.
//      InBufLen
//          [IN] Number of bytes of data in the buffer specified for pBufIn.
//      pOutBuf
//          [out] Pointer to the buffer used to transfer the output data
//                from the device.
//      OutBufLen
//          [IN] Maximum number of bytes in the buffer specified by pBufOut.
//      pdwBytesTransferred
//          [IN] Pointer to the DWORD buffer that this function uses to return
//               the actual number of bytes received from the device.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//------------------------------------------------------------------------------
BOOL ROT_IOControl(HANDLE pPortObj,
                   DWORD Ioctl,
                   PUCHAR pInBuf,
                   DWORD InBufLen,
                   PUCHAR pOutBuf,
                   DWORD OutBufLen,
                   PDWORD pdwBytesTransferred)
{

    DWORD dwErr=ERROR_INVALID_PARAMETER;
    BOOL rc = TRUE;
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pInBuf);
    UNREFERENCED_PARAMETER(InBufLen);
    UNREFERENCED_PARAMETER(pPortObj);
    switch (Ioctl)
    {
    case IOCTL_POWER_CAPABILITIES:

        // tell the power manager about ourselves.
        DEBUGMSG(ZONE_FUNCTION, (TEXT("ROT_IOControl_POWER_CAPABILITIES\r\n")));
        if ( pOutBuf != NULL && OutBufLen >= sizeof(POWER_CAPABILITIES) &&
             pdwBytesTransferred != NULL){

            PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pOutBuf;
            memset(ppc, 0, sizeof(POWER_CAPABILITIES));
            ppc->DeviceDx = 0x11;   // support D0, D4
            ppc->WakeFromDx = 0x00; // No wake capability
            ppc->InrushDx = 0x00;       // No in rush requirement
            ppc->Power[D0] = 600;                   // 0.6W
            ppc->Power[D1] = (DWORD) PwrDeviceUnspecified;
            ppc->Power[D2] = 0;
            ppc->Power[D3] = (DWORD) PwrDeviceUnspecified;
            ppc->Power[D4] = (DWORD) PwrDeviceUnspecified;
            ppc->Latency[D0] = 0;
            ppc->Latency[D1] = (DWORD) PwrDeviceUnspecified;
            ppc->Latency[D2] = 0;
            ppc->Latency[D3] = (DWORD) PwrDeviceUnspecified;
            ppc->Latency[D4] = (DWORD) PwrDeviceUnspecified;
            ppc->Flags = 0;
            *pdwBytesTransferred = sizeof(POWER_CAPABILITIES);
            dwErr = ERROR_SUCCESS;
        }
        break;

    case IOCTL_POWER_QUERY:

        DEBUGMSG(ZONE_FUNCTION, (TEXT("ROT_IOControl_POWER_QUERY\r\n")));
        if ( pOutBuf != NULL &&
             OutBufLen == sizeof(CEDEVICE_POWER_STATE) &&
             pdwBytesTransferred != NULL)
        {
            CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("NewDx = %d\r\n"), NewDx));

            if (VALID_DX(NewDx))
            {
                // this is a valid Dx state so return a good status
                *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            DEBUGMSG(ZONE_FUNCTION, (TEXT("ROT_IOControl_POWER_QUERY %u %s\r\n"),
                                     NewDx, dwErr == ERROR_SUCCESS ?
                                     TEXT("succeeded") : TEXT("failed")));
        }
        break;

    case IOCTL_POWER_SET:
        DEBUGMSG(ZONE_FUNCTION, (TEXT("ROT_IOControl_POWER_SET\r\n")));
        if ( pOutBuf != NULL &&
             OutBufLen == sizeof(CEDEVICE_POWER_STATE) &&
             pdwBytesTransferred != NULL)
        {
            CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
            DEBUGMSG(ZONE_FUNCTION, (TEXT("NewDx = %d\r\n"), NewDx));
            if (NewDx != CurDx)
            {
                if (NewDx == D4)
                {                 
                    //Do not Turn off the clock
                    //  RotarySetClkGate(TRUE);
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("Rotary OFF\r\n")));
                }
                else
                {
                    // if asked for a state we don't support, Set the state
                    //to default power up state which is D0
                    NewDx = D0;              
                    ////Turn on the clock 
                    // RotarySetClkGate(FALSE);
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("Rotary Off\r\n")));
                }
                CurDx = NewDx;
                *(PCEDEVICE_POWER_STATE)pOutBuf = CurDx;
            }
            *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("CurDx = %d\r\n"), CurDx));
            dwErr = ERROR_SUCCESS;
        }
        break;

    case IOCTL_POWER_GET:
        DEBUGMSG(ZONE_FUNCTION, (TEXT("ROT_IOControl_POWER_GET\r\n")));
        if ( pOutBuf != NULL &&
             OutBufLen == sizeof(CEDEVICE_POWER_STATE) &&
             pdwBytesTransferred != NULL)
        {
            // just return our CurrentDx value
            *(PCEDEVICE_POWER_STATE)pOutBuf = CurDx;
            *pdwBytesTransferred = sizeof(CEDEVICE_POWER_STATE);
            dwErr = ERROR_SUCCESS;
        }
        break;
    default:
    {
        DEBUGMSG (ZONE_ERROR,(TEXT("ROTARY_IOControl - unknown\r\n")));
        RETAILMSG (1,(TEXT("ROTARY_IOControl - unknown\r\n")));
        rc = FALSE;
        break;
    }

    }     // endswitch (Ioctl)*/

    return rc;

}

//-----------------------------------------------------------------------------
//
//  Function: DllEntry
//
//  This function called when the DLL is loaded.
//
//  Parameters:
//       hinstDll
//        [IN]  Handle to the DLL.
//       dwReason
//        [IN]   Specifies a flag indicating why the DLL entry-point function is being called.
//       lpReserved
//        [IN]  Specifies further aspects of DLL initialization and cleanup.
//
//  Returns:
//      TRUE for success, and FALSE for failure.
//
//-----------------------------------------------------------------------------
BOOL WINAPI DllEntry(HANDLE hinstDll, DWORD dwReason, LPVOID lpReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpReserved);
    switch(dwReason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER((HMODULE)hinstDll);
        DEBUGMSG(ZONE_FUNCTION, (TEXT("ROTARY-process attach\r\n")));

        // don't need thread attach/detach messages
        DisableThreadLibraryCalls ((HMODULE)hinstDll);
        break;

    case DLL_PROCESS_DETACH:
        DEBUGMSG(ZONE_FUNCTION, (TEXT("ROTARY-process detach\r\n")));
        break;
    }
    return TRUE;
}
