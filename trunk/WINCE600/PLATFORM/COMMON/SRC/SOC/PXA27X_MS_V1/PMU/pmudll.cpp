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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/
/*++

Module Name:  PMUdll.cpp

Abstract:  
 Defines the entry points for the VTune PMU DLL.

Functions:


Notes: 

--*/


#include "stdafx.h"
#include <stdlib.h>
#include <ceddk.h>
#include <malloc.h>
#include <pmu.h>  
#include <pmuioctl.h>  
#include "pmudll.h"     

#define MAXFREQS 8


static HANDLE trySemaphore = 0;      // Semaphore for PMU resource allocation
static HANDLE semHandle = 0;         // Semaphore for PMU resource allocation

// Local copies of callback ptrs
//
LPVOID PVTuneInterrupt = NULL;      
LPVOID PVTuneReleasePMU = NULL;     
LPVOID PVTuneReleaseCCF = NULL;   


//------------------------------------------------------------------------------
//
//  Function:  AllocatePMU
//
//  Returns a handle identifying the PMU resource
//  If PMU not available, returns 0.
//
PMUHandle WINAPI AllocatePMU(void (*pCallBack)(void))
{

    PMUHandle   handle = 0;
    PMURegInfo  PMURegBuffer;
    BOOL        rtnvalue = FALSE;
    LPCTSTR     semName = TEXT("XSC_PMU_ALLOC");

    // Allocate a system resource, initialize to unsignalled state.
    // If it's already allocated, return NULL handle.
    //
    trySemaphore = CreateSemaphore (NULL, 1, 1, semName);
    if (trySemaphore == NULL) 
    {
        return 0;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        //
        // Invalidate this handle
        //
        CloseHandle(trySemaphore);
        return 0;
    }

        semHandle = trySemaphore;

        // Register callback with kernel
        // NOTE: Intel no longer uses the pCallBack parameter that was stored in 
        // the PVTuneReleasePMU variable, it is understood to be NULL.
        PMURegBuffer.subcode = PMU_ALLOCATE;
        PMURegBuffer.pCallback = PVTuneReleasePMU;

        rtnvalue = 
                KernelIoControl (IOCTL_PMU_CONFIG, (void *)&PMURegBuffer, sizeof(PMURegInfo), 
                                 (LPVOID)NULL, 0, (LPDWORD)NULL);
        if (rtnvalue == FALSE)
        {
            return 0;
        }

        // Null other callbacks
        //
        PVTuneInterrupt = NULL;
        PVTuneReleaseCCF = NULL;

        // Allocate a PMUHandle
        //
        handle = (PMUHandle) malloc (sizeof (PMUHResource));

    return handle;

}

//------------------------------------------------------------------------------
//
//  Function:  ReleasePMUResources
//
//  Release the PMU Resources
//  So far this releases the PMU resource and
//  NULLs the handle.
//
BOOL WINAPI ReleasePMUResources(PMUHandle handle)
{   
    PMURegInfo  PMURegBuffer;
    BOOL        rtnvalue = FALSE;

    if (handle == 0)
    {
        return FALSE;
    }

    //
    // Remove Interrupt callback
    //
    RemovePMUInterruptCallback(handle);

    //
    // Remove lock, if any, and release CCF callback
    //
    UnlockAndRestoreCoreClockFrequency(handle);

    //
    // Null local copy
    //
    PVTuneReleasePMU = NULL;

    //
    // Unregister ReleasePMU callback in kernel
    //
    PMURegBuffer.subcode = PMU_RELEASE;
    rtnvalue = 
            KernelIoControl (IOCTL_PMU_CONFIG, (void *)&PMURegBuffer, sizeof(PMURegInfo), 
                             (LPVOID)NULL, 0, (LPDWORD)NULL);

    //
    // Release resource, NULL handle
    //
    ReleaseSemaphore(semHandle, 1, NULL);
    CloseHandle(semHandle);
    free ((PMUHResource*) handle);
    handle = 0;

    return rtnvalue;
}


//------------------------------------------------------------------------------
//
//  Function:  RemovePMUInterruptCallback
//
//  Unregisters the PMU interrupt callback routine.
//
STATUS_PMU WINAPI RemovePMUInterruptCallback(
    PMUHandle handle )
{
    PMURegInfo      PMURegBuffer;
    BOOL            rtnvalue = FALSE;

    if (handle == 0)
    {
        return INVALIDHANDLE_PMU;
    }

    //
    // Remove the interrupt callback
    //
    if (PVTuneInterrupt == NULL)
    {
        return NOCALLBACK_PMU;
    }

    PVTuneInterrupt = NULL;

    //
    // Disable the interrupt (IRQ for now...) and 
    // unregister the callback in the kernel
    //
    PMURegBuffer.subcode = PMU_DISABLE_IRQ;
    rtnvalue = 
            KernelIoControl (IOCTL_PMU_CONFIG, (void *)&PMURegBuffer, sizeof(PMURegInfo), 
                             (LPVOID)NULL, 0, (LPDWORD)NULL);

    if (rtnvalue == FALSE)
    {
        return KERNELIOFAILED_PMU;
    }

    return SUCCESS_PMU;

}


