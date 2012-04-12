//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
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
//  File:  i2c_io.cpp
//
//  This module provides a stream interface for the I2C bus
//  driver.  Client drivers can use the stream interface to
//  configure and exchange data with the I2C peripheral.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <windev.h>
#include <ceddk.h>
#pragma warning(pop)

#include "i2cbus.h"
#include "i2cclass.h"

#pragma warning(push)
#pragma warning(disable: 4512)
#include "marshal.hpp" //helper classes to marshal/alloc embedded/async buffer
#pragma warning(pop)

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

#define REG_DEVINDEX_VAL_NAME           L"Index"
#define I2C_DEVINDEX_MAX_VAL            2
#define I2C_DEVINDEX_MIN_VAL            1
#define I2C_DEVINDEX_DEFAULT_VAL        1



#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("i2c"), {
        TEXT("Init"),TEXT("Deinit"),TEXT("Open"),TEXT("Close"),
        TEXT("IOCtl"),TEXT("Thread"),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
    (ZONEMASK_WARN | ZONEMASK_ERROR)
};
#endif // DEBUG


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: I2C_Init
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
DWORD I2C_Init(LPCTSTR pContext)
{    
    UINT32 error;
    HKEY  hKey;
    DWORD  dwIndex, dwSize;
    
    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("I2C_Init +\r\n")));
    
    // try to open active device registry key for this context
    hKey = OpenDeviceKey(pContext);
    if (hKey == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("I2C_Init:  OpenDeviceKey failed!!!\r\n")));
        return 0;
    }

    // try to load I2C index from registry data
    dwSize = sizeof(DWORD);
    error = RegQueryValueEx(
        hKey,                       // handle to currently open key
        REG_DEVINDEX_VAL_NAME,      // string containing value to query
        NULL,                       // reserved, set to NULL
        NULL,                       // type not required, set to NULL
        (LPBYTE)(&dwIndex),      // pointer to buffer receiving value
        &dwSize);               // pointer to buffer size

    // close handle to open key
    RegCloseKey(hKey);

    // check for errors during RegQueryValueEx
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("I2C_Init:  RegQueryValueEx failed!!!\r\n")));
        return 0;
    }
    
    // Construct the I2C Module Class
    I2CClass* pI2C = new I2CClass(dwIndex);

    // Managed to create the class?
    if (pI2C == NULL)
    {
        return NULL;
    }

    // If class construction not successful?
    if (pI2C->IsLastActionOK() != TRUE)
    {
        // Dispose the instance
        DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("I2C_Init: I2C Class Failed! Err=%d\r\n"), pI2C->GetLastResult()));
        delete pI2C;
        return NULL;
    }

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("I2C_Init - hDev=0x%x\r\n"), pI2C));

    // Otherwise return the created instance
    return (DWORD) pI2C;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Deinit
//
// This function uninitializes a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL I2C_Deinit(DWORD hDeviceContext)
{
    I2CClass * pI2C = (I2CClass*) hDeviceContext;

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("I2C_Deinit +DeviceContext=0x%x\r\n"),hDeviceContext));

    if (pI2C != NULL)
    {
        delete pI2C;
    }

    DEBUGMSG (ZONE_DEINIT|ZONE_FUNCTION, (TEXT("I2C_Deinit -\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Open
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
DWORD I2C_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("I2C_Open +hDeviceContext=0x%x\r\n"),hDeviceContext));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(AccessCode);
    UNREFERENCED_PARAMETER(ShareMode);

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("I2C_Open -\r\n")));

    // Open is meaningless!
    return hDeviceContext;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Close
//
// This function opens a device for reading, writing, or both.
//
// Parameters:
//      hOpenContext
//          [in] Handle returned by the XXX_Open function, used to identify
//                the open context of the device.
//
// Returns:  
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL I2C_Close(DWORD hOpenContext)
{
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("I2C_Close +\r\n")));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("I2C_Close -\r\n")));

    // Close is meaningless!
    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Read
//
// This function reads data from the device identified by the open 
//      context.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
//      pBuffer 
//         [out] Pointer to the buffer that stores the data read from the 
//                 device. This buffer should be at least Count bytes long. 
//      Count 
//          [in] Number of bytes to read from the device into pBuffer. 
//
// Returns:  
//      Returns zero to indicate end-of-file. Returns -1 to indicate an 
//      error. Returns the number of bytes read to indicate success.
//
//-----------------------------------------------------------------------------
DWORD I2C_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(Count);

    // Nothing to read
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Write
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
DWORD I2C_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(dwNumBytes);

    // Nothing to write
    return 0;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_Seek
