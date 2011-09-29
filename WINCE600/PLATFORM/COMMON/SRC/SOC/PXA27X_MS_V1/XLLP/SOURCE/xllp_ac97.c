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
 **
 **  COPYRIGHT (C) 2000, 2001 Intel Corporation.
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
 **  FILENAME:       xllp_ac97.c
 **
 **  PURPOSE:        XLLP for Bulverde's AC'97 Controller Unit.
 **                  Includes initialization, API and support
 **                  functions.  Corresponds to XLLP_AC97_LLD.doc.
 **
 **  Valid for    :  Subset of AC '97 Rev 2.1
 **
 **
 ******************************************************************************/

#include "xllp_ac97.h"
#include "xllp_ac97acodec.h"
#include "xllp_ost.h"
#include "xllp_clkmgr.h"
#include "xllp_gpio.h"
#include "xllp_intc.h"

static XLLP_BOOL_T XllpAc97LinkLock(P_XLLP_AC97_T pAc97);


/*******************************************************************************
 *
 * FUNCTION:         XllpAc97Init
 *
 * DESCRIPTION:      Set up AC'97 relative GPIOs.
 *                   Enable AC'97 Controller device clock.
 *                   Perform a cold reset of the AC'97 controller and codec(s).
 *                   
 *                   Note: Does not enable any interrupt types within the 
 *                     ACUNIT, so no interrupts should occur at this point.
 *
 * INPUT PARAMETERS: 
 *       pAc97ctxt   a pointer to a XLLP_AC97_CONTEXT_T struct, which contains 
 *						necessary setup information.
 *
 * RETURNS:   
 *       Success:    0 (XLLP_AC97_NO_ERROR)
 *       Failure:    XLLP_AC97_CODEC_NOT_READY
 *
 * GLOBAL EFFECTS:   
 *
 * ASSUMPTIONS:      - No other systems, such as debuggers, are using the 
 *                     AC '97 Controller or codecs, or the AC Link.
 *                   - The software system will be unharmed by a cold reset of 
 *                     the entire AC '97 subsystem and any associated devices.
 *
 * CALLS:            XllpAc97ColdReset      
 *
 * CALLED BY:        
 *
 * PROTOTYPE:        XLLP_AC97_ERROR_T  XllpAc97Init(XLLP_AC97_CONTEXT_T *pAc97ctxt);
 *
 *******************************************************************************/

XLLP_AC97_ERROR_T  XllpAc97Init(XLLP_AC97_CONTEXT_T *pAc97ctxt)
{
    XLLP_AC97_ERROR_T	status ;
    P_XLLP_GPIO_T		pGPIO = pAc97ctxt->pGpioReg;
	P_XLLP_INTC_T		pINTC = pAc97ctxt->pIntcReg;
	P_XLLP_CLKMGR_T		pCLKMGR = pAc97ctxt->pClockReg;
    XLLP_UINT32_T		pins[6], fn[6];
	
    // Set bitclk, sdata_in_0
	pins[0] = 2;
    pins[1] = XLLP_GPIO_AC97BITCLK;
    pins[2] = XLLP_GPIO_AC97_SDATA_IN_0;
    fn[0] = 2;
    fn[1] = XLLP_GPIO_ALT_FN_1;
    fn[2] = XLLP_GPIO_ALT_FN_1;
    if (XLLP_TRUE == pAc97ctxt->useSecondaryCodec)
    {
		pins[0] = 3;
		pins[3] = XLLP_GPIO_KP_MKIN5;      // use this pin as AC97_SDATA_IN_1 
		fn[0] = 3;
		fn[3] = XLLP_GPIO_ALT_FN_2;
	}
	XllpGpioSetDirectionIn(pGPIO, pins);
    XllpGpioSetAlternateFn(pGPIO, pins, fn);
    
    // Set sdata_out, sync, sysclock
    pins[0] = 2;
    pins[1] = XLLP_GPIO_AC97_SDATA_OUT;
    pins[2] = XLLP_GPIO_AC97_SYNC;
//    pins[3] = XLLP_GPIO_KP_MKIN4;     // use this pin as AC97_SYSCLK 
    fn[0] = 2;
    fn[1] = XLLP_GPIO_ALT_FN_2;
    fn[2] = XLLP_GPIO_ALT_FN_2;
//    fn[3] = XLLP_GPIO_ALT_FN_1;
    XllpGpioSetOutputState1(pGPIO, pins);
    XllpGpioSetDirectionOut(pGPIO, pins);
    XllpGpioSetAlternateFn(pGPIO, pins, fn);
    
    // Set sdata_reset_n
    pins[0] = 1;
    pins[1] = XLLP_GPIO_AC97_RESET_n;
    fn[0] = 1;
    fn[1] = XLLP_GPIO_ALT_FN_0;
    XllpGpioSetOutput0(pGPIO, pins);
    XllpGpioSetDirectionOut(pGPIO, pins);
    XllpGpioSetAlternateFn(pGPIO, pins, fn);
    
    // Disable of ACUNIT interrupt.
    pINTC->icmr &= ~XLLP_INTC_AC97;
	
    // Enable clocking of AC '97 controller device in processor
    pCLKMGR->cken |= XLLP_CLKEN_AC97;
	
    // Perform the cold reset.
    // Also enables the codec(s), control unit and the control unit's FIFOs
    status = XllpAc97ColdReset(pAc97ctxt);
    
    return (status);

} // End XllpAc97Init ()

