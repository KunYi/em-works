//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  simhw.c
//
//   This file implements the device specific functions for sim
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#include <nkintr.h>
#pragma warning(pop)
#include <devload.h>
#pragma warning(push)
#pragma warning(disable: 4201)
#pragma warning(disable: 4214)
#include <smclib.h>
#include <winsmcrd.h>
#pragma warning(pop)
#include "simcb.h"
#include "simhw.h"


//------------------------------------------------------------------------------
// External Functions

extern UINT32 BSPGetSIMCLK(DWORD dwIndex);
extern BOOL BSPSetSIMClockGatingMode(DWORD dwIndex, BOOL startClocks);
extern void BSPSimIomuxSetPin(DWORD dwIndex,DWORD dwPort);
extern void BSPSimSelect3Volts(DWORD dwIndex,DWORD dwPort);
extern DWORD CspSIMGetIRQs(DWORD dwIndex,DWORD* irqArray,DWORD* pdwSize);

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

#define FACTOR_NUMBER       16


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

static BOOL g_fPort0IsOK = FALSE;

//Clock rate factor (ISO 7816 part3)
static const ClockRateFactor g_crf[FACTOR_NUMBER] = {
    {0x0, 372, 4},
    {0x1, 372, 5},
    {0x2, 558, 6},
    {0x3, 744, 8},
    {0x4, 1116, 12},
    {0x5, 1488, 16},
    {0x6, 1860, 20},
    {0x7, RFU, RFU},
    {0x8, RFU, RFU},
    {0x9, 512, 5},
    {0xa, 768, 7},
    {0xb, 1024, 10},
    {0xc, 1536, 15},
    {0xd, 2048, 20},
    {0xe, RFU, RFU},
    {0xf, RFU, RFU}
};

//Baud Rate factor (ISO 7816 part3)
static const BaudRateFactor g_brf[FACTOR_NUMBER] = {
    {0x0, RFU},
    {0x1, 1},
    {0x2, 2},
    {0x3, 4},
    {0x4, 8},
    {0x5, 16},
    {0x6, 32},
    {0x7, RFU},
    {0x8, 12},
    {0x9, 20},
    {0xa, RFU},
    {0xb, RFU},
    {0xc, RFU},
    {0xd, RFU},
    {0xe, RFU},
    {0xf, RFU}
};

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

static void SIM_MaskInterrupts(PCSP_SIM_REG pSIMReg);
void Dump_Registers(PCSP_SIM_REG pSIMReg);


//-----------------------------------------------------------------------------
//
// Function: SIM_InternalMapRegisterAddresses
//
// This function maps the virtual addresses for SIM registers
//
// Parameters:
//      iobase
//          [in]Physical iobase address
//
// Returns:
//      PVOID pointer of mapped SIM register structure
//
//-----------------------------------------------------------------------------
PVOID SIM_InternalMapRegisterAddresses(DWORD iobase)
{
    PCSP_SIM_REG pSIMReg;
    PHYSICAL_ADDRESS phyAddr;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_InternalMapRegisterAddresses\n") ));

    phyAddr.QuadPart = iobase;

    // Map peripheral physical address to virtual address
    pSIMReg = (PCSP_SIM_REG) MmMapIoSpace(phyAddr, sizeof(CSP_SIM_REG), FALSE);

    // Check if virtual mapping failed
    if (pSIMReg == NULL)
    {
        SmartcardDebug( DEBUG_ERROR,( TEXT("SIM_InternalMapRegisterAddresses Failed!\n")));
    }

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_InternalMapRegisterAddresses\n") ));
    return(pSIMReg);
}

