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

#ifndef __TCA6424A_I2C_H
#define __TCA6424A_I2C_H

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define	TCA6424A_ADDR_WRITE			0x44
#define	TCA6424A_ADDR_READ			0x45

#define TCA6424A_CMD_ADDR_PORT0		(0x00 << 0)
#define TCA6424A_CMD_ADDR_PORT1		(0x01 << 0)
#define TCA6424A_CMD_ADDR_PORT2		(0x02 << 0)

#define TCA6424A_CMD_INPUT			(0x00 << 2)
#define TCA6424A_CMD_OUTPUT			(0x01 << 2)			// default = 1
#define TCA6424A_CMD_POL_INV		(0x02 << 2)			// = 0: not inv, = 1: inv; default = 0
#define TCA6424A_CMD_DIR			(0x03 << 2)			// = 1: input, = 0: output, default = 1

#define TCA6424A_CMD_AUTO_INC		(0x01 << 7)

#endif   // __TCA6424A_I2C_H

