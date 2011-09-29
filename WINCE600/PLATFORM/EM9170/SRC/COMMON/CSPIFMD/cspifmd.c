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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
#include "cspifmd.h"

//-----------------------------------------------------------------------------
// External Functions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
extern VOID OALStall(UINT32 uSecs);

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

//port that SPI Flash is connected to
#define SPIFMD_PORT     CSP_BASE_REG_PA_CSPI1
//SS signal that SPI Flash is connected to
#define SPIFMD_SS       CSPI_CONREG_SS1
//from Flash datasheet, Flash supports modes 0 and 3
#define SPIFMD_PHA      CSPI_CONREG_PHA0
#define SPIFMD_CPOL     CSPI_CONREG_POL_ACTIVE_HIGH
//burst length of 8 bits in a WORD (TODO: this needs to be verified)
#define SPIFMD_BURST_LENGTH 0x7

#define SPIFMD_FREQ     2000000

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
PCSP_CSPI_REG g_pCspi;

//-----------------------------------------------------------------------------
// Local Variables
static DWORD g_dwCSPICfg;

//-----------------------------------------------------------------------------
// Local Functions
static DWORD CalcClockDiv(DWORD dwDesiredFreq, DWORD dwControllerFreq);
static VOID CSPI_ClockEnable(BOOL bEnable);
static VOID CSPI_GpioMux();
static BOOL TransactMessage(LPDWORD pbyTxBuf, LPDWORD pbyRxBuf, DWORD dwMsgLen);
static BOOL WriteInProgress();
static BOOL FlashIsWriteable();
static BOOL WriteEnable();

//-----------------------------------------------------------------------------
//
//  Function: CalcClockDiv
//
//  This function calculates the divisor used to divide down the controller clock to yield the data bit clock
//
//  Parameters:
//      dwDesiredFreq 
//          [in] Desired Frequency for bit clock of SPI bus (in Hz).
//      dwControllerFreq 
//          [in] Current clock frequency gated into controller (in Hz).
//
//  Returns:  
//      Returns divisor to be used for SPI Data Rate Control bits.
//
//-----------------------------------------------------------------------------
static DWORD CalcClockDiv(DWORD dwDesiredFreq, DWORD dwControllerFreq)
{
    DWORD dwDesiredDivisor = dwControllerFreq / dwDesiredFreq;
    DWORD dwCurDivisor = 4;
    DWORD dwDivisorBitCode = CSPI_CONREG_DIV4;

    //divisor has to be power of 2 starting at 4, assuming actual frequency has to be less than dwDesiredFreq
    for(dwDivisorBitCode = CSPI_CONREG_DIV4; dwDivisorBitCode <= CSPI_CONREG_DIV512; dwDivisorBitCode++)
    {
        //located smallest valid divisor that will generate the bit clock closest to dwDesiredFreq
        if(dwCurDivisor >= dwDesiredDivisor)
        {
            return dwDivisorBitCode;
        }
        dwCurDivisor *= 2;
    }

    //max out at 512
    return CSPI_CONREG_DIV512;
}

//-----------------------------------------------------------------------------
//
//  Function: CSPI_ClockEnable
//
//  This function eanbles/disables the clock into a CSPI controller
//
//  Parameters:
//      bEnable 
//          [in] If TRUE then enable clock, else disable clock.
//
//  Returns:  
//      none.
//
//-----------------------------------------------------------------------------
static VOID CSPI_ClockEnable(BOOL bEnable)
{
    DDK_CLOCK_GATE_INDEX index;

#if SPIFMD_PORT == CSP_BASE_REG_PA_CSPI1
        index = DDK_CLOCK_GATE_INDEX_CSPI1;
#elif SPIFMD_PORT == CSP_BASE_REG_PA_CSPI2
        index = DDK_CLOCK_GATE_INDEX_CSPI2;
#elif SPIFMD_PORT == CSP_BASE_REG_PA_CSPI3
        index = DDK_CLOCK_GATE_INDEX_CSPI3;
#endif

    OALClockSetGatingMode(index, (bEnable ? DDK_CLOCK_GATE_MODE_ENABLED : DDK_CLOCK_GATE_MODE_DISABLED));
}