//-----------------------------------------------------------------------------
//
// Function: SIM_InternalUnMapRegisterAddresses
//
// This function unmaps the virtual addresses for SIM registers
//
// Parameters:
//      pSIMReg
//          [in]pointer of mapped SIM register structure
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID SIM_InternalUnMapRegisterAddresses(PCSP_SIM_REG pSIMReg)
{
    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_InternalUnMapRegisterAddresses\n") ));

    // Check if virtual address is valid
    if (pSIMReg != NULL)
    {
        MmUnmapIoSpace(pSIMReg, sizeof(PCSP_SIM_REG));
        pSIMReg = NULL;        
    }

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_InternalUnMapRegisterAddresses\n") ));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: SIM_Init
//
// This function initializes the SIM module
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
void SIM_Init(PREADER_EXTENSION pReaderExtension)
{
    DWORD dwIrqs[4];
    DWORD dwSize;
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_Init\n") ));

    BSPSimIomuxSetPin(pReaderExtension->dwIndex,pReaderExtension->dwPort);
    //Change the Power Supply to SIM to 3Volts
    BSPSimSelect3Volts(pReaderExtension->dwIndex,pReaderExtension->dwPort);

    BSPSetSIMClockGatingMode(pReaderExtension->dwIndex, TRUE);

    //mask all interrupts
    SIM_MaskInterrupts(pReaderExtension->pSIMReg);

    // Create SIM interrupt event
    pReaderExtension->hIntrEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (pReaderExtension->hIntrEvent == NULL)
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to create event for SIM interrupt.\r\n")));
        return;
    }

    memset(dwIrqs,0,sizeof(dwIrqs));
    dwIrqs[0] = (UINT32)-1;
    dwIrqs[1] = 0;
    dwSize = sizeof(dwIrqs)-3*sizeof(DWORD);
    if (CspSIMGetIRQs(pReaderExtension->dwIndex,&dwIrqs[2],&dwSize) == 0)
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to get the list of IRQs for SIM interrupt.\r\n")));
        return;
    }
    
    // Call the OAL to translate the IRQ into a SysIntr value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, dwIrqs,
        sizeof(dwIrqs), &pReaderExtension->dwSysintr, sizeof(DWORD), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for SIM interrupt.\r\n")));
        pReaderExtension->dwSysintr = (DWORD)SYSINTR_UNDEFINED;
        return;
    }

    // Initialize SIM interrupt
    if (!InterruptInitialize(pReaderExtension->dwSysintr, pReaderExtension->hIntrEvent, NULL, 0))
    {
        CloseHandle(pReaderExtension->hIntrEvent);
        pReaderExtension->hIntrEvent = NULL;
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pReaderExtension->dwSysintr, sizeof(DWORD), NULL, 0, NULL);
        DEBUGMSG(1, (TEXT("IsrThreadProc: KeybdInterruptEnable failed\r\n")));
        return;
    }

    if (pReaderExtension->dwPort == PORT0)
    {
        //select port 0
        INSREG32BF(&pSIMReg->SETUP, SIM_SETUP_SPS, SIM_SETUP_SPS_PORT0);

        //config port 0
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_3VOLT, SIM_PORT0_CNTL_3VOLT_DISABLE);   // disable 3 volt
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SVEN, SIM_PORT0_CNTL_SVEN_DISABLE);   //disable power for port 0
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SCSP, SIM_PORT0_CNTL_SCSP_DISABLE);   // Stopped clock = low
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SAPD, SIM_PORT0_CNTL_SAPD_DISABLE);   // Disable auto power down

        //set port 0 to be open drain
        INSREG32BF(&pSIMReg->OD_CONFIG, SIM_OD_CONFIG_OD_P0, SIM_OD_CONFIG_OD_P0_OD);        
    } 
    else
    {
        //select port 1
        INSREG32BF(&pSIMReg->SETUP, SIM_SETUP_SPS, SIM_SETUP_SPS_PORT1);

        //config port 1
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_3VOLT, SIM_PORT1_CNTL_3VOLT_DISABLE);   // disable 3 volt
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SVEN, SIM_PORT1_CNTL_SVEN_DISABLE);   //disable power for port 1
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SCSP, SIM_PORT1_CNTL_SCSP_DISABLE);   // Stopped clock = low
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SAPD, SIM_PORT1_CNTL_SAPD_DISABLE);   // Disable auto power down

        //set port 1 to be open drain
        INSREG32BF(&pSIMReg->OD_CONFIG, SIM_OD_CONFIG_OD_P1, SIM_OD_CONFIG_OD_P1_OD);
    }
    
    //Assume the card clock is about 3MHz sim_clock = 54MHz CLOCK_SELECT = 1000 (16)
    INSREG32BF(&pSIMReg->CLK_PRESCALER, SIM_CLK_PRESCALER, 0x2F);

    //Configure the TX and RX buffer threshold
    INSREG32BF(&pSIMReg->XMT_THRESHOLD, SIM_XMT_THRESHOLD_TDT, 0x1);
    INSREG32BF(&pSIMReg->RCV_THRESHOLD, SIM_RCV_THRESHOLD_RDT, 0x1);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_Init\n") ));

}


//-----------------------------------------------------------------------------
//
// Function: SIM_Open
//
// This function Opens SIM module
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
void SIM_Open(PREADER_EXTENSION pReaderExtension)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_Open\n") ));

    if (!BSPSetSIMClockGatingMode(pReaderExtension->dwIndex, TRUE))
    {
        ERRORMSG(TRUE, (TEXT("SIM_Open:  BSPSetSIMClockGatingMode failed!\r\n")));
        return;
    }

    // Enable Transmiter & Receiver (may need critical section here)
    INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_XMTEN, SIM_ENABLE_XMTEN_ENABLE);
    INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_RCVEN, SIM_ENABLE_RCVEN_ENABLE);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_Open\n") ));
}


//-----------------------------------------------------------------------------
//
// Function: SIM_Close
//
// This function closes SIM module
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
void SIM_Close(PREADER_EXTENSION pReaderExtension)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;
    int  uTries;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_Close\n") ));

    uTries = 0;

    //Wait for Transmit FIFO empty;
    while (!EXTREG32BF(&pSIMReg->XMT_STATUS, SIM_XMT_STATUS_TFE)&& (uTries++ < RETRYS))
    {
        Sleep(10);
    }

    // Disable Transmiter & Receiver (may need critical section here)
    INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_XMTEN, SIM_ENABLE_XMTEN_DISABLE);
    INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_RCVEN, SIM_ENABLE_RCVEN_DISABLE);


    // Release SYSINTR
    InterruptDisable(pReaderExtension->dwSysintr);
    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pReaderExtension->dwSysintr, sizeof(DWORD), NULL, 0, NULL);
    pReaderExtension->dwSysintr = (DWORD)SYSINTR_UNDEFINED;

    // close interrupt event handle
    if (pReaderExtension->hIntrEvent)
    {
        CloseHandle(pReaderExtension->hIntrEvent);
        pReaderExtension->hIntrEvent = NULL;
    }

    if (!BSPSetSIMClockGatingMode(pReaderExtension->dwIndex, FALSE))
    {
        ERRORMSG(TRUE, (TEXT("SIM_Open:  BSPSetSIMClockGatingMode failed!\r\n")));
        return;
    }

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_Close\n") ));
}

//-----------------------------------------------------------------------------
//
// Function: SIM_ColdReset
//
// This function performs cold reset to SIM
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//      *Buffer
//          [in]Pointer of the buffer to store the ATR
//      *BufferLength
//          [in]/[out]Pointer of the length of the buffer to store the ATR
//
// Returns:
//      NTSTATUS
//
//-----------------------------------------------------------------------------

