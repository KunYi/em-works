//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pmupdk.cpp
//
//  This file contains a power Lower Level interface for the drivers who need to config power.
//
//-----------------------------------------------------------------------------
    
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"
#include "regspower.h"
#include "pmu_ioctl.h"
#include "pmu.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define POWER_OFF_HOLD_TIME             2000000

/* CS&ZHL SEP24-2012: defined board type */
#define	BOARD_TYPE_EM9280				9280
#define	BOARD_TYPE_EM9283				9283

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

DBGPARAM dpCurSettings = {
    _T("PMU"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN // ulZoneMask
};

#endif  // DEBUG

//lqk:Jul-25-2012
#define BATTERY_LOW                     2400
#define BATTERY_HIGH                    4300

//-----------------------------------------------------------------------------
// Local Variables

static PVOID pv_HWregPOWER = NULL;
static PVOID pv_HWregUSBPhy0 = NULL;
static PVOID pv_HWregUSBPhy1 = NULL;
static PVOID pv_HWregCLKCTRL = NULL;
static PVOID pv_HWregDIGCTL = NULL;
static PVOID pv_HWregRTC = NULL;

static BOOL b4p2Enabled = TRUE;

static UINT32 SysIntr5V = 0;
static UINT32 SysIntrVDDD = 0;
static UINT32 SysIntrVDDIO = 0;
static UINT32 SysIntrVDDA = 0;

static BOOL  bOrinVbusvalid_5Vdetect = FALSE;
static BOOL  bOrinPwnBo = FALSE;
static BOOL  bOrinVDDIOBo = FALSE;
static BOOL  bOrinVDDDBo = FALSE;
static BOOL  bOrinVDDABo = FALSE;

static HANDLE g_h5VIntEvent = NULL;
static LPTSTR g_5VDetectEventName = TEXT("5V_Detect");
static HANDLE g_h5VIntEventNotify = NULL;

static HANDLE g_hVDDDBNOTEvent = NULL;
static HANDLE g_hVDDIOBNOTEvent = NULL;
static HANDLE g_hVDDABNOTEvent = NULL;

static HANDLE IrqHandler_thread = NULL;
static HANDLE PowerHandler_thread = NULL;
static HANDLE VDDDBOHandler_thread = NULL;
static HANDLE VDDIOBOHandler_thread = NULL;
static HANDLE VDDABOHandler_thread = NULL;

static BOOL bPLL = FALSE;
static BOOL sb_Vbus5v = FALSE;
//-----------------------------------------------------------------------------
// Local Functions
static BOOL PowerIRQHandler(void);
static BOOL VDDDBOHandler(void);
static BOOL VDDIOBOHandler(void);
static BOOL VDDABOHandler(void);
static BOOL PowerInitHandle(void);

static BOOL    PowerInit(VOID);
static BOOL    PowerDeInit(VOID);
static BOOL    PowerAlloc(VOID);
static BOOL    PowerDealloc(VOID);

static void    PmuIoctlInitBatteryMonitor(VOID);
static BOOL    PmuIoctlGetBatteryChargingStatus(VOID);
static UINT32  PmuIoctlGetBatteryVoltage(VOID);
static BOOL    PmuIoctlSetCharger(UINT32 current);
static VOID    PmuIoctlStopCharger(VOID);
static VOID    PmuIoctlSetFets(PMU_POWER_FETSSET bFetsMode);
static PMU_POWER_SUPPLY_MODE PmuIoctlGet5vPresentFlag(VOID);
static VOID    PowerExecuteBatteryTo5VoltsHandoff(VOID);
static VOID    PowerEnable5VoltsToBatteryHandoff(VOID);
static VOID    PowerExecute5VoltsToBatteryHandoff(VOID);
static VOID    PowerEnableBatteryTo5VoltsHandoff(VOID);
static PMU_POWER_SUPPLY_MODE    PowerGet5vPresentFlag(VOID);
static UINT32  PowerInitPowerSupplies(VOID);
static VOID    PowerClear5VIrq();
static BOOL    PowerGetLimit();
static BOOL    PowerGetUSBPhy();
static void    PowerInitCpuClock(void);
static VOID    PowerOffChip(void);
static VOID    DumpPowerRegisters(void);
static UINT32  PowerSetVdddPowerSource(POWER_POWERSOURCE eSource);
static UINT32  PowerSetVddioPowerSource(POWER_POWERSOURCE eSource);
static UINT32  PowerSetVddaPowerSource(POWER_POWERSOURCE eSource);
static UINT32  PowerStart4p2(VOID);
static BOOL    PowerStart4p2HW(VOID);
static VOID    PowerStop4p2(VOID);
static POWER_LINREGOFFSETSTEP PowerGetVdddLinRegOffset(VOID);
static UINT32  PowerConvertSettingToVddd(UINT32 Setting);
static POWER_POWERSOURCE    PowerGetVdddPowerSource(VOID);
static VOID    PowerSetVdddBrownoutValue(UINT32 VdddBoOffsetmV);
static VOID    PowerSetVdddValue(UINT32 VdddmV);
static UINT32  PowerGetVdddValue(VOID);

extern BOOL IsUSBDeviceDriverEnable();
//extern BOOL Is5VFromVbus();

//LQK:Jul-25-2012
static BOOL    IsBatteryAttach(VOID);

BOOL g_bIs5VFromVbus=TRUE;

extern BOOL Is5VFromVbus();

extern DWORD BSPGetBoardType();
extern void  BSPSetVFromVbusValue( BOOL bVal );

//-----------------------------------------------------------------------------
//
//  Function:  PowerAlloc
//
//  This function Maps the register mapping
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerAlloc(VOID)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    if (pv_HWregPOWER == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_POWER;

        // Map peripheral physical address to virtual address
        pv_HWregPOWER = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregPOWER == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace pv_HWregPOWER failed!\r\n")));
            goto cleanUp;
        }
    }
    
    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;

        // Map peripheral physical address to virtual address
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregUSBPhy0 == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace pv_HWregUSBPhy0 failed!\r\n")));
            goto cleanUp;
        }
    }

    if (pv_HWregDIGCTL == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_DIGCTL;

        // Map peripheral physical address to virtual address
        pv_HWregDIGCTL = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregDIGCTL == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace pv_HWregDIGCTL failed!\r\n")));
            goto cleanUp;
        }
    }

    if (pv_HWregCLKCTRL == NULL)
    {
         phyAddr.QuadPart = CSP_BASE_REG_PA_CLKCTRL;
        
         // Map peripheral physical address to virtual address
         pv_HWregCLKCTRL = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregCLKCTRL == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace pv_HWregCLKCTRL failed!\r\n")));
            goto cleanUp;
        }        
    }

    if (pv_HWregRTC == NULL)
    {
         phyAddr.QuadPart = CSP_BASE_REG_PA_RTC;
        
         // Map peripheral physical address to virtual address
         pv_HWregRTC = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregRTC == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace pv_HWregRTC failed!\r\n")));
            goto cleanUp;
        }        
    }

    rc = TRUE;
    
cleanUp:
    if (!rc) 
    {
        PowerDealloc();
    }
    
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerDealloc
//
//  This function Unmaps the register mapping
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerDealloc(VOID)
{
    // Unmap peripheral registers
    if (pv_HWregPOWER)
    {
        MmUnmapIoSpace(pv_HWregPOWER, 0x1000);
        pv_HWregPOWER = NULL;
    }

    if (pv_HWregUSBPhy0)
    {
        MmUnmapIoSpace(pv_HWregUSBPhy0, 0x1000);
        pv_HWregUSBPhy0 = NULL;
    }

    if (pv_HWregDIGCTL)
    {
        MmUnmapIoSpace(pv_HWregDIGCTL, 0x1000);
        pv_HWregDIGCTL = NULL;
    }

    if (pv_HWregCLKCTRL)
    {
        MmUnmapIoSpace(pv_HWregCLKCTRL, 0x1000);
        pv_HWregCLKCTRL = NULL;
    }    

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: PowerStart4p2
//
//  This routine initializes the 4p2 enabled power supply.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns the status.
//
//-----------------------------------------------------------------------------
static UINT32 PowerStart4p2(VOID)
{
    UINT32 Status;

    if(HW_POWER_STS.B.VBUSVALID0)
    {
        Status = PowerStart4p2HW();
    }
    else
    {   
        // for 4p2 mode enabled, the os level driver will control the handoff
        // from DCDC battery to DCDC 4p2 on a 5V connection.  We do not
        // want automatic handoffs to linear regulator.

        // Enable Dcdc 
        BF_SET(POWER_5VCTRL, ENABLE_DCDC);
        
        // For LiIon battery, all the rails can start off as DCDC mode.
        if((Status = PowerSetVdddPowerSource(POWER_DCDC_LINREG_READY)) != 0)
        {
            return Status;
        }
        if((Status = PowerSetVddaPowerSource(POWER_DCDC_LINREG_READY)) != 0)
        {
            return Status;
        }
        if((Status = PowerSetVddioPowerSource(POWER_DCDC_LINREG_READY)) != 0)
        {
            return Status;
        }
    }

    if(Status==0)
    {
        // This is a logical, not physical state.  Todo: possibly re-work this
        b4p2Enabled = TRUE;
    }
    else
    {
        b4p2Enabled = FALSE;
    }

    return Status;
}
//-----------------------------------------------------------------------------
//
//  Function: PowerStart4p2
//
//  This routine initializes the 4p2 enabled power supply.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful otherwise FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerStart4p2HW(VOID)
{
    UINT32		i = 0;
    UINT32		u32Chargetime = 0;
    UINT32		u32Vbusvalid_trsh = 0;
    BOOL		bLoop = TRUE;   
	DWORD		dwBoardType = BSPGetBoardType();     /* CS&ZHL SEP24-2012 */
    

    //Must do these things to avoid the the glitchs ,due to chip bug
    //record the value 
    bOrinVbusvalid_5Vdetect = HW_POWER_5VCTRL.B.VBUSVALID_5VDETECT;
    u32Vbusvalid_trsh       = HW_POWER_5VCTRL.B.VBUSVALID_TRSH;
    bOrinPwnBo              = HW_POWER_MINPWR.B.PWD_BO;
    bOrinVDDIOBo            = HW_POWER_CTRL.B.ENIRQ_VDDIO_BO;
    bOrinVDDDBo             = HW_POWER_CTRL.B.ENIRQ_VDDD_BO;
    bOrinVDDABo             = HW_POWER_CTRL.B.ENIRQ_VDDA_BO;

    BF_CLR(POWER_CTRL, POLARITY_VBUSVALID);

    while(bLoop)
    {       
        //enable the VDDD,VDDIO,VDDA power down.
        BF_CLR(POWER_VDDDCTRL,PWDN_BRNOUT);    
        BF_CLR(POWER_VDDACTRL,PWDN_BRNOUT);     
        BF_CLR(POWER_VDDIOCTRL,PWDN_BRNOUT);  
        
        //disable the VDDD VDDA VDDIO BO interrupt
        BF_CLR(POWER_CTRL,ENIRQ_VDDD_BO);
        BF_CLR(POWER_CTRL,ENIRQ_VDDA_BO);    
        BF_CLR(POWER_CTRL,ENIRQ_VDDIO_BO); 

        //Power VBUS CMP
        HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_PWRUP_VBUS_CMPS);
        
        //Set the vbusvalid
        HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_VBUSVALID_5VDETECT);
        HW_POWER_5VCTRL_CLR(BM_POWER_5VCTRL_VBUSVALID_TRSH);//set to 2.9V
        
        //Clear VBUSVALID0 IRQ
        HW_POWER_CTRL_CLR(BM_POWER_CTRL_VBUSVALID_IRQ);
        
        HW_POWER_CTRL_CLR(BM_POWER_CTRL_ENIRQ_VDD5V_DROOP);
        HW_POWER_CTRL_CLR(BM_POWER_CTRL_VDD5V_DROOP_IRQ);
        
        if((PowerSetVdddPowerSource(POWER_DCDC_LINREG_OFF)) != 0)
            return FALSE;
    
        if((PowerSetVddaPowerSource(POWER_DCDC_LINREG_OFF)) != 0)
            return FALSE;
    
        if((PowerSetVddioPowerSource(POWER_DCDC_LINREG_READY)) != 0)
            return FALSE;

        // We must first clear this bit and then set it again due to bug with the
        // hardware.  If the hardware has at anytime seen a 5V removal, 4p2 is
        // powered down internal even though this bit remains set in the registers
        
        BF_SET(POWER_DCDC4P2, ENABLE_4P2);      //enable 5V to 4p2 regulator
        
        // This must be used if there is no static load on 4p2 as 4p2 will
        // become unstable with no load.
        BF_SET(POWER_CHARGE,ENABLE_LOAD);
        
        BF_CLR(POWER_5VCTRL, PWD_CHARGE_4P2);
        
        BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT,0x1);//set limit current to 10mA

        //charge 4P2 capacity to 4.2V
        i = 1;
        while(i <= (BM_POWER_5VCTRL_CHARGE_4P2_ILIMIT >> BP_POWER_5VCTRL_CHARGE_4P2_ILIMIT))
        {
            BF_WR(POWER_5VCTRL,CHARGE_4P2_ILIMIT,i);
            i++;
            u32Chargetime = HW_DIGCTL_MICROSECONDS_RD();
            while(HW_DIGCTL_MICROSECONDS_RD() - u32Chargetime < 100);//waiting 50us
        }

        if(Is5VFromVbus())
        {  
//#ifdef EM9283					//SEP-24-2012: lqk
		 if( dwBoardType == BOARD_TYPE_EM9283 )
			// Set current limit to 450mA.    
            BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x24);
		 else
            // Set current limit to 480mA.    
            BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x27);
