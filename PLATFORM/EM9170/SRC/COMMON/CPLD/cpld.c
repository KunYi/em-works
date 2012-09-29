//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cpld.c
//
//  Provides support for the CPLD logic on the 3DS board.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4214)
#include <ceddk.h>
#pragma warning(pop)

#include <bsp.h>

//-----------------------------------------------------------------------------
// External Functions

extern VOID CSPIRequest(PCSP_CSPI_REG pCSPI, UINT32 controlReg);
extern VOID CSPIRelease(PCSP_CSPI_REG pCSPI);

// CSPI exchange packet
typedef struct
{
    UINT32 xchCnt;
    DWORD dwAddr;
    UINT16* pwBuf;
    enum { READ, WRITE} eDirection;
} CSPI_XCH_PKT0_T, *PCSPI_XCH_PKT0_T;

static UINT32 CPLDCspiNonDMADataExchange(PCSPI_XCH_PKT0_T pXchPkt);

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

#define CSPI_SS_FOR_CPLD    CSPI_CONREG_SS0

#define CPLD_CONTROLREG     (CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE) |  \
                        CSP_BITFVAL(CSPI_CONREG_MODE, CSPI_CONREG_MODE_MASTER) |    \
                        CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_IDLE) |        \
                        CSP_BITFVAL(CSPI_CONREG_SMC, CSPI_CONREG_SMC_XCH) |         \
                        CSP_BITFVAL(CSPI_CONREG_DATARATE, CSPI_CONREG_DIV4) |       \
                        CSP_BITFVAL(CSPI_CONREG_CHIPSELECT, CSPI_SS_FOR_CPLD) |     \
                        CSP_BITFVAL(CSPI_CONREG_SSPOL, CSPI_CONREG_SSPOL_ACTIVE_LOW)|\
                        CSP_BITFVAL(CSPI_CONREG_SSCTL, CSPI_CONREG_SSCTL_PULSE)  |  \
                        CSP_BITFVAL(CSPI_CONREG_POL, CSPI_CONREG_POL_ACTIVE_LOW) |  \
                        CSP_BITFVAL(CSPI_CONREG_PHA, CSPI_CONREG_PHA1) |            \
                        CSP_BITFVAL(CSPI_CONREG_BITCOUNT, 46-1) |                   \
                        CSP_BITFVAL(CSPI_CONREG_DRCTL, CSPI_CONREG_DRCTL_DONTCARE))

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static volatile PCSP_CSPI_REG g_pCSPI = NULL;

//-----------------------------------------------------------------------------
//
// Function: CPLDRead16
//
//-----------------------------------------------------------------------------
UINT16 CPLDRead16(UINT32 u32Addr)
{
    UINT16 data;
    CSPI_XCH_PKT0_T pckt;

	// CS&ZHL JUN-1-2011: check g_pCSPI
	if(g_pCSPI == NULL)
	{
		return 0;
	}

    u32Addr >>= 1;
    
    pckt.xchCnt = 2;
    pckt.pwBuf = &data;
    pckt.eDirection = READ;
    pckt.dwAddr = u32Addr;

    CSPIRequest(g_pCSPI, CPLD_CONTROLREG);
    CPLDCspiNonDMADataExchange(&pckt);
    CSPIRelease(g_pCSPI);
    return data;
}


//-----------------------------------------------------------------------------
//
// Function: CPLDReadFifo16
//
//-----------------------------------------------------------------------------
UINT32 CPLDReadFifo16(UINT32 u32Addr, UINT16 *pwBuf, UINT32 dwCount)
{
    CSPI_XCH_PKT0_T pckt;

	// CS&ZHL JUN-1-2011: check g_pCSPI
	if(g_pCSPI == NULL)
	{
		return 0;
	}

    u32Addr >>= 1;

    pckt.xchCnt = 4*dwCount;
    pckt.pwBuf = pwBuf;
    pckt.eDirection = READ;
    pckt.dwAddr = u32Addr;

    CSPIRequest(g_pCSPI, CPLD_CONTROLREG);
    CPLDCspiNonDMADataExchange(&pckt);
    CSPIRelease(g_pCSPI);
    return dwCount;
}


//-----------------------------------------------------------------------------
//
// Function: CPLDWrite16
//
//-----------------------------------------------------------------------------
void CPLDWrite16(UINT32 u32Addr, UINT16 data)
{
    CSPI_XCH_PKT0_T pckt;

	// CS&ZHL JUN-1-2011: check g_pCSPI
	if(g_pCSPI == NULL)
	{
		return;
	}

    u32Addr >>= 1;

    pckt.xchCnt = 2;
    pckt.pwBuf = &data;
    pckt.eDirection = WRITE;
    pckt.dwAddr = u32Addr;

    CSPIRequest(g_pCSPI, CPLD_CONTROLREG);
    CPLDCspiNonDMADataExchange(&pckt);
    CSPIRelease(g_pCSPI);
}


