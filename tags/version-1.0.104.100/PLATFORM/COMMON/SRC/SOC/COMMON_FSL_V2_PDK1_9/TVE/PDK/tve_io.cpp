//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  File:  tve_io.cpp
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_tve.h"

#include "tve_sdk.h"
#include "tve.h"


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
#ifdef DEBUG
extern DBGPARAM dpCurSettings =
{
    _T("TVE"),
    {
        _T("Init"), _T("DeInit"), _T("Ioctl"), _T("Device"),
        _T(""), _T(""), _T(""), _T(""),
        _T(""),_T(""),_T(""),_T(""),
        _T("Info"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    0xffff//ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_FUNCTION | ZONEMASK_INFO | ZONE_INIT | ZONE_DEVICE
//    ZONEMASK_ERROR | ZONEMASK_WARN
};
#endif

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: TVE_Init
//
// The Device Manager calls this function as a result of a call to the
//      ActivateDevice() function.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//                active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD TVE_Init(LPCTSTR pContext)
{ 
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pContext);
    
    TVE_FUNCTION_ENTRY();
    
    TveClass* pTVE = new TveClass();
    
    // Managed to create the class?
    if (pTVE == NULL)
    {
        //DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("TVE_Init: TVE Class Failed!\r\n")));
        delete pTVE;
        return NULL;
    }

    TVE_FUNCTION_EXIT();
    
    return (DWORD) pTVE;
}


//-----------------------------------------------------------------------------
//
// Function: TVE_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success.
//
//-----------------------------------------------------------------------------
BOOL TVE_Deinit(DWORD hDeviceContext)
{

    TveClass * pTVE = (TveClass*) hDeviceContext;

    TVE_FUNCTION_ENTRY();

    if (pTVE != NULL)
    {
        delete pTVE;
    }

    TVE_FUNCTION_EXIT();

    return TRUE;
    
}


//-----------------------------------------------------------------------------
//
// Function: TVE_Open
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context. The XXX_Init function creates
//                and returns this handle.
//      AccessCode
//          [in] Access code for the device. The access is a combination of
//                read and write access from CreateFile.
//      ShareMode
//          [in] File share mode of the device. The share mode is a
//                combination of read and write access sharing from CreateFile.
//
// Returns:
//      This function returns a handle that identifies the open context of
//      the device to the calling application.
//
//-----------------------------------------------------------------------------
DWORD TVE_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    TVE_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);
    TVE_FUNCTION_EXIT();
    return hDeviceContext;
}