//#endif	
        }
        else
        {
            // Set current limit to 780mA.    
            BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x3F);
        }

        //set the DCDC BO to 4.15V
        BF_SETV(POWER_DCDC4P2, BO, 0x16);

        //wait the DCDC to be stable
        while(HW_POWER_STS.B.DCDC_4P2_BO)
            StallExecution(50);

        //enable DCDC 4P2 capability               
        BF_SET(POWER_DCDC4P2, ENABLE_DCDC);      
        //wait the DCDC to be stable
        StallExecution(100);
       
        // enables DCDC during 5V connnection (for both battery or 4p2 powered DCDC)
        // Enable the DCDC.
        BF_SET(POWER_5VCTRL, ENABLE_DCDC);      
        //wait the DCDC to be stable
        StallExecution(100);
    
        if(HW_POWER_CTRL.B.VBUSVALID_IRQ)
            bLoop = TRUE;
        else
            bLoop = FALSE;
    }

    //Restore the 5V detect and the target volt
    BF_WR(POWER_5VCTRL, VBUSVALID_5VDETECT, bOrinVbusvalid_5Vdetect);
    BF_WR(POWER_5VCTRL, VBUSVALID_TRSH, u32Vbusvalid_trsh);
    BF_WR(POWER_MINPWR, PWD_BO, bOrinPwnBo);

    //Clear VBUSVAILD IRQ.  
    BF_CLR(POWER_CTRL,VBUSVALID_IRQ);       
    
    //Restore the VDD*'s BO IRQ stat
    BF_CLR(POWER_CTRL,VDDD_BO_IRQ); 
    BF_CLR(POWER_CTRL,VDDA_BO_IRQ);      
    BF_CLR(POWER_CTRL,VDDIO_BO_IRQ);

    BF_WR(POWER_CTRL, ENIRQ_VDDIO_BO, bOrinVDDIOBo);
    BF_WR(POWER_CTRL, ENIRQ_VDDD_BO, bOrinVDDDBo);
    BF_WR(POWER_CTRL, ENIRQ_VDDA_BO, bOrinVDDABo);

    BF_CLR(POWER_CHARGE,ENABLE_LOAD);
   
    return 0;

}

//-----------------------------------------------------------------------------
//
//  Function:  PowerStop4p2
//
//  This function stops 4p2 functionality, used in i.MX23
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static void PowerStop4p2(void)
{
    // Enable the DCDC.
    BF_CLR(POWER_DCDC4P2, ENABLE_DCDC); 
    
    // Enable DCDC 4P2 regulation circuitry
    BF_CLR(POWER_DCDC4P2, ENABLE_4P2);     
    BF_CLR(POWER_CHARGE,ENABLE_LOAD);
    BF_CLR(POWER_CTRL,ENIRQ_DCDC4P2_BO);
    BF_SET(POWER_5VCTRL, PWD_CHARGE_4P2);
}

