// All rights reserved ADENEO EMBEDDED 2010
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
//  File: prcm_device.c
//
//#include "omap.h"
//#include "omap_prof.h"
#include "am389x.h"
#include "am389x_clocks.h"
#include <nkintr.h>
#include <pkfuncs.h>
#include "oalex.h"
#include "prcm_priv.h"
#include "prcm_device.h"
#include <oal.h>


extern ClockDomainLookupEntry s_rgClkDomainLookupTable[];
extern BOOL g_bSingleThreaded;


BOOL _PrcmHwEnableModuleClocks(AM389X_DEVICE_ID devId, BOOL bEnable)
{
	UINT32   timeout = 2000000;
	volatile unsigned int *pmodulemode;
	
	if (devId >= AM_DEVICE_COUNT) return FALSE;

	pmodulemode = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs +
		                          s_rgDeviceLookupTable[devId].CLKCTRL_REG);


	OUTREG32(pmodulemode, (bEnable)? 0x2 : 0x0);

	if (bEnable){
		while((timeout) && (INREG32(pmodulemode) & 0x30000))
			timeout--;
		if (timeout == 0)
			OALMSG(1,(L"_PrcmHwEnableModuleClocks: CANNOT enable clock for devId=%d\r\n", devId));
	}
	
	return TRUE;	
}



//------------------------------------------------------------------------------
BOOL PrcmDeviceEnableClocks( UINT devId, BOOL bEnable)
//  Enables the appropriate functional and interface clocks
{
    UINT oldState = bEnable ? D4 : D0;
    UINT newState = bEnable ? D0 : D4;

	if (devId >= AM_DEVICE_COUNT) return FALSE;

    OALMux_UpdateOnDeviceStateChange(devId, oldState, newState, TRUE);
/*
	Assumption:
	In most cases a device has one or two clock domains which controll clock gaiting
	for that device. So, we need to enable the clock domains that device belongs to
	or disable when we turn off the device. We look at the device lookup table to see
	what clock domains we need to enable.
	Some devices may require a special clock handling whihc should be implemented
	in corresponding 'cases'
*/
    switch (devId)
    {
// For devices which require special care add control here     
//	    case OMAP_DEVICE_VPFE:
//        {
//            if (bEnable) {
//            }  else  {
//            }
//		  break;
//       }
	  default:
/*
     The s_rgDeviceLookupTable is not complete. I don't know clock domains for all modules
     If the function is called for such module you will see the error message
*/
		if (s_rgDeviceLookupTable[devId].fclk_domain == CLKDMN_UNKNOWN ||
			s_rgDeviceLookupTable[devId].iclk_domain == CLKDMN_UNKNOWN ){
			RETAILMSG(1,(L"HEY THIS IS AN ERROR: WE DON'T KNOW CLOCK DOMAINS FOR %d MODULE\r\n", devId));
			return FALSE;
		}

	   	if (bEnable){
	        InterlockedIncrement(&s_rgDeviceLookupTable[devId].refCount);
	        if (s_rgDeviceLookupTable[devId].refCount == 1){
	            //Enable clock domane
	            if (s_rgDeviceLookupTable[devId].fclk_domain != CLKDMN_NULL)
					ClockEnableClkDomain(s_rgDeviceLookupTable[devId].fclk_domain, TRUE);
	            if (s_rgDeviceLookupTable[devId].iclk_domain != CLKDMN_NULL)
					ClockEnableClkDomain(s_rgDeviceLookupTable[devId].iclk_domain, TRUE);
	            _PrcmHwEnableModuleClocks(devId, TRUE);
	        }
	   } else if (s_rgDeviceLookupTable[devId].refCount > 0){
	        InterlockedDecrement(&s_rgDeviceLookupTable[devId].refCount);
	        if (s_rgDeviceLookupTable[devId].refCount == 0){
	            _PrcmHwEnableModuleClocks(devId, FALSE);
                // disable clock domain
	            if (s_rgDeviceLookupTable[devId].fclk_domain != CLKDMN_NULL)
					ClockEnableClkDomain(s_rgDeviceLookupTable[devId].fclk_domain, FALSE);
	            if (s_rgDeviceLookupTable[devId].iclk_domain != CLKDMN_NULL)
					ClockEnableClkDomain(s_rgDeviceLookupTable[devId].iclk_domain, FALSE);
            }
       }    
	}

    OALMux_UpdateOnDeviceStateChange(devId, oldState, newState, FALSE);

    return TRUE;
}


