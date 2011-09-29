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
#include "xllp_defs.h"
#include "xllp_serialization.h"
#include "xllp_lcd.h"
#include "xllp_gpio.h"
#include "xllp_ost.h"

const unsigned short LS022Q8DD06_DATA_SET_1[] = {
0x000F, 0x0101,     /*R15 = 0x01 - Command Reset */
0x0004, 0x0101,     /*R4  = 0x01 - Standby Mode */
0x0000, 0x0103,     /*R0  = 0x03 - 260K colors */
0x0001, 0x010A,     /*R1  = 0x0A - Horz Display Start Position Setting */
0x0002, 0x0102,     /*R2  = 0x02 - Vert Display Start Position Setting */
0x0009, 0x0100,     /*R9  = 0x00 - Normal Operation Mode */
0x000A, 0x0103,     /*R10 = 0x03 - Neg Voltage of Gate & driver Output = Vss */
0x0019, 0x011A,     /*R25 = 0x1A - DC/DC Step Setting */
0x001A, 0x0115,     /*R26 = 0x15 - DC/DC Frequency Setting */
0x001B, 0x0148,     /*R27 = 0x48 - Regulator Setting (OFF) */
0x001C, 0x0100,     /*R28 = 0x00 - Low Power Setting */
0x0021, 0x0110,     /*R33 = 0x10 - DC/DC Rise Setting */
0x0018, 0x0109,     /*R24 = 0x09 - DC/DC Operation ON (Only VDC2 is ON) */
0x0003, 0x0100,     /*R3  = 0x00 - Horz Available Pixels (240) */
0x0005, 0x0100,     /*R5  = 0x00 - Register to Change 8 Color Mode (260K colors) */
0x0006, 0x0124,     /*R6  = 0x24 - Reverse Scan, Common Output is ON */
0x0008, 0x010E,     /*R8  = 0x0E - Amplitude Driving Period Setting */
0x000B, 0x0105,     /*R11 = 0x05 - Decide Common Amplitude */
0x000C, 0x0100,     /*R12 = 0x00 - Common Center Setting */
0x0024, 0x0102,     /*R36 = 0X02 - Source Output Setting */
0x0025, 0x010E,     /*R37 = 0x0E - Source Output Setting */
0x0026, 0x0112,     /*R38 = 0x12 - Source Output Setting */ 
0x0027, 0x011E,     /*R39 = 0x1E - Source Output Setting */
0x0028, 0x0122,     /*R40 = 0x22 - Source Output Setting */
0x0029, 0x012E,     /*R41 = 0x2E - Source Output Setting */
0x002A, 0x0137,     /*R42 = 0x37 - Gate Driving Signal Setting */
0x002B, 0x013A,     /*R43 = 0x3A - Gate Driving Signal Setting */
0x002C, 0x0137,     /*R44 = 0x37 - Gate Driving Signal Setting */
0x002D, 0x013A,     /*R45 = 0x3A - Gate Driving Signal Setting */
0x002E, 0x0137,     /*R46 = 0x37 - Gate Driving Signal Setting */
0x002F, 0x013A,     /*R47 = 0x3A - Gate Driving Signal Setting */
0x0030, 0x0180,     /*R48 = 0x80 - Gate Driving Signal Setting */
0x0031, 0x0101,     /*R49 = 0x01 - Gate Driving Signal Setting */
0x0032, 0x0136,     /*R50 = 0x36 - Gate Driving Signal Setting */
0x0033, 0x0101      /*R51 = 0x01 - Dummy Line setting */
};

const unsigned short LS022Q8DD06_DATA_SET_2[] = {
0x0018, 0x0179,     /* R24 = 0x79 - Vss1, Vss2 and Vr are ON */
0x001B, 0x0149,     /* R27 = 0x49 - Vs Regulator is ON */
0x0018, 0x017F,     /* R24 = 0x7F - Vdd2 is ON (Voltage in gate ON)*/
0x0036, 0x0101,     /* R54 = 0x01 */
0x0006, 0x0125,     /* R6  = 0x25 - Switch to normal mode*/
0x0004, 0x0100,     /* R4  = 0x00 - Amplifier is normal power */
0x000C, 0x0152,     /* R12 = 0x52 - Release Standby Mode */
0x0021, 0x0100,     /* R33 = 0x00 */
0x0006, 0x0135      /* R6  = 0x35 - Enable the display */
};

XLLP_STATUS_T XllpLCDInit(P_XLLP_LCD_T pXllpLCD)
{
    XLLP_STATUS_T status = 0;

    // Initialize the GPIO registers for proper LCD Controller operation
    LCDSetupGPIOs(pXllpLCD);

    // Initialize the LCD Controller and frame descriptors
    LCDInitController(pXllpLCD);

    // Clear LCD Controller status register
    LCDClearStatusReg(pXllpLCD);

    // Enable the LCD controller
    LCDEnableController(pXllpLCD);

    // If required, load the default palette into palette ram
    // and feed this to the LCD controller.
    if(pXllpLCD->BPP < BPP_16)
    {
        XllpLCDLoadPalette(pXllpLCD);
    }

    return status;
}

void XllpLCDLoadPalette(P_XLLP_LCD_T pXllpLCD)
{   
    volatile LCDRegs *p_LCDRegs;

    p_LCDRegs = (LCDRegs *) pXllpLCD->LCDC;

    // Reconfigure the second frame descriptor so that when loaded,
    // this descriptor loops to itself.
    pXllpLCD->frameDescriptorCh0fd2->FDADR = LCD_FDADR(pXllpLCD->frameDescriptorCh0fd2->PHYSADDR);

    // Reconfigure the palette frame descriptor so that it loads the second frame descriptor
    pXllpLCD->frameDescriptorPalette->FDADR = LCD_FDADR(pXllpLCD->frameDescriptorCh0fd2->FDADR);
    pXllpLCD->frameDescriptorPalette->FSADR = LCD_FSADR(pXllpLCD->_PALETTE_BUFFER_BASE_PHYSICAL); 
    pXllpLCD->frameDescriptorPalette->FIDR  = LCD_FIDR(0);


    if ( (p_LCDRegs->OVL1C1 & LCD_O1EN) || (p_LCDRegs->OVL2C1 & LCD_O2EN)) 
    {
        // Overlays are enabled
        pXllpLCD->frameDescriptorPalette->LDCMD = LCD_Len(pXllpLCD->PaletteSize << 1) | LCD_Pal;
    } else
    {
        // Overlays are disabled
        pXllpLCD->frameDescriptorPalette->LDCMD = LCD_Len(pXllpLCD->PaletteSize) | LCD_Pal;
    }

    pXllpLCD->frameDescriptorPalette->PHYSADDR = LCD_FDADR(pXllpLCD->_PALETTE_FRAME_DESCRIPTOR_BASE_PHYSICAL);

    // Insert the palette descriptor into the descriptor chain to load the palette.
    // When this load completes, fd2 is automatically loaded next in the chain.  
    // fd2 now loops to itself and continues to load frame data.
    pXllpLCD->frameDescriptorCh0fd1->FDADR = LCD_FDADR(pXllpLCD->_PALETTE_FRAME_DESCRIPTOR_BASE_PHYSICAL);      

    // swap frame descriptor pointers so that this operation is reversed the next time through
    pXllpLCD->frameDescriptorTemp   = pXllpLCD->frameDescriptorCh0fd1;
    pXllpLCD->frameDescriptorCh0fd1 = pXllpLCD->frameDescriptorCh0fd2;
    pXllpLCD->frameDescriptorCh0fd2 = pXllpLCD->frameDescriptorTemp;
}

