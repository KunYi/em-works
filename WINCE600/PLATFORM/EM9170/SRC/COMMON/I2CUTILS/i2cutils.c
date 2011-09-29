//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: i2cutils.c
//
//  This file contains the OAL support code for the I2C interface.
//
//-----------------------------------------------------------------------------
#include <bsp.h>

//-----------------------------------------------------------------------------
// External Functions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index,
    DDK_CLOCK_GATE_MODE mode);

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
//PMIC/EEPROM share same I2C port in on this board (library will not work if EEPROM & PMIC are on different busses)
#define BSP_PMIC_EEPROM_BUS_I2C_PORT       1
#define BSP_PMIC_EEPROM_BUS_I2C_FREQ       100000
#define BSP_PMIC_EEPROM_BUS_I2C_CPU_ADDR   0x20


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables

//
// Enable or disable debugging for the I2C.
//
#define DEBUG_I2C               FALSE
#if DEBUG_I2C
#define I2C_TRACE( _params )    OALMSGS( OAL_INFO, _params )
#else
#define I2C_TRACE( _params )
#endif

//-----------------------------------------------------------------------------
// Local Variables

static PCSP_I2C_REG g_pI2CReg;
static DDK_CLOCK_GATE_INDEX g_CGI;

static const WORD wI2CClockRateDivider[] = {
    30,  32,  36,  42,  48,  52,  60,  72,   80,   88,  104,  128,  144,  160,  192,  240,
    288, 320, 384, 480, 576, 640, 768, 960, 1152, 1280, 1536, 1920, 2304, 2560, 3072, 3840,
    22,  24,  26,  28,  32,  36,  40,  44,   48,   56,   64,   72,   80,   96,  112,  128,
    160, 192, 224, 256, 320, 384, 448, 512,  640,  768,  896, 1024, 1280, 1536, 1792, 2048
};

#define I2CDIVTABSIZE   (sizeof(wI2CClockRateDivider)/sizeof(wI2CClockRateDivider[0]))
#define I2C_MAXDIVIDER   3840
#define I2C_MINDIVIDER   22

//-----------------------------------------------------------------------------
// Functions

BOOL OALI2cInit( BYTE *pbySelfAddr, BYTE *pbyClkDiv );
void OALI2cEnable( BOOL bChangeAddr, BOOL bChangeClkDiv, BYTE bySelfAddr, BYTE byClkDiv );
void OALI2cDisable();
WORD OALI2cCalculateClkRateDiv( DWORD dwFrequency );
BOOL OALI2cGenerateStart( BYTE devAddr, BOOL bWrite );
BOOL OALI2cRepeatedStart( BYTE devAddr, BOOL bWrite, BOOL bRSTACycleComplete );
void OALI2cSetReceiveMode( BOOL bReceive );
void OALI2cSetTxAck( BOOL bTxAck );

BOOL OALI2cPutData( BYTE *pData, WORD nBufLen, 
                   BOOL bStopAhead, 
                   BOOL bRepeatedStartCycleAhead, 
                   BOOL *pbStopped, 
                   BOOL *pbRSTACycleCompleted );

BOOL OALI2cGetData( BYTE *pData, WORD nBufLen, 
                   BOOL bStopAhead, 
                   BOOL bRepeatedStartCycleAhead, 
                   BOOL *pbStopped, 
                   BOOL *pbRSTACycleCompleted );

BOOL OALI2cWriteData( BYTE *pData, WORD nBufLen, 
                     BOOL bStopAhead, 
                     BOOL bRepeatedStartCycleAhead, 
                     BOOL *pbStopped, 
                     BOOL *pbRSTACycleCompleted );

BOOL OALI2cReadData( BYTE *pData, WORD nBufLen, 
                    BOOL bStopAhead, 
                    BOOL bRepeatedStartCycleAhead, 
                    BOOL *pbStopped, 
                    BOOL *pbRSTACycleCompleted );

void OALI2cGenerateStop();


//-----------------------------------------------------------------------------
//
//  Function: OALI2cInit
//
//  Initializes the PMIC/EEPROM i2c interface for OAL communication. EEPROM/PMIC share same I2C bus on this board.
//
//  Parameters:
//      pbySelfAddr:[Output]
//          Return applied i2c self address if not NULL.
//
//      pbyClkDiv:
//          Return applied i2c frequency dividor if not NULL.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALI2cInit( BYTE *pbySelfAddr, BYTE *pbyClkDiv )
{
    BOOL    rc = FALSE;
    UINT32  clkdiv;
    PCSP_IOMUX_REGS pIOMUX;

    OALMSGS( OAL_INFO, (_T("OALI2cInit: Trying to init I2C Interface\r\n")));

    pIOMUX = (PCSP_IOMUX_REGS) OALPAtoUA(CSP_BASE_REG_PA_IOMUXC);
    if (pIOMUX == NULL)
    {

        OALMSG(TRUE, (L"OALI2cInit: IOMUXC mapping failed!\r\n"));
        return FALSE;
    }
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_I2C, DDK_CLOCK_GATE_MODE_ENABLED);
#if (BSP_PMIC_EEPROM_BUS_I2C_PORT == 1)
    g_CGI = DDK_CLOCK_GATE_INDEX_I2C1;

    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_I2C1_DAT, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_FORCE);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_I2C1_CLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_FORCE);

    g_pI2CReg = (PCSP_I2C_REG) OALPAtoUA( CSP_BASE_REG_PA_I2C1 );