//-----------------------------------------------------------------------------
//  Sets the clock source for a device
BOOL PrcmDeviceSetSourceClocks( UINT devId, UINT count, UINT rgClocks[])
{
    BOOL rc = FALSE;
    if (!g_bSingleThreaded) 
		OALMSG(1/*OAL_FUNC*/, (L"+PrcmDeviceSetSourceClocks(devId=%d), %d\r\n",
				devId, count ));

//cleanUp:
    return rc;
}   



//------------------------------------------------------------------------------
BOOL PrcmDeviceEnableAutoIdle(UINT devId, BOOL bEnable){
    UNREFERENCED_PARAMETER(devId);
    UNREFERENCED_PARAMETER(bEnable);
	return TRUE;
}    

//------------------------------------------------------------------------------
//  retrieves the context state of a device.
//
//  TRUE - context for device is retained
//  FALSE - context for device is reset.
BOOL PrcmDeviceGetContextState(UINT devId, BOOL bSet)
{
    UNREFERENCED_PARAMETER(devId);
    UNREFERENCED_PARAMETER(bSet);
    return TRUE;
}

BOOL PrcmDeviceEnableIClock( UINT devId, BOOL bEnable){
    UNREFERENCED_PARAMETER(devId);
    UNREFERENCED_PARAMETER(bEnable);
	return FALSE;
}
BOOL PrcmDeviceEnableFClock(UINT devId, BOOL bEnable){
    UNREFERENCED_PARAMETER(devId);
    UNREFERENCED_PARAMETER(bEnable);
	return FALSE;
}

BOOL EnableDeviceClocks(AM389X_DEVICE_ID devID, BOOL bEnable)
{
    return PrcmDeviceEnableClocks(devID,bEnable);
}

BOOL DeviceInitialize(void)
{
	int j;
	volatile unsigned int *pmodulemode;

	OALMSG(1/*OAL_FUNC*/, (L"DeviceInitialize\r\n"));
	// update reference counters for divices enabled by bootloader or earlier
	for (j=0; j<AM_DEVICE_COUNT; j++){
		pmodulemode = (volatile unsigned int*)((UCHAR*)g_pPrcmRegs +
									  s_rgDeviceLookupTable[j].CLKCTRL_REG);

		if ((INREG32(pmodulemode) & 0x3) == 0)// device is disabled
			continue;

		OALMSG(1, (L"DeviceInitialize: %d is enabled\r\n", j));
		s_rgDeviceLookupTable[j].refCount++;
		if (s_rgDeviceLookupTable[j].fclk_domain != CLKDMN_UNKNOWN && 
			s_rgDeviceLookupTable[j].fclk_domain != CLKDMN_NULL){
			s_rgClkDomainLookupTable[s_rgDeviceLookupTable[j].fclk_domain].refCount++;
			OALMSG(1, (L"\tDeviceInitialize: updating clk domain %d for dev %d\r\n",
				s_rgDeviceLookupTable[j].fclk_domain, j));
			
		}
		if (s_rgDeviceLookupTable[j].iclk_domain != CLKDMN_UNKNOWN && 
			s_rgDeviceLookupTable[j].iclk_domain != CLKDMN_NULL){
			s_rgClkDomainLookupTable[s_rgDeviceLookupTable[j].iclk_domain].refCount++;
			OALMSG(1, (L"\tDeviceInitialize: updating clk domain %d for dev %d\r\n",
				s_rgDeviceLookupTable[j].iclk_domain, j));
		}
		
	}

	return TRUE;
}


