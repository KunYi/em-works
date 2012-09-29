//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  tve.cpp
//
//  Provides SOC driver implementation for TV encoder chip.
//

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)
#include <math.h>

#include "common_macros.h"
#include "common_tve.h"
#include "tve.h"

//------------------------------------------------------------------------------
// External Functions

extern "C" DWORD CSPTVEGetBaseAddr();
extern "C" DWORD CSPTVEGetIRQ();
extern "C" BOOL  CSPTVESetClockGatingMode(BOOL bTVEClockEnable);
extern "C" BOOL  CSPTVESetClockConfigBaud(DWORD clock_divide_ratio);
//-----------------------------------------------------------------------------
//
// Function: TveClass
//
// TVE class constructor.  Calls TveInit to initialize module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
TveClass::TveClass(void)
{
    TveInit();
}

//-----------------------------------------------------------------------------
//
// Function: ~TveClass
//
// The destructor for TveClass.  Calls TveDeinit to deinitialize.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
TveClass::~TveClass(void)
{
    TveDeinit();
}


//------------------------------------------------------------------------------
//
// Function: TveInit
//
// This function initializes the TVE encoder, including memory mapping, 
// interrupt, etc.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveInit()
{
    BOOL retVal = TRUE;
    DWORD dwIrq;
    PHYSICAL_ADDRESS phyAddr;

    // Memory mapping for TVE registers         
    phyAddr.QuadPart = CSPTVEGetBaseAddr();
    m_pTVE = (PTVE_REGS)MmMapIoSpace(phyAddr, sizeof(PTVE_REGS), FALSE);
    if(m_pTVE == NULL) 
    {   
        ERRORMSG(1, (TEXT("%s: TVE Memory Mapping failed\r\n"), __WFUNCTION__));
        goto cleanUp;
    }   
        
    // Create the TVE interrupt events 
    m_hTveIntrEvent = CreateEvent(NULL, FALSE, FALSE, TVE_INTR_EVENT);
    if (m_hTveIntrEvent == NULL)
    {
        ERRORMSG(1,
            (TEXT("%s: CreateEvent for TVE Interrupt failed\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }
    
    // Create TVE interrupt events
    m_hTveFieldEvent  = CreateEvent(NULL, FALSE, FALSE, TVE_FIELD_EVENT_NAME);
    m_hTveFrameEvent  = CreateEvent(NULL, FALSE, FALSE, TVE_FRAME_EVENT_NAME);
    m_hTveCgmsF2Event = CreateEvent(NULL, FALSE, FALSE, TVE_CGMS_F2_EVENT_NAME);
    m_hTveCgmsF1Event = CreateEvent(NULL, FALSE, FALSE, TVE_CGMS_F1_EVENT_NAME);
    m_hTveCcF2Event   = CreateEvent(NULL, FALSE, FALSE, TVE_CC_F2_EVENT_NAME);
    m_hTveCcF1Event   = CreateEvent(NULL, FALSE, FALSE, TVE_CC_F1_EVENT_NAME);
    m_hTveCdMonEvent  = CreateEvent(NULL, FALSE, FALSE, TVE_CD_MON_EVENT_NAME);
    m_hTveCdSmEvent   = CreateEvent(NULL, FALSE, FALSE, TVE_CD_SM_EVENT_NAME);
    m_hTveCdLmEvent   = CreateEvent(NULL, FALSE, FALSE, TVE_CD_LM_EVENT_NAME);

    if ((m_hTveFieldEvent == NULL)  || (m_hTveFrameEvent == NULL) || (m_hTveCgmsF2Event == NULL) ||
        (m_hTveCgmsF1Event == NULL) || (m_hTveCcF2Event == NULL)  || (m_hTveCcF1Event == NULL)   ||
        (m_hTveCdMonEvent == NULL)  || (m_hTveCdSmEvent == NULL)  || (m_hTveCdLmEvent == NULL))
    {
        ERRORMSG(1, (TEXT("%s: Create events for TVE HW interrupt failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp; 
    }
    
    // Get kernel to translate IRQ -> System Interrupt ID
    dwIrq = CSPTVEGetIRQ();
    if(!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(DWORD), &dwSysIntr, sizeof(DWORD), NULL)) 
    {   
        ERRORMSG(1, (TEXT("%s: Request SYSINTR failed (TVE Interrupt)!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }

    if(!InterruptInitialize(dwSysIntr, m_hTveIntrEvent, NULL, 0)) 
    {
        ERRORMSG(1, (TEXT("%s: Interrupt initialization failed (TVE Interrupt)!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }

    
    return retVal;

cleanUp:
    TveDeinit();
    
    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TveEnable
//
// This function enables the TVE encoder.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveEnable()
{   
    BOOL retVal = TRUE;
    
    // The following settings are more related TVE internal working mode. 
    // So we don't ask customers to configure them.
    // Here are default settings. You can change to other working modes here if necesary.
    // 
    TVE_CABLE_DETECTION_MODE tveCDMode  = TVE_CD_TRIGGER_IN_FUNC; 
    
    // To save power, enable TVE Clock Gating only when TV is enabled.
    retVal = CSPTVESetClockGatingMode(TRUE);
    if(retVal != TRUE) 
    {
        ERRORMSG(1, (TEXT("%s: TVE clock gating failed\r\n"), __WFUNCTION__));
        TVE_FUNCTION_EXIT();
        goto cleanUp;
    } 
 
    // Create IST thread to receive hardware interrupts
    bIntrThreadLoop = TRUE;
    hTveIST = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TveIST, NULL, 0, NULL);
    if(hTveIST == NULL) 
    {
        ERRORMSG(1, (TEXT("%s: Create thread for TVE interrupt failed!\r\n"),  __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }
    
    // TVE Clock Setup. By default, we set 216MHz for a normal TVE mode.
    DWORD tveClock = TVE_S216_CLK;
    TveSetClock(tveClock);
    
    TVE_TVDAC_SAMPLING_RATE sampleRate = TveGetTVDACSampleRateByClock(tveClock);
    if (sampleRate == SAMPLING_RATE_NONE)
    {
        ERRORMSG(1, (_T("Invaild sample rate!\r\n")));
        retVal = FALSE;
        goto cleanUp;
    }
        
    // Setup TVE common register 
    if (!TveConfigureCommonReg(m_eTVStd, m_eTVOutputMode, sampleRate))
    {
        ERRORMSG(1, (_T("TVEConfigureCommonReg failed!\r\n")));
        retVal = FALSE;
        goto cleanUp;
    }
    
    // Set TVE cable detection using TVE_CD_TRIGGER_IN_FUNC as a default setting.
    if (!TveSetCableDetection(tveCDMode, m_eTVStd, m_eTVOutputMode))
    {
        ERRORMSG(1, (TEXT("%s: Set TVE Cable Detection failed!\r\n"), __WFUNCTION__));
        retVal = FALSE;
        goto cleanUp;
    }
    
    
#if 0  
    // FIXME: TVE HW has a bug to detect the cables
    
    // Check if the cables are connected as we expected
    if (!TveIsCableDetected(m_eTVOutputMode))
    {
        DEBUGMSG(1, (TEXT("%s: TVE Cable is not connected!\r\n"), __WFUNCTION__));
        retVal = FALSE;
    }
#endif
    
    // Setup the cables detect directly
    OUTREG32(&m_pTVE->STAT_REG, 0x010777F1);                             
    SETREG32(&m_pTVE->INT_CONT_REG, CSP_BITFVAL( TVE_CD_LM_IEN, TVE_CD_LM_IEN_ENABLE) |
                                    CSP_BITFVAL( TVE_CD_SM_IEN, TVE_CD_SM_IEN_ENABLE));
    
    // TVE initialization
    OUTREG32(&m_pTVE->RESERVED_REG_4, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_3, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_2, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_1, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_0, 0x00000000);
    OUTREG32(&m_pTVE->RESERVED_REG_5, 0x00000000);
    
    
    // TveDumpRegs();
    
    return retVal;
    
cleanUp:
    TveDeinit();
    return retVal;
}


//------------------------------------------------------------------------------
//
// Function: TveDisable
//
// This function disable the TVE out encoder.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveDisable()
{
    // Give the IST thread a chance to perform any cleanup.
    if (hTveIST) 
    {
        bIntrThreadLoop = FALSE;
        SetEvent(m_hTveIntrEvent);
        CloseHandle(hTveIST);
        hTveIST = NULL;
    }
    
    // Clear up interrupt/CD control registers and stop TV encoder
    if(m_pTVE != 0) 
    {
        OUTREG32(&m_pTVE->INT_CONT_REG, 0);
        OUTREG32(&m_pTVE->CD_CONT_REG, 0);
    
        // Disable the TVE power to stop TV encoder
        OUTREG32(&m_pTVE->COM_CONF_REG, 0);
    }

    // Disable TVE Clock Gating
    CSPTVESetClockGatingMode(FALSE); 
    
    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: TveDeinit
//
// This function deinitializes the TVE out encoder.
//
// Parameters:
//      None
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TveClass::TveDeinit()
{
   
    if(m_hTveIntrEvent)
    {
        CloseHandle(m_hTveIntrEvent); 
        m_hTveIntrEvent = NULL;
    }
    
    if (m_hTveFieldEvent)
    {
        CloseHandle(m_hTveFieldEvent);
        m_hTveFieldEvent = NULL;
    }
    
    if (m_hTveFrameEvent)
    {
        CloseHandle(m_hTveFrameEvent);
        m_hTveFrameEvent = NULL;
    }
    
    if (m_hTveCgmsF2Event)
    {
        CloseHandle(m_hTveCgmsF2Event);
        m_hTveCgmsF2Event = NULL;
    }
    
    if (m_hTveCgmsF1Event)
    {
        CloseHandle(m_hTveCgmsF1Event);
        m_hTveCgmsF1Event = NULL;
    }
    
    if (m_hTveCcF2Event)
    {
        CloseHandle(m_hTveCcF2Event);
        m_hTveCcF2Event = NULL;
    }
    
    if (m_hTveCcF1Event)
    { 
        CloseHandle(m_hTveCcF1Event);
        m_hTveCcF1Event = NULL;
    }
    
    if (m_hTveCdMonEvent)
    { 
        CloseHandle(m_hTveCdMonEvent);
        m_hTveCdMonEvent = NULL;
    }
    
    if (m_hTveCdSmEvent)
    {
        CloseHandle(m_hTveCdSmEvent);
        m_hTveCdSmEvent = NULL;
    }
    
    if (m_hTveCdLmEvent)
    {
        CloseHandle(m_hTveCdLmEvent);
        m_hTveCdLmEvent = NULL;
    }
    
    // Disable TVE
    TveDisable();
    
    // Do unmap memory
    if(m_pTVE != 0) 
    {
        MmUnmapIoSpace((PTVE_REGS)m_pTVE, sizeof(PTVE_REGS));
        m_pTVE = NULL;
    }
    
    return TRUE;
}



//-----------------------------------------------------------------------------
//
//  Function: TveConfigureCommonReg
//
//  This function is to configure the TVE common configuration register. 
//
//  Parameters:
//
//     tv_stand
//           [in] tv standard format (NTSC, PALM, PALN, PAL, SECAM) to set
//     
//     tv_out_mode
//           [in] tv output mode (TVE's 8 output modes, including composite, svideo, 
//                componment with channel combinations ) to set
//     sampling_rate
//           [in] Set TVE sampling rate
//
//  Returns:
//     TRUE if sucessful, otherwise return FALSE.
//
//-----------------------------------------------------------------------------
BOOL TveClass::TveConfigureCommonReg(TVE_TV_STAND tv_stand, TVE_TV_OUT_MODE tv_out_mode, TVE_TVDAC_SAMPLING_RATE sampling_rate)
{
    int data=0;
    
    // Enable the Common Configuration Register
    data = CSP_BITFVAL( TVE_TVE_EN, TVE_TVE_EN_ENABLE)                  |
           CSP_BITFVAL( TVE_TVDAC_SAMP_RATE, sampling_rate)             |  // The nominal sampling rate of the TVDAC is 
                                                                           // 216 MHz, 16x output oversampling
           CSP_BITFVAL( TVE_IPU_CLK_EN, TVE_IPU_CLK_EN_ENABLE)          |
           CSP_BITFVAL( TVE_TV_OUT_MODE, tv_out_mode)                   |  // TV Output mode (TV_OUT_DISABLE,
                                                                           // TV_OUT_COMPOSITE_CH0,.., TV_OUT_COMPONMENT_RGB) 
           CSP_BITFVAL( TVE_TV_STAND, tv_stand)                         |  // TV standards (NTSC, PALM, PALN, PAL, SECAM)       
           CSP_BITFVAL( TVE_YC_DELAY, TVE_YC_DELAY_ENABLE)              |  // Workaround for Errate ENGcm02631      
           CSP_BITFVAL( TVE_CHROMA_BW, TVE_CHROMA_BW_1P3MHZ)            |  // Chroma bandwidth control. 1.3MHz as a default
           CSP_BITFVAL( TVE_VBI_CHROMA_EN, TVE_VBI_CHROMA_EN_DISABLE)   |  // VBI chroma disable
           CSP_BITFVAL( TVE_SCH_PHASE, 0);
          
    
    // Sync on Read/Green/Blue enable when using RGB
    if (tv_out_mode == TV_OUT_COMPONENT_RGB)
    {
        data |= CSP_BITFVAL( TVE_R_SYNC_EN, TVE_R_SYNC_EN_ENABLE)       |  // Sync on red enable. Valid in RGB mode
                CSP_BITFVAL( TVE_G_SYNC_EN, TVE_G_SYNC_EN_ENABLE)       |  // Sync on green enable. Valid in RGB mode
                CSP_BITFVAL( TVE_B_SYNC_EN, TVE_B_SYNC_EN_ENABLE);         // Sync on blue enable. valid in RGB mode 
    }
    else
    {
        data |= CSP_BITFVAL( TVE_R_SYNC_EN, TVE_R_SYNC_EN_DISABLE)      |  // Sync on red enable. Valid in RGB mode
                CSP_BITFVAL( TVE_G_SYNC_EN, TVE_G_SYNC_EN_DISABLE)      |  // Sync on green enable. Valid in RGB mode
                CSP_BITFVAL( TVE_B_SYNC_EN, TVE_B_SYNC_EN_DISABLE);        // Sync on blue enable. valid in RGB mode 
    }
    
    // Pedestal (setup) enable when using NTSC and PAL-M
    if ((tv_stand == TV_STAND_NTSC) || (tv_stand == TV_STAND_PALM))
    {
        data |= CSP_BITFVAL( TVE_PED_EN, TVE_PED_EN_ENABLE);               // This bit is valid for NTSC and PAL-M 
                                                                           // only and is masked for all the other standards.            
    }
    else
    {
        data |= CSP_BITFVAL( TVE_PED_EN, TVE_PED_EN_DISABLE);             
               
    }
    
    // SECAM field identificator enable if using SECAM
    if (tv_stand == TV_STAND_SECAM)
    {
        data |= CSP_BITFVAL( TVE_SECAM_FID_EN, TVE_SECAM_FID_EN_ENABLE)  |  
                CSP_BITFVAL( TVE_PHASE_RST_EN, TVE_PHASE_RST_EN_DISABLE);        // disable for SECAM
    }
    else
    {
        data |= CSP_BITFVAL( TVE_SECAM_FID_EN, TVE_SECAM_FID_EN_DISABLE) |
                CSP_BITFVAL( TVE_PHASE_RST_EN, TVE_PHASE_RST_EN_ENABLE);        // NTSC and PAL only
    }
     
     
    // reset the COM_CONF_REG register with the new settings
    OUTREG32(&m_pTVE->COM_CONF_REG, data);
    
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: TveSetClock
//
//  This function sets TVE clock.
//
//  Parameters:
//     clock_frequency
//           [in] clock frequency to set
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL TveClass::TveSetClock(DWORD clock_frequency)
{
    DWORD clock_divide_ratio;
    
    clock_divide_ratio = TVE_S216_CLK / clock_frequency - 1;
            
    if (!CSPTVESetClockConfigBaud(clock_divide_ratio))
    {
        DEBUGMSG(1, (TEXT("%s: CSPTVESetClockConfigBaud failed !\r\n"), __WFUNCTION__));
        return FALSE;
    }
    return TRUE;   
}


//------------------------------------------------------------------------------
//
//  Function:  TveIST
//
//  This function is the interrupt thread to handle TVE interrupts
//  
//  Parameters:  
//      LPVOID lpParameter
//
//  Returns:  
//      None
// 
//------------------------------------------------------------------------------
void TveClass::TveIST(LPVOID lpParameter)
{
    UNREFERENCED_PARAMETER(lpParameter);
#if 0
    DWORD statReg;
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParameter);

    CeSetThreadPriority(GetCurrentThread(), 97);

    while (bIntrThreadLoop)
    {
        WaitForSingleObject(g_hTveIntrEvent, INFINITE);  
        
        if(!bIntrThreadLoop)
            break;
            
        statReg = INREG32(&m_pTVE->STAT_REG);
        //RETAILMSG(1, (_T("TVE status register is %x \r\n"), statReg));
   
        if (statReg & 0x02000000) // field end
        {
            //clear interrupt
            SETREG32(&m_pTVE->STAT_REG, 0x02000000);
            SetEvent(m_hTveFieldEvent);
        }
   
        if (statReg & 0x01000000) // frame end
        {
            //clear interrupt
            SETREG32(&m_pTVE->STAT_REG, 0x01000000);
            SetEvent(m_hTveFrameEvent);
        }
    
        if (statReg & 0x00000080) //CGMS F2 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000080);
           SetEvent(m_hTveCgmsF2Event);
        }
        
        if (statReg & 0x00000040) //CGMS F1 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000040);
           SetEvent(m_hTveCgmsF1Event);
        }
        
        if (statReg & 0x00000020) //CC F2 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000020);
           SetEvent(m_hTveCcF2Event);
        }
        
        if (statReg & 0x00000010) //CC F1 done
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000010);
           SetEvent(m_hTveCcF1Event);
        }
        
        if (statReg & 0x00000004) //CD Monitoring end
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000004);
           SetEvent(m_hTveCdMonEvent);
        }
        
        if (statReg & 0x00000004) //CD Short detected
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000004);
           SetEvent(m_hTveCdMonEvent);
        }
        
        if (statReg & 0x00000002) //CD Short detected
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000002);
           SetEvent(m_hTveCdSmEvent);
        }
        
        if (statReg & 0x00000001) //CD Cable detected
        {
           // clear interrupt
           SETREG32(&m_pTVE->STAT_REG, 0x00000001);
           SetEvent(m_hTveCdLmEvent);
        }
        
   
        // Kernel call to unmask the interrupt so that it can be signalled again
        InterruptDone(dwSysIntr);
    }
#endif
    return;
}



void TveClass::TveDumpRegs()
{
    int i;
    
    UINT32* ptr = (UINT32 *)&m_pTVE->COM_CONF_REG;
    RETAILMSG (1, (TEXT("\n\nDump TVE Registers \n")));
    for (i = 0; i < 3; i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), 
                      (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
   
}
  
