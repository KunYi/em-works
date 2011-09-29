//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_power.cpp
//
//  This file contains a power interface for the Battery module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"
#include "power.h"
#include "regspower.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static BOOL g_bInitialised = FALSE;
static POWER_5VDETECTION DetectionMethod;
PVOID pv_HWregPOWER = NULL;
PVOID pv_HWregUSBPhy = NULL;
static PVOID pv_HWregDIGCTL = NULL;
POWER_POWERSOURCE VdddPowerSource;
BOOL bExternalPowerSourceActive = FALSE;
BOOL bPLL = FALSE;
#ifdef STMP378x
static BOOL b4p2Enabled = TRUE;
#endif

//-----------------------------------------------------------------------------
// Local Functions
UINT16 PowerConvertVdddToSetting(UINT16 u16Vddd);
UINT16 PowerConvertSettingToVddd(UINT16 u16Setting);


//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerInit
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
BOOL DDKPowerInit(POWER_INITVALUES* pInitValues)
{
    BOOL rc = FALSE;
    
    ULONG Status = 0;
    POWER_5VDETECTION eDetection;

    //use it from the parameter list
    RETAILMSG(1, (TEXT("PowerInit ++ \r\n")));
    if(!g_bInitialised)
    {
        //Map the peripheral register space of the power module
        if (!DDKPowerAlloc())
        {
            ERRORMSG(1, (_T("PowerInit :: DDKPowerAlloc() failed!")));
            goto cleanUp;
        }
        rc = TRUE;
    }

    if((HW_POWER_5VCTRL_RD() & 0x40) && ((HW_POWER_5VCTRL_RD() & 0x3F000) == 0x8000))
    {    
        RETAILMSG(1, (TEXT("Wait USB enumeration done... \r\n"))); 
        bPLL = TRUE;
    }
    
    while((HW_POWER_5VCTRL_RD() & 0x40) && ((HW_POWER_5VCTRL_RD() & 0x3F000) == 0x8000))
        Sleep(100);
    
    BF_CLR(POWER_CTRL, CLKGATE);

    //DumpPowerRegisters();
    // Improve efficieny and reduce transient ripple
    BF_SET(POWER_LOOPCTRL, TOGGLE_DIF);

    BF_SET(POWER_LOOPCTRL, EN_CM_HYST);
    BF_SET(POWER_LOOPCTRL, EN_DF_HYST);
    BF_WR(POWER_DCLIMITS, POSLIMIT_BUCK, 0x30); 
    
    if(bPLL == TRUE)
        DDKPowerInitCpuHclkClock();     
    // Initialize 5V/USB detection.
    // Parse the init structure for the 5V detection method.

    eDetection = (POWER_5VDETECTION) pInitValues->e5vDetection;

    // Initialize the 5V or USB insertion/removal detection.  Does not
    // prepare the 5V-DCDC/DCDC-5V handoff.

    if((Status = DDKPowerEnable5vDetection(eDetection)) != 0)
        return Status;
        
     rc = TRUE;
    //DDKPowerInitSupply();
    
cleanUp:
    if(!rc)
        DDKPowerDeInit();

    RETAILMSG(1, (TEXT("PowerInit -- \r\n")));
    HW_POWER_CTRL_SET(BM_POWER_CTRL_ENIRQ_VDD5V_GT_VDDIO);
    HW_POWER_CTRL_SET(BM_POWER_CTRL_POLARITY_PSWITCH | BM_POWER_CTRL_ENIRQ_PSWITCH);

    //Enable double FETs.
    HW_POWER_MINPWR_CLR(BM_POWER_MINPWR_HALF_FETS);
    HW_POWER_MINPWR_SET(BM_POWER_MINPWR_DOUBLE_FETS);

    //DumpPowerRegisters();

    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerDeInit
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
BOOL DDKPowerDeInit(VOID)
{
    
    //Deallocate the resources
    DDKPowerDealloc();

    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerAlloc
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
BOOL DDKPowerAlloc(VOID)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    //RETAILMSG(1, (TEXT("PowerAlloc ++ \r\n")));

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
    if (pv_HWregUSBPhy == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY;

        // Map peripheral physical address to virtual address
        pv_HWregUSBPhy = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregUSBPhy == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace pv_HWregUSBPhy failed!\r\n")));
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


    
    //RETAILMSG(1, (TEXT("PowerAlloc -- \r\n")));
    rc = TRUE;
    g_bInitialised = TRUE;
    
cleanUp:
    if (!rc) DDKPowerDealloc();
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerDealloc
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
BOOL DDKPowerDealloc(VOID)
{
    // Unmap peripheral registers
    if (pv_HWregPOWER)
    {
        MmUnmapIoSpace(pv_HWregPOWER, 0x1000);
        pv_HWregPOWER = NULL;
    }
    g_bInitialised = FALSE;
    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerInitBatteryMonitor
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
//          returns 0 if successful else returns error values.
//
//-----------------------------------------------------------------------------
UINT32 DDKPowerInitBatteryMonitor(LRADC_DELAYTRIGGER eTrigger, UINT32 SamplingInterval)
{
  

    UNREFERENCED_PARAMETER(eTrigger);
    UNREFERENCED_PARAMETER(SamplingInterval);
    RETAILMSG(1, (TEXT("PowerInitBatteryMonitor ++\r\n")));
   
    // Update the DCFUNCV register for battery adjustment
    //PowerUpdateDcFuncvVddd();     // function not present
    //PowerUpdateDcFuncvVddio();

    // Finally enable the battery adjust
    BF_SET(POWER_BATTMONITOR,EN_BATADJ);

    // 4X increase to transient load response.  Enable this after the
    // battery voltage is correct.
    if(DDKPowerGetBatteryMode() == HW_POWER_BATT_MODE_ALKALINE_NIMH)
    {
        // Set some bits to help alkaline mode.
        BF_SET(POWER_MINPWR, DOUBLE_FETS);
        BF_WR(POWER_LOOPCTRL, DC_R, 0x3);
        DDKPowerEnableRcScale(HW_POWER_RCSCALE_4X_INCR);
    }
    else
    {
        // The following settings give optimal power supply capability and
        // efficiency.  Extreme loads will need HALF_FETS cleared and
        // possibly DOUBLE_FETS set.  The below setting are probably also
        // the best for alkaline mode also but more characterization is
        // needed to know for sure.

        // Increase the RCSCALE_THRESHOLD
        BF_SET(POWER_LOOPCTRL, RCSCALE_THRESH);

        // Increase the RCSCALE level for quick DCDC response to dynamic load
        DDKPowerEnableRcScale(HW_POWER_RCSCALE_8X_INCR);

        // Enable half fets for increase efficiency.
        DDKPowerEnableHalfFets(TRUE);
    }

    RETAILMSG(1, (TEXT("PowerInitBatteryMonitor --\r\n")));
    return 0;
}


//-----------------------------------------------------------------------------
//
//  Function: DDKPowerEnableRcScale
//
//  This routine initializes the Battery monitor for battery module.
//
//  Parameters:
//          eLevel
//              [in]  Specifies the RCSCALE level
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableRcScale(POWER_RCSCALELEVELS eLevel)
{
    BF_WR(POWER_LOOPCTRL, EN_RCSCALE, eLevel);
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerEnableHalfFets
//
//  This routine initializes the HalfFets, used for saving power.
//
//  Parameters:
//          bEnable
//              [in]  TRUE for enabling and FALSE for disabling the HalfFets
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableHalfFets(BOOL bEnable)
{
    //RETAILMSG(1,(TEXT("PowerEnableHalfFets ++\r\n")));
    if(bEnable)
    {
        BF_SET(POWER_MINPWR, HALF_FETS);
    }
    else
    {
        BF_CLR(POWER_MINPWR, HALF_FETS);
    }
    //RETAILMSG(1,(TEXT("PowerEnableHalfFets --\r\n")));
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerEnable5vDetection
//
//  This routine enables the 5V DDKPower detection.
//
//  Parameters:
//          eDetection
//              [in]  Possible 5V detection methods
//
//  Returns:
//          reurn 0 if TRUE otherwise an error value.
//
//-----------------------------------------------------------------------------
UINT32 DDKPowerEnable5vDetection(POWER_5VDETECTION eDetection)
{
    // Set the detection method for all the 5V detection calls
    DetectionMethod = eDetection;
    //RETAILMSG(1,(TEXT("PowerEnable5vDetection ++\r\n")));
    // Disable hardware 5V removal powerdown
    BF_CLR(POWER_5VCTRL, PWDN_5VBRNOUT);

    // Prepare the hardware for the detection method.  We used to set and clear
    // the VBUSVALID_5VDETECT bit, but that is also used for the DCDC 5V
    // detection.  It is sufficient to just check the status bits to see if
    // 5V is present.

    // Use VBUSVALID for DCDC 5V detection.  The DCDC's detection is
    // different than the USB/5V detection used to switch profiles.  This
    // is used to determine when a handoff should occur.
    BF_SET(POWER_5VCTRL, VBUSVALID_5VDETECT);

    // Set 5V detection threshold to normal level for VBUSVALID.
#ifdef STMP378x
    // For TA1 silicon, the VBUSVALID threshold levels aren't matching up with
    // what is specified in the datasheet.  The value of 6 appears to be more
    // like ~4.25V.  We'll use it for now.
    //    DDKPowerSetVbusValidThresh((hw_power_VbusValidThresh_t)6);
    //    BF_WR(POWER_5VCTRL, VBUSVALID_TRSH, eThresh);
    //RETAILMSG(1,(TEXT("PowerEnable5vDetection :: STMP378x++\r\n")));
    BF_WR(POWER_5VCTRL, VBUSVALID_TRSH, 1);
#else
    //hw_power_SetVbusValidThresh(VBUS_VALID_THRESH_NORMAL);
    //RETAILMSG(1,(TEXT("PowerEnable5vDetection :: STMP37xx++\r\n")));
    BF_WR(POWER_5VCTRL, VBUSVALID_TRSH, VBUS_VALID_THRESH_NORMAL);
#endif

    //Enable the 5V detect
    DDKPowerEnableVbusValid5vDetect(TRUE);
    //RETAILMSG(1,(TEXT("PowerEnable5vDetection --\r\n")));

    return 0;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerInit4p2EnabledPowerSupply
//
//  This routine initializes hardware specific registers for enabling 4p2 power supply.
//
//  Parameters:
//          None.
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
#ifdef STMP378x
VOID DDKPowerInit4p2EnabledPowerSupply(VOID)
{

    // Make sure the power supplies are configured for their power sources.
    // This also sets the LINREG_OFFSET field correctly for each power supply.

    // initial setup, only needs to be called once.
    //RETAILMSG(1,(TEXT("PowerInit4p2EnabledPowerSupply ++\r\n")));

    HW_POWER_DCDC4P2.B.CMPTRIP = 0;

    BF_CS1(POWER_DCDC4P2, TRG, 0x0);   // 4p2 =4.2V
    BF_CS1(POWER_5VCTRL, HEADROOM_ADJ, 0x4);
    BF_CS1(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x24);  //450mA +/-10% limit


    BF_CS1(POWER_DCDC4P2, DROPOUT_CTRL, 0xA);  //100mV drop before stealing charging current

    //RETAILMSG(1,(TEXT("PowerInit4p2EnabledPowerSupply --\r\n")));
}
#endif
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerStart4p2
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
UINT32 DDKPowerStart4p2(VOID)
{
    UINT32 Status;
    //RETAILMSG(1,(TEXT("PowerStart4p2 ++\r\n")));

    if(HW_POWER_STS.B.VBUSVALID)
    {
        Status = DDKPowerStart4p2HW();
        while(HW_POWER_CTRL.B.DCDC4P2_BO_IRQ)
        {
            HW_POWER_CTRL_CLR(BM_POWER_CTRL_DCDC4P2_BO_IRQ);
            DDKPowerStart4p2HW();
        }
    }
    else
    {   
        // for 4p2 mode enabled, the os level driver will control the handoff
        // from DCDC battery to DCDC 4p2 on a 5V connection.  We do not
        // want automatic handoffs to linear regulator.

        DDKPowerEnableDcdc(TRUE);
        // For LiIon battery, all the rails can start off as DCDC mode.
        if((Status = DDKPowerSetVdddPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
            return Status;
        if((Status = DDKPowerSetVddaPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
            return Status;
        if((Status = DDKPowerSetVddioPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
            return Status;
    }

    if(Status==0)
        // This is a logical, not physical state.  Todo: possibly re-work this
        b4p2Enabled = TRUE;
    else
        b4p2Enabled = FALSE;

    //RETAILMSG(1,(TEXT("PowerStart4p2 --\r\n")));
    return Status;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerStart4p2
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
BOOL DDKPowerStart4p2HW(VOID)
{
    UINT32 i = 0;
    UINT32 u32Chargetime = 0;
    //RETAILMSG(1,(TEXT("PowerStart4p2HW ++\r\n")));

    if((DDKPowerSetVdddPowerSource(HW_POWER_DCDC_LINREG_OFF)) != 0)
        return FALSE;
    if((DDKPowerSetVddaPowerSource(HW_POWER_DCDC_LINREG_OFF)) != 0)
        return FALSE;
    if((DDKPowerSetVddioPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
        return FALSE;
    // We must first clear this bit and then set it again due to bug with the
    // hardware.  If the hardware has at anytime seen a 5V removal, 4p2 is
    // powered down internal even though this bit remains set in the registers
    BF_SET(POWER_5VCTRL, PWD_CHARGE_4P2);
    BF_CLR(POWER_5VCTRL, PWD_CHARGE_4P2);

    HW_POWER_5VCTRL_CLR(0x3F000);
    HW_POWER_5VCTRL_SET(0x1000);//set limit current to 10mA

    //charge 4P2 capacity to 4.2V
    i = 1;
    while(i <= 0x3F)
    {
        BF_WR(POWER_5VCTRL,CHARGE_4P2_ILIMIT,i);
        i++;
        u32Chargetime = HW_DIGCTL_MICROSECONDS_RD();
        while(HW_DIGCTL_MICROSECONDS_RD() - u32Chargetime < 15000);
    }

    // This must be used if there is no static load on 4p2 as 4p2 will
    // become unstable with no load.
    BF_SET(POWER_CHARGE,ENABLE_LOAD);

    BF_SET(POWER_DCDC4P2, ENABLE_4P2);      //enable 5V to 4p2 regulator

    // for transitioning from lin reg mode for Vddd, Vddio, or Vdda
    // this might make things worse!
    //BF_SET(POWER_5VCTRL, DCDC_XFER);

    BF_SET(POWER_5VCTRL, PWRUP_VBUS_CMPS);
    BF_CLR(POWER_CTRL,ENIRQ_DCDC4P2_BO);

    BF_SET(POWER_DCDC4P2, ENABLE_DCDC);      //enable DCDC 4P2 capability

    // on one particular set of tests on one board, a wait of 2us failed,
    // a wait of 5us passed.  To be safe, waiting 100uS
    //hw_digctl_MicrosecondWait(100);

    // this actually isn't the only thing needed because you can't enable DCDC for 4p2
    // until in the circuiry is ready, regardless of what voltage may be present on
    // 4p2
    while(HW_POWER_STS.B.VBUSVALID && HW_POWER_CTRL.B.DCDC4P2_BO_IRQ)
    {
        BF_CLR(POWER_CTRL, DCDC4P2_BO_IRQ);
        //hw_digctl_MicrosecondWait(10);
    }

    //disable FET
    HW_POWER_VDDDCTRL_CLR(0x200000);
    HW_POWER_VDDDCTRL_CLR(0x100000);
    HW_POWER_VDDACTRL_CLR(0x10000);
    HW_POWER_VDDIOCTRL_CLR(0x10000);
    
    //BO offset
    HW_POWER_VDDIOCTRL_CLR(0xF00);
    HW_POWER_VDDIOCTRL_SET(0x400);
    HW_POWER_VDDDCTRL_CLR(0xF00);
    HW_POWER_VDDDCTRL_SET(0x400);
    HW_POWER_VDDACTRL_CLR(0xF00);
    HW_POWER_VDDACTRL_SET(0x400);
    
    HW_POWER_5VCTRL_SET(0x3F000);

    // enables DCDC during 5V connnection (for both battery or 4p2 powered DCDC)
    BF_SET(POWER_5VCTRL, ENABLE_DCDC);       // Enable the DCDC.

    //RETAILMSG(1,(TEXT("PowerStart4p2HW --\r\n")));
    return 0;

}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerSetVdddPowerSource
//
//  This routine Sets up the Vddd DDKPowerSource.
//
//  Parameters:
//          eSource
//              [in] power sources for power supply
//
//  Returns:
//          returns 0 if successful otherwise returns error value.
//
//-----------------------------------------------------------------------------
UINT32 DDKPowerSetVdddPowerSource(POWER_POWERSOURCE eSource)
{
    UINT32 rtn = 0;
    VdddPowerSource = eSource;
    //RETAILMSG(1,(TEXT("PowerSetVdddPowerSource ++\r\n")));
    // The VDDD can use two power sources in three configurations: the Linear
    // Regulator, the DCDC with LinReg off, and the DCDC with LinReg on.
    // Each has its own configuration that must be set up to
    // prevent power rail instability.
    switch(eSource)
    {
    // DDKPower the VDDD rail from DCDC.  LinReg is off.
    case HW_POWER_DCDC_LINREG_OFF:
    case HW_POWER_DCDC_LINREG_READY:

        // Use LinReg offset for DCDC mode.
        BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDD DCDC output and turn off the VDDD LinReg output.
        BF_CLR(POWER_VDDDCTRL, DISABLE_FET);
        BF_CLR(POWER_VDDDCTRL, ENABLE_LINREG);

        // Make sure stepping is enabled when using DCDC.
        BF_CLR(POWER_VDDDCTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDD rail from DCDC.  LinReg is on.
    case HW_POWER_DCDC_LINREG_ON:
        // Use LinReg offset for DCDC mode.
        BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDD DCDC output and turn on the VDDD LinReg output.
        BF_CLR(POWER_VDDDCTRL, DISABLE_FET);
        BF_SET(POWER_VDDDCTRL, ENABLE_LINREG);

        // Make sure stepping is enabled when using DCDC.
        BF_CLR(POWER_VDDDCTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDD rail from the linear regulator.  The DCDC is not
    // set up to automatically power the chip if 5V is removed.  Use this
    // configuration when battery is powering the player, but we want to
    // use LinReg instead of DCDC.
    case HW_POWER_LINREG_DCDC_OFF:

        // Use LinReg offset for LinReg mode.
        BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_LINREG_MODE);

        // Turn on the VDDD LinReg and turn off the VDDD DCDC output.
        BF_SET(POWER_VDDDCTRL, ENABLE_LINREG);
        BF_SET(POWER_VDDDCTRL, DISABLE_FET);

        // Make sure stepping is disabled when using linear regulators.
        BF_SET(POWER_VDDDCTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDD rail from the linear regulator.  The DCDC is ready
    // to automatically power the chip when 5V is removed.  Use this
    // configuration when powering from 5V.
    case HW_POWER_LINREG_DCDC_READY:

        // Use LinReg offset for LinReg mode.
        BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDD LinReg and turn on the VDDD DCDC output.  The
        // ENABLE_DCDC must be cleared to avoid LinReg and DCDC conflict.
        BF_CLR(POWER_VDDDCTRL, ENABLE_LINREG);
        BF_CLR(POWER_VDDDCTRL, DISABLE_FET);

        // Make sure stepping is disabled when using linear regulators.
        BF_SET(POWER_VDDDCTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDD rail from an external source.
    case HW_POWER_EXTERNAL_SOURCE_5V:

        // Enable the VDDD linear regulator in case 5V is lost
        BF_WR(POWER_VDDDCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);
        BF_SET(POWER_VDDDCTRL, ENABLE_LINREG);

        // Disable stepping
        BF_SET(POWER_VDDDCTRL, DISABLE_STEPPING);
        DDKPowerEnableExternalPowerSource(TRUE);


        // This needs to use an application based wait of 1ms (i.e., OS wait if os
        // enabled, digctl wait if no os.
        //hw_digctl_MicrosecondWait(10);

        // Disable the VDDD FET to turn off DCDC.
        BF_SET(POWER_VDDDCTRL, DISABLE_FET);
        break;

    // Invalid power source option.
    default:
        rtn = 1;

        break;

    }
    //RETAILMSG(1,(TEXT("PowerSetVdddPowerSource --\r\n")));
    return rtn;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerSetVddioPowerSource
//
//  This routine Sets up the Vddio DDKPowerSource.
//
//  Parameters:
//          eSource
//              [in] power sources for power supply
//
//  Returns:
//          returns 0 if successful otherwise returns error value.
//
//-----------------------------------------------------------------------------
UINT32 DDKPowerSetVddioPowerSource(POWER_POWERSOURCE eSource)
{
    UINT32 rtn = 0;

    // The VDDIO can use two power sources in three configurations: the Linear
    // Regulator, the DCDC with LinReg off, and the DCDC with LinReg on.
    // Each has its own configuration that must be set up to prevent power
    // rail instability. VDDIO will only use the linear regulator when
    // 5V is present.
    //RETAILMSG(1,(TEXT("PowerSetVddioPowerSource ++\r\n")));
    switch(eSource)
    {
    // DDKPower the VDDIO rail from DCDC with LinReg off.
    case HW_POWER_DCDC_LINREG_OFF:

        // Use LinReg offset for DCDC mode.
        BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDIO DCDC output and turn off the LinReg output.
        BF_CLR(POWER_VDDIOCTRL, DISABLE_FET);
        DDKPowerDisableVddioLinearRegulator(TRUE);

        // Make sure stepping is enabled when using DCDC.
        BF_CLR(POWER_VDDIOCTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDIO rail from DCDC with LinReg on.
    case HW_POWER_DCDC_LINREG_READY:
    case HW_POWER_DCDC_LINREG_ON:

        // Use LinReg offset for DCDC mode.
        BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDIO DCDC output and turn on the LinReg output.
        BF_CLR(POWER_VDDIOCTRL, DISABLE_FET);
        DDKPowerDisableVddioLinearRegulator(FALSE);

        // Make sure stepping is enabled when using DCDC.
        BF_CLR(POWER_VDDIOCTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDIO rail from the linear regulator.
    // Assumes 5V is present.
    case HW_POWER_LINREG_DCDC_OFF:

        // Use LinReg offset for LinReg mode.
        BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_LINREG_MODE);

        // Turn on the VDDIO LinReg output and turn off the VDDIO DCDC output.
        DDKPowerDisableVddioLinearRegulator(FALSE);
        BF_SET(POWER_VDDIOCTRL, DISABLE_FET);

        // Make sure stepping is disabled when using linear regulators.
        BF_SET(POWER_VDDIOCTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDIO rail from the linear regulator.
    // Assumes 5V is present.  The DCDC is ready to take over when 5V
    // is removed.
    case HW_POWER_LINREG_DCDC_READY:

        // Use LinReg offset for LinReg mode.
        BF_WR(POWER_VDDIOCTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDIO LinReg output and prepare the VDDIO DCDC output.
        // ENABLE_DCDC must be cleared to prevent DCDC and LinReg conflict.
        DDKPowerDisableVddioLinearRegulator(FALSE);
        BF_CLR(POWER_VDDIOCTRL, DISABLE_FET);

        // Make sure stepping is disabled when using linear regulators.
        BF_SET(POWER_VDDIOCTRL, DISABLE_STEPPING);

        break;

    // Invalid power source option.
    default:
        rtn = 1;
        break;
    }
    //RETAILMSG(1,(TEXT("PowerSetVddioPowerSource --\r\n")));
    return rtn;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerSetVddaPowerSource
//
//  This routine Sets up the Vdda DDKPowerSource.
//
//  Parameters:
//          eSource
//              [in] power sources for power supply
//
//  Returns:
//          returns 0 if successful otherwise returns error value.
//
//-----------------------------------------------------------------------------
UINT32 DDKPowerSetVddaPowerSource(POWER_POWERSOURCE eSource)
{
    UINT32 rtn = 0;

    // The VDDA can use two power sources: the DCDC, or the Linear Regulator.
    // Each has its own configuration that must be set up to prevent power
    // rail instability.
    //RETAILMSG(1,(TEXT("PowerSetVddaPowerSource ++\r\n")));
    switch(eSource)
    {
    // DDKPower the VDDA supply from DCDC with VDDA LinReg off.

    case HW_POWER_DCDC_LINREG_OFF:
    case HW_POWER_DCDC_LINREG_READY:

        // Use LinReg offset for DCDC mode.
        BF_WR(POWER_VDDACTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDA DCDC converter output and turn off the LinReg output.
        BF_CLR(POWER_VDDACTRL, DISABLE_FET);
        BF_CLR(POWER_VDDACTRL, ENABLE_LINREG);

        // Make sure stepping is enabled when using DCDC.
        BF_CLR(POWER_VDDACTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDA supply from DCDC with VDDA LinReg off.
    case HW_POWER_DCDC_LINREG_ON:

        // Use LinReg offset for DCDC mode.
        BF_WR(POWER_VDDACTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_DCDC_MODE);

        // Turn on the VDDA DCDC converter output and turn on the LinReg output.
        BF_CLR(POWER_VDDACTRL, DISABLE_FET);
        BF_SET(POWER_VDDACTRL, ENABLE_LINREG);

        // Make sure stepping is enabled when using DCDC.
        BF_CLR(POWER_VDDACTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDA supply from the linear regulator.  The DCDC output is
    // off and not ready to power the rail if 5V is removed.
    case HW_POWER_LINREG_DCDC_OFF:

        // Use LinReg offset for LinReg mode.
        BF_WR(POWER_VDDACTRL,LINREG_OFFSET,HW_POWER_LINREG_OFFSET_LINREG_MODE);

        // Turn on the VDDA LinReg output and turn off the VDDA DCDC output.
        BF_SET(POWER_VDDACTRL, ENABLE_LINREG);
        BF_SET(POWER_VDDACTRL, DISABLE_FET);

        // Make sure stepping is disabled when using linear regulators.
        BF_SET(POWER_VDDACTRL, DISABLE_STEPPING);

        break;

    // DDKPower the VDDA supply from the linear regulator.  The DCDC output is
    // ready to power the rail if 5V is removed.
    case HW_POWER_LINREG_DCDC_READY:

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
    //RETAILMSG(1,(TEXT("PowerSetVddaPowerSource --\r\n")));
    return rtn;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerEnableExternalPowerSource
//
//  This routine enables the external DDKPowerSource.
//
//  Parameters:
//          bEnable
//              [in] TRUE for Enabling and FALSE for Disabling
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableExternalPowerSource(BOOL bEnable)
{
    UNREFERENCED_PARAMETER(bEnable);

    //RETAILMSG(1,(TEXT("PowerEnableExternalPowerSource ++\r\n")));
#if defined(STMP37xx)
    #define PINCTRL_GPIO_CONFIG 0x3
    #define PINCTRL_VOLT_1p8    0
    #define PINCTRL_VOLT_3p3    1
    #define PINCTRL_CURR_4mA    0
    #define PINCTRL_CURR_8mA    1
    #define PINCTRL_CURR_12mA   2

    #define PINCTRL_SETMASK_BANK1_PIN28 0x10000000
    #define PINCTRL_CLRMASK_BANK1_PIN28 (~PINCTRL_SETMASK_BANK1_PIN28)

    if(bEnable)
    {
        // Configure the GPIO pin.
        HW_PINCTRL_MUXSEL3.B.BANK1_PIN28 = PINCTRL_GPIO_CONFIG;
        HW_PINCTRL_DRIVE7.B.BANK1_PIN28_V = PINCTRL_VOLT_3p3;
        HW_PINCTRL_DRIVE7.B.BANK1_PIN28_MA = PINCTRL_CURR_8mA;

        // Now output on the pin.
        HW_PINCTRL_DOE1.B.DOE |= PINCTRL_SETMASK_BANK1_PIN28;
        HW_PINCTRL_DOUT1.B.DOUT |= PINCTRL_SETMASK_BANK1_PIN28;
    }
    else
    {
        // Clear the output
        HW_PINCTRL_DOUT1.B.DOUT &= PINCTRL_CLRMASK_BANK1_PIN28;
    }

    bExternalPowerSourceActive = bEnable;

#elif defined(STMP378x) || defined(STMP377x)
    return;
#else
    //#error Must define STMP37xx, STMP377x or STMP378x
#endif
    //RETAILMSG(1,(TEXT("PowerEnableExternalPowerSource --\r\n")));
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerDisableVddioLinearRegulator
//
//  This routine enables the external DDKPowerSource.
//
//  Parameters:
//          bDisable
//              [in] TRUE for Disabling and FALSE for Enabling
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerDisableVddioLinearRegulator(BOOL bDisable)
{
    //RETAILMSG(1,(TEXT("PowerDisableVddioLinearRegulator ++\r\n")));
    if(bDisable)
    {
        BF_SET(POWER_5VCTRL, ILIMIT_EQ_ZERO);
    }
    else
    {
        BF_CLR(POWER_5VCTRL, ILIMIT_EQ_ZERO);
    }
    //RETAILMSG(1,(TEXT("PowerDisableVddioLinearRegulator --\r\n")));
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerEnableDcdc
//
//  This routine enables ENABLE_DCDC.
//
//  Parameters:
//          bEnable
//              [in] TRUE for Enabling and FALSE for Disabling
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableDcdc(BOOL bEnable)
{
    //RETAILMSG(1,(TEXT("PowerEnableDcdc ++\r\n")));

    if(bEnable)
        BF_SET(POWER_5VCTRL, ENABLE_DCDC);
    else
        BF_CLR(POWER_5VCTRL, ENABLE_DCDC);

    //RETAILMSG(1,(TEXT("PowerEnableDcdc --\r\n")));
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerInitPowerSupplies
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
UINT32 DDKPowerInitPowerSupplies(VOID)
{
    UINT32 Status;

    // Make sure the power supplies are configured for their power sources.
    // This sets the LINREG_OFFSET field correctly for each power supply.
    //RETAILMSG(1,(TEXT("PowerInitPowerSupplies ++\r\n")));

    if(HW_POWER_STS.B.VBUSVALID)
    {
        if((Status = DDKPowerSetVdddPowerSource(HW_POWER_LINREG_DCDC_OFF)) != 0)
            return Status;
        if((Status = DDKPowerSetVddaPowerSource(HW_POWER_LINREG_DCDC_OFF)) != 0)
            return Status;
        if((Status = DDKPowerSetVddioPowerSource(HW_POWER_LINREG_DCDC_OFF)) != 0)
            return Status;
    }
    else if(DDKPowerGetBatteryMode() == HW_POWER_BATT_MODE_LIION)
    {
        // For LiIon battery, all the rails can start off as DCDC mode.
        if((Status = DDKPowerSetVdddPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
            return Status;
        if((Status = DDKPowerSetVddaPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
            return Status;
        if((Status = DDKPowerSetVddioPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
            return Status;
    }
    else
    {
        // For Alkaline/NiMH, VDDD needs to come from the linear regulator
        // because the battery voltage may be above or below the VDDD target.
        // VDDA and VDDIO can be powered from the DCDC.
        if((Status = DDKPowerSetVdddPowerSource(HW_POWER_LINREG_DCDC_OFF)) != 0)
            return Status;
        if((Status = DDKPowerSetVddaPowerSource(HW_POWER_DCDC_LINREG_READY)) != 0)
            return Status;
        if((Status = DDKPowerSetVddioPowerSource(HW_POWER_DCDC_LINREG_ON)) != 0)
            return Status;
    }
    //RETAILMSG(1,(TEXT("PowerInitPowerSupplies --\r\n")));
    // Done.
    return 0;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerGet5vPresentFlag
//
//  This routine checks if the 5V supply is present.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns TRUE if successful otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKPowerGet5vPresentFlag(VOID)
{
    //RETAILMSG(1,(TEXT("PowerGet5vPresentFlag ++\r\n")));
    //RETAILMSG(1,(TEXT("PowerGet5vPresentFlag --\r\n")));
    return HW_POWER_STS.B.VBUSVALID;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerGetBatteryChargingStatus
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
BOOL DDKPowerGetBatteryChargingStatus(VOID)
{
    //RETAILMSG(1,(TEXT("PowerGetBatteryChargingStatus ++\r\n")));
    //RETAILMSG(1,(TEXT("PowerGetBatteryChargingStatus --\r\n")));
    BOOL Flag = FALSE;
    if(HW_POWER_STS.B.VBUSVALID == 1)
    {
        do{Flag = (BOOL)HW_POWER_STS.B.CHRGSTS;}
        while(Flag != (BOOL)HW_POWER_STS.B.CHRGSTS);
        return Flag;
    }
    else
        return FALSE;
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerGetBatteryVoltage
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
UINT16 DDKPowerGetBatteryVoltage(VOID)
{
    UINT16 u16BattVolt = 0,u16SumVoltage = 0;
    UINT16 loop = 0;    
    //RETAILMSG(1,(TEXT("PowerGetBatteryVoltage ++\r\n")));
    // Get the raw result of battery measurement
    //PowerEnableDcdc(FALSE);

    if(loop < 5)
    {
        while(u16BattVolt == HW_POWER_BATTMONITOR.B.BATT_VAL);
        u16BattVolt = (UINT16)HW_POWER_BATTMONITOR.B.BATT_VAL;
        u16SumVoltage = u16SumVoltage + u16BattVolt;
        loop++;
    }
    else
    {
        u16BattVolt = u16SumVoltage / 5;
        u16SumVoltage = 0;
        loop = 0;
    }

    //DumpPowerRegisters();
    // Adjust for 8-mV LSB resolution and return
    return (u16BattVolt * BATT_VOLTAGE_8_MV);
}
//-----------------------------------------------------------------------------
//
//  Function: DDKPowerGetBatteryMode
//
//  This routine returns the current Battery Mode.
//
//  Parameters:
//          None.
//
//  Returns:
//          returns battery mode.
//
//-----------------------------------------------------------------------------
POWER_BATTERYMODE DDKPowerGetBatteryMode(VOID)
{

#if defined(STMP37xx) || defined(STMP377x)
    return (POWER_BATTERYMODE) HW_POWER_STS.B.MODE;
#elif defined(STMP378x)
    // Removed from the 378x chip
    return HW_POWER_BATT_MODE_LIION;
//#else
//    //#error Must define STMP37xx, STMP377x or STMP378x
#endif
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerExecuteBatteryTo5VoltsHandoff
//
//  This function hands off power source from battery to 5Volts supply
//
//  Parameters:
//          None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerExecuteBatteryTo5VoltsHandoff()
{

#ifdef STMP378x
    
    // Startup the 4p2 rail and enabled 4p2 from DCDC
    if(b4p2Enabled)
    {
        DDKPowerStart4p2();
        return;
    }
   
#endif

    // Use the linear regulators to power the chip.
    if(bExternalPowerSourceActive)
    {
        DDKPowerSetVdddPowerSource(HW_POWER_EXTERNAL_SOURCE_5V);
    }
    else
    {
        DDKPowerSetVdddPowerSource(HW_POWER_LINREG_DCDC_READY);
    }

    DDKPowerSetVddaPowerSource(HW_POWER_LINREG_DCDC_READY);
    DDKPowerSetVddioPowerSource(HW_POWER_LINREG_DCDC_READY);

    // Disable the DCDC during 5V connections.
    DDKPowerEnableDcdc(FALSE);

    // Disable hardware power down when 5V is inserted or removed
    DDKPowerDisableAutoHardwarePowerdown(TRUE);
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerExecute5VoltsToBatteryHandoff
//
//  This function hands off power source from 5Volts supply to battery
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void DDKPowerExecute5VoltsToBatteryHandoff(void)
{

#ifdef STMP378x
    {
        // Disable DCDC from 4p2 and turn off the 4p2 rail
        DDKPowerStop4p2();

        // Disable hardware power down when 5V is inserted or removed
        DDKPowerDisableAutoHardwarePowerdown(TRUE);

        // Re-enable the battery brownout interrupt in case it was disabled.
        DDKPowerEnableBatteryBrownoutInterrupt(TRUE);

        return;
    }
#else
    // VDDD has different configurations depending on the battery type and
    // battery level.

    // Enforce battery powered operation
    DDKPowerEnableDcdc(true);

    if(PowerGetBatteryMode() == HW_POWER_BATT_MODE_LIION)
    {
        // For LiIon battery, we will use the DCDC to power VDDD, but we need
        // to check what the previous source was.
        if(PowerGetVdddPowerSource() == HW_POWER_EXTERNAL_SOURCE_5V)
        {
            // When powered from external sources, we need to temporarily
            // increase the target to prevent the DCDC from fighting
            // with the external source.  PMI will change the target to
            // the correct voltage after the source transition.
            DDKPowerSetVdddBrownoutValue(175);
            DDKPowerSetVdddValue(1575);
            //PowerWaitForVdddStable();
            //using sleep for wait instead of DigCtrl register temporarily
            Sleep(10);
            DDKPowerSetVdddBrownoutValue(125);
        }

        DDKPowerSetVdddPowerSource(HW_POWER_DCDC_LINREG_READY);
    }
    else
    {
        // Alkaline mode uses DCDC if the battery is below the target so it can
        // buck.  LinRegs are used if the battery is above the target so power
        // will come from the VDDA (1.8V) rail because the DCDC does not boost
        // in alkaline mode.  The LinReg will buck from the 1.8V line.

        //right now we are supporting Li-Ion Batt support
        //if(PowerGetBatteryVoltage() <= DDKPowerGetVddd())
        //{
        //    //------------------------------------------------------------------
        //    // BATT <= VDDD_TRG so use
        //    //------------------------------------------------------------------
        //
        //    DDKPowerSetVdddPowerSource(HW_POWER_DCDC_LINREG_READY);
        //
        //}
        //else
        //{
        //    //------------------------------------------------------------------
        //    // BATT > VDDD_TRG
        //    //------------------------------------------------------------------
        //
        //    DDKPowerSetVdddPowerSource(HW_POWER_LINREG_DCDC_OFF);
        //
        //}
    }

    // DDKPower VDDA and VDDIO from the DCDC.

    DDKPowerSetVddaPowerSource(HW_POWER_DCDC_LINREG_READY);
    DDKPowerSetVddioPowerSource(HW_POWER_DCDC_LINREG_ON);

    // Disable hardware power down when 5V is inserted or removed

    DDKPowerDisableAutoHardwarePowerdown(TRUE);

    // Re-enable the battery brownout interrupt in case it was disabled.
    DDKPowerEnableBatteryBrownoutInterrupt(TRUE);
#endif
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnable5VoltsToBatteryHandoff
//
//  This function enables the handoff from 5Volts supply to battery
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnable5VoltsToBatteryHandoff(VOID)
{
    // Enable detection of 5V removal (unplug)
    DDKPowerEnable5vUnplugDetect(TRUE);

#ifdef STMP378x
    //hw_power_EnableAutoDcdcTransfer(TRUE) causes strange behavior when 4p2 is enabled.
    if(b4p2Enabled)
    { 
        return;
    }
#endif

    // Enable automatic transition to DCDC
    DDKPowerEnableAutoDcdcTransfer(TRUE);

    // Alkaline mode uses DCDC if the battery is below the target so it can
    // buck.  LinRegs are used if the battery is above the target so power
    // will come from the VDDA (1.8V) rail because the DCDC does not boost
    // in alkaline mode.  The LinReg will buck from the 1.8V line.
    //
    // Note: The VDDD configuration is set here because it is more of a
    // preparation for battery power rather than a necessary setting for
    // the rail when 5V is connected.

    // currently not using the Alkaline NIMH battery
    //if(PowerGetBatteryMode() == DDI_POWER_BATT_MODE_ALKALINE_NIMH)
    //{
    //    if(PowerGetBatteryVoltage() <= DDKPowerGetVddd())
    //    {
    //        //------------------------------------------------------------------
    //        // BATT <= VDDD_TRG so use DCDC's.
    //        //------------------------------------------------------------------
    //        hw_power_SetVdddPowerSource(HW_POWER_DCDC_LINREG_READY);
    //
    //    }
    //    else
    //    {
    //        //------------------------------------------------------------------
    //        // BATT > VDDD_TRG
    //        //------------------------------------------------------------------
    //        DDKPowerSetVdddPowerSource(HW_POWER_LINREG_DCDC_OFF);
    //
    //    }
    //}
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnableBatteryTo5VoltsHandoff
//
//  This function enables the handoff from battery to 5Volts
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void DDKPowerEnableBatteryTo5VoltsHandoff(void)
{
    // Enable 5V plug-in detection
    DDKPowerEnable5vPlugInDetect(TRUE);

    // Allow DCDC be to active when 5V is present.
    DDKPowerEnableDcdc(TRUE);
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerStop4p2
//
//  This function stops 4p2 functionality, used in STMP378x
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void DDKPowerStop4p2(void)
{

    BF_CLR(POWER_DCDC4P2, ENABLE_DCDC);       // Enable the DCDC.

    BF_CLR(POWER_DCDC4P2, ENABLE_4P2);      //enable DCDC 4P2 regulation circuitry

    BF_CLR(POWER_CHARGE,ENABLE_LOAD);



//      BF_SET(POWER_5VCTRL, PWRUP_VBUS_CMPS);

    BF_CLR(POWER_CTRL,ENIRQ_DCDC4P2_BO);

    BF_SET(POWER_5VCTRL, PWD_CHARGE_4P2);

}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGetVdddPowerSource
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
POWER_POWERSOURCE DDKPowerGetVdddPowerSource(void)
{

    // If the Vddd DCDC Converter output is disabled, LinReg must be powering Vddd.

    if(HW_POWER_VDDDCTRL.B.DISABLE_FET)
    {

        if(VdddPowerSource == HW_POWER_EXTERNAL_SOURCE_5V)
        {
            return HW_POWER_EXTERNAL_SOURCE_5V;
        }

        // Make sure the LinReg offset is correct for this source.

        if(DDKPowerGetVdddLinRegOffset() == HW_POWER_LINREG_OFFSET_LINREG_MODE)
        {
            return HW_POWER_LINREG_DCDC_OFF;
        }

    }

    // If here, DCDC must be powering Vddd.  Determine if the LinReg is also on.
    if(HW_POWER_VDDDCTRL.B.ENABLE_LINREG)
    {
        // The LinReg offset must be below the target if DCDC and LinRegs' are
        // on at the same time.  Otherwise, we have an invalid configuration.
        if(DDKPowerGetVdddLinRegOffset() == HW_POWER_LINREG_OFFSET_DCDC_MODE)
        {
            return HW_POWER_DCDC_LINREG_ON;
        }
    }
    else
    {
        // Is the LinReg offset set to power Vddd from linreg?
        if(DDKPowerGetVdddLinRegOffset() == HW_POWER_LINREG_OFFSET_DCDC_MODE)
        {
            return HW_POWER_DCDC_LINREG_OFF;
        }
    }

    // If we get here, the power source is in an unknown configuration.  Most
    // likely, the LinReg offset is incorrect for the given power supply.
    return HW_POWER_UNKNOWN_SOURCE;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerSetVdddBrownoutValue
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
VOID DDKPowerSetVdddBrownoutValue(UINT16 u16VdddBoOffset_mV)
{
    UINT16 u16VdddBoOffset_Set;

    // Convert millivolts to register setting (1 step = 25mV)
    u16VdddBoOffset_Set = (UINT16)(u16VdddBoOffset_mV/(VOLTAGE_STEP_MV));

    // Use this way of writing to the register because we want a read, modify,
    // write operation.  Using the other available macros will call a clear/set
    // which momentarily makes the brownout offset zero which may trigger a
    // false brownout.
    HW_POWER_VDDDCTRL.B.BO_OFFSET = u16VdddBoOffset_Set;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerSetVdddValue
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
void DDKPowerSetVdddValue(UINT16 u16Vddd_mV)
{
    UINT16 u16Vddd_Set;

    // Convert mV to register setting
    u16Vddd_Set = PowerConvertVdddToSetting(u16Vddd_mV);

    // Use this way of writing to the register because we want a read, modify,
    // write operation.  Using the other available macros will call a clear/set
    // which momentarily clears the target field causing the voltage to dip
    // then rise again.  That should be avoided.
    HW_POWER_VDDDCTRL.B.TRG = u16Vddd_Set;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGetVdddValue
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
UINT32 DDKPowerGetVdddValue(VOID)
{
    UINT16 u16Vddd_Set;
    UINT16 u16Vddd_mV;

    //Read VDDD bitfiled value
    u16Vddd_Set = (UINT16)HW_POWER_VDDDCTRL.B.TRG;

    //  Convert to mVolts
    u16Vddd_mV = PowerConvertSettingToVddd(u16Vddd_Set);   

    return (UINT32)u16Vddd_mV;
}


//-----------------------------------------------------------------------------
//
//  Function:  PowerConvertVdddToSetting
//
//
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
UINT16 PowerConvertVdddToSetting(UINT16 u16Vddd)
{
    return ((UINT16)((u16Vddd - VDDD_BASE_MV)/25));
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

UINT16 PowerConvertSettingToVddd(UINT16 u16Setting)
{
    return ((u16Setting * 25) + VDDD_BASE_MV);
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGetVdddLinRegOffset
//
//  This function gets the Vddd LinRegOffset
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
POWER_LINREGOFFSETSTEP DDKPowerGetVdddLinRegOffset(VOID)
{
    return (POWER_LINREGOFFSETSTEP) HW_POWER_VDDDCTRL.B.LINREG_OFFSET;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnable5vPlugInDetect
//
//  This function enables the 5V plugin detect
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void DDKPowerEnable5vPlugInDetect(BOOL bEnable)
{
    switch(DetectionMethod)
    {
    case DDI_POWER_VBUSVALID:

        // Set the VBUSVALID interrupt polarity.
        if(bEnable)
        {
            DDKPowerSetVbusValidInterruptPolarity(HW_POWER_CHECK_5V_CONNECTED);
        }
        else
        {
            DDKPowerSetVbusValidInterruptPolarity(HW_POWER_CHECK_5V_DISCONNECTED);
        }
        break;


    case DDI_POWER_VDD5V_GT_VDDIO:

        // Set the VDD5V_GT_VDDIO interrupt polarity.
        if(bEnable)
        {
            DDKPowerSetVdd5vGtVddioInterruptPolarity(HW_POWER_CHECK_5V_CONNECTED);
        }
        else
        {
            DDKPowerSetVdd5vGtVddioInterruptPolarity(HW_POWER_CHECK_5V_DISCONNECTED);
        }
        break;

    default:

        // Not a valid state.
        break;
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnable5vUnplugDetect
//
//  This function enables the 5VUnplug detect
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnable5vUnplugDetect(BOOL bEnable)
{
    switch(DetectionMethod)
    {
    case DDI_POWER_VBUSVALID:

        // Set the VBUSVALID interrupt polarity.
        if(bEnable)
        {
            DDKPowerSetVbusValidInterruptPolarity(HW_POWER_CHECK_5V_DISCONNECTED);
        }
        else
        {
            DDKPowerSetVbusValidInterruptPolarity(HW_POWER_CHECK_5V_CONNECTED);
        }
        break;



    case DDI_POWER_VDD5V_GT_VDDIO:

        //------------------------------------------------------------------
        // Set the VDD5V_GT_VDDIO interrupt polarity.
        //------------------------------------------------------------------

        if(bEnable)
        {
            DDKPowerSetVdd5vGtVddioInterruptPolarity(HW_POWER_CHECK_5V_DISCONNECTED);
        }
        else
        {
            DDKPowerSetVdd5vGtVddioInterruptPolarity(HW_POWER_CHECK_5V_CONNECTED);
        }
        break;

    default:

        // Not a valid state.
        break;
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerSetVbusValidInterruptPolarity
//
//  This function sets the VBUS_VALID interrupt
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerSetVbusValidInterruptPolarity(BOOL bPolarity)
{
    if(bPolarity == HW_POWER_CHECK_5V_DISCONNECTED)
    {
        BF_CLR(POWER_CTRL, POLARITY_VBUSVALID);
    }

    // bPolarity == HW_POWER_CHECK_5V_CONNECTED
    else
    {
        BF_SET(POWER_CTRL, POLARITY_VBUSVALID);
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerSetVdd5vGtVddioInterruptPolarity
//
//  This function sets the VDD5V_GT_VDDIO interrupt
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerSetVdd5vGtVddioInterruptPolarity(BOOL bPolarity)
{
    if(bPolarity == HW_POWER_CHECK_5V_DISCONNECTED)
    {
        BF_CLR(POWER_CTRL, POLARITY_VDD5V_GT_VDDIO);
    }

    // bPolarity == HW_POWER_CHECK_5V_CONNECTED
    else
    {
        BF_SET(POWER_CTRL, POLARITY_VDD5V_GT_VDDIO);
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerDisableAutoHardwarePowerdown
//
//  This function disables the autoo hardware power down funtion
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerDisableAutoHardwarePowerdown(BOOL bDisable)
{
    if(bDisable)
    {
        BF_CLR(POWER_5VCTRL, PWDN_5VBRNOUT);
    }
    else
    {
        BF_SET(POWER_5VCTRL, PWDN_5VBRNOUT);
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnableAutoDcdcTransfer
//
//  This function enables the automatic DCDC transfer
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableAutoDcdcTransfer(BOOL bEnable)
{
    if(bEnable)
    {
        BF_SET(POWER_5VCTRL, DCDC_XFER);
    }
    else
    {
        BF_CLR(POWER_5VCTRL, DCDC_XFER);
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnableBatteryBrownoutInterrupt
//
//  This function enables the Battery Brownout interrupt
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableBatteryBrownoutInterrupt(BOOL bEnable)
{
    if(bEnable)
    {
        BF_SET(POWER_CTRL, ENIRQBATT_BO);
    }
    else
    {
        BF_CLR(POWER_CTRL, ENIRQBATT_BO);
    }
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnableUSBPHY
//
//  This function enables the Battery Brownout interrupt
//
//  Parameters:
//          None.
//
//  Returns:
//      Returns the TRUE if successful,
//      otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableUSBPHY(VOID)
{
    // ungate clock to power module
    HW_POWER_CTRL_CLR(BM_POWER_CTRL_CLKGATE);
    HW_POWER_CTRL_RD();

    // Turn on VBUS comparators
    HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_PWRUP_VBUS_CMPS);
    HW_POWER_CTRL_RD();
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerEnableVbusValid5vDetect
//
//  This function is used to enable the 5V Supply detection
//
//  Parameters:
//          bEnable
//              [in] TRUE to enable, FALSE to disable
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerEnableVbusValid5vDetect(BOOL bEnable)
{
    if(bEnable)
    {
        BF_SET(POWER_5VCTRL, VBUSVALID_5VDETECT);
    }
    else
    {
        BF_CLR(POWER_5VCTRL, VBUSVALID_5VDETECT);
    }
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerSetCharger
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
VOID DDKPowerSetCharger(DWORD current)
{
    if(DDKPowerGetLimit())
    {        
        RETAILMSG(1, (TEXT("USB current 100mA limitation, can not charging...\r\n")));
        return;
    }

    BF_CLRV(POWER_CHARGE, STOP_ILIMIT, 0xF);
    BF_SETV(POWER_CHARGE, STOP_ILIMIT, 0x3);//stop limit current = 30mA  

    //HW_POWER_CHARGE_CLR(0xF00);
    //HW_POWER_CHARGE_SET(0x400);  
    
    BF_CLRV(POWER_CHARGE, BATTCHRG_I, 0x3F);
    BF_SETV(POWER_CHARGE, BATTCHRG_I, current); 

    //HW_POWER_CHARGE_CLR(0x3F);
    //HW_POWER_CHARGE_SET(current);  

    //BF_CLR(POWER_5VCTRL, CHARGE_4P2_ILIMIT);
    //BF_SETV(POWER_5VCTRL, CHARGE_4P2_ILIMIT,0x3F);

    BF_CLR(POWER_CHARGE, PWD_BATTCHRG);
    BF_CLR(POWER_5VCTRL, PWD_CHARGE_4P2); 

    //HW_POWER_CHARGE_CLR(0x10000);
    //HW_POWER_5VCTRL_CLR(0x100000);
    


}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerStopCharger
//
//  This function is used to stop the charger
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
VOID DDKPowerStopCharger()
{
    BF_CLRV(POWER_CHARGE, STOP_ILIMIT, 0xF);
    BF_CLRV(POWER_CHARGE, BATTCHRG_I, 0x3F);
    BF_SET(POWER_CHARGE, PWD_BATTCHRG);
    //BF_SET(POWER_5VCTRL, PWD_CHARGE_4P2);  

    //HW_POWER_CHARGE_CLR(0xF00);
    //HW_POWER_CHARGE_CLR(0x3F);
    //HW_POWER_CHARGE_SET(0x10000);
    //HW_POWER_5VCTRL_SET(0x100000);    
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerClear5VIrq
//
//  This function is used to clear the 5V irq
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------

VOID DDKPowerClear5VIrq()
{
    BF_CLR(POWER_CTRL, VBUSVALID_IRQ);
    BF_CLR(POWER_CTRL, VDD5V_GT_VDDIO_IRQ);
    BF_CLR(POWER_CTRL, ENIRQ_VDD5V_DROOP);
    BF_CLR(POWER_CTRL, VDD5V_DROOP_IRQ);
    //BF_CLR(POWER_CTRL, PSWITCH_IRQ);
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGet5VIrq
//
//  This function is used to get the 5V irq
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
BOOL DDKPowerGet5VIrq()
{
    return HW_POWER_CTRL.B.VBUSVALID_IRQ;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGetPSwitchIrq
//
//  This function is used to get the PSwitch irq
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
BOOL DDKPowerGetPSwitchIrq()
{
    return HW_POWER_CTRL.B.PSWITCH_IRQ;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGetPSwitchStatus
//
//  This function is used to get the PSwitch status
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
BOOL DDKPowerGetPSwitchStatus()
{
    return HW_POWER_STS.B.PSWITCH;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerClearPSwitchIrq
//
//  This function is used to clear the PSwitch irq
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------

VOID DDKPowerClearPSwitchIrq()
{
    HW_POWER_CTRL_CLR(0x100000);
}
//-----------------------------------------------------------------------------
//
//  Function:  PowerGetLimit
//
//  This function is used to clear the limit info
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------

BOOL DDKPowerGetLimit()
{
    if( (((HW_POWER_5VCTRL_RD() & 0x40) == 0x40) || ((HW_POWER_5VCTRL_RD() & 0x3F000) == 0x8000)) && (DDKPowerGetUSBPhy() == TRUE))
        return TRUE;
    else
        return FALSE;
}
//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGetUSBPhy
//
//  This function is used to clear the limit info
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------

BOOL DDKPowerGetUSBPhy()
{
    if ((HW_USBPHY_CTRL_RD()& 0x40000000) == 1)
        HW_USBPHY_CTRL_CLR(0x40000000);
    if ((HW_USBPHY_CTRL_RD()& 0x80000000) == 1)
        HW_USBPHY_CTRL_CLR(0x80000000);
    
    HW_USBPHY_CTRL_SET(0x10);
    Sleep(1);
    if((HW_USBPHY_STATUS_RD() & BM_USBPHY_STATUS_DEVPLUGIN_STATUS)) 
        return TRUE;
    else
        return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerSetLimit
//
//  This function is used to clear the limit info
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------

VOID DDKPowerSetLimit(void)
{
    HW_POWER_5VCTRL_CLR(0x37000);
    HW_POWER_5VCTRL_SET(0x8000);
    HW_POWER_5VCTRL_SET(0x40);
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerClearLimit
//
//  This function is used to clear the limit info
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------

VOID DDKPowerClearLimit(void)
{
    HW_POWER_5VCTRL_SET(0x37000);
    HW_POWER_5VCTRL_CLR(0x40);
}

//-----------------------------------------------------------------------------
//
//  Function:  DDKPowerGetDirectBoot
//
//  This function is used to clear the limit info
//
//  Parameters:
//
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------

BOOL DDKPowerGetDirectBoot(void)
{
    return bPLL;
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

void DDKPowerInitCpuHclkClock(void)
{
    PVOID pv_HWregCLKCTRL = NULL;
    PHYSICAL_ADDRESS phyAddr;

    if (pv_HWregCLKCTRL == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_CLKCTRL;

        // Map peripheral physical address to virtual address
        pv_HWregCLKCTRL = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

        // Check if virtual mapping failed
        if (pv_HWregCLKCTRL == NULL)
        {
            ERRORMSG(1, (_T("PowerAlloc::MmMapIoSpace failed!\r\n")));
            return;
        }
    }

    //
    // let CPU sink the xtal clock
    //
    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    // config CLK_CPU driver for 320MHZ
    //HW_CLKCTRL_CPU_CLR(BF_CLKCTRL_CPU_DIV_CPU(0x3F));
    HW_CLKCTRL_CPU_CLR(BF_CLKCTRL_CPU_DIV_CPU(0x3F));
    HW_CLKCTRL_CPU_SET(BF_CLKCTRL_CPU_DIV_CPU(0x1));

    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
    //Sleep(10);
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
VOID DDKDumpPowerRegisters(VOID)
{
    //Dump All the DDKPower registers
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
}