//-----------------------------------------------------------------------------
//
//  Function: PowerSetVdddPowerSource
//
//  This routine Sets up the Vddd PowerSource.
//
//  Parameters:
//          eSource
//              [in] power sources for power supply
//
//  Returns:
//          returns 0 if successful otherwise returns error value.
//
//-----------------------------------------------------------------------------
static UINT32 PowerSetVdddPowerSource(POWER_POWERSOURCE eSource)
{
    UINT32 rtn = 0;
       
    // The VDDD can use two power sources in three configurations: the Linear
    // Regulator, the DCDC with LinReg off, and the DCDC with LinReg on.
    // Each has its own configuration that must be set up to
    // prevent power rail instability.
    
    switch(eSource)
    {
        // Power the VDDD rail from DCDC.  LinReg is off.
        case POWER_DCDC_LINREG_OFF:
        case POWER_DCDC_LINREG_READY:
            
            // Use LinReg offset for DCDC mode.
            BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDD DCDC output and turn off the VDDD LinReg output.
            BF_CLR(POWER_VDDDCTRL, DISABLE_FET);
            BF_CLR(POWER_VDDDCTRL, ENABLE_LINREG);
            
            // Make sure stepping is enabled when using DCDC.
            BF_CLR(POWER_VDDDCTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDD rail from DCDC.  LinReg is on.
        case POWER_DCDC_LINREG_ON:
            
            // Use LinReg offset for DCDC mode.
            BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDD DCDC output and turn on the VDDD LinReg output.
            BF_CLR(POWER_VDDDCTRL, DISABLE_FET);
            BF_SET(POWER_VDDDCTRL, ENABLE_LINREG);
            
            // Make sure stepping is enabled when using DCDC.
            BF_CLR(POWER_VDDDCTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDD rail from the linear regulator.  The DCDC is not
        // set up to automatically power the chip if 5V is removed.  Use this
        // configuration when battery is powering the player, but we want to
        // use LinReg instead of DCDC.
        case POWER_LINREG_DCDC_OFF:
            
            // Use LinReg offset for LinReg mode.
            BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_LINREG_MODE);
            
            // Turn on the VDDD LinReg and turn off the VDDD DCDC output.
            BF_SET(POWER_VDDDCTRL, ENABLE_LINREG);
            BF_SET(POWER_VDDDCTRL, DISABLE_FET);
            
            // Make sure stepping is disabled when using linear regulators.
            BF_SET(POWER_VDDDCTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDD rail from the linear regulator.  The DCDC is ready
        // to automatically power the chip when 5V is removed.  Use this
        // configuration when powering from 5V.
        case POWER_LINREG_DCDC_READY:
            
            // Use LinReg offset for LinReg mode.
            BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDD LinReg and turn on the VDDD DCDC output.  The
            // ENABLE_DCDC must be cleared to avoid LinReg and DCDC conflict.
            BF_CLR(POWER_VDDDCTRL, ENABLE_LINREG);
            BF_CLR(POWER_VDDDCTRL, DISABLE_FET);
            
            // Make sure stepping is disabled when using linear regulators.
            BF_SET(POWER_VDDDCTRL, DISABLE_STEPPING);
            
            break;
            
        // Invalid power source option.
        default:
            rtn = 1;
            
            break;
            
    }
       
    return rtn;
}

//-----------------------------------------------------------------------------
//
//  Function: PowerSetVddioPowerSource
//
//  This routine Sets up the Vddio PowerSource.
//
//  Parameters:
//          eSource
//              [in] power sources for power supply
//
//  Returns:
//          returns 0 if successful otherwise returns error value.
//
//-----------------------------------------------------------------------------
static UINT32 PowerSetVddioPowerSource(POWER_POWERSOURCE eSource)
{
    UINT32 rtn = 0;

    // The VDDIO can use two power sources in three configurations: the Linear
    // Regulator, the DCDC with LinReg off, and the DCDC with LinReg on.
    // Each has its own configuration that must be set up to prevent power
    // rail instability. VDDIO will only use the linear regulator when
    // 5V is present.
       
    switch(eSource)
    {
        // Power the VDDIO rail from DCDC with LinReg off.
        case POWER_DCDC_LINREG_OFF:
            
            // Use LinReg offset for DCDC mode.
            BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDIO DCDC output and turn off the LinReg output.
            BF_CLR(POWER_VDDIOCTRL, DISABLE_FET);
            BF_SET(POWER_5VCTRL, ILIMIT_EQ_ZERO);
            
            // Make sure stepping is enabled when using DCDC.
            BF_CLR(POWER_VDDIOCTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDIO rail from DCDC with LinReg on.
        case POWER_DCDC_LINREG_READY:
        case POWER_DCDC_LINREG_ON:
            
            // Use LinReg offset for DCDC mode.
            BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDIO DCDC output and turn on the LinReg output.
            BF_CLR(POWER_VDDIOCTRL, DISABLE_FET);
            BF_CLR(POWER_5VCTRL, ILIMIT_EQ_ZERO);  
            
            // Make sure stepping is enabled when using DCDC.
            BF_CLR(POWER_VDDIOCTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDIO rail from the linear regulator.
        // Assumes 5V is present.
        case POWER_LINREG_DCDC_OFF:
            
            // Use LinReg offset for LinReg mode.
            BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_LINREG_MODE);
            
            // Turn on the VDDIO LinReg output and turn off the VDDIO DCDC output.            
            BF_CLR(POWER_5VCTRL, ILIMIT_EQ_ZERO);            
            BF_SET(POWER_VDDIOCTRL, DISABLE_FET);
            
            // Make sure stepping is disabled when using linear regulators.
            BF_SET(POWER_VDDIOCTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDIO rail from the linear regulator.
        // Assumes 5V is present.  The DCDC is ready to take over when 5V
        // is removed.
        case POWER_LINREG_DCDC_READY:
            
            // Use LinReg offset for LinReg mode.
            BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDIO LinReg output and prepare the VDDIO DCDC output.
            // ENABLE_DCDC must be cleared to prevent DCDC and LinReg conflict.
            BF_CLR(POWER_5VCTRL, ILIMIT_EQ_ZERO);  
            
            BF_CLR(POWER_VDDIOCTRL, DISABLE_FET);
            
            // Make sure stepping is disabled when using linear regulators.
            BF_SET(POWER_VDDIOCTRL, DISABLE_STEPPING);
            
            break;
            
        // Invalid power source option.
        default:
            rtn = 1;
            
            break;
            
    }
       
    return rtn;
}
//-----------------------------------------------------------------------------
//
//  Function: PowerSetVddaPowerSource
//
//  This routine Sets up the Vdda PowerSource.
//
//  Parameters:
//          eSource
//              [in] power sources for power supply
//
//  Returns:
//          returns 0 if successful otherwise returns error value.
//
//-----------------------------------------------------------------------------
static UINT32 PowerSetVddaPowerSource(POWER_POWERSOURCE eSource)
{
    UINT32 rtn = 0;

    // The VDDA can use two power sources: the DCDC, or the Linear Regulator.
    // Each has its own configuration that must be set up to prevent power
    // rail instability.
        
    switch(eSource)
    {
        // Power the VDDA supply from DCDC with VDDA LinReg off.
        case POWER_DCDC_LINREG_OFF:
        case POWER_DCDC_LINREG_READY:
            
            // Use LinReg offset for DCDC mode.
            BF_WR(POWER_VDDACTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDA DCDC converter output and turn off the LinReg output.
            BF_CLR(POWER_VDDACTRL, DISABLE_FET);
            BF_CLR(POWER_VDDACTRL, ENABLE_LINREG);
            
            // Make sure stepping is enabled when using DCDC.
            BF_CLR(POWER_VDDACTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDA supply from DCDC with VDDA LinReg off.
        case POWER_DCDC_LINREG_ON:
            
            // Use LinReg offset for DCDC mode.
            BF_WR(POWER_VDDACTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDA DCDC converter output and turn on the LinReg output.
            BF_CLR(POWER_VDDACTRL, DISABLE_FET);
            BF_SET(POWER_VDDACTRL, ENABLE_LINREG);
            
            // Make sure stepping is enabled when using DCDC.
            BF_CLR(POWER_VDDACTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDA supply from the linear regulator.  The DCDC output is
        // off and not ready to power the rail if 5V is removed.
        case POWER_LINREG_DCDC_OFF:
            
            // Use LinReg offset for LinReg mode.
            BF_WR(POWER_VDDACTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_LINREG_MODE);
            
            // Turn on the VDDA LinReg output and turn off the VDDA DCDC output.
            BF_SET(POWER_VDDACTRL, ENABLE_LINREG);
            BF_SET(POWER_VDDACTRL, DISABLE_FET);
            
            // Make sure stepping is disabled when using linear regulators.
            BF_SET(POWER_VDDACTRL, DISABLE_STEPPING);
            
            break;
            
        // Power the VDDA supply from the linear regulator.  The DCDC output is
        // ready to power the rail if 5V is removed.
        case POWER_LINREG_DCDC_READY:
            
            // Use LinReg offset for LinReg mode.
            BF_WR(POWER_VDDACTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
            
            // Turn on the VDDA LinReg output and prepare the DCDC for transfer.
            // ENABLE_DCDC must be clear to avoid DCDC and LinReg conflict.
            BF_CLR(POWER_VDDACTRL, ENABLE_LINREG);
            BF_CLR(POWER_VDDACTRL, DISABLE_FET);
            
            // Make sure stepping is disabled when using linear regulators.
            BF_SET(POWER_VDDACTRL, DISABLE_STEPPING);
            
            break;
            
        // Invalid power source option.
        default:
            rtn = 1;            
            break;
            
    }
        
    return rtn;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerGetVdddPowerSource
//
//  This function sets Vddd power source
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static POWER_POWERSOURCE PowerGetVdddPowerSource(void)
{
    // If the Vddd DCDC Converter output is disabled, LinReg must be powering Vddd.
    if(HW_POWER_VDDDCTRL.B.DISABLE_FET)
    {
        // Make sure the LinReg offset is correct for this source.
        if(PowerGetVdddLinRegOffset() == HW_POWER_LINREG_OFFSET_LINREG_MODE)
        {
            return POWER_LINREG_DCDC_OFF;
        }
    }
    
    // If here, DCDC must be powering Vddd.  Determine if the LinReg is also on.
    if(HW_POWER_VDDDCTRL.B.ENABLE_LINREG)
    {
        // The LinReg offset must be below the target if DCDC and LinRegs' are
        // on at the same time.  Otherwise, we have an invalid configuration.
        if(PowerGetVdddLinRegOffset() == HW_POWER_LINREG_OFFSET_DCDC_MODE)
        {
            return POWER_DCDC_LINREG_ON;
        }
    }
    else
    {
        // Is the LinReg offset set to power Vddd from linreg?
        if(PowerGetVdddLinRegOffset() == HW_POWER_LINREG_OFFSET_DCDC_MODE)
        {
            return POWER_DCDC_LINREG_OFF;
        }
    }
    
    // If we get here, the power source is in an unknown configuration.  Most
    // likely, the LinReg offset is incorrect for the given power supply.
    return POWER_UNKNOWN_SOURCE;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerSetVdddBrownoutValue
//
//  This function sets Vddd Brownout value
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static VOID PowerSetVdddBrownoutValue(UINT32 VdddBoOffsetmV)
{
    UINT32 VdddBoOffsetSet;
    
    // Convert millivolts to register setting (1 step = 25mV)
    VdddBoOffsetSet = (UINT32)(VdddBoOffsetmV/(VOLTAGE_STEP_MV));
    
    // Use this way of writing to the register because we want a read, modify,
    // write operation.  Using the other available macros will call a clear/set
    // which momentarily makes the brownout offset zero which may trigger a
    // false brownout.
    
    HW_POWER_VDDDCTRL.B.BO_OFFSET = VdddBoOffsetSet;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerGetVdddBrownoutValue
//
//  This function sets Vddd Brownout value
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static UINT32 PowerGetVdddBrownoutValue(void)
{
    UINT32 VdddBoOffsetSet;
    UINT32 VdddBoOffsetmV;

    // Read the VDDD brownout offset field.
    VdddBoOffsetSet = HW_POWER_VDDDCTRL.B.BO_OFFSET;

    // Convert setting to millivolts. (1 step = 25mV)
    VdddBoOffsetmV = (VdddBoOffsetSet * VOLTAGE_STEP_MV);

    // Return the brownout offset in mV.
    return VdddBoOffsetmV;
}


//-----------------------------------------------------------------------------
//
//  Function:  PowerSetVdddValue
//
//  This function sets Vddd value
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static void PowerSetVdddValue(UINT32 VdddmV)
{
    UINT32 VdddSet;
    
    // Convert mV to register setting
    VdddSet = (UINT32)((VdddmV - VDDD_BASE_MV) / 25);
    
    // Use this way of writing to the register because we want a read, modify,
    // write operation.  Using the other available macros will call a clear/set
    // which momentarily clears the target field causing the voltage to dip
    // then rise again.  That should be avoided.
    HW_POWER_VDDDCTRL.B.TRG = VdddSet;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerGetVdddValue
//
//  This function Gets Vddd value
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static UINT32 PowerGetVdddValue(VOID)
{
    UINT32 VdddSet;
    UINT32 VdddmV;

    //Read VDDD bitfiled value
    VdddSet = (UINT32)HW_POWER_VDDDCTRL.B.TRG;
    
    //  Convert to mVolts
    VdddmV = PowerConvertSettingToVddd(VdddSet);  
    
    return (UINT32)VdddmV;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerLimitVdddAndBo
//      Check VDDD target value and BO value is valid
//
//  Parameters:
//          [In] [Out] *pVdddmV: target value
//          [In] [Out] *pBomV: BO value
//
//  Returns:
//          None
//
//-----------------------------------------------------------------------------
static void PowerLimitVdddAndBo(UINT32 *pVdddmV, UINT32 *pBomV)
{
    UINT32    VdddmV = *pVdddmV;
    UINT32    BomV = *pBomV;

    // Check Vddd limits
    // Make sure Vddd is not above the safe voltage
    if (VdddmV > VDDD_SAFE_MAX_MV)
    {
        VdddmV = VDDD_SAFE_MAX_MV;
    }

    // Make sure Vddd is not below the safe voltage
    if (VdddmV < VDDD_SAFE_MIN_MV)
    {
        VdddmV = VDDD_SAFE_MIN_MV;
    }

    // Check Vddd brownout limits
    // Make sure there's at least a margin of difference between Vddd and Bo
    if (BO_MIN_OFFSET_MV > (VdddmV - BomV))
    {
        BomV = VdddmV - BO_MIN_OFFSET_MV;
    }

    // Make sure the brownout value does not exceed the maximum allowed
    // by the system.
    if ((VdddmV - BomV) > BO_MAX_OFFSET_MV)
    {
        BomV = VdddmV - BO_MAX_OFFSET_MV;
    }

    // Give the results back to the caller.
    *pVdddmV = VdddmV;
    *pBomV = BomV;

}

//-----------------------------------------------------------------------------
//
//  Function:  Power_ConvertSettingToVddd
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static UINT32 PowerConvertSettingToVddd(UINT32 Setting)
{
    return ((Setting * 25) + VDDD_BASE_MV);
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerGetVdddLinRegOffset
//
//  This function gets the Vddd LinRegOffset
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns Vddd  Offset
//
//-----------------------------------------------------------------------------
static POWER_LINREGOFFSETSTEP PowerGetVdddLinRegOffset(VOID)
{
    return (POWER_LINREGOFFSETSTEP) HW_POWER_VDDDCTRL.B.LINREG_OFFSET;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerInitSupply
//
//  This function is used to clear the limit info
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
static void PowerInitCpuClock(void)
{
    // Let CPU sink the xtal clock
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_CPUFRAC) | \
                        BF_CLKCTRL_FRAC0_CPUFRAC(33));

    // Config CLK_CPU driver for 261MHZ
    HW_CLKCTRL_CPU_CLR(BF_CLKCTRL_CPU_DIV_CPU(BM_CLKCTRL_CPU_DIV_CPU));
    HW_CLKCTRL_CPU_SET(BF_CLKCTRL_CPU_DIV_CPU(0x1));
    
    // Config CLK_HBUS as CPU/2 (130MHz)
    HW_CLKCTRL_HBUS_WR((BF_CLKCTRL_HBUS_DIV(2)                    | \
                        BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)            | \
                        BF_CLKCTRL_HBUS_SLOW_DIV(5)               | \
                        BF_CLKCTRL_HBUS_ASM_ENABLE(1)             | \
                        BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(1)    | \
                        BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(1)     | \
                        BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(1)  | \
                        BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(1)      | \
                        BF_CLKCTRL_HBUS_ASM_EMIPORT_AS_ENABLE(1)  | \
                        BF_CLKCTRL_HBUS_PXP_AS_ENABLE(1)          | \
                        BF_CLKCTRL_HBUS_DCP_AS_ENABLE(1)));

    RETAILMSG(1,(TEXT("CPU clock up to 261MHz\r\n")));
    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
}

//-----------------------------------------------------------------------------
//
//  Function: PowerInitPowerSupplies
//
//  This routine initializes the power supplies.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns 0 if successful otherwise returns error value.
//
//-----------------------------------------------------------------------------
static UINT32 PowerInitPowerSupplies(VOID)
{
    UINT32 Status;

    // Make sure the power supplies are configured for their power sources.
    // This sets the LINREG_OFFSET field correctly for each power supply.
   
    if(PowerGet5vPresentFlag() == PMU_POWER_SUPPLY_5V)
    {
        if((Status = PowerSetVdddPowerSource(POWER_LINREG_DCDC_OFF)) != 0)
        {
            return Status;
        }
        if((Status = PowerSetVddaPowerSource(POWER_LINREG_DCDC_OFF)) != 0)
        {
            return Status;
        }
        if((Status = PowerSetVddioPowerSource(POWER_LINREG_DCDC_OFF)) != 0)
        { 
            return Status;
        }
    }
    else
    {
        // For LiIon battery, all the rails can start off as DCDC mode.
        if((Status = PowerSetVdddPowerSource(POWER_DCDC_LINREG_READY)) != 0)
        {
            return Status;
        }
        if((Status = PowerSetVddaPowerSource(POWER_DCDC_LINREG_READY)) != 0)
        {
            return Status;
        }
        if((Status = PowerSetVddioPowerSource(POWER_DCDC_LINREG_READY)) != 0)
        {
            return Status;
        }
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
//
//  Function: PowerGet5vPresentFlag
//
//  This routine checks if the 5V supply is present.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns VBUS status.
//
//-----------------------------------------------------------------------------
static PMU_POWER_SUPPLY_MODE PowerGet5vPresentFlag(VOID)
{
    if(HW_POWER_STS.B.VBUSVALID0)
        return PMU_POWER_SUPPLY_5V;
    
    return PMU_POWER_SUPPLY_BATTERY;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerExecuteBatteryTo5VoltsHandoff
//
//  This function hands off power source from battery to 5Volts supply
//
//  Parameters:
//          None.
//
//  Returns:
//         None.
//
//-----------------------------------------------------------------------------
static VOID PowerExecuteBatteryTo5VoltsHandoff()
{
    // Startup the 4p2 rail and enabled 4p2 from DCDC
    if(b4p2Enabled)
    {
        PowerStart4p2();
        return;
    }
      
    PowerSetVdddPowerSource(POWER_LINREG_DCDC_READY);
    PowerSetVddaPowerSource(POWER_LINREG_DCDC_READY);
    PowerSetVddioPowerSource(POWER_LINREG_DCDC_READY);

    // Disable the DCDC during 5V connections.
    BF_CLR(POWER_5VCTRL, ENABLE_DCDC);

    // Disable hardware power down when 5V is inserted or removed
    BF_CLR(POWER_5VCTRL, PWDN_5VBRNOUT);
    
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerExecute5VoltsToBatteryHandoff
//
//  This function hands off power source from 5Volts supply to battery
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
static void PowerExecute5VoltsToBatteryHandoff(void)
{
    // Disable DCDC from 4p2 and turn off the 4p2 rail
    PowerStop4p2();

    // Disable hardware power down when 5V is inserted or removed
    BF_CLR(POWER_5VCTRL, PWDN_5VBRNOUT);

    // Re-enable the battery brownout interrupt in case it was disabled.
    BF_SET(POWER_CTRL, ENIRQBATT_BO);
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerEnable5VoltsToBatteryHandoff
//
//  This function enables the handoff from 5Volts supply to battery
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
static VOID PowerEnable5VoltsToBatteryHandoff(VOID)
{
    // Enable detection of 5V removal (unplug)
    BF_CLR(POWER_CTRL, POLARITY_VBUSVALID);

    //hw_power_EnableAutoDcdcTransfer(TRUE) causes strange behavior when 4p2 is enabled.
    if(b4p2Enabled)
    { 
        return;
    }

    // Enable automatic transition to DCDC
    BF_SET(POWER_5VCTRL, DCDC_XFER);    
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerEnableBatteryTo5VoltsHandoff
//
//  This function enables the handoff from battery to 5Volts
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
static void PowerEnableBatteryTo5VoltsHandoff(void)
{
    // Enable 5V plug-in detection
    BF_SET(POWER_CTRL, POLARITY_VBUSVALID);

    // Allow DCDC be to active when 5V is present.
    BF_SET(POWER_5VCTRL, ENABLE_DCDC);
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerClear5VIrq
//
//  This function is used to clear the 5V irq
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
static VOID PowerClear5VIrq()
{
    BF_CLR(POWER_CTRL, VBUSVALID_IRQ);
    BF_CLR(POWER_CTRL, VDD5V_GT_VDDIO_IRQ);
    BF_CLR(POWER_CTRL, VDD5V_DROOP_IRQ);
    BF_CLR(POWER_CTRL, DC_OK_IRQ);    
    BF_CLR(POWER_CTRL, PSWITCH_IRQ);
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerGetLimit
//
//  This function is used to get the limit info
//
//  Parameters:
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerGetLimit()
{   
    if(Is5VFromVbus())
    {
        sb_Vbus5v = TRUE;
    }
    
    // If it is a Wall 5V,we should clear current limitation.
    if(sb_Vbus5v == FALSE)
    {
        return FALSE;
    }
    // If USB driver isn't exist,return.
    if(!IsUSBDeviceDriverEnable())
        return FALSE;

    if(((HW_POWER_5VCTRL_RD() & BM_POWER_5VCTRL_CHARGE_4P2_ILIMIT) == 0x20000)
       && (PowerGetUSBPhy() == TRUE))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerGetUSBPhy
//
//  This function is used to clear the limit info
//
//  Parameters:
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

static BOOL PowerGetUSBPhy()
{
    if ((HW_USBPHY_CTRL_RD(0)& BM_USBPHY_CTRL_CLKGATE) == 1)
    {
        HW_USBPHY_CTRL_CLR(0, BM_USBPHY_CTRL_CLKGATE);
    }
    
    if ((HW_USBPHY_CTRL_RD(0)& BM_USBPHY_CTRL_SFTRST) == 1)
    {
        HW_USBPHY_CTRL_CLR(0, BM_USBPHY_CTRL_SFTRST);
    }
    
    HW_USBPHY_CTRL_SET(0, BM_USBPHY_CTRL_ENDEVPLUGINDETECT);
    Sleep(1);
    
    if((HW_USBPHY_STATUS_RD(0) & BM_USBPHY_STATUS_DEVPLUGIN_STATUS)) 
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerInit
//
//  This function initializes the power funtionality for the supporting modlules
//
//  Parameters:
//          pInitValues.
//              [in] pointer to the structure POWER_INITVALUES
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerInit()
{
    BOOL rc = FALSE;
    UINT32 irq = 0;

//#ifdef EM9283	// LQK:Jul 9,2012
	DWORD dwStatus,dwVal,dwSize;
	HKEY  hKey;
	TCHAR szName[] = TEXT("Is5VFromUSB");

	dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\Power"),
		0,0,&hKey);
	if (dwStatus != ERROR_SUCCESS)
	{
		RETAILMSG(1, (TEXT("PowerInit: OpenDeviceKey failed \r\n")));
		g_bIs5VFromVbus = TRUE;
	}
	else
	{
		dwSize = sizeof(DWORD);
		dwStatus = RegQueryValueEx(hKey, szName, NULL, 
			NULL, (LPBYTE)&dwVal, &dwSize);
		if (dwStatus != ERROR_SUCCESS )
		{
			RETAILMSG(1, (TEXT("PowerInit: RegQueryValueEx failed \r\n")));
			g_bIs5VFromVbus = TRUE;
		}
		else
		{
			g_bIs5VFromVbus = dwVal;
		}
		RegCloseKey(hKey);

	}
	if( g_bIs5VFromVbus )
	{
		RETAILMSG(1, (TEXT("PowerInit: 5V is from USB.\r\n")));
	}
	else
	{
		RETAILMSG(1, (TEXT("PowerInit: 5V is from Wall charger.\r\n")));
	}
	
	BSPSetVFromVbusValue( g_bIs5VFromVbus );   /* CS&ZHL SEP24-2012: */

//#endif  //EM9283
 
    //Map the peripheral register space of the power module
    if (!PowerAlloc())
    {
        ERRORMSG(1, (_T("PowerInit :: PowerAlloc() failed!")));
        goto cleanUp;
    }      

    //DumpPowerRegisters();
    if((HW_POWER_5VCTRL.B.PWD_CHARGE_4P2 == 0) && ((HW_POWER_5VCTRL_RD() & BM_POWER_5VCTRL_CHARGE_4P2_ILIMIT) == 0x20000))
    {
        RETAILMSG(1,(TEXT("Wait USB enumeration done...\r\n")));
        if(IsUSBDeviceDriverEnable())
            bPLL = TRUE;
        else
        {
            RETAILMSG(1,(TEXT("USB client driver not exist,cancel current limit!!!\r\n")));            
            BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x3F);
            PowerInitCpuClock();
        }
    }
    while((HW_POWER_5VCTRL.B.PWD_CHARGE_4P2 == 0) && ((HW_POWER_5VCTRL_RD() & BM_POWER_5VCTRL_CHARGE_4P2_ILIMIT) == 0x20000))
    {
        Sleep(100);
    }
    
    //BF_CLR(POWER_CTRL, CLKGATE);
    
    // Improve efficieny and reduce transient ripple
    BF_SET(POWER_LOOPCTRL, TOGGLE_DIF);
    BF_SET(POWER_LOOPCTRL, EN_CM_HYST);
    BF_SET(POWER_LOOPCTRL, EN_DF_HYST);
    BF_WR(POWER_DCLIMITS, POSLIMIT_BUCK, 0x30); 

    if(bPLL == TRUE)
        PowerInitCpuClock();
    
    // Use VBUSVALID for DCDC 5V detection.
    BF_SET(POWER_5VCTRL, VBUSVALID_5VDETECT);
    BF_WR(POWER_5VCTRL, VBUSVALID_TRSH, 1);
    // Use VBUSVALID for DCDC 5V detection.
    BF_SET(POWER_5VCTRL, VBUSVALID_5VDETECT);

    //Enable double FETs.
    HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_HALF_FETS);
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_DOUBLE_FETS);
    
    BF_SET(POWER_BATTMONITOR,EN_BATADJ);
    BF_SET(POWER_LOOPCTRL,RCSCALE_THRESH);
    BF_WR(POWER_LOOPCTRL,EN_RCSCALE,HW_POWER_RCSCALE_8X_INCR);

    // Create an event for the initializing to 5V interrupt
    g_h5VIntEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Create an event for the 
    g_h5VIntEventNotify = CreateEvent(NULL, FALSE, FALSE, g_5VDetectEventName);

    if(g_h5VIntEvent == NULL)
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to create 5V interrupt event.\r\n")));
        return FALSE;
    }
    
    irq = IRQ_VDD5V;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(UINT32),
                         &SysIntr5V, sizeof(UINT32), NULL))
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to obtain sysintr value for 5V interrupt.\r\n")));
        return FALSE;
    }

    if ( !InterruptInitialize(SysIntr5V,g_h5VIntEvent,NULL,0) ) 
    {
        RETAILMSG(TRUE,(TEXT("Error initializing interrupt\n\r")));
        return FALSE;
    }
    
    // CreateThread to check USB 100ma limited and init Power
    PowerHandler_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PowerInitHandle, NULL, 0, NULL);
    if (PowerHandler_thread == NULL)
        RETAILMSG(TRUE, (TEXT("PowerHandler_thread : CreateThread Failed!\r\n")));
    
    CeSetThreadPriority(PowerHandler_thread,CE_THREAD_PRIO_256_HIGHEST);
    
#if 1
    DEBUGMSG(ZONE_FUNC, (_T("%s: About to enable the wake source %d %d\r\n"),__WFUNCTION__, SysIntr5V, irq));
    // Ask the OAL to enable our interrupt to wake the system from suspend.
    KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &SysIntr5V,sizeof(SysIntr5V), NULL, 0, NULL);
    
    DEBUGMSG(ZONE_FUNC, (_T("%s: Done enabling the wake source\r\n"),__WFUNCTION__));
#endif    

    // CreateThread for Waiting on the 5V interrupt
    IrqHandler_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PowerIRQHandler, NULL, 0, NULL);
    if (IrqHandler_thread == NULL)
        RETAILMSG(TRUE, (TEXT("PowerIRQHandler : CreateThread Failed!\r\n")));
   
    CeSetThreadPriority(IrqHandler_thread,CE_THREAD_PRIO_256_ABOVE_NORMAL); 

    //
    // Create  event for Power rail Brownout interrupt
    //
    
    //VDDD BrownOut IRQ  
    g_hVDDDBNOTEvent = CreateEvent(NULL, FALSE, FALSE, NULL);     
    if(g_hVDDDBNOTEvent == NULL)
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to create g_hVDDxBNOTEvent0 event\r\n")));
        return FALSE;
    }    
    irq = IRQ_VDDD_BROWNOUT;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(UINT32), &SysIntrVDDD, sizeof(UINT32), NULL))
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to obtain sysintr value for VDDD interrupt.\r\n")));
        return FALSE;
    }     
    if (!InterruptInitialize(SysIntrVDDD,g_hVDDDBNOTEvent,NULL,0)) 
    {
        RETAILMSG(TRUE,(TEXT("Error initializing interrupt\n\r")));
        return FALSE;
    }
    VDDDBOHandler_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)VDDDBOHandler, NULL, 0, NULL);
    if (VDDDBOHandler_thread == NULL)
    {
        RETAILMSG(TRUE, (TEXT("PowerIRQHandler : CreateThread Failed!\r\n")));
    }    
    CeSetThreadPriority(VDDDBOHandler_thread,CE_THREAD_PRIO_256_TIME_CRITICAL);

    //VDDIO BrownOut IRQ  
    g_hVDDIOBNOTEvent = CreateEvent(NULL, FALSE, FALSE, NULL);     
    if(g_hVDDIOBNOTEvent == NULL)
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to create g_hVDDIOBNOTEvent0 event\r\n")));
        return FALSE;
    }    
    irq = IRQ_VDDIO_BROWNOUT;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(UINT32), &SysIntrVDDIO, sizeof(UINT32), NULL))
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to obtain sysintr value for VDDD interrupt.\r\n")));
        return FALSE;
    }     
    if (!InterruptInitialize(SysIntrVDDIO,g_hVDDIOBNOTEvent,NULL,0)) 
    {
        RETAILMSG(TRUE,(TEXT("Error initializing interrupt\n\r")));
        return FALSE;
    }
    VDDIOBOHandler_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)VDDIOBOHandler, NULL, 0, NULL);
    if (VDDIOBOHandler_thread == NULL)
    {
        RETAILMSG(TRUE, (TEXT("PowerIRQHandler : CreateThread Failed!\r\n")));
    }    
    CeSetThreadPriority(VDDIOBOHandler_thread, CE_THREAD_PRIO_256_TIME_CRITICAL);


    //VDDA BrownOut IRQ  
    g_hVDDABNOTEvent = CreateEvent(NULL, FALSE, FALSE, NULL);     
    if(g_hVDDABNOTEvent == NULL)
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to create g_hVDDIOBNOTEvent0 event\r\n")));
        return FALSE;
    }    
    irq = IRQ_VDDA_BROWNOUT;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(UINT32), &SysIntrVDDA, sizeof(UINT32), NULL))
    {
        RETAILMSG(TRUE, (TEXT("ERROR: Failed to obtain sysintr value for VDDD interrupt.\r\n")));
        return FALSE;
    }     
    if (!InterruptInitialize(SysIntrVDDA,g_hVDDABNOTEvent,NULL,0)) 
    {
        RETAILMSG(TRUE,(TEXT("Error initializing interrupt\n\r")));
        return FALSE;
    }
    VDDABOHandler_thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)VDDABOHandler, NULL, 0, NULL);
    if (VDDABOHandler_thread == NULL)
    {
        RETAILMSG(TRUE, (TEXT("PowerIRQHandler : CreateThread Failed!\r\n")));
    }    
    CeSetThreadPriority(VDDABOHandler_thread, CE_THREAD_PRIO_256_TIME_CRITICAL);

    //Enable VBUSVALID for 5V detection.
    HW_POWER_CTRL_CLR(BM_POWER_CTRL_ENIRQ_VDD5V_GT_VDDIO);
    HW_POWER_CTRL_SET(BM_POWER_CTRL_ENIRQ_VBUS_VALID);

    //Enable Pswitch
    HW_POWER_CTRL_SET(BM_POWER_CTRL_POLARITY_PSWITCH | 
                      BM_POWER_CTRL_ENIRQ_PSWITCH);

    //Clear BO interrupt state
    HW_POWER_CTRL_CLR(BM_POWER_CTRL_VDDD_BO_IRQ |
                      BM_POWER_CTRL_VDDA_BO_IRQ |
                      BM_POWER_CTRL_VDDIO_BO_IRQ); 
    //Enable Power rail BO interrupt
    HW_POWER_CTRL_SET(BM_POWER_CTRL_ENIRQ_VDDD_BO |
                      BM_POWER_CTRL_ENIRQ_VDDA_BO |
                      BM_POWER_CTRL_ENIRQ_VDDIO_BO );

    rc = TRUE;

