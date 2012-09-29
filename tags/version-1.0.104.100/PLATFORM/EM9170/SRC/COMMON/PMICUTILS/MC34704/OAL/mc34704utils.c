//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File mc34704utils.c
// Support MC34704 PMIC via I2C interface for OAL
//
//------------------------------------------------------------------------------

#include <bsp.h>
#include "i2cbus.h"
#include "mc34704.h"


//-----------------------------------------------------------------------------
// Extern Functions
//-----------------------------------------------------------------------------
extern BOOL OALI2cInit( BYTE *pbySelfAddr, BYTE *pbyClkDiv );
extern void OALI2cEnable( BOOL bChangeAddr, BOOL bChangeClkDiv, BYTE bySelfAddr, BYTE byClkDiv );
extern void OALI2cDisable();
extern WORD OALI2cCalculateClkRateDiv( DWORD dwFrequency );
extern BOOL OALI2cGenerateStart( BYTE devAddr, BOOL bWrite );
extern BOOL OALI2cRepeatedStart( BYTE devAddr, BOOL bWrite, BOOL bRSTACycleComplete );;
extern void OALI2cGenerateStop();
extern BOOL OALI2cWriteData( BYTE *pData, WORD nBufLen, 
                     BOOL bStopAhead, 
                     BOOL bRepeatedStartCycleAhead, 
                     BOOL *pbStopped, 
                     BOOL *pbRSTACycleCompleted );
extern BOOL OALI2cReadData( BYTE *pData, WORD nBufLen, 
                    BOOL bStopAhead, 
                    BOOL bRepeatedStartCycleAhead, 
                    BOOL *pbStopped, 
                    BOOL *pbRSTACycleCompleted );
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

#define I2C_DEVICE_NAME         _T("I2C1:")
#define I2C_SCLK_FREQ           400000

#define I2C_PMIC_ADDR      (0xA8>>1)

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
static I2C_PACKET g_I2CPacket = 
{
    I2C_PMIC_ADDR, 
    I2C_RW_WRITE, 
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
// Local Functions
//-----------------------------------------------------------------------------
BOOL PmicSetRegister(MC34704_REGISTER reg, BYTE byVal);


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
    OALI2cInit(0, 0);

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
VOID PmicClose(VOID)
{

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
    MC34704_REGISTER regToWrite=RESERVEC;

    switch(regulatorCode)
    {
    case MC34704_REGULATOR1:
        RETAILMSG(1, (L"Regulator 1 management not implemented"));
        bRet = FALSE;
        break;
    case MC34704_REGULATOR2:
        regToWrite = REG2SET1;
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
    UNREFERENCED_PARAMETER(regulatorCode);
    UNREFERENCED_PARAMETER(voltageCode);
    RETAILMSG(1,(L"Not implemented"));
    return FALSE;
}

//-----------------------------------------------------------------------------
// Local Functions
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
    WORD wClkDiv;
    BOOL bRet = TRUE;

    wClkDiv=OALI2cCalculateClkRateDiv(I2C_SCLK_FREQ);
    OALI2cEnable(FALSE, TRUE, 0, (BYTE)wClkDiv);

    byBuf[0]=(BYTE)reg;
    byBuf[1]=byVal;

    if (!OALI2cGenerateStart(I2C_PMIC_ADDR, TRUE))
    {
        bRet = FALSE;
        goto exit;
    }

    if (!OALI2cWriteData(byBuf, 2, FALSE, FALSE, NULL, NULL))
    {
        bRet = FALSE;
        goto exit;
    }

    OALI2cGenerateStop();
    OALI2cDisable();

exit:
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
//      byVal: value to set
//
// Returns:
//      TRUE:  success
//      FALSE: fail 
//
//-----------------------------------------------------------------------------
BOOL PmicGetRegister(MC34704_REGISTER reg, BYTE* pbyVal)
{
    BYTE byData=0;
    WORD wClkDiv;
    BOOL bRet = TRUE;

    wClkDiv=OALI2cCalculateClkRateDiv(I2C_SCLK_FREQ);
    OALI2cEnable(FALSE, TRUE, 0, (BYTE)wClkDiv);

    if (!OALI2cGenerateStart(I2C_PMIC_ADDR,TRUE))
    {
        OALMSGS( OAL_ERROR, (_T("PmicGetRegister: OALI2cGenerateStart Error!\r\n")));
        bRet = FALSE;
        goto exit;
    }
    
    reg|=0x80;
    if (!OALI2cWriteData((BYTE*)&reg,1,FALSE, FALSE, NULL, NULL))
    {
        OALMSGS( OAL_ERROR, (_T("PmicGetRegister: OALI2cWriteData Set Reg Error!\r\n")));
        bRet = FALSE;
        goto exit;
    }
    
    if (!OALI2cRepeatedStart(I2C_PMIC_ADDR,FALSE, FALSE))
    {
        OALMSGS( OAL_ERROR, (_T("PmicGetRegister: OALI2cGenerateStart Error!\r\n")));
        bRet = FALSE;
        goto exit;
    }
    
    if (!OALI2cReadData(&byData,sizeof(byData), TRUE, FALSE, NULL, NULL))
    {
        OALMSGS( OAL_ERROR, (_T("PmicGetRegister: OALI2cReadData Set byData Error!\r\n")));
        bRet = FALSE;
        goto exit;
    }
    if (pbyVal)
    {
        *pbyVal = byData;
    }
     
    //OALI2cGenerateStop();

exit:
    OALI2cDisable();
    return bRet;
}
