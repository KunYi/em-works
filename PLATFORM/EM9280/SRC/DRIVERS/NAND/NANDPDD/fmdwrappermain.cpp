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
#include "FmdWrapperMain.h"
#include "FmdWrapperPdd.h"
#pragma warning(disable: 4100 6320)
DBGPARAM dpCurSettings = {
    TEXT("FmdWrapperPdd"), {
        TEXT("Init"),
        TEXT("Error"),
        TEXT("Write"),
        TEXT("Read"),
        TEXT("Function"),
        TEXT("5"),
        TEXT("6"),
        TEXT("7"),
        TEXT("8"),
        TEXT("9"),
        TEXT("10"),
        TEXT("11"),
        TEXT("12"),
        TEXT("13"),
        TEXT("14"),
        TEXT("15")},
    0x2          // Errors are only on by default
}; 

#define ZONE_ERROR      DEBUGZONE(1)

/*-----------------------------------------------------------------*/

STDAPI_(BOOL) WINAPI
DllEntry(HINSTANCE DllInstance, DWORD Reason, LPVOID Reserved)
{
    switch(Reason) 
    {
    case DLL_PROCESS_ATTACH:
        RETAILREGISTERZONES (DllInstance);  
        DEBUGMSG(ZONE_INIT,(TEXT("Flash PDD: DLL_PROCESS_ATTACH \r\n")));
        break;

    case DLL_PROCESS_DETACH:
        DEBUGMSG(ZONE_INIT,(TEXT("Flash PDD: DLL_PROCESS_DETACH \r\n")));
        break;
    }
    return TRUE;
} 



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_Init()

Description:    Initializes the flash wrapper.

Returns:        Context data for the flash device.
-------------------------------------------------------------------*/
DWORD FlashPdd_Init(DWORD Context) 
{
    LRESULT Result = ERROR_SUCCESS;
    FmdWrapperPdd* pFlashWrapper = NULL;
	// CS&ZHL MAY-14-2011: read registry item RegionNumber
	DWORD	dwType;
	DWORD	dwDataSize;
	HKEY	hkDevice = NULL;
    
    DEBUGMSG(ZONE_FUNCTION,(TEXT("Flash PDD: Init()\r\n")));

    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try 
    {
        pFlashWrapper = new FmdWrapperPdd();
        if (!pFlashWrapper)
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

		//
		// CS&ZHL MAY-14-2011: read RegionNumber from registry
		//
		hkDevice = OpenDeviceKey((LPCTSTR)Context);
		if(!hkDevice)
		{
			Result = ERROR_INVALID_PARAMETER;
            goto exit;
		}

		dwDataSize = sizeof(DWORD);
		RegQueryValueEx(hkDevice, REG_NAND_DRIVER_REGION_NUMBER, NULL, &dwType, (LPBYTE)(&(pFlashWrapper->m_dwRegionNumber)), &dwDataSize);
		if((pFlashWrapper->m_dwRegionNumber != 1) && (pFlashWrapper->m_dwRegionNumber != 2))
		{
			pFlashWrapper->m_dwRegionNumber = 0;		// 
		}
		RETAILMSG(1, (TEXT("FlashPdd_Init: m_dwRegionNumber = %d.\r\n"), pFlashWrapper->m_dwRegionNumber));

		Result = pFlashWrapper->Init(Context);
        if (Result != ERROR_SUCCESS)
        {
            goto exit;
        }
    }        
    __except (EXCEPTION_EXECUTE_HANDLER)  
    {      
        Result = ERROR_INVALID_PARAMETER;
    }

exit:
	if(hkDevice)			// CS&ZHL MAY-14-2011: close registry handle which used to read RegionNumber
	{
		CloseHandle(hkDevice);
	}

	if (Result != ERROR_SUCCESS)
    {
        if (pFlashWrapper)
        {
            pFlashWrapper->Deinit();
            delete pFlashWrapper;
            pFlashWrapper = NULL;
        }
        SetLastError(Result);
    }

    return (DWORD)pFlashWrapper;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_Deinit()

Description:    Unloads the console device

Returns:        Boolean indicating success; DEVICE.EXE does not
                check the return code.
-------------------------------------------------------------------*/
BOOL FlashPdd_Deinit(DWORD Context)
{
    LRESULT Result = ERROR_SUCCESS;
    
    DEBUGMSG(ZONE_FUNCTION,(TEXT("Flash PDD: Deinit()\r\n")));

    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try 
    {
        FmdWrapperPdd* pFlashWrapper = (FmdWrapperPdd*)Context;

        if (pFlashWrapper)
        {
            Result = pFlashWrapper->Deinit();        
            delete pFlashWrapper;
        }
    }        
    __except (EXCEPTION_EXECUTE_HANDLER)  
    {      
        Result = ERROR_INVALID_PARAMETER;
    }

    return (Result == ERROR_SUCCESS);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_Open()

Description:    Clears the console screen and restores defaults

Returns:        Context data for the console device 
-------------------------------------------------------------------*/
DWORD
FlashPdd_Open(
    DWORD Data,       // pdev
    DWORD Access,
    DWORD ShareMode)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("Flash PDD: Open()\r\n")));

    return Data;  
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_Close()

