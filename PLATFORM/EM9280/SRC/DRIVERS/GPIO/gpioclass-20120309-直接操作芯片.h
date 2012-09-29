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
//------------------------------------------------------------------------------
class GPIOClass
{
private:

    //-------------------------------------
    // GPIO critical section
    //
    CRITICAL_SECTION csGpioLock;

	DDK_IOMUX_PIN	*pGpioPinTab;
	DWORD			dwGpioPinTabLen;

	DDK_IOMUX_PIN	I2C_SCL_PIN;
	DDK_IOMUX_PIN	I2C_SDA_PIN;
	DDK_IOMUX_PIN	TCA6424A_INT_PIN;

	BYTE			uTCA6424A_Addr;
	DWORD			dwGPIOX_DOUT;			// valid with BIT0 - BIT23 
	DWORD			dwGPIOX_INV;			// valid with BIT0 - BIT23 
	DWORD			dwGPIOX_DIR;			// valid with BIT0 - BIT23 

	BOOL TCA6424A_GpioI2CWrite(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen);
	BOOL TCA6424A_GpioI2CRead(BYTE ucCmd, PBYTE pDatBuf, DWORD dwDatLen);

public:
    //-------------------------------------
    // ISA CONSTRUCTOR/DESTRUCTOR METHODS
    //
    GPIOClass(UINT32 index);
    ~GPIOClass(void);
 
	//
	// GPIO functions
	//
	BOOL PIO_OutEnable( UINT32 dwGpioBits );
	BOOL PIO_OutDisable( UINT32 dwGpioBits );
	BOOL PIO_OutSet( UINT32 dwGpioBits );
	BOOL PIO_OutClear( UINT32 dwGpioBits );
    BOOL PIO_State( UINT32* pStateBits );

	// other public functions
	void udelay(DWORD dwMicroSecond);
};


#ifdef __cplusplus
}
#endif

#endif   // __GPIOCLASS_H

