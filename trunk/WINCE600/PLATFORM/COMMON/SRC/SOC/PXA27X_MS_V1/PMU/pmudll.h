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

#ifndef __PMUDLL_H__
#define __PMUDLL_H__

// PMU interrupt type
//
enum PMUIntType{
    IRQ,
    FIQ
};

enum PMURegAccessType{
    READ,
    WRITE
};

typedef unsigned long STATUS_PMU;   // PMU Status type

// PMU Handle
//
typedef struct {
    unsigned long   clockfreq;          // The CCF that the PMU resource has set
} PMUHResource, *PMUHandle;

// Status values for VTUNE PMU library
//
#define SUCCESS_PMU         0x0000  // no bit set for success
#define INVALIDHANDLE_PMU   0x0001
#define ACTIVECALLBACK_PMU  0x0002
#define NOCALLBACK_PMU      0x0003
#define KERNELIOFAILED_PMU  0x0004
#define FAILURE_PMU         0x0005

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PMUDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PMUDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef __cplusplus
    extern "C" {
#endif

#ifdef PMUDLL_EXPORTS
#define PMU_API __declspec(dllexport)
#else
#define PMU_API
#endif


//
// PMU API prototypes
//
PMU_API PMUHandle WINAPI AllocatePMU(void (*pCallBack)(void));
PMU_API BOOL WINAPI ReleasePMUResources(PMUHandle handle);
PMU_API STATUS_PMU WINAPI SetPMUInterruptCallback(
    PMUHandle handle, 
    enum PMUIntType FIQ_or_IRQ, 
    BOOL (*pCallBack)(SampleInput *));
PMU_API STATUS_PMU WINAPI  RemovePMUInterruptCallback(PMUHandle handle );
PMU_API unsigned int WINAPI GetNumberOfPossibleCoreClockFrequencies(void);
PMU_API STATUS_PMU WINAPI GetPossibleCoreClockFrequencies(
    unsigned int numberfreqs, 
    unsigned long *FrequencyArray);
PMU_API unsigned long WINAPI GetNominalCoreClockFrequency(void);
PMU_API unsigned long WINAPI GetCurrentCoreClockFrequency(void);
PMU_API unsigned long WINAPI SaveSetAndLockCoreClockFrequency(
    PMUHandle handle,
    unsigned long NewFrequency, 
    void (*pCallBack)(void));
PMU_API unsigned long WINAPI UnlockAndRestoreCoreClockFrequency(
    PMUHandle handle);
PMU_API BOOL WINAPI AccessPMUReg(
    PMUHandle handle, 
    enum PMURegAccessType Access, 
    unsigned long RegisterNumber,
    unsigned long *pValue);
PMU_API BOOL WINAPI GetCPUId(
    unsigned long *CPUId);
PMU_API BOOL WINAPI GetOEMConfigData(
    unsigned long   *configArray,
    unsigned long   maxItems,
    unsigned long   *nOutItems);

#ifdef __cplusplus
    }
#endif

#endif // __PMUDLLH__