cleanUp:
    if(!rc)
        PowerDeInit();
    return rc;         
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerDeInit
//
//  This function De-initializes the resources allocated
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerDeInit(VOID)
{
    //Deallocate the resources
    PowerDealloc();

    if (PowerHandler_thread)
    {     
        CloseHandle(PowerHandler_thread);
        PowerHandler_thread = NULL;
    }
    if (IrqHandler_thread)
    {     
        CloseHandle(IrqHandler_thread);
        IrqHandler_thread = NULL;
    }

    if (g_h5VIntEvent)
    {     
        CloseHandle(g_h5VIntEvent);
        g_h5VIntEvent = NULL;
    }

    if (g_hVDDDBNOTEvent)
    {     
        CloseHandle(g_hVDDDBNOTEvent);
        g_hVDDDBNOTEvent = NULL;
    }    

    if (VDDDBOHandler_thread)
    {     
        CloseHandle(VDDDBOHandler_thread);
        VDDDBOHandler_thread = NULL;
    }

    if (g_hVDDIOBNOTEvent)
    {     
        CloseHandle(g_hVDDIOBNOTEvent);
        g_hVDDIOBNOTEvent = NULL;
    }    

    if (VDDIOBOHandler_thread)
    {     
        CloseHandle(VDDIOBOHandler_thread);
        VDDIOBOHandler_thread = NULL;
    }

    if (g_hVDDABNOTEvent)
    {     
        CloseHandle(g_hVDDABNOTEvent);
        g_hVDDABNOTEvent = NULL;
    }    

    if (VDDABOHandler_thread)
    {     
        CloseHandle(VDDABOHandler_thread);
        VDDABOHandler_thread = NULL;
    }

    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: PmuIoctlInitBatteryMonitor
//
//  This routine initializes the Battery monitor for battery module.
//

//  Parameters:
//          eTrigger
//              [in]  Specifies the Lradc trigger used
//          SamplingInterval
//              [out]  Specifies the sampling interval for the Battery value
//                     to be sampled
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void PmuIoctlInitBatteryMonitor(void)
{   
    // Finally enable the battery adjust
    BF_SET(POWER_BATTMONITOR,EN_BATADJ);

    // The following settings give optimal power supply capability and
    // efficiency.  Extreme loads will need HALF_FETS cleared and
    // possibly DOUBLE_FETS set.  The below setting are probably also
    // the best for alkaline mode also but more characterization is
    // needed to know for sure.

    // Increase the RCSCALE_THRESHOLD
    BF_SET(POWER_LOOPCTRL, RCSCALE_THRESH);

    // Increase the RCSCALE level for quick DCDC response to dynamic load
    BF_WR(POWER_LOOPCTRL, EN_RCSCALE, HW_POWER_RCSCALE_8X_INCR);
}


//-----------------------------------------------------------------------------
//
//  Function:PmuIoctlGet5vPresentFlag
//
//  This routine checks if the 5V supply is present.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns VBUS status.
//
//-----------------------------------------------------------------------------
PMU_POWER_SUPPLY_MODE PmuIoctlGet5vPresentFlag(VOID)
{
    return PowerGet5vPresentFlag();
}

//-----------------------------------------------------------------------------
//
//  Function: PmuIoctlGetBatteryVoltage
//
//  This routine returns the current Battery voltage.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns battery voltage.
//
//-----------------------------------------------------------------------------
UINT32 PmuIoctlGetBatteryVoltage(VOID)
{
    UINT32 BattVolt = 0;   
       
    // Get the raw result of battery measurement
    BattVolt = (UINT32)HW_POWER_BATTMONITOR.B.BATT_VAL;

    // Adjust for 8-mV LSB resolution and return
    return (BattVolt * BATT_VOLTAGE_8_MV);
}

//-----------------------------------------------------------------------------
//
//  Function:  PmuIoctlSetCharger
//
//  This function is used to set the charger
//
//  Parameters:
//          current
//              [in] The current value of charger
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
BOOL PmuIoctlSetCharger(UINT32 current)
{
    if(PowerGetLimit())
    {        
        RETAILMSG(1, (TEXT("USB current 100mA limitation, can not charging...\r\n")));
        return FALSE;
    }
    BF_CLRV(POWER_CHARGE, STOP_ILIMIT, 0xF);
    BF_SETV(POWER_CHARGE, STOP_ILIMIT, 0x3);//stop limit current = 30mA  
    
    BF_CLRV(POWER_CHARGE, BATTCHRG_I, 0x3F);
    BF_SETV(POWER_CHARGE, BATTCHRG_I, current); 

    BF_CLR(POWER_CHARGE, PWD_BATTCHRG);
    
    return TRUE;    
}

//-----------------------------------------------------------------------------
//
//  Function:  PmuIoctlStopCharger
//
//  This function is used to stop the charger
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID PmuIoctlStopCharger()
{
    BF_CLRV(POWER_CHARGE, STOP_ILIMIT, 0xF);
    BF_CLRV(POWER_CHARGE, BATTCHRG_I, 0x3F);
    BF_SET(POWER_CHARGE, PWD_BATTCHRG);   
}

//-----------------------------------------------------------------------------
//
//  Function: PmuIoctlrGetBatteryChargingStatus
//
//  This routine returns the Battery charging status.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL PmuIoctlGetBatteryChargingStatus(VOID)
{
    BOOL Flag = FALSE;

    if(PowerGet5vPresentFlag() == PMU_POWER_SUPPLY_5V)
    {
        do{
             Flag = (BOOL)HW_POWER_STS.B.CHRGSTS;
        }while(Flag != (BOOL)HW_POWER_STS.B.CHRGSTS);
        
        return Flag;
    }
    else
    {
        return FALSE;
    }
}

//-----------------------------------------------------------------------------
//
//  Function:  PmuIoctlSetVddd
//
// This function sets the VDDD value and VDDD brownout level specified by the
// input parameters. If the new brownout level is equal to the current setting
// it'll only update the VDDD setting. If the new brownout level is less than
// the current setting, it will update the VDDD brownout first and then the VDDD.
// Otherwise, it will update the VDDD first and then the brownout. This
// arrangement is intended to prevent from false VDDD brownout. This function
// will not return until the output VDDD stable.
//
//  Parameters:
//          NewTarget
//             [in] Vddd voltage in millivolts
//
//          NewBrownout
//             [in]  Vddd brownout in millivolts
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------

void PmuIoctlSetVddd(UINT32 NewTarget,  UINT32 NewBrownout )
{
    UINT32    CurrentTarget, StepTarget, TargetDifference;
    UINT32    RegData;
    BOOL      bPoweredByLinReg;

    //--------------------------------------------------------------------------
    // Limit inputs and get the current target level
    //--------------------------------------------------------------------------

    // Apply ceiling and floor limits to Vddd and BO
    PowerLimitVdddAndBo(&NewTarget, &NewBrownout);

    // Convert the brownout as millivolt value to an offset from the target
    NewBrownout = NewTarget - NewBrownout;

    // Get the current target value
    CurrentTarget = PowerGetVdddValue();

    // Determine if this rail is powered by linear regulator.
    if(PowerGetVdddPowerSource() == POWER_LINREG_DCDC_READY ||
       PowerGetVdddPowerSource() == POWER_LINREG_DCDC_OFF )
    {
        bPoweredByLinReg = TRUE;
    }
    else
    {
        bPoweredByLinReg = FALSE;
    }

    //--------------------------------------------------------------------------
    // Voltage and brownouts need to be changed in specific order.
    //
    // Because the brownout is now an offset of the target, as the target
    // steps up or down, the brownout voltage will follow.  This causes a
    // problem where the brownout will be one step too close when the target
    // raised because the brownout changes instantly, but the output voltage
    // ramps up.
    //
    //  Target does this            ____
    //                                  /
    //                          ____/
    //  but brownout does this  _____
    //                                |
    //                          ____|
    //
    //  We are concerned about the time under the ramp when the brownout
    //  is too close.
    //--------------------------------------------------------------------------
    if(NewTarget > CurrentTarget)
    {
        //----------------------------------------------------------------------
        // Temporarily change the rail's brownout.
        //----------------------------------------------------------------------
        if(bPoweredByLinReg)
        {
            // Disable detection if powered by linear regulator.  This avoids
            // the problem where the brownout level reaches its new value before
            // the target does.
            //PowerEnableVdddBrownoutInterrupt(FALSE);
            BF_CLR(POWER_CTRL, ENIRQ_VDDD_BO);
        }

        {
            // Temporarily use the maximum offset for DCDC transitions.  DCDC
            // transitions step the target and the brownout offset in 25mV
            // increments so there is not a risk of an inverted brownout level.
            PowerSetVdddBrownoutValue(BO_MAX_OFFSET_MV);
        }
        
        //----------------------------------------------------------------------
        // We want to limit voltage step sizes to account for FUNCV changes and
        // linreg voltage change which may result in current surges/voltage dips
        // on VDD5V.  To accomplish this, we will step the voltage in 
        // pre-determined levels until the new target is reached.   
        //----------------------------------------------------------------------
        do{
        
            // Calculate the next target to step to.  If we can't get to the 
            // target without exceeding the maximum step voltage, then step
            // by the maximum and try again next loop.
            TargetDifference = NewTarget - CurrentTarget;
            if(TargetDifference > POWER_MAX_VOLTAGE_STEP_MV)
            {
                StepTarget = CurrentTarget + POWER_MAX_VOLTAGE_STEP_MV;
            }
            else
            {
                StepTarget = NewTarget;
            }    
            
            // Now change target and wait for it to stabilize.
            PowerSetVdddValue(StepTarget);
            //ddi_power_WaitForVdddStable();
            if(bPoweredByLinReg)
            {
                // need to wait more than 100 microsecond
                RegData=HW_DIGCTL_MICROSECONDS_RD();
               for(; (HW_DIGCTL_MICROSECONDS_RD()-RegData) <= 5000;);             
            }
            else
            {
                // need to wait more than 15 microsecond before the DC_OK is valid
                RegData=HW_DIGCTL_MICROSECONDS_RD();
               for(; (HW_DIGCTL_MICROSECONDS_RD()-RegData) <= 15;); 
    
               // wait for DC_OK
               while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));
            }                
            // Read the current value for the next comparison.
            CurrentTarget = PowerGetVdddValue();

        }
        while(NewTarget > CurrentTarget);

        //----------------------------------------------------------------------
        // Clean up any brownout issues and set the new brownout level.
        //----------------------------------------------------------------------
        if(bPoweredByLinReg)
        {
            // Clear the interrupt in case it occured.  
            //hw_power_ClearVdddBrownoutInterrupt();
            BF_CLR(POWER_CTRL, VDDD_BO_IRQ);

            // Reenable brownout since it was disabled for linreg cases.
            //hw_power_EnableVdddBrownoutInterrupt(TRUE);
            BF_SET(POWER_CTRL, ENIRQ_VDDD_BO);            
        }
        
        // Set the real brownout offset.
        PowerSetVdddBrownoutValue(NewBrownout);

    }
    else
    {
        //----------------------------------------------------------------------
        // We want to limit voltage step sizes to account for FUNCV changes and
        // linreg voltage change which may result in current surges/voltage dips
        // on VDD5V.
        //----------------------------------------------------------------------
        do{        
            // Calculate the next target to step to.  If we can't get to the 
            // target without exceeding the maximum step voltage, then step
            // by the maximum and try again next loop.
            TargetDifference = CurrentTarget - NewTarget;
            if(TargetDifference > POWER_MAX_VOLTAGE_STEP_MV)
            {
                StepTarget = CurrentTarget - POWER_MAX_VOLTAGE_STEP_MV;
            }
            else
            {
                StepTarget = NewTarget;
            }
            
            // Now change target and wait for it to stabilize.
            PowerSetVdddValue(StepTarget);
            //ddi_power_WaitForVdddStable();
            if(bPoweredByLinReg)
            {
                // need to wait more than 100 microsecond
                RegData=HW_DIGCTL_MICROSECONDS_RD();
               for(; (HW_DIGCTL_MICROSECONDS_RD()-RegData) <= 5000;);             
            }
            else
            {
                // need to wait more than 15 microsecond before the DC_OK is valid
                RegData=HW_DIGCTL_MICROSECONDS_RD();
               for(; (HW_DIGCTL_MICROSECONDS_RD()-RegData) <= 15;); 
    
               // wait for DC_OK
               while (!(HW_POWER_STS_RD() & BM_POWER_STS_DC_OK));
            } 

            // Read the current value for the next comparison.
            CurrentTarget = PowerGetVdddValue();
        }
        while(NewTarget < CurrentTarget);

        //----------------------------------------------------------------------
        // Set the new brownout level.
        //----------------------------------------------------------------------
        PowerSetVdddBrownoutValue(NewBrownout);
    }

}