/*******************************************************************************
 *
 * FUNCTION:         XllpAc97DeInit
 *
 * DESCRIPTION:      Shutdown AC97 unit clearly and thoroughly.
 *                   Release GPIOs used by AC97.
 *                   Disable clock to AC97 unit and interrupts from it. 
 *
 * INPUT PARAMETERS: 
 *       pAc97ctxt   a pointer to a XLLP_AC97_CONTEXT_T struct, which contains 
 *						necessary information.
 *
 * RETURNS:
 *       Success:    0 (XLLP_AC97_NO_ERROR)
 *       Failure:    XLLP_AC97_LINK_SHUTDOWN_FAIL
 *
 * CALLS:            XllpAc97ShutdownAclink
 *
 * CALLED BY:        
 *
 * PROTOTYPE:        XLLP_AC97_ERROR_T  XllpAc97DeInit(P_XLLP_AC97_CONTEXT_T pAc97ctxt);
 *
 *******************************************************************************/

XLLP_AC97_ERROR_T  XllpAc97DeInit(P_XLLP_AC97_CONTEXT_T pAc97ctxt)
{
    XLLP_AC97_ERROR_T	status ;
    P_XLLP_GPIO_T		pGPIO = pAc97ctxt->pGpioReg;
	P_XLLP_INTC_T		pINTC = pAc97ctxt->pIntcReg;
	P_XLLP_CLKMGR_T		pCLKMGR = pAc97ctxt->pClockReg;
    XLLP_UINT32_T		pins[8];
	
    status = XllpAc97ShutdownAclink(pAc97ctxt->pAc97Reg, pAc97ctxt->pOstRegs);

	if (XLLP_AC97_NO_ERROR == status)
	{
        // Disable of ACUNIT interrupt.
	    pINTC->icmr &= ~XLLP_INTC_AC97;

	    // Disable clocking of AC '97 controller device in processor
	    pCLKMGR->cken &= ~XLLP_CLKEN_AC97;

		// Set all pins to default general input configuration.
    	pins[0] = 6;
        pins[1] = XLLP_GPIO_AC97BITCLK;
        pins[2] = XLLP_GPIO_AC97_SDATA_IN_0;
        pins[3] = XLLP_GPIO_AC97_SDATA_OUT;
        pins[4] = XLLP_GPIO_AC97_SYNC;
        pins[5] = XLLP_GPIO_KP_MKIN4;     // use this pin as AC97_SYSCLK 
        pins[6] = XLLP_GPIO_AC97_RESET_n;

        if (XLLP_TRUE == pAc97ctxt->useSecondaryCodec)
        {
			pins[0] = 7;
			pins[7] = XLLP_GPIO_KP_MKIN5;      // use this pin as AC97_SDATA_IN_1 
		}
		XllpGpioSetDirectionIn(pGPIO, pins);
        XllpGpioClearAlternateFn(pGPIO, pins);
	}
    	
    return (status);
}

