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

#ifndef __ISACLASS_H
#define __ISACLASS_H

#include "bsp.h"

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
typedef struct _GPIO_INFO
{
	DDK_IOMUX_PIN						iomux_pin;
	DDK_IOMUX_PAD					iomux_pad;
	DDK_IOMUX_PIN_MUXMODE		muxmode;
	DDK_IOMUX_PIN_SION			sion;
	DDK_GPIO_PORT						gpioport;
	UINT32									gpio_pin;
}GPIO_INFO, *PGPIO_INFO;

class ISAClass
{
private:

    //-------------------------------------
    // ISA bus critical section
    //
    CRITICAL_SECTION gcsISABusLock;

	PGPIO_INFO  pGPIOTab;

	//
	// CS&ZHL JUN-24-2011: loacal variances for EM9170 CPLD access
	//
	PEM9K_CPLD_REGS	pCPLD;
	DWORD						dwCPLDSize;			// we only use A0 - A6
	DWORD						dwISACtrlReg;		// D0: ISA_EN; D1: CS0_EN
	UINT32						dwGPIOEnable;		// 32-bit, =1: enable; = 0: disable

public:
    //-------------------------------------
    // ISA CONSTRUCTOR/DESTRUCTOR METHODS
    //
    ISAClass(UINT32 index);
    ~ISAClass(void);
 
	//
	// GPIO functions
	//
	int	PIO_OutEnable( UINT32 EnBits );
	int PIO_OutDisable( UINT32 DisBits );
	int PIO_OutSet( UINT32 SetBits );
	int PIO_OutClear( UINT32 CleaeBits );
    int PIO_State( UINT32* pInValue );

	//
	// ISA read/write functions
	//
	int	ISA_ReadUchar( int nSeg, UINT nOffset, UCHAR* pRdValue );
	int	ISA_WriteUchar( int nSeg, UINT nOffset, UCHAR  WrValue );

	int ISA_Reset( UINT nMilliseconds );
};


#ifdef __cplusplus
}
#endif

#endif   // __ISACLASS_H

