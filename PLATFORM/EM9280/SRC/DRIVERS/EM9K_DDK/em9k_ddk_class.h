//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2011,Emtronix, Inc. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  isaclass.h
//
//  Header file, for isa_dio driver.
//
//------------------------------------------------------------------------------

#ifndef __EM9K_DDK_CLASS_H
#define __EM9K_DDK_CLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_INIT         0
#define ZONEID_DEINIT       1
#define ZONEID_OPEN         2
#define ZONEID_CLOSE        3
#define ZONEID_IOCTL        4
#define ZONEID_THREAD       5
#define ZONEID_FUNCTION     13
#define ZONEID_WARN         14
#define ZONEID_ERROR        15

// Debug zone masks
#define ZONEMASK_INIT       (1 << ZONEID_INIT)
#define ZONEMASK_DEINIT     (1 << ZONEID_DEINIT)
#define ZONEMASK_OPEN       (1 << ZONEID_OPEN)
#define ZONEMASK_CLOSE      (1 << ZONEID_CLOSE)
#define ZONEMASK_IOCTL      (1 << ZONEID_IOCTL)
#define ZONEMASK_THREAD     (1 << ZONEID_THREAD)
#define ZONEMASK_FUNCTION   (1 << ZONEID_FUNCTION)
#define ZONEMASK_WARN       (1 << ZONEID_WARN)
#define ZONEMASK_ERROR      (1 << ZONEID_ERROR)

#define ZONE_INIT       DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT     DEBUGZONE(ZONEID_DEINIT)
#define ZONE_OPEN       DEBUGZONE(ZONEID_OPEN)
#define ZONE_CLOSE      DEBUGZONE(ZONEID_CLOSE)
#define ZONE_IOCTL      DEBUGZONE(ZONEID_IOCTL)
#define ZONE_THREAD     DEBUGZONE(ZONEID_THREAD)
#define ZONE_FUNCTION   DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN       DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR      DEBUGZONE(ZONEID_ERROR)
#endif // DEBUG

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
class Em9kDDKClass
{
private:

    //-------------------------------------
    // critical section for I2C interface
	CRITICAL_SECTION	csGpioI2cMutex;

	DDK_IOMUX_PIN	I2C_SCL_PIN;
	DDK_IOMUX_PIN	I2C_SDA_PIN;
	DDK_IOMUX_PIN	TCA6424A_INT_PIN;
	DDK_IOMUX_PIN	TCA6424A_RST_PIN;

	BYTE			uTCA6424A_Addr;
	BYTE			uGPIOX_DOUT[3];			// P0 - P2
	BYTE			uGPIOX_INV[3];			// P0 - P2
	BYTE			uGPIOX_DIR[3];			// P0 - P2
	BYTE			uGPIOX_PORT[3];			// port index = 0, 1, 2

	//primitive control line operation functions for I2C interface
	__inline void	SCL_SET(void);
	__inline void	SCL_CLR(void);
	__inline void	SDA_SET(void);
	__inline void	SDA_CLR(void);
	__inline void	SDA_OUTEN(void);
	__inline void	SDA_OUTDIS(void);
	__inline UINT32	SDA_STATE(void);

	BOOL GpioI2CInit(void);

    //-------------------------------------
    // critical section for SPI interface
	CRITICAL_SECTION	csGpioSpiMutex;

	DDK_IOMUX_PIN	SPI_MISO_PIN;			// data input pin for SPI interface
	DDK_IOMUX_PIN	SPI_MOSI_PIN;			// data output pin for SPI interface
	DDK_IOMUX_PIN	SPI_SCLK_PIN;			// clock output pin for SPI interface
	DDK_IOMUX_PIN	SPI_CS0N_PIN;			// chip select 0 pin for SPI interface 
	DDK_IOMUX_PIN	SPI_CS1N_PIN;			// chip select 0 pin for SPI interface 
	DDK_IOMUX_PIN	SPI_CS2N_PIN;			// chip select 0 pin for SPI interface 

	//primitive control line operation functions for SPI interface
	__inline void	SPI_ENABLE(DWORD dwCSNum);
	__inline void	SPI_DISABLE(DWORD dwCSNum);
	__inline void	SPI_DOUT_SET(void);
	__inline void	SPI_DOUT_CLR(void);
	__inline void	SPI_SCLK_SET(void);
	__inline void	SPI_SCLK_CLR(void);
	__inline UINT32	SPI_DIN_STATE(void);

	BOOL GpioSPIInit(void);

public:
    //-------------------------------------
    // ISA CONSTRUCTOR/DESTRUCTOR METHODS
    //
    Em9kDDKClass(void);
    ~Em9kDDKClass(void);
 
	BOOL DDKGpioSpiRead(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);
	BOOL DDKGpioSpiWrite(DWORD dwCS, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);

	BOOL DDKGpioI2cWrite(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);
	BOOL DDKGpioI2cRead(DWORD dwDeviceID, DWORD dwCmdAddr, PBYTE pBuf, DWORD dwLength);

	BOOL DDKGpioxPinOutEn(DDK_GPIOX_PIN gpiox_pin);
	BOOL DDKGpioxPinOutDis(DDK_GPIOX_PIN gpiox_pin);
	BOOL DDKGpioxPinSet(DDK_GPIOX_PIN gpiox_pin);
	BOOL DDKGpioxPinClear(DDK_GPIOX_PIN gpiox_pin);
	BOOL DDKGpioxPinState(DDK_GPIOX_PIN gpiox_pin, PDWORD pRetState);

	// other public functions
	void udelay(DWORD dwMicroSecond);
};


#ifdef __cplusplus
}
#endif

#endif   // __EM9K_DDK_CLASS_H