/*******************************************************************************
 *
 * FUNCTION:         XllpAc97GetStatus 
 *
 * DESCRIPTION:      Reports the value of the specified status indicator, as
 *                   obtained from the ACUNIT's GSR register.  Clears the
 *                   status indicator, if possible, after reading it.
 *
 *                   Should not be used for status types that are currently in 
 *                   use as interrupt triggers.
 *
 * INPUT PARAMETERS: 
 *             P_XLLP_AC97_T pAc97Reg : a pointer to AC97 unit registers structure.
 *             XLLP_AC97_CODEC_SEL_T codecSel : Select which codec to be concerned.
 *
 * OUTPUT PARAMETERS:
 *             XLLP_AC97_STAT_T *pStat : a pointer to a status structure where lies result. 
 *
 * RETURNS:    None
 *
 * CALLS:            
 *
 * CALLED BY:        
 *
 * PROTOTYPE:   void  XllpAc97GetStatus(XLLP_AC97_STAT_T *pStat, P_XLLP_AC97_T pAc97Reg,
 *                                       XLLP_AC97_CODEC_SEL_T codecSel);
 *
 *******************************************************************************/

void  XllpAc97GetStatus(XLLP_AC97_STAT_T *pStat, P_XLLP_AC97_T pAc97Reg, XLLP_AC97_CODEC_SEL_T codecSel)
{
	if (XLLP_AC97_CODEC_PRIMARY == codecSel) 
	{
		pStat->codecReady = (pAc97Reg->GSR & XLLP_AC97_GSR_PCRDY_MSK) ? XLLP_TRUE : XLLP_FALSE;
    }
	else
	{
	    pStat->codecReady = (pAc97Reg->GSR & XLLP_AC97_GSR_SCRDY_MSK) ? XLLP_TRUE : XLLP_FALSE;
	}
} // Ac97CtrlGetStatus()


/*******************************************************************************
 *
 * FUNCTION:         XllpAc97Write
 *
 * DESCRIPTION:      Write a value to a specific mixer register in a specific 
 *                   AC'97 codec or modem, using the AC Link.
 *
 * INPUT PARAMETERS: 
 *        XLLP_UINT16_T offset : the offset address of the codec register to write.  
 *        XLLP_UINT16_T data : the data to be written. 
 *        P_XLLP_AC97_T pAc97Reg : a pointer to AC97 unit registers structure.  
 *        XLLP_UINT32_T maxRWTimeOutUs : timeout value in the writing process. 
 *        XLLP_AC97_CODEC_SEL_T codecSel : specify which codec device to write.
 *
 * RETURNS:          
 *       Success:    0 (XLLP_AC97_NO_ERROR)
 *       Failure:    XLLP_AC97_LINK_LOCK_FAIL: AC Link was not available within the 
 *                           timeout interval.
 *                   XLLP_AC97_CODEC_ACCESS_TIMEOUT: Writing was not finished within the 
 *                           timeout interval.
 *
 *
 * CALLS:            XllpAc97LinkLock, XllpOstDelayMicroSeconds          
 *
 * CALLED BY:        
 *
 * PROTOTYPE:        XLLP_AC97_ERROR_T  XllpAc97Write(XLLP_UINT16_T offset, 
 *								 XLLP_UINT16_T data, 
 *								 P_XLLP_AC97_T pAc97Reg,
 *								 P_XLLP_OST_T pOstRegs,
 *								 XLLP_UINT32_T maxRWTimeOutUs, 
 *								 XLLP_AC97_CODEC_SEL_T codecSel);
 *
 *******************************************************************************/