NTSTATUS SIM_ColdReset(PREADER_EXTENSION pReaderExtension, UCHAR* Buffer, ULONG* BufferLength)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;
    BOOL SmallVolt = TRUE;
    UINT8 flag = 0, ATR_RemBytes = 0, Bit_Num, Y_Bytes = 0 , T = 0;
    UCHAR TA1_Byte;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_ColdReset\n") ));
   
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, SIM_CNTL_SAMPLE12_ENABLE);

    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_LRCEN, SIM_CNTL_LRCEN_DISABLE);    
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_LRCEN, SIM_CNTL_LRCEN_ENABLE);

    if (pReaderExtension->dwPort == PORT0)
    {
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SRST, SIM_PORT0_CNTL_SRST_DISABLE);
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_3VOLT, SIM_PORT0_CNTL_3VOLT_ENABLE);   // enable 3 volt
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SVEN, SIM_PORT0_CNTL_SVEN_ENABLE);
    
    }
    else
    {
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT0_CNTL_SRST, SIM_PORT0_CNTL_SRST_DISABLE);
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_3VOLT, SIM_PORT1_CNTL_3VOLT_ENABLE);   // enable 3 volt
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SVEN, SIM_PORT1_CNTL_SVEN_ENABLE);

    }
start:
    if (pReaderExtension->dwPort == PORT0)
    {
        //enable transmit data out for port 0
        INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_XMTEN, SIM_ENABLE_XMTEN_ENABLE);
        INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_RCVEN, SIM_ENABLE_RCVEN_ENABLE);

        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_STEN, SIM_PORT0_CNTL_STEN_ENABLE);        
        //enable clock for port 0
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SCEN, SIM_PORT0_CNTL_SCEN_ENABLE);
    }
    else
    {
        //enable transmit data out for port 1
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_STEN, SIM_PORT1_CNTL_STEN_ENABLE);
        //enable clock for port 1
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SCEN, SIM_PORT1_CNTL_SCEN_ENABLE);
    }


    // Delay for at least 400 * BAUD_CLK (400<t<40000)
    INSREG32BF(&pSIMReg->GPCNT, SIM_GPCNT, COLDRESET_DELAY1);
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, SIM_CNTL_GPCNT_CLK_SEL_DISABLE);       // Disable and reset GPCNT
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, SIM_CNTL_GPCNT_CLK_SEL_CARD);     // Enable GPCNT with card clock    

    //Wait for GP counter interrupt
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_ENABLE);   
    while(!EXTREG32BF(&pSIMReg->XMT_STATUS, SIM_XMT_STATUS_GPCNT))
    {
        if(WaitForSingleObject(pReaderExtension->hIntrEvent,1000)== WAIT_TIMEOUT)
        {
            RETAILMSG(1,(TEXT("GPCNT time out!0x%x\r\n"),pSIMReg->XMT_STATUS));
            return(STATUS_TIMEOUT);
        }
        InterruptDone(pReaderExtension->dwSysintr);
    }
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_MASK);
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_GPCNT, SIM_XMT_STATUS_GPCNT_CLR));     //Clear GP Counter flag
    
    if (pReaderExtension->dwPort == PORT0)
    {
        // SIM reset H
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SRST, SIM_PORT0_CNTL_SRST_ENABLE);           
    }
    else
    {
        // SIM reset H
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SRST, SIM_PORT1_CNTL_SRST_ENABLE); 
    }

    // Get the ATR Data block (40000)
    INSREG32BF(&pSIMReg->GPCNT, SIM_GPCNT, COLDRESET_DELAY2);
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, SIM_CNTL_GPCNT_CLK_SEL_DISABLE);       // Disable and reset GPCNT
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, SIM_CNTL_GPCNT_CLK_SEL_CARD);     // Enable GPCNT with card clock    
 
    // Enable the receiver and transmitter
    INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_XMTEN, SIM_ENABLE_XMTEN_ENABLE);
    INSREG32BF(&pSIMReg->ENABLE, SIM_ENABLE_RCVEN, SIM_ENABLE_RCVEN_ENABLE);

    //Start reading ATR
    (*BufferLength) = 0;

    // SmartCard Cold reset sequence
    // Set the CWT comparator register SIM_CHAR_WAIT = 0x2574(9600-12 = 9588) according to ISO 7816-3
    INSREG32BF(&pSIMReg->CHAR_WAIT, SIM_CHAR_WAIT_CWT, COLDRESET_CWT);
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_CWTEN, SIM_CNTL_CWTEN_DISABLE);
    OUTREG32(&pSIMReg->RCV_STATUS, CSP_BITFVAL(SIM_RCV_STATUS_CWT, SIM_RCV_STATUS_CWT_CLR));   
    // Enable CWT, but the CWT starts counting after the STOP bits of a received character 
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_CWTEN, SIM_CNTL_CWTEN_ENABLE);

    //Enable interrupt
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RIM, SIM_INT_MASK_RIM_ENABLE);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_CWTM, SIM_INT_MASK_CWTM_ENABLE);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_ENABLE);    

    for (;;)
    {

        if(WaitForSingleObject(pReaderExtension->hIntrEvent,1000)== WAIT_TIMEOUT)
        {
            RETAILMSG(1,(TEXT("SIM RECV interrupt time out!RCV:0x%x,XMT:0x%x\r\n"),
                        pSIMReg->RCV_STATUS,pSIMReg->XMT_STATUS));
            break;
        }

        if (EXTREG32BF(&pSIMReg->RCV_STATUS, SIM_RCV_STATUS_RFD))
        {
            // GPCNT Disable and reset if necessary
            if (EXTREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL)!=0)
            {
                INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, SIM_CNTL_GPCNT_CLK_SEL_DISABLE);
            }
            // signal that interrupt has been handled
            InterruptDone(pReaderExtension->dwSysintr);
            if ( flag ==1)
            {

                if (ATR_RemBytes == 1)
                {
                    if ((pReaderExtension->dwPort == PORT0) && (g_fPort0IsOK))
                        Buffer[*BufferLength]= (unsigned char)(EXTREG32BF(&pSIMReg->PORT0_RCV_BUF, SIM_PORT0_RCV_BUF_RCV_BUF));
                    else
                        Buffer[*BufferLength]= (unsigned char)(EXTREG32BF(&pSIMReg->PORT1_RCV_BUF, SIM_PORT1_RCV_BUF_RCV_BUF));
                 
                    (*BufferLength)++;
                    ATR_RemBytes --;
                    break;
                }
                else if (ATR_RemBytes != 1)
                {
                    if ((pReaderExtension->dwPort == PORT0) && (g_fPort0IsOK))
                        Buffer[*BufferLength]= (unsigned char)(EXTREG32BF(&pSIMReg->PORT0_RCV_BUF, SIM_PORT0_RCV_BUF_RCV_BUF));
                    else
                        Buffer[*BufferLength]= (unsigned char)(EXTREG32BF(&pSIMReg->PORT1_RCV_BUF, SIM_PORT1_RCV_BUF_RCV_BUF));
              
                    (*BufferLength)++;

                    //To get the next IO package
                    if(Y_Bytes)
                    {
                        Y_Bytes --;
                        if(Y_Bytes == 1)
                            flag = 0;
                    }
                    ATR_RemBytes --;
                }
            }
            else if ( flag == 0)
            {
                if ((pReaderExtension->dwPort == PORT0) && (g_fPort0IsOK))
                    Buffer[*BufferLength]= (unsigned char)(EXTREG32BF((&(pSIMReg->PORT0_RCV_BUF)), SIM_PORT0_RCV_BUF_RCV_BUF));
                else
                    Buffer[*BufferLength]= (unsigned char)(EXTREG32BF((&(pSIMReg->PORT1_RCV_BUF)), SIM_PORT1_RCV_BUF_RCV_BUF));

                
                if (*BufferLength == 1)
                {
                    //T0
                    TA1_Byte = 0xF0 & Buffer[*BufferLength];

                    for (Bit_Num = 7; Bit_Num > 3; Bit_Num --)
                    {
                        if ((TA1_Byte  & 0x80) == 0x80)
                            ATR_RemBytes ++;
                        TA1_Byte = TA1_Byte << 1;
                    }
                    //TD0 exist
                    if(TA1_Byte)
                        Y_Bytes = ATR_RemBytes;
                    TA1_Byte = 0x0F & Buffer[*BufferLength];
                    ATR_RemBytes = TA1_Byte + ATR_RemBytes;
                    flag = 1;
                }
                else if(*BufferLength > 1)
                {
                    //TD(i)
                    TA1_Byte = 0xF0 & Buffer[*BufferLength];
                    Y_Bytes = 0;
                    for (Bit_Num = 7; Bit_Num > 3; Bit_Num --)
                    {
                        if (((TA1_Byte = TA1_Byte << 1) & 0x80) == 0x80)
                        {
                            ATR_RemBytes ++;
                            Y_Bytes ++;
                        }
                    }
                    if(T==0)
                    {
                        T = 0x0F & Buffer[*BufferLength];
                        //TCK exists
                        if(T)
                            ATR_RemBytes ++;
                    }
                    //check if TD(i+1) exist
                    if(!TA1_Byte)
                        Y_Bytes = 0;
                    flag = 1;
                }
                else if(*BufferLength == 0)
                {
                    if(Buffer[*BufferLength]==0)
                        (*BufferLength)--;
                }
                
                (*BufferLength)++;

            }
        }

        // Deal with the GPCNT timeout 
        else if (EXTREG32BF(&pSIMReg->XMT_STATUS, SIM_XMT_STATUS_GPCNT))
        {

            //clear the GPCNT flag
            RETAILMSG(1,(TEXT("GPCOUNT TimeOut\r\n")));
            break;
        }
        // Deal with the CWT timeout 
        else if (EXTREG32BF(&pSIMReg->RCV_STATUS, SIM_RCV_STATUS_CWT))
        {
            RETAILMSG(1,(TEXT("CWT TimeOut\r\n")));
            break;
        }
        else
        {
            InterruptDone(pReaderExtension->dwSysintr);
        }

    }

    // Disable GPCNT
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, SIM_CNTL_GPCNT_CLK_SEL_DISABLE);      
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_GPCNT, SIM_XMT_STATUS_GPCNT_CLR));

    //Disable CWT
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_CWTEN, SIM_CNTL_CWTEN_DISABLE);
    OUTREG32(&pSIMReg->RCV_STATUS, CSP_BITFVAL(SIM_RCV_STATUS_CWT, SIM_RCV_STATUS_CWT_CLR)); 

    //Disable interrupt and reset interrupt event        
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RIM, SIM_INT_MASK_RIM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_CWTM, SIM_INT_MASK_CWTM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_MASK);   
    InterruptDone(pReaderExtension->dwSysintr);
    ResetEvent(pReaderExtension->hIntrEvent);
    
    //need LRC check
    if(T&&(!EXTREG32BF(&pSIMReg->RCV_STATUS, SIM_RCV_STATUS_LRCOK)))
    {
        DEBUGMSG(1,(TEXT("LRC Error")));
        OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_RCV_STATUS_LRCOK, SIM_RCV_STATUS_LRCOK_CLR));
        return(STATUS_DATA_ERROR);    
    }

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_ColdReset\n") ));    

    if ((*BufferLength) > 0)
    {
        return(STATUS_SUCCESS);
    }
    else if ((*BufferLength) == 0 && SmallVolt == TRUE)
    {
        //Change the voltage and try again.
        SmallVolt = FALSE;
        if (pReaderExtension->dwPort == PORT0)
        {
            INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SRST, SIM_PORT0_CNTL_SRST_DISABLE);
            INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_3VOLT, SIM_PORT0_CNTL_3VOLT_DISABLE);
            INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SVEN, SIM_PORT0_CNTL_SVEN_ENABLE);
        }
        else
        {
            INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SRST, SIM_PORT0_CNTL_SRST_DISABLE);
            INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_3VOLT, SIM_PORT1_CNTL_3VOLT_DISABLE);
            INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SVEN, SIM_PORT1_CNTL_SVEN_ENABLE);
        }
        goto start;
    }
    else
        return(STATUS_NOT_SUPPORTED);
}

