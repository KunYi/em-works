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
#ifndef __XLLP_USBHOST_H__
#define __XLLP_USBHOST_H__

/******************************************************************************
**
**  COPYRIGHT (C) 2002 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document. 
**  Except as permitted by such license, no part of this document may be 
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation. 
**
**  FILENAME:   xllp_clkmgr.h
**
**  PURPOSE:    contains all XLLP CLKMGR typedefs and bit definitions..
**
******************************************************************************/

#include "xllp_defs.h"

//
// Current to Bulverde EAS V1.5
//

//
// USB HOST OHCI (USBOHCI) Register Bank
//
typedef struct _XLLP_USBOHCI_S

{
    XLLP_VUINT32_T    uhcrev;		// HCI Spec Revision
    XLLP_VUINT32_T    uhchcon;		// Control register
    XLLP_VUINT32_T    uhccoms;		// Command Status
    XLLP_VUINT32_T    uhcints;		// Interrupt Status
    XLLP_VUINT32_T    uhcinte;		// Interrupt Enable Control register
    XLLP_VUINT32_T    uhcintd;		// Interrupt Disable Control register
    XLLP_VUINT32_T    uhchcca;		// Host controller Communication Area
    XLLP_VUINT32_T    uhcpced;		// Period Current Endpoint Descriptor
    XLLP_VUINT32_T    uhcched;		// Control Head Endpoint Descriptor register
    XLLP_VUINT32_T    uhccced;		// Control Current Endpoint Descriptor register
    XLLP_VUINT32_T    uhcbhed;		// Bulk Head Endpoint Descriptor register
    XLLP_VUINT32_T    uhcbced;		// Bulk Current Endpoint Descriptor register
    XLLP_VUINT32_T    uhcdhead;		// Done head register
    XLLP_VUINT32_T    uhcfmi;		// Frame Interval register
    XLLP_VUINT32_T    uhcfmr;		// Frame Remaining register
    XLLP_VUINT32_T    uhcfmn;		// Frame Number register
    XLLP_VUINT32_T    uhcpers;		// Periodic Start register
    XLLP_VUINT32_T    uhclst;		// Low Speed Threshold register
    XLLP_VUINT32_T    uhcrhda;		// Root Hub Descriptor A register
    XLLP_VUINT32_T    uhcrhdb;		// Root Hub Descriptor B register
    XLLP_VUINT32_T    uhcrhs;		// Root Hub Status register
    XLLP_VUINT32_T    uhcrhps1;		// Root Hub Port 1 Status register
    XLLP_VUINT32_T    uhcrhps2;		// Root Hub Port 2 Status register
    XLLP_VUINT32_T    reserved;		// Reserved
    XLLP_VUINT32_T    uhcstat;		// USB Host Status
    XLLP_VUINT32_T    uhchr;		// USB Host Reset
    XLLP_VUINT32_T    uhchie;		// USB Host Interrupt Enable
    XLLP_VUINT32_T    uhchit;		// USB Host Interrupt Test

 } XLLP_USBOHCI_T, *P_XLLP_USBOHCI_T;



// fields and bits for uhcrev:		HCI Spec Revision
#define XLLP_USBOHCI_UHCREV_OHCISPEC1_0_A		0x10

// fields and bits for uhchcon:		Control register
#define XLLP_USBOHCI_UHCCON_CBSR_MASK			( 0x3u << 0 )

#define	XLLP_USBOHCI_UHCCON_CBSR_PLE			( 2u   << 2 )
#define	XLLP_USBOHCI_UHCCON_CBSR_IE				( 3u   << 2 )
#define	XLLP_USBOHCI_UHCCON_CBSR_CLE			( 4u   << 2 )
#define	XLLP_USBOHCI_UHCCON_CBSR_BLE			( 5u   << 2 )

#define XLLP_USBOHCI_UHCCON_HCFS_MASK			( 0x3u << 6 )

#define	XLLP_USBOHCI_UHCCON_CBSR_IR				( 8u   << 2 )
#define	XLLP_USBOHCI_UHCCON_CBSR_RWC			( 9u   << 2 )
#define	XLLP_USBOHCI_UHCCON_CBSR_RWE			( 10u  << 2 )

