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
// Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------
//
// Header: mx27_gpio.h
//
// Provides definitions for GPIO module based on MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_GPIO_H__
#define __MX27_GPIO_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define GPIO_INTR_SOURCES_MAX           32
#define GPIO_PINS_PER_PORT              32

#define GPIO_PORT(port)                 GPIOCat(GPIO_PORT_, port)
#define GPIO_CONFIG_TYPE(type)          GPIOCat(GPIO_CFG_, type)
#define GPIO_INT_TYPE(type)             GPIOCat(GPIO_INT_TYPE_, type)
#define GPIO_OUTPUT_SOURCE(source)      GPIOCat(GPIO_OUTPUT_SOURCE_, source)
#define GPIO_INPUT_DEST(dest)           GPIOCat(GPIO_INPUT_DEST_, dest)

#define GPIO_PIN_MASK(pin)              (1U << (pin))
#define GPIO_PIN_2BITS_MASK(pin)        (3U << (((pin) % 16) << 1))
#define GPIO_PIN_2BITS_VAL(pin, val)    (val << (((pin) % 16) << 1))

typedef enum {
    GPIO_PORT_A,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    GPIO_PORT_E,
    GPIO_PORT_F,
    GPIO_PORT_MAX,
} GPIO_PORT;

// Note: GPIO_INT_TYPE defines are in sync with ICR register
typedef enum {
    GPIO_INT_TYPE_POSEDGE,          // Rising edge triggered
    GPIO_INT_TYPE_NEGEDGE,          // Falling edge triggered
    GPIO_INT_TYPE_POSLEVEL,         // Level high triggered
    GPIO_INT_TYPE_NEGLEVEL,         // Level low triggered
    GPIO_INT_TYPE_MAX
} GPIO_INT_TYPE;

typedef enum {
    GPIO_CFG_PRI,
    GPIO_CFG_ALT,
    GPIO_CFG_MODULEIO,
    GPIO_CFG_INT,
    GPIO_CFG_IO,
    GPIO_CFG_MAX,
} GPIO_CFG_TYPE;

// Note: GPIO_OUTPUT_SOURCE_TYPE defines are in sync with OCR register
typedef enum {
    GPIO_OUTPUT_SOURCE_AIN,
    GPIO_OUTPUT_SOURCE_BIN,
    GPIO_OUTPUT_SOURCE_CIN,
    GPIO_OUTPUT_SOURCE_DATA,
    GPIO_OUTPUT_SOURCE_MAX,
} GPIO_OUTPUT_SOURCE_TYPE;

// Note: GPIO_INPUT_DEST_TYPE defines are in sync with ICONF register
typedef enum {
    GPIO_INPUT_DEST_AOUT,
    GPIO_INPUT_DEST_BOUT,
    GPIO_INPUT_DEST_INTSTATUS,
    GPIO_INPUT_DEST_MAX,
} GPIO_INPUT_DEST_TYPE;


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    REG32 DDIR;         // 0x00
    REG32 OCR1;         // 0x04
    REG32 OCR2;         // 0x08
    REG32 ICONFA1;      // 0x0C
    REG32 ICONFA2;      // 0x10
    REG32 ICONFB1;      // 0x14
    REG32 ICONFB2;      // 0x18
    REG32 DR;           // 0x1C
    REG32 GIUS;         // 0x20
    REG32 SSR;          // 0x24
    REG32 ICR1;         // 0x28
    REG32 ICR2;         // 0x2C
    REG32 IMR;          // 0x30
    REG32 ISR;          // 0x34
    REG32 GPR;          // 0x38
    REG32 SWR;          // 0x3C
    REG32 PUEN;         // 0x40
    REG32 RESERVED[47]; // Reserved 0x44-0x100
} CSP_GPIO_PORT_REGS, *PCSP_GPIO_PORT_REGS;