#elif (BSP_PMIC_EEPROM_BUS_I2C_PORT == 2)
    g_CGI = DDK_CLOCK_GATE_INDEX_I2C2;

    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_RDATA1, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_FEC_RX_DV, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);

    g_pI2CReg = (PCSP_I2C_REG) OALPAtoUA( CSP_BASE_REG_PA_I2C2 );
#elif (BSP_PMIC_EEPROM_BUS_I2C_PORT == 3)
    g_CGI = DDK_CLOCK_GATE_INDEX_I2C3;

    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_GPIO_A, DDK_IOMUX_PIN_MUXMODE_ALT4, DDK_IOMUX_PIN_SION_REGULAR);
    OAL_IOMUX_SET_MUX(pIOMUX, DDK_IOMUX_PIN_GPIO_B, DDK_IOMUX_PIN_MUXMODE_ALT4, DDK_IOMUX_PIN_SION_REGULAR);

    g_pI2CReg = (PCSP_I2C_REG) OALPAtoUA( CSP_BASE_REG_PA_I2C3 );
#else
#error "Invalid I2C port"
#endif

    if ( !g_pI2CReg )
    {
        OALMSGS( OAL_ERROR, (_T("OALPmicInit: OALPAtoUA returned NULL\r\n")));
        goto cleanUp;
    }

    // Turn on I2C clocks
    OALClockSetGatingMode( g_CGI, DDK_CLOCK_GATE_MODE_ENABLED );

    // Reset I2CR & I2DR
    OUTREG16(&g_pI2CReg->I2CR, 0x0);
    OUTREG16(&g_pI2CReg->I2DR, 0x0);

    // Configure data sampling rate
    clkdiv = OALI2cCalculateClkRateDiv( BSP_PMIC_EEPROM_BUS_I2C_FREQ );
    OUTREG16(&g_pI2CReg->IFDR, CSP_BITFVAL(I2C_IFDR_IC, clkdiv));
    if (pbyClkDiv)
    {
        *pbyClkDiv = (BYTE)(clkdiv&0xff);
    }

    // Configure slave address
    OUTREG16(&g_pI2CReg->IADR, CSP_BITFVAL(I2C_IADR_ADR, BSP_PMIC_EEPROM_BUS_I2C_CPU_ADDR));
    if (pbySelfAddr)
    {
        *pbySelfAddr = (BYTE)(BSP_PMIC_EEPROM_BUS_I2C_CPU_ADDR);
    }

    // Turn the i2c module and clocks off to save power between transfers
    OUTREG16(&g_pI2CReg->I2CR, 0);
    OALClockSetGatingMode(g_CGI, DDK_CLOCK_GATE_MODE_DISABLED);

    I2C_TRACE(( _T("%s: I2C%d: REGS = 0x%X\r\n"),
               _T(__FUNCTION__),
               g_pI2CReg  ));
           

    rc = TRUE;

cleanUp:

    return rc;

}

 
//-----------------------------------------------------------------------------
//
//  Function: OALI2cEnable
//
//  Obtains the control of i2c hardware.
//
//  Parameters:
//      bChangeAddr:
//          if the caller want to change the i2c self address.
//
//      bChangeClkDiv:
//          if the caller want to change the i2c frequency dividor.
//
//      bySelfAddr:
//          the i2c self address to set if bChangeAddr is set to TRUE, 
//          otherwise, ignore this parameter.
//
//      bChangeClkDiv:
//          the i2c frequency dividor to set if bChangeClkDiv is set to TRUE, 
//          otherwise, ignore this parameter.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void OALI2cEnable( BOOL bChangeAddr, BOOL bChangeClkDiv, BYTE bySelfAddr, BYTE byClkDiv )
{
    I2C_TRACE(( _T("%s\r\n"), _T(__FUNCTION__) ));
               
    // Turn on I2C clocks
    OALClockSetGatingMode( g_CGI, DDK_CLOCK_GATE_MODE_ENABLED );

    // Reset I2CR
    OUTREG16(&g_pI2CReg->I2CR, 0x0);

    // Try resetting I2DR = 0x0
    OUTREG16(&g_pI2CReg->I2DR, 0x0);

    if (bChangeClkDiv)
    {
        OUTREG16(&g_pI2CReg->IFDR, CSP_BITFVAL(I2C_IFDR_IC, byClkDiv));
    }

    if (bChangeAddr)
    {
        // Configure slave address
        OUTREG16(&g_pI2CReg->IADR, CSP_BITFVAL(I2C_IADR_ADR, bySelfAddr));
    }

    // Enable I2C
    INSREG16(&g_pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_IEN), CSP_BITFVAL(I2C_I2CR_IEN, I2C_I2CR_IEN_ENABLE));
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cDisable
//
//  Release the control of i2c hardware.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void OALI2cDisable()
{
    // Disable I2C Module
    OUTREG16(&g_pI2CReg->I2CR, 0);
    OALClockSetGatingMode(g_CGI, DDK_CLOCK_GATE_MODE_DISABLED);
    
    I2C_TRACE(( _T("%s\r\n"), _T(__FUNCTION__) ));     
}

