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
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  FECHWINIT.c
//
//  Routines for setting FEC hardware registers.
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <ethdbg.h>
#include <fmd.h>
#include <halether.h>
#include <oal.h>

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types



//-----------------------------------------------------------------------------
// Local Variables


/////////////////////////////////////////////////////////////////////////////////
// init FEC from Bootloader
void    InitFecPhysical(void);
void cSPI_writeX(unsigned int);
unsigned int cSPI_readX(void);
void cSPI_transferX(void);

#define GPIOB_BASE_ADDR 0x10015100
#define GPIOB_DDIR          (GPIOB_BASE_ADDR+0x00)     // 32bit gpio ptb data direction reg
#define GPIOB_OCR2          (GPIOB_BASE_ADDR+0x08)     // 32bit gpio ptb output config 2 reg
#define GPIOB_DR            (GPIOB_BASE_ADDR+0x1C)     // 32bit gpio ptb data reg
#define GPIOB_GIUS          (GPIOB_BASE_ADDR+0x20)     // 32bit gpio ptb in use reg

#define GPIOD_BASE_ADDR 0x10015300
#define GPIOD_DDIR          (GPIOD_BASE_ADDR+0x00)     // 32bit gpio ptd data direction reg
#define GPIOD_ICONFA1       (GPIOD_BASE_ADDR+0x0C)     // 32bit gpio ptd input config A1 reg
#define GPIOD_GIUS          (GPIOD_BASE_ADDR+0x20)     // 32bit gpio ptd in use reg
#define GPIOD_GPR           (GPIOD_BASE_ADDR+0x38)     // 32bit gpio ptd general purpose reg

#define GPIOF_BASE_ADDR 0x10015500
#define GPIOF_DDIR          (GPIOF_BASE_ADDR+0x00)     // 32bit gpio ptf data direction reg
#define GPIOF_OCR1          (GPIOF_BASE_ADDR+0x04)     // 32bit gpio ptf output config 1 reg
#define GPIOF_DR            (GPIOF_BASE_ADDR+0x1C)     // 32bit gpio ptf data reg
#define GPIOF_GIUS          (GPIOF_BASE_ADDR+0x20)     // 32bit gpio ptf in use reg

#define CRM_BASE_ADDR       0x10027000
#define CSPI2_BASE_ADDR     0x1000F000

#define WRITE   1
#define READ    0

#define XCH     0x200

#define FEC_TXD0    0x00000001
#define FEC_TXD1    0x00000002
#define FEC_TXD2    0x00000004
#define FEC_TXD3    0x00000008
#define FEC_MDC     0x00000200
#define FEC_TX_EN   0x00800000
#define FEC_TX_ER   0x00010000
#define FEC_MDIO    0x00000100
#define FEC_PORTD_ICONFA1   0x000F00FF
#define PORTD_CONFIG_FEC_SPI_PINS   0x0E1FFFFF      
#define FEC_ENABLE_PORTB24_ON            (0x01 << 24)
#define FEC_ENABLE_PORTB24_GIUS          (0x01 << 24)
#define FEC_ENABLE_PORTB24_DDIR          (0x01 << 24)
#define FEC_ENABLE_PORTB24_DATAOUTPUT    0x0030000
#define FEC_RESET_PORTF10_ON             (0x01 << 10) 
#define FEC_RESET_PORTF10_GIUS           (0x01 << 10)
#define FEC_RESET_PORTF10_DDIR           (0x01 << 10)
#define FEC_RESET_PORTF10_DATAOUTPUT     0x00300000
#define GPIO_GUIS_OFFSET                 0x08
#define GPIO_PUEN_OFFSET                 0x10
#define GPIO_GPR_OFFSET                  0x0E
#define CSPI2_CONREG_RESERVED_BIT        0xFCC8E000
#define CSPI2_CONREG_DATARATE_BIT        0x00070000
#define CSPI2_CONREG_SET_DATARATE        0x00020000
#define CSPI2_CONREG_DRCTL               0x00300000
#define CSPI2_CONREG_STARTSPI            0x0000001F
#define CSPI2_CONREG_SET_BITCOUNT        0x00000D00
#define CSPI2_CONREG_BITCOUNT_BIT        0x00001F00
#define CSPI2_CONREG_SET_SSCTL           0x00000040
#define CSPI2_CONREG_SET_PHA             0x00000020
#define CSPI2_CONREG_SET_SSPOL           0x00000080
#define CRM_PCCR0_OFFSET                 0x08
#define CRM_PCCR1_OFFSET                 0x09
#define CSPI2_RXDATA_OFFSET              0x00
#define CSPI2_TXDATA_OFFSET              0x01
#define CSPI2_CONREG_OFFSET              0x02
#define CSPI2_INTREG_OFFSET              0x03
#define CSPI2_RESET_OFFSET               0x07
#define CSPI2_START                      0x01
#define CRM_PCCR0_CSPI1_EN               (1<<31) 
#define CRM_PCCR0_CSPI2_EN               (1<<30) 
#define CRM_PCCR1_PERCLK2_EN             (1<<9)
#define CSPI2_INTREG_RHEN                0x10