//-----------------------------------------------------------------------------
//
// Function: SIM_WarmReset
//
// This function performs warm reset to SIM
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//
// Returns:
//      NTSTATUS
//
//-----------------------------------------------------------------------------
NTSTATUS SIM_WarmReset(PREADER_EXTENSION pReaderExtension)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;
    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_WarmReset\n") ));

    // SIM reset = low
    if (pReaderExtension->dwPort == PORT0)
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SRST, 0);
    else
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SRST, 0);

    // Delay for at least 400 * BAUD_CLK
    INSREG32BF(&pSIMReg->GPCNT, SIM_GPCNT, COLDRESET_DELAY1);
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, CLK_SEL_DISABLE);    // Disable and reset GPCNT
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, CLK_SEL_CARD);   // Enable GPCNT with card clock 

    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_ENABLE);   
    while(!EXTREG32BF(&pSIMReg->XMT_STATUS, SIM_XMT_STATUS_GPCNT))
    {
        if(WaitForSingleObject(pReaderExtension->hIntrEvent,1000)== WAIT_TIMEOUT)
        {
            RETAILMSG(1,(TEXT("GPCNT time out (1)!0x%x\r\n"),pSIMReg->XMT_STATUS));
            return(STATUS_TIMEOUT);
        }
        InterruptDone(pReaderExtension->dwSysintr);
    }
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_MASK);
    // Clear GP Counter flag
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_GPCNT, SIM_XMT_STATUS_GPCNT_CLR));

    // SIM reset
    if (pReaderExtension->dwPort == PORT0)
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SRST, SIM_PORT0_CNTL_SRST_ENABLE);
    else
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SRST, SIM_PORT1_CNTL_SRST_ENABLE);

    // Delay for at least 40000 * BAUD_CLK
    INSREG32BF(&pSIMReg->GPCNT, SIM_GPCNT, COLDRESET_DELAY2);
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, CLK_SEL_DISABLE);    // Disable and reset GPCNT
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, CLK_SEL_CARD);   // Enable GPCNT with card clock 

    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_ENABLE);   
    while(!EXTREG32BF(&pSIMReg->XMT_STATUS, SIM_XMT_STATUS_GPCNT))
    {
        if(WaitForSingleObject(pReaderExtension->hIntrEvent,1000)== WAIT_TIMEOUT)
        {
            RETAILMSG(1,(TEXT("GPCNT time out (2)!0x%x\r\n"),pSIMReg->XMT_STATUS));
            return(STATUS_TIMEOUT);
        }
        InterruptDone(pReaderExtension->dwSysintr);
    }
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_MASK);
    // Clear GP Counter flag
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_GPCNT, SIM_XMT_STATUS_GPCNT_CLR));
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_GPCNT_CLK_SEL, CLK_SEL_DISABLE);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_WarmReset\n") ));

    return(STATUS_SUCCESS);
}


