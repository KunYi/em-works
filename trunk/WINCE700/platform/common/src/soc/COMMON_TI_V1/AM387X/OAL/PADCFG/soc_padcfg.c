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

#include "am387x.h"
#include "am387x_config.h"
#include "sdk_padcfg.h"
#include "soc_cfg.h"

static UINT32* g_pPadCfg = NULL;

static _inline void MapPadRegisters()
{
    if (g_pPadCfg == NULL)
        g_pPadCfg = OALPAtoUA(AM387X_SYSC_PADCONFS_REGS_PA);
}

BOOL SOCSetPadConfig(UINT16 padId, UINT16 cfg)
{
    UINT32 hwcfg = 0;

    MapPadRegisters();
    
    //Translate config to CONTROL_PADCONF_X format
    hwcfg |= (cfg & PULL_RESISTOR_ENABLED) ? ((cfg & PULLUP_RESISTOR) ? PULL_UP : PULL_DOWN) : PULL_DISABLE;    
    hwcfg |= (cfg & SLEWCTRL_SLOW) ? MUX_SLEWCTRL_SLOW : MUX_SLEWCTRL_FAST;    
    hwcfg |= (cfg & INPUT_ENABLED) ? INPUT_ENABLE : INPUT_DISABLE;
    hwcfg |= ((1 << ((cfg >> 3) & 0xf)) & 0xff);
    //write CONTROL_PADCONF_X. don't worry about the offset it has been checked by upper layers
    OUTREG32(&g_pPadCfg[padId],hwcfg);   

    return TRUE;
}

BOOL SOCGetPadConfig(UINT16 padId, UINT16* pCfg)
{
    UINT32 hwcfg;
    UINT16 cfg = 0;
	UINT32 modnum;
	

    MapPadRegisters();

    //read CONTROL_PADCONF_X. don't worry about the offset it has been checked by upper layers
    hwcfg = INREG32(&g_pPadCfg[padId]);   
    //Translate CONTROL_PADCONF_X format to the API format

    cfg |= (hwcfg & INPUT_ENABLE) ? INPUT_ENABLED : INPUT_DISABLED;
    cfg |= (hwcfg & MUX_SLEWCTRL_SLOW) ? SLEWCTRL_SLOW : SLEWCTRL_FAST;
    cfg |= ((hwcfg & PULL_DISABLE)?
              (PULL_RESISTOR_DISABLED):
              (PULL_RESISTOR_ENABLED | (hwcfg & PULL_UP ? PULLUP_RESISTOR : PULLDOWN_RESISTOR))
           );
			  
    for (modnum = 0, hwcfg & 0xff; modnum < 8; modnum++, hwcfg >>= 1 )
    	if (hwcfg & 1)
    		break;

    cfg |= (modnum & 0xf) << 3; 
 
    *pCfg = cfg;

    return TRUE;
}

//------------------------------------------------------------------------------