//-----------------------------------------------------------------------------
//
//  Function:  PmuIoctlGetVddd
//
//  This function returns the present values of the VDDD voltage in millivolts
//
//  Parameters:
//      None.
//
//  Returns:
//      Vddd voltage in millivolts
//
//-----------------------------------------------------------------------------
UINT32 PmuIoctlGetVddd(void )
{
    UINT32 VdddSet;
    UINT32 VdddmV;

    //Read VDDD bitfiled value
    VdddSet = (UINT32)HW_POWER_VDDDCTRL.B.TRG;
    
    //  Convert to mVolts
    VdddmV = PowerConvertSettingToVddd(VdddSet);   
    return (UINT32)VdddmV;

}

//-----------------------------------------------------------------------------
//
//  Function:  PmuIoctlGetVdddBrownont
//
//  This function returns the present values of the VDDD brownout in millivolts
//
//  Parameters:
//      None.
//
//  Returns:
//      Vddd Brownout voltage in millivolts
//
//-----------------------------------------------------------------------------
UINT32 PmuIoctlGetVdddBrownont(void )
{
    UINT32 VdddBoOffsetmV;
    UINT32 VdddmV;
    UINT32 VdddBo;

    //--------------------------------------------------------------------------
    // Read the target and brownout register values.
    //--------------------------------------------------------------------------
    VdddmV = PowerGetVdddValue();
    VdddBoOffsetmV = PowerGetVdddBrownoutValue();

    //--------------------------------------------------------------------------
    // The brownout level is the difference between the target and the offset.
    //--------------------------------------------------------------------------
    VdddBo = VdddmV - VdddBoOffsetmV;

    return VdddBo;

}

