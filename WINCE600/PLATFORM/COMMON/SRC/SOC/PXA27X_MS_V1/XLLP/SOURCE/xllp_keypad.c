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
/* Copyright 1999-2001 Intel Corp.  */

#include    "xllp_keypad.h"
#include    "xllp_gpio.h"
//#include  "windows.h"
static XLLP_UINT32_T XllpKpdGpioDirOutList[]={7,96,103,104,105,106,107,108};
static XLLP_UINT32_T XllpKpdGpioDirInList[]={9,93,94,95,97,98,99,100,101,102};
static XLLP_UINT32_T XllpKpdGpioAltFnPinList[]={16,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108};
static XLLP_UINT32_T XllpKpdGpioAltFnValList[]={16,1,1,3,3,3,3,3,1,1,1,2,2,2,2,2,2};
static XLLP_UINT32_T XllpKpdGpioDirNonScrollWheelInList[]={7,95,97,98,99,100,101,102};
XLLP_UINT32_T   KP_Status;
//#define NMD

//---------------------------------------------------------------------------------------------------------------
// Function: ReadScanCodeAutomatically
// Purpose:  This functions reads the scan code from the KeyPad controller triggered by setting of the ASACT bit
//           in the KeyPad Control register. If there is a valid key detected then it returns the scan code.
// Returns:  success/failure.
//---------------------------------------------------------------------------------------------------------------

XLLP_BOOL_T ReadScanCodeAutomatically(XLLP_KEYPAD_REGS *v_pKeyPadRegs,XLLP_UINT8_T *key)
{
    XLLP_BOOL_T     retval=XLLP_FALSE;
    XLLP_UINT32_T   C,R,RthBit,c0,c1,c2,c3,c4,c5,c6,c7;
    XLLP_UINT32_T   numOfKeysPressed=0;

    if(KP_Status & MATRIX_INTR_BIT)
    {
        numOfKeysPressed = ((v_pKeyPadRegs->kpAutomaticScanReg & MULTI_KEYS_PRESS) >> 26);
//   RETAILMSG(1,(TEXT("XLLP:ReadScanCodeAutomatically numOfKeysPressed %x>\r\n"),numOfKeysPressed));
// checks to see if it was a "Major" key that was pressed
        if(numOfKeysPressed == 1)
        {
            C =  (v_pKeyPadRegs->kpAutomaticScanReg & COL_SELECTED_MASK);
            R =  (v_pKeyPadRegs->kpAutomaticScanReg & ROW_SELECTED_MASK);
            *key = (unsigned char) (C | R);
        }
        else
// if it was a "Minor" key, then more than one key was activated and that is how
//  you can determine which register to read from.
        if(numOfKeysPressed > 1)
        {
            c0 = v_pKeyPadRegs->kpAutoScanMultiKeyPress0 & 0xFF;
            c1 = ((v_pKeyPadRegs->kpAutoScanMultiKeyPress0 >> 16) & 0xFF);
            c2 = v_pKeyPadRegs->kpAutoScanMultiKeyPress1 & 0xFF;
            c3 = ((v_pKeyPadRegs->kpAutoScanMultiKeyPress1 >> 16) & 0xFF);
            c4 = v_pKeyPadRegs->kpAutoScanMultiKeyPress2 & 0xFF;
            c5 = ((v_pKeyPadRegs->kpAutoScanMultiKeyPress2 >> 16) & 0xFF);
            c6 = v_pKeyPadRegs->kpAutoScanMultiKeyPress3 & 0xFF;
            c7 = ((v_pKeyPadRegs->kpAutoScanMultiKeyPress3 >> 16) & 0xFF);

// these keys are the "minor keys", the ones that needs top right and bottom left of the
// cooresponding 4 keys surrounding them to trigger the correct key. Doing a binary search
// there are 5 keys, the middle key reads 0x8, the first key reads 0x2 and the last reads 0x20.
// this needs to be done for each row.  This will be encorporated into a routine for the next
// upgrade of keypad.
            if(c0!=0)
            {
                C = 0x7;
                if(c0 == 0x8)
                {
                   RthBit = 0x2;
                }
                else
                if(c0 > 0x8)
                {
                    if (c0 < 0x20)
                       RthBit = 0x3;
                    else
                       RthBit = 0x4;
                }else       
                {
                    if (c0 > 0x2)
                       RthBit = 0x1;
                    else
                       RthBit = 0x0;
                }       
            }else       
            if(c1!=0)
            {
                C = 0x8;
                if(c1 == 0x8)
                {
                   RthBit = 0x2;
                }
                else
                if(c1 > 0x8)
                {
                    if (c1 < 0x20)
                       RthBit = 0x3;
                    else
                       RthBit = 0x4;
                }else       
                {
                    if (c1 > 0x2)
                       RthBit = 0x1;
                    else
                       RthBit = 0x0;
                }       
            }else       
            if(c2!=0)
            {
                C = 0x9;
                if(c2 == 0x8)
                {
                   RthBit = 0x2;
                }
                else
                if(c2 > 0x8)
                {
                    if (c2 < 0x20)
                       RthBit = 0x3;
                    else
                       RthBit = 0x4;
                }else       
                {
                    if (c2 > 0x2)
                       RthBit = 0x1;
                    else
                       RthBit = 0x0;
                }       
            }else       
            if(c3!=0)
            {
                C = 0xa;
                if(c3 == 0x8)
                {
                   RthBit = 0x2;
                }
                else
                if(c3 > 0x8)
                {
                    if (c3 < 0x20)
                       RthBit = 0x3;
                    else
                       RthBit = 0x4;
                }else       
                {
                    if (c3 > 0x2)
                       RthBit = 0x1;
                    else
                       RthBit = 0x0;
                }       
            }else       
            if(c4!=0)
            {
                C = 0xb;
                if(c4 == 0x8)
                {
                   RthBit = 0x2;
                }
                else
                if(c4 > 0x8)
                {
                    if (c4 < 0x20)
                       RthBit = 0x3;
                    else
                       RthBit = 0x4;
                }else       
                {
                    if (c4 > 0x2)
                       RthBit = 0x1;
                    else
                       RthBit = 0x0;
                }       
            }       
            *key = (unsigned char) ((C<<4) | RthBit);
        }
        else
            *key = NO_KEY;

//       RETAILMSG(1,(TEXT("R is %x C is %x key is %x\r\n"), RthBit,C,*key));

        retval = XLLP_TRUE;
    }
//    RETAILMSG(1,(TEXT("ReadScanCodeAutomatically<\r\n")));
    return(retval);
}

