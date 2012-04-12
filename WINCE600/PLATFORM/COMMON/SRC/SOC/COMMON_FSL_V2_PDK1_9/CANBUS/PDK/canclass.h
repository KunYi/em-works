//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  canclass.h
//
//  Header file, for CAN bus driver.
//
//------------------------------------------------------------------------------

#ifndef _INC_CANCLASS_H
#define _INC_CANCLASS_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_INIT         0
#define ZONEID_DEINIT       1
#define ZONEID_OPEN         2
#define ZONEID_CLOSE        3
#define ZONEID_IOCTL        4
#define ZONEID_THREAD       5
#define ZONEID_FUNCTION     13
#define ZONEID_WARN         14
#define ZONEID_ERROR        15

// Debug zone masks
#define ZONEMASK_INIT       (1 << ZONEID_INIT)
#define ZONEMASK_DEINIT     (1 << ZONEID_DEINIT)
#define ZONEMASK_OPEN       (1 << ZONEID_OPEN)
#define ZONEMASK_CLOSE      (1 << ZONEID_CLOSE)
#define ZONEMASK_IOCTL      (1 << ZONEID_IOCTL)
#define ZONEMASK_THREAD     (1 << ZONEID_THREAD)
#define ZONEMASK_FUNCTION   (1 << ZONEID_FUNCTION)
#define ZONEMASK_WARN       (1 << ZONEID_WARN)
#define ZONEMASK_ERROR      (1 << ZONEID_ERROR)

#define ZONE_INIT       DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT     DEBUGZONE(ZONEID_DEINIT)
#define ZONE_OPEN       DEBUGZONE(ZONEID_OPEN)
#define ZONE_CLOSE      DEBUGZONE(ZONEID_CLOSE)
#define ZONE_IOCTL      DEBUGZONE(ZONEID_IOCTL)
#define ZONE_THREAD     DEBUGZONE(ZONEID_THREAD)
#define ZONE_FUNCTION   DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN       DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR      DEBUGZONE(ZONEID_ERROR)
#endif // DEBUG

#define CAN_DEF_NEW_ENTRY_WAIT_TIME     10000       // Default New List Entry Waiting Time

#define CAN_MASTER_MODE                 1           // CAN Master Mode

//Set slave mode from IOCTL is obsolete feature, use enable slave instead.
//Keep the definition here for back compatible purpose.

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

class CANClass
{
public:
    //-------------------------------------
    // CAN CONSTRUCTOR/DESTRUCTOR METHODS
    //

    CANClass(UINT32 index);
    ~CANClass(void);
   
   BOOL ReadPacket(PCAN_PACKET pCANPkt, BOOL bLast);

   BOOL WritePacket(PCAN_PACKET pCANPkt, BOOL bLast);

   void CANSetMode(DWORD index,CAN_MODE mode);

   BOOL CANEnableTLPRIOMode (BOOL ISEnable);

   BOOL CANPowerHandler(BOOL boff);
   
    //-------------------------------------
    // CAN TRANSACTION METHODS
    //

    BOOL ProcessPackets(CAN_PACKET [], INT32);


    //-------------------------------------
    // CAN CR METHODS
    //

   BOOL CANReset() ;// Only reset CAN, does not affect the class fields

    //-------------------------------------
    // CAN CLOCK METHODS

private:

    //-------------------------------------
    // CAN Private Helper methods
    //    
    static void WINAPI CANIST(LPVOID lpParameter);
    void CANInterruptHandler(UINT32 timeout);
    
    //-------------------------------------
    // CAN Module Index
    //
    UINT32 m_iModuleIndex;

    //-------------------------------------
    // CAN bus critical section
    //
    CRITICAL_SECTION gcsCANBusLock;
    
    //-------------------------------------
    // CAN INTERRUPT HANDLING
    //
    HANDLE m_hCANIST;

    HANDLE hInterrupted;                // Hardware Interrupt Occurence Event (Shared)

    HANDLE hRevEvent[64];
    HANDLE m_hCANIntrEvent;             // Interrupt Occurence Event (Shared)

    volatile 
    BOOL   bTerminateISTThread;         // Boolean flag used to allow the IST
                                        // thread to gracefully terminate.
    //-------------------------------------
    // CAN PROPERTIES
    //

    WORD wLastClkRate;                  // Last Known Frequency
    INT iResult;                        // Last Known Result
    DWORD index;


    //-------------------------------------
    // CAN TRANSACTION FIELDS   
    //

    PCSP_CAN_REG pCANreg;           // CAN Register Base Mapped Virtual Address
    DWORD dwSysIntr;                // System Interrupt ID
    
public:
    CEDEVICE_POWER_STATE m_dxCurrent;

};


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

extern "C" BOOL   BSPCANClockConfig( DWORD index,IN BOOL Enable );
extern "C" void   BSPCANConfigureGPIO(DWORD index);
extern "C" DWORD  BSPGetCANBaseRegAddr(DWORD index);
extern "C" DWORD  BSPGetCANIRQ(DWORD index);
extern "C" DWORD  BSPSETCANIOMUX(DWORD index);
extern "C" void   BSPSetCanPowerEnable(DWORD index,BOOL Enable);

#ifdef __cplusplus
}
#endif

#endif   // _INC_CAN_H