//-----------------------------------------------------------------------------
//
// Function: SIM_ReadData
//
// This function reads the data from receive buffer
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//      *Buffer
//          [in]Pointer of the destination buffer
//      Length
//          [in]Length of the data to be read
//
// Returns:
//      NTSTATUS
//
//-----------------------------------------------------------------------------
NTSTATUS SIM_ReadData(PREADER_EXTENSION pReaderExtension, UCHAR *Buffer, ULONG Length)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;
    ULONG i = 0;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_ReadData\n") ));

    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_CWTEN, SIM_CNTL_CWTEN_DISABLE);
    OUTREG32(&pSIMReg->RCV_STATUS, CSP_BITFVAL(SIM_RCV_STATUS_CWT, SIM_RCV_STATUS_CWT_CLR));        
    INSREG32BF(&pSIMReg->CHAR_WAIT, SIM_CHAR_WAIT_CWT, COLDRESET_CWT);
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_CWTEN, SIM_CNTL_CWTEN_ENABLE);

    //Enable interrupt
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RIM, SIM_INT_MASK_RIM_ENABLE);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_CWTM, SIM_INT_MASK_CWTM_ENABLE);

    while(Length >0)
    {
        if(WaitForSingleObject(pReaderExtension->hIntrEvent,1000)== WAIT_TIMEOUT)
        {
            RETAILMSG(1,(TEXT("SIM read data interrupt time out!RCV:0x%x,XMT:0x%x\r\n"),
                        pSIMReg->RCV_STATUS,pSIMReg->XMT_STATUS));
            break;
        }
        
        if (EXTREG32BF(&pSIMReg->RCV_STATUS, SIM_RCV_STATUS_RFD))
        {
            InterruptDone(pReaderExtension->dwSysintr);
            if ((pReaderExtension->dwPort == PORT0) && (g_fPort0IsOK))
                Buffer[i]= (unsigned char)EXTREG32BF(&pSIMReg->PORT0_RCV_BUF, SIM_PORT0_RCV_BUF_RCV_BUF);
            else
                Buffer[i]= (unsigned char)EXTREG32BF(&pSIMReg->PORT1_RCV_BUF, SIM_PORT1_RCV_BUF_RCV_BUF);

            i++;
            Length--;
        }
        // Deal with the CWT timeout 
        else if (EXTREG32BF(&pSIMReg->RCV_STATUS, SIM_RCV_STATUS_CWT))
        {
            INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_CWTEN, SIM_CNTL_CWTEN_DISABLE);
            OUTREG32(&pSIMReg->RCV_STATUS, CSP_BITFVAL(SIM_RCV_STATUS_CWT, SIM_RCV_STATUS_CWT_CLR)); 
                    
            INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RIM, SIM_INT_MASK_RIM_MASK);
            INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_CWTM, SIM_INT_MASK_CWTM_MASK);
            InterruptDone(pReaderExtension->dwSysintr);
            ResetEvent(pReaderExtension->hIntrEvent);
            RETAILMSG(1,(TEXT("CWT TimeOut in SIM_Read")));

            return(STATUS_TIMEOUT);
        }
        else
        {
            InterruptDone(pReaderExtension->dwSysintr);
        }
    }
    //Disable CWT
    INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_CWTEN, SIM_CNTL_CWTEN_DISABLE);
    OUTREG32(&pSIMReg->RCV_STATUS, CSP_BITFVAL(SIM_RCV_STATUS_CWT, SIM_RCV_STATUS_CWT_CLR)); 

    //Disable interrupt and reset interrupt event         
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RIM, SIM_INT_MASK_RIM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_CWTM, SIM_INT_MASK_CWTM_MASK);
    InterruptDone(pReaderExtension->dwSysintr);
    ResetEvent(pReaderExtension->hIntrEvent);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_ReadData\n") ));

    return(STATUS_SUCCESS);
}