void XllpLCDSuspend(P_XLLP_LCD_T pXllpLCD, int SuspendType)
{
    volatile LCDRegs *p_LCDRegs;
    volatile XLLP_GPIO_T *p_GPIORegs;

    p_LCDRegs = (LCDRegs *) pXllpLCD->LCDC;
    p_GPIORegs = (XLLP_GPIO_T *) pXllpLCD->GPIO;

    switch(SuspendType)
    {
    case Suspend_Graceful:
        // suspend if LCD is enabled
        if (p_LCDRegs->LCCR0 & LCD_ENB)
        {
            // Initiate power down sequence
            p_LCDRegs->LCCR0 |= LCD_DIS;

            // Wait for LDD bit to get set once the last DMA transfer has completed
            while(!(p_LCDRegs->LCSR0 & LCD_LDD));

            // Clear the sticky LDD bit
            p_LCDRegs->LCSR0 |= LCD_LDD;
        }
        break;
    case Suspend_Immediate:
        p_LCDRegs->LCCR0 &= ~LCD_ENB;
        break;
    default :
        break;
    }

    // don't use lock/unlock here because system call may be unavailable.
    p_GPIORegs->GPCR0   |= XLLP_GPIO_BIT_PWM_OUT0;

}

void XllpLCDResume(P_XLLP_LCD_T pXllpLCD)
{
    XllpLCDInit(pXllpLCD);
}

void XllpLCDSetDisplayPage(P_XLLP_LCD_T pXllpLCD, int page)
{
    // Set the physical address of the frame buffer for all three frame descriptors
    // Make sure that you've initialized FrameBufferSize before calling this function either manually
    // or through a call to XllpLCDInit().
    pXllpLCD->CurrentPage = page;
    pXllpLCD->frameDescriptorCh0fd1->FSADR = LCD_FSADR(pXllpLCD->_FRAME_BUFFER_BASE_PHYSICAL + pXllpLCD->CurrentPage*pXllpLCD->FrameBufferSize);
    pXllpLCD->frameDescriptorCh0fd2->FSADR = LCD_FSADR(pXllpLCD->_FRAME_BUFFER_BASE_PHYSICAL + pXllpLCD->CurrentPage*pXllpLCD->FrameBufferSize);
    pXllpLCD->frameDescriptorCh1->FSADR = LCD_FSADR(pXllpLCD->_FRAME_BUFFER_BASE_PHYSICAL + pXllpLCD->CurrentPage*pXllpLCD->FrameBufferSize + (pXllpLCD->FrameBufferSize >> 1));
}

