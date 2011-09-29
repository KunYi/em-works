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
//------------------------------------------------------------------------------
//
//  File:  i2c.c
//
//  This I2C.c file is local to the RTC communication.

#include <windows.h>
#include <oal.h>
#include <omap2420.h>
#include "kerneli2c.h"

//------------------------------------------------------------------------------

#define I2C_MENELAUS_ADDRESS        0x72


//------------------------------------------------------------------------------
// Function : WriteRTCCtrlData
//
// This function writes the control data to the register address of RTC hardware

BOOL WriteRTCCtrlData(UCHAR reg, UCHAR data)
{
    I2CTRANS trans;

    ZeroMemory(&trans,sizeof(trans));
    
    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;

    /* just write the register # then the data in one shot */
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mTransLen[0] = 2;
    trans.mBuffer[0] = reg;
    trans.mBuffer[1] = data;

    KERNELI2C_PreemptibleSubmit(I2C_MENELAUS_ADDRESS,7,&trans);

    return (trans.mErrorCode==0);
}

//------------------------------------------------------------------------------
// Function : ReadRTCCtrlData
//
// This function reads the control data from the register address of RTC hardware

BOOL ReadRTCCtrlData( UCHAR reg, UCHAR *pData )
{
    I2CTRANS trans;

    ZeroMemory(&trans,sizeof(trans));

    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;
    /* first write register address */
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mTransLen[0] = 1;
    trans.mBuffer[0] = reg;
    /* then read back data from that address */
    trans.mOpCode[1] = I2C_OPCODE_READ;
    trans.mBufferOffset[1] = 0;
    trans.mTransLen[1] = 1;

    KERNELI2C_PreemptibleSubmit(I2C_MENELAUS_ADDRESS,7,&trans);

    if (trans.mErrorCode)
        return FALSE;

    *pData = trans.mBuffer[0];
    
    return TRUE;
}

//----------------------------------------------------------------------------
// Function : ReadRTCTimeData
//
// This function reads the RTC time information into the buffer passed from
// RTC Read function.

BOOL ReadRTCTimeData  ( PVOID pBuffer, DWORD size )
{
    I2CTRANS trans;
    DWORD i;
    UCHAR RegOffset;
    UCHAR *pData;

    ZeroMemory(&trans,sizeof(trans));

    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;

    /* first byte in buffer is register # offset */
    pData = (UCHAR *)pBuffer;
    RegOffset = *pData;
    pData++;

    for (i=0;i<size;i++)
    {
        /* write register address */
        trans.mOpCode[0] = I2C_OPCODE_WRITE;
        trans.mBufferOffset[0] = 0;
        trans.mTransLen[0] = 1;
        trans.mBuffer[0] = RegOffset;
        /* read data from that register */
        trans.mOpCode[1] = I2C_OPCODE_READ;
        trans.mBufferOffset[1] = 0;
        trans.mTransLen[1] = 1;

        KERNELI2C_PreemptibleSubmit(I2C_MENELAUS_ADDRESS,7,&trans);

        if (trans.mErrorCode)
            break;

        /* write returned data into the output buffer */
        *pData = trans.mBuffer[0];
        pData++;
        RegOffset++;
    }

    if (trans.mErrorCode)
        return FALSE;

    return TRUE;
}
 
//------------------------------------------------------------------------------
// Function : WriteRTCTimeData
//
// This function writes the RTC data passed by the RTCWrite function into the 
// RTC hardware 

BOOL WriteRTCTimeData ( PUCHAR pBuffer, DWORD size  )
{
    I2CTRANS trans;
    DWORD i;
    UCHAR RegOffset;
    UCHAR *pData;

    ZeroMemory(&trans,sizeof(trans));

    trans.mClk_HL_Divisor = I2C_CLOCK_100Khz;

    /* first byte in buffer is register # offset */
    pData = (UCHAR *)pBuffer;
    RegOffset = *pData;
    pData++;

    for (i=0;i<size;i++)
    {
        /* write register address, then data for that register */
        trans.mOpCode[0] = I2C_OPCODE_WRITE;
        trans.mBufferOffset[0] = 0;
        trans.mTransLen[0] = 2;
        trans.mBuffer[0] = RegOffset;
        trans.mBuffer[1] = *pData;

        KERNELI2C_PreemptibleSubmit(I2C_MENELAUS_ADDRESS,7,&trans);

        if (trans.mErrorCode)
            break;

        /* update for next write */
        pData++;
        RegOffset++;
    }

    if (trans.mErrorCode)
        return FALSE;

    return TRUE;
}   

//----------------------------------------------------------------------------