//------------------------------------------------------------------------------
//
//  Function:  GetNumberOfPossibleCoreClockFrequencies
//
//  Returns the number of frequency values that
//  can be set by the OS.
//
unsigned int WINAPI GetNumberOfPossibleCoreClockFrequencies(void)
{
    return MAXFREQS;
}


//------------------------------------------------------------------------------
//
//  Function:  GetPossibleCoreClockFrequencies
// 
//  Write the possible frequencies as unsigned ints in the buffer provided.
//  FrequencyArray buffer is allocated by the caller.
//
//  Frequencies expressed in KHz
//
STATUS_PMU WINAPI GetPossibleCoreClockFrequencies(
    unsigned int numberfreqs, 
    unsigned long *FrequencyArray)
{

/*
 * Restrict frequencies between the same LCD Frequency
 * (K) range until LCD controller can be disabled
 *
 *    static unsigned long FreqArray[MAXFREQS] =
 *      {26000, 39000, 52000, 65000, 78000,
 *         91000, 104000, 117000, 130000, 143000,
 *         156000, 169000, 182000, 195000,
 *         208000, 377000, 390000, 403000};
 */

    static unsigned long FreqArray[MAXFREQS] =
    {104000, 117000, 130000, 143000,
        156000, 169000, 182000, 195000};


    unsigned int    i;

    if (numberfreqs > MAXFREQS)
    {
        return FAILURE_PMU;
    }

    //
    // Calculate possible frequencies
    //
    for (i=0; i < numberfreqs; i++)
    {
        FrequencyArray[i] = FreqArray[i];
    }

    return SUCCESS_PMU;
}

//------------------------------------------------------------------------------
//
//  Function:  GetNominalCoreClockFrequency
//
//  Returns the nominal or default runtime frequency (in KHz).
//
unsigned long WINAPI GetNominalCoreClockFrequency(void)
{
    return 208000;
}


//------------------------------------------------------------------------------
//
//  Function:  GetCurrentCoreClockFrequency
//
//  Returns the current core clock frequency (in KHz).
//
unsigned long WINAPI GetCurrentCoreClockFrequency(void)
{

    PMUCCFInfo      PMUCCFBuffer;
    unsigned long   CurFrequency,
                    nOutBytes;
    BOOL            rtnvalue = FALSE;

    //
    // This API calls KernelIoControl to access to get
    // the current core clock frequency.
    // This in turn calls the OEMIoControl routine.  
    // Note, the system is fully preemptible when this
    // routine is called.
    //

    //
    // Set the subcode for get ccf
    //
    PMUCCFBuffer.subcode = PMU_CCF_GETCURRENT;
    rtnvalue = 
        KernelIoControl (IOCTL_PMU_CCF, (void *)&PMUCCFBuffer, sizeof(PMUCCFInfo), 
                         &CurFrequency, sizeof(CurFrequency), &nOutBytes);

    if (rtnvalue && (nOutBytes == sizeof(unsigned long)))
    {
        return CurFrequency;
    }
    else
    {
        return 0;
    }
}


//------------------------------------------------------------------------------
//
//  Function:  UnlockAndRestoreCoreClockFrequency
//
//  Unregister the callback routine, relinquishes core clock resource ownership,
//  and requests that the OS restore the core clock frequency as it was before
//  the SaveSetandLock... call
//
unsigned long WINAPI UnlockAndRestoreCoreClockFrequency(
    PMUHandle handle
    )
{
    PMUCCFInfo      PMUCCFBuffer;
    BOOL            rtnvalue = FALSE;

    if (handle == 0)
    {
        return 0;
    }

    PVTuneReleaseCCF = NULL;

    //
    // This API calls KernelIoControl to unlock and
    // restore the previous core clock frequency.
    //
    // Note, the system is fully preemptible when this
    // routine is called.
    //

    //
    // Set the subcode for unlock CCF
    //
    PMUCCFBuffer.subcode = PMU_CCF_UNLOCK;

    rtnvalue = 
        KernelIoControl (IOCTL_PMU_CCF, (void *)&PMUCCFBuffer, sizeof(PMUCCFInfo), 
                         (LPVOID)NULL, 0, (LPDWORD)NULL);

    if (rtnvalue)
    {
        // Returns the restored core clock frequency (in KHz).
        //
        return GetCurrentCoreClockFrequency();
    }
    else
    {
        return 0;
    }


}

