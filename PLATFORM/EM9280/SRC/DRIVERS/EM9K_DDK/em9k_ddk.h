//------------------------------------------------------------------------------
//
//  Copyright (C) 2012,Emtronix, Inc. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  tca6424a_i2c.h
//
//  Header file for chip TCA6424A, a chip featured with i2c to 24-bit GPIO.
//
//------------------------------------------------------------------------------

#ifndef __EM9K_DDK_H
#define __EM9K_DDK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _DDK_GPIOX_PIN
{
	EM9280_PIN_GPIO2   = (0),
	EM9280_PIN_GPIO3   = (1),
	EM9280_PIN_GPIO4   = (2),
	EM9280_PIN_GPIO5   = (3),
	EM9280_PIN_GPIO8   = (4),
	EM9280_PIN_GPIO9   = (5),

	EM9280_PIN_GPIO12  = (8),
	EM9280_PIN_GPIO13  = (9),
	EM9280_PIN_GPIO14  = (10),
	EM9280_PIN_GPIO15  = (11),
	EM9280_PIN_GPIO16  = (12),
	EM9280_PIN_GPIO17  = (13),
	EM9280_PIN_GPIO18  = (14),
	EM9280_PIN_GPIO19  = (15),

	EM9280_PIN_COM7MUX = (16),
	EM9280_PIN_COM8MUX = (17),
	EM9280_PIN_COM9MUX = (18),
	EM9280_PIN_PHY_RST = (22),
	EM9280_PIN_RSTO    = (23),
	EM9280_PIN_SIZE
}DDK_GPIOX_PIN;

BOOL DDKGpioSpiRead(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);
BOOL DDKGpioSpiWrite(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);

BOOL DDKGpioI2cWrite(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);
BOOL DDKGpioI2cRead(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);

BOOL DDKGpioxPinOutEn(DDK_GPIOX_PIN gpiox_pin);
BOOL DDKGpioxPinOutDis(DDK_GPIOX_PIN gpiox_pin);
BOOL DDKGpioxPinSet(DDK_GPIOX_PIN gpiox_pin);
BOOL DDKGpioxPinClear(DDK_GPIOX_PIN gpiox_pin);
BOOL DDKGpioxPinState(DDK_GPIOX_PIN gpiox_pin, PDWORD pRetState);

#ifdef __cplusplus
}
#endif

#endif   // __EM9K_DDK_H

