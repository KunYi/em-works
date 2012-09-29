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

#define TIME_TEST					0
#define TIME_ERROR					1

extern PCSP_GPIO_REGS			g_pGPIO_SCL;				// -> GPIO1.18
extern PCSP_GPIO_REGS			g_pGPIO_SDA;				// -> GPIO4.25

//
// CS&ZHL JLY-22-2011: copy VOID OALStall(UINT32 uSecs); from bspfec.c
//
void DelayInuSec(UINT32 uSecs)
{
    LARGE_INTEGER liStart, liEnd, liStallCount;
    static LARGE_INTEGER liFreq = {0, 0};
    static PCSP_EPIT_REG pEPIT = NULL;
    BSP_ARGS *pBspArgs;

    if (pEPIT == NULL)
    {
        // Map EPIT registers
        pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
        if (pEPIT == NULL)
        {
            //EdbgOutputDebugString("OALStall: EPIT mapping failed!\r\n");
            return;
        }
    }

    if (liFreq.QuadPart == 0)
    {
        pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
        
        switch(EXTREG32BF(&pEPIT->CR, EPIT_CR_CLKSRC))
        {
        case EPIT_CR_CLKSRC_CKIL:
            liFreq.QuadPart = BSP_CLK_CKIL_FREQ;
            break;

        case EPIT_CR_CLKSRC_IPGCLK:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_EPIT];
            break;
        case EPIT_CR_CLKSRC_HIGHFREQ:
            liFreq.QuadPart = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB];
            break;
        }

        liFreq.QuadPart = liFreq.QuadPart / (EXTREG32BF(&pEPIT->CR, EPIT_CR_PRESCALAR) + 1);
    }

    liStallCount.QuadPart = ((liFreq.QuadPart * uSecs - 1) / 1000000) + 1;   // always round up
    
    liStart.QuadPart = INREG32(&pEPIT->CNT);
    do {
        liEnd.QuadPart = INREG32(&pEPIT->CNT);
    } while ( (liStart.QuadPart - liEnd.QuadPart) <= liStallCount.QuadPart);

}

////////////////////////////////////////////////////////////////////////////
//
// CS&ZHL NOV-24-2008: The following code are from ETR232H for ISL1208
//
////////////////////////////////////////////////////////////////////////////

// Dir = 0: as input
// Dir = 1: as output
void SetSDADir( int Dir )
{
	if( Dir )		// set as output
	{
		// set SDA as ouput
		OUTREG32 (&g_pGPIO_SDA->GDIR, (INREG32(&g_pGPIO_SDA->GDIR) | (1 << 25)));
	}
	else			// set as input
	{
		// set SDA as input
		OUTREG32 (&g_pGPIO_SDA->GDIR, (INREG32(&g_pGPIO_SDA->GDIR) & ~(1 << 25)));
	}
};

// Level = 1: output 1 on SCL
// Level = 0: output 0 on SCL
void SetSCL( int Level )
{
	if( Level )
	{
		OUTREG32 (&g_pGPIO_SCL->DR, (INREG32(&g_pGPIO_SCL->DR) | (1 << 18)));
	}
	else
	{
		OUTREG32 (&g_pGPIO_SCL->DR, (INREG32(&g_pGPIO_SCL->DR) & ~(1 << 18)));
	}
};

// Level = 1: output 1 on SDA
// Level = 0: output 0 on SDA
void SetSDA( int Level )
{
	if( Level ) 
	{
		OUTREG32 (&g_pGPIO_SDA->DR, (INREG32(&g_pGPIO_SDA->DR) | (1 << 25)));
	}
	else
	{
		OUTREG32 (&g_pGPIO_SDA->DR, (INREG32(&g_pGPIO_SDA->DR) & ~(1 << 25)));
	}
};

UCHAR GetSDA( )
{
	if(INREG32(&g_pGPIO_SDA->PSR) & (1 << 25))
	{
		return 1;
	}

	return 0;
};