//-----------------------------------------------------------------------------
//
// Function: SIM_WriteData
//
// This function writes the data to transmitter buffer
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//      *Buffer
//          [in]Pointer of the destination buffer
//      Length
//          [in]Length of the data to be read
//
// Returns:
//      NTSTATUS
//
//-----------------------------------------------------------------------------
NTSTATUS SIM_WriteData(PREADER_EXTENSION pReaderExtension, UCHAR *Buffer, ULONG Length)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;
    ULONG i;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_WriteData\n") ));
    //INSREG32BF(&pSIMReg->GUARD_CNTL, SIM_GUARD_CNTL_GETU, 0xFF);

    //Enable interrupt
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_TC, SIM_XMT_STATUS_TC_CLR));
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_TCIM, SIM_INT_MASK_TCIM_ENABLE);

    // Write the characters to be sent as response to the XMT FIFO 
    for ( i = 0; i < Length; i++)
    {
        if ((pReaderExtension->dwPort == PORT0) && (g_fPort0IsOK))
        {
            pSIMReg->PORT0_XMT_BUF = Buffer[i];
        }
        else
        {
            pSIMReg->PORT1_XMT_BUF = Buffer[i];
        }
        if(WaitForSingleObject(pReaderExtension->hIntrEvent,50)== WAIT_TIMEOUT)
        {
            RETAILMSG(1,(TEXT("SIM write data interrupt time out!RCV:0x%x,XMT:0x%x\r\n"),
                        pSIMReg->RCV_STATUS,pSIMReg->XMT_STATUS));
        }
        OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_TC, SIM_XMT_STATUS_TC_CLR));
        InterruptDone(pReaderExtension->dwSysintr);
    }
    // Clear the flags
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_TFE, SIM_XMT_STATUS_TFE_CLR));
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_ETC, SIM_XMT_STATUS_ETC_CLR));
    OUTREG32(&pSIMReg->XMT_STATUS, CSP_BITFVAL(SIM_XMT_STATUS_TC, SIM_XMT_STATUS_TC_CLR));
        

    //Disable interrupt and reset interrupt event 
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_TCIM, SIM_INT_MASK_TCIM_MASK);
    InterruptDone(pReaderExtension->dwSysintr);
    ResetEvent(pReaderExtension->hIntrEvent);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_WriteData\n") ));

    return(STATUS_SUCCESS);
}


//-----------------------------------------------------------------------------
//
// Function: SIM_Deactivate
//
// This function deactives SIM
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//
// Returns:
//      NTSTATUS
//
//-----------------------------------------------------------------------------
NTSTATUS SIM_Deactivate(PREADER_EXTENSION pReaderExtension)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_Deactivate\n") ));

    if (pReaderExtension->dwPort == PORT0)
    {
        // RST shall be put to 0 
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SRST, SIM_PORT0_CNTL_SRST_DISABLE);

        // CLK shall be put to 0 
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SCSP, SIM_PORT0_CNTL_SCSP_DISABLE);

        // I/O shall be put to 0
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_STEN, SIM_PORT0_CNTL_STEN_DISABLE);

        // VCC shall be deactivate 
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_SVEN, SIM_PORT0_CNTL_SVEN_DISABLE);
        INSREG32BF(&pSIMReg->PORT0_CNTL, SIM_PORT0_CNTL_3VOLT, SIM_PORT0_CNTL_3VOLT_DISABLE);
    }
    else
    {
        // RST shall be put to 0 
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SRST, SIM_PORT1_CNTL_SRST_DISABLE);

        // CLK shall be put to 0 
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SCSP, SIM_PORT1_CNTL_SCSP_DISABLE);

        // I/O shall be put to 0
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_STEN, SIM_PORT1_CNTL_STEN_DISABLE);

        // VCC shall be deactivate 
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_SVEN, SIM_PORT1_CNTL_SVEN_DISABLE);
        INSREG32BF(&pSIMReg->PORT1_CNTL, SIM_PORT1_CNTL_3VOLT, SIM_PORT1_CNTL_3VOLT_DISABLE);
    }

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_Deactivate\n") ));

    return(STATUS_SUCCESS);
}