//
// This function moves the data pointer in the device.
//
// Parameters:
//      hOpenContext 
//          [in] Handle to the open context of the device. The XXX_Open 
//                function creates and returns this identifier.
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
DWORD I2C_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hOpenContext);
    UNREFERENCED_PARAMETER(Amount);
    UNREFERENCED_PARAMETER(Type);

    // Seeking is meaningless!
    return (DWORD)-1;
}


//-----------------------------------------------------------------------------
//
// Function: I2C_IOControl
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
//      The new data pointer for the device indicates success. A value of -1 
//      indicates failure.
//
//-----------------------------------------------------------------------------
BOOL I2C_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, 
                   DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                   PDWORD pdwActualOut)
{
    BOOL bRet = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pdwActualOut);

    // hOpenContext is a pointer to I2CClass instance!
    I2CClass* pI2C = (I2CClass*) hOpenContext;

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl: +hOpenContext=0x%x\r\n"),hOpenContext));

    if (pI2C != NULL)
    {
        switch (dwCode)
        {
            case I2C_IOCTL_SET_SLAVE_MODE:
            {
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_SLAVE_MODE +\r\n")));
                //slave mode is not implemented,
                bRet = FALSE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_SLAVE_MODE -\r\n")));
                break;
            }
            case I2C_IOCTL_SET_MASTER_MODE:
            {
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_MASTER_MODE +\r\n")));
                pI2C->SetMode(I2C_MASTER_MODE);
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_MASTER_MODE +\r\n")));
                break;
            }
            case I2C_IOCTL_IS_MASTER:
            {
                if (dwLenOut != sizeof(BOOL))
                    return FALSE;

                if( (NULL == pBufOut) )
                {
                    return FALSE;
                }

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_MASTER +\r\n")));
                PBOOL pbIsMaster = (PBOOL) pBufOut;
                if (pI2C->GetMode() == I2C_MASTER_MODE)
                    *pbIsMaster = TRUE;
                else
                    *pbIsMaster = FALSE;
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_MASTER - Val=0x%x\r\n"), *pbIsMaster));
                break;
            }
            case I2C_IOCTL_IS_SLAVE:
            {
                if (dwLenOut != sizeof(BOOL))
                    return FALSE;

                if( (NULL == pBufOut) )
                {
                    return FALSE;
                }

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_SLAVE +\r\n")));
                PBOOL pbIsSlave = (PBOOL) pBufOut;
                if (pI2C->GetMode() == I2C_SLAVE_MODE)
                    *pbIsSlave = TRUE;
                else
                    *pbIsSlave = FALSE;
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:IS_SLAVE - Val=0x%x\r\n"), *pbIsSlave));
                break;
            }
            case I2C_IOCTL_GET_CLOCK_RATE:
            {
                if (dwLenOut != sizeof(WORD))
                    return FALSE;

                if( (NULL == pBufOut) )
                {
                    return FALSE;
                }
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_CLOCK_RATE +\r\n")));
                PDWORD pdwClkRate = (PDWORD) pBufOut;
                *pdwClkRate = pI2C->GetClockRate();
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_CLOCK_RATE - Val=0x%x\r\n"), *pdwClkRate));
                break;
            }
            case I2C_IOCTL_SET_CLOCK_RATE:
            {
                if (dwLenIn != sizeof(WORD))
                    return FALSE;

                if( (NULL == pBufIn) )
                {
                    return FALSE;
                }

                PDWORD pdwClkRate = (PDWORD) pBufIn;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_CLOCK_RATE + ValIn=0x%x\r\n"), *pdwClkRate));
                if (*pdwClkRate > I2C_MAX_FREQUENCY)
                {
                    RETAILMSG (1, (TEXT("I2C_IOControl: I2C frequency may not exceed %d...\r\n"), I2C_MAX_FREQUENCY));
                    return FALSE;
                }

                
                pI2C->SetClockRate(*pdwClkRate);
                bRet = TRUE;
                break;
            }
            case I2C_IOCTL_SET_SELF_ADDR:
            {
                if (dwLenIn != sizeof(BYTE))
                    return FALSE;

                if( (NULL == pBufIn) )
                {
                    return FALSE;
                }

                PBYTE pbySelfAddr = (PBYTE) pBufIn;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:SET_SELF_ADDR + ValIn=0x%x\r\n"), *pbySelfAddr));
                pI2C->SetSelfAddress(*pbySelfAddr);
                bRet = TRUE;
                break;
            }
            case I2C_IOCTL_GET_SELF_ADDR:
            {
                if (dwLenOut != sizeof(BYTE))
                    return FALSE;

                if( (dwLenOut > 0) && (NULL == pBufOut) )
                {
                    return FALSE;
                }

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_SELF_ADDR +\r\n")));
                PBYTE pbySelfAddr = (PBYTE) pBufOut;
                *pbySelfAddr = pI2C->GetSelfAddress();
                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:GET_SELF_ADDR - Val=0x%x\r\n"), *pbySelfAddr));
                break;
            }
            case I2C_IOCTL_TRANSFER:
            {
                DuplicatedBuffer_t Marshalled_pInBuf(pBufIn, dwLenIn, ARG_I_PTR);
                pBufIn = reinterpret_cast<PBYTE>( Marshalled_pInBuf.ptr() );
                if( (dwLenIn > 0) && (NULL == pBufIn) )
                {
                    return FALSE;
                }

                I2C_TRANSFER_BLOCK *pXferBlock = (I2C_TRANSFER_BLOCK *) pBufIn;
                if (pXferBlock->iNumPackets<=0 || pXferBlock->iNumPackets > NUM_BUFFERS_IN_CHAIN) 
                {
                    ERRORMSG(1, (L"NumPackets out of range: 1 <= Number of packets <= %d", NUM_BUFFERS_IN_CHAIN));
                    return FALSE;
                }

                MarshalledBuffer_t Marshalled_pPackets(pXferBlock->pI2CPackets, 
                                                       pXferBlock->iNumPackets*sizeof(I2C_PACKET), 
                                                       ARG_I_PTR);

                I2C_PACKET *pPackets = reinterpret_cast<I2C_PACKET *>(Marshalled_pPackets.ptr());
                if( (NULL == pPackets) )
                {
                    return FALSE;
                }

               struct Marshalled_I2C_PACKET
                {
                    MarshalledBuffer_t *pbyBuf;
                    MarshalledBuffer_t *lpiResult;
                } *Marshalled_of_pPackets;

                Marshalled_of_pPackets = new Marshalled_I2C_PACKET[pXferBlock->iNumPackets];
                if (Marshalled_of_pPackets==0)
                {
                    return FALSE;
                }

                MarshalledBuffer_t *pMarshalled_ptr;
                int i;

               // Map pointers for each packet in the array
                for (i = 0; i < pXferBlock->iNumPackets; i++)
                {
                    switch( pPackets[i].byRW)
                    {
                    case I2C_RW_WRITE:
                        pMarshalled_ptr = new MarshalledBuffer_t(
                                               pPackets[i].pbyBuf,
                                               pPackets[i].wLen,
                                               ARG_I_PTR,
                                               FALSE, FALSE);
                        if (pMarshalled_ptr ==0)
                        {
                            bRet = FALSE;
                            goto cleanupPass1;
                        }
                        if (pMarshalled_ptr->ptr()==0)
                        {
                            bRet = FALSE;
                            delete pMarshalled_ptr;
                            goto cleanupPass1;
                        }
                        break;

                    case I2C_RW_READ:
                        pMarshalled_ptr = new MarshalledBuffer_t(
                                               pPackets[i].pbyBuf,
                                               pPackets[i].wLen,
                                               ARG_O_PTR, FALSE, FALSE);
                        if (pMarshalled_ptr ==0)
                        {
                            bRet = FALSE;
                            goto cleanupPass1;
                        }
                        if (pMarshalled_ptr->ptr()==0)
                        {
                            bRet = FALSE;
                            delete pMarshalled_ptr;
                            goto cleanupPass1;
                        }
                        break;

                    default:
                        {
                            bRet = FALSE;
                            goto cleanupPass1;
                        }
                    }

                    pPackets[i].pbyBuf = reinterpret_cast<PBYTE>(pMarshalled_ptr->ptr());
                    Marshalled_of_pPackets[i].pbyBuf = pMarshalled_ptr;
                }

                for (i = 0; i < pXferBlock->iNumPackets; i++)
                {
                    pMarshalled_ptr = new MarshalledBuffer_t(
                                     pPackets[i].lpiResult, sizeof(INT), 
                                     ARG_O_PDW, FALSE, FALSE);

                    if (pMarshalled_ptr ==0)
                    {
                        bRet = FALSE;
                        goto cleanupPass2;
                    }
                    if (pMarshalled_ptr->ptr()==0)
                    {
                        bRet = FALSE;
                        delete pMarshalled_ptr;
                        goto cleanupPass2;
                    }
                    pPackets[i].lpiResult = reinterpret_cast<LPINT>(pMarshalled_ptr->ptr());
                    Marshalled_of_pPackets[i].lpiResult = pMarshalled_ptr;
                }

                bRet = pI2C->ProcessPackets(pPackets, pXferBlock->iNumPackets);

                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:I2C_IOCTL_TRANSFER -\r\n")));

                i = pXferBlock->iNumPackets;
cleanupPass2:
                for (--i; i>=0; --i)
                {
                    delete Marshalled_of_pPackets[i].lpiResult;
                }
                                
                i = pXferBlock->iNumPackets;
cleanupPass1:
                for (--i; i>=0; --i)
                {
                    delete Marshalled_of_pPackets[i].pbyBuf;
                }

                delete[] Marshalled_of_pPackets;

                break;
            }


            case I2C_IOCTL_RESET:
            {
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:RESET +\r\n")));

                pI2C->Reset();

                bRet = TRUE;
                DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl:RESET -\r\n")));
                break;
            }
            case I2C_IOCTL_ENABLE_SLAVE:
            {
                bRet = pI2C->EnableSlave();
                break;
            }
            case I2C_IOCTL_DISABLE_SLAVE:
            {
                bRet = pI2C->DisableSlave();
                break;
            }
            case IOCTL_POWER_CAPABILITIES:
            {
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
                        bRet = TRUE;
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        ERRORMSG(TRUE, (_T("Exception in CSPI IOCTL_POWER_CAPABILITIES\r\n")));
                    }
                }

                break;
            }
            case IOCTL_POWER_SET: 
            {
                if(pBufOut != NULL 
                    && dwLenOut == sizeof(CEDEVICE_POWER_STATE) 
                    && pdwActualOut != NULL) 
                {
                    PREFAST_SUPPRESS(6320, "Generic exception handler");
                    __try 
                    {
                        CEDEVICE_POWER_STATE dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                        if (VALID_DX(dx)) 
                        {
                            // Any request that is not D0 becomes a D4 request
                            if (dx != D0) 
                            {
                                dx = D4;
                            }

                            *(PCEDEVICE_POWER_STATE) pBufOut = dx;
                            *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                            // wait until any pending transaction is complete
                            if (dx == D4)
                            {
                                pI2C->BusyWait();
                            }
                            // reset the block and DMA channel before starting any new transfers
                            else if (dx == D0)
                            {
                                pI2C->Reset();                           
                            }
                            
                            // change the current power state                            
                            pI2C->m_dxCurrent = dx;

                            // Leave 
                            bRet = TRUE;
                        }
                    } 
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        ERRORMSG(TRUE, (_T("Exception in I2C IOCTL_POWER_CAPABILITIES\r\n")));
                    }
                }

                break;
            }
            case IOCTL_POWER_GET: 
            {
                if(pBufOut != NULL 
                    && dwLenOut == sizeof(CEDEVICE_POWER_STATE) 
                    && pdwActualOut != NULL) 
                {
                    // Just return our current Dx value
                    PREFAST_SUPPRESS(6320, "Generic exception handler");
                    __try 
                    {
                        *(PCEDEVICE_POWER_STATE) pBufOut = pI2C->m_dxCurrent; //getCurrentPowerState();
                        *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                        bRet = TRUE;
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        ERRORMSG(TRUE, (_T("Exception in CSPI IOCTL_POWER_CAPABILITIES\r\n")));
                    }
                }
                break;
            }
            default:
            {
                bRet = FALSE;
                break;
            }
        }
                
    }

    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION, (TEXT("I2C_IOControl -\r\n")));

    return bRet;
}

BOOL WINAPI I2C_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HINSTANCE) hInstDll);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2C_DllEntry: DLL_PROCESS_ATTACH lpvReserved(0x%x)\r\n"),lpvReserved));
            DisableThreadLibraryCalls((HMODULE) hInstDll);
            break;
         
        case DLL_PROCESS_DETACH:
            DEBUGMSG(ZONE_FUNCTION, (TEXT("I2C_DllEntry: DLL_PROCESS_DETACH lpvReserved(0x%x)\r\n"),lpvReserved));
            break;
    }
    // return TRUE for success
    return TRUE;
}