//-----------------------------------------------------------------------------
//
//  Function: I2CCalculateClkRateDiv
//
//  This function will, on obtaining the frequency, determines the nearest clock
//  rate divider needed to step the I2C Bus up/down. 
//
//  Parameters:
//      dwFrequency
//          [in] Contains the desired clock frequency of the slave device.
//
//  Returns:
//      Returns an index to the array wI2CClockRateDivider. The content in the
//      index holds the nearest clock rate divider available.
//
//-----------------------------------------------------------------------------

WORD OALI2cCalculateClkRateDiv( DWORD dwFrequency )
{
    INT iFirstRd, iSecondRd;
    WORD wEstDivider;
    UINT32 freq;
    BYTE byCRDALen = I2CDIVTABSIZE;
    BSP_ARGS *pBspArgs = (BSP_ARGS *) IMAGE_SHARE_ARGS_UA_START;

    // Obtain an estimate of the divider required
    freq = pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_PER_I2C];
    wEstDivider = (WORD)(freq / dwFrequency);

    // Tolerance control, the look for frequency shall never exceed target frequency +1%
    if ((freq-(dwFrequency*wEstDivider))*100>dwFrequency)
    {
        ++wEstDivider;
    }

    // Search for the nearest divider in the first half of the array
    for (iFirstRd = 0; iFirstRd < (byCRDALen/2-1); iFirstRd++)
    {
        // Once found a divider greater than the estimate, stop
        if (wEstDivider <= wI2CClockRateDivider[iFirstRd])
            break;
    }
    if (wEstDivider == wI2CClockRateDivider[iFirstRd])
    {
        // If the estimated divider matched one of the array entries, no need
        // to search further
        wEstDivider = (WORD)iFirstRd;
    }
    else
    {
        // Going to second round
        for (iSecondRd = (byCRDALen/2); iSecondRd < (byCRDALen-1); iSecondRd++)
        {
            // Again, if a greater entry is found, stop
            if (wEstDivider <= wI2CClockRateDivider[iSecondRd])
                break;
        }
        if (wEstDivider == wI2CClockRateDivider[iSecondRd])
        {
            // If the estimated divider is found in the second round, stop
            wEstDivider = (WORD)iSecondRd;
        }
        else
        {
            // Search for the nearest divider among the 2 portion of the array
            if ((wI2CClockRateDivider[iFirstRd] > wEstDivider) && (wI2CClockRateDivider[iSecondRd] > wEstDivider))
            {
                if ((wI2CClockRateDivider[iFirstRd] - wEstDivider) < (wI2CClockRateDivider[iSecondRd] - wEstDivider))
                    wEstDivider = (WORD)iFirstRd;
                else
                    wEstDivider = (WORD)iSecondRd;
            }
            else
                if (wI2CClockRateDivider[iSecondRd] > wEstDivider)
                {
                    wEstDivider = (WORD)iSecondRd;
                }
                else
                {
                    // Less than setting, use wI2CClockRateDivider[31] as default
                    wEstDivider = (WORD)iFirstRd;
                }
        }
    }
    // Obtain the nearest clock rate divider
    return wEstDivider;
}