//-----------------------------------------------------------------------------
//
// Function: SIM_ConfigCLK
//
// This function configures the clock and baud rate for sim
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//      Fi
//          [in]Clock correction factor
//      Di
//          [in]Data rate adjustment factor
//
// Returns:
//      BOOL true means success false means fail
//
//-----------------------------------------------------------------------------
BOOL SIM_ConfigCLK(PREADER_EXTENSION pReaderExtension, UINT8 Fi, UINT8 Di)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;
    int baudrate, freq;
    UINT32 sim_clock;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_ConfigCLK\n") ));

    if ((Fi == (UINT8)0) && (Di == (UINT8)0x1))     /* Fd and Dd */
        return TRUE;

    if ((Fi == (UINT8)0) && (Di == (UINT8)0x0))     /* Fd and Dd */
        return TRUE;

    // Search and check Fi and fmax in Clock Rate Factor table
    if (g_crf[(INT8)Fi].f == RFU)
    {
        DEBUGMSG(1,(TEXT("Unsupported f : %d\r\n"),g_crf[(UINT8)Fi].f));
        return FALSE;
    }

    // Search and check Di in Baud Rate Factor table
    if (g_brf[(INT8)Di].Di == RFU)
    {
        DEBUGMSG(1,(TEXT("Unsupported f : %02x\r\n"),Di));
        return FALSE;
    }

    // Reconfigure the CLK_PRESCALER 
    sim_clock = BSPGetSIMCLK(pReaderExtension->dwIndex);
    freq = (int)sim_clock/(g_crf[Fi].f*MHz);
    DEBUGMSG(1,(TEXT("frep:%d = %d MHz / %d MHz\r\n"),freq,sim_clock/MHz,g_crf[Fi].f));

    if(freq<=255)
    {
        INSREG32BF(&pSIMReg->CLK_PRESCALER, SIM_CLK_PRESCALER, freq);
    }
    else
    {
        DEBUGMSG(1,(TEXT("Unsupported CLK_PRESCALER\r\n")));
        return FALSE;
    }

    // Reconfigure BAUD_SEL 
    baudrate = (int)g_crf[Fi].Fi/g_brf[Di].Di;
    DEBUGMSG(1,(TEXT("Current baud rate : %d\r\n"),baudrate));
    switch (baudrate)
    {
    case 372:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, 0x1);
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0);   
        break;
    case 256:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, 0);
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0x1);
        break;
    case 128:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, 0);
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0x2);
        break;
    case 64:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, 0);
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0x3);
        break;
    case 32:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, 0);
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0x4);
        break;
    case 16:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, 0);
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0x5);
        break;
    case 8:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_SAMPLE12, 0);
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0x6);
        break;
    default:
        INSREG32BF(&pSIMReg->CNTL, SIM_CNTL_BAUD_SEL, 0x7);
        INSREG32BF(&pSIMReg->DIVISOR, SIM_DIVISOR, (UINT8)baudrate & 0xff);
        break;
    }

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_ConfigCLK\n") ));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: SIM_ConfigTime
//
// This function configures the guard time and character wait time
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//      N
//          [in]Extra guard time needed
//      WI
//          [in]Waiting integer
//
// Returns:
//      BOOL true means success false means fail
//
//-----------------------------------------------------------------------------
BOOL SIM_ConfigTime(PREADER_EXTENSION pReaderExtension, UINT8 N, UINT8 WI)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_ConfigTime\n") ));

    if ( N == 0xFF )
    {
        N = 0;
    }

    //set the extra guard time
    INSREG32BF(&pSIMReg->GUARD_CNTL, SIM_GUARD_CNTL_GETU, N);

    //set the character wait time for T=0
    INSREG32BF(&pSIMReg->CHAR_WAIT, SIM_CHAR_WAIT_CWT, WI);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_ConfigTime\n") ));

    return(TRUE);
}


//-----------------------------------------------------------------------------
//
// Function: SIM_CheckCard
//
// This function checks if the card is insert. Current implementation is fake.
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//
// Returns:
//      BOOL true means card present false means card not present
//
//-----------------------------------------------------------------------------
BOOL SIM_CheckCard(PREADER_EXTENSION pReaderExtension)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("SIM_CheckCard\n") ));

    if (pReaderExtension->dwPort == PORT0)
    {
        if (!EXTREG32BF(&pSIMReg->PORT0_DETECT, SIM_PORT0_DETECT_SPDP))
            return(TRUE);
        else
            return(FALSE);
    }
    else
    {
        if (!EXTREG32BF(&pSIMReg->PORT1_DETECT, SIM_PORT1_DETECT_SPDP))
            return(TRUE);
        else
            return(FALSE);
    }

    return(TRUE);
}
//-----------------------------------------------------------------------------
//
// Function: SIM_MaskInterrupts
//
// This function masks all the interrupts for SIM
//
// Parameters:
//      pSIMReg
//          [in]Pointer of the startingl address of SIM register structures
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
static void SIM_MaskInterrupts(PCSP_SIM_REG pSIMReg)
{
    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_MaskInterrupts\n") ));

    INSREG32BF(&pSIMReg->PORT1_DETECT, SIM_PORT1_DETECT_SDIM, SIM_PORT1_DETECT_SDIM_MASK);
    INSREG32BF(&pSIMReg->PORT0_DETECT, SIM_PORT0_DETECT_SDIM, SIM_PORT0_DETECT_SDIM_MASK);

    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RFEM, SIM_INT_MASK_RFEM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_BGTM, SIM_INT_MASK_BGTM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_BWTM, SIM_INT_MASK_BWTM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RTM, SIM_INT_MASK_RTM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_CWTM, SIM_INT_MASK_CWTM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_GPCNTM, SIM_INT_MASK_GPCNTM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_TDTFM, SIM_INT_MASK_TDTFM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_TFOM, SIM_INT_MASK_TFOM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_XTM, SIM_INT_MASK_XTM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_TFEIM, SIM_INT_MASK_TFEIM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_ETCIM, SIM_INT_MASK_ETCIM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_OIM, SIM_INT_MASK_OIM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_TCIM, SIM_INT_MASK_TCIM_MASK);
    INSREG32BF(&pSIMReg->INT_MASK, SIM_INT_MASK_RIM, SIM_INT_MASK_RIM_MASK);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_MaskInterrupts\n") ));
}


