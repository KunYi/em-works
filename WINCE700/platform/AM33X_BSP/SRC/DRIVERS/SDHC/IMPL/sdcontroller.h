/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: sdcontroller.h
//
//

#ifndef __SDCONTROLLER_H
#define __SDCONTROLLER_H

#include "sdhc.h"
#include "am33x_base_regs.h"
#include "am33x_config.h"
#include "bsp_cfg.h"

// #define MMCHS1_VDDS_WORKAROUND

#define MMC_NO_GPIO_CARD_WP            (-1)     // no GPIO write-protect pin
#define MMC_NO_GPIO_CARD_WP_STATE      (-1)     // no GPIO write-protect pin state
#define MMC_NO_GPIO_CARD_DETECT        (-1)     // no GPIO card detect pin
#define MMC_NO_GPIO_CARD_DETECT_STATE  (-1)     // no GPIO card detect pin state

class CSDIOController : public CSDIOControllerBase
{
public:
    CSDIOController();
    ~CSDIOController();
       
protected:
    BOOL IsWriteProtected();
    BOOL SDCardDetect();
    DWORD SDHCCardDetectIstThreadImpl();
    virtual BOOL InitializeHardware();
    virtual void DeinitializeHardware();
    virtual VOID TurnCardPowerOn();
    virtual VOID TurnCardPowerOff();
    virtual VOID PreparePowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler);
    virtual VOID PostPowerChange(CEDEVICE_POWER_STATE curPowerState, BOOL bInPowerHandler);

    VOID    SetCDPolarity();
    VOID    EnableSDHCCDInterrupts();
    VOID    DisableSDHCCDInterrupts();
    inline  void  Write_MMC_CD_STAT( DWORD wVal );

    BOOL InitializeCardDetect();
    BOOL DeInitializeCardDetect();
    BOOL InitializeWPDetect(void );
    BOOL DeInitializeWPDetect(void);

    BOOL InitializeInterrupt(void);
    
    DWORD    m_dwCDIntrId;
    DWORD    m_dwCDSysintr;
    SDHCINTRENABLE      eSDHCCDIntr;    // for Netra only.  Netra does not use GPIO for CD.
    
#if 0
#ifdef MMCHS1_VDDS_WORKAROUND 
    OMAP_SYSC_GENERAL_REGS *m_pSyscGeneralRegs;
#endif
#endif
};

#endif // __SDCONTROLLER_H