//-----------------------------------------------------------------------------
//
//  Function: OALI2cGenerateStart
//
//  Generates an I2C START signal to a slave device.
//
//  Parameters:
//      devAddr:
//          The slave device address to access.
//
//      bWrite:
//          If true, select write access mode, else select read access mode.
//
//  Returns:
//      TRUE    Success
//      FALSE   Timeout waiting for completion
//
//-----------------------------------------------------------------------------
BOOL OALI2cGenerateStart( BYTE devAddr, BOOL bWrite )
{
    UINT16  i2sr;
    UINT16  i2cr;
    UINT16  i2dr;

    int count;

    //
    // Wait until the bus goes quiet then claim ownership.
    //
    count = 0;
    while (EXTREG16BF(&g_pI2CReg->I2SR, I2C_I2SR_IBB)==I2C_I2SR_IBB_BUSY )
    { 
        count++;
        if ( count >= 1000 )
        {
            OALMSG( OAL_ERROR, (
                  _T("%s: Bus busy for %d cycles\r\n"),
                  _T(__FUNCTION__),
                  count ));
            goto error;
        }
        i2sr = INREG16(&g_pI2CReg->I2SR);
    }
            
    OUTREG16(&g_pI2CReg->I2SR, 0);

    // Grant Bus Master
    INSREG16(&g_pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
             CSP_BITFVAL(I2C_I2CR_MSTA, I2C_I2CR_MSTA_MASTER));

    // Transmit the slave address, then change to receive mode after
    // we complete the address cycle.
    INSREG16(&g_pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX),
             CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT));
    //
    // Check whether we've lost arbitration.
    //
    if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
    {
        // Clear IAL bit (we are already put into Stop)
        INSREG16( &g_pI2CReg->I2SR,
                  CSP_BITFMASK( I2C_I2SR_IAL ),
                  CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ));
               
        goto error;
    }

    i2dr = CSP_BITFVAL( I2C_IADR_ADR, devAddr ) | (bWrite ? 0 : 1);
    OUTREG16( &g_pI2CReg->I2DR, i2dr );

    // Wait for data transmission to complete.
    i2sr = INREG16( &g_pI2CReg->I2SR );
    count = 0;
    while ((i2sr&CSP_BITFMASK(I2C_I2SR_IIF))==I2C_I2SR_IIF_NOT_PENDING)
    {
        count++;
        i2sr = INREG16( &g_pI2CReg->I2SR );
        i2cr = INREG16( &g_pI2CReg->I2CR );
    }

    //
    // Check whether we've lost arbitration.
    //
    if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
    {
        // Clear IAL bit (we are already put into Stop)
        INSREG16( &g_pI2CReg->I2SR,
                  CSP_BITFMASK( I2C_I2SR_IAL ),
                  CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ));
               
        goto error;
    }

    // Clear the interrupt bit again.
    INSREG16( &g_pI2CReg->I2SR, CSP_BITFMASK( I2C_I2SR_IIF ),
              CSP_BITFVAL( I2C_I2SR_IIF, I2C_I2SR_IIF_NOT_PENDING ));
    return TRUE;

error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cRepeatedStart
//
//  Generates an I2C repeated start signal to a slave device.
//
//  Parameters:
//      devAddr:
//          The slave device address to access.
//
//      bWrite:
//          If true, select write access mode, else select read access mode.
//
//      bRSTACycleComplete:
//          If true, tell the RSTA signal is generated ahead. 
//
//  Returns:
//      TRUE    Success
//      FALSE   Timeout waiting for interrupt event
//
//-----------------------------------------------------------------------------
BOOL OALI2cRepeatedStart( BYTE devAddr, BOOL bWrite, BOOL bRSTACycleComplete )
{
    UINT16  i2sr;
    UINT16  i2cr;
    UINT16  i2dr;
    int     count;

    if (!bRSTACycleComplete)
    {
        OUTREG16(&g_pI2CReg->I2SR, 0);

        // Set the repeated start bit in the I2C CR.
        INSREG16( &g_pI2CReg->I2CR,
                  CSP_BITFMASK( I2C_I2CR_RSTA ),
                  CSP_BITFVAL( I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE ) );
    }

    // Switch to Transmit Mode
    INSREG16(&g_pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MTX), CSP_BITFVAL(I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT));

    // Temporary fix related to Repeated Start.  Delay after repeated start for 1 PAT_REF_CLK period.
    //INREG16( &g_pI2CReg->I2SR );
    //INREG16( &g_pI2CReg->I2CR );

    //
    // Check whether repeated start flag cleared.
    //
    if ( EXTREG16BF( &g_pI2CReg->I2CR, I2C_I2CR_RSTA ) )
    {
        goto error;
    }
    //
    // Check whether we've lost arbitration.
    //
    if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
    {
        // Clear IAL bit (we are already put into Stop)
        INSREG16( &g_pI2CReg->I2SR,
                  CSP_BITFMASK( I2C_I2SR_IAL ),
                  CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ) );
               
        goto error;
    }

    // Set the device address again
    i2dr = CSP_BITFVAL( I2C_IADR_ADR, devAddr ) | (bWrite ? 0 : 1);
    OUTREG16( &g_pI2CReg->I2DR, i2dr );

    // Wait for data transmission to complete.
    i2sr = INREG16( &g_pI2CReg->I2SR );
    count =0;
    while ( (i2sr&CSP_BITFMASK(I2C_I2SR_IIF)) == I2C_I2SR_IIF_NOT_PENDING )
    {
        count++;
        i2sr = INREG16( &g_pI2CReg->I2SR );
        i2cr = INREG16( &g_pI2CReg->I2CR );
    }

    //
    // Check whether we've lost arbitration.
    //
    if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
    {
        // Clear IAL bit (we are already put into Stop)
        INSREG16( &g_pI2CReg->I2SR,
                  CSP_BITFMASK( I2C_I2SR_IAL ),
                  CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ) );
               
        goto error;
    }

    // Clear the interrupt bit again.
    INSREG16( &g_pI2CReg->I2SR, CSP_BITFMASK( I2C_I2SR_IIF ),
              CSP_BITFVAL( I2C_I2SR_IIF, I2C_I2SR_IIF_NOT_PENDING ));


    return TRUE;

