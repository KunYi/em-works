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
/******************************************************************************
 * File:		 aspen.h
 * Author:		 Naresh Gupta (nkgupta@hotmail.com)
 * Date:		 Thursday Sept 30, 1999
 * Organization: Renesas Technology Corp.
 * Purpose:		 Will contain all definitions specific to ASPEN
 *
 * Copyright (C) 1999 Renesas Technology Corp.
 ******************************************************************************/



/******************************************************************************
 * Important Note:-
 * ===============
 *
 *   To properly view this file, you need to set tabstop=4 in your editor
 *****************************************************************************/


#ifndef _ASPEN_H

#define _ASPEN_H


/******************************************************************************
 * User modifiable stuff
 *****************************************************************************/

/* 
 * Define DISABLE_PPSH if you do not want ppsh in the image. This is useful
 * to speed up booting time when ppsh is not present. 
 */
//#define DISABLE_PPSH

/******************************************************************************
 * End of User modifiable stuff
 *****************************************************************************/



/******************************************************************************
 * Peripheral Base Addresses for Aspen.
 *****************************************************************************/

// FPGA...
#define FPGA_BASE					0xA4000000

#define UNCACH_OFFSET				0x20000000
#define ALPHA_LED 					0xA4010060 	// Alphanumeric LEDS
#define LED_ALPHA 					ALPHA_LED 	// Same as above
#define LED_ALPHA_MIRROR			0x1f
#define PAR_BASE					0xA4020000  // Parallel Port
#define SW_RESET_REG				0xA4040020  // Software Reset address
	#define SW_RESET_KEY			0xDEAD		// Reset keyword
#define PILOT_LED_ADDR  			0xA4040030  // Pilot LEDs
#define FPGA_CMD_ADDR   			0xA4040000  // FPGA Command Register
#define FPGA_IMASK					0xA4040060  // Interrupts Mask
#define FPGA_ISTAT					0xA4040070  // Interrupts Status
#define ATAPI_BASE					0xA4050000  // IDE

// Companion Chips...
#define HD64465_BASE     			0xA4000000  // HD64465
#define HD64464_BASE				0xA4000000  // HD64464 if present.
#define SMC_BASE					0xA4400300  // Base of SMC91C100FD Ethernet
#define ETHERNET_BASE				SMC_BASE

// PCI...
#define PHYSICAL_PCI_MEM_BASE       0x10000000  // Base for Memory accesses
#define PCI_MEM_LENGTH				0x04000000
#define PHYSICAL_PCI_CONFIG_BASE    0x10000000  // Configuration access
#define PHYSICAL_PCI_IO_BASE        0x04300000  // I/O Access
#define PCI_IO_LENGTH				0x00040000
#define PHYSICAL_V3_INTERNAL_REG_BASE  0xA4200000 // Internal registers of V3

// RAM/Flash...
#define DRAM_BASE					0x8C000000  // RAM
#define DRV_GLOBAL_OFFSET			0x00150000  // Driver Globals
#define DRV_GLOBAL_BASE				(DRAM_BASE + DRV_GLOBAL_OFFSET+UNCACH_OFFSET)

#define CACHED_BASE					0x80000000
#define UNCACHED_BASE				0xA0000000
												

/******************************************************************************
 * ATAPI related definitions.
 *****************************************************************************/

#define ATAPI_SIZE					0xFFFF      

// Temporary buffer for ATAPI
#define ATAPI_WORKBASE				(DRV_GLOBAL_BASE + 0x1000)
#define ATAPI_WORKSIZE				0x020000


// ATAPI registers 
// TBD: Write full names of these registes in comments.
#define ATAPI_REG_DATW_OFFSET		0x0FC0

#define ATAPI_REG_AERR_OFFSET		0x0FC4
#define ATAPI_REG_FETR_OFFSET		0x0FC4

#define ATAPI_REG_IRRN_OFFSET		0x0FC8

#define ATAPI_REG_RSVE_OFFSET		0x0FCc

#define ATAPI_REG_BCTL_OFFSET		0x0FD0

