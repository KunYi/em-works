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
//
// Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  accpdd.cpp
//
//  Purpose: Provides the API implementation for accelerometer.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// INCLUDE FILES
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4214)
#include <ceddk.h>
#include <nkintr.h>
#include <Winreg.h>
#include "acc.h"
#include "accmain.h"

//----------------------------------------------------------------------------
// Global Variables

//----------------------------------------------------------------------------
// Definition
#define INTPIN       0  //default

//----------------------------------------------------------------------------
// Local Variable
static BOOL fIntrThreadLoop;// Interrupt thread loop flag
static DWORD  dwSysIntr1;
static DWORD  dwSysIntr2;
static HANDLE hACCIST1;
static HANDLE hACCIST2;
static HANDLE hInterrupted[2]; // Hardware Interrupt Occurence Event
static HANDLE hDRDYEvent1;
static HANDLE hDRDYEvent2;
static CRITICAL_SECTION csACC;

static BOOL   bValid;

// Variable for ACC operation
static ACC_MODE eMode;
static ACC_GSEL eGSel;
static ACC_POL  eLDPOL;
static ACC_POL  ePDPOL;
static ACC_OUTPUT_WIDTH eACCOutputWid;
static INT8 iACCLDTH;
static INT8 iACCPDTH;
static BYTE byLatencyTime;
static BYTE byPulseDuration;
static BYTE byTimeWindow;

static DWORD s_dwStatus;

extern DWORD BSPACCGetIRQ1(void);
extern DWORD BSPACCGetIRQ2(void);
extern BOOL BSPClearGPIOIntr(BYTE index);
extern BOOL BSPACCIOMUXConfig();
extern bool BSPEnableACC(void);
extern bool BSPDisableACC(void);
extern bool BSPACCSelfTest(void);
extern bool BSPACCCalibration(BYTE byGSel);
extern bool BSPACCSetMode(BYTE byMode);
extern bool BSPACCSetGSel(BYTE byGSel);
extern bool BSPACCSetDFBW(BYTE bDFBW);
extern bool BSPACCSetLDTH(INT8 iLDTH);
extern bool BSPACCSetPDTH(INT8 iPDTH);
extern bool BSPACCSetLDPOL(BYTE bLDPOL);
extern bool BSPACCSetPDPOL(BYTE bPDPOL);
extern bool BSPACCSetPulseDuration(BYTE byPulseDuration);
extern bool BSPACCSetLatencyTime(BYTE byLatencyTime);
extern bool BSPACCSetTimeWindow(BYTE byTimeWindow);
extern bool BSPACCSetTHOPT(BYTE byThopt);
extern bool BSPACCSetDetection(BYTE byDetection);
extern bool BSPACCSetXOffset(INT16 iXOffset);
extern bool BSPACCSetYOffset(INT16 iYOffset);
extern bool BSPACCSetZOffset(INT16 iZOffset);
extern bool BSPACCSetOffset(INT16 iXOffset, INT16 iYOffset, INT16 iZOffset);
extern bool BSPACCClearIntr();
extern bool BSPACCGetOutput8bit(INT8* iXOutput, INT8* iYOutput, INT8* iZOutput);
extern bool BSPACCGetOutput10bit(INT16* iXOutput, INT16* iYOutput, INT16* iZOutput);
extern bool BSPACCGetOffset(INT16* iXOffset, INT16* iYOffset, INT16* iZOffset);
extern bool BSPACCGetMode(PBYTE pbyMode);
extern bool BSPACCGetGSel(PBYTE pbyGSel);
extern bool BSPACCGetTHOPT(PBYTE byTHOPT);
extern bool BSPACCGetStatus(PBYTE byStatus);
extern bool BSPACCGetLDTH(PINT8 pLDTH);
extern bool BSPACCGetPDTH(PINT8 pPDTH);
extern bool BSPACCGetPulseDuration(PBYTE pbyPulseDuration);
extern bool BSPACCGetLatencyTime(PBYTE pbyLatencyTime);
extern bool BSPACCGetTimeWindow(PBYTE pbyTW);
extern bool BSPACCGetLDPOL(PBYTE pbyLDPOL);
extern bool BSPACCGetPDPOL(PBYTE pbyPDPOL);
extern bool BSPACCGetDFBW(PBYTE pbDFBW);
extern bool BSPACCGetXOffset(INT16* iXOffset);
extern bool BSPACCGetYOffset(INT16* iYOffset);
extern bool BSPACCGetZOffset(INT16* iZOffset);
extern bool BSPACCGetDetectionStatus(PBYTE pbyStatus);
extern bool BSPACCSetDRPD(BYTE byDRPD);