void LCDInitController(P_XLLP_LCD_T pXllpLCD)
{
    int i = 0;
    int BPP = 0;
    int PCD = 0;
    unsigned int CCCR_L = 0;
    volatile LCDRegs *p_LCDRegs;
    volatile XLLP_CLKMGR_T *p_CLKRegs;
    volatile XLLP_SSPREGS_T *p_SSPRegs;
    volatile XLLP_GPIO_T *p_GPIORegs;
    XLLP_OST_T *p_OSTRegs;

    int LCLK = 0;

    XLLP_UINT32_T LockID;
    XLLP_UINT32_T LockID2;

    p_LCDRegs = (LCDRegs *) pXllpLCD->LCDC;
    p_CLKRegs = (XLLP_CLKMGR_T *) pXllpLCD->CLKMan;
    p_GPIORegs = (XLLP_GPIO_T *) pXllpLCD->GPIO;
    p_OSTRegs = (XLLP_OST_T *) pXllpLCD->OST;
    p_SSPRegs = (XLLP_SSPREGS_T *) pXllpLCD->SSP;

    p_LCDRegs->LCCR0 = 0;
    p_LCDRegs->LCCR1 = 0;
    p_LCDRegs->LCCR2 = 0;
    p_LCDRegs->LCCR3 = 0;
    p_LCDRegs->LCCR4 = 0;
    p_LCDRegs->LCCR5 = (LCD_SOFM1|LCD_SOFM2|LCD_SOFM3|LCD_SOFM4|LCD_SOFM5|LCD_SOFM6|
                        LCD_EOFM1|LCD_EOFM2|LCD_EOFM3|LCD_EOFM4|LCD_EOFM5|LCD_EOFM6|
                        LCD_BSM1 |LCD_BSM2 |LCD_BSM3 |LCD_BSM4 |LCD_BSM5 |LCD_BSM6 |
                        LCD_IUM1 |LCD_IUM2 |LCD_IUM3 |LCD_IUM4 |LCD_IUM5 |LCD_IUM6 );


    // Determine the frame buffer size for the DMA transfer length.
    // Scale the size based on the bpp of the frame buffer to determine
    // an actual size in bytes
    pXllpLCD->FrameBufferSize = pXllpLCD->FrameBufferWidth * pXllpLCD->FrameBufferHeight;
    switch (pXllpLCD->BPP)
    {
        case BPP_1:
            pXllpLCD->FrameBufferSize >>= 3;
            pXllpLCD->PaletteSize = 8;
            break;
        case BPP_2:
            pXllpLCD->FrameBufferSize >>= 2;
            pXllpLCD->PaletteSize = 8;
            break;
        case BPP_4:
            pXllpLCD->FrameBufferSize >>= 1;
            pXllpLCD->PaletteSize = 32;
            break;
        case BPP_8:
            pXllpLCD->PaletteSize = 512;
            break;
        case BPP_16:
            pXllpLCD->FrameBufferSize <<= 1;
            break;
        case BPP_18:        /* Fall through */
        case BPP_18_PACKED:
        case BPP_19:
        case BPP_19_PACKED:
        case BPP_24:
        case BPP_25:
            pXllpLCD->FrameBufferSize <<= 2;
            break;
        default:
            break;
    }

    // Enable the LCD and SRAM clocks
    LockID = XllpLock(CKEN);

    p_CLKRegs->cken = (p_CLKRegs->cken & XLLP_CLKEN_MASK) | CLK_LCD | CLK_SRAM;

    XllpUnlock(LockID);

    // Configure the general purpose frame descriptors
    // Set the physical address of the frame descriptor
    pXllpLCD->frameDescriptorCh0fd1->FDADR = LCD_FDADR(pXllpLCD->_DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_PHYSICAL);

    // Set the physical address of the frame buffer
    pXllpLCD->frameDescriptorCh0fd1->FSADR = LCD_FSADR(pXllpLCD->_FRAME_BUFFER_BASE_PHYSICAL + pXllpLCD->CurrentPage*pXllpLCD->FrameBufferSize);

    // Clear the frame ID
    pXllpLCD->frameDescriptorCh0fd1->FIDR  = LCD_FIDR(0);

    // Set the DMA transfer length to the size of the frame buffer
    pXllpLCD->frameDescriptorCh0fd1->LDCMD = LCD_Len(pXllpLCD->FrameBufferSize);

    // Store the physical address of this frame descriptor in the frame descriptor
    pXllpLCD->frameDescriptorCh0fd1->PHYSADDR = pXllpLCD->frameDescriptorCh0fd1->FDADR;

    // frameDescriptorCh0fd2 is used only if a palette load is performed.
    // Set the physical address of the frame descriptor
    pXllpLCD->frameDescriptorCh0fd2->FDADR = LCD_FDADR(pXllpLCD->_DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_PHYSICAL);

    // Set the physical address of the frame buffer
    pXllpLCD->frameDescriptorCh0fd2->FSADR = LCD_FSADR(pXllpLCD->_FRAME_BUFFER_BASE_PHYSICAL + pXllpLCD->CurrentPage*pXllpLCD->FrameBufferSize);

    // Clear the frame ID
    pXllpLCD->frameDescriptorCh0fd2->FIDR  = LCD_FIDR(0);

    // Set the DMA transfer length to the size of the frame buffer
    pXllpLCD->frameDescriptorCh0fd2->LDCMD = LCD_Len(pXllpLCD->FrameBufferSize);
    
    // Store the physical address of this frame descriptor in the frame descriptor
    pXllpLCD->frameDescriptorCh0fd2->PHYSADDR = pXllpLCD->frameDescriptorCh0fd2->FDADR;
    
    // FBR0 is cleared and is not used.
    p_LCDRegs->FBR0 = 0;

    // Load the contents of FDADR0 with the physical address of this frame descriptor
    p_LCDRegs->FDADR0 = LCD_FDADR(pXllpLCD->frameDescriptorCh0fd1->FDADR);
            
    // Determine the LCLK frequency programmed into the CCCR.
    // This value will be used to calculate a Pixel Clock Divisor (PCD)
    // for a given display type.
    CCCR_L = (p_CLKRegs->cccr & 0x0000001F);


    if (CCCR_L < 8) // L = [2 - 7]
        LCLK = (13 * CCCR_L) * 100;
    else if (CCCR_L < 17) // L = [8 - 16] 
        LCLK = ((13 * CCCR_L) * 100) >> 1;
    else if (CCCR_L < 32) // L = [17 - 31]
        LCLK = ((13 * CCCR_L) * 100) >> 2;
        
    
    // Convert the bpp setting into a value that the LCD controller understands.
    switch(pXllpLCD->BPP)
    {
        case BPP_1:
            BPP = 0;
            break;
        case BPP_2:
            BPP = 1;
            break;
        case BPP_4:
            BPP = 2;
            break;
        case BPP_8:
            BPP = 3;
            break;
        case BPP_16:
            BPP = 4;
            break;
        case BPP_18:
            BPP = 5;
            break;
        case BPP_18_PACKED:
            BPP = 6;
            break;
        case BPP_19:
            BPP = 7;
            break;
        case BPP_19_PACKED:
            BPP = 8;
            break;
        case BPP_24:
            BPP = 9;
            break;
        case BPP_25:
            BPP = 10;
            break;
        default:
        {
            BPP = 0;
            break;
        }
    }

    switch(pXllpLCD->DisplayType)
    {
        case LTM04C380K: // 640x480 16bpp active matrix
        {

            // 
            // The actual equation requires that we take the ceiling of a floating point result.
            // Rather than use floats, we'll calculate an approximation to the correct PCD value
            // using integers.  
            //
            PCD = (LCLK / (2 * LTM04C380K_PIXEL_CLOCK_FREQUENCY));

            // Configure the LCD Controller Control Registers
            p_LCDRegs->LCCR0 = (LCD_LDM | LCD_SFM | LCD_IUM | LCD_EFM | 
                                LCD_PAS | LCD_QDM | LCD_BM  | LCD_OUM |
                                LCD_RDSTM | LCD_CMDIM | LCD_OUC | LCD_LDDALT);

            p_LCDRegs->LCCR1 = (LCD_PPL(0x27FU) | LCD_HSW(0x01) | 
                                LCD_ELW(0x01)  | LCD_BLW(0x9fU) );
            
            p_LCDRegs->LCCR2 = (LCD_LPP(0x1df) | LCD_VSW(0x2c) |
                                LCD_EFW(0x00)  | LCD_BFW(0x00) );

            p_LCDRegs->LCCR3 = (LCD_PCD(PCD)  | LCD_BPP(BPP) | LCD_PCP |
                                LCD_PDFOR(pXllpLCD->PixelDataFormat));
            
            p_LCDRegs->LCCR4 = LCD_PAL_FOR(0);
            if ( (p_LCDRegs->OVL1C1 & LCD_O1EN) || (p_LCDRegs->OVL2C1 & LCD_O2EN))
            {
                p_LCDRegs->LCCR4 = LCD_PAL_FOR(1);
            }
        }
        break;

    case LTM035A776C: // 240x320 16bpp active matrix
        {

            // 
            // The actual equation requires that we take the ceiling of a floating point result.
            // Rather than use floats, we'll calculate an approximation to the correct PCD value
            // using integers.
            //
            PCD = (LCLK / (2 * LTM035A776C_PIXEL_CLOCK_FREQUENCY));
            
            // Configure the LCD Controller Control Registers
            p_LCDRegs->LCCR0 = (LCD_LDM | LCD_SFM | LCD_IUM | LCD_EFM | 
                                LCD_PAS | LCD_QDM | LCD_BM  | LCD_OUM |
                                LCD_RDSTM | LCD_CMDIM | LCD_OUC | LCD_LDDALT);

            p_LCDRegs->LCCR1 = (LCD_PPL(0xEF) | LCD_HSW(0x04) | 
                                LCD_ELW(0x04)  | LCD_BLW(0x05) );
            
            p_LCDRegs->LCCR2 = (LCD_LPP(0x13f) | LCD_VSW(0x02) |
                                LCD_EFW(0x03)  | LCD_BFW(0x02) );

            p_LCDRegs->LCCR3 = (LCD_PCD(PCD)  | LCD_BPP(BPP) | LCD_PCP | LCD_HSP |
                                LCD_PDFOR(pXllpLCD->PixelDataFormat));
            
            p_LCDRegs->LCCR4 = LCD_PAL_FOR(0);
            if ( (p_LCDRegs->OVL1C1 & LCD_O1EN) || (p_LCDRegs->OVL2C1 & LCD_O2EN))
            {
                p_LCDRegs->LCCR4 = LCD_PAL_FOR(1);
            }
        }
        break;

    case LM8V31: // 640x480 16bpp dual panel passive
        {

            // 
            // The actual equation requires that we take the ceiling of a floating point result.
            // Rather than use floats, we'll calculate an approximation to the correct PCD value
            // using integers.
            //
            PCD = (LCLK / (2 * LM8V31_PIXEL_CLOCK_FREQUENCY));

            // Reconfigure the upper panel frame descriptors for dual panel operation by
            // setting the DMA transfer length to half the size of the frame buffer
            pXllpLCD->frameDescriptorCh0fd1->LDCMD = pXllpLCD->FrameBufferSize >> 1;
            pXllpLCD->frameDescriptorCh0fd2->LDCMD = pXllpLCD->FrameBufferSize >> 1;

            // Configure the lower panel frame descriptor for dual panel operation.
            // Set the physical address of the frame descriptor
            pXllpLCD->frameDescriptorCh1->FDADR = LCD_FDADR(pXllpLCD->_DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_PHYSICAL);

            // Set the physical address of the frame buffer
            pXllpLCD->frameDescriptorCh1->FSADR = LCD_FSADR(pXllpLCD->_FRAME_BUFFER_BASE_PHYSICAL + pXllpLCD->CurrentPage*pXllpLCD->FrameBufferSize + (pXllpLCD->FrameBufferSize >> 1));

            // Clear the frame ID
            pXllpLCD->frameDescriptorCh1->FIDR  = LCD_FIDR(0);

            // Set the DMA transfer length to half the size of the frame buffer
            pXllpLCD->frameDescriptorCh1->LDCMD = LCD_Len(pXllpLCD->FrameBufferSize >> 1);

            // Store the physical address of this frame descriptor in the frame descriptor
            pXllpLCD->frameDescriptorCh1->PHYSADDR = pXllpLCD->frameDescriptorCh1->FDADR;
            
            // FBR1 is cleared and is not used in this implementation
            p_LCDRegs->FBR1 = 0;

            // Load the contents of FDADR1 with the physical address of this frame descriptor
            p_LCDRegs->FDADR1 = pXllpLCD->frameDescriptorCh1->FDADR;
        
            
            // Configure the TMED dithering engine
            // Use the magic number described in the EAS, 0x00AA5500;
            p_LCDRegs->TRGBR = LCD_TRS(0x00) | LCD_TGS(0x55) | LCD_TBS(0xAA);

            // Use the magic number described in the EAS, 0x0000754F;
            p_LCDRegs->TCR = LCD_TM2S | LCD_TM1S | LCD_TM2En | LCD_TM1En        |
                             LCD_TVBS(0x04) | LCD_THBS(0x05) | LCD_TSCS(0x03)   |  
                             LCD_TED;

        
            p_LCDRegs->LCCR0 = (LCD_SDS | LCD_LDM | LCD_SFM | LCD_IUM   |
                                LCD_EFM | LCD_PDD(0x01)     | LCD_BM    |
                                LCD_RDSTM | LCD_CMDIM | LCD_OUC | LCD_LDDALT);

            p_LCDRegs->LCCR1 = (LCD_PPL(0x27F)  | LCD_HSW(0x02) |
                                LCD_ELW(0x03)   | LCD_BLW(0x03) );

            p_LCDRegs->LCCR2 = (LCD_LPP(0xef)   | LCD_VSW(0x01) |
                                LCD_EFW(0x00)   | LCD_BFW(0x00) );

            p_LCDRegs->LCCR3 = (LCD_PCD(PCD)    | LCD_ACB(0xff) |
                                LCD_PCP         | LCD_BPP(BPP)  |
                                LCD_PDFOR(pXllpLCD->PixelDataFormat));

            p_LCDRegs->LCCR4 = LCD_PAL_FOR(0);
            if ( (p_LCDRegs->OVL1C1 & LCD_O1EN) || (p_LCDRegs->OVL2C1 & LCD_O2EN))
            {
                p_LCDRegs->LCCR4 = LCD_PAL_FOR(1);
            }
        }
        break;
    case LQ64D341: // 176x220 active matrix Stinger display
        {

            // 
            // The actual equation requires that we take the ceiling of a floating point result.
            // Rather than use floats, we'll calculate an approximation to the correct PCD value
            // using integers.
            //
            PCD = (LCLK / (2 * LQ64D341_PIXEL_CLOCK_FREQUENCY));

            p_LCDRegs->LCCR0 = ( LCD_LDM | LCD_SFM | LCD_IUM    |
                                 LCD_EFM | LCD_PAS | LCD_BM     |
                                 LCD_RDSTM | LCD_CMDIM | LCD_OUC | LCD_LDDALT);

            p_LCDRegs->LCCR1 = ( LCD_PPL(0xAF)  | LCD_HSW(0x02) |
                                 LCD_ELW(0x7B)  | LCD_BLW(0x03) );

            p_LCDRegs->LCCR2 = ( LCD_LPP(0xdb)  | LCD_VSW(0x01) |
                                 LCD_EFW(0x02)  | LCD_BFW(0x00) );

            p_LCDRegs->LCCR3 = ( LCD_PCD(PCD)   | LCD_BPP(BPP)   | 
                                 LCD_VSP        | LCD_HSP        | LCD_PCP  | 
                                 LCD_OEP        | LCD_PDFOR(pXllpLCD->PixelDataFormat));

            p_LCDRegs->LCCR4 = LCD_PAL_FOR(0);
            if ( (p_LCDRegs->OVL1C1 & LCD_O1EN) || (p_LCDRegs->OVL2C1 & LCD_O2EN))
            {
                p_LCDRegs->LCCR4 = LCD_PAL_FOR(1);
            }

            }
        break;

    case LS022Q8DD06: // Sharp LS022Q8DD06 Sharp 240 x 320 for ZOAR
        {
            // 
            // The actual equation requires that we take the ceiling of a floating point result.
            // Rather than use floats, we'll calculate an approximation to the correct PCD value
            // using integers.
            //

            PCD = (LCLK / (2 * LS022Q8DD06_PIXEL_CLOCK_FREQUENCY));

            // Configure the LCD Controller Control Registers
            p_LCDRegs->LCCR0 = (LCD_LDM | LCD_SFM | LCD_IUM | LCD_EFM | 
                        LCD_PAS | LCD_QDM | LCD_BM  | LCD_OUM |
                        LCD_RDSTM | LCD_CMDIM | LCD_OUC | LCD_LDDALT);

            p_LCDRegs->LCCR1 = (LCD_PPL(0xEF) | LCD_HSW(0x01) | 
                        LCD_ELW(0x00)  | LCD_BLW(0x07) );
            
            p_LCDRegs->LCCR2 = (LCD_LPP(0x13f) | LCD_VSW(0x02) |
                        LCD_EFW(0x02)  | LCD_BFW(0x00) );

            p_LCDRegs->LCCR3 = (LCD_PCD(PCD)  | LCD_BPP(BPP) | LCD_PCP | 
                        LCD_PDFOR(pXllpLCD->PixelDataFormat));
            
            p_LCDRegs->LCCR4 = LCD_PAL_FOR(0);
            if ( (p_LCDRegs->OVL1C1 & LCD_O1EN) || (p_LCDRegs->OVL2C1 & LCD_O2EN))
            {
                p_LCDRegs->LCCR4 = LCD_PAL_FOR(1);
            }

            LockID = XllpLock(CKEN);
            p_CLKRegs->cken = (p_CLKRegs->cken & XLLP_CLKEN_MASK) | CLK_SSP3;
            XllpUnlock(LockID);

            // Assert chip select on the LCD
            LockID = XllpLock(GPCR2);
            LockID2 = XllpLock(GPSR2);
            
            p_GPIORegs->GPCR2 &= ~XLLP_GPIO_BIT_L_BIAS;
            p_GPIORegs->GPSR2 |= XLLP_GPIO_BIT_L_BIAS;  
    
            
            XllpOstDelayMilliSeconds(p_OSTRegs, 1);
            p_SSPRegs->sscr0 = 0x00C01030;
            p_SSPRegs->sscr1 = 0x00008000;
            p_SSPRegs->sspsp = 0x0025000C;
            p_SSPRegs->sscr0 |= 0x8F;

            // Wait for the operation to complete
            while(p_SSPRegs->sssr & 0x10);

            for (i = 0; i < sizeof(LS022Q8DD06_DATA_SET_1) >> 1; i+=2)
            {
                p_GPIORegs->GPCR2 &= ~XLLP_GPIO_BIT_L_BIAS;
                p_GPIORegs->GPSR2 |= XLLP_GPIO_BIT_L_BIAS;  
    
                p_SSPRegs->ssdr = LS022Q8DD06_DATA_SET_1[i];
                p_SSPRegs->ssdr = LS022Q8DD06_DATA_SET_1[i+1];

                // Wait for the operation to complete
                while(p_SSPRegs->sssr & 0x10);
    
                XllpOstDelayMicroSeconds(p_OSTRegs, 50);
                p_GPIORegs->GPSR2 &= ~XLLP_GPIO_BIT_L_BIAS;
                p_GPIORegs->GPCR2 |= XLLP_GPIO_BIT_L_BIAS;

                XllpOstDelayMilliSeconds(p_OSTRegs, 100);
            }

            for (i = 0; i < sizeof(LS022Q8DD06_DATA_SET_2) >> 1; i+=2)
            {
                p_GPIORegs->GPCR2 &= ~XLLP_GPIO_BIT_L_BIAS;
                p_GPIORegs->GPSR2 |= XLLP_GPIO_BIT_L_BIAS;  
    
                p_SSPRegs->ssdr = LS022Q8DD06_DATA_SET_2[i];
                p_SSPRegs->ssdr = LS022Q8DD06_DATA_SET_2[i+1];

                // Wait for the operation to complete
                while(p_SSPRegs->sssr & 0x10);
                XllpOstDelayMicroSeconds(p_OSTRegs, 50);
                p_GPIORegs->GPSR2 &= ~XLLP_GPIO_BIT_L_BIAS;
                p_GPIORegs->GPCR2 |= XLLP_GPIO_BIT_L_BIAS;
            
                XllpOstDelayMilliSeconds(p_OSTRegs, 100);
            }


            // De-assert chip select on the LCD
            p_GPIORegs->GPSR2 &= ~XLLP_GPIO_BIT_L_BIAS;
            p_GPIORegs->GPCR2 |= XLLP_GPIO_BIT_L_BIAS;
            XllpUnlock(LockID);
            XllpUnlock(LockID2);

        }
        break;

    default:
        {

        }
        break;
    }

}