typedef struct {
    CSP_GPIO_PORT_REGS PORT[GPIO_PORT_MAX];
    REG32 PMASK;
} CSP_GPIO_REGS, *PCSP_GPIO_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define GPIO_DDIR_OFFSET            0x0000
#define GPIO_OCR1_OFFSET            0x0004
#define GPIO_OCR2_OFFSET            0x0008
#define GPIO_ICONFA1_OFFSET         0x000C
#define GPIO_ICONFA2_OFFSET         0x0010
#define GPIO_ICONFB1_OFFSET         0x0014
#define GPIO_ICONFB2_OFFSET         0x0018
#define GPIO_DR_OFFSET              0x001C
#define GPIO_GIUS_OFFSET            0x0020
#define GPIO_SSR_OFFSET             0x0024
#define GPIO_ICR1_OFFSET            0x0028
#define GPIO_ICR2_OFFSET            0x002C
#define GPIO_IMR_OFFSET             0x0030
#define GPIO_ISR_OFFSET             0x0034
#define GPIO_GPR_OFFSET             0x0038
#define GPIO_SWR_OFFSET             0x003C
#define GPIO_PUEN_OFFSET            0x0040
#define GPIO_PMASK_OFFSET           0x0600

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define GPIO_DDIR_INPUT             0 // GPIO pin is input
#define GPIO_DDIR_OUTPUT            1 // GPIO pin is output

#define GPIO_OCR_AIN                0 // Output selected is Input A_IN[i]
#define GPIO_OCR_BIN                1 // Output selected is Input B_IN[i]
#define GPIO_OCR_CIN                2 // Output selected is Input C_IN[i]
#define GPIO_OCR_DATA               3 // Output selected is data register[i]

#define GPIO_ICONFA_GPIO            0 // A_OUT[i] is GPIO_In[i]
#define GPIO_ICONFA_ISR             1 // A_OUT[i] is Interrupt Status register[i]
#define GPIO_ICONFA_0               2 // A_OUT[i] is 0
#define GPIO_ICONFA_1               3 // A_OUT[i] is 1

#define GPIO_ICONFB_GPIO            0 // B_OUT[i] is GPIO_In[i]
#define GPIO_ICONFB_ISR             1 // B_OUT[i] is Interrupt Status register[i]
#define GPIO_ICONFB_0               2 // B_OUT[i] is 0
#define GPIO_ICONFB_1               3 // B_OUT[i] is 1

#define GPIO_GIUS_MUX               0 // GPIO pin is utilized for multiplexed function
#define GPIO_GIUS_GPIO              1 // GPIO pin is utilized for GPIO function

#define GPIO_ICR_RISE_EDGE          0 // Interrupt is rising edge
#define GPIO_ICR_FALL_EDGE          1 // Interrupt is falling edge
#define GPIO_ICR_HIGH_LEVEL         2 // Interrupt is high-level
#define GPIO_ICR_LOW_LEVEL          3 // Interrupt is low-level

#define GPIO_IMR_MASKED             0 // Interrupt is masked
#define GPIO_IMR_UNMASKED           1 // Interrupt is unmasked

#define GPIO_GPR_PRI                0 // Select primary pin function
#define GPIO_GPR_ALT                1 // Select alternate pin function

#define GPIO_SWR_NOEFFECT           0 // No reset
#define GPIO_SWR_RESET              1 // GPIO circuitry for Port X reset

#define GPIO_PUEN_TRISTATE          0 // Pin [i] is tri-stated when not driven
#define GPIO_PUEN_HIGH              1 // Pin [i] is pulled high1 when not driven

#define GPIO_PMASK_MASKED           0 // Port interrupt is masked
#define GPIO_PMASK_UNMASKED         1 // Port interrupt is unmasked


//------------------------------------------------------------------------------
//
// GPIO configuration setting macros:
// 
// These configuration setting macros are used to fill in the DDK_GPIO_CFG 
// structure. Each set of macros is defined for a certain module/function.
// Each set a macros is identified via an unique name. ie <MODULE>
//
//  MACRO                       settings
//  GPIO_<MODULE>_PORT          A / B / C / D / E / F 
//  GPIO_<MODULE>_CONFIG_TYPE   PRI / ALT / MODULEIO / INT / IO
//  GPIO_<MODULE>_MASK          set to 1 for pins used (for PRI, ALT and INT only)
//  GPIO_<MODULE>_IN_MASK       set to 1 for input pins (for MODULEIO and IO only)
//  GPIO_<MODULE>_OUT_MASK      set to 1 for output pins (for MODULEIO and IO only)
//  GPIO_<MODULE>_IN_DEST       AOUT / BOUT / MAX (for MODULEIO only, sets input pin dest)
//  GPIO_<MODULE>_OUT_SOURCE    AIN / BIN / CIN / MAX (for MODULEIO only, sets output pin source)
//  GPIO_<MODULE>_PIN           0 to 31 (For INTR, IO only)
//  GPIO_<MODULE>_INT_TYPE      POSLEVEL / POSEDGE / NEGLEVEL / NEGEDGE (for INT only)
//
// NOTES:
//  1) "MAX" means non applicable.
//  2) (PRI, ALT, MODULEIO) are defined here as these are chip specific.
//  3) (INT and IO macros) are defined in bsp_gpio.h since these are BSP 
//     specific.
//
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
// PRIMARY function modules
////////////////////////////////////////////////////////////////////////////////