//-----------------------------------------------------------------------------
//
//  Function: PmuIoctlSetFets
//
//  This routine initializes the Fets mode, 
//
//  Parameters:
//          bFetsMode
//              [in]  Fets mode
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID PmuIoctlSetFets(PMU_POWER_FETSSET bFetsMode)
{
    switch(bFetsMode)
    {
    case PMU_POWER_HALF_FETS:
        BF_CLR(POWER_MINPWR, DOUBLE_FETS);
        BF_SET(POWER_MINPWR, HALF_FETS);
        break;
    case PMU_POWER_DOUBLE_FETS:
        BF_CLR(POWER_MINPWR, HALF_FETS);
        BF_SET(POWER_MINPWR, DOUBLE_FETS);
        break;
    default:
        BF_CLR(POWER_MINPWR, DOUBLE_FETS);
        BF_CLR(POWER_MINPWR, HALF_FETS);        
        break;        
    }
}

//-----------------------------------------------------------------------------
//
//  Function: PmuIoctlThermalSet
//
//  This routine initializes the Thermal reset temperture.
//
//  Parameters:
//          bFetsMode
//              [in]  Fets mode
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID PmuIoctlThermalSet(PMU_POWER_THERMAL_TEMP bTempMode)
{
    if(bTempMode < PMU_POWER_THERMAL_TEST40_DEGC)
    {
        //Clear TEST and ADJ mode
        HW_POWER_THERMAL_CLR(BM_POWER_THERMAL_TEST);
        HW_POWER_THERMAL_CLR(BM_POWER_THERMAL_OFFSET_ADJ);
    
        HW_POWER_THERMAL_SET(bTempMode);
    }
    else if(bTempMode == PMU_POWER_THERMAL_TEST40_DEGC)
    {
        HW_POWER_THERMAL_SET(BM_POWER_THERMAL_TEST);
    }
    else if(bTempMode == PMU_POWER_THERMAL_105_DEGC)
    {
        //Clear TEST and ADJ mode
        HW_POWER_THERMAL_CLR(BM_POWER_THERMAL_TEST);
        HW_POWER_THERMAL_SET(BM_POWER_THERMAL_OFFSET_ADJ);
    
        HW_POWER_THERMAL_SET(PMU_POWER_THERMAL_115_DEGC);
        //Subtract 10 degC
        HW_POWER_THERMAL_SET(BF_POWER_THERMAL_OFFSET_ADJ(0x3));
    }
    else if(bTempMode == PMU_POWER_THERMAL_110_DEGC)
    {
        //Clear TEST and ADJ mode
        HW_POWER_THERMAL_CLR(BM_POWER_THERMAL_TEST);
        HW_POWER_THERMAL_SET(BM_POWER_THERMAL_OFFSET_ADJ);
    
        HW_POWER_THERMAL_SET(PMU_POWER_THERMAL_115_DEGC);
        //Subtract 5 degC
        HW_POWER_THERMAL_SET(BF_POWER_THERMAL_OFFSET_ADJ(0x2));
    }
    else if(bTempMode == PMU_POWER_THERMAL_155_DEGC)
    {
        //Clear TEST and ADJ mode
        HW_POWER_THERMAL_CLR(BM_POWER_THERMAL_TEST);
        HW_POWER_THERMAL_SET(BM_POWER_THERMAL_OFFSET_ADJ);
    
        HW_POWER_THERMAL_SET(PMU_POWER_THERMAL_150_DEGC);
        //Add 5 degC
        HW_POWER_THERMAL_SET(BF_POWER_THERMAL_OFFSET_ADJ(0x0));
    }
    else if(bTempMode == PMU_POWER_THERMAL_160_DEGC)
    {
        //Clear TEST and ADJ mode
        HW_POWER_THERMAL_CLR(BM_POWER_THERMAL_TEST);
        HW_POWER_THERMAL_SET(BM_POWER_THERMAL_OFFSET_ADJ);
    
        HW_POWER_THERMAL_SET(PMU_POWER_THERMAL_150_DEGC);
        //Add 10 degC
        HW_POWER_THERMAL_SET(BF_POWER_THERMAL_OFFSET_ADJ(0x1));
    }
}

