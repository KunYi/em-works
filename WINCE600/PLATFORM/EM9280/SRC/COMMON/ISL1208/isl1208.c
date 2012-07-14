//************************************************************************
//                                                                      
// Filename: ISL1208.c
//                                                                      
// Copyright(c) Cirrus Logic Corporation 2004, All Rights Reserved                       
//
// Author: YUJIANG.ZHENG@CIRRUS.BEIJING
//
// TODOTODOTODO: Replace all while{} with timeout function.
//
//************************************************************************
#include <bsp.h>
#include "isl1208.h"
#include "em9280_oal.h"

#define TIME_TEST					0
#define TIME_ERROR					1

//
// CS&ZHL MAR-18-2012: supporting I2C bus for RTC and GPIOX
//
extern CRITICAL_SECTION		g_oalI2cMutex;
extern BOOL OALI2C_Access(VOID* pInpBuffer, UINT32 inpSize);
// CS&ZHL JUN-7-2012: it's necessary to check the time read from ISL1208
extern BOOL CheckRealTime(LPSYSTEMTIME lpst);

//
// CS&ZHL NOV-24-2008: Write the registers of ISL1208 based on ETR232H's code 
//
BOOL write_ISL1208(int address, UCHAR* pdata, int nbytes)
{
	I2cAccessInfo   I2C_Info;
	BOOL			bRet;

	I2C_Info.dwAccessCode = I2C_ACCESS_CODE_WRITE;
	I2C_Info.dwAddr = ISL1208_I2C_DEVICE_ADDRESS;	// I2C device hardware address
	I2C_Info.dwCmd = (DWORD)address;				// ISL1208 register address
	I2C_Info.dwDataLength = (DWORD)nbytes;
	I2C_Info.pDataBuf = (PBYTE)pdata;

	EnterCriticalSection(&g_oalI2cMutex);
	bRet = OALI2C_Access((VOID*)&I2C_Info, sizeof(I2cAccessInfo));
	LeaveCriticalSection(&g_oalI2cMutex);

	return bRet;
}

//
// CS&ZHL NOV-24-2008: Read the registers of ISL1208 based on ETR232H's code
//
BOOL read_ISL1208(int address, UCHAR* pdata, int nbytes)
{
	I2cAccessInfo   I2C_Info;
	BOOL			bRet;

	I2C_Info.dwAccessCode = I2C_ACCESS_CODE_READ;
	I2C_Info.dwAddr = ISL1208_I2C_DEVICE_ADDRESS;	// I2C device hardware address
	I2C_Info.dwCmd = (DWORD)address;				// ISL1208 register address
	I2C_Info.dwDataLength = (DWORD)nbytes;
	I2C_Info.pDataBuf = (PBYTE)pdata;

	EnterCriticalSection(&g_oalI2cMutex);
	bRet = OALI2C_Access((VOID*)&I2C_Info, sizeof(I2cAccessInfo));
	LeaveCriticalSection(&g_oalI2cMutex);

	return bRet;
}