// PCMCIA
//      PF7-14, PF16-20                 PRI
#define GPIO_PCMCIA_PORT                GPIO_PORT(F)
#define GPIO_PCMCIA_CONFIG_TYPE         PRI
#define GPIO_PCMCIA_MASK                (0x001F7F80)
#define GPIO_PCMCIA_PUEN                GPIO_PCMCIA_MASK

// UART1
//      PE12-15                         PRI
#define GPIO_UART1_PORT                 GPIO_PORT(E)
#define GPIO_UART1_CONFIG_TYPE          PRI
#define GPIO_UART1_MASK                 (0x0000F000)
#define GPIO_UART1_PUEN                 GPIO_UART1_MASK

// UART1_IR
//      PE12-13                         PRI
#define GPIO_UART1_IR_PORT              GPIO_PORT(E)
#define GPIO_UART1_IR_CONFIG_TYPE       PRI
#define GPIO_UART1_IR_MASK              (0x00003000)
#define GPIO_UART1_IR_PUEN              GPIO_UART1_IR_MASK

// UART2
//      PE3-4, PE6-7                    PRI
#define GPIO_UART2_PORT                 GPIO_PORT(E)
#define GPIO_UART2_CONFIG_TYPE          PRI
#define GPIO_UART2_MASK                 (0x000000D8)
#define GPIO_UART2_PUEN                 GPIO_UART2_MASK

// UART2_IR
//      PE6-7                           PRI
#define GPIO_UART2_IR_PORT              GPIO_PORT(E)
#define GPIO_UART2_IR_CONFIG_TYPE       PRI
#define GPIO_UART2_IR_MASK              (0x000000C0)
#define GPIO_UART2_IR_PUEN              GPIO_UART2_IR_MASK  

// UART3
//      PE8-11                          PRI
#define GPIO_UART3_PORT                 GPIO_PORT(E)
#define GPIO_UART3_CONFIG_TYPE          PRI
#define GPIO_UART3_MASK                 (0x00000F00)
#define GPIO_UART3_PUEN                 GPIO_UART3_MASK

// UART3_IR
//      PE8-9                           PRI
#define GPIO_UART3_IR_PORT              GPIO_PORT(E)
#define GPIO_UART3_IR_CONFIG_TYPE       PRI
#define GPIO_UART3_IR_MASK              (0x00000300)
#define GPIO_UART3_IR_PUEN              GPIO_UART3_IR_MASK  

// CSPI1
//      PD25-31,                        PRI     
#define GPIO_CSPI1_PORT                 GPIO_PORT(D)
#define GPIO_CSPI1_CONFIG_TYPE          PRI
#define GPIO_CSPI1_MASK                 (0xFE000000)
#define GPIO_CSPI1_PUEN                 GPIO_CSPI1_MASK
                                                     
// CSPI2
//      PD19-24,                        PRI     
#define GPIO_CSPI2_PORT                 GPIO_PORT(D)
#define GPIO_CSPI2_CONFIG_TYPE          PRI
#define GPIO_CSPI2_MASK                 (0x01F80000)
#define GPIO_CSPI2_PUEN                 GPIO_CSPI2_MASK

// LCDC
//      PA5-31 PRI function
#define GPIO_LCDC_PORT                  GPIO_PORT(A)
#define GPIO_LCDC_CONFIG_TYPE           PRI
#define GPIO_LCDC_MASK                  (0xFFFFFFE0)
#define GPIO_LCDC_PUEN                  GPIO_LCDC_MASK  

// PWMO
//      PE5,                            PRI
#define GPIO_PWMO_PORT                  GPIO_PORT(E)
#define GPIO_PWMO_CONFIG_TYPE           PRI
#define GPIO_PWMO_MASK                  (0x00000020)
#define GPIO_PWMO_PUEN                  GPIO_PWMO_MASK  

// I2C1
//      PD17,PD18 PRI function
#define GPIO_I2C1_PORT                  GPIO_PORT(D)
#define GPIO_I2C1_CONFIG_TYPE           PRI
#define GPIO_I2C1_MASK                  (0x00060000)
#define GPIO_I2C1_PUEN                  GPIO_I2C1_MASK 