// return =  0: ok
//        = -1: fail! no acknowledgement
int I2C_ByteWrite( int Idx, unsigned char abyte )
{
	int				i1 = 0;
	int				i2;
	UCHAR		ub1, ub2;

	ub1 = WRITE_ADDRESS;				// 1st byte = ID + Write-Bit(=0)
	ub2 = (UCHAR)(Idx & 0xff);			// 2nd byte = register pointer

	// START condition
	//outp( SCL_Port, 1 );	// set SCL HIGH
	SetSCL( 1 );
	//outp( SDA_Port, 1 );	// set SDA HIGH
	SetSDA( 1 );
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	//outp( SDA_Port, 0 );	// set SDA LOW
	SetSDA( 0 );
	DelayInuSec(4);		// delay 4us
	//outp( SCL_Port, 0 );	// set SCL LOW
	SetSCL( 0 );
	DelayInuSec(4);		// delay 4us

	// transfer the 1st byte
	for( i2=0; i2<8; i2++ )
	{
		if( ub1&0x80 )
		{
			//outp( SDA_Port, 1 );
			SetSDA( 1 );
		}
		else
		{
			//outp( SDA_Port, 0 );
			SetSDA( 0 );
		}

		//outp( SCL_Port, 1 );
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//outp( SCL_Port, 0 );
		SetSCL( 0 );
		ub1 = ub1 << 1;
	}

	// check ACK_Bit from ISL1208 for the 1st byte writing
	//outp( DOE_Port, 0 );
	SetSDADir( 0 );			// set SDA as input
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	//outp( SCL_Port, 1 );	// issue a clock
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//ub1 = inp( SDA_Port );
	ub1 = GetSDA( );
	//outp( SCL_Port, 0 );
	SetSCL( 0 );
	if( ub1&0x01 )
	{
		//outp( SCL_Port, 1 );
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//outp( SDA_Port, 1 );
		SetSDA( 1 );
		return -1;		// no acknowledgement
	}
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	DelayInuSec(4);		// delay 4us

	// transfer the 2nd byte
	for( i2=0; i2<8; i2++ )
	{
		if( ub2&0x80 )
		{
			//outp( SDA_Port, 1 );
			SetSDA( 1 );
		}
		else
		{
			//outp( SDA_Port, 0 );
			SetSDA( 0 );
		}

		//outp( SCL_Port, 1 );
		SetSCL( 1 );				// issue a clock
		DelayInuSec(4);			// delay 4us
		//outp( SCL_Port, 0 );
		SetSCL( 0 );
		ub2 = ub2 << 1;
	}

	// check ACK_Bit from ISL1208 for the 2nd byte writing
	//outp( DOE_Port, 0 );
	SetSDADir( 0 );			// set SDA as input
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	//outp( SCL_Port, 1 );	// issue a clock
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//ub1 = inp( SDA_Port );
	ub1 = GetSDA( );
	//outp( SCL_Port, 0 );
	SetSCL( 0 );
	if( ub1&0x01 )
	{
		//outp( SCL_Port, 1 );	// STOP condition
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//outp( SDA_Port, 1 );
		SetSDA( 1 );
		return -1;		// no acknowledgement
	}
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	DelayInuSec(4);		// delay 4us

	// transfer the 3rd byte => last byte
	for( i2=0; i2<8; i2++ )
	{
		if( abyte&0x80 )
		{
			//outp( SDA_Port, 1 );
			SetSDA( 1 );
		}
		else
		{
			//outp( SDA_Port, 0 );
			SetSDA( 0 );
		}

		//outp( SCL_Port, 1 );
		SetSCL( 1 );				//issue a clock
		DelayInuSec(4);		// delay 4us
		//outp( SCL_Port, 0 );
		SetSCL( 0 );
		abyte = abyte << 1;
	}

	// check ACK_Bit from ISL1208 for the last byte writing
	//outp( DOE_Port, 0 );
	SetSDADir( 0 );			// set SDA as input
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	//outp( SCL_Port, 1 );	// issue a clock
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//ub1 = inp( SDA_Port );
	ub1 = GetSDA( );
	//outp( SCL_Port, 0 );
	SetSCL( 0 );
	if( ub1&0x01 )
	{
		//outp( SCL_Port, 1 );	// STOP condition
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//outp( SDA_Port, 1 );
		SetSDA( 1 );
		return -1;		// no acknowledgement
	}
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	DelayInuSec(4);		// delay 4us

	// Here we issue a STOP, no acknowledgement
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	DelayInuSec(4);		// delay 4us
	//outp( SCL_Port, 1 );	// STOP condition
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//outp( SDA_Port, 1 );
	SetSDA( 1 );
	//outp( DOE_Port, 0 );	// disable SDA
	SetSDADir( 0 );			// as SDA as input

	return i1;
}