#define ATAPI_REG_BCTH_OFFSET		0x0FD4

#define ATAPI_REG_DSEL_OFFSET		0x0FD8

#define ATAPI_REG_STAT_OFFSET		0x0FDc
#define ATAPI_REG_COMD_OFFSET		0x0FDc

#define ATAPI_REG_ASTA_OFFSET		0x07d8
#define ATAPI_REG_DCTR_OFFSET		0x07d8


/***************************************************************************** 
 * FPGA Interrupt Mask and Status Register 
 *****************************************************************************/

/* 
 * Please note that the Interrupt Mask register is actually an Interrupt
 * UnMask Register. Whatever bits are made 1, the corresponding interrupts
 * are allowed to occur
 */

/* No need to define ISTAT separately since the bit positions are the
 * same as the imask register.
 */
#define	FPGA_IMASK_APCI9			0x00800000  // Auto PC PCI interrupt
#define	FPGA_IMASK_APCI8			0x00400000  // Auto PC PCI interrupt
#define	FPGA_IMASK_APCI7			0x00200000  // Auto PC PCI interrupt
#define	FPGA_IMASK_APCI6			0x00100000  // Auto PC PCI interrupt
#define	FPGA_IMASK_APCI5			0x00080000  // Auto PC PCI interrupt
#define	FPGA_IMASK_APCI4			0x00040000  // Auto PC PCI interrupt
#define	FPGA_IMASK_APCI3			0x00020000  // Auto PC PCI interrupt
#define	FPGA_IMASK_APCI2			0x00010000  // Auto PC PCI interrupt
#define	FPGA_IMASK_IPCID			0x00008000  // PCI Power supply degraded
#define	FPGA_IMASK_IPCIE			0x00004000  // PCI ENUM#
#define	FPGA_IMASK_ILAN				0x00002000  // Ethernet
#define	FPGA_IMASK_IV3				0x00000800  // V3 interrupt
#define	FPGA_IMASK_IV3T				0x00000400  // V3 Timeout interrupt
#define	FPGA_IMASK_IPCIS			0x00000200  // PCI System error
#define	FPGA_IMASK_IPCIP			0x00000100  // PCI Parity error
#define	FPGA_IMASK_APCI				0x00000080  // AutoPCI interrupt
#define	FPGA_IMASK_IIDE				0x00000020  // IDE
#define	FPGA_IMASK_IIO				0x00000010  // Expansion I/o connector intr
#define	FPGA_IMASK_I465				0x00000008  // HD64465
#define	FPGA_IMASK_I464				0x00000004  // HD64464
#define	FPGA_IMASK_IRS2				0x00000002  // interrupt from RI (Ring
                                                // indicator from RS232 CN1.

#ifdef JUNK
/***************************General *****************************************/
#define VL(x) 						(*(volatile long *)(x))

/***************************LED *********************************************/
#define LED 						VL(PILOT_LED_ADDR )
#endif JUNK

/****************************************************************************** 
 * Flash related stuff
 *****************************************************************************/
typedef unsigned long 				flash_t;
#define FLASH_BANK_SIZE 			(1024 * 1024 * 16)
#define FLASH_BANK_COUNT 			2
#define FLASH_SIZE 					(FLASH_BANK_SIZE * FLASH_BANK_COUNT)

#define SECTOR_SIZE 				(0x20000L*2) // 128k sectors per I.C., 
												 // 2 I.C.'s on bus
#define LAST_SECTOR 				((FLASH_SIZE/SECTOR_SIZE)-1)
#define FLASH_START					0xa8000000
#define FLASH_END					(FLASH_START+FLASH_SIZE)
#define FLASH_BLOCK_SIZE    		SECTOR_SIZE
#define FLASH_BLOCK_QTY     		(LAST_SECTOR+1)
#define SECTOR_SHIFT        		18  // Bits to shift to get Sector number
#define FLASH_WIDTH 				(sizeof(flash_t) << 3)
#define FLASH_ADDRESS 				((flash_t *)0xa8000000)
#define ROM_ADDRESS 				(flash_t *)0xA0000000
#define ALL_FLASH 					-1