// I2C2
//      PC5,PC6 PRI function
#define GPIO_I2C2_PORT                  GPIO_PORT(C)
#define GPIO_I2C2_CONFIG_TYPE           PRI
#define GPIO_I2C2_MASK                  (0x00000060)
#define GPIO_I2C2_PUEN                  GPIO_I2C2_MASK  

//SDHC1
//          PE18-23, PRI    PE18-20, 22 pullup, PE23 pull low
#define GPIO_SDHC1_PORT             GPIO_PORT(E)
#define GPIO_SDHC1_CONFIG_TYPE      PRI
#define GPIO_SDHC1_MASK             (0x00FC0000)
#define GPIO_SDHC1_DAT0_PIN         18
#define GPIO_SDHC1_DAT0_MASK        GPIO_PIN_MASK(GPIO_SDHC1_DAT0_PIN)
#define GPIO_SDHC1_PUEN             (0x00FC0000)       

// SDHC2
//          PB4-9, PRI  (PB4-5, 6,7, 8 pullup, PB 9 pull low)
#define GPIO_SDHC2_PORT             GPIO_PORT(B)
#define GPIO_SDHC2_CONFIG_TYPE      PRI
#define GPIO_SDHC2_MASK             (0x000003F0)
#define GPIO_SDHC2_DAT0_PIN         4
#define GPIO_SDHC2_DAT0_MASK        GPIO_PIN_MASK(GPIO_SDHC2_DAT0_PIN)
#define GPIO_SDHC2_PUEN             (0x000001F0)   //clear 9


// NFC
//      PF0-6, PRI function
#define GPIO_NFC_PORT                   GPIO_PORT(F)
#define GPIO_NFC_CONFIG_TYPE            PRI
#define GPIO_NFC_MASK                   (0x0000007F)
#define GPIO_NFC_PUEN                   GPIO_NFC_MASK

// ATAD1 (DATA0-14)
//      PD2-16, PRI function
#define GPIO_ATAD1_PORT                   GPIO_PORT(D)
#define GPIO_ATAD1_CONFIG_TYPE            PRI
#define GPIO_ATAD1_MASK                   (0x0001FFFC)
#define GPIO_ATAD1_PUEN                   GPIO_ATAD1_MASK

// ATAD2 (DATA15)
//      PF23, PRI function
#define GPIO_ATAD2_PORT                   GPIO_PORT(F)
#define GPIO_ATAD2_CONFIG_TYPE            PRI
#define GPIO_ATAD2_MASK                   (0x00800000)
#define GPIO_ATAD2_PUEN                   GPIO_ATAD2_MASK


//SSI1
//      PE20-23                         PRI
#define GPIO_SSI1_PORT                 GPIO_PORT(C)
#define GPIO_SSI1_CONFIG_TYPE          PRI
#define GPIO_SSI1_MASK                 (0x00F00000)
#define GPIO_SSI1_PUEN                 GPIO_SSI1_MASK

// SSI2
//      PE24-27                         PRI
#define GPIO_SSI2_PORT                 GPIO_PORT(C)
#define GPIO_SSI2_CONFIG_TYPE          PRI
#define GPIO_SSI2_MASK                 (0x0F000000)
#define GPIO_SSI2_PUEN                 GPIO_SSI2_MASK

// USB
#define GPIO_USBOTG_E_PORT                GPIO_PORT(E)
#define GPIO_USBOTG_E_CONFIG_TYPE         PRI
#define GPIO_USBOTG_E_MASK                (0x03000007)
#define GPIO_USBOTG_E_PUEN                GPIO_USBOTG_E_MASK

#define GPIO_USBOTG_C_PORT                GPIO_PORT(C)
#define GPIO_USBOTG_C_CONFIG_TYPE         PRI
#define GPIO_USBOTG_C_MASK                (0x00003F80)
#define GPIO_USBOTG_C_PUEN                GPIO_USBOTG_C_MASK

// USB GPIO_PORT  E
#define GPIO_USBOTG_E_PORT                GPIO_PORT(E)
#define GPIO_USBOTG_E_CONFIG_TYPE         PRI
#define GPIO_USBOTG_E_MASK                (0x03000007)
#define GPIO_USBOTG_E_PUEN                GPIO_USBOTG_E_MASK
// USB GPIO_PORT  C
#define GPIO_USBOTG_C_PORT                GPIO_PORT(C)
#define GPIO_USBOTG_C_CONFIG_TYPE         PRI
#define GPIO_USBOTG_C_MASK                (0x00003F80)
#define GPIO_USBOTG_C_PUEN                GPIO_USBOTG_C_MASK
// USB GPIO_PORT  B
#define GPIO_USBOTG_B_PORT                GPIO_PORT(B) 
#define GPIO_USBOTG_B_CONFIG_TYPE         PRI
#define GPIO_USBOTG_B_MASK                (0x01800000)
#define GPIO_USBOTG_B_PUEN                GPIO_USBOTG_B_MASK

