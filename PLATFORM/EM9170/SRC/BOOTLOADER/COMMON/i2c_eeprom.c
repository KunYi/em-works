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
//  File:  i2c_eeprom.c
//
//  Contains BOOT I2C EEPROM support functions.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "loader.h"

//-----------------------------------------------------------------------------
// External Functions
BOOL OALI2cInit( BYTE *pbySelfAddr, BYTE *pbyClkDiv );
void OALI2cEnable( BOOL bChangeAddr, BOOL bChangeClkDiv, BYTE bySelfAddr, BYTE byClkDiv );
void OALI2cDisable();
BOOL OALI2cGenerateStart( BYTE devAddr, BOOL bWrite );
BOOL OALI2cRepeatedStart( BYTE devAddr, BOOL bWrite, BOOL bRSTACycleComplete );
void OALI2cGenerateStop();
void OALI2cSetReceiveMode( BOOL bReceive );
void OALI2cSetTxAck( BOOL bTxAck );
WORD OALI2cCalculateClkRateDiv(DWORD dwFrequency);
 
BOOL OALI2cWriteData( BYTE *pData, WORD nBufLen, 
                     BOOL bStopAhead, 
                     BOOL bRepeatedStartCycleAhead, 
                     BOOL *pbStopped, 
                     BOOL *pbRSTACycleCompleted );
 
BOOL OALI2cReadData( BYTE *pData, WORD nBufLen, 
                    BOOL bStopAhead, 
                    BOOL bRepeatedStartCycleAhead, 
                    BOOL *pbStopped, 
                    BOOL *pbRSTACycleCompleted );
 
void OALI2cGenerateStop();

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

//page size of I2C EEPROM in bytes
// The EEPROM model is IS24C32A (marking IS432A) and not a AT24C512B (In the schematics SCH-23250 12th june 2008 it is a AT24C512B)
#define     EEPROM_PAGE_SIZE        (32)
#define     EEPROM_MAX_TRANSFER_SIZE    (EEPROM_PAGE_SIZE) //must be a sub-multiple of EEPROM_PAGE_SIZE
#define     EEPROM_ADDR_SIZE        (2)
#define     EEPROM_SIZE             (4*1024)
#define     EEPROM_BUFFER_PAGES     (2)
#define     EEPROM_SLAVE_ADDR       0x50

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static BYTE bDiv;

//-----------------------------------------------------------------------------
// Local Functions
BOOL PageWrite(LPBYTE pAddr, WORD wAddrLen, LPBYTE pPageBuf);
BOOL PageRead(LPBYTE pAddr, WORD wAddrLen, LPBYTE pReadBuf);
BOOL IsWriteOperationComplete();

//-----------------------------------------------------------------------------
//
//  Function:  I2CEEPROMWriteXldr
//
//  This function writes to I2C EEPROM the XLDR image stored 
//  in the RAM file cache area.
//
//  Parameters:
//      dwStartAddr 
//          [in] Address in flash memory where the start of the downloaded 
//          XLDR image is to be written.
//
//      dwLength 
//          [in] Length of the XLDR image, in bytes, to be written to flash
//          memory.            
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL I2CEEPROMWriteXldr(DWORD dwStartAddr, DWORD dwLength)
{
    LPBYTE pPageBuf, pImage;
    DWORD dwNumPageWrites;
    DWORD dwCurPage = 0;
    static BYTE byReadBackBuf[EEPROM_MAX_TRANSFER_SIZE];
    static BYTE byAddr[2];
    
    //Init I2C Bus
    OALI2cInit(0, &bDiv);
    
    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);
    
    // If image is larger than 4K, this must be xldr.bin produced by
    // ROMIMAGE
    EdbgOutputDebugString("I2CEEPROMWriteXldr: dwLength = 0x%x\r\n", dwLength);
    if (dwLength > 0x1000)
    {
        // ROMIMAGE adds 4K page at the beginning of the image.
        // We will flash the first 2K bytes that appear after this 4K page.
        //
        pImage += 0x1000;
        dwLength -= 0x1000;
    }

    //number of page writes + EEPROM_BUFFER_PAGES (for buffering & for partial page sizes) 
    dwNumPageWrites = (dwLength / EEPROM_MAX_TRANSFER_SIZE);// + EEPROM_BUFFER_PAGES;

    // Fill unused space with 0xFF
    memset(pImage + dwLength, 0xFF, (dwNumPageWrites * EEPROM_MAX_TRANSFER_SIZE) - dwLength);
    
    // Make sure XLDR length does not exceed size that can be supported by EEPROM
    if (dwLength > EEPROM_SIZE)
    {
        EdbgOutputDebugString("ERROR: XLDR exceeds 64KByte\r\n");
        return(FALSE);
    }

    //writes out page writes
    pPageBuf = pImage;
    for(dwCurPage = 0; dwCurPage < dwNumPageWrites; dwCurPage++)
    {
        
        //write out page
        byAddr[0] = (BYTE)((dwCurPage * EEPROM_MAX_TRANSFER_SIZE) / 256);
        byAddr[1] = (BYTE)((dwCurPage * EEPROM_MAX_TRANSFER_SIZE) % 256);

        //Program the page
        if(PageWrite(byAddr, sizeof(byAddr), pPageBuf) == FALSE)
        {
            EdbgOutputDebugString("ERROR: I2CEEPROMWriteXldr: PageWrite Failed\r\n");
        }
        
        //Wait until the Write operation is complete by polling the EEPROM
        while(IsWriteOperationComplete() == FALSE)
        {
        }

        //read back page and verify
        if(PageRead(byAddr, sizeof(byAddr), byReadBackBuf) == FALSE)
        {
            EdbgOutputDebugString("ERROR: I2CEEPROMWriteXldr: PageRead Failed\r\n");
        }

        
        if(memcmp(pPageBuf, byReadBackBuf, (EEPROM_MAX_TRANSFER_SIZE)) != 0)
        {
            EdbgOutputDebugString("ERROR: I2CEEPROMWriteXldr: Read back buffer does not equal written buffer.\r\n");
            return(FALSE);
        }

        pPageBuf = pPageBuf + EEPROM_MAX_TRANSFER_SIZE;
    }

    
    OALMSGS( OAL_INFO, (_T("I2C EEPROM Xldr Written Successfully\r\n")));
    return(TRUE);

}


