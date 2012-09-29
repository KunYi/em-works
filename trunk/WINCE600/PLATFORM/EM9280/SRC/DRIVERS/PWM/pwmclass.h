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

#ifndef __PWMCLASS_H
#define __PWMCLASS_H

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
// CS&ZHL JLY17-2012: Defines
//------------------------------------------------------------------------------
typedef struct _PWM_PIN_INFO
{
	DDK_IOMUX_PIN					iomux_pin;
	DDK_IOMUX_PIN_MUXMODE			muxmode;
	DWORD							pwmIndex;
	DWORD							pwmEnble;
}PWM_PIN, *PPWM_PIN;


//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
class PWMClass
{
private:

	PPWM_PIN	pPwmPinTab;
	PVOID	pv_HWRegPWM;
	DWORD	m_dwIndex;
	DWORD	m_dwFreq;
	DWORD	m_dwDuty;
	DWORD	m_dwResolution;

	DWORD	m_dwHsadcClock;
	DWORD	m_dwXtalClock;
	DWORD	m_dwPwmIndex;
	DWORD	m_dwPwmEnable;

	DWORD	m_dwCDIV_Index;
	DWORD	m_dwCDIV_TAB[8];

public:
    //-------------------------------------
    // ISA CONSTRUCTOR/DESTRUCTOR METHODS
    //
    PWMClass(DWORD dwIndex);
    ~PWMClass();
 
	//
	// PWM functions
	//
	DWORD GetIndex( ) { return m_dwIndex; }
	DWORD GetFreq( ) { return m_dwFreq; }
	DWORD GetDuty( ) { return m_dwDuty; }
	DWORD GetResolution( ) { return m_dwResolution; }
	BOOL  PinConfig( );
	BOOL  OutputConfig(DWORD dwFreq, DWORD dwDuty, DWORD dwResolution);
};


#ifdef __cplusplus
}
#endif

#endif   // __PWMCLASS_H