// return = -1: fail!
//        = other: lower byte is read result!
int I2C_ByteRead( int Idx )
{
	int           i1, i2;
	unsigned char ub1, ub2;

	// Session 1: setup register pointer
	ub1 = WRITE_ADDRESS;					// 1st byte = ID + Write-Bit(=0)
	ub2 = (UCHAR)(Idx & 0xff);				// 2nd byte = register pointer

	// START condition
	//outp( SCL_Port, 1 );	// set SCL HIGH
	SetSCL( 1 );
	//outp( SDA_Port, 1 );	// set SDA HIGH
	SetSDA( 1 );
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	//outp( SDA_Port, 0 );	// set SDA LOW
	SetSDA( 0 );
	DelayInuSec(4);		// delay 4us
	//outp( SCL_Port, 0 );	// set SCL LOW
	SetSCL( 0 );
	DelayInuSec(4);		// delay 4us

	// transfer the 1st byte
	for( i2=0; i2<8; i2++ )
	{
		if( ub1&0x80 )
		{
			//outp( SDA_Port, 1 );
			SetSDA( 1 );
		}
		else
		{
			//outp( SDA_Port, 0 );
			SetSDA( 0 );
		}
		//outp( SCL_Port, 1 );
		SetSCL( 1 );			//issue a clock
		DelayInuSec(4);		// delay 4us
		//outp( SCL_Port, 0 );
		SetSCL( 0 );
		ub1 = ub1 << 1;
	}

	// check ACK_Bit from ISL1208 for the 1st byte writing
	//outp( DOE_Port, 0 );
	SetSDADir( 0 );			// set SDA as input
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	//outp( SCL_Port, 1 );	// issue a clock
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//ub1 = inp( SDA_Port );
	ub1 = GetSDA( );
	//outp( SCL_Port, 0 );
	SetSCL( 0 );
	if( ub1&0x01 )
	{
		//outp( SCL_Port, 1 );
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//outp( SDA_Port, 1 );
		SetSDA( 1 );
		return -1;		// no acknowledgement
	}
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	DelayInuSec(4);		// delay 4us

	// transfer the 2nd byte => last byte
	ub1 = ub2;
	for( i2=0; i2<8; i2++ )
	{
		if( ub1&0x80 )
		{
			//outp( SDA_Port, 1 );
			SetSDA( 1 );
		}
		else
		{
			//outp( SDA_Port, 0 );
			SetSDA( 0 );
		}
		//outp( SCL_Port, 1 );
		SetSCL( 1 );			//issue a clock
		DelayInuSec(4);		// delay 4us
		//outp( SCL_Port, 0 );
		SetSCL( 0 );
		ub1 = ub1 << 1;
	}
	// check ACK_Bit from ISL1208 for the last byte writing
	//outp( DOE_Port, 0 );
	SetSDADir( 0 );			// set SDA as input
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	//outp( SCL_Port, 1 );	// issue a clock
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//ub1 = inp( SDA_Port );
	ub1 = GetSDA( );
	//outp( SCL_Port, 0 );
	SetSCL( 0 );
	if( ub1&0x01 )
	{
		//outp( SCL_Port, 1 );
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//outp( SDA_Port, 1 );
		SetSDA( 1 );
		return -1;		// no acknowledgement
	}
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	DelayInuSec(4);		// delay 4us

	// Here we issue a STOP for the Session 1
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	DelayInuSec(4);		// delay 4us
	//outp( SCL_Port, 1 );	// STOP condition
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//outp( SDA_Port, 1 );
	SetSDA( 1 );
	//outp( DOE_Port, 0 );	// disable SDA
	SetSDADir( 0 );			// set SDA as input
	DelayInuSec(10);		// delay 10us

	// Session 2: read a byte from ISL1208
	ub1 = READ_ADDRESS;		// 1st byte = ID + Read-Bit(=1)

	// START condition
	//outp( SCL_Port, 1 );	// set SCL HIGH
	SetSCL( 1 );
	//outp( SDA_Port, 1 );	// set SDA HIGH
	SetSDA( 1 );
	//outp( DOE_Port, 1 );	// enable SDA
	SetSDADir( 1 );			// set SDA as output
	//outp( SDA_Port, 0 );	// set SDA LOW
	SetSDA( 0 );
	DelayInuSec(4);		// delay 4us
	//outp( SCL_Port, 0 );	// set SCL LOW
	SetSCL( 0 );
	DelayInuSec(4);		// delay 4us

	// transfer the 1st byte
	for( i2=0; i2<8; i2++ )
	{
		if( ub1&0x80 )
		{
			//outp( SDA_Port, 1 );
			SetSDA( 1 );
		}
		else
		{
			//outp( SDA_Port, 0 );
			SetSDA( 0 );
		}
		//outp( SCL_Port, 1 );
		SetSCL( 1 );			//issue a clock
		DelayInuSec(4);		// delay 4us
		//outp( SCL_Port, 0 );
		SetSCL( 0 );
		ub1 = ub1 << 1;
	}

	// check ACK_Bit from ISL1208 for the 1st byte writing
	//outp( DOE_Port, 0 );
	SetSDADir( 0 );			// set SDA as input
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	//outp( SCL_Port, 1 );	// issue a clock
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//ub1 = inp( SDA_Port );
	ub1 = GetSDA( );
	//outp( SCL_Port, 0 );
	SetSCL( 0 );
	if( ub1&0x01 )
	{
		//outp( SCL_Port, 1 );
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//outp( SDA_Port, 1 );
		SetSDA( 1 );
		return -1;		// no acknowledgement
	}
	DelayInuSec(4);		// delay 4us

	// then read a abyte
	for( ub2=0, i2=0; i2<8; i2++ )
	{
		ub2 = ub2 << 1;
		//outp( SCL_Port, 1 );
		SetSCL( 1 );
		DelayInuSec(4);		// delay 4us
		//ub1 = inp( SDA_Port );
		ub1 = GetSDA( );
		//outp( SCL_Port, 0 );
		SetSCL( 0 );
		DelayInuSec(4);		// delay 4us
		ub2 = ub2|(ub1&0x01);
	}

	// issue a NOT-ACK condition to end the read session
	//outp( DOE_Port, 1 );
	SetSDADir( 1 );			// set SDA as output
	//outp( SDA_Port, 1 );
	SetSDA( 1 );
	DelayInuSec(4);		// delay 4us
	//outp( SCL_Port, 1 );	// issue a clock
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//outp( SCL_Port, 0 );	// issue a clock
	SetSCL( 0 );
	DelayInuSec(4);		// delay 4us
	//outp( SDA_Port, 0 );
	SetSDA( 0 );
	DelayInuSec(4);		// delay 4us

	// Here we issue a STOP condition for Session 2
	//outp( SCL_Port, 1 );	// STOP condition
	SetSCL( 1 );
	DelayInuSec(4);		// delay 4us
	//outp( SDA_Port, 1 );
	SetSDA( 1 );
	//outp( DOE_Port, 0 );	// disable SDA
	SetSDADir( 0 );			// set SDA as input

	i1 = (unsigned int)ub2;
	return i1;
}
////////////////////////////////////////////////////////////////////////
// End of ETR232H code
////////////////////////////////////////////////////////////////////////