//-----------------------------------------------------------------------------
//
//  Function: CSPI_GpioMux
//
//  This function muxes the appropriate GPIO for the CSPI controller being used (Note, for each board there may exist a different GPIO muxing scheme,
//  this function may need to be altered for a different board).
//
//  Parameters:
//      bEnable 
//          [in] If TRUE then enable clock, else disable clock.
//
//  Returns:  
//      none.
//
//-----------------------------------------------------------------------------
static VOID CSPI_GpioMux()
{
    PCSP_IOMUX_REGS pIOMUX = (PCSP_IOMUX_REGS) OALPAtoUA(CSP_BASE_REG_PA_IOMUXC);

#if SPIFMD_PORT == CSP_BASE_REG_PA_CSPI1
    //MOSI
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PAD_CSPI1_MOSI, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    //MISO
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PAD_CSPI1_MISO, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    //SCLK
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PAD_CSPI1_SCLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    //SS0
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PAD_CSPI1_SS0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    //SS1
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PAD_CSPI1_SS1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    //SS2 (commented out since for this board this pin is used for CAN)
    //OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PAD_GPIO_C, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    //SS3 (commented out since NFC uses this pin)
    //OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_NF_CE0, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
    //RDY
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PAD_CSPI1_RDY, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
#elif SPIFMD_PORT == CSP_BASE_REG_PA_CSPI2
    //board currently does not use CSPI 2 (pins are muxed for different peripherals)
#elif SPIFMD_PORT == CSP_BASE_REG_PA_CSPI3
    //board currently does not use CSPI 3 (pins are muxed for different peripherals)
#endif
}

//-----------------------------------------------------------------------------
//
//  Function: PrepareBuffer
//
//  This function prepares a buffer so that the least significant byte will get shifted out first. This function is 
//  neccessary to use instead of a memcpy because the hardware fifos can only be loaded DWORD at a time, and will 
//  shift it out MSB first. This is an issue when you have to transfer multiple bytes in one exchange
//  without deasserting the chip select, as in the case when dealing with the eeprom.
//
//  Parameters:
//        pOutBuffer
//          [in] Pointer to output buffer
//        pbyRxBpInBufferuf
//          [in] Pointer to input buffer
//        dwSize
//          [in] Length of data to copy
//
//  Returns:
//        TRUE if transaction was  successful, else return FALSE
//
//-----------------------------------------------------------------------------
void PrepareBuffer(PVOID pOutBuffer, PVOID pInBuffer, DWORD dwSize)
{
    DWORD i = 0;
    PBYTE pIn = (PBYTE) pInBuffer;
    PBYTE pOut = (PBYTE) pOutBuffer;

    for(i = 0; i < dwSize; i = i+4)
    {
        if((dwSize - i) < 4)
        {
            switch (dwSize - i)
            {
            case 1:
                pOut[i] = pIn[i];
                break;

            case 2:
                pOut[i] = pIn[i+1];
                pOut[i+1] = pIn[i];
                break;

            case 3:
                pOut[i] = pIn[i+2];
                pOut[i+1] = pIn[i+1];
                pOut[i+2] = pIn[i];
                break;

            default:
                break;
            }
        }
        else
        {
            pOut[i] = pIn[i+3];
            pOut[i+1] = pIn[i+2];
            pOut[i+2] = pIn[i+1];
            pOut[i+3] = pIn[i];
        }
    }
}