// for the HCFS field
#define XLLP_USBOHCI_UHCCON_HCFS_USBRESET		( 0u << 6 )
#define XLLP_USBOHCI_UHCCON_HCFS_USBRESUME		( 1u << 6 )
#define XLLP_USBOHCI_UHCCON_HCFS_USBOPERATIONAL	( 2u << 6 )
#define XLLP_USBOHCI_UHCCON_HCFS_USBSUSPEND		( 3u << 6 )


// fields and bits for uhccoms:		Command Status
#define	XLLP_USBOHCI_UHCCOMS_HCR				( 1u   << 0)
#define	XLLP_USBOHCI_UHCCOMS_CLF				( 1u   << 1 )
#define	XLLP_USBOHCI_UHCCOMS_BLF				( 1u   << 2 )
#define	XLLP_USBOHCI_UHCCOMS_OCR				( 1u   << 3 )

#define XLLP_USBOHCI_UHCCOMS_SOC_MASK			( 0x3u << 16 )



// fields and bits for uhcints:		Interrupt Status
// fields and bits for uhcinte:		Interrupt Enable Control register
// fields and bits for uhcintd:		Interrupt Disable Control register
#define	XLLP_USBOHCI_UHCINT_SO					( 1u   << 0 )
#define	XLLP_USBOHCI_UHCINT_WDH					( 1u   << 1 )
#define	XLLP_USBOHCI_UHCINT_SF					( 1u   << 2 )
#define	XLLP_USBOHCI_UHCINT_RD					( 1u   << 3 )
#define	XLLP_USBOHCI_UHCINT_UE					( 1u   << 4 )
#define	XLLP_USBOHCI_UHCINT_FNO					( 1u   << 5 )
#define	XLLP_USBOHCI_UHCINT_RHSC				( 1u   << 6 )
#define	XLLP_USBOHCI_UHCINT_OC					( 1u   << 30 )
#define	XLLP_USBOHCI_UHCINT_MIE					( 1u   << 31 )

// fields and bits for uhchcca:		Host controller Communication Area
#define XLLP_USBOHCI_UHCHCCA_MASK				( 0xfffffffu << 8 )

// fields and bits for uhcpced:		Period Current Endpoint Descriptor
#define XLLP_USBOHCI_UHCPCED_MASK				( 0xffffffffu << 4 )

// fields and bits for uhcched:		Control Head Endpoint Descriptor register
#define XLLP_USBOHCI_UHCCHED_MASK				( 0xffffffffu << 4 )

// fields and bits for uhccced:		Control Current Endpoint Descriptor register
#define XLLP_USBOHCI_UHCCCED_MASK				( 0xffffffffu << 4 )

// fields and bits for uhcbhed:		Bulk Head Endpoint Descriptor register
#define XLLP_USBOHCI_UHCBHED_MASK				( 0xffffffffu << 4 )

// fields and bits for uhcbced:		Bulk Current Endpoint Descriptor register
#define XLLP_USBOHCI_UHCBCED_MASK				( 0xffffffffu << 4 )

// fields and bits for uhcdhead:	Done head register
#define XLLP_USBOHCI_UHCDHED_MASK				( 0xffffffffu << 4 )	// should be "DHTD" because its a transfer descriptor

// fields and bits for uhcfmi:		Frame Interval register
#define XLLP_USBOHCI_UHCFMI_FI_MASK				( 0x3fffu << 0 )
#define XLLP_USBOHCI_UHCFMI_FSMPS_MASK			( 0x7fffu << 16 )
#define	XLLP_USBOHCI_UHCFMI_FIT					( 1u   << 31 )

// fields and bits for uhcfmr:		Frame Remaining register
#define XLLP_USBOHCI_UHCFMR_FR_MASK				( 0x3fffu << 0 )
#define	XLLP_USBOHCI_UHCFMI_FRT					( 1u   << 31 )

// fields and bits for uhcfmn:		Frame Number register
#define XLLP_USBOHCI_UHCFMN_FN_MASK				( 0xffffu << 0 )

// fields and bits for uhcpers:		Periodic Start register
#define XLLP_USBOHCI_UHCPERS_PS_MASK			( 0x3fffu << 0 )