void LCDClearStatusReg(P_XLLP_LCD_T pXllpLCD)
{
    volatile LCDRegs *p_LCDRegs;
    p_LCDRegs = (LCDRegs *) pXllpLCD->LCDC;

    // Clear the status registers by writing 1's to each bit.
    p_LCDRegs->LCSR0 =  ( LCD_LDD | LCD_SOF0| LCD_BER | LCD_ABC | LCD_IU0   |
                          LCD_IU1 | LCD_OU  | LCD_QD  | LCD_EOF0| LCD_BS0   | 
                          LCD_SINT| LCD_RD_ST | LCD_CMD_INTR );

    p_LCDRegs->LCSR1 =  ( LCD_SOF1| LCD_SOF2| LCD_SOF3| LCD_SOF4| LCD_SOF5  | LCD_SOF6  |
                          LCD_EOF1| LCD_EOF2| LCD_EOF3| LCD_EOF4| LCD_EOF5  | LCD_EOF6  |
                          LCD_BS1 | LCD_BS2 | LCD_BS3 | LCD_BS4 | LCD_BS5   | LCD_BS6   |
                                    LCD_IU2 | LCD_IU3 | LCD_IU4 | LCD_IU5   | LCD_IU6 );

}

void LCDSetupGPIOs(P_XLLP_LCD_T pXllpLCD)
{
    XLLP_UINT32_T LockID;
    volatile XLLP_GPIO_T *p_GPIORegs;

    p_GPIORegs = (XLLP_GPIO_T *) pXllpLCD->GPIO;

    LockID = XllpLock(GPDR0);

    if (pXllpLCD->DisplayType != LS022Q8DD06)
    {
        p_GPIORegs->GPDR0 = (p_GPIORegs->GPDR0 & ~XLLP_GPIO_BIT_L_VSYNC) | (XLLP_GPIO_BIT_PWM_OUT0 | XLLP_GPIO_BIT_L_CS);
    }

    XllpUnlock(LockID);

    LockID = XllpLock(GPDR1);
    p_GPIORegs->GPDR1 |= ( XLLP_GPIO_BIT_L_DD0 | XLLP_GPIO_BIT_L_DD1 | XLLP_GPIO_BIT_L_DD2 | XLLP_GPIO_BIT_L_DD3 | XLLP_GPIO_BIT_L_DD4 | XLLP_GPIO_BIT_L_DD5);

    // Set GPIO 38 and 40 as outputs
    #if defined XLLP_GPIO_BIT_SSPTXD3 && XLLP_GPIO_BIT_SSPCLK3
        if (pXllpLCD->DisplayType == LS022Q8DD06)
        {
            p_GPIORegs->GPDR1 |= (XLLP_GPIO_BIT_SSPTXD3 | XLLP_GPIO_BIT_SSPCLK3);
        }
    #endif

    XllpUnlock(LockID);

    LockID = XllpLock(GPDR2);

    if (pXllpLCD->DisplayType != LS022Q8DD06)
    {
        p_GPIORegs->GPDR2 |= (  XLLP_GPIO_BIT_L_DD6 | XLLP_GPIO_BIT_L_DD7 | XLLP_GPIO_BIT_L_DD8 | XLLP_GPIO_BIT_L_DD9 | XLLP_GPIO_BIT_L_DD10 | 
                            XLLP_GPIO_BIT_L_DD11 | XLLP_GPIO_BIT_L_DD12 | XLLP_GPIO_BIT_L_DD13 | XLLP_GPIO_BIT_L_DD14 | XLLP_GPIO_BIT_L_DD15 |
                            XLLP_GPIO_BIT_L_FCLK | XLLP_GPIO_BIT_L_LCLK | XLLP_GPIO_BIT_L_PCLK | XLLP_GPIO_BIT_L_BIAS | XLLP_GPIO_BIT_L_DD16 | 
                            XLLP_GPIO_BIT_L_DD17);
    } else 
    {
        p_GPIORegs->GPDR2 |= (  XLLP_GPIO_BIT_L_DD6 | XLLP_GPIO_BIT_L_DD7 | XLLP_GPIO_BIT_L_DD8 | XLLP_GPIO_BIT_L_DD9 | XLLP_GPIO_BIT_L_DD10 | 
                            XLLP_GPIO_BIT_L_DD11 | XLLP_GPIO_BIT_L_DD12 | XLLP_GPIO_BIT_L_DD13 | XLLP_GPIO_BIT_L_DD14 | XLLP_GPIO_BIT_L_DD15 |
                            XLLP_GPIO_BIT_L_FCLK | XLLP_GPIO_BIT_L_LCLK | XLLP_GPIO_BIT_L_PCLK | XLLP_GPIO_BIT_L_BIAS );

        #if defined XLLP_GPIO_BIT_SSPRXD3
            // Set GPIO 89 as input     
            p_GPIORegs->GPDR2 &= ~(XLLP_GPIO_BIT_SSPRXD3);
        #endif
    }
    
    XllpUnlock(LockID);

    if (pXllpLCD->DisplayType != LS022Q8DD06)
    {
        // Program the GAFR0_L to select alternate function 1 for GPIO 14.
        LockID = XllpLock(GAFR0_L);
        p_GPIORegs->GAFR0_L = (p_GPIORegs->GAFR0_L & ~XLLP_GPIO_AF_BIT_L_VSYNC_MASK) | (XLLP_GPIO_AF_BIT_L_VSYNC);
        XllpUnlock(LockID);

        // Program the GAFR0_U to select alternate function 2 for GPIO 19.
        LockID = XllpLock(GAFR0_U);
        p_GPIORegs->GAFR0_U = (p_GPIORegs->GAFR0_U & ~XLLP_GPIO_AF_BIT_L_CS_MASK) | (XLLP_GPIO_AF_BIT_L_CS);
        XllpUnlock(LockID);
    }

    if (pXllpLCD->DisplayType == LS022Q8DD06)
    {
        #if defined XLLP_GPIO_AF_BIT_SSPTXD3_MASK && XLLP_GPIO_AF_BIT_SSPCLK3_MASK && XLLP_GPIO_AF_BIT_SSPTXD3 && XLLP_GPIO_AF_BIT_SSPCLK3
            // Program the GAFR1_L to select alternate function 1 for GPIO 38, 40.
            LockID = XllpLock(GAFR1_L);
            p_GPIORegs->GAFR1_L = (p_GPIORegs->GAFR1_L & ~(XLLP_GPIO_AF_BIT_SSPTXD3_MASK | XLLP_GPIO_AF_BIT_SSPCLK3_MASK)) | 
                    (XLLP_GPIO_AF_BIT_SSPTXD3 | XLLP_GPIO_AF_BIT_SSPCLK3);
            XllpUnlock(LockID);
        #endif
    }

    // Program the GAFR1_U to select alternate function 2 for GPIO 58 through 63.
    LockID = XllpLock(GAFR1_U);
    p_GPIORegs->GAFR1_U = (p_GPIORegs->GAFR1_U & ~(XLLP_GPIO_AF_BIT_L_DD0_MASK | XLLP_GPIO_AF_BIT_L_DD1_MASK | XLLP_GPIO_AF_BIT_L_DD2_MASK|
                                                   XLLP_GPIO_AF_BIT_L_DD3_MASK | XLLP_GPIO_AF_BIT_L_DD4_MASK | XLLP_GPIO_AF_BIT_L_DD5_MASK)) | 
                                                  (XLLP_GPIO_AF_BIT_L_DD0 | XLLP_GPIO_AF_BIT_L_DD1 | XLLP_GPIO_AF_BIT_L_DD2 | 
                                                   XLLP_GPIO_AF_BIT_L_DD3 | XLLP_GPIO_AF_BIT_L_DD4 | XLLP_GPIO_AF_BIT_L_DD5 );
    XllpUnlock(LockID);

    // Program the GAFR2_L to select alternate function 2 for GPIO 64 through 77.
    LockID = XllpLock(GAFR2_L);
    p_GPIORegs->GAFR2_L = (p_GPIORegs->GAFR2_L & ~(XLLP_GPIO_AF_BIT_L_DD6_MASK  | XLLP_GPIO_AF_BIT_L_DD7_MASK   | XLLP_GPIO_AF_BIT_L_DD8_MASK   |
                                                   XLLP_GPIO_AF_BIT_L_DD9_MASK  | XLLP_GPIO_AF_BIT_L_DD10_MASK  | XLLP_GPIO_AF_BIT_L_DD11_MASK  |
                                                   XLLP_GPIO_AF_BIT_L_DD12_MASK | XLLP_GPIO_AF_BIT_L_DD13_MASK  | XLLP_GPIO_AF_BIT_L_DD14_MASK  |
                                                   XLLP_GPIO_AF_BIT_L_DD15_MASK | XLLP_GPIO_AF_BIT_L_FCLK_RD_MASK | XLLP_GPIO_AF_BIT_L_LCLK_A0_MASK |
                                                   XLLP_GPIO_AF_BIT_L_PCLK_WR_MASK | XLLP_GPIO_AF_BIT_L_BIAS_MASK)) |
                                                  (XLLP_GPIO_AF_BIT_L_DD6   | XLLP_GPIO_AF_BIT_L_DD7    | XLLP_GPIO_AF_BIT_L_DD8    | 
                                                   XLLP_GPIO_AF_BIT_L_DD9   | XLLP_GPIO_AF_BIT_L_DD10   | XLLP_GPIO_AF_BIT_L_DD11   | 
                                                   XLLP_GPIO_AF_BIT_L_DD12  | XLLP_GPIO_AF_BIT_L_DD13   | XLLP_GPIO_AF_BIT_L_DD14   | 
                                                   XLLP_GPIO_AF_BIT_L_DD15  | XLLP_GPIO_AF_BIT_L_FCLK_RD| XLLP_GPIO_AF_BIT_L_LCLK_A0|
                                                   XLLP_GPIO_AF_BIT_L_PCLK_WR   | XLLP_GPIO_AF_BIT_L_BIAS ); 
    if (pXllpLCD->DisplayType == LS022Q8DD06)
    {
        p_GPIORegs->GAFR2_L = (p_GPIORegs->GAFR2_L & ~XLLP_GPIO_AF_BIT_L_BIAS_MASK); 
    }

    XllpUnlock(LockID);


    if (pXllpLCD->DisplayType != LS022Q8DD06)
    {
        // Program the GAFR2_U to select alternate function 2 for GPIO 86, 87.
        LockID = XllpLock(GAFR2_U);
        p_GPIORegs->GAFR2_U = (p_GPIORegs->GAFR2_U & ~(XLLP_GPIO_AF_BIT_L_DD16_MASK | XLLP_GPIO_AF_BIT_L_DD17_MASK))  | (XLLP_GPIO_AF_BIT_L_DD16 | XLLP_GPIO_AF_BIT_L_DD17); 
        XllpUnlock(LockID);
    }
        
    if (pXllpLCD->DisplayType == LS022Q8DD06)
    {
        LockID = XllpLock(GAFR2_U);
        #if defined XLLP_GPIO_AF_BIT_SSPRXD3_MASK && XLLP_GPIO_AF_BIT_SSPRXD3
            p_GPIORegs->GAFR2_U = (p_GPIORegs->GAFR2_U & ~XLLP_GPIO_AF_BIT_SSPRXD3_MASK)  | XLLP_GPIO_AF_BIT_SSPRXD3; 
        #endif
        XllpUnlock(LockID);
    }

    

    if (pXllpLCD->DisplayType != LS022Q8DD06)
    {
        LockID = XllpLock(GPSR0);
        // Turn on the backlight...
        p_GPIORegs->GPSR0 |= XLLP_GPIO_BIT_PWM_OUT0;
        XllpUnlock(LockID);
    }

    if (pXllpLCD->DisplayType == LS022Q8DD06)
    {
        p_GPIORegs->GPSR2 &= ~XLLP_GPIO_BIT_L_BIAS;
        p_GPIORegs->GPCR2 |= XLLP_GPIO_BIT_L_BIAS;
    }

}