//-----------------------------------------------------------------------------
//
//  Function: TransactMessage
//
//  This function performs the SPI transactions. Inspired by COMMON_FSL_V2 CSPI driver code.
//
//  Parameters:
//        pbyTxBuf
//          [in] Pointer to transmit buffer
//        pbyRxBuf
//          [in] Pointer to receive buffer
//        dwMsgLen
//          [in] Length of TX/RX buffer in units of message words (in this case bytes)
//
//  Returns:
//        TRUE if transaction was  successful, else return FALSE
//
//-----------------------------------------------------------------------------
static BOOL TransactMessage(LPDWORD pbyTxBuf, LPDWORD pbyRxBuf, DWORD dwMsgLen)
{
    enum {
        LOAD_TXFIFO, 
        CONTINUE_XCHG, 
        FETCH_RXFIFO
    } xchState;

    DWORD dwTxCnt = 0;
    DWORD dwRxCnt = 0;
    DWORD dwTmpData = 0;
    LPDWORD pbyTxPtr = pbyTxBuf;
    LPDWORD pbyRxPtr = pbyRxBuf;
    DWORD dwTemp = 0;
    BOOL bXchDone = FALSE;

    if(pbyTxBuf == NULL)
    {
        return FALSE;
    }

    //copy the transmit data
    memcpy(pbyTxPtr, pbyTxBuf, dwMsgLen);
    
    OUTREG32(&g_pCspi->CONREG, g_dwCSPICfg);

    //dwMsgLen will be the number of bytes, we need the number of bits. multiply by 8.
    INSREG32(&g_pCspi->CONREG, CSP_BITFMASK(CSPI_CONREG_EN) | CSP_BITFMASK(CSPI_CONREG_BITCOUNT), 
                CSP_BITFVAL(CSPI_CONREG_EN , CSPI_CONREG_EN_ENABLE) | CSP_BITFVAL(CSPI_CONREG_BITCOUNT , (dwMsgLen*8)-1));
    

    xchState = LOAD_TXFIFO;

    while(!bXchDone)
    {
xch_loop:
        switch (xchState)
        {
            //initial load of TX FIFO, also starts transaction
            case LOAD_TXFIFO: 
                // load Tx FIFO until full, or until we run out of data
                while ((!(INREG32(&g_pCspi->STATREG) & CSP_BITFMASK(CSPI_STATREG_TF)))
                    && (dwTxCnt < dwMsgLen))
                {
                        // put next Tx data into CSPI FIFO
                        OUTREG32(&g_pCspi->TXDATA, *pbyTxPtr);

                        // increment Tx Buffer to next data point
                        pbyTxPtr++;

                        // increment Tx exchange counter
                        dwTxCnt = dwTxCnt + sizeof(DWORD);
                }
                
                // start exchange
                INSREG32(&g_pCspi->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
                    CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));

                xchState = (dwTxCnt < dwMsgLen) ? CONTINUE_XCHG : FETCH_RXFIFO;
                break;
            //enters this state if TX Buf has more data to send than what the initial load TX FIFO code could handle,
            //also reads from RX FIFO.
            case CONTINUE_XCHG:
                if (dwTxCnt >= dwMsgLen)
                {
                    xchState = FETCH_RXFIFO;
                    break;
                }

                // wait until RX FIFO Ready
                dwTemp = INREG32(&g_pCspi->STATREG);
                while (!(dwTemp & CSP_BITFMASK(CSPI_STATREG_RR)))
                {
                    ////if the transfer completed and TxFifo is full then set transmit again
                    //if((dwTemp & CSP_BITFMASK(CSPI_STATREG_TC)) && (dwTemp & CSP_BITFMASK(CSPI_STATREG_TF)))
                    //{
                    //    // start exchange
                    //    INSREG32(&g_pCspi->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
                    //    CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));
                    //}

                    //if the transfer completed that means CSPI hardware run faster than CPU test it
                    //we need take all RXFIFO and go back to check if all data are actually sent.
                    if(dwTemp & CSP_BITFMASK(CSPI_STATREG_TC))
                    {
                        xchState = FETCH_RXFIFO;
                        goto xch_loop;
                    }
                    dwTemp = INREG32(&g_pCspi->STATREG);
 
                }
                dwTmpData = INREG32(&g_pCspi->RXDATA);

                // if receive data is not to be discarded
                if (pbyRxBuf != NULL)
                {
                    // get next Rx data from CSPI FIFO
                    *pbyRxPtr = dwTmpData;

                    // increment Rx Buffer to next data point
                    pbyRxPtr++;
                }

                // increment Rx exchange counter
                dwRxCnt = dwRxCnt + sizeof(DWORD);

                OUTREG32(&g_pCspi->TXDATA, *pbyTxPtr);

                // increment Tx Buffer to next data point
                pbyTxPtr++;

                // increment Tx exchange counter
                dwTxCnt = dwTxCnt + sizeof(DWORD);

                //CSPI Driver here checks if Transaction Complete flag is set, this situation show not occur in this code here
                //since there should always be data in the TX FIFO so Transaction COmplete flag should not be detected here.
                //COMMENT b06505: the condition does occur when CPU loading TX FIFO is slower than CSPI hardware exchanging data. 
                if (INREG32(&g_pCspi->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC))
                {
                    xchState = FETCH_RXFIFO;
                }
                break;
            //reads out RX FIFO after all TX data has been sent
            case FETCH_RXFIFO:
                
                //Here the CSPI driver reads until the RX FIFO is empty, however, we don't
                //see why they did it this way since there will never be an RX FIFO overflow (since TX byte is only placed
                //in FIFO if RX byte is read from FIFO so 1 for 1 exchange)
                // COMMENT b06505: keep those code might be better for we can do something instead of just waiting hardware.

                // wait until transaction is complete OR Tx fifo is Empty
                dwTemp = INREG32(&g_pCspi->STATREG);
               
                while (!(dwTemp & CSP_BITFMASK(CSPI_STATREG_TC)))
                {
                    dwTemp = INREG32(&g_pCspi->STATREG);
                }

                // Fetch all remaining rxdata in RXFIFO
                /*while ((INREG32(&g_pCspi->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))*/
                while (dwRxCnt < dwMsgLen)
                {
                    dwTmpData = INREG32(&g_pCspi->RXDATA);

                    // if receive data is not to be discarded
                    if (pbyRxBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
                        *pbyRxPtr = dwTmpData;
                        // increment Rx Buffer to next data point
                        pbyRxPtr++;
                        
                    }

                    // increment Rx exchange counter
                    dwRxCnt = dwRxCnt + sizeof(DWORD);
                }

                if(dwTxCnt >= dwMsgLen)
                {
                    bXchDone = TRUE;
                }
                else
                {
                    xchState = LOAD_TXFIFO;
                }


                break;
        }
    }

    INSREG32(&g_pCspi->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), CSP_BITFVAL(CSPI_CONREG_EN , CSPI_CONREG_EN_DISABLE));
  
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: WriteInProgress
//
//  This function checks if there is a write in progress on the SPI Flash chip.
//
//  Parameters:
//        None.
//
//  Returns:
//        TRUE if Write is in progress, else return FALSE
//
//-----------------------------------------------------------------------------
static BOOL WriteInProgress()
{
    BOOL bRet = FALSE;
    DWORD dwTxBuf;
    BYTE byRxBuf[2] = {0, 0};

    dwTxBuf = RDSR_CMD << 8*1;
    TransactMessage(&dwTxBuf, (LPDWORD)byRxBuf, 2);

    //if write in progress then return true
    if((byRxBuf[0] & WIP) == WIP)
    {
        bRet = TRUE;
    }

    return bRet;
}