//-----------------------------------------------------------------------------
//
// Function: CPLDWriteFifo16
//
//-----------------------------------------------------------------------------
UINT32 CPLDWriteFifo16(UINT32 u32Addr, UINT16 *pwBuf, UINT32 dwCount)
{
    CSPI_XCH_PKT0_T pckt;
    
	// CS&ZHL JUN-1-2011: check g_pCSPI
	if(g_pCSPI == NULL)
	{
		return 0;
	}

	u32Addr >>= 1;
    
    pckt.xchCnt = 4*dwCount; //dwCount is in DWORD unit and xchCnt is in BYTE unit

    pckt.pwBuf = pwBuf;
    pckt.eDirection = WRITE;
    pckt.dwAddr = u32Addr;

    CSPIRequest(g_pCSPI, CPLD_CONTROLREG);
    CPLDCspiNonDMADataExchange(&pckt);
    CSPIRelease(g_pCSPI);
    
    return dwCount;
}


//-----------------------------------------------------------------------------
//
// Function: CPLDMap
//
//-----------------------------------------------------------------------------
BOOL CPLDMap(void)
{
    BOOL bRet = FALSE;
    
    // Initialize the global memory pointer to access the CSPI port
    // configuration registers.
#if (BSP_CPLD_CSPI == 1)
    g_pCSPI = (PCSP_CSPI_REG)OALPAtoUA(CSP_BASE_REG_PA_CSPI1);
#else
//#error unsupported CSPI
	g_pCSPI = NULL;
#endif
    
    if (g_pCSPI == NULL) 
    {
        OALMSG(OAL_ERROR, (TEXT("OALPAtoUA failed for CSPI\r\n")));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
    return bRet;
    
}


//-----------------------------------------------------------------------------
//
// Function: CPLDInit
//
//-----------------------------------------------------------------------------
BOOL CPLDInit(void)
{
    BOOL bRet = FALSE;
    UINT16 data;

    PCSP_IOMUX_REGS pIOMUX = (PCSP_IOMUX_REGS) OALPAtoUA(CSP_BASE_REG_PA_IOMUXC);

    RETAILMSG(1,(TEXT("CPLD Init 1\r\n")));
    
    if (pIOMUX == NULL)
    {
        RETAILMSG(1, (TEXT("ERROR:  OALPAtoUA failed for IOMUX.\r\n")));
        OALMSG(OAL_ERROR, (TEXT("ERROR:  OALPAtoUA failed for IOMUX.\r\n")));
        goto cleanUp;
    }
#if (BSP_CPLD_CSPI==1)
	RETAILMSG(1, (TEXT("CPLDInit::BSP_CPLD_CSPI = 1\r\n")));

    OAL_IOMUX_SET_MUX(pIOMUX,
        DDK_IOMUX_PIN_CSPI1_MOSI, DDK_IOMUX_PIN_MUXMODE_ALT0,
                      DDK_IOMUX_PIN_SION_REGULAR);

    OAL_IOMUX_SET_MUX(pIOMUX,
                      DDK_IOMUX_PIN_CSPI1_MISO, DDK_IOMUX_PIN_MUXMODE_ALT0,
                      DDK_IOMUX_PIN_SION_REGULAR);
    
    OAL_IOMUX_SET_MUX(pIOMUX,
                      DDK_IOMUX_PIN_CSPI1_SS0, DDK_IOMUX_PIN_MUXMODE_ALT0,
                      DDK_IOMUX_PIN_SION_REGULAR);


    OAL_IOMUX_SET_MUX(pIOMUX,
                      DDK_IOMUX_PIN_CSPI1_SCLK, DDK_IOMUX_PIN_MUXMODE_ALT0,
                      DDK_IOMUX_PIN_SION_REGULAR);
    

    // SPI1_SCLK
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_CSPI1_SCLK, DDK_IOMUX_PAD_SLEW_SLOW, 
            DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    // SPI1_SS0
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_CSPI1_SS0, DDK_IOMUX_PAD_SLEW_SLOW, 
            DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    
    // SPI1_MISO
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_CSPI1_MISO, DDK_IOMUX_PAD_SLEW_SLOW, 
            DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);


    // SPI1_MOSI
    OAL_IOMUX_SET_PAD(pIOMUX, DDK_IOMUX_PAD_CSPI1_MOSI, DDK_IOMUX_PAD_SLEW_SLOW, 
            DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
            DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);


//#else
//#error "Unsupported CSPI"
#endif

    if (!CPLDMap())
    {
        RETAILMSG(1, (TEXT("ERROR:  CPLDMap failed.\r\n")));
        OALMSG(OAL_ERROR, (TEXT("ERROR:  CPLDMap failed.\r\n")));
        goto cleanUp;
    }

    OUTREG32(&g_pCSPI->CONREG, CPLD_CONTROLREG);    
    OUTREG32(&g_pCSPI->PERIODREG, 2);    
    
    data = CPLDRead16(CPLD_RET_AAAA_OFFSET);
    if (data != 0xAAAA)
    {
        
        RETAILMSG(1, (TEXT("ERROR:  CPLD read failed (data = 0x%x)\r\n"), data));
        OALMSG(OAL_ERROR, (TEXT("ERROR:  CPLD read failed (data = 0x%x)\r\n"), data));
        
    }
    

    data = CPLDRead16(CPLD_RET_5555_OFFSET);

    if (data != 0x5555)
    {
        RETAILMSG(1, (TEXT("ERROR:  CPLD read failed (data = 0x%x)\r\n"), data));
        OALMSG(OAL_ERROR, (TEXT("ERROR:  CPLD read failed (data = 0x%x)\r\n"), data));
        goto cleanUp;
    }

    bRet = TRUE;

cleanUp:
    
    return bRet;
}