//------------------------------------------------------------------------------
//
//  Function:  ACCIST1
//
//  This function is the interrupt thread to handle ACC interrupts 1
//
//  Parameters:
//      LPVOID lpParameter
//
//  Returns:
//      None
//
//------------------------------------------------------------------------------
static DWORD WINAPI ACCIST1(LPVOID lpParameter)
{
    DWORD result;
    BYTE byteDetSrc;
    BYTE byteStatus;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

    CeSetThreadPriority(GetCurrentThread(), 247);

    while(fIntrThreadLoop) {
        result = WaitForSingleObject(hInterrupted[0], INFINITE);
        switch(result)
        {
            case (WAIT_OBJECT_0 + 0):
                //Read Three-Axis output
                bValid = FALSE;
#if 0
                if( (eMode == ACC_MODE_LEVELDETECTION)  ||
                    (eMode == ACC_MODE_PULSEDETECTION) ||
                    (eMode == ACC_MODE_MEASUREMENT)
                    )
                {
                    //Notes, in measurement mode, we may not trigger the event.
                    SetEvent(hDRDYEvent);
                }
#endif
                BSPACCGetDetectionStatus(&byteDetSrc);
                BSPACCGetStatus(&byteStatus);
                s_dwStatus = (byteDetSrc & 0x0FF) | 
                             ((byteStatus & 0x0FF) << 8);

                SetEvent(hDRDYEvent1);

                BSPACCClearIntr();
                BSPClearGPIOIntr(1);
                InterruptDone(dwSysIntr1);
                break;

            default:
                // Error
                break;
        }
    }
    return 0;
}