//-----------------------------------------------------------------------------
//
//  Function: FlashIsWriteable
//
//  This function waits until Flash is in Writeable state (i.e WIP bit is not set) or timeout is reached.
//
//  Parameters:
//        None.
//
//  Returns:
//        TRUE if Flash goes to a writeable state before the timeout is reached, else return FALSE
//
//-----------------------------------------------------------------------------
static BOOL FlashIsWriteable()
{
    DWORD dwCnt = 0;
    while(dwCnt < WIP_TIMEOUT)
    {
        if(!WriteInProgress())
        {
            return TRUE;
        }
        OALStall(ONE_MS);
        dwCnt++;
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
//
//  Function: WriteEnable
//
//  This function sets the Flash Chip to be in the Write Enable condition. Note
//  this has to be set at Power-up, after a Program/Erase instruction, Write Status
//  Register 
//
//  Parameters:
//        None.
//
//  Returns:
//        TRUE if Write Enable command could be sent out and if Flash is returned to a writeable state before a timeout,
//        else return FALSE
//
//-----------------------------------------------------------------------------
static BOOL WriteEnable()
{
    BYTE byTxBuf = WREN_CMD;

    if(!FlashIsWriteable())
    {
        return FALSE;
    }

    //Write out WREN command (1 Byte Transaction)
    TransactMessage((LPDWORD)&byTxBuf, NULL, sizeof(byTxBuf));

    if(!FlashIsWriteable())
    {
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: SPIFMD_Init
//
//  This function initializes the SPI Flash interface.
//
//  Parameters:
//        None.
//
//  Returns:
//        None.
//
//-----------------------------------------------------------------------------
BOOL SPIFMD_Init(void)
{  
    BOOL bInit = TRUE;
    DWORD dwTxBuf = 0;
    BYTE byRxBuf[4] = {0,0,0,0};
    BSP_ARGS *pArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START; 
    static BYTE pRxBuff[SPIFMD_SECTOR_SIZE];
    static BYTE pTxBuff[SPIFMD_SECTOR_SIZE];
    BOOL TEST = 0;


    g_pCspi = (PCSP_CSPI_REG)OALPAtoUA(SPIFMD_PORT);

    //gate clock into controller
    CSPI_ClockEnable(TRUE);

    //mux GPIO pins
    CSPI_GpioMux();

    g_dwCSPICfg = CSP_BITFVAL(CSPI_CONREG_BITCOUNT, SPIFMD_BURST_LENGTH) | 
        CSP_BITFVAL(CSPI_CONREG_DATARATE, CalcClockDiv(SPIFMD_FREQ, pArgs->clockFreq[DDK_CLOCK_SIGNAL_IPG])) | 
        CSP_BITFVAL(CSPI_CONREG_CHIPSELECT, SPIFMD_SS) | 
        CSP_BITFVAL(CSPI_CONREG_DRCTL, CSPI_CONREG_DRCTL_DONTCARE) |
        CSP_BITFVAL(CSPI_CONREG_SSPOL, CSPI_CONREG_SSPOL_ACTIVE_LOW) | 
        CSP_BITFVAL(CSPI_CONREG_SSCTL, CSPI_CONREG_SSCTL_ASSERT) |
        CSP_BITFVAL(CSPI_CONREG_PHA, SPIFMD_PHA) |
        CSP_BITFVAL(CSPI_CONREG_POL, SPIFMD_CPOL) |
        CSP_BITFVAL(CSPI_CONREG_SMC, CSPI_CONREG_SMC_XCH) | 
        CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_IDLE) |
        CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_MASTER) |
        CSP_BITFVAL(CSPI_CONREG_EN , CSPI_CONREG_EN_DISABLE);
    
    //set up SPI control reg + clock divider
    OUTREG32(&g_pCspi->CONREG, g_dwCSPICfg);

    //Read in RDID (4 Byte Transaction)
    dwTxBuf = (DWORD)(RDID_CMD << 8*3);
    TransactMessage(&dwTxBuf, (LPDWORD)byRxBuf, sizeof(dwTxBuf));

    //check Manufacturer ID, 1st RX buffer byte is dummy
    if(byRxBuf[2] != MANUFACTURER_ID)
    {
        RETAILMSG(1,(TEXT("SPIFMD_Init: Manufacturer ID = 0x%x, Should be 0x%x \r\n"), byRxBuf[1], MANUFACTURER_ID));
        bInit = FALSE;
        goto cleanup;
    }

    if(!WriteEnable())
    {
        RETAILMSG(1,(TEXT("SPIFMD_Init: WriteEnable failed \r\n")));
        bInit = FALSE;
        goto cleanup;
    }

    //Write out WRSR command (2 Byte Transaction)
    dwTxBuf = WRSR_CMD << 8*1; //set SRWD & BP0-BP3 bits to 0 to enable write to any location in flash
    TransactMessage(&dwTxBuf, NULL, sizeof(BYTE)*2);
    OALStall(WRSR_CYCLE_TIME);

    //READ status register
    dwTxBuf = 0;
    dwTxBuf = RDSR_CMD << 8*1;
    TransactMessage(&dwTxBuf, (LPDWORD)&pRxBuff, sizeof(BYTE)*2);

    if(TEST)
    {
        DWORD i =0;
        //erase
        RETAILMSG(1,(TEXT("SPIFMD_Init: erasing\r\n")));
        SPIFMD_EraseBlock(0);
        ////read
        memset(pRxBuff, 0x00, SPIFMD_SECTOR_SIZE);
        RETAILMSG(1,(TEXT("SPIFMD_Init: reading\r\n")));
        SPIFMD_ReadSector(0, pRxBuff,
                      NULL, 1);

        //write
        for(i = 0; i < SPIFMD_SECTOR_SIZE; i++)
        {
            *(pTxBuff+i) = (BYTE)i;
        }
        RETAILMSG(1,(TEXT("SPIFMD_Init: WRITING 0xa5\r\n")));
        SPIFMD_WriteSector(0, pTxBuff,
                      NULL, 1);

        //read again
        memset(pRxBuff, 0x00, SPIFMD_SECTOR_SIZE);
        RETAILMSG(1,(TEXT("SPIFMD_Init: READING\r\n")));
        SPIFMD_ReadSector(0, pRxBuff,
                      NULL, 1);
        

        /*RETAILMSG(1,(TEXT("\r\nTx Buffer\r\n")));
        for(i = 0; i < SPIFMD_SECTOR_SIZE; i++)
        {
            RETAILMSG(1,(TEXT("0x%x\r\n"), *((pTxBuff+i))));
        }

        RETAILMSG(1,(TEXT("\r\nRx Buffer\r\n")));
        for(i = 0; i < SPIFMD_SECTOR_SIZE; i++)
        {
            RETAILMSG(1,(TEXT("0x%x\r\n"), *((pRxBuff+i))));
        }*/
        if(memcmp(pTxBuff, pRxBuff, SPIFMD_SECTOR_SIZE) != 0)
        {
            RETAILMSG(1,(TEXT("\r\nBuffers are not the same\r\n")));
        }
        else
        {
            RETAILMSG(1,(TEXT("\r\nBuffers are the same\r\n")));
        }
    }

cleanup:
    return bInit;
}

//-----------------------------------------------------------------------------
//
//  Function: SPIFMD_Deinit
//
//  This function de-initializes the flash write enable (can still read).
//
//  Parameters:
//        None.
//
//  Returns:  
//        None.
//
//-----------------------------------------------------------------------------
BOOL SPIFMD_Deinit (void)
{
    BYTE byTxBuf;
    BOOL bInit = TRUE;
    
    if(!FlashIsWriteable())
    {
        bInit = FALSE;
        goto cleanup;
    }

    //Write out WRDI command to disable write (1 Byte Transaction)
    byTxBuf = WRDI_CMD;
    TransactMessage((LPDWORD)&byTxBuf, NULL, sizeof(byTxBuf));

    if(g_pCspi)
    {
        g_pCspi = NULL;
    }

cleanup:
    return bInit;
}


//-----------------------------------------------------------------------------
//
//  Function: SPIFMD_GetInfo
//
//  This function determines the size characteristics for the SPI Flash
//
//  Parameters:
//      pFlashInfo 
//          [out] A pointer to a structure that contains the size 
//          characteristics for the flash memory device.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL SPIFMD_GetInfo(PFlashInfo pFlashInfo)
{
    if (!pFlashInfo)
        return(FALSE);

    pFlashInfo->flashType           = (FLASH_TYPE)SPIFMD;
    pFlashInfo->wDataBytesPerSector = SPIFMD_SECTOR_SIZE;
    pFlashInfo->dwNumBlocks         = SPIFMD_BLOCK_CNT;
    pFlashInfo->wSectorsPerBlock    = SPIFMD_SECTOR_CNT;
    pFlashInfo->dwBytesPerBlock     = (pFlashInfo->wSectorsPerBlock * pFlashInfo->wDataBytesPerSector);

    return(TRUE);
}


//-----------------------------------------------------------------------------
//
//  Function: SPIFMD_GetBlockStatus
//
//  This function returns the status of a block.
//
//  Parameters:
//      blockID 
//          [in] The block number used to check status.
//
//  Returns:  
//      Flags to describe the status of the block.
//
//-----------------------------------------------------------------------------
DWORD SPIFMD_GetBlockStatus(BLOCK_ID blockID)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(blockID);

    return(BLOCK_STATUS_UNKNOWN);
}


//-----------------------------------------------------------------------------
//
//  Function: SPIFMD_SetBlockStatus
//
//  This function sets the status of a block.
//
//  Parameters:
//      blockID 
//          [in] The block number used to set status. 
//
//      dwStatus
//          [in] The status value to set.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL SPIFMD_SetBlockStatus(BLOCK_ID blockID, DWORD dwStatus)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(blockID);
    UNREFERENCED_PARAMETER(dwStatus);

    return(TRUE);
}