// fields and bits for uhclst:		Low Speed Threshold register
#define XLLP_USBOHCI_UHCPLST_LST_MASK			( 0xfffu << 0 )

// fields and bits for uhcrhda:		Root Hub Descriptor A register
#define XLLP_USBOHCI_UHCRHDA_NDP_MASK			( 0xffu << 0 )

#define	XLLP_USBOHCI_UHCRHDA_PSM				( 1u   << 8 )
#define	XLLP_USBOHCI_UHCRHDA_NPS				( 1u   << 9 )
#define	XLLP_USBOHCI_UHCRHDA_DT					( 1u   << 10 )
#define	XLLP_USBOHCI_UHCRHDA_OCPM				( 1u   << 11 )
#define	XLLP_USBOHCI_UHCRHDA_NOCP				( 1u   << 12 )

#define XLLP_USBOHCI_UHCRHDA_POTPGT_MASK		( 0xffu << 24 )

// fields and bits for uhcrhdb:		Root Hub Descriptor B register
#define XLLP_USBOHCI_UHCRHDB_DR_MASK			( 0xffffu << 0 )
#define XLLP_USBOHCI_UHCRHDB_PPCM_MASK			( 0xffffu << 16 )

// fields and bits for uhcrhs:		Root Hub Status register
#define	XLLP_USBOHCI_UHCRHS_LPS					( 1u   << 0 )		// meaning on read
#define	XLLP_USBOHCI_UHCRHS_CGP					( 1u   << 0 )		// meaining on write
#define	XLLP_USBOHCI_UHCRHS_OCI					( 1u   << 1 )
#define	XLLP_USBOHCI_UHCRHS_DRWE				( 1u   << 15 )		// meaning on read
#define	XLLP_USBOHCI_UHCRHS_SRWE				( 1u   << 15 )		// meaning on write
#define	XLLP_USBOHCI_UHCRHS_LPSC				( 1u   << 16 )		// meaning on read
#define	XLLP_USBOHCI_UHCRHS_SGP					( 1u   << 16 )		// meaning on write
#define	XLLP_USBOHCI_UHCRHS_OCIC				( 1u   << 17 )
#define	XLLP_USBOHCI_UHCRHS_CRWE				( 1u   << 31 )

// fields and bits for uhcrhps1:	Root Hub Port 1 Status register
// fields and bits for uhcrhps2:	Root Hub Port 2 Status register
#define	XLLP_USBOHCI_UHCRHPS_CCS				( 1u   << 0 )
#define	XLLP_USBOHCI_UHCRHPS_PES				( 1u   << 1 )
#define	XLLP_USBOHCI_UHCRHPS_PSS				( 1u   << 2 )
#define	XLLP_USBOHCI_UHCRHPS_POCI				( 1u   << 3 )
#define	XLLP_USBOHCI_UHCRHPS_PRS				( 1u   << 4 )
#define	XLLP_USBOHCI_UHCRHPS_PPS				( 1u   << 8 )
#define	XLLP_USBOHCI_UHCRHPS_LSDA				( 1u   << 9 )		// meaning on read
#define	XLLP_USBOHCI_UHCRHPS_CPP				( 1u   << 9 )		// meaning on write
#define	XLLP_USBOHCI_UHCRHPS_CSC				( 1u   << 16 )
#define	XLLP_USBOHCI_UHCRHPS_PESC				( 1u   << 17 )
#define	XLLP_USBOHCI_UHCRHPS_PSSC				( 1u   << 18 )
#define	XLLP_USBOHCI_UHCRHPS_POCIC				( 1u   << 19 )
#define	XLLP_USBOHCI_UHCRHPS_PRSC				( 1u   << 20 )

// fields and bits for uhcstat:		USB Host Status
#define	XLLP_USBOHCI_UHCSTAT_RWUE				( 1u   << 7 )
#define	XLLP_USBOHCI_UHCSTAT_HBA				( 1u   << 8 )
#define	XLLP_USBOHCI_UHCSTAT_HTA				( 1u   << 10 )
#define	XLLP_USBOHCI_UHCSTAT_UPS1				( 1u   << 11 )
#define	XLLP_USBOHCI_UHCSTAT_UPS2				( 1u   << 12 )
#define	XLLP_USBOHCI_UHCSTAT_UPRI				( 1u   << 13 )
#define	XLLP_USBOHCI_UHCSTAT_SBTAI				( 1u   << 14 )
#define	XLLP_USBOHCI_UHCSTAT_SBMAI				( 1u   << 15 )

