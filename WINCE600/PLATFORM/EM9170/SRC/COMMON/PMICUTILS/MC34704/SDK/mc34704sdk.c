//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File mc34704sdk.c
// Support MC34704 PMIC via I2C interface
//
//------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
#include <bsp.h>
#include "i2cbus.h"
#include "mc34704.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

#define I2C_DEVICE_NAME         I2C1_FID
#define I2C_SCLK_FREQ           400000

// PMIC MC34704 default I2C address
#define I2C_PMIC_ADDR           (0x54 )

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Variables
//-----------------------------------------------------------------------------
static HANDLE hI2C = INVALID_HANDLE_VALUE;

static I2C_PACKET g_I2CPacket = 
{
    0, 
    0, 
    0, 
    0, 
    0
};

static I2C_TRANSFER_BLOCK g_I2CXferBlock=
{
    &g_I2CPacket,
    1
};


//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  Function: PmicInit
//
//  This function provides platform-specific initialization for supporting 
//  MC34704 PMIC.
//
//  Parameters:
//      I2C_DEVICE_NAME:  i2c interface device name.
//      bySlaveAddr:  MC34704 PMIC I2C interface slave device address.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL PmicOpen(VOID)
{
    // Open the I2C port use for PMIC
    hI2C = I2COpenHandle(I2C_DEVICE_NAME);

    if (hI2C ==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    // Initialize I2C packet
    g_I2CPacket.byAddr=I2C_PMIC_ADDR;
    g_I2CPacket.byRW=I2C_RW_WRITE;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: PMICDeInit
//
//  This function deinitializes the MC34704 support.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
 void PmicClose(void)
{
    if (hI2C !=INVALID_HANDLE_VALUE)
    {
        // Close the I2C port.
        CloseHandle(hI2C);
    }
    
}

//-----------------------------------------------------------------------------
//
//  Function: PmicEnable
//
//  This function enable or disable the MC34704 chip.
//
//  Parameters:
//      bEnable
//          [in] Wanted pmic state. (TRUE for enable)
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
 VOID PmicEnable(BOOL bEnable)
{
    if (bEnable == FALSE)
    {
        PmicSetRegister(GENERAL2,0);
    }
    else
    {
        PmicSetRegister(GENERAL2,GEN2_ONOFFA);
        PmicSetRegister(GENERAL2,GEN2_ONOFFA | GEN2_ONOFFE);
    }
}

//-----------------------------------------------------------------------------
//
// Function: PmicRegulatorSetVoltageLevel
//
// This method set target regulator voltage to target voltage code.
//
// Parameters:
//      regulatorCode: regulator identifier
//      voltageCode: wanted voltage level
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
 BOOL PmicRegulatorSetVoltageLevel(MC34704_REGULATOR regulatorCode, BYTE voltageCode)
{
    BOOL bRet = TRUE;
    MC34704_REGISTER regToWrite = RESERVEC;

    switch(regulatorCode)
    {
    case MC34704_REGULATOR1:
        RETAILMSG(1, (L"Regulator 1 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR2:
        RETAILMSG(1, (L"Regulator 2 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR3:
        regToWrite = REG3SET1;
        break;
    case MC34704_REGULATOR4:
        RETAILMSG(1, (L"Regulator 4 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR5:
        RETAILMSG(1, (L"Regulator 5 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR6:
        RETAILMSG(1, (L"Regulator 6 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR7:
        RETAILMSG(1, (L"Regulator 7 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR8:
        RETAILMSG(1, (L"Regulator 8 management not implemented"));
        bRet = FALSE;
        break;
    default:
        RETAILMSG(1,(L" Regulator not supported"));
        bRet = FALSE;
    }

    if (bRet == TRUE)
    {
        bRet = PmicSetRegister(regToWrite, (voltageCode<<1));
    }

    return bRet;
}

//-----------------------------------------------------------------------------
//
// Function: PmicRegulatorGetVoltageLevel
//
// This method get target regulator voltage.
//
// Parameters:
//      regulatorCode: regulator identifier
//      voltageCode: voltage level
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
BOOL PmicRegulatorGetVoltageLevel(MC34704_REGULATOR regulatorCode, PBYTE voltageCode)
{
    BOOL bRet = TRUE;
    MC34704_REGISTER regToWrite = RESERVEC;
    BYTE tmpVoltageCode;

    switch(regulatorCode)
    {
    case MC34704_REGULATOR1:
        RETAILMSG(1, (L"Regulator 1 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR2:
        RETAILMSG(1, (L"Regulator 2 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR3:
        regToWrite = REG3SET1;
        break;
    case MC34704_REGULATOR4:
        RETAILMSG(1, (L"Regulator 4 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR5:
        RETAILMSG(1, (L"Regulator 5 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR6:
        RETAILMSG(1, (L"Regulator 6 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR7:
        RETAILMSG(1, (L"Regulator 7 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR8:
        RETAILMSG(1, (L"Regulator 8 management not implemented"));
        bRet = FALSE;
        break;
    default:
        RETAILMSG(1,(L" Regulator not supported"));
        bRet = FALSE;
    }

    if (bRet == TRUE)
    {
        bRet = PmicGetRegister(regToWrite, &tmpVoltageCode);
        if (bRet == TRUE)
        {
            *voltageCode = ((tmpVoltageCode>>1) & 0x0F);
        }
    }

    return bRet;
}


//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Function: PmicSetRegister
//
// This method set MC34704 registers via i2c interface.
//
// Parameters:
//      reg: register address
//      byVal: value to set
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
BOOL PmicSetRegister(MC34704_REGISTER reg, BYTE byVal)
{
    BYTE byBuf[2];
    INT iResult;
    DWORD dwFreq;
    BOOL bRet = TRUE;

    if (hI2C !=INVALID_HANDLE_VALUE)
    {
        iResult = 1;
        byBuf[0]=(BYTE)reg;
        byBuf[1]=byVal;
        
        g_I2CPacket.byRW=I2C_RW_WRITE;
        g_I2CPacket.pbyBuf=byBuf;
        g_I2CPacket.wLen=2;
        g_I2CPacket.lpiResult=&iResult;

        dwFreq = I2C_SCLK_FREQ;
        I2CSetFrequency(hI2C, dwFreq);
        I2CTransfer(hI2C, &g_I2CXferBlock);
        
        if (iResult==I2C_NO_ERROR)
        {
            bRet = TRUE;
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}


//-----------------------------------------------------------------------------
//
// Function: PmicGetRegister
//
// This method get MC34704 registers via i2c interface.
//
// Parameters:
//      reg: register address
//      pData: got value
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-------------------------------------------------------------------------------
BOOL PmicGetRegister(MC34704_REGISTER reg, PBYTE pData)
{
    I2C_TRANSFER_BLOCK I2CXferBlock;
    I2C_PACKET I2CPacket[2];
    INT iResult[2];
    BYTE byOutData;
    BYTE byInData;
    BYTE byBuf[2];
    
    byBuf[0]=(BYTE)reg|0x80;
    I2CPacket[0].pbyBuf = (PBYTE)byBuf;
    I2CPacket[0].wLen = sizeof(byOutData);

    I2CPacket[0].byRW = I2C_RW_WRITE;
    I2CPacket[0].byAddr = I2C_PMIC_ADDR;
    I2CPacket[0].lpiResult = &iResult[0];

    I2CPacket[1].pbyBuf = (PBYTE) pData;
    I2CPacket[1].wLen = sizeof(byInData);

    I2CPacket[1].byRW = I2C_RW_READ;
    I2CPacket[1].byAddr = I2C_PMIC_ADDR;
    I2CPacket[1].lpiResult = &iResult[1];

    I2CXferBlock.pI2CPackets = I2CPacket;
    I2CXferBlock.iNumPackets = 2;

    if(I2CTransfer(hI2C, &I2CXferBlock))
    {
        return TRUE;
    }
    else 
    { 
        return FALSE;
    }
  
}