UINT32 *pCSPI2;

#define MAXVAL      0xffffff

/////////////////////////////////////////////////////////////////////////////////


void SPI_port_initX(void)
{
    UINT32 *pGPIOD;
    UINT32 temp;

    pGPIOD = OALPAtoUA(GPIOD_BASE_ADDR);

    temp = INREG32(pGPIOD + GPIO_GUIS_OFFSET);
    temp &= PORTD_CONFIG_FEC_SPI_PINS;
    OUTREG32(pGPIOD+0x08,temp);
    temp = INREG32(pGPIOD + GPIO_GUIS_OFFSET);

    temp = INREG32(pGPIOD + GPIO_GPR_OFFSET);
    temp &= PORTD_CONFIG_FEC_SPI_PINS;
    OUTREG32(pGPIOD+ GPIO_GPR_OFFSET,temp);
    temp = INREG32(pGPIOD + GPIO_GPR_OFFSET);

    temp = INREG32(pGPIOD + GPIO_PUEN_OFFSET);
    temp &= PORTD_CONFIG_FEC_SPI_PINS;
    OUTREG32(pGPIOD+ GPIO_PUEN_OFFSET,temp);
    temp = INREG32(pGPIOD + GPIO_PUEN_OFFSET);
}


void SPI_module_initX(void)
{
    UINT32 temp,reg;
    UINT32 *pCRMBase;

    pCRMBase = OALPAtoUA(CRM_BASE_ADDR);

    temp = INREG32(pCRMBase + CRM_PCCR0_OFFSET);
    temp |= CRM_PCCR0_CSPI1_EN;
    temp |= CRM_PCCR0_CSPI2_EN;
    OUTREG32(pCRMBase + CRM_PCCR0_OFFSET,temp);

    temp = INREG32(pCRMBase + CRM_PCCR1_OFFSET);
    temp |= CRM_PCCR1_PERCLK2_EN;
    OUTREG32(pCRMBase + CRM_PCCR1_OFFSET,temp);

    temp = INREG32(pCSPI2 + CSPI2_RESET_OFFSET);
    temp |= CSPI2_START;
    OUTREG32(pCSPI2 + CSPI2_RESET_OFFSET,temp);

    reg = INREG32(pCSPI2 + CSPI2_CONREG_OFFSET);

    //init module

    reg &= ~CSPI2_CONREG_RESERVED_BIT; 
    reg &= ~CSPI2_CONREG_DRCTL; 
    reg &= ~CSPI2_CONREG_DATARATE_BIT; 
    reg |=  CSPI2_CONREG_SET_DATARATE; 
    reg &= ~CSPI2_CONREG_BITCOUNT_BIT; 
    reg |=  CSPI2_CONREG_SET_BITCOUNT; 
    reg |=  CSPI2_CONREG_SET_SSPOL; 
    reg &= ~CSPI2_CONREG_SET_SSCTL; 
    reg &= ~CSPI2_CONREG_SET_PHA; 
    reg &= ~CSPI2_CONREG_STARTSPI;
    reg |= CSPI2_CONREG_STARTSPI;
    
    OUTREG32(pCSPI2 + CSPI2_CONREG_OFFSET,reg);

}

void SPI_link_initX(void)
{
    // Enable lights to show Atlas is alive
    SPI_port_initX();
    SPI_module_initX();
}

unsigned int frameX(UINT32 readwrite, unsigned int address, unsigned int data)
{
    unsigned int frame_data = 0;

    frame_data |= (readwrite << 31);
    frame_data |= (address << 25);
    frame_data |= data;

    return frame_data;
}

void cSPI_writeX(unsigned int data)
{
    OUTREG32(pCSPI2 + CSPI2_TXDATA_OFFSET,data);
}

void cSPI_transferX(void)
{
    UINT32 temp;

    temp = INREG32(pCSPI2 + CSPI2_CONREG_OFFSET);
    temp |= XCH;
    OUTREG32(pCSPI2 + CSPI2_CONREG_OFFSET,temp);

    temp = INREG32(pCSPI2 + CSPI2_CONREG_OFFSET);

    while((temp & XCH) == XCH) 
    {
        temp = INREG32(pCSPI2 + CSPI2_CONREG_OFFSET);
    }
}

unsigned int  cSPI_readX(void)
{
    UINT32 temp;

    temp = INREG32(pCSPI2 + CSPI2_RXDATA_OFFSET);

    return temp;
}

unsigned int Atlas_readX(unsigned int reg)
{
    unsigned int data=0;
    UINT32  temp;

    cSPI_writeX(frameX(READ, reg, 0));
    cSPI_transferX();

    temp = INREG32(pCSPI2 + CSPI2_INTREG_OFFSET);
    while((temp & CSPI2_INTREG_RHEN) == 0) 
    {
        temp = INREG32(pCSPI2 + CSPI2_INTREG_OFFSET);
    }

    data = cSPI_readX();

    return data;
}


