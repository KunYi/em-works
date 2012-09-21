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
//  Copyright (C) 2011,Emtronix, Inc. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  isaclass.h
//
//  Header file, for isa_dio driver.
//
//------------------------------------------------------------------------------

#ifndef __GPIOCLASS_H
#define __GPIOCLASS_H

#ifdef __cplusplus
extern "C" {
#endif

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

//------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------

class GPIOClass
{
private:

	DDK_IOMUX_PIN	*pGpioPinTab;
	DWORD			*pTCA6424APinTab;
	DWORD			dwPinTabLen;

	DDK_IOMUX_PIN	TCA6424A_INT_PIN;

	BYTE			uTCA6424A_Addr;
	BYTE			uPortOut[3];
	BYTE			uPortDir[3];
	BYTE			uPortPinState[3];

	BOOL I2CInit();
	BOOL I2CWrite(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen);
	BOOL I2CRead(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen);
	BOOL I2CSet(BYTE ucCmd, BYTE uMask);
	BOOL I2CClear(BYTE ucCmd, BYTE uMask);

	BOOL	bIrqGpioPinEnabled;
	DWORD	dwDeviceID;				// @field logical interrupt number
	DWORD	dwIrqGpioPin;			// @filed enum { DDK_IOMUX_GPIO1_16 } 	
	DWORD   dwSysIntr;
	HANDLE	hIRQEvent;

public:
    //-------------------------------------
    // ISA CONSTRUCTOR/DESTRUCTOR METHODS
    //
    GPIOClass();
    ~GPIOClass();
 
	//
	// GPIO functions
	//
	BOOL PIO_OutEnable( UINT32 dwGpioBits );
	BOOL PIO_OutDisable( UINT32 dwGpioBits );
	BOOL PIO_OutSet( UINT32 dwGpioBits );
	BOOL PIO_OutClear( UINT32 dwGpioBits );
    BOOL PIO_State( UINT32* pStateBits );

	// other public functions
	void  udelay(DWORD dwMicroSecond);

	// GPIO input change interrupt function
	DWORD WaitGpioInterrupt(DWORD dwTimeout);
};


#ifdef __cplusplus
}
#endif

#endif   // __GPIOCLASS_H