// fields and bits for uhchr:		USB Host Reset
// UHCHR host reset register bit positions
#define XLLP_USBOHCI_UHCHR_SSEP1				( 1u << 10)	// Sleep Standby Enable: enable/disable port1 SE receivers & power  supply
#define XLLP_USBOHCI_UHCHR_SSEP0				( 1u <<  9)	// Sleep Standby Enable: enable/disable port1 SE receivers & power  supply
#define XLLP_USBOHCI_UHCHR_PCPL					( 1u <<  7)	// Power CONTROL Polarity: Control polarity of Power Enable signals output to the MAX1693EUB USB Power Switch
#define XLLP_USBOHCI_UHCHR_PSPL					( 1u <<  6)	// Power SENSE Polarity: Control polarity of Over-current Indicator signals input from the MAX1693EUB USB Power Switch
#define XLLP_USBOHCI_UHCHR_SSE					( 1u <<  5)	// Sleep Standby Enable: enable/disable both ports SE receivers & power  supply
#define XLLP_USBOHCI_UHCHR_UIT					( 1u <<  4)	// USB Interrupt Test: Enable Interrupt Test Mode and UHCHIT register
#define XLLP_USBOHCI_UHCHR_SSDC					( 1u <<  3)	// Simulation Scale Down Clock: When 1, internal 1 mSec timer changes to 1 uSec to speed up simulations
#define XLLP_USBOHCI_UHCHR_CGR					( 1u <<  2)	// Clock Generation Reset: When 0, resets the OHCI Clock Generation block (DPLL). Used only in simulation
#define	XLLP_USBOHCI_UHCHR_FHR					( 1u <<  1)	// Force Host controller Reset: When 1, resets OHCI core. Must be held high for 10 uSeconds, then cleared
#define XLLP_USBOHCI_UHCHR_FSBIR				( 1u <<  0)	// Force System Bus Interface Reset: When 1, resets the logic that interfaces to the system bus, DMA, etc. Auto clears after three system bus clocks.

// fields and bits for uhchie:		USB Host Interrupt Enable
#define	XLLP_USBOHCI_UHCIE_RWIE					( 1u   << 7 )
#define	XLLP_USBOHCI_UHCIE_HBAIE				( 1u   << 8 )
#define	XLLP_USBOHCI_UHCIE_TAIE					( 1u   << 10 )
#define	XLLP_USBOHCI_UHCIE_UPS1IE				( 1u   << 11 )
#define	XLLP_USBOHCI_UHCIE_UPS2IE				( 1u   << 12 )
#define	XLLP_USBOHCI_UHCIE_UPRIE				( 1u   << 13 )


// fields and bits for uhchit:		USB Host Interrupt Test
#define	XLLP_USBOHCI_UHCIT_RWIT					( 1u   << 7 )
#define	XLLP_USBOHCI_UHCIT_BAT					( 1u   << 8 )
#define	XLLP_USBOHCI_UHCIT_IRQT					( 1u   << 9 )
#define	XLLP_USBOHCI_UHCIT_TAT					( 1u   << 10 )
#define	XLLP_USBOHCI_UHCIT_UPS1T				( 1u   << 11 )
#define	XLLP_USBOHCI_UHCIT_UPS2T				( 1u   << 12 )
#define	XLLP_USBOHCI_UHCIT_UPRT					( 1u   << 13 )
#define	XLLP_USBOHCI_UHCIT_STAT					( 1u   << 14 )
#define	XLLP_USBOHCI_UHCIT_SMAT					( 1u   << 15 )




// Port Power Managment Modes
enum {
	XLLP_USBOHCI_PPM_NPS,
	XLLP_USBOHCI_PPM_GLOBAL,
	XLLP_USBOHCI_PPM_PERPORT,
	XLLP_USBOHCI_PPM_MIXED
};

// root hub descriptor A information
#define	XLLP_USBOHCI_UHCRHDA_PSM_PERPORT		1

#endif