error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cWriteData
//
//  This method performs a write operation to the I2C bus.
//
//  Parameters:
//      pData:
//          Data to be transferred.
//
//      nBufLen:
//          Data number in transfer buffer.
//
//      bStopAhead:
//          if TRUE, will apply stop operation ahead.
//
//      bRepeatedStartCycleAhead:
//          if TRUE, will apply Repeated Start operation ahead.
//
//      pbStopped:
//          return stop operation result.
//
//      pbRSTACycleCompleted:
//          return Repeated Start operation result.
//
//  Returns:
//      TRUE    Success
//      FALSE   Timeout waiting for interrupt event
//
//-----------------------------------------------------------------------------
BOOL OALI2cWriteData( BYTE *pData, WORD nBufLen,
                    BOOL bStopAhead, 
                    BOOL bRepeatedStartCycleAhead, 
                    BOOL *pbStopped, 
                    BOOL *pbRSTACycleCompleted )
{
    PBYTE   pBufPtr = pData;
    UINT16  i2sr;
    UINT16  i2cr;
    WORD    wdAckDetect;
    int     i;
    int     count;

    // Set MTX to switch to transmit mode
    INSREG16( &g_pI2CReg->I2CR,
              CSP_BITFMASK( I2C_I2CR_MTX ),
              CSP_BITFVAL( I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT ));

    //
    // Transmit each byte.
    //
    for ( i = 0; i < nBufLen; i++ )
    {
        OUTREG16( &g_pI2CReg->I2DR, *pBufPtr );
        pBufPtr++;


        // Wait for data transmission to complete.
        i2sr = INREG16( &g_pI2CReg->I2SR );
        count =0; 
        while ( (i2sr&CSP_BITFMASK(I2C_I2SR_IIF)) == I2C_I2SR_IIF_NOT_PENDING )
        {
            count++;
            i2sr = INREG16( &g_pI2CReg->I2SR );
            i2cr = INREG16( &g_pI2CReg->I2CR );
        }

        //
        // Check whether we've lost arbitration.
        //
        if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
        {
            // Clear IAL bit (we are already put into Stop)
            INSREG16( &g_pI2CReg->I2SR,
                      CSP_BITFMASK( I2C_I2SR_IAL ),
                      CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ));
                    
            goto error;
        }

        // Clear the interrupt bit again.
        INSREG16( &g_pI2CReg->I2SR, CSP_BITFMASK( I2C_I2SR_IIF ),
                  CSP_BITFVAL( I2C_I2SR_IIF, I2C_I2SR_IIF_NOT_PENDING ));


        wdAckDetect = EXTREG16BF( &g_pI2CReg->I2SR,  I2C_I2SR_RXAK );
                          
        if ( wdAckDetect ==I2C_I2SR_RXAK_NO_ACK_DETECT )
        {
            // Send a STOP Signal
            INSREG16( &g_pI2CReg->I2CR,
                      CSP_BITFMASK( I2C_I2CR_MSTA ),
                      CSP_BITFVAL( I2C_I2CR_MSTA, I2C_I2CR_MSTA_SLAVE ));
                    
            if ( pbStopped )
            {
                *pbStopped = TRUE;
            }
            goto error;
        }
    }

    if ( bStopAhead )
    {
        OALI2cGenerateStop();
        if ( pbStopped )
        {
            *pbStopped = TRUE;
        }
    }
    else if ( bRepeatedStartCycleAhead )
    {
        // Set the repeated start bit in the I2C CR.
        INSREG16( &g_pI2CReg->I2CR,
                  CSP_BITFMASK( I2C_I2CR_RSTA ),
                  CSP_BITFVAL( I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE ) );

        if ( pbRSTACycleCompleted )
        {
            *pbRSTACycleCompleted = TRUE;
        }
    }

    return TRUE;