//-----------------------------------------------------------------------------
//
// Function: TVE_Close
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//                the open context of the device.
//
// Returns:  
//      TRUE indicates success.
//
//-----------------------------------------------------------------------------
BOOL TVE_Close(DWORD hOpenContext)
{
    TVE_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    TVE_FUNCTION_EXIT();
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: TVE_PowerDown
//
// This function suspends power to the device. It is useful only with 
//      devices that can power down under software control.
//
// Parameters:
//      hDeviceContext 
//          [in] Handle to the device context.
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
void TVE_PowerDown(DWORD hDeviceContext)
{
    TVE_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDeviceContext);
    TVE_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: TVE_PowerUp
//
// This function restores power to a device.
//
// Parameters:
//      None
//          
//
// Returns:  
//      None.
//
//-----------------------------------------------------------------------------
void TVE_PowerUp(void)
{
    TVE_FUNCTION_ENTRY();
    TVE_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: TVE_Read
//
// This function reads data from the device identified by the open 
//      context.
//
// Parameters:
//      hOpenContext 
//         [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      pBuffer 
//         [out] Pointer to the buffer that stores the data read from the 
//                 device. This buffer should be at least Count bytes long. 
//      Count 
//         [in] Number of bytes to read from the device into pBuffer. 
//
// Returns:  
//      Returns zero to indicate end-of-file. Returns -1 to indicate an 
//      error. Returns the number of bytes read to indicate success.
//
//-----------------------------------------------------------------------------
DWORD TVE_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    TVE_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Count);
    TVE_FUNCTION_EXIT();
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: TVE_Write
//
// This function writes data to the device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      pBuffer 
//         [out] Pointer to the buffer that contains the data to write. 
//      Count 
//          [in] Number of bytes to read from the device into pBuffer. 
//
// Returns:  
//      The number of bytes written indicates success. A value of -1 indicates 
//      failure.
//
//-----------------------------------------------------------------------------
DWORD TVE_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    TVE_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);
    TVE_FUNCTION_EXIT();
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: TVE_Seek
//
// This function moves the data pointer in the device.
//
// Parameters:
//      hOpenContext 
//         [in] Handle to the open context of the device. The XXX_Open 
//               function creates and returns this identifier.
//      Amount 
//         [in] Number of bytes to move the data pointer in the device. 
//               A positive value moves the data pointer toward the end of the 
//               file, and a negative value moves it toward the beginning.
//      Type 
//         [in] Starting point for the data pointer. 
//
// Returns:  
//      The new data pointer for the device indicates success. A value of -1 
//      indicates failure.
//
//-----------------------------------------------------------------------------
DWORD TVE_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    TVE_FUNCTION_ENTRY();
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);
    TVE_FUNCTION_EXIT();
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: TVE_IOControl
//
// This function sends a command to a device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      dwCode 
//          [in] I/O control operation to perform. These codes are 
//                device-specific and are usually exposed to developers through 
//                a header file.
//      pBufIn 
//          [in] Pointer to the buffer containing data to transfer to the 
//                device. 
//      dwLenIn 
//         [in] Number of bytes of data in the buffer specified for pBufIn.
//
//      pBufOut 
//          [out] Pointer to the buffer used to transfer the output data 
//                  from the device.
//      dwLenOut 
//          [in] Maximum number of bytes in the buffer specified by pBufOut.
//
//      pdwActualOut 
//          [out] Pointer to the DWORD buffer that this function uses to 
//                  return the actual number of bytes received from the device.
//
// Returns:  
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL TVE_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    
    DWORD *outputMode, *stdType, *resSizeType;
    BOOL bRet = TRUE;
  
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pdwActualOut);
    UNREFERENCED_PARAMETER(dwLenOut);
    
    TveClass* pTVE = (TveClass*) hOpenContext;
    
    switch (dwCode)
    {
     
       case TVE_IOCTL_INIT:
            pTVE->TveInit();
            break;
         
       case TVE_IOCTL_DEINIT:
            pTVE->TveDeinit();
            break;
       
       case TVE_IOCTL_ENABLE:
            pTVE->TveEnable();
            break;
       
       case TVE_IOCTL_DISABLE:
            pTVE->TveDisable();
            break;
                               
       case TVE_IOCTL_SET_CGMS_WSS_NTSC:
       {
           TVECgmsWssNTSC525Info *pCgmsWssNTSC = (TVECgmsWssNTSC525Info *)pBufIn;
                                    
           if ((dwLenIn != sizeof(TVECgmsWssNTSC525Info)) || (pBufIn == NULL))
           {
               bRet = FALSE; 
               break;
           }
                
           pTVE->TveSetCgmsWssFor525LineNTSC(pCgmsWssNTSC->tveCgmsWssF1Data, pCgmsWssNTSC->tveCgmsF2Data);
           break;
    
        }
           
        case   TVE_IOCTL_SET_CGMS_WSS_PAL:
        {
           TVECgmsWssPAL625Info *pCgmsWssPAL = (TVECgmsWssPAL625Info *)pBufIn;
                                    
           if ((dwLenIn != sizeof(TVECgmsWssPAL625Info)) || (pBufIn == NULL))
           {
              bRet = FALSE; 
              break;
           }
                
           pTVE->TveSetCgmsWssFor626LinePAL(pCgmsWssPAL->tveCgmsWssF1Data);
           break;
    
       }         
         
       case  TVE_IOCTL_SET_CC_FIELDS:
       {
           TVECCInfo *pCCData = (TVECCInfo *)pBufIn;
           if ((dwLenIn != sizeof(TVECCInfo)) || (pBufIn == NULL))
           {
               bRet = FALSE;
               break;
           }
           
           pTVE->TveSetClosedCaption(pCCData->cc_f1_odd_field_data, pCCData->cc_f2_even_field_data);
           break;
    
       } 
       
       
       case  TVE_IOCTL_SET_OUTPUT_MODE:
       {
           TVEOutputModeInfo *pOutputModeData = (TVEOutputModeInfo *)pBufIn;
           if ((dwLenIn != sizeof(TVEOutputModeInfo)) || (pBufIn == NULL))
           {
               bRet = FALSE;
               break;
           }
           // set TVE output mode
           pTVE->m_eTVOutputMode = (TVE_TV_OUT_MODE)pOutputModeData->iTVOutputMode;
           break;
    
       } 
       
       case  TVE_IOCTL_SET_OUTPUT_STD_TYPE:
       {
           TVEOutputStdInfo *pOutputStdData = (TVEOutputStdInfo *)pBufIn;
           if ((dwLenIn != sizeof(TVEOutputStdInfo)) || (pBufIn == NULL))
           {
               bRet = FALSE;
               break;
           }
           // set TVE output standard type
           pTVE->m_eTVStd = (TVE_TV_STAND)pOutputStdData->iTVOutputStd;
           break;
    
       } 
       
       case  TVE_IOCTL_SET_OUTPUT_RES_SIZE_TYPE:
       {
           TVEOutputResSizeInfo *pOutputResSizeData = (TVEOutputResSizeInfo *)pBufIn;
           if ((dwLenIn != sizeof(TVEOutputResSizeInfo)) || (pBufIn == NULL))
           {
               bRet = FALSE;
               break;
           }
           // set TVE resolution size
           pTVE->m_eTVResSize = (TVE_TV_RES_SIZE)pOutputResSizeData->iTVOutputResSize;
           break;
    
       } 
       
       case  TVE_IOCTL_GET_OUTPUT_MODE:
           outputMode = (DWORD *) pBufOut;
           *outputMode = (DWORD) pTVE->m_eTVOutputMode;
           bRet = TRUE;
           break;
       
       case  TVE_IOCTL_GET_OUTPUT_STD_TYPE:
           stdType = (DWORD *) pBufOut;
           *stdType = (DWORD) pTVE->m_eTVStd;
           bRet = TRUE;
           break;
           
       case  TVE_IOCTL_GET_OUTPUT_RES_SIZE_TYPE:
           resSizeType = (DWORD *) pBufOut;
           *resSizeType = (DWORD) pTVE->m_eTVResSize;
           bRet = TRUE;
           break;
       
       default:
           bRet = FALSE;
           DEBUGMSG(1, (TEXT("%s: Unsupported TVE_IOControl code!\r\n"), __WFUNCTION__));
           break;
                
    } //End of switch case
    
   
    return bRet;
} //End of TVE_IOCTL


//------------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the TVE module. This function is 
// called when processed and threads attach and detach from this module.
//
// Parameters:
//      hInstDll
//          [in] The handle to this module.
//      dwReason
//      [in] Specifies a flag indicating why the DLL entry-point function
//               is being called.
//      lpvReserved
//          [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the everything is OK; FALSE if an error occurred.
//
//------------------------------------------------------------------------------
extern "C"
BOOL WINAPI DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);
    switch (dwReason) 
    {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(1, (TEXT("DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;
         
        case DLL_PROCESS_DETACH:
            DEBUGMSG(1, (TEXT("DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // return TRUE for success
    return TRUE;
}
