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
/* Copyright 1999 Intel Corp.  */
#include    "xllp_defs.h"
#include    "xllp_gpio.h"

typedef struct
{
    XLLP_VUINT32_T kpControlReg;
    XLLP_VUINT32_T  rsvd1;
    XLLP_VUINT32_T kpDirectKeyReg;
    XLLP_VUINT32_T  rsvd2;
    XLLP_VUINT32_T kpRotaryEncoderCountReg;
    XLLP_VUINT32_T  rsvd3;
    XLLP_VUINT32_T kpMatrixKeyReg;
    XLLP_VUINT32_T  rsvd4;
    XLLP_VUINT32_T kpAutomaticScanReg;
    XLLP_VUINT32_T  rsvd5;
    XLLP_VUINT32_T kpAutoScanMultiKeyPress0;
    XLLP_VUINT32_T  rsvd6;
    XLLP_VUINT32_T kpAutoScanMultiKeyPress1;
    XLLP_VUINT32_T  rsvd7;
    XLLP_VUINT32_T kpAutoScanMultiKeyPress2;
    XLLP_VUINT32_T  rsvd8;
    XLLP_VUINT32_T kpAutoScanMultiKeyPress3;
    XLLP_VUINT32_T  rsvd9;
    XLLP_VUINT32_T kpKeyDebounceInterval;

} XLLP_KEYPAD_REGS;

typedef struct
{
    XLLP_UINT8_T    rowcol;
} KEY;



// maximum number of keys that can be detected simultaneously
#define     MAX_KEYS                            1

// Bit Definitions for KeyPad Interface Control Register

#define     DIRECT_KP_INTR_ENABLE               (0x1    <<  0)
#define     DIRECT_KP_ENABLE                    (0x1    <<  1)
#define     ROTARY_ENCODER_0_ENABLE             (0x1    <<  2)
#define     ROTARY_ENCODER_1_ENABLE             (0x1    <<  3)
#define     ROTARY_ENCODER_ZERO_DEB             (0x1    <<  4)
#define     DIRECT_INTR_BIT                     (0x1    <<  5)
#define     DIRECT_DEBOUNCE_BIT                 (0x1    <<  9)
#define     MATRIX_INTR_ENABLE                  (0x1    <<  11)
#define     MATRIX_KP_ENABLE                    (0x1    <<  12)
#define     IGNORE_MULTIPLE_KEY_PRESS           (0x1    <<  21)
#define     MATRIX_KP_NUMBER_OF_COLUMNS         (0x7    <<  26)
#define     MATRIX_KP_NUMBER_OF_ROWS            (0x6    <<  23)
#define     MATRIX_INTR_BIT                     (0x1    <<  22)
#define     AUTO_SCAN_ON_ACTIVITY               (0x1    <<  29)
#define     AUTO_SCAN_BIT                       (0x1    <<  30)
#define     MAT_SCAN_LINE0                      (0x1    <<  13)
#define     MAT_SCAN_LINE1                      (0x1    <<  14)
#define     MAT_SCAN_LINE2                      (0x1    <<  15)
#define     MAT_SCAN_LINE3                      (0x1    <<  16)
#define     MAT_SCAN_LINE4                      (0x1    <<  17)
#define     MAT_SCAN_LINE5                      (0x1    <<  18)
#define     MAT_SCAN_LINE6                      (0x1    <<  19)
#define     MAT_SCAN_LINE7                      (0x1    <<  20)

// Bit Definitions for KeyPad Interface Direct Key Register