error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cReadData
//
//  This method performs a read operation for the a series of bytes data.
//  Note that the device address must already have been opened.
//
//  Parameters:
//      pData:
//          Data receive buffer.
//
//      nBufLen:
//          Data receive buffer length.
//
//      bStopAhead:
//          if TRUE, will apply stop operation ahead.
//
//      bRepeatedStartCycleAhead:
//          if TRUE, will apply Repeated Start operation ahead.
//
//      pbStopped:
//          return stop operation result.
//
//      pbRSTACycleCompleted:
//          return Repeated Start operation result.
//
//  Returns:
//      TRUE    Success
//      FALSE   Timeout waiting for interrupt event
//
//-----------------------------------------------------------------------------
BOOL OALI2cReadData( BYTE *pData, WORD nBufLen,
                    BOOL bStopAhead, 
                    BOOL bRepeatedStartCycleAhead, 
                    BOOL *pbStopped, 
                    BOOL *pbRSTACycleCompleted )
{
    PBYTE   pReadBuf;
    UINT16  i2sr;
    UINT16  i2cr;
    int     i;
    int     count;

    // Switch to Receive Mode
    INSREG16( &g_pI2CReg->I2CR, CSP_BITFMASK( I2C_I2CR_MTX ),
              CSP_BITFVAL( I2C_I2CR_MTX, I2C_I2CR_MTX_RECEIVE ));

    if ( nBufLen == 1)
    {
        INSREG16( &g_pI2CReg->I2CR, CSP_BITFMASK( I2C_I2CR_TXAK ),
                  CSP_BITFVAL( I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND ));
    }
    else
    {
        INSREG16( &g_pI2CReg->I2CR, CSP_BITFMASK( I2C_I2CR_TXAK ),
                  CSP_BITFVAL( I2C_I2CR_TXAK, I2C_I2CR_TXAK_ACK_SEND ));
    }

    pReadBuf = pData;
    // Dummy read to trigger I2C Read operation
    INREG16( &g_pI2CReg->I2DR );

    for ( i = 0; i < nBufLen; i++ )
    {

        // Wait for data transmission to complete.
        i2sr = INREG16( &g_pI2CReg->I2SR );
        count =0;
        while ( (i2sr&CSP_BITFMASK(I2C_I2SR_IIF)) == I2C_I2SR_IIF_NOT_PENDING )
        {
            count++;
            i2sr = INREG16( &g_pI2CReg->I2SR );
            i2cr = INREG16( &g_pI2CReg->I2CR );
        }

        //
        // Check whether we've lost arbitration.
        //
        if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
        {
            // Clear IAL bit (we are already put into Stop)
            INSREG16( &g_pI2CReg->I2SR,
                      CSP_BITFMASK( I2C_I2SR_IAL ),
                      CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ));
                   
            goto error;
        }

        // Clear the interrupt bit again.
        INSREG16( &g_pI2CReg->I2SR, CSP_BITFMASK( I2C_I2SR_IIF ),
                  CSP_BITFVAL( I2C_I2SR_IIF, I2C_I2SR_IIF_NOT_PENDING ));

        // Do not generate an ACK for the last byte
        if ( i == (nBufLen - 2))
        {
            // Change to No ACK for last byte. 
            INSREG16( &g_pI2CReg->I2CR,
                      CSP_BITFMASK( I2C_I2CR_TXAK ),
                      CSP_BITFVAL( I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND ));
        }

        if ( bStopAhead )
        {
            OALI2cGenerateStop();
            if ( pbStopped )
            {
                *pbStopped = TRUE;
            }
        }
        else if ( bRepeatedStartCycleAhead )
        {
            // Set the repeated start bit in the I2C CR.
            INSREG16( &g_pI2CReg->I2CR,
                      CSP_BITFMASK( I2C_I2CR_RSTA ),
                      CSP_BITFVAL( I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE ) );

            if ( pbRSTACycleCompleted )
            {
                *pbRSTACycleCompleted = TRUE;
            }
        }

        *pReadBuf = (BYTE) INREG16( &g_pI2CReg->I2DR );
        ++pReadBuf;
    }

    return TRUE;

