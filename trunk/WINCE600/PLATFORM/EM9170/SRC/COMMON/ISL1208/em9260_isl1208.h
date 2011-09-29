#ifndef _EM9260_ISL1208_H_
#define _EM9260_ISL1208_H_

#define   ISL1208_ADDRESS			0x6F
#define   WRITE_ADDRESS				0xDE
#define   READ_ADDRESS				0xDF

//Register defines for ISL1208

#define REG1208_SECOND	0x0
#define REG1208_MIN		0x1
#define REG1208_HOUR	0x2
#define REG1208_DAY		0x3
#define REG1208_MONTH	0x4
#define REG1208_YEAR	0x5
#define REG1208_WEEK	0x6

#define REG1208_STATUS	0x7
#define REG1208_INT		0x8

#define REG1208_ATR		0xA
#define REG1208_DTR		0xB

#define REG1208_USER1	0x12
#define REG1208_USER2	0x13

#define STATUS_WRTC		0x10
#define STATUS_XTOSCB	0x40
#define STATUS_ARST		0x80

//===================================
// init TWI controller
//===================================
int AT91_TWI_Init( );

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
int AT91_TWI_WriteByte( int int_address, char data2send );

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
int AT91_TWI_ReadByte(int int_address, char *pdata );

//=========================================================================================
// Initialize ISL1208
// return = 0: the first time of battery backup
//        = 1: ok!
//        < 0: fail
//=========================================================================================
int ISL1208_Init( );

BOOL ISL1208SetRealTime(LPSYSTEMTIME ptime);

BOOL ISL1208GetRealTime(LPSYSTEMTIME ptime);

BOOL InitializeISL1208(LPSYSTEMTIME ptime);

#endif // _EM9260_ISL1208_H_