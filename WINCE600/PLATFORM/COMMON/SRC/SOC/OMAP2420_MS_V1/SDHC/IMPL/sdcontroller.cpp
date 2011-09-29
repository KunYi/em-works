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

// Copyright (c) 2002 BSQUARE Corporation.  All rights reserved.
// DO NOT REMOVE --- BEGIN EXTERNALLY DEVELOPED SOURCE CODE ID 40973--- DO NOT REMOVE
//#pragma optimize("",off)                          // debug 
#include "SDController.h"

#include <nkintr.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <twl.h>

const GUID DEVICE_IFC_I2C_GUID;

CSDIOController::CSDIOController()
:   CSDIOControllerBase()
,   m_Menelaus(*new CMenelaus(I2C_DEVICE_NAME, I2C_MENELAUS_ADDRESS, I2C_MENELAUS_ADDRSIZE))
,   m_pGPIO1Regs(NULL)
,   m_pGPIO4Regs(NULL)
{
}

CSDIOController::~CSDIOController()
{
    delete &m_Menelaus;
}

// Is this card write protected?
BOOL CSDIOController::IsWriteProtected()
{
    if (m_Menelaus.get_Slot1State())
    {
        BOOL state = (INREG32(&m_pGPIO1Regs->ulGPIO_DATAIN) & 
                     (1 << m_dwCardReadWriteSlot1GPIO % sizeof(UINT32)))
                     && m_dwCardWriteProtectedState ? TRUE : FALSE;
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOController::IsWriteProtected: "
            L"WriteProtect State Slot1 %d\r\n", state
        ));
        return state;
    } 
    else if (m_Menelaus.get_Slot2State())
    {
        BOOL state = (INREG32(&m_pGPIO4Regs->ulGPIO_DATAIN) & 
                     (1 << m_dwCardReadWriteSlot2GPIO % sizeof(UINT32)))
                     && m_dwCardWriteProtectedState ? TRUE : FALSE;
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOController::IsWriteProtected: "
            L"WriteProtect State Slot2 %d\r\n", state
        ));
        return state;
    }
    else
        return FALSE;
}

// Is the card present?
BOOL CSDIOController::SDCardDetect()
{
    
    if (m_Menelaus.get_Slot1State() || m_Menelaus.get_Slot2State())
    {
        return TRUE;                            // card inserted into Slot1 or Slot2
    }
    else
    {
        return FALSE;                           // card not inserted into either slot
    }
}

VOID CSDIOController::TurnCardPowerOn()
{
    if (m_Menelaus.get_Slot1State())
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOController::TurnCardPowerOn: "
            L"TurnCardPowerOn & Select- Slot1\r\n"));
        m_Menelaus.HandleSlot1Insert();
    } 
    else if (m_Menelaus.get_Slot2State())
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOController::TurnCardPowerOn: "
            L"Select - Slot2\r\n"));
        m_Menelaus.HandleSlot2Insert();
    }
    else
    {
        DEBUGMSG(SDCARD_ZONE_INFO, (L"CSDIOController::TurnCardPowerOn: "
            L"TurnCardPowerOn - Slot ? \r\n"));
    }
        

}

VOID CSDIOController::TurnCardPowerOff()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CSDIOController::TurnCardPowerOff\r\n"));
    m_Menelaus.SlotsEnabled();      
}

BOOL CSDIOController::InitializeHardware()
{
    PHYSICAL_ADDRESS pa;

    m_Menelaus.MenelausInit();


    // map gpio memory space
    pa.QuadPart = OMAP2420_GPIO1_REGS_PA;
    m_pGPIO1Regs = (OMAP2420_GPIO_REGS *) MmMapIoSpace(pa, sizeof(OMAP2420_GPIO_REGS), FALSE);
    if (m_pGPIO1Regs == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOController::InitializeHardware: "
            L"Failed to map GPIO1 registers\r\n"
        ));
        return FALSE;
    }

    pa.QuadPart = OMAP2420_GPIO4_REGS_PA;
    m_pGPIO4Regs = (OMAP2420_GPIO_REGS *) MmMapIoSpace(pa, sizeof(OMAP2420_GPIO_REGS), FALSE);
    if (m_pGPIO4Regs == NULL)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CSDIOController::InitializeHardware: "
            L"Failed to map GPIO4 registers\r\n"
        ));
        return FALSE;
    }


    Sleep(1000);
    return TRUE;
}

void CSDIOController::DeinitializeHardware()
{
    m_Menelaus.MenelausDeinit();

    MmUnmapIoSpace((VOID*)m_pGPIO1Regs, sizeof(OMAP2420_GPIO_REGS));
    MmUnmapIoSpace((VOID*)m_pGPIO4Regs, sizeof(OMAP2420_GPIO_REGS));
}


///////////////////////////////////////////////////////////////////////////////
//  SDHCCardDetectIstThreadImpl - card detect IST thread for driver
//  Input:  lpParameter - pController - controller instance
//  Output: 
//  Return: Thread exit status
//  Notes:      This platform has the odd behavior of
//              generating multiple interrupts for every
//              insertion and removal event (interrupt line is bouncing).  Consequently, it is 
//              necessary to use some time-out logic in order to 
//              correctly determine if a card is inserted into the
//              I/O slot.
///////////////////////////////////////////////////////////////////////////////
DWORD CSDIOController::SDHCCardDetectIstThreadImpl()
{
    DWORD  dwWaitStatus = WAIT_TIMEOUT;     // wait status
    DWORD dwWaitTime    = 10000;            // INFINITE;

    if (!CeSetThreadPriority(GetCurrentThread(), m_dwCDPriority )) {
        DEBUGMSG(SDCARD_ZONE_WARN, (L"SDHCCardDetectIstThread: warning, failed to set CEThreadPriority\r\n"));
    }

    // check for the card already inserted when the driver is loaded
    SetEvent( m_hCardDetectEvent );
    
    while (TRUE) {
        dwWaitStatus = WaitForSingleObject(m_hCardDetectEvent, dwWaitTime);
    
        if (m_fDriverShutdown) {
            break;
        }

        if( WAIT_TIMEOUT == dwWaitStatus )
        {
            continue;
        }

        // Delay to ensure the spurrious interrupts are done. 
        Sleep( 250 );

        DEBUGMSG(SDCARD_ZONE_INIT, (L"SDHCardDetectIstThread: calling CSDHCSlot::HandleCardDetectInterrupt\r\n"));

        m_Menelaus.UpdateSlotsState();
    
        HandleCardDetectInterrupt();

        m_Menelaus.AckInterrupts();

    }

    DEBUGMSG(SDCARD_ZONE_INIT, (L"SDHCCardDetectIstThread: Thread Exiting\r\n"));

    return 0;
}


BOOL CSDIOController::GetRegistrySettings( CReg *pReg )
{
    if( !CSDIOControllerBase::GetRegistrySettings( pReg ) )
    {
        return FALSE;
    }
    
    m_dwCardReadWriteSlot1GPIO = pReg->ValueDW(SHC_SLOT1_WRITEPROTECT_KEY, 1);
    m_dwCardReadWriteSlot2GPIO = pReg->ValueDW(SHC_SLOT2_WRITEPROTECT_KEY, 96);
    m_dwCardWriteProtectedState = pReg->ValueDW(SHC_CARD_WRITE_PROTECTED_STATE_KEY, 1);
    return TRUE;
}


CSDIOControllerBase *CreateSDIOController()
{
    return new CSDIOController();
}

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE
//#pragma optimize("",on)           // debug