void LCDEnableController(P_XLLP_LCD_T pXllpLCD)
{
    volatile LCDRegs *p_LCDRegs;

    p_LCDRegs = (LCDRegs *) pXllpLCD->LCDC;

    p_LCDRegs->LCCR0 |= LCD_ENB;
}


XLLP_STATUS_T XllpLCD_Overlay2_Enable(P_XLLP_LCD_T pXllpLCD, P_XLLP_OVERLAY_T pXllpOverlay)
{
    XLLP_STATUS_T status = 0;
    
    volatile LCDRegs *p_LCDRegs;

    p_LCDRegs = (LCDRegs *) pXllpLCD->LCDC; 

    // Set the physical address of the frame descriptor
    pXllpLCD->frameDescriptorCh2_YCbCr_Y->FDADR =  LCD_FDADR(pXllpLCD->_DMA_CHANNEL_2_Y_FRAME_DESCRIPTOR_BASE_PHYSICAL);
    pXllpLCD->frameDescriptorCh3_YCbCr_Cb->FDADR = LCD_FDADR(pXllpLCD->_DMA_CHANNEL_3_Cb_FRAME_DESCRIPTOR_BASE_PHYSICAL);
    pXllpLCD->frameDescriptorCh4_YCbCr_Cr->FDADR = LCD_FDADR(pXllpLCD->_DMA_CHANNEL_4_Cr_FRAME_DESCRIPTOR_BASE_PHYSICAL);

    // Set the physical address of the frame buffer
    pXllpLCD->frameDescriptorCh2_YCbCr_Y->FSADR =  LCD_FSADR(pXllpLCD->_OVERLAY2_Y_CHANNEL_BASE_PHYSICAL);
    pXllpLCD->frameDescriptorCh3_YCbCr_Cb->FSADR = LCD_FSADR(pXllpLCD->_OVERLAY2_Cb_CHANNEL_BASE_PHYSICAL);
    pXllpLCD->frameDescriptorCh4_YCbCr_Cr->FSADR = LCD_FSADR(pXllpLCD->_OVERLAY2_Cr_CHANNEL_BASE_PHYSICAL);
    
    // Clear the frame ID
    pXllpLCD->frameDescriptorCh2_YCbCr_Y->FIDR  = LCD_FIDR(0);
    pXllpLCD->frameDescriptorCh3_YCbCr_Cb->FIDR  = LCD_FIDR(0);
    pXllpLCD->frameDescriptorCh4_YCbCr_Cr->FIDR  = LCD_FIDR(0);

    // Set the DMA transfer size (calculated already by XllpLCD_DMALength())
    pXllpLCD->frameDescriptorCh2_YCbCr_Y->LDCMD = LCD_Len(pXllpOverlay->ch2_size);
    pXllpLCD->frameDescriptorCh3_YCbCr_Cb->LDCMD = LCD_Len(pXllpOverlay->ch3_size);
    pXllpLCD->frameDescriptorCh4_YCbCr_Cr->LDCMD = LCD_Len(pXllpOverlay->ch4_size);

    // Store the physical address of each frame descriptor in the frame descriptor
    pXllpLCD->frameDescriptorCh2_YCbCr_Y->PHYSADDR = pXllpLCD->frameDescriptorCh2_YCbCr_Y->FDADR;
    pXllpLCD->frameDescriptorCh3_YCbCr_Cb->PHYSADDR = pXllpLCD->frameDescriptorCh3_YCbCr_Cb->FDADR;
    pXllpLCD->frameDescriptorCh4_YCbCr_Cr->PHYSADDR = pXllpLCD->frameDescriptorCh4_YCbCr_Cr->FDADR;
    
    // FBRx is cleared and is not used.
    p_LCDRegs->FBR2 = 0;
    p_LCDRegs->FBR3 = 0;
    p_LCDRegs->FBR4 = 0;

    // Load the contents of FDADRx with the physical address of this frame descriptor
    p_LCDRegs->FDADR2 = LCD_FDADR(pXllpLCD->frameDescriptorCh2_YCbCr_Y->FDADR);
    p_LCDRegs->FDADR3 = LCD_FDADR(pXllpLCD->frameDescriptorCh3_YCbCr_Cb->FDADR);
    p_LCDRegs->FDADR4 = LCD_FDADR(pXllpLCD->frameDescriptorCh4_YCbCr_Cr->FDADR);
    

    // Reinit the LCD controller so that the pixel data format can be updated for use with overlays
    XllpLCDSuspend(pXllpLCD, Suspend_Graceful);

    pXllpOverlay->TmpBPP = pXllpLCD->BPP;
    if (pXllpOverlay->DegradeBaseFrame)
    {
        pXllpLCD->BPP = BPP_1;
    }
    
    pXllpLCD->PixelDataFormat = PDFOR_11;

    // Configure the overlay registers and enable the overlay
    p_LCDRegs->OVL2C2 = (LCD_FOR(pXllpOverlay->Format) | LCD_O2YPOS(pXllpOverlay->Y_Position) | LCD_O2XPOS(pXllpOverlay->X_Position));
    p_LCDRegs->OVL2C1 = (LCD_O2EN | LCD_BPP2(pXllpOverlay->OverlayBPP) | LCD_LPO2(pXllpOverlay->OverlayHeight-1) | LCD_PPL2(pXllpOverlay->OverlayWidth-1));

    XllpLCDResume(pXllpLCD);

    return status;
}