#define FLASH_READY 				0x00800080
#define FL_ADDRESS_ERROR    		1

/* Constants */
enum
{
FLASH_SUCCESS,
FLASH_BUSY,
FLASH_ERROR,
FLASH_TIMEOUT,
FLASH_ILLEGAL_SECTOR
};

/* Flash related addresses for ASPEN */
/* Aspen has 16 Mb of Flash */
#define FLASH_ADDR_START	0x80000000
#define FLASH_ADDR_END 		0x82000000
extern unsigned ulRamBufStart;

// Since we have 16 Mb of Flash currently, it's sufficient to have a 
// 16 Mb buffer for Flash data.
#define FLASH_CACHE			0x8c180000

/****************************************************************************** 
 * Start address of each AREA 
 *****************************************************************************/
#define	AREA_0						0x00000000
#define	AREA_1						0x04000000
#define	AREA_2						0x08000000
#define	AREA_3						0x0C000000
#define	AREA_4						0x10000000
#define	AREA_5						0x14000000
#define	AREA_6						0x18000000
#define	AREA_7						0x1C000000

/*****************************************************************************
 * Switch SW1
 ****************************************************************************/
#define USER_SW1					(FPGA_BASE+ 0x00040010)
#define USER_SW2					undefined 		// Not present on Aspen.

#define USER_SW1_BOOT_PROM			0x01
#define USER_SW1_MISCSW_N			0x02
#define USER_SW1_DEBUG_SER_N		0x04
#define USER_SW1_FLWREN				0x08
#define USER_SW1_FLASH_PROT_N		0x10
#define USER_SW1_SYSRESET0			0x20
#define USER_SW1_SYSRESET1			0x40
#define USER_SW1_SYSRESET2			0x80

// Right now the bits are flipped (1-4 is actually 4-1, and complimented,
// 1 is 0 and 0 is 1. Put a software patch meanwhile
#define USER_SW2_LM_ETHERNET		0x0C	// Load Monitor thru Ethernet.
#define USER_SW2_LM_SERIAL			0x04	// Load Monitor thru Serial.
#define USER_SW2_LM_PARALLEL		0x08	// Load Monitor thru Parallel.

/****************************************************************************** 
 * Parallel Port
 *****************************************************************************/
#define PAR_CONTROL_REG 			(PAR_BASE) 
#define PAR_DATA_REG				(PAR_BASE + 4) 

#define DF_PAR_CONTROL_REG							PAR_CONTROL_REG 
#define DF_PAR_EN									0x80000000
#define DF_PAR_AUTOEN								0x20000000
#define DF_PAR_BUSY									0x10000000
#define DF_PAR_NACK									0x08000000
#define DF_PAR_ERROR								0x04000000
#define DF_PAR_SELECT								0x02000000
#define DF_PAR_NFAULT								0x01000000
#define DF_PAR_INTR_MASK							0x00200000
#define DF_PAR_INTR									0x00100000
#define DF_PAR_SELECTIN								0x00080000
#define DF_PAR_INIT									0x00040000
#define DF_PAR_AUTOFD								0x00020000
#define DF_PAR_STROBE								0x00010000
#define DF_PAR_DATA_IN								0x0000FF00
#define DF_PAR_DATA_OUT								0x000000FF
#define DF_PAR_BUSY_NFAULT							(DF_PAR_BUSY | DF_PAR_NFAULT)
#define DF_PAR_BUSY_NFAULT_AUTOEN_SELECT			(DF_PAR_BUSY | DF_PAR_NFAULT | DF_PAR_AUTOEN | DF_PAR_SELECT)


/****************************************************************************** 
 * PCMCIA definitions
 *****************************************************************************/
#define PCMCIA0_ATTR_WIN_BASE		(AREA_6 + 0x00000000)
#define PCMCIA1_ATTR_WIN_BASE		(AREA_5 + 0x00000000)
#define PCMCIA_ATTR_WIN_SIZE		0x01000000				// 16 MB 