UINT32 CPLDCspiNonDMADataExchange(PCSPI_XCH_PKT0_T pXchPkt)
{
    enum {
        LOAD_TXFIFO, 
        MAX_XCHG, 
        FETCH_RXFIFO
    } xchState;
    register DWORD dwTx0,dwTx1;
    register UINT32 tmp;
    
    UINT32 xchTxCnt = 0;
    UINT32 xchRxCnt = 0;
    
    BOOL bXchDone;    

    DWORD addrToggle = 0;
    DWORD rxToggle = 0;
    DWORD u32Addr = pXchPkt->dwAddr;
    UINT16* pwBuf = pXchPkt->pwBuf;

    
    // check all translated pointers
    if (pwBuf == NULL)
    {
        return 0;
    }

    // disable all interrupts
    OUTREG32(&g_pCSPI->INTREG, 0);

    // enable the CSPI
    INSREG32(&g_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_EN), 
        CSP_BITFVAL(CSPI_CONREG_EN, CSPI_CONREG_EN_ENABLE));

    
    bXchDone = FALSE;
    xchState = LOAD_TXFIFO;

    // until we are done with requested transfers
    while(!bXchDone)
    {
        switch (xchState)
        {
            case LOAD_TXFIFO: 

                // load Tx FIFO until full, or until we run out of data
                while ((!(INREG32(&g_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TF)))
                    && (xchTxCnt < pXchPkt->xchCnt))
                {
                    if (pXchPkt->eDirection == WRITE)
                    {
                        dwTx0 = ((0 << 13) |    // WRITE
                            (1 << 12) |    // SET CS
                            (((u32Addr + addrToggle)& 0x0001FFFF) >> 5));  // A[16:5]

                        dwTx1 = ((((u32Addr + addrToggle) & 0x0000001F) << 27) | // A[4:0]
                            ((*pwBuf & 0x0000FFFF) << 6) |     // D[15:0] 
                            (0x0 << 26) |    // CLEAR CS
                            (0xF << 22) |    // WR
                            (0x13 << 1) |    // WR
                            (0x1));          // SET CS
                        pwBuf++;
                    }
                    else
                    {                        
                        dwTx0 = ((1 << 13) |      // READ
                            (1 << 12) |      // SET CS
                            ((((u32Addr + addrToggle)) & 0x0001FFFF) >> 5));  // A[16:5]

                        dwTx1 = (((((u32Addr + addrToggle)) & 0x0000001F) << 27) | // A[4:0]
                            (0x0 << 26) |    // CLEAR CS
                            (0x8 << 22) |    // RD
                            (0xF << 1) |     // RD
                            (0x1));          // SET CS
                    }


                    addrToggle = 1 - addrToggle;

                    // put next Tx data into CSPI FIFO
                    OUTREG32(&g_pCSPI->TXDATA, dwTx0);                    
                    // put next Tx data into CSPI FIFO
                    OUTREG32(&g_pCSPI->TXDATA, dwTx1);
                    // increment Tx exchange counter
                    xchTxCnt+=2;
                }
                
                // start exchange
                INSREG32(&g_pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
                    CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));

                xchState = (xchTxCnt<pXchPkt->xchCnt)? MAX_XCHG: FETCH_RXFIFO;

                break;

            case MAX_XCHG:

                if (xchTxCnt >= pXchPkt->xchCnt)
                {
                    xchState = FETCH_RXFIFO;
                    break;
                }                
                // read out data that arrived during exchange                
                {
                    // wait until RR
                    while (!(INREG32(&g_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                        ;
                    tmp = INREG32(&g_pCSPI->RXDATA);

                    if ((pXchPkt->eDirection == READ) && pwBuf)
                    {
                        if (rxToggle)
                        {
                            *pwBuf = (UINT16) (tmp >> 6);
                            pwBuf++;
                        }
                        rxToggle = 1 - rxToggle;
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;

                    // wait until RR
                    while (!(INREG32(&g_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                        ;
                    tmp = INREG32(&g_pCSPI->RXDATA);

                    
                    if ((pXchPkt->eDirection == READ) && pwBuf)
                    {
                        if (rxToggle)
                        {
                            *pwBuf = (UINT16) (tmp >> 6);
                            pwBuf++;
                        }
                        rxToggle = 1 - rxToggle;
                    }



                    // increment Rx exchange counter
                    xchRxCnt++;

                    if (pXchPkt->eDirection == WRITE)
                    {

                        dwTx0 = ((0 << 13) |    // WRITE
                            (1 << 12) |    // SET CS
                            (((u32Addr + addrToggle)& 0x0001FFFF) >> 5));  // A[16:5]

                        dwTx1 = ((((u32Addr + addrToggle) & 0x0000001F) << 27) | // A[4:0]
                            ((*pwBuf & 0x0000FFFF) << 6) |     // D[15:0] 
                            (0x0 << 26) |    // CLEAR CS
                            (0xF << 22) |    // WR
                            (0x13 << 1) |    // WR
                            (0x1));          // SET CS

                        pwBuf++;                    
                    }
                    else
                    {                        
                        dwTx0 = ((1 << 13) |      // READ
                            (1 << 12) |      // SET CS
                            ((((u32Addr + addrToggle)) & 0x0001FFFF) >> 5));  // A[16:5]

                        dwTx1 = (((((u32Addr + addrToggle)) & 0x0000001F) << 27) | // A[4:0]
                            (0x0 << 26) |    // CLEAR CS
                            (0x8 << 22) |    // RD
                            (0xF << 1) |     // RD
                            (0x1));          // SET CS
                    }

                    addrToggle = 1 - addrToggle;

                    // put next Tx data into CSPI FIFO
                    OUTREG32(&g_pCSPI->TXDATA, dwTx0);                    
                    // put next Tx data into CSPI FIFO
                    OUTREG32(&g_pCSPI->TXDATA, dwTx1);
                    // increment Tx exchange counter
                    xchTxCnt+=2;

                }
                
                if (INREG32(&g_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC))
                {
                    xchState = FETCH_RXFIFO;
                }
                break;

            case FETCH_RXFIFO:

                // Fetch all rxdata already in RXFIFO
                while ((INREG32(&g_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                {
                    tmp = INREG32(&g_pCSPI->RXDATA);

                    if ((pXchPkt->eDirection == READ) && pwBuf)
                    {
                        if (rxToggle)
                        {
                            *pwBuf = (UINT16) (tmp >> 6);
                            pwBuf++;
                        }
                        rxToggle = 1 - rxToggle;
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;
                }

                // wait until transaction is complete
                while (!(INREG32(&g_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC)))
                {
                }
                

                while ((INREG32(&g_pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                {
                    tmp = INREG32(&g_pCSPI->RXDATA);

                    // if receive data is not to be discarded
                    if ((pXchPkt->eDirection == READ) && pwBuf)
                    {
                        if (rxToggle)
                        {
                            *pwBuf = (UINT16) (tmp >> 6);
                            pwBuf++;
                        }
                        rxToggle = 1 - rxToggle;
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;
                }

                // acknowledge transfer complete (w1c)
                OUTREG32(&g_pCSPI->STATREG, CSP_BITFMASK(CSPI_STATREG_TC));
 
                if (xchRxCnt >= pXchPkt->xchCnt)
                {
                    // set flag to indicate requested exchange done
                    bXchDone = TRUE;
                }
                else 
                {
                    // exchange stopped, and we have received all data in RXFIFO
                    // then there MUST be some data in the TxBuf, or in the TXFIFO,
                    // or both.
                    // we return the state to restart cspi XCH. 
                    xchState = LOAD_TXFIFO;
                }

                break;

            default:
                bXchDone = TRUE;
                break;
        }
    }
    
    return xchRxCnt;
}