// USB GPIO_PORT  D
#define GPIO_USBH2_D_PORT                GPIO_PORT(D) 
#define GPIO_USBH2_D_CONFIG_TYPE         ALT
#define GPIO_USBH2_D_MASK                (0x05F80000)
#define GPIO_USBH2_D_PUEN                GPIO_USBH2_D_MASK

// USB GPIO_PORT  A
#define GPIO_USBH2_A_PORT                GPIO_PORT(A) 
#define GPIO_USBH2_A_CONFIG_TYPE         PRI
#define GPIO_USBH2_A_MASK                (0x0000001F)
#define GPIO_USBH2_A_PUEN                GPIO_USBH2_A_MASK

//eric
#define GPIO_USBF1_B_PORT                GPIO_PORT(B)  
#define GPIO_USBF1_B_CONFIG_TYPE         PRI
#define GPIO_USBF1_B_MASK                (0x0FE400000)
#define GPIO_USBF1_B_PUEN                GPIO_USBF1_B_MASK

#define GPIO_USBOTG_PWR_B_PORT            GPIO_PORT(B)
#define GPIO_USBOTG_PWR_B_CONFIG_TYPE     PRI
#define GPIO_USBOTG_PWR_B_MASK            (0x01800000)       //PB23 as USB_PWR
#define GPIO_USBOTG_PWR_B_PUEN            (GPIO_USBOTG_PWR_B_MASK)

////////////////////////////////////////////////////////////////////////////////
// ALT pin configurations
////////////////////////////////////////////////////////////////////////////////

//		OWIRE
//      PE16,                           ALT
#define GPIO_OWIRE_PORT                 GPIO_PORT(E)
#define GPIO_OWIRE_CONFIG_TYPE          ALT
#define GPIO_OWIRE_MASK                 (0x00010000)
#define GPIO_OWIRE_PUEN                 GPIO_OWIRE_MASK 

//   CSPI3
//      PE18, PE21-23,                  ALT     
#define GPIO_CSPI3_PORT                 GPIO_PORT(E)
#define GPIO_CSPI3_CONFIG_TYPE          ALT
#define GPIO_CSPI3_MASK                 (0x00E40000)
#define GPIO_CSPI3_PUEN                 GPIO_CSPI3_MASK

// UART4_ALT
//      PB28                            ALT
#define GPIO_UART4_ALT_PORT             GPIO_PORT(B)
#define GPIO_UART4_ALT_CONFIG_TYPE      ALT
#define GPIO_UART4_ALT_MASK             (0xb4000000)
#define GPIO_UART4_ALT_PUEN             GPIO_UART4_ALT_MASK 

// UART4_IR
//      PB28, PB31                      ALT
#define GPIO_UART4_IR_PORT              GPIO_PORT(B)
#define GPIO_UART4_IR_CONFIG_TYPE       ALT
#define GPIO_UART4_IR_MASK              (0x90000000)
#define GPIO_UART4_IR_PUEN              GPIO_UART4_IR_MASK

// UART5
//      PB18-21,                        ALT
#define GPIO_UART5_ALT_PORT             GPIO_PORT(B)
#define GPIO_UART5_ALT_CONFIG_TYPE      ALT
#define GPIO_UART5_ALT_MASK             (0x003c0000)
#define GPIO_UART5_ALT_PUEN             GPIO_UART5_ALT_MASK 

// UART5_IR
//      PB18-21,                        ALT
#define GPIO_UART5_IR_PORT              GPIO_PORT(B)
#define GPIO_UART5_IR_CONFIG_TYPE       ALT
#define GPIO_UART5_IR_MASK              (0x000c0000)
#define GPIO_UART5_IR_PUEN              GPIO_UART5_IR_MASK  

// UART6
//      PB10-13,                        ALT
#define GPIO_UART6_ALT_PORT             GPIO_PORT(B)
#define GPIO_UART6_ALT_CONFIG_TYPE      ALT
#define GPIO_UART6_ALT_MASK             (0x00003c00)
#define GPIO_UART6_ALT_PUEN             GPIO_UART6_ALT_MASK

