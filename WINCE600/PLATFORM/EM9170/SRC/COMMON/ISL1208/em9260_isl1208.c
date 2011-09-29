//**********************************************************************
//                                                                      
// Filename: em9260_isl1208.c
//                                                                      
// Description: 
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Use of this source code is subject to the terms of the Cirrus end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to 
// use this source code. For a copy of the EULA, please see the 
// EULA.RTF on your install media.
//
// Copyright(c) Emtronix 2007, All Rights Reserved
//                                                                      
//**********************************************************************
#include <windows.h>
#include <oal.h>
#include <nkintr.h>

#include "AT91SAM9260.h"
#include "em9260_isl1208.h"

// Controller includes
#include "AT91SAM926x_oal_ioctl.h"

#define TIME_TEST	0
#define TIME_ERROR	1

DWORD ComputeTwiClock( DWORD dwTransferRate );

int AT91_TWI_Init( )
{
	DWORD      dwFlags, dwTmp;
	AT91PS_TWI pTWIReg;
	AT91PS_PMC pPMCReg;
	AT91PS_PIO pPIOAReg;

	//pTWIReg = (AT91PS_TWI)TWI_BASE;
	pTWIReg = (AT91PS_TWI)OALPAtoVA((DWORD)AT91C_BASE_TWI, FALSE);
	//pPMCReg = (AT91PS_PMC)PMC_BASE;
	pPMCReg  = (AT91PS_PMC)OALPAtoVA((DWORD)AT91C_BASE_PMC, FALSE);
	//pPIOAReg = (AT91PS_PIO)PIOA_BASE; 
	pPIOAReg = (AT91PS_PIO)OALPAtoVA((DWORD)AT91C_BASE_PIOA, FALSE);

	//enable twi clock
	dwFlags = (DWORD)1 << AT91C_ID_TWI;
	dwTmp = pPMCReg->PMC_PCSR;
	if( (dwTmp & dwFlags) != dwFlags )
	{
		pPMCReg->PMC_PCER = dwFlags;
	}

	//disable PIOA23, PIOA24
	dwTmp = pPIOAReg->PIO_PSR;
	if( (dwTmp & (AT91C_PIO_PA23 | AT91C_PIO_PA24)) != 0 )
	{
		pPIOAReg->PIO_PDR = AT91C_PIO_PA23 | AT91C_PIO_PA24;
	}

	//select peripheral A
	dwTmp = pPIOAReg->PIO_ABSR;
	if( (dwTmp & (AT91C_PA23_TWD | AT91C_PA24_TWCK)) != 0 )
	{
		pPIOAReg->PIO_ASR = AT91C_PA23_TWD | AT91C_PA24_TWCK;
	}

	//get divisor for 200KHz
	dwTmp = ComputeTwiClock( 200000 );
	
	//reset I2C controller
	pTWIReg->TWI_IDR = (DWORD)0xffffffff;		//mask all interrupt
	pTWIReg->TWI_CR = AT91C_TWI_SWRST;			//soft reset
	pTWIReg->TWI_CR = AT91C_TWI_MSEN;			//set master mode
	pTWIReg->TWI_CWGR = dwTmp;					//set transfer rate 
	//Sleep( 4 );

	return 0;
}

//
// dwTransferRate -> Unit=bit per second
//
DWORD ComputeTwiClock( DWORD dwTransferRate )
{
#define CLDIV_MAX 0xFF
#define CKDIV_MAX 0x7

	int   iClockDiv;
	DWORD dwCLDIV;		// CLDIV == CKDIV
	DWORD dwCKDIV;
		
	DWORD dwMasterClock;

	dwMasterClock = 50070885;		//about 50MHz
	//KernelIoControl(IOCTL_HAL_MASTERCLOCK, NULL, 0, &dwMasterClock, sizeof(dwMasterClock), 0);
		
	// (dwTransfert * 2) because we configure a whole periode (high periode and low periode)
	iClockDiv = (dwMasterClock / (dwTransferRate * 2));	

	iClockDiv -= 6;

	// at this point the computation is iClockDiv = dwCLDIV * (1<<dwCKDIV);
	dwCLDIV = iClockDiv;
	dwCKDIV = 0;
	while (dwCLDIV>CLDIV_MAX)
	{
		dwCLDIV >>= 1;
		dwCKDIV++;
	}

	if (dwCKDIV>CKDIV_MAX)
	{
		dwCKDIV = CKDIV_MAX;
		dwCLDIV = CLDIV_MAX;
	}

	return (DWORD) (dwCKDIV<<16) | (dwCLDIV << 8) | (dwCLDIV);
}

#define TIMELIMIT 1000000