//-----------------------------------------------------------------------------
//
//  Function: PmuIoctlThermalGet
//
//  This routine initializes the Thermal reset temperture.
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
PMU_POWER_THERMAL_TEMP PmuIoctlThermalGet()
{
    UINT32 uTemp1, uTemp2;

    if(HW_POWER_THERMAL.B.TEST)
        return PMU_POWER_THERMAL_TEST40_DEGC;
    
    uTemp1 = HW_POWER_THERMAL.B.TEMP_THRESHOLD;
    
    if(!HW_POWER_THERMAL.B.OFFSET_ADJ_ENABLE)
        return uTemp1;
    
    uTemp2 = HW_POWER_THERMAL.B.OFFSET_ADJ;
    
    if(uTemp1 == PMU_POWER_THERMAL_115_DEGC)
    {
        if(uTemp2 == 0x2)
            return PMU_POWER_THERMAL_110_DEGC;
        return PMU_POWER_THERMAL_105_DEGC;
    }
    else
    {
        if(uTemp2 == 0x0)
            return PMU_POWER_THERMAL_155_DEGC;
        return PMU_POWER_THERMAL_160_DEGC;
    }
    
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerOffChip
//
//  This function will power down the chip
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID PowerOffChip(void )
{
	//JLY05-2012: LQK
    HANDLE hSysShutDownEvent;


    // Clear auto restart.
    HW_RTC_PERSISTENT0_CLR(BM_RTC_PERSISTENT0_AUTO_RESTART);
	
	//
	//JLY05-2012: LQK
	//
	hSysShutDownEvent = CreateEvent(NULL, FALSE, FALSE, L"PowerManager/SysShutDown_Active");
	SetEvent( hSysShutDownEvent );
	Sleep( 2000 );
	//

    SetDevicePower(TEXT("BKL1:"),POWER_NAME | POWER_FORCE,D4);

    while(((HW_POWER_STS_RD() & BM_POWER_STS_PSWITCH) >> BP_POWER_STS_PSWITCH) == ((BM_POWER_STS_PSWITCH >> BP_POWER_STS_PSWITCH) & 0x1))
        Sleep(50);
       
	//
	//JLY05-2012: LQK
	//
	CloseHandle( hSysShutDownEvent );
	Sleep(1000);

    // Chip power off

    BF_WR(POWER_RESET,UNLOCK,BV_POWER_RESET_UNLOCK__KEY);
    

    HW_POWER_RESET_WR((BV_POWER_RESET_UNLOCK__KEY << BP_POWER_RESET_UNLOCK) | BM_POWER_RESET_PWD);
    for(;;) 
    { 
        ERRORMSG(1, (TEXT("Error the code should never been executed!! \r\n")));
    };
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Init
//
// The Device Manager calls this function as a result of a call to the
// ActivateDevice() function.  This function will retrieve the handle to the
// device context from the registry and then initialize the PMI module by
// turning off all of the peripherals that are not being used.
//
// Parameters:
//      pContext
//          [in] Pointer to a string containing the registry path to the
//          active key for the stream interface driver.
//
// Returns:
//      Returns a handle to the device context created if successful. Returns
//      zero if not successful.
//
//-----------------------------------------------------------------------------
DWORD
PMI_Init(LPCTSTR pContext)
{
    DWORD rc = 0;

#ifndef DEBUG
    UNREFERENCED_PARAMETER( pContext );
#endif

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("+%s(%s)\r\n"), __WFUNCTION__, pContext));

    // Initialize the PMIC driver
    if (!PowerInit())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("%s:  Failed to initialize PMIC!!!\r\n"),
            __WFUNCTION__));
        goto cleanUp;
    }

    rc = 1;

cleanUp:
    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("-%s() returning 1\r\n"), __WFUNCTION__));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Deinit
//
// The Device Manager calls this function as a result of a call to the
// DeactivateDevice() function.  This function will return any resources
// allocated while using this driver.
//
// Parameters:
//      hDeviceContext
//          [in] The handle to the context.
//
// Returns:
//      TRUE
//
//-----------------------------------------------------------------------------
BOOL
PMI_Deinit(DWORD hDeviceContext)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER( hDeviceContext );
#endif

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("+%s(%d)\r\n"), __WFUNCTION__, hDeviceContext));

    PowerDeInit();

    DEBUGMSG(ZONE_INIT||ZONE_FUNC,
                (TEXT("-%s() returning %d\r\n"), __WFUNCTION__, FALSE));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Open
//
// Called when an application attempts to establish a connection to this driver
// This function will verify that a trusted application has made the request
// and deny access to all non-trusted applications.
//
// Parameters:
//      hDeviceContext
//          [in] Ignored
//      AccessCode
//          [in] Ignored
//      ShareMode
//          [in] Ignored
//
// Returns:
//      Returns 0 if the calling application is not trusted and 1 if it is
//      trusted.
//
//-----------------------------------------------------------------------------
DWORD
PMI_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER( hDeviceContext );
    UNREFERENCED_PARAMETER( AccessCode );
    UNREFERENCED_PARAMETER( ShareMode );
#endif

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d, %d, %d)\r\n"), __WFUNCTION__,
        hDeviceContext, AccessCode, ShareMode));

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() returning 1\r\n"), __WFUNCTION__));

    return 1;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_Close
//
// Called when an application attempts to close a connection to this driver.
// This function does nothing.
//
// Parameters:
//      hOpenContext
//          [in] Ignored
//
// Returns:
//      TRUE
//
//-----------------------------------------------------------------------------
BOOL
PMI_Close(DWORD hOpenContext)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER( hOpenContext );
#endif

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s(%d)\r\n"), __WFUNCTION__, hOpenContext));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s() returning TRUE\r\n"), __WFUNCTION__));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: PMI_IOControl
//
// Called when an application calls the DeviceIoControl() function.  This
// function operates differently based upon the IOCTL that is passed to it.
// The following table describes the expected values associated with each
// IOCTL implemented by this function.
//
// dwCode                      pBufIn         pBufOut         Description
// --------------------------- -------------- --------------- -----------------
//
// Parameters:
//      hOpenContext
//          [in] Ignored
//
//      dwCode
//          [in] The IOCTL requested.
//
//      pBufIn
//          [in] Input buffer.
//
//      dwLenIn
//          [in] Length of the input buffer.
//
//      pBufOut
//          [out] Output buffer.
//
//      dwLenOut
//          [out] The length of the output buffer.
//
//      pdwActualOut
//          [out] Size of output buffer returned to application.
//
// Returns:
//      TRUE if the IOCTL is handled. FALSE if the IOCTL was not recognized or
//      an error occurred while processing the IOCTL
//
//-----------------------------------------------------------------------------
BOOL
PMI_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    BOOL result = FALSE;
    UINT32 temp;
    PPOWER_RAIL_VALUE prv;

    UNREFERENCED_PARAMETER( hOpenContext );
    UNREFERENCED_PARAMETER( dwLenIn );

    switch (dwCode)
    {
    case PMU_IOCTL_BATT_MONITOR_INIT:
        PmuIoctlInitBatteryMonitor();        
        result = TRUE;
        break;

    case PMU_IOCTL_BATT_CHARGE_SET:
        if (pBufIn != NULL)
        {
            result = PmuIoctlSetCharger(*(UINT32 *)pBufIn);
        }
        break;

    case PMU_IOCTL_BATT_CHARGE_STOP:        
        PmuIoctlStopCharger();
        result = TRUE;
        break;

    case PMU_IOCTL_BATT_CHARGE_GET_STATUS:
        if (pBufOut != NULL)
        {
            *(BOOL*)pBufOut = PmuIoctlGetBatteryChargingStatus();
            dwLenOut = sizeof(BOOL);

            result = TRUE;
        }
        break;

    case PMU_IOCTL_BATT_GET_VOLTAGE:
        if (pBufOut != NULL)
        {        
            *(UINT32*)pBufOut = PmuIoctlGetBatteryVoltage();
            dwLenOut = sizeof(UINT32);

            result = TRUE;
        }
        break;

    case PMU_IOCTL_VDDD_GET_VOLTAGE:
        if (pBufOut != NULL)
        {        
            *(UINT32*)pBufOut = PmuIoctlGetVddd();
            dwLenOut = sizeof(UINT32);

            result = TRUE;
        }
        break;

	//LQK:Jul-12-1012 
	case PMU_IOCTL_GET_POWER_SOURCE:
		if( pBufOut != NULL )
		{
			*(UINT32*)pBufOut = Is5VFromVbus();
			dwLenOut = sizeof(UINT32);

			result = TRUE;
		}
		break;

	//LQK:Jul-25-2012
	case PMU_IOCTL_BATTERY_ATTACH:
		if( pBufOut != NULL )
		{
			*(BOOL*)pBufOut = IsBatteryAttach();
			dwLenOut = sizeof(BOOL);

			result = TRUE;
		}
		break;

	case PMU_IOCTL_VDDD_GET_BRNOUT:
        if (pBufOut != NULL)
        {        
            *(UINT32*)pBufOut = PmuIoctlGetVdddBrownont();
            dwLenOut = sizeof(UINT32);

            result = TRUE;
        }
        break;

    case PMU_IOCTL_VDDD_SET_VOLTAGE:
        if (pBufIn != NULL)
        {        
            prv = (POWER_RAIL_VALUE *) pBufIn;
        
            PmuIoctlSetVddd(prv->TargetmV, prv->BrownOutmV);
        
            result = TRUE;
        }
        break;

    case PMU_IOCTL_FET_SET_MODE:
        if (pBufIn != NULL)
        {        
            temp = *pBufIn;
        
            PmuIoctlSetFets(*pBufIn);
        
            result = TRUE;
        }
        break;  

    case PMU_IOCTL_POWER_SUPPLY_MODE_GET:
        if (pBufOut != NULL)
        {        
            *(PMU_POWER_SUPPLY_MODE *)pBufOut = PmuIoctlGet5vPresentFlag();
        
            result = TRUE;
        }
        break;  

    case PMU_IOCTL_POWER_THERMAL_SET:
        if (pBufIn != NULL)
        {        
            PmuIoctlThermalSet(*(PMU_POWER_THERMAL_TEMP *)pBufIn);
        
            result = TRUE;
        }
        break;
    
    case PMU_IOCTL_POWER_THERMAL_GET:
        if (pBufOut != NULL)
        {        
            *(UINT32*)pBufOut = PmuIoctlThermalGet();        
        
            result = TRUE;
        }
        break;

    case PMU_IOCTL_POWER_THERMAL_ENABLE:
        HW_POWER_THERMAL_CLR(BM_POWER_THERMAL_PWD);
        
        result = TRUE;
        break;

    case PMU_IOCTL_POWER_THERMAL_DISABLE:
        HW_POWER_THERMAL_SET(BM_POWER_THERMAL_PWD);
        
        result = TRUE;
        break;        

    case IOCTL_POWER_CAPABILITIES:
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
                result = TRUE;
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                ERRORMSG(TRUE, (_T("Exception in IOCTL_POWER_CAPABILITIES\r\n")));
            }
        }

        break;

    case IOCTL_POWER_SET:
        if(pBufOut != NULL
            && dwLenOut == sizeof(CEDEVICE_POWER_STATE)
            && pdwActualOut != NULL)
        {
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try
            {
                CEDEVICE_POWER_STATE dx = *(PCEDEVICE_POWER_STATE) pBufOut;
                if(VALID_DX(dx))
                {
                    // Any request that is not D0 becomes a D4 request
                    if (dx != D0 && dx != D1)
                    {
                        dx = D4;
                    }                

                    *(PCEDEVICE_POWER_STATE) pBufOut = dx;
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    result = TRUE;
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE, (_T("Exception in IOCTL_POWER_SET\r\n")));
            }
        }
        break;

    case IOCTL_POWER_GET:
        if(pBufOut != NULL
            && dwLenOut == sizeof(CEDEVICE_POWER_STATE)
            && pdwActualOut != NULL) {
            // Just return our current Dx value
            PREFAST_SUPPRESS(6320, "Generic exception handler");
            __try
            {
                *(PCEDEVICE_POWER_STATE) pBufOut = D0;
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                result = TRUE;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ERRORMSG(TRUE, (_T("Exception in IOCTL_POWER_SET\r\n")));
            }
        }
        break;

    default:
        RETAILMSG(1,(TEXT("%s: Unrecognized IOCTL 0x%X\r\n"), __WFUNCTION__, dwCode));
        result = FALSE;
        break;
    }

    //DEBUGMSG(ZONE_FUNC,
    //            (TEXT("-%s(0x%X) returning %d\r\n"), __WFUNCTION__, dwCode, result));

    return result;
}


//-----------------------------------------------------------------------------
//
// Function: PMI_PowerUp
//
// This function restores power to a device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PMI_PowerUp(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER( hDeviceContext );
}


//-----------------------------------------------------------------------------
//
// Function: PMI_PowerDown
//
// This function suspends power to the device.
//
// Parameters:
//      hDeviceContext
//          [in] Handle to the device context.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void PMI_PowerDown(DWORD hDeviceContext)
{
    UNREFERENCED_PARAMETER( hDeviceContext );
}