XLLP_AC97_ERROR_T  XllpAc97Write(XLLP_UINT16_T offset, 
								 XLLP_UINT16_T data, 
								 P_XLLP_AC97_T pAc97Reg, 
								 P_XLLP_OST_T pOstRegs,
								 XLLP_UINT32_T maxRWTimeOutUs, 
								 XLLP_AC97_CODEC_SEL_T codecSel) 
{
    XLLP_AC97_ERROR_T	status = XLLP_AC97_NO_ERROR;
    XLLP_BOOL_T			gotLink;
    XLLP_UINT32_T		timeRemaining;
    P_XLLP_VUINT32_T	pCodecReg;
    
    // Point to specified register within area mapped to target codec regs

    // Check for special case register 54h the GPIO status register

    if(offset == XLLP_AC97_CR_E_MDM_GPIO_PIN_STAT)
    {
#ifdef WM9712
        
        // This is a work around for the WM9712 GPIO status issue.
        // Note that the WM9712 can only be used as a primary
        // AC97 device.
        
        XLLP_UINT16_T offsetdata = data << 1;
        
        pCodecReg = &(pAc97Reg->CodecRegsPrimaryAud[0]);

        pCodecReg += offset / XLLP_AC97_CODEC_REGS_PER_WORD;

        // The data will be sent out on slots 1&2 to register 54h.
        *pCodecReg = (XLLP_VUINT32_T)data;

        pCodecReg = &(pAc97Reg->CodecRegsPrimaryMdm[0]);

        pCodecReg += offset / XLLP_AC97_CODEC_REGS_PER_WORD;

        // The data will be sent out on slot 12.
        *pCodecReg = (XLLP_VUINT32_T)offsetdata;
       
#else
        // Select the Primary or Secondary modem IO address space
	    if (XLLP_AC97_CODEC_PRIMARY == codecSel)
            pCodecReg = &(pAc97Reg->CodecRegsPrimaryMdm[0]);
        else
            pCodecReg = &(pAc97Reg->CodecRegsSecondaryMdm[0]);

        pCodecReg += offset / XLLP_AC97_CODEC_REGS_PER_WORD;

        // The data will be sent out on slot 12.
        *pCodecReg = data;
   
#endif
        goto done;
    }
    else
    {
        // Select the Primary or Secondary Audio IO address space
	    if (XLLP_AC97_CODEC_PRIMARY == codecSel)
            pCodecReg = &(pAc97Reg->CodecRegsPrimaryAud[0]);
        else
            pCodecReg = &(pAc97Reg->CodecRegsSecondaryAud[0]);

        pCodecReg += offset / XLLP_AC97_CODEC_REGS_PER_WORD;  
    }
    

	//Lock the ACLINK
    timeRemaining = XLLP_AC97_LOCK_TIMEOUT_DEF;
    do
    {
        gotLink = XllpAc97LinkLock(pAc97Reg);
        if (XLLP_FALSE == gotLink)	// 1 usec is a long time.  Skip delay if possible.
        {
			XllpOstDelayMicroSeconds(pOstRegs, 1);
        }
    }        // Wait while time remaining and ACLINK not available
    while (timeRemaining-- && (XLLP_FALSE == gotLink));

    if (XLLP_FALSE == gotLink)	// Didn't get the ACLINK
    {
        status = XLLP_AC97_LINK_LOCK_FAIL;
    }
    else	// We got the link. Perform the write operation and don't wait.
    {

        // First, clear old write status indication CDONE by writing a ONE to that bit.
        pAc97Reg->GSR = XLLP_AC97_GSR_CDONE_MSK;

        *pCodecReg = data;       // Now the write!

        // Wait until write cycle is complete. There should be a way
        //  to do this speculatively at the beginning of the procedure.
        //  Need to discover it. Too inefficient to always wait.

        timeRemaining = maxRWTimeOutUs;
        do
        {
			XllpOstDelayMicroSeconds(pOstRegs, 1);
        }     // Wait while time remaining and command I/O still incomplete.
        while ( (timeRemaining--) && !(pAc97Reg->GSR & XLLP_AC97_GSR_CDONE_MSK));
		if (!(pAc97Reg->GSR & XLLP_AC97_GSR_CDONE_MSK))
			status = XLLP_AC97_CODEC_ACCESS_TIMEOUT;
    }  // Got AC link

done:
    return(status);
} // Ac97CtrlCodecWrite()


/*******************************************************************************
 *
 * FUNCTION:         XllpAc97Read
 *
 * DESCRIPTION:      Read the value of a specific mixer register in a specified 
 *                   AC'97 codec or modem, using the AC Link.
 *
 * INPUT PARAMETERS: 
 *        XLLP_UINT16_T offset : the offset address of the codec register to read.  
 *        XLLP_UINT16_T pData : the pointer to where will hold the read result. 
 *        P_XLLP_AC97_T pAc97Reg : a pointer to AC97 unit registers structure.  
 *        XLLP_UINT32_T maxRWTimeOutUs : timeout value in the reading process. 
 *        XLLP_AC97_CODEC_SEL_T codecSel : specify which codec device to read.
 *
 * RETURNS:          
 *       Success:    0 (XLLP_AC97_NO_ERROR)
 *       Failure:    XLLP_AC97_LINK_LOCK_FAIL: AC Link was not available within the 
 *                           timeout interval.
 *                   XLLP_AC97_CODEC_ACCESS_TIMEOUT: Reading was not finished within the 
 *                           timeout interval.
 *
 *
 * CALLS:            XllpAc97LinkLock, XllpOstDelayMicroSeconds          
 *
 * CALLED BY:        
 *
 * PROTOTYPE:        XLLP_AC97_ERROR_T  XllpAc97Read(XLLP_UINT16_T offset, 
 *								XLLP_UINT16_T *pdata,
 *								P_XLLP_AC97_T pAc97Reg, 
 *								P_XLLP_OST_T pOstRegs,
 *								XLLP_UINT32_T maxRWTimeOutUs, 
 *								XLLP_AC97_CODEC_SEL_T codecSel);
 *
 *******************************************************************************/
 
