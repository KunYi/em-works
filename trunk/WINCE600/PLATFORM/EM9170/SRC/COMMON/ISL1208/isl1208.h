//************************************************************************
//                                                                      
// Filename: ds1337.h
//                                                                      
// Copyright(c) Cirrus Logic Corporation 2004, All Rights Reserved                       
//
//************************************************************************

#ifndef      _HW_DEFS_
#define     _HW_DEFS_

//#undef ULONG
//typedef unsigned long ULONG;

#define   WRITE_ADDRESS			0xDE
#define   READ_ADDRESS				0xDF

//Register defines for ISL1208

#define REG1208_SECOND	0x0
#define REG1208_MIN			0x1
#define REG1208_HOUR		0x2
#define REG1208_DAY			0x3
#define REG1208_MONTH		0x4
#define REG1208_YEAR		0x5
#define REG1208_WEEK		0x6

#define REG1208_STATUS	0x7
#define REG1208_INT			0x8

#define REG1208_ATR			0xA
#define REG1208_DTR			0xB

#define REG1208_USER1		0x12
#define REG1208_USER2		0x13		// = 0x55: default setting; = 0xaa: user setting; otherwise = not setting

#define STATUS_WRTC			0x10
#define STATUS_XTOSCB		0x40
#define STATUS_ARST			0x80

/*
typedef struct _SYSTEMTIME { 
    unsigned long wYear; 
    unsigned long wMonth; 
    unsigned long wDayOfWeek; 
    unsigned long wDay; 
    unsigned long wHour; 
    unsigned long wMinute; 
    unsigned long wSecond; 
    unsigned long wMilliseconds; 
} SYSTEMTIME, *PSYSTEMTIME; 
*/

#endif

