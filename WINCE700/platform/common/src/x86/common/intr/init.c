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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  init.c

Abstract:  
   This file implements the interrupt initialization routines
 
Functions:


Notes: 

--*/

#include <windows.h>
#include <pc.h>
#include <oal.h>
#include <nkintr.h>
#include <oalintr.h>


BOOL OALIntrInit (void)
{
    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALInterruptInit\r\n"));

    // Initialize interrupt mapping
    OALIntrMapInit();


    // The following block of code sets up the system interrupt 
    // mapping table. The goal is to follow the interrrupt lay
    // out of a legacy PC for simplicity. This may be modified
    // according to hardware requirements.
    //
    // IRQ0 is timer0, the scheduler tick

    OALIntrStaticTranslate(SYSINTR_RESCHED, INTR_TIMER0);
    OALIntrStaticTranslate(SYSINTR_KEYBOARD, 1);
    
    // IRQ2 is the cascade interrupt for the second PIC

    // Serial Port Info
    //
    // The legacy COM port layout defines IRQ4 being shared by 
    // COM ports 1 and 3 and IRQ3 is shared COM ports 2 and 4. 
    // If the legacy IRQ layout is followed, only 1 COM port 
    // per IRQ should be enabled.
    //
    // COM1 - 0x3F8-0x3FF, IRQ4
    // COM2 - 0x2F8-0x3FF, IRQ3
    // COM3 - 0x3E8-0x3EF, IRQ4
    // COM4 - 0x2E8-0x2EF, IRQ3
    //
    // IRQ3 - COM2 or COM4
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 3, 3);

    // IRQ4 - COM1 or COM3
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 4, 4);

    // IRQ5 is legacy LPT2 - this IRQ is potentially available.
    //
    // Currently this interrupt is being used for Audio support
    // on the emulator.
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 5, 5);

    // IRQ6 is normally the floppy controller.
    
    // IRQ7 is normally LPT1
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 7, 7);

    // IRQ8 is the real time clock
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM,  8);
    
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 9,  9);
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 10, 10);
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 11, 11);
    OALIntrStaticTranslate(SYSINTR_TOUCH, 12);
    
    // IRQ13 is normally the coprocessor
    // IRQ14 is normally the hard disk controller
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 14, 14);
    OALIntrStaticTranslate(SYSINTR_FIRMWARE + 15, 15);

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALInterruptInit(rc = %d)\r\n", TRUE));

    return TRUE;
}