void Atlas_writeX(unsigned int reg, unsigned int data)
{
    UINT32 temp;

    cSPI_writeX(frameX(WRITE, reg, data));
    cSPI_transferX();

    temp = INREG32(pCSPI2 + CSPI2_INTREG_OFFSET);

    // Wait to receive one word

    while((temp & CSPI2_INTREG_RHEN) == 0) 
    {
        temp = INREG32(pCSPI2 + CSPI2_INTREG_OFFSET);
    }

    // Don't care of the received word, just read it to flush the FIFO
    cSPI_readX();
}

void InitFecPhysical(void)
{
    volatile UINT32  i;                 // i is declared as volatile to avoid
                                        // complier optimization.
    UINT32  *pGPIO;
    UINT32  temp;
    UINT32 iValue;

    pCSPI2 = OALPAtoUA(CSPI2_BASE_ADDR);

    //
    // config GPIO
    //
    pGPIO = OALPAtoUA(GPIOD_DDIR);      // config output
    iValue = FEC_TXD0 | FEC_TXD1 | FEC_TXD2 | FEC_TXD3 | FEC_MDIO | FEC_TX_ER;
    OUTREG32(pGPIO, iValue);

    pGPIO = OALPAtoUA(GPIOD_GIUS);
    OUTREG32(pGPIO, FEC_TX_EN);

    pGPIO = OALPAtoUA(GPIOF_DDIR);      // config FEC_TX_EN
    OUTREG32(pGPIO, FEC_TX_EN);

    pGPIO = OALPAtoUA(GPIOD_ICONFA1);   // config input
    OUTREG32(pGPIO, FEC_PORTD_ICONFA1);        // All transmit pins are configured 
                                        // to 1 and recieve pins are configured to zero.

    // MDIO
    pGPIO = OALPAtoUA(GPIOD_GIUS);      // disable GPIO
    OUTREG32(pGPIO, (~FEC_MDIO));

    pGPIO = OALPAtoUA(GPIOD_GPR);       // select alternate function
    OUTREG32(pGPIO, FEC_MDIO);

    //
    // Config SPI interface
    //
    SPI_link_initX();

    //
    // Turn ON GPIO1_EN (bit 6, register 34) on the Atlas PMIC. This is wired as the
    // PWR_EN (SS) to the DC-DC converter, that feeds the 2.77V needed by the FEC.
    //
    temp = Atlas_readX(34);
    temp |= (0x01<<6);
    Atlas_writeX(34,temp);
    for (i=0; i < MAXVAL; ++i);
    for (i=0; i < MAXVAL; ++i);
    KITLOutputDebugString("ATLAS GPIO1_EN Write Done !\r\n");   //Note: This adds a good delay. Don't remove for now.

    //
    // config more GPIOs
    //
    pGPIO   = OALPAtoUA(GPIOB_GIUS);
    temp    = INREG32(pGPIO);
    temp    |= FEC_ENABLE_PORTB24_GIUS;
    OUTREG32(pGPIO, temp);

    pGPIO   = OALPAtoUA(GPIOB_DDIR);
    temp    = INREG32(pGPIO);
    temp    |= FEC_ENABLE_PORTB24_DDIR;
    OUTREG32(pGPIO, temp);

    pGPIO   = OALPAtoUA(GPIOB_OCR2);
    temp    = INREG32(pGPIO);
    temp    |= FEC_ENABLE_PORTB24_DATAOUTPUT;    //FEC_ENABLE
    OUTREG32(pGPIO, temp);

    pGPIO   = OALPAtoUA(GPIOB_DR);              //Set PB24(FEC_ENABLE) high
    temp    = INREG32(pGPIO);
    temp    |= FEC_ENABLE_PORTB24_ON;
    OUTREG32(pGPIO, temp);

    pGPIO   = OALPAtoUA(GPIOF_GIUS);
    temp    = INREG32(pGPIO);
    temp    |= FEC_RESET_PORTF10_GIUS;
    OUTREG32(pGPIO, temp);

    pGPIO   = OALPAtoUA(GPIOF_DDIR);
    temp    = INREG32(pGPIO);
    temp    |= FEC_RESET_PORTF10_DDIR;
    OUTREG32(pGPIO, temp);

    pGPIO   = OALPAtoUA(GPIOF_OCR1);
    temp    = INREG32(pGPIO);
    temp    |= FEC_RESET_PORTF10_DATAOUTPUT;   //FEC_RST
    OUTREG32(pGPIO, temp);

    pGPIO   = OALPAtoUA(GPIOF_DR);
    temp    = INREG32(pGPIO);
    temp    &= ~FEC_RESET_PORTF10_ON;
    OUTREG32(pGPIO, temp);
    for (i=0; i < MAXVAL; ++i);

    temp |= FEC_RESET_PORTF10_ON;
    OUTREG32(pGPIO,temp);                       //Set PF10(FEC_RST) high
    for (i=0; i < MAXVAL; ++i);
}