//-----------------------------------------------------------------------------
//
// Function: SIM_FlushFIFO
//
// This function does the flushing for fifo
//
// Parameters:
//      pReaderExtension
//          [in]Pointer of the device context
//
// Returns:
//      void
//
//-----------------------------------------------------------------------------
void SIM_FlushFIFO(PREADER_EXTENSION pReaderExtension)
{
    PCSP_SIM_REG pSIMReg = pReaderExtension->pSIMReg;

    SmartcardDebug( DEBUG_DRIVER,( TEXT("+SIM_FlushFIFO\n") ));

    INSREG32BF(&pSIMReg->RESET_CNTL, SIM_RESET_CNTL_FLUSH_XMT, SIM_RESET_CNTL_FLUSH_XMT_ENABLE);
    INSREG32BF(&pSIMReg->RESET_CNTL, SIM_RESET_CNTL_FLUSH_RCV, SIM_RESET_CNTL_FLUSH_RCV_ENABLE);

    Sleep(10);

    INSREG32BF(&pSIMReg->RESET_CNTL, SIM_RESET_CNTL_FLUSH_XMT, SIM_RESET_CNTL_FLUSH_XMT_DISABLE);
    INSREG32BF(&pSIMReg->RESET_CNTL, SIM_RESET_CNTL_FLUSH_RCV, SIM_RESET_CNTL_FLUSH_RCV_DISABLE);

    SmartcardDebug( DEBUG_DRIVER,( TEXT("-SIM_FlushFIFO\n") ));
}




void Dump_Registers(PCSP_SIM_REG pSIMReg)
{  
    DEBUGMSG (1, (TEXT("port1_cntl: %x \r\n"),INREG16(&pSIMReg->PORT1_CNTL)));
    DEBUGMSG (1, (TEXT("SetupRegister: %x \r\n"),INREG16(&pSIMReg->SETUP)));
    DEBUGMSG (1, (TEXT("port1detect: %x \r\n"),INREG16(&pSIMReg->PORT1_DETECT)));
    DEBUGMSG (1, (TEXT("port0_cntl: %x \r\n"),INREG16(&pSIMReg->PORT0_CNTL)));
    DEBUGMSG (1, (TEXT("ControlRegister: %x \r\n"),INREG16(&pSIMReg->CNTL)));
    DEBUGMSG (1, (TEXT("clcok_select: %x \r\n"),INREG16(&pSIMReg->CLK_PRESCALER)));
    DEBUGMSG (1, (TEXT("recieve_threshold: %x \r\n"),INREG16(&pSIMReg->RCV_THRESHOLD)));
    DEBUGMSG (1, (TEXT("EnableReg: %x \r\n"),INREG16(&pSIMReg->ENABLE)));
    DEBUGMSG (1, (TEXT("Txmt_status: %x \r\n"),INREG16(&pSIMReg->XMT_STATUS)));
    DEBUGMSG (1, (TEXT("Rxv_status: %x \r\n"),INREG16(&pSIMReg->RCV_STATUS)));
    DEBUGMSG (1, (TEXT("Int_mask: %x \r\n"),INREG16(&pSIMReg->INT_MASK)));
    DEBUGMSG (1, (TEXT("port0_detect: %x \r\n"),INREG16(&pSIMReg->PORT0_DETECT)));
    DEBUGMSG (1, (TEXT("Data_Format: %x \r\n"),INREG16(&pSIMReg->DATA_FORMAT)));
    DEBUGMSG (1, (TEXT("Xmt_threshold: %x \r\n"),INREG16(&pSIMReg->XMT_THRESHOLD)));
    DEBUGMSG (1, (TEXT("Guard_Cntl: %x \r\n"),INREG16(&pSIMReg->GUARD_CNTL)));
    DEBUGMSG (1, (TEXT("OD_CONFIG: %x \r\n"),INREG16(&pSIMReg->OD_CONFIG)));
    DEBUGMSG (1, (TEXT("Reset_Control: %x \r\n"),INREG16(&pSIMReg->RESET_CNTL)));
    DEBUGMSG (1, (TEXT("Char_Wait: %x \r\n"),INREG16(&pSIMReg->CHAR_WAIT)));
    DEBUGMSG (1, (TEXT("GPCOUNT: %x \r\n"),INREG16(&pSIMReg->GPCNT)));
    DEBUGMSG (1, (TEXT("Divisor: %x \r\n"),INREG16(&pSIMReg->DIVISOR)));
    DEBUGMSG (1, (TEXT("BWT: %x \r\n"),INREG16(&pSIMReg->BWT)));
    DEBUGMSG (1, (TEXT("BGT: %x \r\n"),INREG16(&pSIMReg->BGT)));
}
