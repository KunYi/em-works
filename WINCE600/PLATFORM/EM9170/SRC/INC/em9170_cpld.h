//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Header: cpld.h
//
//  Provides definitions for the CPLD logic on the 3DS board.
//
//------------------------------------------------------------------------------
#ifndef __EM9170_CPLD_H
#define __EM9170_CPLD_H

#if    __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

// IRQ line that CPLD connects to SoC

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
	UCHAR	TypeReg;				// OFFSET = 0
	UCHAR	SubTypeReg;			// OFFSET = 1, 
	UCHAR	ResetReg;				// OFFSET = 2, 
	UCHAR	DebugReg;			// OFFSET = 3, 
	UCHAR	ISACtrlReg;			// OFFSET = 4, 
	UCHAR	PwrBtnReg;			// OFFSET = 5,
	UCHAR	PWMCtrlReg;			// OFFSET = 6, 
	UCHAR	StateReg;				// OFFSET = 7, 
	UCHAR Reserved[80];
	UCHAR	VID[4];					// OFFSET = 0x58, Vendor ID = "em9k"
	UCHAR	UID[4];					// OFFSET = 0x5C, User ID, default = {0}
	UCHAR	ISA_CS0[16];		// OFFSET = 0x60, 
	UCHAR	ISA_CS1[16];		// OFFSET = 0x70, 
} EM9K_CPLD_REGS, *PEM9K_CPLD_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define EM9K_CPLD_TYEP_VER_1					(1 << 0)
#define EM9K_CPLD_TYEP_VER_2					(2 << 0)
#define EM9K_CPLD_TYEP_VER_3					(3 << 0)
#define EM9K_CPLD_TYEP_VER_4					(4 << 0)
#define EM9K_CPLD_TYEP_VER_5					(5 << 0)
#define EM9K_CPLD_TYEP_VER_6					(6 << 0)
#define EM9K_CPLD_TYEP_VER_7					(7 << 0)
#define EM9K_CPLD_TYEP_SPEED_STD			(0 << 3)
#define EM9K_CPLD_TYEP_SPEED_HIGH		(1 << 3)
#define EM9K_CPLD_TYEP_SUBTYPE				(0 << 4)
#define EM9K_CPLD_TYEP_9160					(1 << 4)
#define EM9K_CPLD_TYEP_9260					(2 << 4)
#define EM9K_CPLD_TYEP_9360					(3 << 4)
#define EM9K_CPLD_TYEP_9460					(4 << 4)
#define EM9K_CPLD_TYEP_9161					(5 << 4)
#define EM9K_CPLD_TYEP_9170					(6 << 4)
#define EM9K_CPLD_TYEP_9180					(7 << 4)
#define EM9K_CPLD_TYEP_OS_CE				(0 << 7)
#define EM9K_CPLD_TYEP_OS_LINUX			(1 << 7)

#define EM9K_CPLD_ISACTRL_ISAEN			(1 << 0)
#define EM9K_CPLD_ISACTRL_CS0EN			(1 << 1)

#define EM9K_CPLD_PWRBTN_EN					(1 << 4)

#define EM9K_CPLD_PWMCTRL_EN				(1 << 0)

#define EM9K_CPLD_STATE_ISA_EN				(1 << 0)
#define EM9K_CPLD_STATE_CS0_EN			(1 << 1)
#define EM9K_CPLD_STATE_CPU_EN			(1 << 2)
#define EM9K_CPLD_STATE_OUT_EN			(1 << 3)
#define EM9K_CPLD_STATE_ON_OFF_EN		(1 << 4)
#define EM9K_CPLD_STATE_DBGSL				(1 << 5)
#define EM9K_CPLD_STATE_DEBUG_FLAG	(1 << 7)


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//------------------------------------------------------------------------------
BOOL EM9170_IOConfig(void);
BOOL EM9170_ISABusSetup(void);

//
// CS&ZHL JUN-27-2011: 
//
typedef struct
{
	void			(*pfnKickWatchDog)(void);
	DWORD		dwWatchDogPeriod;
	DWORD		dwWatchDogThreadPriority;
} WATCHDOG_INFO, *PWATCHDOG_INFO;


#ifdef __cplusplus
}
#endif

#endif // __EM9170_CPLD_H