//------------------------------------------------------------------------------
//
//  Function:  ACCIST2
//
//  This function is the interrupt thread to handle ACC interrupts 2
//
//  Parameters:
//      LPVOID lpParameter
//
//  Returns:
//      None
//
//------------------------------------------------------------------------------
static DWORD WINAPI ACCIST2(LPVOID lpParameter)
{
    DWORD result;
    BYTE byteDetSrc;
    BYTE byteStatus;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

    CeSetThreadPriority(GetCurrentThread(), 247);

    while(fIntrThreadLoop) {
        result = WaitForSingleObject(hInterrupted[1], INFINITE);
        switch(result)
        {
            case (WAIT_OBJECT_0 + 0):
                //Read Three-Axis output
                BSPACCGetDetectionStatus(&byteDetSrc);
                BSPACCGetStatus(&byteStatus);
                s_dwStatus = (byteDetSrc & 0x0FF) | 
                             ((byteStatus & 0x0FF) << 8);

                SetEvent(hDRDYEvent2);

                BSPACCClearIntr();
                BSPClearGPIOIntr(2);
                InterruptDone(dwSysIntr2);
                break;

            default:
                // Error
                break;
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
//
//  Function:  ACCInitialize
//
//  This function initialize accelerometer driver.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
bool ACCInitialize(void)
{
    bool rc = FALSE;
    DWORD dwIrq1 = BSPACCGetIRQ1();
    DWORD dwIrq2 = BSPACCGetIRQ2();
    bValid = FALSE;
    // enable Accelerometer, set power, GPIO port
    if(!BSPEnableACC())
    {
        ERRORMSG(TRUE, (TEXT("ACCInitialize: BSPEnableACC failed!\r\n")));
        goto CleanUp;
    }

#if 0
    // do SelfTest function
    rc = BSPACCSelfTest();
    if(!rc)
    {
        ERRORMSG(TRUE, (TEXT("ACCInitialize: SelfTest failed!\r\n")));
        goto CleanUp;
    }
#endif
    // Create the hardware interrupt event
    hInterrupted[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(hInterrupted[0] == NULL){
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Create event for HW interrupt1 failed!\r\n")));
        goto CleanUp;
    }

    // Create the hardware interrupt event
    hInterrupted[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(hInterrupted[1] == NULL){
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Create event for HW interrupt2 failed!\r\n")));
        goto CleanUp;
    }

    // Create software interrupt event for DataRDY
    // This event is used as DataRDY in measurement mode, and is used as
    // PulseDetected/LevelDetected in Pulse/Level Detection mode.
    hDRDYEvent1 = CreateEvent(NULL, FALSE, FALSE, ACC_DRDY_NAME1);
    if(hDRDYEvent1 == NULL){
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Create event for ACC DRDY event failed!\r\n")));
        goto CleanUp;
    }

    hDRDYEvent2 = CreateEvent(NULL, FALSE, FALSE, ACC_DRDY_NAME2);
    if(hDRDYEvent2 == NULL){
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Create event for ACC DRDY event failed!\r\n")));
        goto CleanUp;
    }

    // Get kernel to translate IRQ -> System Interrupt ID
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq1, sizeof(DWORD), &dwSysIntr1, sizeof(DWORD), NULL)){
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Request SYSINTR for acc interrupt1 failed !\r\n")));
        goto CleanUp;
    }

    // Get kernel to translate IRQ -> System Interrupt ID
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq2, sizeof(DWORD), &dwSysIntr2, sizeof(DWORD), NULL)){
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Request SYSINTR for acc interrupt2 failed !\r\n")));
        goto CleanUp;
    }

    // Create IST thread to receive hardware interrupts
    fIntrThreadLoop = TRUE;
    hACCIST1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ACCIST1, NULL, 0, NULL);
    if(hACCIST1 == NULL) {
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Create thread1 for ACC failed !\r\n")));
        goto CleanUp;
    }

    hACCIST2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ACCIST2, NULL, 0, NULL);
    if(hACCIST2 == NULL) {
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Create thread2 for ACC failed !\r\n")));
        goto CleanUp;
    }

    // Link hInterrupted -> ACC1 Interrupt Pin
    if (!InterruptInitialize(dwSysIntr1, hInterrupted[0], NULL, 0)) {
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Initialize ACC interrupt failed !\r\n")));
        goto CleanUp;
    }

    if (!InterruptInitialize(dwSysIntr2, hInterrupted[1], NULL, 0)) {
        ERRORMSG(TRUE, (TEXT("ACCInitialize: Initialize ACC interrupt failed !\r\n")));
        goto CleanUp;
    }

    InitializeCriticalSection(&csACC);

    rc = TRUE;

CleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  ACCDeinitialize
//
// This function deinitializes the ACC driver.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void ACCDeinitialize(void)
{
    // Give the IST thread a chance to perform any cleanup.
    if(hACCIST1) {
        fIntrThreadLoop = FALSE;
        SetEvent(hInterrupted[0]);
        CloseHandle(hACCIST1);
        hACCIST1 = NULL;
    }

    if(hACCIST2) {
        fIntrThreadLoop = FALSE;
        SetEvent(hInterrupted[1]);
        CloseHandle(hACCIST2);
        hACCIST2 = NULL;
    }

    if(hInterrupted[0]){
        CloseHandle(hInterrupted[0]);
        hInterrupted[0] = NULL;
    }

    if(hInterrupted[1]){
        CloseHandle(hInterrupted[1]);
        hInterrupted[1] = NULL;
    }

    if(hDRDYEvent1){
        CloseHandle(hDRDYEvent1);
        hDRDYEvent1 = NULL;
    }

    if(hDRDYEvent2){
        CloseHandle(hDRDYEvent2);
        hDRDYEvent2 = NULL;
    }

    // Release SYSINTR
    if(dwSysIntr1 != SYSINTR_UNDEFINED){
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntr1,
            sizeof(DWORD), NULL, 0, NULL);
        dwSysIntr1 = (DWORD) SYSINTR_UNDEFINED;
    }

    if(dwSysIntr2 != SYSINTR_UNDEFINED){
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &dwSysIntr2,
            sizeof(DWORD), NULL, 0, NULL);
        dwSysIntr2 = (DWORD) SYSINTR_UNDEFINED;
    }

    BSPDisableACC();

    DeleteCriticalSection(&csACC);

}




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//   The following methods are the APIs of VPU to access the hardware
//
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetMode
//
//  This function set accelerometer mode.
//
//  Parameters:
//      eACCMode
//          [in] accelerometer mode.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetMode(ACC_MODE eACCMode)
{
    bool bRet;
    // first using eGSel to calibrate accelerometer
#if 0
    bRet = BSPACCCalibration((BYTE)eGSel);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetMode: BSPACCCalibration failed !\r\n")));
        return FALSE;
    }