//-----------------------------------------------------------------------------
//
//  Function:  PageWrite
//
//  This function writes data in page length to the I2C EEPROM
//
//  Parameters:
//      pAddr 
//          [in] Starting Address in EEPROM to write a page of data to
//      wAddrLen 
//          [in] Length of Address (pAddr)
//      pPageBuf
//          [in]  Page buffer in write buffer
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL PageWrite(LPBYTE pAddr, WORD wAddrLen, LPBYTE pPageBuf)
{
    OALI2cEnable(FALSE, FALSE, 0, bDiv);

    if(OALI2cGenerateStart(EEPROM_SLAVE_ADDR, TRUE) == FALSE)
    {
         EdbgOutputDebugString("ERROR: PageWrite: Generate Start Failed");
         return(FALSE);
    }
    if(OALI2cWriteData(pAddr, wAddrLen, FALSE, FALSE, NULL, NULL) == FALSE)
    {
         EdbgOutputDebugString("ERROR: PageWrite: Write EEPROM address Failed");
         return(FALSE);
    }
    if(OALI2cWriteData(pPageBuf, EEPROM_MAX_TRANSFER_SIZE, FALSE/*TRUE*/, FALSE, NULL/*&bStopped*/, NULL) == FALSE)
    {
        EdbgOutputDebugString("ERROR: PageWrite: Write EEPROM Data Failed");
        return(FALSE);
    }

    OALI2cGenerateStop();

    OALI2cDisable();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  PageRead
//
//  This function reads data in page length from the I2C EEPROM
//
//  Parameters:
//      pAddr 
//          [in] Starting Address in EEPROM to write a page of data to
//      dwAddrLen 
//          [in] Length of Address (pAddr)
//      pPageBuf
//          [in]  Page buffer in write buffer
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL PageRead(LPBYTE pAddr, WORD wAddrLen, LPBYTE pReadBuf)
{
   
    OALI2cEnable(FALSE, FALSE, 0, bDiv);
    
    if(OALI2cGenerateStart(EEPROM_SLAVE_ADDR, TRUE) == FALSE)
    {
         EdbgOutputDebugString("ERROR: PageRead: Generate Start Failed");
         return(FALSE);
    }
    if(OALI2cWriteData(pAddr, wAddrLen, FALSE, FALSE, NULL, NULL) == FALSE)
    {
         EdbgOutputDebugString("ERROR: PageRead: Write EEPROM address Failed");
         return(FALSE);
    }
    if(OALI2cRepeatedStart(EEPROM_SLAVE_ADDR,FALSE, FALSE) == FALSE)
    {
         EdbgOutputDebugString("ERROR: PageRead: Repeated Start Failed");
         return(FALSE);
    }
   
    if(OALI2cReadData(pReadBuf, EEPROM_MAX_TRANSFER_SIZE, FALSE, FALSE, NULL/*&bStopped*/, NULL) == FALSE)
    {
        EdbgOutputDebugString("ERROR: PageRead: Read EEPROM data Failed");
        return(FALSE);
    }

    OALI2cDisable();

    return(TRUE);
}
//-----------------------------------------------------------------------------
//
//  Function:  IsWriteOperationComplete
//
//  This function read the status of the EEPROM and check if the write operation is complete
//
//  Parameters:
//      no parameter
//
//  Returns:
//      TRUE indicates that the write operation complete. FALSE indicates that the write operation is not yet complete
//
//-----------------------------------------------------------------------------
BOOL IsWriteOperationComplete()
{
    BOOL fReady = FALSE;
    BYTE c;

    OALI2cEnable(FALSE, FALSE, 0, bDiv);
    
    if(OALI2cGenerateStart(EEPROM_SLAVE_ADDR, FALSE) == FALSE)
    {
        goto endOfTest;        
    }
    if (OALI2cReadData(&c, 1, FALSE, FALSE, NULL, NULL) == FALSE)
    {
        goto endOfTest;
    }

    if (c==0)   //The EEPROM return 0 when it's ready
    {
        fReady = TRUE;
    }
     

endOfTest:
    OALI2cDisable();

    return(fReady);
}