//---------------------------------------------------------------------------------------------------------------
// Function: XllpKpKeypressIsInProgress
// Purpose:  This function reports nonzero if the keypad is currently active during a keypress.
//           (Keypress active as indicated by non-rotary DKIN levels high, any MKIN levels high
//              or any MKOUT levels low (indicates scan is active))
//
// Note:     It cannot detect the debouncing of a complete key up event.  Therefore, full
//           protection from performing conflicting actions during keypad activity must include
//           support from higher level software.  (One example is avoiding entering Standby while
//           a key release is being debounced because the key release detection could be lost.)
//
// Returns:  nonzero ("true") / zero ("false")
// Limitations:  Platform-specific because it requires complete knowledge of keypad configuration
//---------------------------------------------------------------------------------------------------------------
XLLP_UINT32_T XllpKpKeypressIsInProgress (XLLP_GPIO_T *v_pGPIOReg)
{
    // Note that GPIOs used here only include MKOUTs, MKINs and non-rotary DKINs.

    static XLLP_BOOL_T xllpKpKIP_Initialized = XLLP_FALSE;
    static XLLP_UINT32_T gplr0_InPinMask, gplr0_OutPinMask, gplr0_AllPinsMask,
                         gplr1_InPinMask, gplr1_OutPinMask, gplr1_AllPinsMask,
                         gplr2_InPinMask, gplr2_OutPinMask, gplr2_AllPinsMask,
                         gplr3_InPinMask, gplr3_OutPinMask, gplr3_AllPinsMask;

    XLLP_UINT32_T i;
    XLLP_UINT32_T gpioRegTmp;
    XLLP_UINT32_T activity  = 0;

    // Set up masks only once.  Do this in code rather than precalculation to expose
    //  the algorithm and make it easily re-usable.  Perform full init because fairly
    //  fast and only done once.

    if (!xllpKpKIP_Initialized)
    {
        gplr0_InPinMask = gplr0_OutPinMask = gplr0_AllPinsMask =
        gplr1_InPinMask = gplr1_OutPinMask = gplr1_AllPinsMask =
        gplr2_InPinMask = gplr2_OutPinMask = gplr2_AllPinsMask =
        gplr3_InPinMask = gplr3_OutPinMask = gplr3_AllPinsMask = 0;

        for (i=1 ;i<(XllpKpdGpioDirNonScrollWheelInList[0]+1) ;i++ )
        {
            switch (XllpKpdGpioDirNonScrollWheelInList[i] / 32) // 32 pins per level register
            {
                case 0:
                    gplr0_InPinMask |= (1u << (XllpKpdGpioDirNonScrollWheelInList[i] &31));
                    break;
                case 1:
                    gplr1_InPinMask |= (1u << (XllpKpdGpioDirNonScrollWheelInList[i] &31));
                    break;
                case 2:
                    gplr2_InPinMask |= (1u << (XllpKpdGpioDirNonScrollWheelInList[i] &31));
                    break;
                case 3:
                    gplr3_InPinMask |= (1u << (XllpKpdGpioDirNonScrollWheelInList[i] &31));
                    break;
            }
        }  // Input pin masks

        for (i=1 ;i<XllpKpdGpioDirOutList[0]+1 ;i++ )
        {
            switch (XllpKpdGpioDirOutList[i] / 32) // 32 pins per level register
            {
                case 0:
                    gplr0_OutPinMask |= (1u << (XllpKpdGpioDirOutList[i] &31));
                    break;
                case 1:
                    gplr1_OutPinMask |= (1u << (XllpKpdGpioDirOutList[i] &31));
                    break;
                case 2:
                    gplr2_OutPinMask |= (1u << (XllpKpdGpioDirOutList[i] &31));
                    break;
                case 3:
                    gplr3_OutPinMask |= (1u << (XllpKpdGpioDirOutList[i] &31));
                    break;
            }
        }  // Output pin masks

        gplr0_AllPinsMask = gplr0_InPinMask | gplr0_OutPinMask;
        gplr1_AllPinsMask = gplr1_InPinMask | gplr1_OutPinMask;
        gplr2_AllPinsMask = gplr2_InPinMask | gplr2_OutPinMask;
        gplr3_AllPinsMask = gplr3_InPinMask | gplr3_OutPinMask;

        xllpKpKIP_Initialized = XLLP_TRUE;
    }

    // Main calculation
    // Platform-specific optimization: For Mainstone, no keypad pins in GPLR[1:0].

    gpioRegTmp = v_pGPIOReg->GPLR2 ;
    activity =  (gpioRegTmp ^ gplr2_OutPinMask) & gplr2_AllPinsMask;

    gpioRegTmp = v_pGPIOReg->GPLR3 ;
    activity |= (gpioRegTmp ^ gplr3_OutPinMask) & gplr3_AllPinsMask;

    return(activity);

} // XllpKpKeypressIsInProgress()