//-----------------------------------------------------------------------------
//
// Function: SPIFMD_EraseBlock
//
// Function Erases the specified flash block. 
//
// Parameters:
//      blockID 
//          [in] block number to erase.
//
// Returns:  
//      TRUE/FALSE for success
//
//-----------------------------------------------------------------------------
BOOL SPIFMD_EraseBlock(BLOCK_ID blockID)
{
    BOOL erase_request = TRUE;
    DWORD dwFlashAddr = 0;
    DWORD dwTxBuf;

    //convert block ID to 24 bit address
    dwFlashAddr = blockID * SPIFMD_SECTOR_SIZE * SPIFMD_SECTOR_CNT;
    
    if(!WriteEnable())
    {
        erase_request = FALSE;
        goto cleanup;
    }

    //Block Erase command
    dwFlashAddr &= 0xFFFFFF;  //make sure it is only 3 bytes long
    dwTxBuf = (DWORD)(BE_CMD<<8*3 | dwFlashAddr);
    TransactMessage(&dwTxBuf, NULL, sizeof(dwTxBuf));

cleanup:
    return erase_request;
}

//-----------------------------------------------------------------------------
//
// Function: SPIFMD_ReadSector
//
// Function Reads the requested sector data and metadata from the flash media. 
//
// Parameters:
//      startSectorAddr 
//          [in] starting sector address.
//
//      pSectorBuff 
//          [in] Ptr to  buffer that contains sector data read from flash.
//                 Set to NULL if  data is not needed.
//
//      pSectorInfoBuff 
//          [in] Buffer for an array of sector information structures.
//                 One sector    information entry for every sector to be read. 
//                 Set to NULL if not needed.
//
//      dwNumSectors 
//          [in] Number of sectors to read.
//
// Returns:  
//      TRUE/FALSE for success
//
//-----------------------------------------------------------------------------
BOOL SPIFMD_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                      PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
    BOOL read_status = TRUE;
    DWORD dwCurPage = 0;
    LPBYTE pCurPageBuf = pSectorBuff;
    DWORD dwCurAddr = startSectorAddr * SPIFMD_SECTOR_SIZE; //changed to 24-bit flat address (from sector address)
    DWORD dwNumOfPages = dwNumSectors * (SPIFMD_SECTOR_SIZE / SPIFMD_PAGE_SIZE);
    //could do single sector reads however, to make the buffer smaller we make reads only page length
    BYTE byTxBuf[SPIFMD_PAGE_SIZE + 4];
    BYTE byRxBuf[SPIFMD_PAGE_SIZE + 4];

    UNREFERENCED_PARAMETER(pSectorInfoBuff);

    //check if Flash is still being written to
    if(!FlashIsWriteable())
    {
        read_status = FALSE;
        goto cleanup;
    }
    
    byTxBuf[3] = READ_CMD;
    for(dwCurPage = 0; dwCurPage  < dwNumOfPages; dwCurPage++)
    {
        byTxBuf[2] = (BYTE)(dwCurAddr >> 16);
        byTxBuf[1] = (BYTE)(dwCurAddr >> 8);
        byTxBuf[0] = (BYTE)(dwCurAddr);
        
        memset(&byTxBuf[4], 0x0, sizeof(byTxBuf)-4);
        TransactMessage((LPDWORD) byTxBuf, (LPDWORD) byRxBuf, sizeof(byTxBuf));

        //copy into sector buffer (first 4 read bytes are dummy)
        PrepareBuffer(pCurPageBuf, (byRxBuf + 4), SPIFMD_PAGE_SIZE);

        dwCurAddr += SPIFMD_PAGE_SIZE;
        pCurPageBuf += SPIFMD_PAGE_SIZE;
    }

