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

#ifndef __SDCONTROLLER_H
#define __SDCONTROLLER_H

#include "../base/sdhc.h"
#include "menelaus.h"
#include <omap2420.h>
#include <bsp_i2c_cfg.h>

class CSDIOController : public CSDIOControllerBase
{
public:
    CSDIOController();
    virtual ~CSDIOController();

protected:
    BOOL IsWriteProtected();
    BOOL SDCardDetect();
    DWORD SDHCCardDetectIstThreadImpl();
    virtual BOOL InitializeHardware();
    virtual void DeinitializeHardware();
    virtual BOOL GetRegistrySettings( CReg *pReg );
    virtual VOID TurnCardPowerOn();
    virtual VOID TurnCardPowerOff();

	DWORD m_dwCardReadWriteSlot1GPIO;
	DWORD m_dwCardReadWriteSlot2GPIO;
    DWORD m_dwCardWriteProtectedState;

	OMAP2420_GPIO_REGS *m_pGPIO1Regs;
	OMAP2420_GPIO_REGS *m_pGPIO4Regs;
	CMenelaus& m_Menelaus;
};
#define SHC_SLOT1_WRITEPROTECT_KEY  TEXT("CardReadWriteSlot1GPIO")
#define SHC_SLOT2_WRITEPROTECT_KEY  TEXT("CardReadWriteSlot2GPIO")
#define SHC_CARD_WRITE_PROTECTED_STATE_KEY TEXT("CardWriteProtectedState")
#endif // __SDCONTROLLER_H

// DO NOT REMOVE --- END EXTERNALLY DEVELOPED SOURCE CODE ID --- DO NOT REMOVE