XLLP_AC97_ERROR_T  XllpAc97Read(XLLP_UINT16_T offset, 
								XLLP_UINT16_T *pdata,
								P_XLLP_AC97_T pAc97Reg, 
								P_XLLP_OST_T pOstRegs,
								XLLP_UINT32_T maxRWTimeOutUs, 
								XLLP_AC97_CODEC_SEL_T codecSel)
{
    XLLP_AC97_ERROR_T	status = XLLP_AC97_NO_ERROR;
    XLLP_BOOL_T			gotLink;
	XLLP_UINT32_T		timeRemaining;
    P_XLLP_VUINT32_T	pCodecReg;

    // Point to specified register within area mapped to target codec regs
    // Check for special case register 54h the GPIO status register
#ifdef WM9712
    if(offset == XLLP_AC97_CR_E_MDM_GPIO_PIN_STAT)
    {
        // Select the Primary or Secondary modem IO address space
	    if (XLLP_AC97_CODEC_PRIMARY == codecSel)
            pCodecReg = &(pAc97Reg->CodecRegsPrimaryMdm[0]);
        else
            pCodecReg = &(pAc97Reg->CodecRegsSecondaryMdm[0]);

        pCodecReg += offset / XLLP_AC97_CODEC_REGS_PER_WORD;

        // The data is received on Slot 12 and stored by the
        // ACUNIT so we can read back straight away.
        *pdata = (XLLP_UINT16_T)(*pCodecReg);
            
        goto done;
    }
    else
    {
        // Select the Primary or Secondary Audio IO address space
	    if (XLLP_AC97_CODEC_PRIMARY == codecSel)
            pCodecReg = &(pAc97Reg->CodecRegsPrimaryAud[0]);
        else
            pCodecReg = &(pAc97Reg->CodecRegsSecondaryAud[0]);

        pCodecReg += offset / XLLP_AC97_CODEC_REGS_PER_WORD;  
    }
#else
    // Select the Primary or Secondary Audio IO address space
	if (XLLP_AC97_CODEC_PRIMARY == codecSel)
        pCodecReg = &(pAc97Reg->CodecRegsPrimaryAud[0]);
    else
        pCodecReg = &(pAc97Reg->CodecRegsSecondaryAud[0]);
    pCodecReg += offset / XLLP_AC97_CODEC_REGS_PER_WORD;  
#endif
	//Lock the ACLINK
    timeRemaining = XLLP_AC97_LOCK_TIMEOUT_DEF;
    do
    {
        gotLink = XllpAc97LinkLock(pAc97Reg);
        if (XLLP_FALSE == gotLink)	// 1 usec is a long time.  Skip delay if possible.
        {
			XllpOstDelayMicroSeconds(pOstRegs, 1);
        }
    }        // Wait while time remaining and ACLINK not available
    while (timeRemaining-- && (XLLP_FALSE == gotLink));

    if (XLLP_FALSE == gotLink)	// Didn't get the ACLINK
    {
        status = XLLP_AC97_LINK_LOCK_FAIL;
    }
    else	// We got the link. Perform the write operation and don't wait.
    {
         // First, clear old read status indications.
        pAc97Reg->GSR = XLLP_AC97_GSR_SDONE_MSK | XLLP_AC97_GSR_RCS_ERR_MSK;

        *pdata = (XLLP_UINT16_T)(*pCodecReg); // This is THE DUMMY READ.

         // Wait for read I/O with codec to complete before doing real read.
        timeRemaining = maxRWTimeOutUs;
        do
        {
			XllpOstDelayMicroSeconds(pOstRegs, 1);
        }   // Wait while time remaining and read I/O still incomplete
        while( (timeRemaining--) && (!(pAc97Reg->GSR & XLLP_AC97_GSR_SDONE_MSK)) );

        if ((pAc97Reg->GSR & XLLP_AC97_GSR_SDONE_MSK) && (!(pAc97Reg->GSR & XLLP_AC97_GSR_RCS_ERR_MSK)) )
        {
             // succeed in reading. clear status bits first.
             pAc97Reg->GSR = XLLP_AC97_GSR_SDONE_MSK | XLLP_AC97_GSR_RCS_ERR_MSK;
            *pdata = (XLLP_UINT16_T)(*pCodecReg);	// THE REAL READ.
     		timeRemaining = maxRWTimeOutUs;
    	    do
	        {
				XllpOstDelayMicroSeconds(pOstRegs, 1);
  		    }   // Wait while time remaining and read I/O still incomplete
    	    while( (timeRemaining--) && (!(pAc97Reg->GSR & XLLP_AC97_GSR_SDONE_MSK)) );

        }
        else	// failed
        {
            status = XLLP_AC97_CODEC_ACCESS_TIMEOUT;
            pAc97Reg->CAR = XLLP_AC97_CAR_CAIP_CLEAR;

        } // else  (OK to do real read)

    } // else  (We got the link.  Perform the read operations.)

    return (status);
} // XllpAc97Read ()


