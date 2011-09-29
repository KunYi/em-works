//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  File:  debug.c
//
//  This file implements bootloader functions related to serial debug output.
//  
#include <windows.h>
#include <shx.h>
#include <oal.h>
#include <nkintr.h>

//------------------------------------------------------------------------------
//
#define SERIAL_ERROR_TIMEOUT    5000

//------------------------------------------------------------------------------
//  Caculate Baud Rate Register Settings
//
#define CEIL(x) ((x > (float)(int)x) ? ((int)x + 1) : ((int)x))
#define BITRATEREGISTERVAL(baud,sysClock) (CEIL(((float)sysClock / (32 * baud)) - 1))

//------------------------------------------------------------------------------
VOID DumpRegisters();


//------------------------------------------------------------------------------
//
//  Function:  OEMInitDebugSerial
//  
//  This function initializes the debug serial port on the target device.
//
void OEMInitDebugSerial()
{
    UINT8           brrVal;
    SH4_SCIF_REGS   *pSCIFRegs = OALPAtoUA(SH4_REG_PA_SCIF);

    // Disable Clock, Transmit, and Receive
    OUTREG16(&pSCIFRegs->SCSCR2, 0x0000);

    // Reset FIFO Transmit and Receive
    OUTREG16(&pSCIFRegs->SCFCR2, 0x0006);

    // Clear any errors due to FIFO reset
    OUTREG16(&pSCIFRegs->SCSCR2, 0x0000);

    // Set SCIF mode to be 8-Bit Data, Parity Disabled, 1 Stop Bit, No Baud Divisor
    OUTREG16(&pSCIFRegs->SCSMR2, 0x0000);

    brrVal = BITRATEREGISTERVAL(SERIAL_BAUD_RATE, PERIPHERAL_CLOCK_FREQ);
    OUTREG8(&pSCIFRegs->SCBRR2, brrVal);

    // Disable FIFO Error Resets for Transmit and Receive
    OUTREG16(&pSCIFRegs->SCFCR2, 0x0000);	

    // Clear all errors and data status bits
    OUTREG16(&pSCIFRegs->SCFSR2, 0x0000);

    // Enable Transmit and Receive
    OUTREG16(&pSCIFRegs->SCSCR2, 0x0030);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMWriteDebugByte
//
//  Write byte to debug serial port
//
void OEMWriteDebugByte(BYTE ch)
{
    UINT16          statusVal;
    UINT32          timeVal    = 0;
    SH4_SCIF_REGS   *pSCIFRegs = OALPAtoUA(SH4_REG_PA_SCIF);

    // Wait for data
    while(!(INREG16(&pSCIFRegs->SCFSR2) & SCIF_SCFSR2_TDFE))
    {
        if(timeVal++ >= SERIAL_ERROR_TIMEOUT)
        {
            OEMClearDebugCommError();
            return;
        }
    }

    // Send Data
    OUTREG8(&pSCIFRegs->SCFTDR2, ch);

    // Clear transmit buffer not-empty bit
    statusVal =  INREG16(&pSCIFRegs->SCFSR2);
    statusVal &= ~SCIF_SCFSR2_TDFE;
    OUTREG16(&pSCIFRegs->SCFSR2, statusVal);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMWriteDebugString
//
//  Output unicode string to debug serial port
//
void OEMWriteDebugString(unsigned short *string)
{
    while((*string) != L'\0')
    {
        OEMWriteDebugByte((BYTE)(*string)); 
        string++;
    }
}


//------------------------------------------------------------------------------
//
//  Function:  OEMReadDebugByte
//
//  Input character/byte from debug serial port
//
int OEMReadDebugByte()
{
    UINT8           data        = OEM_DEBUG_READ_NODATA;
    UINT16          statusVal;
    UINT16          lineVal;
    SH4_SCIF_REGS   *pSCIFRegs  = OALPAtoUA(SH4_REG_PA_SCIF);

    // Get serial status
    statusVal =  INREG16(&pSCIFRegs->SCFSR2);
    statusVal &= (SCIF_SCFSR2_DR  | SCIF_SCFSR2_ER | SCIF_SCFSR2_BRK |
                  SCIF_SCFSR2_FER | SCIF_SCFSR2_PER);

    // Get line status
    lineVal =  INREG16(&pSCIFRegs->SCLSR2);
    lineVal &= SCIF_SCLSR2_ORER;

    // Check to see if there were any communication errors
    if(!(statusVal || lineVal)) 
    {
        // Check for data in receive buffer
        statusVal = INREG16(&pSCIFRegs->SCFSR2) & SCIF_SCFSR2_RDF;
        if(statusVal)
        {
            // Get Data
            data = INREG8(&pSCIFRegs->SCFRDR2);

            // Clear receive buffer not-empty bit
            statusVal =  INREG16(&pSCIFRegs->SCFSR2); 
            statusVal &= ~SCIF_SCFSR2_RDF;
            OUTREG16(&pSCIFRegs->SCFSR2, statusVal);
        }
    }
    else
    {
        OEMClearDebugCommError();
    }

    return (int)data;
}


//------------------------------------------------------------------------------
//
//  Function:     OEMClearDebugCommError
//
//  Clear debug serial port error
//
void OEMClearDebugCommError()
{
    UINT16          statusVal;
    UINT16          lineVal;
    SH4_SCIF_REGS   *pSCIFRegs  = OALPAtoUA(SH4_REG_PA_SCIF);

    // Clear errors on serial port
    statusVal =  INREG16(&pSCIFRegs->SCFSR2);
    statusVal &= ~(SCIF_SCFSR2_ER  | SCIF_SCFSR2_BRK | SCIF_SCFSR2_DR |
                   SCIF_SCFSR2_PER | SCIF_SCFSR2_FER);
    OUTREG16(&pSCIFRegs->SCFSR2, statusVal);

    // Clear overrun error
    lineVal =  INREG16(&pSCIFRegs->SCLSR2);
    lineVal &= ~SCIF_SCLSR2_ORER;
    OUTREG16(&pSCIFRegs->SCLSR2, lineVal);
}


//------------------------------------------------------------------------------
// This function assumes that serial debug spew is up and running
//
VOID DumpRegisters()
{
    SH4_TMU_REGS    *pTMURegs   = OALPAtoUA(SH4_REG_PA_TMU);
    SH4_RTC_REGS    *pRTCRegs   = OALPAtoUA(SH4_REG_PA_RTC);
    SH4_SCI_REGS    *pSCIRegs   = OALPAtoUA(SH4_REG_PA_SCI);
    SH4_BSC_REGS    *pBSCRegs   = OALPAtoUA(SH4_REG_PA_BSC);
    SH4_CCN_REGS    *pCCNRegs   = OALPAtoUA(SH4_REG_PA_CCN);
    SH4_CPG_REGS    *pCPGRegs   = OALPAtoUA(SH4_REG_PA_CPG);
    SH4_DMAC_REGS   *pDMACRegs  = OALPAtoUA(SH4_REG_PA_DMAC);
    SH4_SCIF_REGS   *pSCIFRegs  = OALPAtoUA(SH4_REG_PA_SCIF);
    SH4_INTC_REGS   *pINTCRegs  = OALPAtoUA(SH4_REG_PA_INTC);

    // Print out the control register for all of the major on chip peripherals
    OALLog(L"-------------------------\r\n");
    OALLog(L"CCN MMUCR:     0x%08x\r\n", INREG32(&pCCNRegs->MMUCR));
    OALLog(L"CCN CCR:       0x%08x\r\n", INREG32(&pCCNRegs->CCR));
    OALLog(L"-------------------------\r\n");
    OALLog(L"TMU TOCR:      0x%08x\r\n", INREG8(&pTMURegs->TOCR));
    OALLog(L"TMU TSTR:      0x%08x\r\n", INREG8(&pTMURegs->TSTR));
    OALLog(L"-------------------------\r\n");
    OALLog(L"CPG FRQCR:     0x%08x\r\n", INREG16(&pCPGRegs->FRQCR));
    OALLog(L"CPG STBCR:     0x%08x\r\n", INREG8(&pCPGRegs->STBCR));
    OALLog(L"CPG STBCR2:    0x%08x\r\n", INREG8(&pCPGRegs->STBCR2));
    OALLog(L"CPG WTCSR:     0x%08x\r\n", INREG8(&pCPGRegs->WTCSR_READ));
    OALLog(L"-------------------------\r\n");
    OALLog(L"RTC RCR1:      0x%08x\r\n", INREG8(&pRTCRegs->RCR1));
    OALLog(L"RTC RCR2:      0x%08x\r\n", INREG8(&pRTCRegs->RCR2));
    OALLog(L"-------------------------\r\n");
    OALLog(L"BSC BCR1:      0x%08x\r\n", INREG32(&pBSCRegs->BCR1));
    OALLog(L"BSC BCR2:      0x%08x\r\n", INREG16(&pBSCRegs->BCR2));
    OALLog(L"BSC WCR1:      0x%08x\r\n", INREG32(&pBSCRegs->WCR1));
    OALLog(L"BSC WCR2:      0x%08x\r\n", INREG32(&pBSCRegs->WCR2));
    OALLog(L"BSC WCR3:      0x%08x\r\n", INREG32(&pBSCRegs->WCR3));
    OALLog(L"BSC MCR:       0x%08x\r\n", INREG32(&pBSCRegs->MCR));
    OALLog(L"BSC PCR:       0x%08x\r\n", INREG32(&pBSCRegs->PCR));
    OALLog(L"BSC RTCSR:     0x%08x\r\n", INREG16(&pBSCRegs->RTCSR));
    OALLog(L"-------------------------\r\n");
    OALLog(L"DMAC DMAOR:    0x%08x\r\n", INREG32(&pDMACRegs->DMAOR));
    OALLog(L"DMAC CHCR1:    0x%08x\r\n", INREG32(&pDMACRegs->CHCR1));
    OALLog(L"DMAC CHCR2:    0x%08x\r\n", INREG32(&pDMACRegs->CHCR2));
    OALLog(L"DMAC CHCR3:    0x%08x\r\n", INREG32(&pDMACRegs->CHCR3));
    OALLog(L"-------------------------\r\n");
    OALLog(L"SCI SCSCR1:    0x%08x\r\n", INREG8(&pSCIRegs->SCSCR1));
    OALLog(L"SCI SCBRR1:    0x%08x\r\n", INREG8(&pSCIRegs->SCBRR1));
    OALLog(L"SCIF SCSCR2:   0x%08x\r\n", INREG8(&pSCIFRegs->SCSCR2));
    OALLog(L"SCIF SCBRR2:   0x%08x\r\n", INREG8(&pSCIFRegs->SCBRR2));
    OALLog(L"-------------------------\r\n");
    OALLog(L"INTC ICR:      0x%08x\r\n", INREG16(&pINTCRegs->ICR));
    OALLog(L"INTC IPRA:     0x%08x\r\n", INREG16(&pINTCRegs->IPRA));
    OALLog(L"INTC IPRB:     0x%08x\r\n", INREG16(&pINTCRegs->IPRB));
    OALLog(L"INTC IPRC:     0x%08x\r\n", INREG16(&pINTCRegs->IPRC));
    OALLog(L"-------------------------\r\n");
}

//------------------------------------------------------------------------------