//------------------------------------------------------------------------------
//
//  Function:  AccessPMUReg
//
//  This API calls KernelIoControl to access the PMU Register.
//  This in turn calls the OEMIoControl routine.  
//  Note, the system is fully preemptible when this
//  routine is called.
//
BOOL WINAPI AccessPMUReg(
    PMUHandle handle, 
    enum PMURegAccessType Access, 
    unsigned long RegisterNumber,
    unsigned long *pValue)
{
    PMURegInfo      PMURegBuffer;
    unsigned long   PMURegResults,
                    nOutBytes;
    BOOL            rtnvalue = FALSE;
        
    if (handle == 0)
    {
        return FALSE;
    }

    PMURegBuffer.PMUReg = RegisterNumber;

    switch (Access)
    {
    case READ:
        PMURegBuffer.subcode = PMU_READ_REG;
        rtnvalue = 
            KernelIoControl (IOCTL_PMU_CONFIG, (void *)&PMURegBuffer, sizeof(PMURegInfo), 
                             &PMURegResults, sizeof(PMURegResults), &nOutBytes);
        if (rtnvalue && (nOutBytes == sizeof(unsigned long)))
        {
            *pValue = PMURegResults;
        }
        else
        {
            rtnvalue = FALSE;
        }

        break;
    case WRITE:
        PMURegBuffer.subcode = PMU_WRITE_REG;
        PMURegBuffer.PMUValue = *pValue;

        rtnvalue = 
            KernelIoControl (IOCTL_PMU_CONFIG, (void *)&PMURegBuffer, sizeof(PMURegInfo), 
                             &PMURegResults, sizeof(PMURegResults), &nOutBytes);
        break;
    default:
        return FALSE;
        break;

    }

    return rtnvalue;
}

//------------------------------------------------------------------------------
//
//  Function:  GetCPUId
//
//  This API calls KernelIoControl to retrieve the CPU Id.
//  This in turn calls the OEMIoControl routine.  
//  Note, the system is fully preemptible when this
//  routine is called.
//
BOOL WINAPI GetCPUId(
    unsigned long *CPUId)
{
    CPUIdInfo       CPUIdBuffer;
    unsigned long   CPUIdResults,
                    nOutBytes;
    BOOL            rtnvalue = FALSE;

    rtnvalue = 
        KernelIoControl (IOCTL_GET_CPU_ID, (void *)&CPUIdBuffer, sizeof(CPUIdInfo), 
                         &CPUIdResults, sizeof(CPUIdResults), &nOutBytes);
    if (rtnvalue && (nOutBytes == sizeof(unsigned long)))
    {
        *CPUId = CPUIdResults;
    }
    else
    {
        rtnvalue = FALSE;
    }

    return rtnvalue;

}

//------------------------------------------------------------------------------
//
//  Function:  GetOEMConfigData
//
//  This API calls KernelIoControl to retrieve the OEM 
//  configuration data. This in turn calls the OEMIoControl routine.  
//  Note, the system is fully preemptible when this
//  routine is called.
//
//  Currently, the OEM configuration data retrieved consists 
//  of two items:  the system interrupt ID assigned by the OEM, 
//  and the address to the VTune PMU driver globals area.
//
BOOL WINAPI GetOEMConfigData (
    unsigned long   *configArray,
    unsigned long   maxItems,
    unsigned long   *nOutItems)
{
    OEMInfo         OEMBuffer;
    PMURegInfo      PMURegBuffer;
    unsigned long   nOutBytes;
    BOOL            rtnvalue = FALSE;

    PMURegBuffer.subcode = PMU_OEM_INFO;
    rtnvalue = 
        KernelIoControl (IOCTL_PMU_CONFIG, (void *)&PMURegBuffer, sizeof(PMURegInfo), 
                     &OEMBuffer, sizeof(OEMInfo), &nOutBytes);

    if (rtnvalue && ((nOutBytes != sizeof(OEMInfo) || 
        (nOutBytes > (maxItems*sizeof(unsigned long))))))
    {
        rtnvalue = FALSE;
        *nOutItems = 0;
    }
    else
    {
        configArray[0] = OEMBuffer.sysintrID;
        configArray[1] = OEMBuffer.PMUglobals;
        *nOutItems = 2;
   }

    return rtnvalue;

}