/*******************************************************************************
 *
 * FUNCTION:         XllpAc97ColdReset
 *
 * DESCRIPTION:      After a cold reset of the processor's AC'97 Controller,
 *                   check the AC Link and all AC'97 codecs and modems attached to 
 *                   the controller's AC'97 cold reset pin.
 *
 * INPUT PARAMETERS: 
 *       pAc97ctxt   a pointer to a XLLP_AC97_CONTEXT_T struct, which contains 
 *						necessary information.
 *
 * RETURNS:
 *       Success:    0 (XLLP_AC97_NO_ERROR)
 *       Failure:    XLLP_AC97_CODEC_NOT_READY:   A timeout occurred on one of the codecs.
 *                        In systems with multiple codecs, the calling function 
 *                        could determine which device(s) is/are not ready by 
 *                        performing XllpAc97GetStatus() on 
 *                        AC97CTRL_STAT_PCRDY and AC97CTRL_STAT_SCRDY.
 *
 * GLOBAL EFFECTS:   All codecs initialized to default settings.  Controller
 *                   set to default state and is operational.
 *
 * ASSUMPTIONS:      Not used in an interrupt service routine.
 *
 * CALLS:            XllpOstDelayMicroSeconds     
 *
 * CALLED BY:        XllpAc97Init 
 *
 * PROTOTYPE:        XLLP_AC97_ERROR_T  XllpAc97ColdReset(P_XLLP_AC97_CONTEXT_T pAc97ctxt);
 *
 *******************************************************************************/
XLLP_AC97_ERROR_T  XllpAc97ColdReset(P_XLLP_AC97_CONTEXT_T pAc97ctxt)  
{ 
    XLLP_AC97_ERROR_T	status = XLLP_AC97_NO_ERROR; 
    P_XLLP_AC97_T		pAC97 = pAc97ctxt->pAc97Reg;
    P_XLLP_OST_T 		pOstRegs = pAc97ctxt->pOstRegs;
    XLLP_BOOL_T			priCodecReady, secCodecReady; 
    XLLP_UINT32_T		timeRemaining; 

    pAC97->GCR = 0;    

    // Hold reset active for a minimum time
	XllpOstDelayMicroSeconds(pOstRegs, XLLP_AC97_COLD_HOLDTIME);

    // Deactivate cold reset condition
	pAC97->GCR |= XLLP_AC97_GCR_COLD_RESET_MSK; 
    
    // Set nReset high. This is a workaround for some strange behavior of nReset pin.
    {
	    XLLP_UINT32_T pins[6];
    	pins[0] = 1;
    	pins[1] = 113;
    	XllpGpioSetOutputState1(pAc97ctxt->pGpioReg, pins);
    }
    	
    // And wait with timeout for all codecs to respond.

    priCodecReady = XLLP_FALSE;
	if (XLLP_FALSE == pAc97ctxt->useSecondaryCodec)
	{
	    secCodecReady = XLLP_TRUE;
	}
	else
	{
	    secCodecReady = XLLP_FALSE;
	}
    timeRemaining = pAc97ctxt->maxSetupTimeOutUs;
    do
    {
		XllpOstDelayMicroSeconds(pOstRegs, 1);
        if (pAC97->GSR & XLLP_AC97_GSR_PCRDY_MSK)
            priCodecReady = XLLP_TRUE;
        if (pAC97->GSR & XLLP_AC97_GSR_SCRDY_MSK)
            secCodecReady = XLLP_TRUE;
    }
    while (timeRemaining-- && ((priCodecReady == XLLP_FALSE) || (secCodecReady == XLLP_FALSE)));

    // Timeout status if some of the devices weren't ready.
    if ((priCodecReady == XLLP_FALSE) || (secCodecReady == XLLP_FALSE))
    {
        status = XLLP_AC97_CODEC_NOT_READY;
    }

    return (status);
} // XllpAc97ColdReset ()

