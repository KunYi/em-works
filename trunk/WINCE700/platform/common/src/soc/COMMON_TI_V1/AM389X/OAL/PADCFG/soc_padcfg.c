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

#include "am389x.h"
#include "am389x_config.h"
#include "sdk_padcfg.h"
#include "soc_cfg.h"

static UINT32* g_pPadCfg = NULL;

static _inline void MapPadRegisters()
{
    if (g_pPadCfg == NULL)
    {
        g_pPadCfg = OALPAtoUA(AM389X_SYSC_PADCONFS_REGS_PA);
    }
}

BOOL SOCSetPadConfig(UINT16 padId, UINT16 cfg)
{
    UINT32 hwcfg = 0;

    MapPadRegisters();
    
    //Translate config to CONTROL_PADCONF_X format
    hwcfg |= (cfg & PULL_RESISTOR_ENABLED) ? ((cfg & PULLUP_RESISTOR) ? PULL_UP : PULL_DOWN) : PULL_DISABLE;    
    hwcfg |= ((cfg >> 3) & 0x7) << 0;
    
    //write CONTROL_PADCONF_X. don't worry about the offset it has been checked by upper layers
//	OALMSG(1,(L"PAD[%d]@0x%08X:  0x%08X\r\n", padId, &g_pPadCfg[padId], hwcfg));
    OUTREG32(&g_pPadCfg[padId],hwcfg);   

    return TRUE;
}

BOOL SOCGetPadConfig(UINT16 padId, UINT16* pCfg)
{
    UINT32 hwcfg;
    UINT16 cfg = 0;    

    MapPadRegisters();

    //read CONTROL_PADCONF_X. don't worry about the offset it has been checked by upper layers
    hwcfg = INREG32(&g_pPadCfg[padId]);   
    //Translate CONTROL_PADCONF_X format to the API format
    // cfg |= (hwcfg & INPUT_ENABLE) ? INPUT_ENABLED : 0; // Netra doesn't have input enable bit ???

    cfg |= ((hwcfg & PULL_DISABLE)?
              (PULL_RESISTOR_DISABLED):
              (PULL_RESISTOR_ENABLED | (hwcfg & PULL_UP ? PULLUP_RESISTOR : PULLDOWN_RESISTOR))
           );
    cfg |= ((hwcfg >> 0) & 0x7) << 3;

    *pCfg = cfg;

    return TRUE;
}

//------------------------------------------------------------------------------