//-----------------------------------------------------------------------------
//
//  Function: VDDDBOHandler
//
//  This function handles the VDDD brownbout interrupt
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL VDDDBOHandler(void)
{
    BOOL bValue = TRUE; 
    do {
           WaitForSingleObject (g_hVDDDBNOTEvent, INFINITE);

           //VDDD BROWN
           //RETAILMSG(1, (TEXT("VDDDBOHandler VDDD BROWN...\r\n")));
           if(HW_POWER_STS.B.VDDD_BO)
           {
               StallExecution(100);
               if(HW_POWER_STS.B.VDDD_BO)
                   PowerOffChip();
           }
           
           HW_POWER_CTRL_CLR(BM_POWER_CTRL_VDDD_BO_IRQ);     
           InterruptDone(SysIntrVDDD);    
           
        }while(bValue);
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: VDDIOBOHandler
//
//  This function handles the VDDIO brownbout interrupt
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL VDDIOBOHandler(void)
{
    BOOL bValue = TRUE; 
    do {
           WaitForSingleObject (g_hVDDIOBNOTEvent, INFINITE);

           //VDDIO BROWN
           //RETAILMSG(1, (TEXT("VDDIOBOHandler VDDIO BROWN...\r\n")));
           if(HW_POWER_STS.B.VDDIO_BO)
           {
               StallExecution(100);
               if(HW_POWER_STS.B.VDDIO_BO)
                   PowerOffChip();
           }
           HW_POWER_CTRL_CLR(BM_POWER_CTRL_VDDIO_BO_IRQ);               
           InterruptDone(SysIntrVDDIO);    
           
        }while(bValue);
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: VDDABOHandler
//
//  This function handles the VDDA brownbout interrupt
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL VDDABOHandler(void)
{
    BOOL bValue = TRUE; 
    do {
           WaitForSingleObject (g_hVDDABNOTEvent, INFINITE);

           //VDDA BROWN
           //RETAILMSG(1, (TEXT("VDDABOHandler VDDA BROWN...\r\n")));
           if(HW_POWER_STS.B.VDDA_BO)
           {
               StallExecution(100);
               if(HW_POWER_STS.B.VDDA_BO)
                   PowerOffChip();
           }
           HW_POWER_CTRL_CLR(BM_POWER_CTRL_VDDA_BO_IRQ);               
           InterruptDone(SysIntrVDDA);    
           
        }while(bValue);
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: PowerIRQHandler
//
//  This function handles the power transitions between the Battery/5V/USB
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerIRQHandler(void)
{

    BOOL bValue = TRUE; 

    UINT32 PSwitchStartTime;
    do {
           //RETAILMSG(1, (TEXT("PowerIRQHandler wait for event\r\n")));
           WaitForSingleObject(g_h5VIntEvent, INFINITE);
           //check if we can use polling method
           // VDD5V_GT_VDDIO_IRQ, DC_OK_IRQ,VBUSVALID_IRQ, LINEREG_OK_IRQ and PSWITCH_IRQ all
           // share a single interrupt line.
           // Now check which interrupts brings us here
           Sleep(10);

           //RETAILMSG(1, (TEXT("PowerIRQHandler +++++++++++++++\r\n")));

           //Pswitch IRQ
           if(HW_POWER_CTRL.B.PSWITCH_IRQ)
           {               
               //RETAILMSG(1, (TEXT("PSWITCH_IRQ ++\r\n")));
               if(HW_POWER_STS.B.PSWITCH == ((BM_POWER_STS_PSWITCH >> BP_POWER_STS_PSWITCH) & 0x1))
               {
                   //PSWITCH MID LEVEL
                   //RETAILMSG(1, (TEXT("PSWITCH_IRQ MID\r\n")));                   
                   PSwitchStartTime = HW_DIGCTL_MICROSECONDS_RD();
    
                   while((HW_POWER_STS.B.PSWITCH == ((BM_POWER_STS_PSWITCH >> BP_POWER_STS_PSWITCH) & 0x1)) 
                              && (HW_DIGCTL_MICROSECONDS_RD() - PSwitchStartTime < POWER_OFF_HOLD_TIME))
                       Sleep(30);
				   
				   //Power Off Chip
				   //if(HW_DIGCTL_MICROSECONDS_RD() - PSwitchStartTime >= POWER_OFF_HOLD_TIME)
				   //	 PowerOffChip();
				   //
				   // JLY05-2012: LQK
				   //
				   if( PowerGet5vPresentFlag() == PMU_POWER_SUPPLY_BATTERY )
				   {
						//Power Off Chip
						if(HW_DIGCTL_MICROSECONDS_RD() - PSwitchStartTime >= POWER_OFF_HOLD_TIME)
							PowerOffChip();
				   }

                   //Clear PSwitch interrupt
                   HW_POWER_CTRL_CLR(BM_POWER_CTRL_PSWITCH_IRQ);                   
                   InterruptDone(SysIntr5V);
                   
                   // ZZZzzz...
                   //SetSystemPowerState(NULL,POWER_STATE_SUSPEND,0);
					//LQK:Jul-12-2012
				   if( PowerGet5vPresentFlag() == PMU_POWER_SUPPLY_BATTERY )
				   {
						SetSystemPowerState(NULL,POWER_STATE_SUSPEND,0);
				   }
               }
               else
               {
                   //PSWITCH High LEVEL
                   //RETAILMSG(1, (TEXT("PSWITCH_IRQ High\r\n")));
                   HW_POWER_CTRL_CLR(BM_POWER_CTRL_PSWITCH_IRQ);                   
                   InterruptDone(SysIntr5V);
               }
               
               continue;
           }
           else if(HW_POWER_CTRL.B.VBUSVALID_IRQ || HW_POWER_CTRL.B.VDD5V_GT_VDDIO_IRQ)
           {
               //5V IRQ
               if (PowerGet5vPresentFlag() == PMU_POWER_SUPPLY_5V)
               {   
                   //RETAILMSG(1, (TEXT("PowerIRQHandler :: 5V\r\n")));
                   if (HW_POWER_5VCTRL.B.PWD_CHARGE_4P2)
                   {
                       if(PowerGetLimit())
                       {
                           while(PowerGetLimit())
                               Sleep(20); 
                       }
                       // It's 5V mode, enable 5V-to-battery handoff.
                       PowerExecuteBatteryTo5VoltsHandoff();
                       PowerEnable5VoltsToBatteryHandoff();
                   }
               }
               else
               {  
                   //RETAILMSG(1, (TEXT("PowerIRQHandler :: BATT\r\n")));       
                   // It's battery mode, enable battery-to-5V handoff.
                   PowerExecute5VoltsToBatteryHandoff();
                   PowerEnableBatteryTo5VoltsHandoff();
                   if(IsUSBDeviceDriverEnable())
                   {
                       // Set current limitation.
                       BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x20);
                       HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_ENABLE_LINREG_ILIMIT);
                   }
               }

               //Clear IRQ
               HW_POWER_CTRL_CLR(BM_POWER_CTRL_VDD5V_GT_VDDIO_IRQ | BM_POWER_CTRL_VBUSVALID_IRQ);
           }
           else
           {
               //RETAILMSG(1, (TEXT("PowerIRQHandler IRQ Not processed! POWER_CTRL=%X\r\n"),HW_POWER_CTRL_RD()));             
               PowerClear5VIrq();
           }
           //RETAILMSG(1, (TEXT("PowerIRQHandler :: g_h5VIntEvent occured\r\n")));

           //Notify battery to update state
           SetEvent(g_h5VIntEventNotify);
           
           // Enable the interrupt
           InterruptDone(SysIntr5V);
           //RETAILMSG(1, (TEXT("PowerIRQHandler-------------------\r\n")));
           
    } while(bValue);

    //RETAILMSG(1, (TEXT("PowerIRQHandler --\r\n")));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: PowerInitHandle
//
//  This function handles the power transitions between the Battery/5V/USB
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful else returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL PowerInitHandle(void)
{
    if(PowerGetLimit())
    {
        RETAILMSG(TRUE,(TEXT("Power initialization, 100mA current limited!\r\n"))); 
        while(PowerGetLimit())
            Sleep(1000);
        RETAILMSG(TRUE,(TEXT("Power initialization, 100mA current limitation release!\r\n")));
    }

    if(bPLL == TRUE)
    {
        PowerClear5VIrq();
        return TRUE;
    }
    //PowerInitPowerSupplies();

    if (PowerGet5vPresentFlag() == PMU_POWER_SUPPLY_5V)
    {
        if((HW_POWER_DCDC4P2_RD() & BM_POWER_DCDC4P2_ENABLE_4P2) == 0)
        {
            // It's 5V mode, enable 5V-to-battery handoff.
            PowerExecuteBatteryTo5VoltsHandoff();
            PowerEnable5VoltsToBatteryHandoff();
        }
    }
    else
    {
        // It's battery mode, enable battery-to-5V handoff.
        PowerExecute5VoltsToBatteryHandoff();
        PowerEnableBatteryTo5VoltsHandoff();
    }

    PowerClear5VIrq();

    return TRUE;
}

// LQK:Jul-25-2012
// Returns:
//      TRUE if the battery is attached; FALSE if battery is not available
static BOOL IsBatteryAttach( )
{
	hw_power_battmonitor_t power_battmonitor_reg;
	UINT32 BattVoltage ;

	BattVoltage = PmuIoctlGetBatteryVoltage( );

	if((BattVoltage > BATTERY_LOW) && (BattVoltage < BATTERY_HIGH))
	{
		// power_battmonitor_reg.B.BRWNOUT_LVL was setted by xldr.c
		// if BRWNOUT_LVL is i6, battery is not available
		// if BRWNOUT_LVL is 15, battery is attached 
		power_battmonitor_reg.U = HW_POWER_BATTMONITOR_RD();
		if( power_battmonitor_reg.B.BRWNOUT_LVL == 15 )
			return TRUE;
	}

	return FALSE;

}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the PMIC control module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER( lpvReserved );

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DEBUGREGISTER((HMODULE)hInstDll);
            DisableThreadLibraryCalls((HMODULE) hInstDll);

            DEBUGMSG(ZONE_INIT,
                (_T("***** DLL PROCESS ATTACH TO PMIC *****\r\n")));

            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    // Return TRUE for success
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  DumpPowerRegisters
//
//  This function dumps the power registers, used for debugging
//
//  Parameters:
//          None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID DumpPowerRegisters(VOID)
{
    //Dump All the Power registers
    RETAILMSG(1, (TEXT("HW_POWER_CTRL = 0x%X \r\n"),HW_POWER_CTRL_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_5VCTRL = 0x%X \r\n"),HW_POWER_5VCTRL_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_MINPWR = 0x%X \r\n"),HW_POWER_MINPWR_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_CHARGE = 0x%X \r\n"),HW_POWER_CHARGE_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_VDDDCTRL = 0x%X \r\n"),HW_POWER_VDDDCTRL_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_VDDACTRL = 0x%X \r\n"),HW_POWER_VDDACTRL_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_VDDIOCTRL = 0x%X \r\n"),HW_POWER_VDDIOCTRL_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_VDDMEMCTRL = 0x%X \r\n"),HW_POWER_VDDMEMCTRL_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_DCDC4P2 = 0x%X \r\n"),HW_POWER_DCDC4P2_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_MISC = 0x%X \r\n"),HW_POWER_MISC_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_DCLIMITS = 0x%X \r\n"),HW_POWER_DCLIMITS_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_LOOPCTRL = 0x%X \r\n"),HW_POWER_LOOPCTRL_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_STS = 0x%X \r\n"),HW_POWER_STS_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_SPEED = 0x%X \r\n"),HW_POWER_SPEED_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_BATTMONITOR = 0x%X \r\n"),HW_POWER_BATTMONITOR_RD()));
    RETAILMSG(1, (TEXT("HW_POWER_RESET = 0x%X \r\n"),HW_POWER_RESET_RD()));    
    RETAILMSG(1, (TEXT("HW_POWER_SPECIAL_RD = 0x%X \r\n"),HW_POWER_SPECIAL_RD())); 

}