#define PCMCIA0_CMN_WIN_BASE		(AREA_6 + 0x01000000)
#define PCMCIA1_CMN_WIN_BASE		(AREA_5 + 0x01000000)
#define PCMCIA_CMN_WIN_SIZE			0x01000000  			// 16 MB 

#define PCMCIA0_IO_WIN_BASE			(AREA_6 + 0x02000000)
#define PCMCIA1_IO_WIN_BASE			(AREA_5 + 0x02000000)
#define PCMCIA_IO_WIN_SIZE			0x01000000  			// 16 MB 

#define PCMCIA_MAPPED_SYSTEM_WINDOW_SIZE			0x1000000	// Only 16 Mbyte mapped in system address space 
													   
#define PCMCIA_NUM_WINDOWS			6	


#if ENABLE_MQ200==1
# define MQ200_BASE									0xA6000000  //ASIC is MQ200
# define DSP_FRAME_BUFFER_OFFSET					0x01800000
# define DSP_FRAME_BUFFER_LENGTH					0x00200000
# define DSP_FRAME_BUFFER							(DSP_FRAME_BUFFER_OFFSET + MQ200_BASE)
#define PhysicaVmemAddr    					 		(MQ200_BASE + 0x01800000)

#else
# define DSP_FRAME_BUFFER_OFFSET						0x02000000
# define DSP_FRAME_BUFFER_LENGTH						0x00200000
// This needs to be moved to cached memory maybe later.
# define PhysicaVmemAddr    				 		(HD64464_BASE + 0x02000000)
# define CURSOR_BASE     							(HD64464_BASE + 0x020F0000)
#endif


/****************************************************************************** 
 * Timing related constants.
 *****************************************************************************/
/* TBD: Need to analyze and calculate the values that need to be put here.
 * Temporariliy using the same values as Aspen.
 */
#if 0
#define SYSTEM_CLOCK_FREQ                           199999998   // 200 MHz
#define USEC_TO_CLOCK_LOW                           (0x15555555 * 6)
#define USEC_TO_CLOCK_HIGH                          (0x00000002 * 6)
#define CLOCK_TO_USEC_LOW                           (0x7AE147AE / 6)
#define CLOCK_TO_USEC_HIGH                          (0x00000000 / 6)
#else
#if BSP_SH7750R
#define SYSTEM_CLOCK_FREQ                           60000000
#define USEC_TO_CLOCK_LOW                           0x00000000
#define USEC_TO_CLOCK_HIGH                          0x0000000F
#define CLOCK_TO_USEC_LOW                           0x10FFFFFF
#define CLOCK_TO_USEC_HIGH                          0x00000000
#else
#define SYSTEM_CLOCK_FREQ                           33333333
#define USEC_TO_CLOCK_LOW                           0x80000000
#define USEC_TO_CLOCK_HIGH                          0x0000000C
#define CLOCK_TO_USEC_LOW                           0x147AE148
#define CLOCK_TO_USEC_HIGH                          0x00000000
#endif
#endif

// Set interrupt priority 

#define INTC_IPRA_TMU0_INT							0x1000
#define INTC_IPRA_TMU1_INT							0x0E00
#define INTC_IPRA_TMU2_INT							0x00F0
#define INTC_IPRA_RTC_INT							0x000F
#define INTC_IPRB_WDT_INT							0x0000
#define INTC_IPRB_REF_INT							0x0000
#define INTC_IPRB_SCI_INT							0x0000
#define INTC_IPRC_DMAC_INT							0x0A00
#define INTC_IPRC_SCIF_INT							0x00B0
#define INTC_IPRC_JTAG_INT							0x0000

/* Dummy values, till i start working on interrupts. */
#define SYSINTR_PCI_SLOT0       1
#define SYSINTR_PCI_SLOT1       1
#define SYSINTR_PCI_SLOT2       1


// Extra type 
typedef volatile BYTE   *PVBYTE;
typedef volatile SHORT  *PVSHORT;
typedef volatile USHORT *PVUSHORT;
typedef volatile ULONG  *PVULONG;
typedef volatile DWORD  *PVDWORD;


#endif _ASPEN_H