// UART6_IR
//      PB10-13,                        ALT 
#define GPIO_UART6_IR_PORT              GPIO_PORT(B)
#define GPIO_UART6_IR_CONFIG_TYPE       ALT
#define GPIO_UART6_IR_MASK              (0x00000c00)
#define GPIO_UART6_IR_PUEN              GPIO_UART6_IR_MASK  

// KPP
//      PE6, ALT  (KPP_COL6)
//      PE7, ALT  (KPP_ROW6)
//      PE3, ALT  (KPP_COL7)
//      PE4, ALT  (KPP_ROW7)
#define GPIO_KPP_PORT                   GPIO_PORT(E)
#define GPIO_KPP_CONFIG_TYPE            ALT
#define GPIO_KPP_COL6_MASK              (0x00000040)
#define GPIO_KPP_ROW6_MASK              (0x00000080)
#define GPIO_KPP_COL7_MASK              (0x00000008)
#define GPIO_KPP_ROW7_MASK              (0x00000010)
#define GPIO_KPP_MASK                   (GPIO_KPP_COL7_MASK | \
                                         GPIO_KPP_COL6_MASK | \
                                         GPIO_KPP_ROW7_MASK | \
                                         GPIO_KPP_ROW6_MASK)
#define GPIO_KPP_PUEN                   GPIO_KPP_MASK  

// ATAC (CTRL)
//      PF7-14, 16-20
#define GPIO_ATAC_PORT                   GPIO_PORT(F)
#define GPIO_ATAC_CONFIG_TYPE            ALT
#define GPIO_ATAC_MASK                   (0x001f7f80)
#define GPIO_ATAC_PUEN                   GPIO_ATAC_MASK


//SDHC 3
//          PD2-5, ALt  (PD2-5 pullup)
#define GPIO_SDHC3_DA_PORT             GPIO_PORT(D)
#define GPIO_SDHC3_DA_CONFIG_TYPE      ALT
#define GPIO_SDHC3_DA_MASK             (0x0000001E)
#define GPIO_SDHC3_DAT0_PIN         2
#define GPIO_SDHC3_DAT0_MASK        GPIO_PIN_MASK(GPIO_SDHC3_DAT0_PIN)
#define GPIO_SDHC3_DA_PUEN             GPIO_SDHC3_DA_MASK

// FEC (MDIO)
//          PD8     Alt   
#define GPIO_FECMDIO_PORT           GPIO_PORT(D)
#define GPIO_FECMDIO_CONFIG_TYPE    ALT
#define GPIO_FECMDIO_MASK           (0x00000100) 
#define GPIO_FECMDIO_PUEN           0x0    

//SDHC 3
//          PD 0,1  (PD 0 pullup,   PD 1 pull low)
#define GPIO_SDHC3_PORT             GPIO_PORT(D)
#define GPIO_SDHC3_CONFIG_TYPE      PRI
#define GPIO_SDHC3_MASK             (0x00000003)
#define GPIO_SDHC3_PUEN             GPIO_SDHC3_MASK    

//PMIC
#define GPIO_PMIC_PORT	GPIO_PORT(C)
////////////////////////////////////////////////////////////////////////////////
// module GPIO pin configurations
////////////////////////////////////////////////////////////////////////////////

// UART4_REDIRECT
//      PB29, PB31                      Input,  A_OUT,  Pull Up
//      PB30                            Output, C_IN,   Pull Up
#define GPIO_UART4_REDIRECT_PORT        GPIO_PORT(B)
#define GPIO_UART4_REDIRECT_CONFIG_TYPE MODULEIO
#define GPIO_UART4_REDIRECT_IN_MASK     (0xA0000000)    // PB29,31
#define GPIO_UART4_REDIRECT_IN_DEST     GPIO_INPUT_DEST(AOUT)
#define GPIO_UART4_REDIRECT_OUT_MASK    (0x40000000)    // PB30
#define GPIO_UART4_REDIRECT_OUT_SOURCE  GPIO_OUTPUT_SOURCE(CIN)
#define GPIO_UART4_REDIRECT_PUEN        (GPIO_UART4_REDIRECT_IN_MASK | \
                                        GPIO_UART4_REDIRECT_OUT_MASK)