#define     DIRECT_KEY_PRESSED                  (0x1    <<  31)
#define     DIRECT_KEY_IN_7                     (0x1    <<  7)
#define     DIRECT_KEY_IN_6                     (0x1    <<  6)
#define     DIRECT_KEY_IN_5                     (0x1    <<  5)
#define     DIRECT_KEY_IN_4                     (0x1    <<  4)
#define     DIRECT_KEY_IN_3                     (0x1    <<  3)
#define     DIRECT_KEY_IN_2                     (0x1    <<  2)
#define     ROTARY_ENC_0_SENSOR_B               (0x1    <<  1)
#define     ROTARY_ENC_0_SENSOR_A               (0x1    <<  0)
#define     DIRECT_KEY_NUMS                     (0x3    <<  6)   // 1 rotary + 1 Direct Key i.e 2 keys in all

// Bit Definitions for KeyPad Encoder Count Register

#define     UNDERFLOW_ROTARY_ENC_0              (0x1    <<  14)
#define     OVERFLOW_ROTARY_ENC_0               (0x1    <<  15)

// Bit Definitions for KeyPad Interface Matrix Key Register

#define     MATRIX_KEY_PRESSED                  (0x1    <<  31)
#define     MATRIX_ROW_MASK                      0xFF

// Bit Definitions for KeyPad Interface Automatic Scan Register

#define     SCAN_ON_BIT                         (0x1    <<  31)
#define     ROW_SELECTED_MASK                    0xF0
#define     COL_SELECTED_MASK                    0x0F
#define     MULTI_KEYS_PRESS                     0x7C000000
#define     SINGLE_KEY_PRESS                    (0x1    <<  26)

// Bit Definitions for KeyPad Interface Automatic Scan Multiple Key Press Register 0,1,2,3

#define     MATRIX_KP_COL_EVEN_MASK             0x0000FF
#define     MATRIX_KP_COL_ODD_MASK              0xFF0000                    

// Bit Definitions for KeyPad Interface Key Debounce Interval Register

#define     MAX_KEY_DEBOUNCE_INTERVAL           0x0000FF

// Debeounce Interval for the keyPad on MainStone is < 12 millisecs

#define     MAINSTONE_KP_DEBOUNCE_INTERVAL      0x00000C

// KeyPad controller supports 3 modes of operation. All three will be tested but the default
// mode shall be AUTO_SCAN_ON_ACT
#define     MANUAL_MATRIX_SCAN                  0
#define     AUTO_SCAN_ON_ACT                    1           
#define     AUTO_SCAN_BY_AS_BIT                 2

#define     NOT_AVAILABLE                       0
#define     AVAILABLE                           1
#define     COUNT_MASK                          0xFF
#define     START_VALUE                         0x7F
#define     SCAN_CODE_MASK                      0xFF
#define     NO_KEY                              0xFF
#define     SCAN_CODE_SCROLL_UP                 0xA
#define     SCAN_CODE_SCROLL_DOWN               0xB
#define     SCAN_CODE_ACTION                    0xC

#define     DISABLE_DIRECT_KEYS_INTR()          v_pKeyPadRegs->kpControlReg &= ~DIRECT_KP_INTR_ENABLE               
#define     DISABLE_MAT_KEYS_INTR()             v_pKeyPadRegs->kpControlReg &= ~MATRIX_INTR_ENABLE
#define     EN_DIRECT_KEYS_INTR()               v_pKeyPadRegs->kpControlReg |=  DIRECT_KP_INTR_ENABLE               
#define     EN_MAT_KEYS_INTR()                  v_pKeyPadRegs->kpControlReg |=  MATRIX_INTR_ENABLE

#ifdef __cplusplus
extern "C"{
#endif

XLLP_BOOL_T XllpKeyPadConfigure(XLLP_KEYPAD_REGS *,XLLP_GPIO_T *);
XLLP_BOOL_T XllpSetUpKeyPadInterrupts(XLLP_KEYPAD_REGS *,XLLP_BOOL_T );
XLLP_BOOL_T XllpReadScanCode(XLLP_KEYPAD_REGS *,XLLP_UINT8_T *);
XLLP_UINT32_T XllpKpKeypressIsInProgress (XLLP_GPIO_T *);

#ifdef __cplusplus
}
#endif