#endif
    eMode = eACCMode;
    // set mode
    bRet = BSPACCSetMode((BYTE)eACCMode);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetMode: BSPACCSetMode failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetGSel
//
//  This function set accelerometer g-sel.
//
//  Parameters:
//      eGSel
//          [in] accelerometer g-sel.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetGSel(ACC_GSEL eACCGSel)
{
    bool bRet;

    // set g-sel
    bRet = BSPACCSetGSel((BYTE)eACCGSel);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetGSel: BSPACCSetGSel failed !\r\n")));
        return FALSE;
    }
    eGSel = eACCGSel;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_SetDRPD
//
//  This function set/unset Data Ready output to Interrupt INT1.
//
//  Parameters:
//      eDRPD
//          [in] enable/disable DRPD.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetDRPD(ACC_DRPD eDRPD)
{
    bool bRet;

    // set DRPD
    bRet = BSPACCSetDRPD((BYTE)eDRPD);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetDRPD: BSPACCSetDRPD failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}



//------------------------------------------------------------------------------
//
//  Function:  ACC_SetOutputWidth
//
//  This function set output width.
//
//  Parameters:
//      eOutputWid
//          [IN] output width
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetOutputWidth(ACC_OUTPUT_WIDTH eOutputWid)
{
    eACCOutputWid = eOutputWid;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetDFBW
//
//  This function set digital filter band width.
//
//  Parameters:
//      eDFBW
//          [IN] digital filter band width
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetDFBW(ACC_DFBW eDFBW)
{
    bool bRet;

    // set DFBW
    bRet = BSPACCSetDFBW((BYTE)eDFBW);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetDFBW: BSPACCSetDFBW failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetTHOPT
//
//  This function set threshold option.
//
//  Parameters:
//      eTHOPT
//          [IN] threshold option
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetTHOPT(ACC_THRESHOLD_OPT eTHOPT)
{
    bool bRet;

    // set DFBW
    bRet = BSPACCSetTHOPT((BYTE)eTHOPT);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetDFBW: BSPACCSetDFBW failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetDetection
//
//  This function set detection enable/disable option.
//
//  Parameters:
//      byDetection
//          [IN] detection enable/disable option.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetDetection(BYTE byDetection)
{
    bool bRet;

    // set DFBW
    bRet = BSPACCSetDetection(byDetection);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetDetection: BSPACCSetDetection failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetXOffset
//
//  This function set x-axis offset.
//
//  Parameters:
//      byDetection
//          [IN] x-axis offset.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetXOffset(INT16 iXOffset)
{
    bool bRet;

    bRet = BSPACCSetXOffset(iXOffset);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetXOffset: BSPACCSetXOffset failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetYOffset
//
//  This function set y-axis offset.
//
//  Parameters:
//      byDetection
//          [IN] y-axis offset.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetYOffset(INT16 iYOffset)
{
    bool bRet;

    // set DFBW
    bRet = BSPACCSetYOffset(iYOffset);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetYOffset: BSPACCSetYOffset failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetZOffset
//
//  This function set z-axis offset.
//
//  Parameters:
//      byDetection
//          [IN] z-axis offset.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetZOffset(INT16 iZOffset)
{
    bool bRet;

    // set DFBW
    bRet = BSPACCSetZOffset(iZOffset);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetZOffset: BSPACCSetZOffset failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetLDTH
//
//  This function set accelerometer level detection threshold limit value.
//
//  Parameters:
//      iLDTH
//          [in] level detection threshold limit value.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetLDTH(INT8 iLDTH)
{
    bool bRet;

    // set LDTH
    bRet = BSPACCSetLDTH(iLDTH);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetLDTH: BSPACCSetLDTH failed !\r\n")));
        return FALSE;
    }
    iACCLDTH = iLDTH;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_SetPDTH
//
//  This function set accelerometer pulse detection threshold limit value.
//
//  Parameters:
//      iPDTH
//          [in] pulse detection threshold limit value.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetPDTH(INT8 iPDTH)
{
    bool bRet;

    // set PDTH
    bRet = BSPACCSetPDTH(iPDTH);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetPDTH: BSPACCSetPDTH failed !\r\n")));
        return FALSE;
    }
    iACCPDTH = iPDTH;
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_SetLDPOL
//
//  This function set accelerometer level detection polarity.
//
//  Parameters:
//      ePOL
//          [in] level detection polaritys.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetLDPOL(ACC_POL ePOL)
{
    bool bRet;
    BYTE byLDPOL = (BYTE)ePOL;

    // set LDPOL
    bRet = BSPACCSetLDPOL(byLDPOL);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetLDTH: BSPACCSetLDPOL failed !\r\n")));
        return FALSE;
    }

    eLDPOL = ePOL;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_SetPDPOL
//
//  This function set accelerometer pulse detection polarity.
//
//  Parameters:
//      ePOL
//          [in] pulse detection polarity.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetPDPOL(ACC_POL ePOL)
{
    bool bRet;
    BYTE byPDPOL = (BYTE)ePOL;

    // set PDPOL
    bRet = BSPACCSetPDPOL(byPDPOL);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetPDTH: BSPACCSetPDPOL failed !\r\n")));
        return FALSE;
    }

    ePDPOL = ePOL;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_SetLatencyTime
//
//  This function set accelerometer LatencyTime.
//
//  Parameters:
//      byLT
//          [in] latency time for pulse dection mode.
//           1-0xFF (1ms-255ms)
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetLatencyTime(BYTE byLT)
{
    bool bRet;

    // set LatencyTime
    bRet = BSPACCSetLatencyTime(byLT);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetLatencyTime: BSPACCSetLatencyTime failed !\r\n")));
        return FALSE;
    }
    byLatencyTime = byLT;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_SetPulseDuration
//
//  This function set accelerometer PulseDuration.
//
//  Parameters:
//      byPD
//          [in] pulse duration time for pulse dection mode.
//           1-0xFF (0.5ms-127ms)
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetPulseDuration(BYTE byPD)
{
    bool bRet;

    // set PulseDuration
    bRet = BSPACCSetPulseDuration(byPD);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetPulseDuration: BSPACCSetPulseDuration failed !\r\n")));
        return FALSE;
    }
    byPulseDuration = byPD;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_SetTimeWindow
//
//  This function set accelerometer TimeWindow.
//
//  Parameters:
//      byTW
//          [in] time window for pulse dection mode.
//           0-0xFF (0ms-255ms)
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_SetTimeWindow(BYTE byTW)
{
    bool bRet;

    // set TimeWindow
    bRet = BSPACCSetTimeWindow(byTW);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_SetTimeWindow: BSPACCSetTimeWindow failed !\r\n")));
        return FALSE;
    }
    byTimeWindow = byTW;
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_GetOutput
//
//  This function get accelerometer output of three axis.
//
//  Parameters:
//      pOutput
//          [OUT] pointer to ACC_OUTPUT
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetOutput(ACC_OUTPUT* pOutput)
{
    bool bRet;
    float range;

    if(eGSel == ACC_GSEL_8G) 
    {
        range = 8.0;
    }
    else if(eGSel == ACC_GSEL_2G)
    {
        range = 2.0;
    }
    else if(eGSel == ACC_GSEL_4G)
    {
        range = 4.0;
    }
    else
    {
        return FALSE;
    }

    if(eACCOutputWid == ACC_OUTPUT_8BIT)
    {
        pOutput->eOutputWidth = ACC_OUTPUT_8BIT;
        bRet = BSPACCGetOutput8bit(&pOutput->iXOutput8, &pOutput->iYOutput8, &pOutput->iZOutput8);
        if(!bRet)
        {
            return FALSE;
        }
        
        pOutput->x = (pOutput->iXOutput8 - 128)*range/128;
        pOutput->y = (pOutput->iYOutput8 - 128)*range/128;
        pOutput->z = (pOutput->iZOutput8 - 128)*range/128;
    }
    else
    {
        pOutput->eOutputWidth = ACC_OUTPUT_10BIT;
        bRet = BSPACCGetOutput10bit(&pOutput->iXOutput10, &pOutput->iYOutput10, &pOutput->iZOutput10);
        if(!bRet)
        {
            return FALSE;
        }

        pOutput->x = (pOutput->iXOutput8 - 512)*range/512;
        pOutput->y = (pOutput->iYOutput8 - 512)*range/512;
        pOutput->z = (pOutput->iZOutput8 - 512)*range/512;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetMode
//
//  This function get accelerometer current mode.
//
//  Parameters:
//      peMode
//          [OUT] pointer to mode
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetMode(ACC_MODE* peMode)
{
    bool bRet;
    BYTE byMode;

    bRet = BSPACCGetMode(&byMode);
    if(!bRet)
    {
        return FALSE;
    }

    *peMode = (ACC_MODE)byMode;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetGSel
//
//  This function get accelerometer current g-sel.
//
//  Parameters:
//      peGSel
//          [OUT] pointer to g-sel
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetGSel(ACC_GSEL *peGSel)
{
    bool bRet;
    BYTE byGSel;

    bRet = BSPACCGetGSel(&byGSel);
    if(!bRet)
    {
        return FALSE;
    }

    *peGSel = (ACC_GSEL)byGSel;

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_GetOutputWidth
//
//  This function get output width.
//
//  Parameters:
//      peOutputWid
//          [OUT] pointer to output width
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetOutputWidth(ACC_OUTPUT_WIDTH *peOutputWid)
{
    *peOutputWid = eACCOutputWid;

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_GetLDTH
//
//  This function get accelerometer level detection threshold limit value.
//
//  Parameters:
//      piLDTH
//          [in] pointer to level detection threshold limit value.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetLDTH(INT8* piLDTH)
{
    bool bRet;

    // get LDTH
    bRet = BSPACCGetLDTH(piLDTH);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetLDTH: BSPACCGetLDTH failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_GetPDTH
//
//  This function get accelerometer pulse detection threshold limit value.
//
//  Parameters:
//      piPDTH
//          [in] pointer to pulse detection threshold limit value.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetPDTH(INT8* piPDTH)
{
    bool bRet;

    // get LDTH
    bRet = BSPACCGetPDTH(piPDTH);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetPDTH: BSPACCGetPDTH failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetLDPOL
//
//  This function get accelerometer level detection polarity.
//
//  Parameters:
//      pePOL
//          [OUT] pointer to level detection polarity.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetLDPOL(ACC_POL* pePOL)
{
    bool bRet;
    BYTE byPOL;

    // get LDTH
    bRet = BSPACCGetLDPOL(&byPOL);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetLDPOL: BSPACCGetLDPOL failed !\r\n")));
        return FALSE;
    }
    *pePOL = (ACC_POL)byPOL;

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  ACC_GetPDPOL
//
//  This function get accelerometer pulse detection polarity.
//
//  Parameters:
//      pePOL
//          [OUT] pointer to pulse detection polarity.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetPDPOL(ACC_POL* pePOL)
{
    bool bRet;
    BYTE byPOL;

    // get LDTH
    bRet = BSPACCGetPDPOL(&byPOL);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetPDPOL: BSPACCGetPDPOL failed !\r\n")));
        return FALSE;
    }
    *pePOL = (ACC_POL)byPOL;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetLatencyTime
//
//  This function get accelerometer current latency time.
//
//  Parameters:
//      pbyLT
//          [OUT] pointer to latency time
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetLatencyTime(PBYTE pbyLT)
{
    bool bRet;

    bRet = BSPACCGetLatencyTime(pbyLT);
    if(!bRet)
    {
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetPulseDuration
//
//  This function get accelerometer current pulse duration.
//
//  Parameters:
//      pbyPD
//          [OUT] pointer to latency time
//          1-0xFF (0.5ms-127ms)
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetPulseDuration(PBYTE pbyPD)
{
    bool bRet;

    bRet = BSPACCGetPulseDuration(pbyPD);
    if(!bRet)
    {
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetTimeWindow
//
//  This function get accelerometer current time window.
//
//  Parameters:
//      pbyTW
//          [OUT] pointer to latency time
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetTimeWindow(PBYTE pbyTW)
{
    bool bRet;

    bRet = BSPACCGetTimeWindow(pbyTW);
    if(!bRet)
    {
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetDFBW
//
//  This function get digital filter band width.
//
//  Parameters:
//      peDFBW
//          [out] pointer to digital filter band width
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetDFBW(ACC_DFBW* peDFBW)
{
    bool bRet;
    BYTE byDFBW;

    // set DFBW
    bRet = BSPACCGetDFBW(&byDFBW);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetDFBW: BSPACCGetDFBW failed !\r\n")));
        return FALSE;
    }
    *peDFBW = (ACC_DFBW)byDFBW;
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetTHOPT
//
//  This function Get threshold option.
//
//  Parameters:
//      peTHOPT
//          [OUT] pointer to threshold option
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetTHOPT(ACC_THRESHOLD_OPT* peTHOPT)
{
    bool bRet;
    BYTE byTHOPT;

    bRet = BSPACCGetTHOPT(&byTHOPT);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetTHOPT: BSPACCGetTHOPT failed !\r\n")));
        return FALSE;
    }
    *peTHOPT = (ACC_THRESHOLD_OPT)byTHOPT;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetXOffset
//
//  This function get x-axis offset.
//
//  Parameters:
//      piXOffset
//          [OUT] pointer to x-axis offset.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetXOffset(INT16* piXOffset)
{
    bool bRet;

    bRet = BSPACCGetXOffset(piXOffset);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetXOffset: BSPACCGetXOffset failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetYOffset
//
//  This function get y-axis offset.
//
//  Parameters:
//      piXOffset
//          [OUT] pointer to y-axis offset.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetYOffset(INT16* piYOffset)
{
    bool bRet;

    bRet = BSPACCGetYOffset(piYOffset);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetYOffset: BSPACCGetYOffset failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetZOffset
//
//  This function get z-axis offset.
//
//  Parameters:
//      piXOffset
//          [OUT] pointer to z-axis offset.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetZOffset(INT16* piZOffset)
{
    bool bRet;

    bRet = BSPACCGetZOffset(piZOffset);
    if(!bRet)
    {
        ERRORMSG(TRUE, (TEXT("ACC_GetZOffset: BSPACCGetZOffset failed !\r\n")));
        return FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ACC_GetStatus
//
//  This function get status.
//
//  Parameters:
//      pStatus
//          [OUT] pointer to status.
//
//  Returns:
//      TRUE if sucess, FALSE if failed
//
//------------------------------------------------------------------------------
bool  ACC_GetStatus(ACC_STATUS* pStatus)
{
    BYTE byStatus;

    byStatus = s_dwStatus & 0x00FF;
    memcpy((PVOID)pStatus, (PVOID)&byStatus, sizeof(BYTE));

    return TRUE;
}