// SLCDC
//      //PA6-PA21, 25, 26, 27 AIN function
#define GPIO_SLCDC_PORT        GPIO_PORT(A)
#define GPIO_SLCDC_CONFIG_TYPE MODULEIO
#define GPIO_SLCDC_IN_MASK     (0x00800000)    							// not used
#define GPIO_SLCDC_IN_DEST     GPIO_INPUT_DEST(AOUT)
#define GPIO_SLCDC_OUT_MASK    (0x0F7FFFC0)    					//PA6-PA21, 25, 26, 27 AIN function
#define GPIO_SLCDC_OUT_SOURCE  GPIO_OUTPUT_SOURCE(AIN)
#define GPIO_SLCDC_PUEN        (GPIO_SLCDC_IN_MASK | \
                                        GPIO_SLCDC_OUT_MASK)

// FEC
//	PD4-7, PD10-15               Input  A-out   except PD5, others pull-up enable
//  PD0-3, PD9, PD16             Output AIN     except PD16, others pull-up enable
#define GPIO_FEC_PORT                       GPIO_PORT(D)
#define GPIO_FEC_CONFIG_TYPE                MODULEIO
#define GPIO_FEC_IN_MASK                    (0x0000FCF0)
#define GPIO_FEC_IN_DEST                    GPIO_INPUT_DEST(AOUT)
#define GPIO_FEC_OUT_MASK                   (0x0001020F)
#define GPIO_FEC_OUT_SOURCE                 GPIO_OUTPUT_SOURCE(AIN)
#define GPIO_FEC_PUEN                       (0x0000FFDF)

// FEC (FEC_TX_EN)
//  PF23                         Output AIN     
#define GPIO_FECTXEN_PORT                   GPIO_PORT(F)
#define GPIO_FECTXEN_CONFIG_TYPE            MODULEIO
#define GPIO_FECTXEN_IN_MASK                (0x0)
#define GPIO_FECTXEN_IN_DEST                GPIO_INPUT_DEST(MAX)
#define GPIO_FECTXEN_OUT_MASK               (0x00800000)
#define GPIO_FECTXEN_OUT_SOURCE             GPIO_OUTPUT_SOURCE(AIN)
#define GPIO_FECTXEN_PUEN                   (0x0)



// SLCDCBKL
//      PA30, DATA function
#define GPIO_SLCDCBKL_PORT					GPIO_PORT(A)
#define GPIO_SLCDCBKL_CONFIG_TYPE			IO
#define GPIO_SLCDCBKL_IN_MASK				(0x0)
#define GPIO_SLCDCBKL_IN_DEST				GPIO_INPUT_DEST(MAX)
#define GPIO_SLCDCBKL_OUT_MASK				(0x40000000)    			// PA30
#define GPIO_SLCDCBKL_OUT_SOURCE			GPIO_OUTPUT_SOURCE(DATA)
#define GPIO_SLCDCBKL_PUEN					(GPIO_SLCDCBKL_IN_MASK | \
                                        GPIO_SLCDCBKL_OUT_MASK)


// SLCDCRESET
//      PA30, DATA function
#define GPIO_SLCDCRESET_PORT					GPIO_PORT(A)
#define GPIO_SLCDCRESET_CONFIG_TYPE			IO
#define GPIO_SLCDCRESET_IN_MASK				(0x0)
#define GPIO_SLCDCRESET_IN_DEST				GPIO_INPUT_DEST(MAX)
#define GPIO_SLCDCRESET_OUT_MASK				(0x80000000)    			// PA31
#define GPIO_SLCDCRESET_OUT_SOURCE			GPIO_OUTPUT_SOURCE(DATA)
#define GPIO_SLCDCRESET_PUEN					(GPIO_SLCDCRESET_IN_MASK | \
                                        GPIO_SLCDCRESET_OUT_MASK)


// SLCDCCS
//      PA30, DATA function
#define GPIO_SLCDCCS_PORT					GPIO_PORT(A)
#define GPIO_SLCDCCS_CONFIG_TYPE			IO
#define GPIO_SLCDCCS_IN_MASK				(0x0)
#define GPIO_SLCDCCS_IN_DEST				GPIO_INPUT_DEST(MAX)
#define GPIO_SLCDCCS_OUT_MASK				(0x00800000)    			// PA23
#define GPIO_SLCDCCS_OUT_SOURCE			GPIO_OUTPUT_SOURCE(DATA)
#define GPIO_SLCDCCS_PUEN					(GPIO_SLCDCCS_IN_MASK | \
                                        GPIO_SLCDCCS_OUT_MASK)