/*******************************************************************************
 *
 * FUNCTION:         XllpAc97ShutdownAcLink
 *
 * DESCRIPTION:      Try to shutdown the AC Link by setting the corresponding bit
 *                      in GCR of AC97 unit.
 *
 * INPUT PARAMETERS: None
 *        P_XLLP_AC97_T pAc97Reg : a pointer to AC97 unit registers structure.  
 *
 * RETURNS:          
 *       Success:    0 (XLLP_AC97_NO_ERROR)
 *       Failure:    XLLP_AC97_LINK_SHUTDOWN_FAIL: A timeout occurred in shutdown process.
 *
 * CALLS:            XllpOstDelayMicroSeconds       
 *
 * CALLED BY:        XllpAc97DeInit
 *
 * PROTOTYPE:        XLLP_AC97_ERROR_T  XllpAc97ShutdownAclink(P_XLLP_AC97_T pAc97Reg,
 *															   P_XLLP_OST_T pOstRegs);
 *
 *******************************************************************************/

XLLP_AC97_ERROR_T  XllpAc97ShutdownAclink(P_XLLP_AC97_T pAc97Reg, P_XLLP_OST_T pOstRegs)
{
    XLLP_AC97_ERROR_T status = XLLP_AC97_NO_ERROR; 
    XLLP_UINT32_T timeRemaining = XLLP_AC97_LINKOFF_TIMEOUT_DEF; 

    pAc97Reg->GCR |= XLLP_AC97_GCR_LINK_OFF_MSK;
	
	while (!(pAc97Reg->GSR & XLLP_AC97_GSR_ACOFFD_MSK))
	{
		timeRemaining --;
		if (0 == timeRemaining)
		{
    		status = XLLP_AC97_LINK_SHUTDOWN_FAIL;
    		break;
		}
		XllpOstDelayMicroSeconds(pOstRegs, 1);
    }
	
    return(status);
}

/*******************************************************************************
 *
 * FUNCTION:         XllpAc97LinkLock
 *
 * DESCRIPTION:      Try to lock the AC Link for command/status accesses to a
 *                   codec.
 *
 * INPUT PARAMETERS: 
 *        P_XLLP_AC97_T pAc97Reg : a pointer to AC97 unit registers structure.  
 *
 * RETURNS:          XLLP_TRUE if the attempt was successful; XLLP_FALSE if not.
 *
 * GLOBAL EFFECTS:   If true, the hardware indicator will show that the AC Link
 *                   is locked until either a codec command or status I/O 
 *                   operation has completed, or Ac97CtrlReleaseAcLink 
 *                   is called.
 *
 * CALLS:            
 *
 * CALLED BY:		 XllpAc97Write, XllpAc97Read        
 *
 * PROTOTYPE:        XLLP_BOOL_T XllpAc97LinkLock(P_XLLP_AC97_T pAc97Reg);
 *
 *******************************************************************************/
XLLP_BOOL_T XllpAc97LinkLock(P_XLLP_AC97_T pAc97Reg)
{
    XLLP_BOOL_T		status = XLLP_TRUE;
    XLLP_VUINT32_T	carTmp;

    carTmp = pAc97Reg->CAR;
    if (carTmp & XLLP_AC97_CAR_CAIP_MSK)	// "1" in CAIP bit means lock failed.
    {
        status = XLLP_FALSE;
    }
    return (status);
} // XllpAc97LinkLock()