void XllpLCD_Overlay2_Disable(P_XLLP_LCD_T pXllpLCD, P_XLLP_OVERLAY_T pXllpOverlay)
{
    volatile LCDRegs *p_LCDRegs;
    
    p_LCDRegs = (LCDRegs *) pXllpLCD->LCDC; 

    // Reinit the LCD controller so that the pixel data format can be updated for use without overlays
    XllpLCDSuspend(pXllpLCD, Suspend_Graceful);

    pXllpLCD->PixelDataFormat = PDFOR_00;

    // Undo the degraded base frame - always.
    pXllpLCD->BPP = pXllpOverlay->TmpBPP;

    p_LCDRegs->OVL2C1 &= 0x00FFFFFF; // Clear the enable bit, and clear the reserved bits 30:24.
    XllpLCDResume(pXllpLCD);
}

void XllpLCD_DMALength(P_XLLP_OVERLAY_T pXllpOverlay)
{
    unsigned int pixels = pXllpOverlay->OverlayHeight * pXllpOverlay->OverlayWidth;
    unsigned int DMALength = 0;

    // Determine the DMA transfer length
    // each DMA transfer length for YUV formatted data must be multiples of 32-bits and adjusted accordingly
    if (pXllpOverlay->Format == FORMAT_RGB)
    {
        switch(pXllpOverlay->OverlayBPP)
        {
            case O_BPP_4:
                DMALength = pixels >> 1;
                break;
            case O_BPP_8:
                DMALength = pixels;
                break;
            case O_BPP_16:
                DMALength = pixels << 1;
                break;
            case O_BPP_18:
                DMALength = pixels << 2;
                break;
            case O_BPP_18_PACKED:
                break;
            case O_BPP_19:
                DMALength = pixels << 2;
                break;
            case O_BPP_19_PACKED:
                break;
            case O_BPP_24:
                DMALength = pixels << 2;
                break;
            case O_BPP_25:
                DMALength = pixels << 2;
                break;
        default:
            break;
        }
        pXllpOverlay->ch2_size = DMALength;
        pXllpOverlay->ch3_size = 0;
        pXllpOverlay->ch4_size = 0;
    }
    if (pXllpOverlay->Format == FORMAT_PACKED_444)
    {
        pXllpOverlay->ch2_size = (pixels << 2);
        pXllpOverlay->ch3_size = 0;
        pXllpOverlay->ch4_size = 0;
    } 
    else if (pXllpOverlay->Format == FORMAT_PLANAR_444) 
    {
        // calculate the number of bits in the frame (pixels << 3)
        // mod by 32 to determine the remainder
        // subtract from 32 to determine how many bits to add to the length to make it a multiple of 32 bits
        // add this value to the number of bits in the frame
        // convert this value back to bytes
        DMALength = pixels;
        if ((DMALength % 4) > 0)
        {
            DMALength = (((32 - ((pixels << 3) % 32)) + (pixels << 3)) >> 3);  // 24 bits total
        }
        pXllpOverlay->ch2_size = DMALength;
        pXllpOverlay->ch3_size = DMALength;
        pXllpOverlay->ch4_size = DMALength;
    }
    else if (pXllpOverlay->Format == FORMAT_PLANAR_422)
    {                                                                           
        DMALength = pixels;
        if ((DMALength % 4) > 0)
        {
            DMALength = (((32 - ((pixels << 3) % 32)) + (pixels << 3)) >> 3);   // 16 bits total
        }
        pXllpOverlay->ch2_size = DMALength;

        DMALength = pixels >> 1;
        if (((pixels << 2) % 32) > 0)
        {
            DMALength = (((32 - ((pixels << 2) % 32)) + (pixels << 2)) >> 3);
        }
        pXllpOverlay->ch3_size = DMALength;
        pXllpOverlay->ch4_size = DMALength;
    }
    else if (pXllpOverlay->Format == FORMAT_PLANAR_420)
    {
        DMALength = pixels;
        if ((DMALength % 4) > 0)
        {
            DMALength = (((32 - ((pixels << 3) % 32)) + (pixels << 3)) >> 3);   // 12 bits total
        }
        pXllpOverlay->ch2_size = DMALength;

        DMALength = pixels >> 2;
        if (((pixels << 1) % 32) > 0)
        {
            DMALength = (((32 - ((pixels << 1) % 32)) + (pixels << 1)) >> 3);
        }
        pXllpOverlay->ch3_size = DMALength;
        pXllpOverlay->ch4_size = DMALength;
    }

}