error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cSetReceiveMode
//
//  This method set receive/send interface in i2c bus interface.
//
//  Parameters:
//      bReceive:
//          If true, set i2c hardwire in receive mode.
//          If false, set i2c hardwire in transimit mode.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void OALI2cSetReceiveMode( BOOL bReceive )
{
    if ( bReceive )
    {
        INSREG16( &g_pI2CReg->I2CR, CSP_BITFMASK( I2C_I2CR_MTX ),
                  CSP_BITFVAL( I2C_I2CR_MTX, I2C_I2CR_MTX_RECEIVE ));
    }
    else
    {
        INSREG16( &g_pI2CReg->I2CR, CSP_BITFMASK( I2C_I2CR_MTX ),
                  CSP_BITFVAL( I2C_I2CR_MTX, I2C_I2CR_MTX_TRANSMIT ));
    }
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cSetTxAck
//
//  This method select Ack/NAck signal for receive slave data.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void OALI2cSetTxAck( BOOL bTxAck )
{
    if ( bTxAck )
    {
        INSREG16( &g_pI2CReg->I2CR, CSP_BITFMASK( I2C_I2CR_TXAK ),
                  CSP_BITFVAL( I2C_I2CR_TXAK, I2C_I2CR_TXAK_ACK_SEND ));
    }
    else
    {
        INSREG16( &g_pI2CReg->I2CR, CSP_BITFMASK( I2C_I2CR_TXAK ),
                  CSP_BITFVAL( I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND ));
    }
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cPutData
//
//  This method performs a write operation to the I2C bus.
//  Next i2c write operation can be continued 
//  before generate stop/repeated start.
//
//  Release i2c bus on error
//  Note that the interface is already in transmit mode
//
//  Parameters:
//      pData:
//          Data send buffer.
//
//      nBufLen:
//          Data to send.
//
//      bStopAhead:
//          if TRUE, will apply stop operation ahead.
//
//      bRepeatedStartCycleAhead:
//          if TRUE, will apply Repeated Start operation ahead.
//
//      pbStopped:
//          return stop operation result.
//
//      pbRSTACycleCompleted:
//          return Repeated Start operation result.
//
//  Returns:
//      TRUE    Success
//      FALSE   Timeout waiting for interrupt event
//
//-----------------------------------------------------------------------------
BOOL OALI2cPutData( BYTE *pData, WORD nBufLen,
                    BOOL bStopAhead, 
                    BOOL bRepeatedStartCycleAhead, 
                    BOOL *pbStopped, 
                    BOOL *pbRSTACycleCompleted )
{
    PBYTE   pBufPtr;
    UINT16  i2sr;
    UINT16  i2cr;
    WORD    wdAckDetect;
    int     i;
    int     count;

    //
    // Transmit each byte.
    //
    pBufPtr = pData;
    for ( i = 0; i < nBufLen; i++ )
    {
        OUTREG16( &g_pI2CReg->I2DR, *pBufPtr );
        pBufPtr++;


        // Wait for data transmission to complete.
        i2sr = INREG16( &g_pI2CReg->I2SR );
        count =0;
        while ( (i2sr&CSP_BITFMASK(I2C_I2SR_IIF)) == I2C_I2SR_IIF_NOT_PENDING )
        {
            count++;
            i2sr = INREG16( &g_pI2CReg->I2SR );
            i2cr = INREG16( &g_pI2CReg->I2CR );
        }

        //
        // Check whether we've lost arbitration.
        //
        if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
        {
            // Clear IAL bit (we are already put into Stop)
            INSREG16( &g_pI2CReg->I2SR,
                      CSP_BITFMASK( I2C_I2SR_IAL ),
                      CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ));
                   
            goto error;
        }

        // Clear the interrupt bit again.
        INSREG16( &g_pI2CReg->I2SR, CSP_BITFMASK( I2C_I2SR_IIF ),
                  CSP_BITFVAL( I2C_I2SR_IIF, I2C_I2SR_IIF_NOT_PENDING ));


        wdAckDetect = EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_RXAK );
                          
        if ( wdAckDetect == I2C_I2SR_RXAK_NO_ACK_DETECT )
        {
            // Send a STOP Signal
            INSREG16( &g_pI2CReg->I2CR,
                      CSP_BITFMASK( I2C_I2CR_MSTA ),
                      CSP_BITFVAL( I2C_I2CR_MSTA, I2C_I2CR_MSTA_SLAVE ));
                    
            if ( pbStopped )
            {
                *pbStopped = TRUE;
            }
            goto error;
        }
    }

    if ( bStopAhead )
    {
        OALI2cGenerateStop();
        if ( pbStopped )
        {
            *pbStopped = TRUE;
        }
    }
    else if ( bRepeatedStartCycleAhead )
    {
        // Set the repeated start bit in the I2C CR.
        INSREG16( &g_pI2CReg->I2CR,
                  CSP_BITFMASK( I2C_I2CR_RSTA ),
                  CSP_BITFVAL( I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE ) );

        if ( pbRSTACycleCompleted )
        {
            *pbRSTACycleCompleted = TRUE;
        }
    }

    return TRUE;

error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cGetData
//
//  This method performs a read operation for the a series of bytes data.
//  Next i2c read operation can be continued 
//  before generate stop/repeated start.
//
//  Parameters:
//      pData:
//          Data receive buffer.
//
//      nBufLen:
//          Data receive buffer length.
//
//      bStopAhead:
//          if TRUE, will apply stop operation ahead.
//
//      bRepeatedStartCycleAhead:
//          if TRUE, will apply Repeated Start operation ahead.
//
//      pbStopped:
//          return stop operation result.
//
//      pbRSTACycleCompleted:
//          return Repeated Start operation result.
//
//  Returns:
//      TRUE    Success
//      FALSE   Timeout waiting for interrupt event
//
//-----------------------------------------------------------------------------
BOOL OALI2cGetData( BYTE *pData, WORD nBufLen,
                    BOOL bStopAhead, 
                    BOOL bRepeatedStartCycleAhead, 
                    BOOL *pbStopped, 
                    BOOL *pbRSTACycleCompleted )
{
    PBYTE   pReadBuf;
    UINT16  i2sr;
    UINT16  i2cr;
    int     i;
    int     count;

    pReadBuf = pData;
    for ( i = 0; i < nBufLen; i++ )
    {
        // Wait for data transmission to complete.
        i2sr = INREG16( &g_pI2CReg->I2SR );
        count =0;
        while ( (i2sr&CSP_BITFMASK(I2C_I2SR_IIF)) == I2C_I2SR_IIF_NOT_PENDING )
        {
            count++;
            i2sr = INREG16( &g_pI2CReg->I2SR );
            i2cr = INREG16( &g_pI2CReg->I2CR );
        }

        //
        // Check whether we've lost arbitration.
        //
        if ( EXTREG16BF( &g_pI2CReg->I2SR, I2C_I2SR_IAL ) )
        {
            // Clear IAL bit (we are already put into Stop)
            INSREG16( &g_pI2CReg->I2SR,
                      CSP_BITFMASK( I2C_I2SR_IAL ),
                      CSP_BITFVAL( I2C_I2SR_IAL, I2C_I2SR_IAL_NOT_LOST ));
                   
            goto error;
        }

        // Clear the interrupt bit again.
        INSREG16( &g_pI2CReg->I2SR, CSP_BITFMASK( I2C_I2SR_IIF ),
                  CSP_BITFVAL( I2C_I2SR_IIF, I2C_I2SR_IIF_NOT_PENDING ));

        if ( bStopAhead )
        {
            OALI2cGenerateStop();
            if ( pbStopped )
            {
                *pbStopped = TRUE;
            }
        }
        else if ( bRepeatedStartCycleAhead )
        {
            // Set the repeated start bit in the I2C CR.
            INSREG16( &g_pI2CReg->I2CR,
                      CSP_BITFMASK( I2C_I2CR_RSTA ),
                      CSP_BITFVAL( I2C_I2CR_RSTA, I2C_I2CR_RSTA_GENERATE ) );

            if ( pbRSTACycleCompleted )
            {
                *pbRSTACycleCompleted = TRUE;
            }
        }

        *pReadBuf = (BYTE) INREG16( &g_pI2CReg->I2DR );
        ++pReadBuf;
    }

    return TRUE;

error:
    return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: OALI2cGenerateStop
//
//  Generates a stop bit on the I2C interface
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void OALI2cGenerateStop()
{
    UINT16  i2cr;
    UINT16 idx;
    WORD wDivider;
    int count;

    i2cr = INREG16(&g_pI2CReg->I2CR);
    if ((CSP_BITFEXT(i2cr, I2C_I2CR_MTX)==I2C_I2CR_MTX_RECEIVE)
        &&(CSP_BITFEXT(i2cr, I2C_I2CR_TXAK)==I2C_I2CR_TXAK_ACK_SEND))
    {
        INSREG16(&g_pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_TXAK),
                 CSP_BITFVAL(I2C_I2CR_TXAK, I2C_I2CR_TXAK_NO_ACK_SEND));

        // Delay for 1 SCL clock
        for(count=0; count<I2C_MAXDIVIDER; ++count)
        {
            if (EXTREG16(&g_pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IIF),
                    I2C_I2SR_IIF_LSH) == I2C_I2SR_IIF_PENDING)
            {
                OUTREG16(&g_pI2CReg->I2SR, 0);
                break;
            }
        }
    }

    idx = EXTREG16BF(&g_pI2CReg->IFDR, I2C_IFDR_IC);
    wDivider = (idx>=I2CDIVTABSIZE)? I2C_MAXDIVIDER :wI2CClockRateDivider[idx];
    // Delay for 2 SCL clock
    for(count=0; count<1; ++count)
    {
        INREG16(&g_pI2CReg->I2SR);
    }

    INSREG16(&g_pI2CReg->I2CR, CSP_BITFMASK(I2C_I2CR_MSTA),
             CSP_BITFVAL(I2C_I2CR_MSTA, I2C_I2CR_MSTA_SLAVE));

    // Delay for 1 SCL clock
    for(count=0; count<1; ++count)
    {
        INREG16(&g_pI2CReg->I2SR);
    }

    // Wait bus is free (IBB bit is clear) after clearing MSTA bit.
    // Wait for the stop condition to clear...
    count = 0;
    while (EXTREG16(&g_pI2CReg->I2SR, CSP_BITFMASK(I2C_I2SR_IBB),
                    I2C_I2SR_IBB_LSH) == I2C_I2SR_IBB_BUSY)
    {
        count++;
        if ( count >= 1000 ) 
        {
            OALMSG( OAL_ERROR, (
                    _T("%s: Bus not cleared for %d cycles\r\n"),
                    _T(__FUNCTION__),
                    count ));
            break;
        }
    }
}