Description:    Closes the console

Returns:        Boolean indicating success
-------------------------------------------------------------------*/
BOOL FlashPdd_Close(DWORD Handle)
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("Flash PDD: Close()\r\n")));

    return TRUE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_Read()

Description:    Not Used.  The file system performs read commands by
                issuing the appopriate IOCTL.  See FlashPdd_IoControl()
                for details.

Returns:        FALSE
-------------------------------------------------------------------*/
DWORD 
FlashPdd_Read(
    DWORD Handle, 
    LPVOID pBuffer,                                             // Output buffer
    DWORD NumBytes)                                             // Buffer size in bytes
{
    DEBUGMSG(ZONE_FUNCTION,(TEXT("Flash PDD: Read()\r\n")));
    return FALSE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_Write()

Description:    Not Used.  The file system performs write commands by
                issuing the appopriate IOCTL.  See FlashPdd_IoControl()
                for details.
                
Returns:        FALSE
-------------------------------------------------------------------*/
DWORD FlashPdd_Write(
    DWORD Handle, 
    LPCVOID pBuffer, 
    DWORD InBytes)
{ 
    DEBUGMSG(ZONE_FUNCTION,(TEXT("Flash PDD: Write()\r\n")));
    return FALSE;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_Seek()

Description:    Seeks are not supported.

Returns:        FALSE
-------------------------------------------------------------------*/
DWORD 
FlashPdd_Seek(
    DWORD Handle, 
    long Distance, 
    DWORD MoveMethod
    )
{ 
    DEBUGMSG(ZONE_FUNCTION,(TEXT("Flash PDD: Seek()\r\n")));
    return FALSE;
}


BOOL
FlashPdd_IoControl(
    DWORD Handle,
    DWORD IoControlCode,
    PBYTE pInBuf,
    DWORD InBufSize,
    PBYTE pOutBuf,
    DWORD OutBufSize,
    PDWORD pBytesReturned)
{
    LRESULT Result = ERROR_SUCCESS;
    FlashPddInterface* pFlashPdd = (FlashPddInterface*)Handle;

    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try 
    {

    switch (IoControlCode) 
    {
        case IOCTL_FLASH_PDD_GET_REGION_COUNT:
        {
            if (pOutBuf == NULL || OutBufSize < sizeof(DWORD)) 
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            Result = pFlashPdd->GetRegionCount((LPDWORD)pOutBuf);
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - GetRegionCount() failed.\r\n")));        
            }   
        }
        break;

        case IOCTL_FLASH_PDD_GET_REGION_INFO:
        {
            if (pOutBuf == NULL || OutBufSize < sizeof(FLASH_REGION_INFO)) 
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            DWORD RegionCount = OutBufSize / sizeof(FLASH_REGION_INFO);
            
            Result = pFlashPdd->GetRegionInfoTable(RegionCount, (FLASH_REGION_INFO*)pOutBuf);
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - GetRegionInfoTable() failed.\r\n")));        
            }   
        }
        break;

        case IOCTL_FLASH_PDD_GET_BLOCK_STATUS:
        {           
            if (pInBuf == NULL || InBufSize < sizeof(FLASH_GET_BLOCK_STATUS_REQUEST) || 
                pOutBuf == NULL || OutBufSize < sizeof(DWORD))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            FLASH_GET_BLOCK_STATUS_REQUEST Request = *(FLASH_GET_BLOCK_STATUS_REQUEST*)pInBuf;

            if (OutBufSize < (Request.BlockRun.BlockCount * sizeof(DWORD)))
            {
                Result = ERROR_INSUFFICIENT_BUFFER;
                goto exit;
            }                

            Result = pFlashPdd->GetBlockStatus (Request.BlockRun, Request.IsInitialFlash, (DWORD*)pOutBuf);                
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - GetBlockStatus() failed.\r\n")));
            }
        }
        break;

        case IOCTL_FLASH_PDD_SET_BLOCK_STATUS:
        {           
            if (pInBuf == NULL || InBufSize < sizeof(FLASH_SET_BLOCK_STATUS_REQUEST))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            FLASH_SET_BLOCK_STATUS_REQUEST Request = *(FLASH_SET_BLOCK_STATUS_REQUEST*)pInBuf;

            Result = pFlashPdd->SetBlockStatus (Request.BlockRun, Request.BlockStatus);                
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - SetBlockStatus() failed.\r\n")));
            }
        }
        break;

        case IOCTL_FLASH_PDD_READ_PHYSICAL_SECTORS:
        {          
            if (pInBuf == NULL || InBufSize < sizeof(FLASH_PDD_TRANSFER))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            Result = pFlashPdd->ReadPhysicalSectors (InBufSize / sizeof(FLASH_PDD_TRANSFER),
                                                    (FLASH_PDD_TRANSFER*)pInBuf,
                                                    (ULONG*)pOutBuf);
             
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - ReadPhysicalSectors() failed.\r\n")));
            }
        }
        break;
           
        case IOCTL_FLASH_PDD_WRITE_PHYSICAL_SECTORS:
        {          
            if (pInBuf == NULL || InBufSize < sizeof(FLASH_PDD_TRANSFER))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            Result = pFlashPdd->WritePhysicalSectors (InBufSize / sizeof(FLASH_PDD_TRANSFER),
                                                     (FLASH_PDD_TRANSFER*)pInBuf,
                                                     (ULONG*)pOutBuf);
            
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - WritePhysicalSectors() failed.\r\n")));
            }
        }
        break;

        case IOCTL_FLASH_PDD_ERASE_BLOCKS:
        {          
            if (pInBuf == NULL || InBufSize < sizeof(BLOCK_RUN))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            Result = pFlashPdd->EraseBlocks (InBufSize / sizeof(BLOCK_RUN), (BLOCK_RUN*)pInBuf);
            
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - EraseBlocks() failed.\r\n")));
            }
        }
        break;

        case IOCTL_FLASH_PDD_COPY_PHYSICAL_SECTORS:
        {          
            if (pInBuf == NULL || InBufSize < sizeof(FLASH_PDD_COPY))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            Result = pFlashPdd->CopyPhysicalSectors (InBufSize / sizeof(FLASH_PDD_COPY),
                                                    (FLASH_PDD_COPY*)pInBuf);
            
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - CopyPhysicalSectors() failed.\r\n")));
            }
        }
        break;

        case IOCTL_FLASH_PDD_GET_PHYSICAL_SECTOR_ADDRESS:
        {
            if (pInBuf == NULL || InBufSize < sizeof(SECTOR_RUN) ||
                pOutBuf == NULL || OutBufSize < sizeof(VOID*))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            FLASH_GET_PHYSICAL_SECTOR_ADDRESS_REQUEST* pRequest = (FLASH_GET_PHYSICAL_SECTOR_ADDRESS_REQUEST*)pInBuf;

            if (pRequest->SectorRun.SectorCount * sizeof(VOID*) < OutBufSize)
            {
                Result = ERROR_INSUFFICIENT_BUFFER;
                goto exit;
            }                    
            
            Result = pFlashPdd->GetPhysicalSectorAddress (pRequest->RegionIndex, pRequest->SectorRun, (VOID**)pOutBuf);
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - GetPhysicalSectorAddress() failed.\r\n")));
            }
        }
        break;
            
        case IOCTL_FLASH_PDD_LOCK_BLOCKS:
        {
            if (pInBuf == NULL || InBufSize < sizeof(BLOCK_RUN))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }

            Result = pFlashPdd->LockBlocks (InBufSize / sizeof(BLOCK_RUN), (BLOCK_RUN*)pInBuf);
            
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - LockBlocks() failed.\r\n")));
            }   
        }
        break;  

        case IOCTL_FLASH_PDD_GET_LIFE_CYCLE_INFO:
        {
            if (pOutBuf == NULL || (OutBufSize < sizeof(FLASH_LIFE_CYCLE_INFO)))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }
            
            Result = pFlashPdd->GetLifeCycleInfo(OutBufSize / sizeof(FLASH_LIFE_CYCLE_INFO),
                                                (FLASH_LIFE_CYCLE_INFO*)pOutBuf);
            
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - GetLifeCycleInfo() failed.\r\n")));
            }
        }
        break;

        case IOCTL_FLASH_PDD_GET_IDENTITY_INFO:
        {
            if (pOutBuf == NULL || (OutBufSize < sizeof(FLASH_IDENTITY_INFO)))
            {
                Result = ERROR_INVALID_PARAMETER;
                goto exit;
            }
            
            Result = pFlashPdd->GetIdentityInfo((FLASH_IDENTITY_INFO*)pOutBuf);
            
            if (Result != ERROR_SUCCESS)
            {
                ReportError((TEXT("FmdWrapperPdd::IoControl - GetIdentityInfo() failed.\r\n")));
            }
        }
        break;
                      
        default:
        {
            // Pass any other IOCTL directly to the flash PDD.
            //
            Result = pFlashPdd->IoControl(
                         IoControlCode, 
                         pInBuf, InBufSize, 
                         pOutBuf, OutBufSize, 
                         pBytesReturned);
        }
        break;
    }
    }        
    __except (EXCEPTION_EXECUTE_HANDLER)  
    {      
        Result = ERROR_INVALID_PARAMETER;
    }
           
exit:
    if (Result != ERROR_SUCCESS)
    {
        SetLastError (Result);
    }
    return (Result == ERROR_SUCCESS);
    
}                                  
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_PowerUp()

Description:    Performs the necessary powerup procedures.
                
Returns:        N/A
-------------------------------------------------------------------*/
VOID FlashPdd_PowerUp()
{
    return;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Function:       FlashPdd_PowerDown()

Description:    Performs the necessary powerdown procedures.
                
Returns:        N/A
-------------------------------------------------------------------*/
VOID FlashPdd_PowerDown()
{
    return;
}