// CSI
//      PB10-21,        PRI
#define GPIO_CSI_PORT                      GPIO_PORT(B)
#define GPIO_CSI_CONFIG_TYPE        PRI
#define GPIO_CSI_MASK                     (0x003FFC00)
#define GPIO_CSI_PUEN                     GPIO_CSI_MASK   

//------------------------------------------------------------------------------
// WARNING: Do not change codes between these comments!!
//------------------------------------------------------------------------------
#define GPIOSetCfgPRI(config, module) \
    do { \
        config.ConfigType                   = GPIO_CONFIG_TYPE(PRI); \
        config.PriConfig.Port               = GPIOValuePort(module); \
        config.PriConfig.PinMap             = GPIOValueMask(module); \
        config.PriConfig.PuenMap            = GPIOValuePuen(module); \
    } while(0)

#define GPIOSetCfgALT(config, module) \
    do { \
        config.ConfigType                   = GPIO_CONFIG_TYPE(ALT); \
        config.AltConfig.Port               = GPIOValuePort(module); \
        config.AltConfig.PinMap             = GPIOValueMask(module); \
        config.AltConfig.PuenMap            = GPIOValuePuen(module); \
    } while(0)

#define GPIOSetCfgMODULEIO(config, module) \
    do { \
        config.ConfigType                   = GPIO_CONFIG_TYPE(MODULEIO); \
        config.ModuleIOConfig.Port          = GPIOValuePort(module); \
        config.ModuleIOConfig.InputPinMap   = GPIOValueInMask(module); \
        config.ModuleIOConfig.InputDest     = GPIOValueInDest(module); \
        config.ModuleIOConfig.OutputPinMap  = GPIOValueOutMask(module); \
        config.ModuleIOConfig.OutputSource  = GPIOValueOutSource(module); \
        config.ModuleIOConfig.PuenMap       = GPIOValuePuen(module); \
    } while(0)

#define GPIOSetCfgINT(config, module) \
    do { \
        config.ConfigType                   = GPIO_CONFIG_TYPE(INT); \
        config.IntrConfig.Port              = GPIOValuePort(module); \
        config.IntrConfig.PinMap            = GPIOValueMask(module); \
        config.IntrConfig.IntType           = GPIOValueIntType(module); \
    } while(0)

#define GPIOSetCfgIO(config, module) \
    do { \
        config.ConfigType                   = GPIO_CONFIG_TYPE(IO); \
        config.IOConfig.Port                = GPIOValuePort(module); \
        config.IOConfig.InputPinMap         = GPIOValueInMask(module); \
        config.IOConfig.OutputPinMap        = GPIOValueOutMask(module); \
        config.IOConfig.PuenMap             = GPIOValuePuen(module); \
    } while(0)


#define GPIOCat(x, y)               x##y
#define GPIOCat2(x, y)              x##y
#define GPIOValueMacro(x)           x   
#define GPIOCatMacro(module, macro) GPIOCat(GPIOValueMacro, (GPIOCat2(module, macro)))

#define GPIOValueCfgType(module)    GPIOCatMacro(GPIOCat(GPIO_, module), _CONFIG_TYPE)
#define GPIOValuePort(module)       GPIOCatMacro(GPIOCat(GPIO_, module), _PORT)
#define GPIOValueMask(module)       GPIOCatMacro(GPIOCat(GPIO_, module), _MASK)
#define GPIOValuePuen(module)       GPIOCatMacro(GPIOCat(GPIO_, module), _PUEN)
#define GPIOValueInMask(module)     GPIOCatMacro(GPIOCat(GPIO_, module), _IN_MASK)
#define GPIOValueOutMask(module)    GPIOCatMacro(GPIOCat(GPIO_, module), _OUT_MASK)
#define GPIOValueInDest(module)     GPIOCatMacro(GPIOCat(GPIO_, module), _IN_DEST)
#define GPIOValueOutSource(module)  GPIOCatMacro(GPIOCat(GPIO_, module), _OUT_SOURCE)
#define GPIOValueIntType(module)    GPIOCatMacro(GPIOCat(GPIO_, module), _INT_TYPE)

#define GPIOSetCfg3(x, y, z)        GPIOCat2(GPIOCat(GPIOSetCfg, x), (y, z))
#define GPIOSetCfg2(x, y, z)        GPIOSetCfg3(x, y, z)
//------------------------------------------------------------------------------
// WARNING: Do not change codes between these comments!!
//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // __MX27_GPIO_H__