//---------------------------------------------------------------------------------------------------------------
// Function: ReadDirectKeys
// Purpose:  This function looks for any Thumbwheel movement or button press and returns a scan code.
// Returns:  success/failure.
//---------------------------------------------------------------------------------------------------------------

XLLP_BOOL_T ReadDirectKeys(XLLP_KEYPAD_REGS *v_pKeyPadRegs,XLLP_UINT8_T *key)
{
    XLLP_UINT32_T CurrCount,SaveKpRotaryEncoderCountReg;
    static XLLP_UINT32_T PrevCount=START_VALUE;
    XLLP_BOOL_T retval;
    if(KP_Status & DIRECT_INTR_BIT)
    {
        SaveKpRotaryEncoderCountReg = v_pKeyPadRegs->kpRotaryEncoderCountReg;
        CurrCount = SaveKpRotaryEncoderCountReg & COUNT_MASK;
        if(SaveKpRotaryEncoderCountReg & OVERFLOW_ROTARY_ENC_0)
        {
            v_pKeyPadRegs->kpRotaryEncoderCountReg = START_VALUE;
            PrevCount   = START_VALUE;
            *key    = SCAN_CODE_SCROLL_UP;                  // Scroll Up
        }
        else if(SaveKpRotaryEncoderCountReg & UNDERFLOW_ROTARY_ENC_0)
        {
            v_pKeyPadRegs->kpRotaryEncoderCountReg = START_VALUE;
            PrevCount   = START_VALUE;
            *key    = SCAN_CODE_SCROLL_DOWN;                // Scroll Down
        }
        else if(CurrCount > PrevCount)
        {
            *key    = SCAN_CODE_SCROLL_UP;
            PrevCount   = CurrCount;                        // Scroll Up
        }
        else if(CurrCount < PrevCount)
        {
            *key    = SCAN_CODE_SCROLL_DOWN;
            PrevCount   = CurrCount;                        // Scroll Down
        }
        else if(v_pKeyPadRegs->kpDirectKeyReg & DIRECT_KEY_IN_2)
        {
            *key    = SCAN_CODE_ACTION;                     // Action Key       
        }

        retval = XLLP_TRUE;                                 // Signal availability
    }
    else
    {
        retval = XLLP_FALSE;
    }

    return(retval);
}

//---------------------------------------------------------------------------------------------------------------
// Function: XllpReadScanCode
// Purpose:
// Returns:
//---------------------------------------------------------------------------------------------------------------

XLLP_BOOL_T XllpReadScanCode(XLLP_KEYPAD_REGS *v_pKeyPadRegs,XLLP_UINT8_T *pui8Data)
{
    // Initialise to NO Key scan code, same as key UP
    XLLP_UINT8_T key = NO_KEY;

    if(!ReadDirectKeys(v_pKeyPadRegs,&key))
        ReadScanCodeAutomatically(v_pKeyPadRegs,&key);

    // Assign the Key Here
    *pui8Data = key;

    if(*pui8Data == NO_KEY)
        return(XLLP_FALSE);
    else
        return(XLLP_TRUE);      // Enjoy
}

