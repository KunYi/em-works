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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_connectivity.h
//
//  This header file defines Connectivity registers of MC13783.
//
//------------------------------------------------------------------------------

#ifndef __MC13783_REGS_CONVITY_H__
#define __MC13783_REGS_CONVITY_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define MC13783_USB0_FSENB_LSH					0
#define MC13783_USB0_USBSUSPEND_LSH				1
#define MC13783_USB0_USBPU_LSH					2
#define MC13783_USB0_UDPPD_LSH					3
#define MC13783_USB0_UDMPD_LSH					4
#define MC13783_USB0_DP150KPU_LSH					5
#define MC13783_USB0_VBUS70KPDENB_LSH				6
#define MC13783_USB0_VBUSPULSETMR_LSH				7
#define MC13783_USB0_DLPSRP_LSH					10
#define MC13783_USB0_SE0CONN_LSH					11
#define MC13783_USB0_USBXCVREN_LSH				12
#define MC13783_USB0_PULLOVR_LSH					13
#define MC13783_USB0_CONMODE_LSH					14
#define MC13783_USB0_DATSE0_LSH					17
#define MC13783_USB0_BIDIR_LSH					18
#define MC13783_USB0_USBCNTRL_LSH					19
#define MC13783_USB0_IDPD_LSH						20
#define MC13783_USB0_IDPULSE_LSH					21
#define MC13783_USB0_IDPUCNTRL_LSH				22
#define MC13783_USB0_DMPULSE_LSH					23

#define MC13783_CHG_USB1_VUSBIN_LSH				0
#define MC13783_CHG_USB1_VUSB_LSH					2
#define MC13783_CHG_USB1_VUSBEN_LSH				3
#define MC13783_CHG_USB1_VBUSEN_LSH				5
#define MC13783_CHG_USB1_RSPOL_LSH				6
#define MC13783_CHG_USB1_RSTRI_LSH				7
#define MC13783_CHG_USB1_ID100KPU_LSH				8

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define MC13783_USB0_FSENB_WID					1
#define MC13783_USB0_USBSUSPEND_WID				1
#define MC13783_USB0_USBPU_WID					1
#define MC13783_USB0_UDPPD_WID					1
#define MC13783_USB0_UDMPD_WID					1
#define MC13783_USB0_DP150KPU_WID					1
#define MC13783_USB0_VBUS70KPDENB_WID				1
#define MC13783_USB0_VBUSPULSETMR_WID				3
#define MC13783_USB0_DLPSRP_WID					1
#define MC13783_USB0_SE0CONN_WID					1
#define MC13783_USB0_USBXCVREN_WID				1
#define MC13783_USB0_PULLOVR_WID					1
#define MC13783_USB0_CONMODE_WID					3
#define MC13783_USB0_DATSE0_WID					1
#define MC13783_USB0_BIDIR_WID					1
#define MC13783_USB0_USBCNTRL_WID					1
#define MC13783_USB0_IDPD_WID						1
#define MC13783_USB0_IDPULSE_WID					1
#define MC13783_USB0_IDPUCNTRL_WID				1
#define MC13783_USB0_DMPULSE_WID					1

#define MC13783_CHG_USB1_VUSBIN_WID				2
#define MC13783_CHG_USB1_VUSB_WID					1
#define MC13783_CHG_USB1_VUSBEN_WID				1
#define MC13783_CHG_USB1_VBUSEN_WID				1
#define MC13783_CHG_USB1_RSPOL_WID				1
#define MC13783_CHG_USB1_RSTRI_WID				1
#define MC13783_CHG_USB1_ID100KPU_WID				1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

#define MC13783_USB0_FSENB_FULL_SPEED				0
#define MC13783_USB0_FSENB_LOW_SPEED				1

#define MC13783_USB0_USBSUSPEND_OFF				0
#define MC13783_USB0_USBSUSPEND_ON				1

#define MC13783_USB_PDPU_SWITCHED_OUT				0
#define MC13783_USB_PDPU_SWITCHED_IN				1

#define MC13783_USB0_VBUS70KPDENB_ENABLE	  		0
#define MC13783_USB0_VBUS70KPDENB_DISABLE			1

#define MC13783_USB0_VBUSPULSETMR_200MA			0
#define MC13783_USB0_VBUSPULSETMR_910UA_10MS		1
#define MC13783_USB0_VBUSPULSETMR_910UA_20MS		2
#define MC13783_USB0_VBUSPULSETMR_910UA_30MS		3
#define MC13783_USB0_VBUSPULSETMR_910UA_40MS		4
#define MC13783_USB0_VBUSPULSETMR_910UA_50MS		5
#define MC13783_USB0_VBUSPULSETMR_910UA_60MS		6
#define MC13783_USB0_VBUSPULSETMR_910UA			7

#define MC13783_USB0_DLPSRP_DISABLE				0
#define MC13783_USB0_DLPSRP_TRIGGER				1

#define MC13783_USB0_SE0CONN_AUTOCON_DISABLE		0
#define MC13783_USB0_SE0CONN_AUTOCON_ENABLE		1

#define MC13783_USB0_USBXCVREN_DISABLE			0
#define MC13783_USB0_USBXCVREN_ENABLE				1

#define MC13783_USB0_PULLOVR_DISABLE				0
#define MC13783_USB0_PULLOVR_ENABLE				1

#define MC13783_USB0_CONMODE_USB					0
#define MC13783_USB0_CONMODE_RS232_1				1
#define MC13783_USB0_CONMODE_RS232_2				2
#define MC13783_USB0_CONMODE_MONO					4
#define MC13783_USB0_CONMODE_STEREO				5
#define MC13783_USB0_CONMODE_TLEFT				6
#define MC13783_USB0_CONMODE_TRIGHT				7

#define MC13783_USB0_DATSE0_DIFF  				0
#define MC13783_USB0_DATSE0_SE					1

#define MC13783_USB0_BIDIR_UNIDIR					0
#define MC13783_USB0_BIDIR_BIDIR					1

#define MC13783_USB0_USBCNTRL_SPI					0
#define MC13783_USB0_USBCNTRL_USBEN				1

#define MC13783_USB0_IDPUCNTRL_R220K				0
#define MC13783_USB0_IDPUCNTRL_I5UA				1

#define MC13783_CHG_USB1_VUSB_2_775V				0
#define MC13783_CHG_USB1_VUSB_3_3V				1

#define MC13783_CHG_USB1_OP_DISABLE				0
#define MC13783_CHG_USB1_OP_ENABLE				1

#define MC13783_CHG_USB1_RSPOL_TXUDM_RXUDP		0
#define MC13783_CHG_USB1_RSPOL_TXUDP_RXUDM		1

#define MC13783_CHG_USB1_RSTRI_NONE				0
#define MC13783_CHG_USB1_RSTRI_TRISTATE			1


#ifdef __cplusplus
}
#endif

#endif // __MC13783_REGS_CONVITY_H__