//=========================================================
//		WRITE
//=========================================================
//----------------------------------------------------------------------------
// int  int_address: ISL1208 internal register address
// char data2send:   the data want to write into the register
//
// return =   0: write ok
//        = -12: timeout error
//        = -13: over run error
//        = -14: under run error
//        = -15: nack error
//----------------------------------------------------------------------------
int AT91_TWI_WriteByte( int int_address, char data2send )
{
	DWORD         status;
	AT91PS_TWI    pTWIReg;
	volatile long counter = 0;

	//pTWIReg = (AT91PS_TWI)TWI_BASE;
	pTWIReg = (AT91PS_TWI)OALPAtoVA((DWORD)AT91C_BASE_TWI, FALSE);

	// Set TWI Internal Address Register
	pTWIReg->TWI_IADR = int_address;

	// Set the TWI Master Mode Register
	pTWIReg->TWI_MMR = ( ((DWORD)ISL1208_ADDRESS<<16) | AT91C_TWI_IADRSZ_1_BYTE ) & ~AT91C_TWI_MREAD;	

	// setup write operation
	pTWIReg->TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN | AT91C_TWI_STOP;
		
	//as soon as data put in, write process start
	pTWIReg->TWI_THR = data2send;

	status = pTWIReg->TWI_SR;
	counter = TIMELIMIT;
	while (!(status & AT91C_TWI_TXCOMP))
	{
    	status = pTWIReg->TWI_SR;
		counter--;

		if( (counter < 0) || (status & (AT91C_TWI_OVRE | AT91C_TWI_UNRE | AT91C_TWI_NACK)) )
		{
			if(status&AT91C_TWI_OVRE)
			{
				//RETAILMSG(1,(TEXT("I2C Error: AT91C_TWI_OVRE\r\n")));
				return -13;
			}

			if(status&AT91C_TWI_UNRE)
			{
				//RETAILMSG(1,(TEXT("I2C Error: AT91C_TWI_UNRE\r\n")));
				return -14;
			}

			if(status&AT91C_TWI_NACK)
			{
				//RETAILMSG(1,(TEXT("I2C Error: AT91C_TWI_NACK\r\n")));
				return -15;
			}
			
			if(counter<0)
			{
				//RETAILMSG(1,(TEXT("I2C Error: Timeout\r\n")));
				return -12;	
			}
		}
		else
		{
			//wait transmit complete...
		}
    }

	return 0;
}

//=========================================================
//		READ
//=========================================================
//----------------------------------------------------------------------------
// int   int_address: ISL1208 internal register address
// char* pdata:       pointer to contain the data read from the register
//
// return =   0: read ok
//        = -12: timeout error
//        = -13: over run error
//        = -14: under run error
//        = -15: nack error
//----------------------------------------------------------------------------
int AT91_TWI_ReadByte(int int_address, char *pdata )
{	
	DWORD         status;
	AT91PS_TWI    pTWIReg;
	volatile long counter = 0;

	//pTWIReg = (AT91PS_TWI)TWI_BASE;
	pTWIReg = (AT91PS_TWI)OALPAtoVA((DWORD)AT91C_BASE_TWI, FALSE);

	// Set the TWI Master Mode Register
	pTWIReg->TWI_MMR = ((DWORD)ISL1208_ADDRESS<<16) | AT91C_TWI_IADRSZ_1_BYTE | AT91C_TWI_MREAD;	
	
	// Set TWI Internal Address Register
	pTWIReg->TWI_IADR = int_address;

	// Start transfer
	pTWIReg->TWI_CR = AT91C_TWI_MSEN | AT91C_TWI_START;
	status = pTWIReg->TWI_SR;	
	pTWIReg->TWI_CR = AT91C_TWI_STOP;
	status = pTWIReg->TWI_SR;	
	counter = TIMELIMIT;
	while (!(status & AT91C_TWI_TXCOMP))
	{
		status = pTWIReg->TWI_SR;		   
		counter--;
		if( (counter < 0) || (status & (AT91C_TWI_OVRE | AT91C_TWI_UNRE | AT91C_TWI_NACK)) )
		{
			if(status&AT91C_TWI_OVRE)
			{
				//RETAILMSG(1,(TEXT("I2C Error: AT91C_TWI_OVRE\r\n")));
				return -13;
			}

			if(status&AT91C_TWI_UNRE)
			{
				//RETAILMSG(1,(TEXT("I2C Error: AT91C_TWI_UNRE\r\n")));
				return -14;
			}

			if(status&AT91C_TWI_NACK)
			{
				//RETAILMSG(1,(TEXT("I2C Error: AT91C_TWI_NACK\r\n")));
				return -15;
			}
			
			if(counter<0)
			{
				//RETAILMSG(1,(TEXT("I2C Error: Timeout\r\n")));
				return -12;	
			}
		}
		else
		{
			//RETAILMSG(1,(TEXT("Wait...2\r\n")));
		}
    }
	//if(status & AT91C_TWI_RXRDY){
	*pdata = (char)(pTWIReg->TWI_RHR);
	//}	

	return 0;
}