//---------------------------------------------------------------------------------------------------------------
// Function: XllpSetUpKeyPadInterrupts
// Purpose:  This function will Enable/Disable the Interrupts.
// Returns:  success/failure.
//---------------------------------------------------------------------------------------------------------------
XLLP_BOOL_T XllpSetUpKeyPadInterrupts(XLLP_KEYPAD_REGS *v_pKeyPadRegs,XLLP_BOOL_T fEnDis)
{
    KP_Status = v_pKeyPadRegs->kpControlReg;

    if(fEnDis)
    {
        EN_DIRECT_KEYS_INTR();
        EN_MAT_KEYS_INTR();
    }
    else
    {
        DISABLE_DIRECT_KEYS_INTR();
        DISABLE_MAT_KEYS_INTR();
    }
    return(XLLP_TRUE);
}

//-----------------------------------------------------------------------------------------
// Function: XllpKeyPadConfigure
// Purpose:  This function configures the KeyPad Controller on Bulverde-Mainstone with the
//           appropriate settings.
// Returns:  success/failure.
//------------------------------------------------------------------------------------------
//  PIN#      GPIO PIN NAME           DIRECTION         ALTERNATE FUNCTION
//
//   95         KP_DKIN[2]              Input                   3
//   97         KP_DKIN[4]              Input                   3
//   98         KP_DKIN[5]              Input                   3
//   99         KP_DKIN[6]              Input                   3
//  100         KP_MKIN[0]              Input                   1
//  101         KP_MKIN[1]              Input                   1
//  102         KP_MKIN[2]              Input                   1
//  
//   96         KP_DKIN[3]              Output                  3
//  103         KP_MKOUT[0]             Output                  2
//  104         KP_MKOUT[1]             Output                  2
//  105         KP_MKOUT[2]             Output                  2
//  106         KP_MKOUT[3]             Output                  2
//  107         KP_MKOUT[4]             Output                  2
//  108         KP_MKOUT[5]             Output                  2
//
//  93          KP_DKIN[0]              Input                   1
//  94          KP_DKIN[1]              Input                   1
//--------------------------------------------------------------------------------------------

XLLP_BOOL_T XllpKeyPadConfigure(XLLP_KEYPAD_REGS *v_pKeyPadRegs,XLLP_GPIO_T *v_pGPIOReg)
{
    XLLP_BOOL_T retval=XLLP_FALSE;
    XLLP_UINT32_T GpioDirOutList[]={7,96,103,104,105,106,107,108};
    XLLP_UINT32_T GpioDirInList[]={9,93,94,95,97,98,99,100,101,102};
    XLLP_UINT32_T GpioAltFnPinList[]={16,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108};
    XLLP_UINT32_T GpioAltFnValList[]={16,1,1,3,3,3,3,3,1,1,1,2,2,2,2,2,2};
    if(v_pGPIOReg != 0)
    {
        XllpGpioSetDirectionIn(v_pGPIOReg, GpioDirInList);
        XllpGpioSetDirectionOut(v_pGPIOReg, GpioDirOutList);
        XllpGpioSetOutput0(v_pGPIOReg, GpioDirOutList);
        XllpGpioSetAlternateFn(v_pGPIOReg, GpioAltFnPinList, GpioAltFnValList);
    }


    if(v_pKeyPadRegs != 0)
    {
        // Init the control regs for direct keys
        v_pKeyPadRegs->kpControlReg = (MATRIX_KP_NUMBER_OF_ROWS |  MATRIX_KP_NUMBER_OF_COLUMNS |
                                        MAT_SCAN_LINE0 | MAT_SCAN_LINE1 | MAT_SCAN_LINE2 |
                                        MAT_SCAN_LINE3 | MAT_SCAN_LINE4 | MAT_SCAN_LINE5 |
                                        MAT_SCAN_LINE6 | MAT_SCAN_LINE7 | IGNORE_MULTIPLE_KEY_PRESS |
                                        AUTO_SCAN_ON_ACTIVITY | MATRIX_INTR_ENABLE |
                                        MATRIX_KP_ENABLE | ROTARY_ENCODER_ZERO_DEB |
                                        DIRECT_KP_INTR_ENABLE | DIRECT_KEY_NUMS |
                                        DIRECT_KP_ENABLE | ROTARY_ENCODER_0_ENABLE) ;    //NMD

        v_pKeyPadRegs->kpRotaryEncoderCountReg = START_VALUE;

        retval = XLLP_TRUE;     
    }
    return(retval);
}