//-------------------------------------------------
// return = 1: RTC is valid, and setup already
//        = 0: RTC is valid, but not setup yet
//        < 0: RTC is invalid
//-------------------------------------------------
int init_ISL1208(void )
{
	UCHAR	buffer[8] = {5};
	UCHAR	status = 0;
	int		nRTCValid = -1;
	int		i1;	

    //OALMSG(TRUE, (L"+init_ISL1208\r\n"));
	//
	// CS&ZHL OCT-06-2008: check more than once
	//
	for(i1 = 0; i1 < 3; i1++ )
	{
		if( read_ISL1208( REG1208_STATUS, &status, 1 ) )
		{
			break;
		}
	}

	if(i1 >= 3)
	{
		OALMSG(TIME_ERROR, (L"init_ISL1208: read failed, status = 0x%x\r\n", status));
		return nRTCValid;
	}
	OALMSG(TIME_TEST, (L"init_ISL1208: RTC status = 0x%x\r\n", status));

	if( status & 0x01 )
	{
		// read YY-MM-DD registers
		read_ISL1208( REG1208_DAY, &buffer[3], 3 );		
		//
		// CS&ZHL OCT-09-2008: make sure power fail bit is true
		//
		if( (buffer[3] != 0x00) && (buffer[4] != 0x00) && (buffer[5] != 0x00) )
		{
			nRTCValid = 1;
		}
		else
		{
			nRTCValid = 0;
		}
	}
	else
	{
		nRTCValid = 1;
	}

	if( !(status & STATUS_WRTC) || (status & 0x01) )
	{
		//OALMSG(TRUE, (L"init_ISL1208: chip startup\r\n"));
		status = STATUS_WRTC | STATUS_ARST;
		if( !write_ISL1208(REG1208_STATUS, &status, 1) )
		{
			OALMSG(TIME_ERROR, (L"init_ISL1208: write REG1208_STATUS failed\r\n"));
			nRTCValid = -1;
			goto cleanup;
		}
		//OALMSG(TRUE, (L"init_ISL1208: force Fout = 32768Hz\r\n"));
		// Fout = 32768Hz for manufacture testing
		read_ISL1208( REG1208_INT, &status, 1 );		//read INT control register
        status &= 0xf0;
		status |= 0x01;			
		if( !write_ISL1208( REG1208_INT, &status, 1 ) )
		{
			OALMSG(TIME_ERROR, (L"init_ISL1208: write REG1208_INT failed\r\n"));
			nRTCValid = -1;
			goto cleanup;
		}

		//--------------------------------------------------------------------
		// CS&ZHL SEP-18-2007: CL = 8.5pF; ATR = 0x30
		// CS&ZHL AUG-20-2008: apply default setting CL = 14.5pF, ATR = 0x08
		//--------------------------------------------------------------------
		status = 0x08;
		if( !write_ISL1208( REG1208_ATR, &status, 1 ) )
		{
			OALMSG(TIME_ERROR, (L"init_ISL1208: write REG1208_ATR failed\r\n"));
			nRTCValid = -1;
			goto cleanup;
		}

		// mark default setting flag = 0x55
		status = 0x55;
		if( !write_ISL1208( REG1208_USER2, &status, 1 ) )
		{
			OALMSG(TIME_ERROR, (L"init_ISL1208: write REG1208_USER2 failed\r\n"));
			nRTCValid = -1;
			goto cleanup;
		}
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
		{0x3c, "11.50pF"},
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

cleanup:
    return nRTCValid;
}

__inline BYTE GETBCDDATA(BYTE data )  {

	return  ( (data>>4)&0xF) *10 + ( (data)&0xF);
 }

__inline BYTE SETBCDDATA( WORD data )  
{
	BYTE		ub = (UCHAR)data;
	return  ((ub/10)<<4) | (ub %10 );
	//return  ((data/10)<<4) | (data %10 );
}

BOOL ISL1208SetRealTime(LPSYSTEMTIME ptime)
{
	BOOL  bRet = FALSE;
	UCHAR buffer[8];

	// fill data
	buffer[REG1208_SECOND] = (UCHAR)SETBCDDATA(ptime->wSecond);
	buffer[REG1208_MIN]    = (UCHAR)SETBCDDATA(ptime->wMinute);
	buffer[REG1208_HOUR]   = (UCHAR)SETBCDDATA(ptime->wHour) | 0x80; //set 24h.
	buffer[REG1208_DAY]    = (UCHAR)SETBCDDATA(ptime->wDay);
	buffer[REG1208_MONTH]  = (UCHAR)SETBCDDATA(ptime->wMonth);
	buffer[REG1208_YEAR]   = (UCHAR)SETBCDDATA(ptime->wYear - 2000);
	buffer[REG1208_WEEK]   = (UCHAR)SETBCDDATA(ptime->wDayOfWeek);

	// write to ISL1208
	bRet = write_ISL1208(REG1208_SECOND, buffer, 7);

	if(bRet)
	{
		// write ISL1208 successfully
		OALMSG(TRUE, (L"ISL1208SetRealTime: OK %d-%d-%d\r\n", ptime->wYear, ptime->wMonth, ptime->wDay));
		//OALMSG(TRUE, (L"ISL1208SetRealTime: OK %x-%x-%x %x:%x:%x %x\r\n", 
		//	buffer[REG1208_YEAR],
		//	buffer[REG1208_MONTH],
		//	buffer[REG1208_DAY],
		//	buffer[REG1208_HOUR],
		//	buffer[REG1208_MIN],
		//	buffer[REG1208_SECOND],
		//	buffer[REG1208_WEEK]
		//	));
	}
	else
	{
		// write ISL1208 failed
		OALMSG(TIME_ERROR, (L"ISL1208SetRealTime: failed\r\n"));
	}

	return bRet;
}


BOOL ISL1208GetRealTime( LPSYSTEMTIME ptime )
{
	BOOL		bRet = FALSE;
	UCHAR		buffer[8];
	SYSTEMTIME  time;

	// read from ISL1208
	bRet = read_ISL1208(REG1208_SECOND, buffer, 7);
	//{
	//	DWORD i;

	//	for(i = 0; i < 7; i++)
	//	{
	//		bRet = read_ISL1208(i, &buffer[i], 1);
	//	}
	//}

	if(bRet)		// read ISL1208 sucessfully!
	{
		// convert to SYSTEMTIME format
		time.wSecond    = GETBCDDATA(buffer[REG1208_SECOND]);
		time.wMinute    = GETBCDDATA(buffer[REG1208_MIN]);
		time.wHour      = GETBCDDATA(buffer[REG1208_HOUR] & 0x3F);
		time.wDay       = GETBCDDATA(buffer[REG1208_DAY]);
		time.wMonth     = GETBCDDATA(buffer[REG1208_MONTH]);
		time.wYear      = GETBCDDATA(buffer[REG1208_YEAR]) + 2000;
		time.wDayOfWeek = GETBCDDATA(buffer[REG1208_WEEK]);
	
		// CS&ZHL JUN-7-2012: it's necessary to check the time read from ISL1208
		if(CheckRealTime(&time))
		{
			OALMSG(TIME_TEST, (L"ISL1208GetRealTime: OK %d-%d-%d %d:%d:%d %d\r\n", 
				ptime->wYear,
				ptime->wMonth,
				ptime->wDay,
				ptime->wHour,
				ptime->wMinute,
				ptime->wSecond,
				ptime->wDayOfWeek
				));

			memcpy(ptime, &time, sizeof(SYSTEMTIME));
		}
		else
		{
			OALMSG(TIME_ERROR, (L"ISL1208GetRealTime: invalid %x-%x-%x %x:%x:%x\r\n",
				(unsigned int)buffer[REG1208_YEAR],
				(unsigned int)buffer[REG1208_MONTH],
				(unsigned int)buffer[REG1208_DAY],
				(unsigned int)buffer[REG1208_HOUR],
				(unsigned int)buffer[REG1208_MIN],
				(unsigned int)buffer[REG1208_SECOND]
				));
		}
	}
	else
	{
		// read ISL1208 failed!
		OALMSG(TIME_ERROR, (L"ISL1208GetRealTime: failed %x-%x-%x %x:%x:%x\r\n",
			(unsigned int)buffer[REG1208_YEAR],
			(unsigned int)buffer[REG1208_MONTH],
			(unsigned int)buffer[REG1208_DAY],
			(unsigned int)buffer[REG1208_HOUR],
			(unsigned int)buffer[REG1208_MIN],
			(unsigned int)buffer[REG1208_SECOND]
			));
	}

	return bRet;
}

BOOL InitializeISL1208(LPSYSTEMTIME ptime)
{
	BOOL bRet = FALSE;
	int  islStatus;

	OALMSG(TIME_TEST, (L"->InitializeISL1208\r\n"));
	//-------------------------------------------------
	// return = 1: RTC is valid, and setup already
	//        = 0: RTC is valid, but not setup yet
	//        < 0: RTC is invalid
	//-------------------------------------------------
	islStatus = init_ISL1208( );
	if( islStatus < 0 )
	{
		OALMSG(TIME_ERROR, (L"InitializeISL1208: init ISL1208 failed!\r\n"));
		return bRet;
	}

	if( islStatus == 1 )
	{
		OALMSG(TIME_TEST, (L"InitializeISL1208: RTC data ready -> ISL1208GetRealTime\r\n"));
		bRet = ISL1208GetRealTime( ptime );
	}
	else
	{
		OALMSG(TIME_TEST, (L"InitializeISL1208: RTC data unavailable -> ISL1208SetRealTime\r\n"));
		bRet = ISL1208SetRealTime( ptime );
	}

	OALMSG(TIME_TEST, (L"<-InitializeISL1208 return=%d\r\n", (DWORD)bRet));
	return bRet;
}