//=========================================================================================
// Initialize ISL1208
// return = 0: the first time of battery backup
//        = 1: ok!
//        < 0: fail
//=========================================================================================
int ISL1208_Init( )
{
	UCHAR status;
	int   i1, bRTCValid;
	
	// setup TWI controller
	AT91_TWI_Init( );

	i1 = AT91_TWI_ReadByte( REG1208_STATUS, (char*)&status );
	if( i1 < 0 )
	{
		RETAILMSG(TIME_ERROR,(TEXT("Test ISL1208, failed to read\r\n")));
		return i1;
	}
	RETAILMSG(TIME_ERROR,(TEXT("RTC status %x\r\n"),status));

	if( status & 0x01 )
	{
		bRTCValid = 0;		//the first time of battery backup
	}
	else
	{
		bRTCValid = 1;
	}

	if( !(status&STATUS_WRTC) || (status & 0x01))
	{
		status = STATUS_ARST | STATUS_WRTC;
		i1 = AT91_TWI_WriteByte( REG1208_STATUS, status );
		if( i1 < 0 )
		{
			RETAILMSG(TIME_ERROR,(TEXT("Init RTC failed\r\n")));
			return i1;
		}

		/*//////////////////////////////////////////////////
		const ISL1208_ATR_PAIR CL_Tab[] =
		{
			{0x2c, "7.50pF"},
			{0x2d, "7.75pF"},
			{0x2e, "8.00pF"},
			{0x2f, "8.25pF"},
			{0x30, "8.50pF"},
			{0x31, "8.75pF"},
			{0x32, "9.00pF"},
			{0x33, "9.25pF"},
			{0x34, "9.50pF"},
			{0x35, "9.75pF"},
			{0x36, "10.00pF"},
			{0x37, "10.25pF"},
			{0x38, "10.50pF"},
			{0x39, "10.75pF"},
			{0x3a, "11.00pF"},
			{0x3b, "11.25pF"},
			{0x3c, "11.50pF"},		//Choose it!!!!
			{0x3d, "11.75pF"},
			{0x3e, "12.00pF"},
			{0x3f, "12.25pF"},
			{0x00, "12.50pF"},
			{0x01, "12.75pF"},
			{0x02, "13.00pF"},
			{0x03, "13.25pF"},
			{0x04, "13.50pF"},
			{0x05, "13.75pF"},
			{0x06, "14.00pF"},
			{0x07, "14.25pF"},
			{0x08, "14.50pF"},
			{0x09, "14.75pF"},
			{0x0a, "15.00pF"},
			{0x0b, "15.25pF"},
			{0x0c, "15.50pF"},
			{0x0d, "15.75pF"},
			{0x0e, "16.00pF"},
			{0x0f, "16.25pF"},
			{0x10, "16.50pF"},
			{0x11, "16.75pF"},
			{0x12, "17.00pF"},
			{0x13, "17.25pF"},
			{0x14, "17.50pF"},
			{0x15, "17.75pF"},
			{0x16, "18.00pF"},
			{0x17, "18.25pF"},
			{0x18, "18.50pF"},
			{0x19, "18.75pF"},
			{0x1a, "19.00pF"},
			{0x1b, "19.25pF"},
			{0x1c, "19.50pF"},
			{0x1d, "19.75pF"},
			{0x1e, "20.00pF"},
		};
		//////////////////////////////////////////////////*/
		//----------------------------------------------------------
		// CS SEP-18-2007: ISL1208 Analog Trimming Register Adjust:
		// ATR       CL          Fout
		//----------------------------------------------------------
		// 0x00    12.50pF      32767.3Hz
		// 0x38    10.50pF      32767.6Hz
		// 0x34     9.50pF      32767.8Hz
		// 0x33     9.25pF      32767.8Hz
		// 0x32     9.00pF      32767.9Hz
		// 0x31     8.75pF      32767.9Hz
		// 0x30     8.50pF      32768.0Hz		//Choose it!!!!
		// 0x2f     8.25pF      32768.1Hz
		// 0x2e     8.00pF      32768.1Hz
		// 0x2d     7.75pF      32768.2Hz
		// 0x2c     7.50pF      32768.3Hz
		//----------------------------------------------------------
		//The Crystal Load Capacitor CL = 12.5pF
		//adjust ISL1208 Load Capacitor => 11.5pF
		status = 0x3c;
		i1 = AT91_TWI_WriteByte( REG1208_ATR, status );
		if( i1 < 0 )
		{
			RETAILMSG(TIME_ERROR,(TEXT("Write RTC REG1208_ATR failed\r\n")));
			return i1;
		}
	}

	//
	// CS&ZHL AUG-21-2008: always set Fout = 32768Hz for manufacture testing
	//
	i1 = AT91_TWI_ReadByte( REG1208_INT, &status );		//read INT control register
	if( i1 < 0 )
	{
		RETAILMSG(TIME_ERROR,(TEXT("Read REG1208_INT failed\r\n")));
		return i1;
	}
    status &= 0xf0;
	status |= 0x01;			
	i1 = AT91_TWI_WriteByte( REG1208_INT, status );
	if( i1 < 0 )
	{
		RETAILMSG(TIME_ERROR,(TEXT("Write REG1208_INT failed\r\n")));
		return i1;
	}

	return bRTCValid;
}