cleanup:
    return read_status;
}

//-----------------------------------------------------------------------------
//
// Function: SPIFMD_WriteSector
//
// Function Writes the requested sector data and metadata to the flash media. 
//
// Parameters:
//      startSectorAddr 
//          [in] starting physical sector address.
//
//      pSectorBuff 
//          [in] Ptr to  buffer that contains sector data to write.
//                 Set to NULL if  data is not needed.
//
//      pSectorInfoBuff 
//          [in] Buffer for an array of sector information structures.
//                 One sector    information entry for every sector to be written.
//                 Set to NULL if not needed. (Not currently used)
//
//      dwNumSectors 
//          [in] Number of sectors to write.
//
// Returns:  
//      TRUE/FALSE for success
//
//-----------------------------------------------------------------------------
BOOL SPIFMD_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                       PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
    BOOL write_status = TRUE;
    DWORD dwCurPage = 0;
    DWORD dwCurAddr = startSectorAddr * SPIFMD_SECTOR_SIZE; //changed to 24-bit flat address (from sector address)
    DWORD dwNumOfPages = dwNumSectors * (SPIFMD_SECTOR_SIZE / SPIFMD_PAGE_SIZE);
    LPBYTE pCurPageBuf = pSectorBuff;
    BYTE byTxBuf[SPIFMD_PAGE_SIZE + 4]; //4 extra bytes for command + address

    UNREFERENCED_PARAMETER(pSectorInfoBuff);

    if(pSectorBuff == NULL)
    {
        write_status = FALSE;
        goto cleanup;
    }
    
    byTxBuf[3] = PP_CMD;

    //page programs for sector
    for(dwCurPage = 0; dwCurPage  < dwNumOfPages; dwCurPage++)
    {
        if(!WriteEnable())
        {
            write_status = FALSE;
            goto cleanup;
        }

        byTxBuf[2] = (BYTE)(dwCurAddr >> 16);
        byTxBuf[1] = (BYTE)(dwCurAddr >> 8);
        byTxBuf[0] = (BYTE)(dwCurAddr);

        PrepareBuffer((byTxBuf + 4), pCurPageBuf, SPIFMD_PAGE_SIZE);

        TransactMessage((LPDWORD)byTxBuf, NULL, sizeof(byTxBuf));

        dwCurAddr += SPIFMD_PAGE_SIZE;
        pCurPageBuf += SPIFMD_PAGE_SIZE;
    }

cleanup:
    return write_status;
}