//
// CS&ZHL NOV-24-2008: Write the registers of ISL1208 based on ETR232H's code 
//
int write_ISL1208(int address, UCHAR* pdata, int nbytes)
{
	int		i1 = 0;
	int    i2 = 0;

	for(i1 = 0; i1 < nbytes; i1++)
	{
		// return =  0: ok
		//        = -1: fail! no acknowledgement
		i2 = I2C_ByteWrite( address, (UCHAR)pdata[i1] );
		if( i2 == -1 )
		{
			return i2;
		}

		// points to next register
		address++;
	}

	return i2;
}

//
// CS&ZHL NOV-24-2008: Read the registers of ISL1208 based on ETR232H's code
//
int read_ISL1208(int address, UCHAR* pdata, int nbytes)
{
	int		i1, i2;

	for(i1 = 0; i1 < nbytes; i1++)
	{
		// return = -1: fail!
		//        = other: lower byte is read result!
		i2 = I2C_ByteRead( address );
		if( i2 == -1 )
		{
			return i2;
		}

		// save the data to buffer
		pdata[i1] = (UCHAR)i2;

		// points to next register
		address++;
	}

	return 0;
}

int init_ISL1208(void )
{
	UCHAR	 buffer[8]={5};
	UCHAR	 status = 0;
	int  nRTCValid = -1;
	int	 i1;	

    //OALMSG(TRUE, (L"+init_ISL1208\r\n"));
	//
	// CS&ZHL OCT-06-2008: check more than once
	//
	for(i1 = 0; i1 < 3; i1++ )
	{
		if( read_ISL1208( REG1208_STATUS ,&status, 1 ) >= 0 )
		{
			break;
		}
	}

	if(i1 >= 3)
	{
		RETAILMSG(1, (TEXT("Test ISL1208, failed to read\r\n")));
		return -1;
	}
	//RETAILMSG(1,(TEXT("RTC status %x\r\n"), (unsigned int)(status & 0xff)));

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
			nRTCValid=0;
		}
	}
	else
	{
		nRTCValid=1;
	}

	if( !(status & STATUS_WRTC) || (status & 0x01) )
	{
		//OALMSG(TRUE, (L"init_ISL1208: chip startup\r\n"));
		status = STATUS_WRTC | STATUS_ARST;
		if( write_ISL1208(REG1208_STATUS,&status, 1) < 0 )
		{
			RETAILMSG(TIME_ERROR,(TEXT("Init RTC failed\r\n")));
			nRTCValid = -1;
			return nRTCValid;
		}
		//OALMSG(TRUE, (L"init_ISL1208: force Fout = 32768Hz\r\n"));
		// Fout = 32768Hz for manufacture testing
		read_ISL1208( REG1208_INT, &status, 1 );		//read INT control register
        status &= 0xf0;
		status |= 0x01;			
		if( write_ISL1208( REG1208_INT, &status, 1 ) < 0 )
		{
			RETAILMSG(TIME_ERROR,(TEXT("Init RTC REG1208_INT failed\r\n")));
			nRTCValid = -1;
		}

		//--------------------------------------------------------------------
		// CS&ZHL SEP-18-2007: CL = 8.5pF; ATR = 0x30
		// CS&ZHL AUG-20-2008: apply default setting CL = 14.5pF, ATR = 0x08
		//--------------------------------------------------------------------
		status = 0x08;
		if( write_ISL1208( REG1208_ATR, &status, 1 ) < 0 )
		{
			RETAILMSG(TIME_ERROR,(TEXT("Write RTC REG1208_ATR failed\r\n")));
			nRTCValid = -1;
		}

		// mark default setting flag = 0x55
		status = 0x55;
		if( write_ISL1208( REG1208_USER2, &status, 1 ) < 0 )
		{
			RETAILMSG(TIME_ERROR,(TEXT("Write RTC REG1208_USER2 failed\r\n")));
			nRTCValid = -1;
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

//
// CS&ZHL OCT-3-2008: get realtime with verification
//
BOOL GetISL1208DataWithVerify( char* datbuf, int buflen )
{
	int			i1;
	int			i2;
	UCHAR	buf[3][8];
	BOOL	bRet = TRUE;

	// clear buf
	memset(&buf[0][0], 0, 24);

	// read data
	for(i1 = 0, i2 = 0; i2 < 10; i2++)
	{
		if(read_ISL1208(0, buf[i1], buflen) < 0)
		{
			continue;
		}
		i1++;
		if( i1 >= 3 )
		{
			break;
		}
	}

	//
	// get data, anyway
	//
	memcpy(datbuf, buf[0], buflen);

	//
	// CS&ZHL OCT-09-2008: error processing
	//
	if( !i1 )		
	{
		bRet = FALSE;		// read ISL1208 fail!
	}

	return bRet;
}


BOOL ISL1208SetRealTime(LPSYSTEMTIME ptime)
{
	BOOL bRet = FALSE;

	RETAILMSG(TIME_ERROR,(TEXT("SetRealTime()::%d-%d-%d\r\n")
		,ptime->wYear
		,ptime->wMonth
		,ptime->wDay));

	if( ptime->wYear >=2000 )
	{
		UCHAR buffer[7];

		buffer[REG1208_SECOND]=(char)SETBCDDATA(ptime->wSecond);
		buffer[REG1208_MIN]=(char) SETBCDDATA(ptime->wMinute);
		buffer[REG1208_HOUR]=(char)SETBCDDATA(ptime->wHour) | 0x80; //set 24h.
		buffer[REG1208_DAY]=(char) SETBCDDATA(ptime->wDay);
		buffer[REG1208_MONTH]=(char)SETBCDDATA(ptime->wMonth);
		buffer[REG1208_YEAR]=(char)SETBCDDATA(ptime->wYear-2000);

		buffer[REG1208_WEEK]=(char)SETBCDDATA(ptime->wDayOfWeek);
		//if(write_ISL1208(0, buffer, 6) >= 0)			//DO NOT set week-day!!
		if(write_ISL1208(0, buffer, 7) >= 0)
		{
			//RETAILMSG(TIME_ERROR,(TEXT("SetRealTime OK\r\n")));
			bRet=TRUE;
			/*
			char rdbuf[7];
			int  i;

			// read back for verification
			if(GetISL1208DataWithVerify(rdbuf, 7))		
			{	
				// read ISL1208 successfully, and verify...
				for(i = 0; i < 7; i++)
				{
					if(rdbuf[i] != buffer[i])
					{	//verify failed
						break;
					}
				}

				if(i >= 7)
				{
					RETAILMSG(TIME_ERROR,(TEXT("SetRealTime OK\r\n")));
					bRet=TRUE;
				}
			}
			*/
		}
		if(!bRet)
		{
			RETAILMSG(1,(TEXT("SetRealTime failed!\r\n")));
		}
	}
	else
	{
		RETAILMSG(TIME_ERROR,(TEXT("SetRealTime Year invalid\r\n")));
	}

	//ISL1208GetRealTime(  ptime );
	return bRet;
}


BOOL ISL1208GetRealTime( LPSYSTEMTIME  ptime )
{
	//
	// CS&ZHL OCT-3-2008: read data from ISL1208 with verification
	//
	char	buffer[7] = {0};
	BOOL	bRet = TRUE;

	if(!GetISL1208DataWithVerify(buffer, 7))		
	{	// read ISL1208 fail!
		RETAILMSG(1,(TEXT("GetRealTime()::read ISL1208 fail! %x-%x-%x %x:%x:%x\r\n"),
			(unsigned int)buffer[REG1208_YEAR],
			(unsigned int)buffer[REG1208_MONTH],
			(unsigned int)buffer[REG1208_DAY],
			(unsigned int)buffer[REG1208_HOUR] & 0x3f,
			(unsigned int)buffer[REG1208_MIN],
			(unsigned int)buffer[REG1208_SECOND]
			));
		bRet = FALSE;
	}
	else				
	{	// read ISL1208 sucessfully!
		//RETAILMSG(1,(TEXT("GetRealTime()::read ISL1208 sucessfully!\r\n")));

		ptime->wSecond=GETBCDDATA(buffer[REG1208_SECOND]);
		ptime->wMinute=GETBCDDATA(buffer[REG1208_MIN]);
		ptime->wHour=GETBCDDATA(buffer[REG1208_HOUR] & 0x3F);
		ptime->wDay=GETBCDDATA(buffer[REG1208_DAY]);
		ptime->wMonth=GETBCDDATA(buffer[REG1208_MONTH]);
		ptime->wYear=GETBCDDATA(buffer[REG1208_YEAR])+2000;
		ptime->wDayOfWeek=GETBCDDATA(buffer[REG1208_WEEK]);
		
		// show the result
		RETAILMSG(1, (TEXT("ISL1208GetRealTime()::%d-%d-%d %d:%d:%d %d\r\n"),
			ptime->wYear,
			ptime->wMonth,
			ptime->wDay,
			ptime->wHour,
			ptime->wMinute,
			ptime->wSecond,
			ptime->wDayOfWeek
			));
	}

/*
	unsigned char buffer[7]={0};
	BOOL bRet=TRUE;

	if( read_ISL1208(0,(char*)buffer, 7) < 0 ){

		bRet=FALSE;
	}
	{
		int i;

		RETAILMSG(TIME_ERROR,(TEXT("Time is\r\n")));

		for(i=0; i< 7;i++ ){

			RETAILMSG(TIME_ERROR,(TEXT(" %x --- (%d) \r\n"),(BYTE)buffer[i], i ));
		}
	}
	ptime->wSecond=GETBCDDATA(buffer[REG1208_SECOND]);
	ptime->wMinute=GETBCDDATA(buffer[REG1208_MIN]);
	ptime->wHour=GETBCDDATA(buffer[REG1208_HOUR]& 0x3F);
	ptime->wDay=GETBCDDATA(buffer[REG1208_DAY]);
	ptime->wMonth=GETBCDDATA(buffer[REG1208_MONTH]);
	ptime->wYear=GETBCDDATA(buffer[REG1208_YEAR])+2000;
	ptime->wDayOfWeek=GETBCDDATA(buffer[REG1208_WEEK]);

	RETAILMSG(1,(TEXT("Time orig %d/%d/%d  %d:%d:%d\r\n")
		,ptime->wYear
		,ptime->wMonth
		,ptime->wDay
		,ptime->wHour
		,ptime->wMinute
		,ptime->wSecond
		,ptime->wDayOfWeek
		));
*/

	return bRet;
}

BOOL InitializeISL1208(LPSYSTEMTIME ptime)
{
	BOOL bRet=TRUE;
	int islStatus;

	islStatus=init_ISL1208( );
	if( islStatus < 0 )
		return FALSE;

	if( islStatus ==1 )
	{
		bRet=ISL1208GetRealTime( ptime );
	}
	else
	{
		bRet=ISL1208SetRealTime( ptime );
	}

	return bRet;
}