__inline BYTE GETBCDDATA(BYTE data )  
{
	return  ( (data>>4)&0xF) *10 + ( (data)&0xF);
}

__inline BYTE SETBCDDATA( WORD data )  
{
	return  ((data/10)<<4) | (data %10 );
}


BOOL ISL1208SetRealTime(LPSYSTEMTIME ptime)
{
	int   i1, i2; 
	BYTE  buffer[7];
	BOOL  bRet=FALSE;

	RETAILMSG(TIME_ERROR,(TEXT("SetRealTime %d/%d/%d\r\n")
		,ptime->wYear
		,ptime->wMonth
		,ptime->wDay));

	if( ptime->wYear >= 2000 )
	{
		buffer[REG1208_SECOND] = SETBCDDATA(ptime->wSecond);
		buffer[REG1208_MIN] = SETBCDDATA(ptime->wMinute);
		buffer[REG1208_HOUR] = SETBCDDATA(ptime->wHour) | 0x80; //set 24h.
		buffer[REG1208_DAY] = SETBCDDATA(ptime->wDay);
		buffer[REG1208_MONTH] = SETBCDDATA(ptime->wMonth);
		buffer[REG1208_YEAR] = SETBCDDATA(ptime->wYear-2000);
		buffer[REG1208_WEEK] = SETBCDDATA(ptime->wDayOfWeek);

		for( i2=0; i2<7; i2++ )
		{
			i1 = AT91_TWI_WriteByte( i2, buffer[i2] );
			if( i1 < 0 )
			{
				break;
			}
		}
		if( i2 >= 7 )
		{
			RETAILMSG(TIME_ERROR,(TEXT("SetRealTime OK\r\n")));
			bRet=TRUE;
		}
	}
	else
	{
		RETAILMSG(TIME_ERROR,(TEXT("SetRealTime fail\r\n")));
	}

	//ISL1208GetRealTime(  ptime );
	return bRet;
}


BOOL ISL1208GetRealTime( LPSYSTEMTIME  ptime )
{
	int   i1, i2;
	BYTE  buffer[7] = {0};
	BOOL  bRet=TRUE;

	for( i2=0; i2<7; i2++ )
	{
		i1 = AT91_TWI_ReadByte( i2, (char*)&buffer[i2] );
		if( i1 < 0 )
		{
			bRet=FALSE;
			break;
		}
	}

	//convert time, if read ok
	if( bRet )
	{
		ptime->wSecond=GETBCDDATA(buffer[REG1208_SECOND]);
		ptime->wMinute=GETBCDDATA(buffer[REG1208_MIN]);
		ptime->wHour=GETBCDDATA(buffer[REG1208_HOUR]& 0x3F);
		ptime->wDay=GETBCDDATA(buffer[REG1208_DAY]);
		ptime->wMonth=GETBCDDATA(buffer[REG1208_MONTH]);
		ptime->wYear=GETBCDDATA(buffer[REG1208_YEAR])+2000;
		ptime->wDayOfWeek=GETBCDDATA(buffer[REG1208_WEEK]);
	}

	return bRet;
}

BOOL InitializeISL1208(LPSYSTEMTIME ptime)
{
	BOOL bRet=TRUE;
	int  islStatus;

	islStatus = ISL1208_Init(  );
	if( islStatus < 0 )
		return FALSE;

	if( islStatus == 1 )
	{
		bRet=ISL1208GetRealTime( ptime );
	}
	else
	{
		bRet=ISL1208SetRealTime( ptime );
	}

	return bRet;
}

