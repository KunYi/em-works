//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bspesdhc.cpp
//
//  Provides BSP-specific routines for use by ESDHC driver.
//
//------------------------------------------------------------------------------

#include "bsp.h"
#include "esdhc.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define SHC_CONTROLLER_INDEX_KEY    TEXT("Index")
#define SHC_DISABLEDMA_KEY              TEXT("DisableDMA")
#define SHC_SDIO_PRIORITY_KEY           TEXT("SDIOPriority")
#define SHC_FREQUENCY_KEY               TEXT("MaximumClockFrequency")
#define SHC_RW_TIMEOUT_KEY              TEXT("ReadWriteTimeout")
#define SHC_WAKEUP_SOURCE_KEY          TEXT("WakeupSources")

#define ESDHC1_WP_GPIO_PORT      DDK_GPIO_PORT2
#define ESDHC1_WP_GPIO_PIN       0
#define ESDHC1_CD_GPIO_PORT      DDK_GPIO_PORT2
#define ESDHC1_CD_GPIO_PIN       1

#define ESDHC_DEBOUNCE_PERIOD  100    // 100 ms
#define ESDHC_DEBOUNCE_CHECKS  2
#define ESDHC_DEBOUNCE_TIMEOUT  1000 // 1000 ms

#define ESDHC_CARD_DETECT_EVENT_NAME1   TEXT("EventESDHC1")

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

static HANDLE hCardDetectEvent1;
static HANDLE hCardDetectIST1;
static BOOL bExitCardDetectIST1= FALSE;
static BOOL bSlotChange = FALSE;
static BOOL bCardDetectInitialed = FALSE;
//static HANDLE hCardDetectEvent2;
//static HANDLE hCardDetectIST2;
static BOOL bHost2CardDetectInitialed = FALSE;
static BOOL bHost2CardDetectInsert = FALSE;
//------------------------------------------------------------------------------
// Local Functions
static DWORD WINAPI ISTCardDetectHandler(LPVOID lpParam); 


// Read the registry settings
BOOL CESDHCBase::BspGetRegistrySettings( CReg *pReg )
{
    BOOL fRet = TRUE;

    DEBUGCHK(pReg);

    // get the controller index (instance of ESDHC in the SOC: 1,2, or 3)
    m_dwControllerIndex = pReg->ValueDW(SHC_CONTROLLER_INDEX_KEY);
    if (m_dwControllerIndex == 0)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CESDHCBase::GetRegistrySettings: No controller index found. Can't load ESDHC driver.\r\n"));
        fRet = FALSE;
        goto EXIT;
    }

    m_fDisableDMA = pReg->ValueDW(SHC_DISABLEDMA_KEY, 0);
    
    // get the controller IST thread priority
    m_dwSDIOPriority = pReg->ValueDW(SHC_SDIO_PRIORITY_KEY, SHC_CARD_CONTROLLER_PRIORITY);

    // get the max clock frequency from the registry (we allow the registry to override)
    m_dwMaxClockRate = pReg->ValueDW(SHC_FREQUENCY_KEY);
    if (m_dwMaxClockRate == 0) 
    {
        // PERCLK needs to be at either 50 MHz or multiple of 50 for high speed (50 Mhz) SD clock
        m_dwMaxClockRate = min(BspESDHCGetBaseClk(), ESDHC_MAX_CLOCK_RATE); 
    }

    // get the read/write timeout value
    m_dwMaxTimeout = pReg->ValueDW(SHC_RW_TIMEOUT_KEY, DEFAULT_TIMEOUT_VALUE);

    // get the wakeup sources
    m_fWakeupSource = pReg->ValueDW(SHC_WAKEUP_SOURCE_KEY, 0);


EXIT:
    return fRet;
}


// Get the base clock of the ESDHC module
ULONG CESDHCBase::BspESDHCGetBaseClk()
{
    UINT32 ulBaseFreq = 0;
    DDK_CLOCK_SIGNAL index = DDK_CLOCK_SIGNAL_ENUM_END;
    
    switch (m_dwControllerIndex)
    {
        case 1:
            index = DDK_CLOCK_SIGNAL_PER_ESDHC1;
            break;
                
        case 2:
            index = DDK_CLOCK_SIGNAL_PER_ESDHC2;
            break;

        default:
            goto EXIT;
            break;

    }

    DDKClockGetFreq(index, &ulBaseFreq);

EXIT:    
    return ulBaseFreq;

}

// Initialize the IOMux signals for particular ESDHC module
BOOL CESDHCBase::BspESDHCInit()
{
    BOOL fRet = TRUE;
    
	RETAILMSG(1, (L"BspESDHCInit::config SD%d pins.\r\n", m_dwControllerIndex));
    switch (m_dwControllerIndex)
    {
    case 1:
        // SD1_CMD setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_SD1_CMD, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_FORCE);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_SD1_CMD, (DDK_IOMUX_PAD_SLEW) 0, (DDK_IOMUX_PAD_DRIVE) 0, 
                                            (DDK_IOMUX_PAD_OPENDRAIN) 0, DDK_IOMUX_PAD_PULL_UP_22K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            (DDK_IOMUX_PAD_VOLTAGE) 0);

        // SD1_CLK setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_SD1_CLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_SD1_CLK, (DDK_IOMUX_PAD_SLEW) 0, (DDK_IOMUX_PAD_DRIVE) 0, 
                                            (DDK_IOMUX_PAD_OPENDRAIN) 0, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_DISABLE,
                                            (DDK_IOMUX_PAD_VOLTAGE) 0);

        // SD1_DATA0 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_SD1_DATA0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_SD1_DATA0, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);


        // SD1_DATA1 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_SD1_DATA1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_SD1_DATA1, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);

        // SD1_DATA2 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_SD1_DATA2, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_SD1_DATA2, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);

        // SD1_DATA3 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_SD1_DATA3, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_SD1_DATA3, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_22K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        
        
        // SD Card Write protection (A14)
        // - GPIO2[0]
        // - Function ALT 5 on A14 pio
        // - PIO already pull-up
        // - PIO on VDD_SD1_IO (3v3) (PGAL: We can find a remarks on the schematics)
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_A14, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A14, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL,
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_1V8);

    
        // Now configure PIO in INPUT mode
        DDKGpioSetConfig(ESDHC1_WP_GPIO_PORT, ESDHC1_WP_GPIO_PIN, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_NONE);

        // SD Card Detection (A15)
        //Note: Not using the DAT[3] for card detection
        // Card Detection is done through the GPIO2_1
        
        // - GPIO2[1]
        // - Function ALT 5 on A15 pio
        // - PIO already pull-up
        // - PIO on VDD_SD1_IO (3v3)
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_A15, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_A15, DDK_IOMUX_PAD_SLEW_FAST, (DDK_IOMUX_PAD_DRIVE) 0,
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_NONE, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_1V8);
        // Now configure PIO in INPUT mode with interrupt on low level
        DDKGpioSetConfig(ESDHC1_CD_GPIO_PORT, ESDHC1_CD_GPIO_PIN, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_BOTH_EDGE);

        // Need to set this D3CD bit so that ESDHC will "know" that card is plugged in, and therefore it allows CINT to be enabled
        INSREG32BF(&m_pESDHCReg->PROCTL, ESDHC_PROCTL_D3CD, 1);
        
        break;
        
        // ESDHC is connected with CSR Wifi on 3DS board
    case 2:
        // SD2_CMD setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_MDC, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_FORCE);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_MDC, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_ESDHC2_IPP_CMD_IN, 0x2);
                         
        // SD2_CLK setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_MDIO, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_MDIO, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_22K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_ESDHC2_IPP_CARD_CLK_IN, 0x2);

        // SD2_DATA0 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TDATA0, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_TDATA0, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT0_IN, 0x2);
        
        // SD2_DATA1 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TDATA1, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_TDATA1, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT1_IN, 0x2);
        

        // SD2_DATA2 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_TX_EN, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_TX_EN, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT2_IN, 0x2);
        

        // SD2_DATA3 setup
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_FEC_RDATA0, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_FEC_RDATA0, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKIomuxSelectInput(DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT3_IN, 0x2);


        // Reset WiFi
        DDKIomuxSetPinMux(DDK_IOMUX_PIN_D10, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
        DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D10, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_HIGH, 
                                            DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE,
                                            DDK_IOMUX_PAD_VOLTAGE_3V3);
        DDKGpioSetConfig(DDK_GPIO_PORT4, 10, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 10, 0);
        Sleep(100);
        DDKGpioWriteDataPin(DDK_GPIO_PORT4, 10, 1); 
        
        INSREG32BF(&m_pESDHCReg->PROCTL, ESDHC_PROCTL_D3CD, 1);
    
        break;

        
    case 3:
    default:
        fRet = FALSE;
        goto EXIT;
        break;

    }


EXIT:

    return fRet;
}


// Do de-init at BSP level if needed
VOID CESDHCBase::BspESDHCDeinit()
{
    if(hCardDetectIST1){
        bExitCardDetectIST1 = TRUE;
        SetEvent(hCardDetectEvent1);
        CloseHandle(hCardDetectEvent1);
        hCardDetectEvent1= NULL;
        hCardDetectIST1 = NULL;
    }
    return;
}

// Multiple IRQs (upto 4) can be mapped to same SYSINTR (for eg, besides the SD Controller IRQ, can also add GPIO IRQ)
BOOL CESDHCBase::BspESDHCSysIntrSetup()
{
    BOOL fRet = FALSE;
    DWORD dwIrqs[4];
    DWORD dwIrqsSize = sizeof(dwIrqs);

    switch(m_dwControllerIndex)
    {
        case 1:
            // Using -1 indicates we are not using the legacy calling convention
            dwIrqs[0] = (DWORD) -1;
            // Flags: in this case we want the existing sysintr if it
            // has already been allocated, and a new sysintr otherwise.
            dwIrqs[1] = OAL_INTR_TRANSLATE;
            // Now, let's add the controller IRQ
            dwIrqs[2] = IRQ_ESDHC1;
            // Also map the card detect GPIO 2_1 IRQ to the array
            dwIrqs[3] = IRQ_GPIO2_PIN1;
            // SDMA IRQs added to this array as well

            break;

        case 2:
            // Using -1 indicates we are not using the legacy calling convention
            dwIrqs[0] = (DWORD) -1;
            // Flags: in this case we want the existing sysintr if it
            // has already been allocated, and a new sysintr otherwise.
            dwIrqs[1] = OAL_INTR_TRANSLATE;
            // Now, let's add the controller IRQ
            dwIrqs[2] = IRQ_ESDHC2;
            // We only use three elements
            dwIrqsSize -= sizeof(DWORD);

            break;
            
        default:
            goto EXIT;
            break;        

    }

    // convert the hardware IRQs for ESDHC (including Card Detect GPIO) into a logical SYSINTR value
    if (KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, dwIrqs, dwIrqsSize, &m_dwControllerSysIntr, sizeof(DWORD), NULL))
    {
        fRet = TRUE;
    }

    // enable wakeup on sd card insertion/removal: since sysintr is wakeup source, all associated IRQs can enable wakeup, but
    // other IRQs (such as IRQ_ESDHC) will not trigger when system is suspended, so only GPIO interrupt (for insertion/removal) will wakeup system
    if (fRet && m_fWakeupSource)
    {
        KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &m_dwControllerSysIntr, sizeof(m_dwControllerSysIntr), NULL, 0, NULL);
    }


EXIT:
    return fRet;

}

VOID CESDHCBase::BspESDHCSetClockGating(DWORD dwPowerState)
{
    DDK_CLOCK_GATE_INDEX index1, index2, index3;

    
    
    switch (m_dwControllerIndex)
    {
        case 1:
            index3 =  DDK_CLOCK_GATE_INDEX_AHB_ESDHC1;
            index2 =  DDK_CLOCK_GATE_INDEX_ESDHC1;   
            index1 =  DDK_CLOCK_GATE_INDEX_PER_ESDHC1;
            break;
                
        case 2:
            index3 =  DDK_CLOCK_GATE_INDEX_AHB_ESDHC2;
            index2 =  DDK_CLOCK_GATE_INDEX_ESDHC2;  
            index1 =  DDK_CLOCK_GATE_INDEX_PER_ESDHC2;
            break;

        default:
            goto EXIT;
            break;
    }

    switch(dwPowerState)
    {
        case D0:
            DDKClockSetGatingMode(index1, DDK_CLOCK_GATE_MODE_ENABLED);
            DDKClockSetGatingMode(index2, DDK_CLOCK_GATE_MODE_ENABLED);
            DDKClockSetGatingMode(index3, DDK_CLOCK_GATE_MODE_ENABLED);
            ClockGateOff();

            break;

        case D1:
        case D2:
        case D3:
        case D4:
                     
            // DDKClockSetGatingMode(index2, DDK_CLOCK_GATE_MODE_DISABLED);
            // DDKClockSetGatingMode(index1, DDK_CLOCK_GATE_MODE_DISABLED);
            if ( !m_fCardIsSDIO ||  !m_fCardPresent)
            {
                // turn on auto-clock gating mode
                ClockGateOn();
                DDKClockSetGatingMode(index3, DDK_CLOCK_GATE_MODE_DISABLED);   
            }

            break;

        default:
            break;

    }
        

EXIT:
    return;
}

VOID CESDHCBase::BspESDHCSetSlotVoltage(DWORD dwVoltage)
{
    switch (m_dwControllerIndex)
    {
       case 1:
         break;
       case 2:
         if(dwVoltage >= SD_VDD_WINDOW_3_0_TO_3_1)
         {
            DDKGpioSetConfig(DDK_GPIO_PORT4, 10, DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
            DDKGpioWriteDataPin(DDK_GPIO_PORT4, 10, 0);
            Sleep(100);
            DDKGpioWriteDataPin(DDK_GPIO_PORT4, 10, 1); 
            bHost2CardDetectInsert = TRUE;
         }
         break;
      default:
         break;    
    }

}

BOOL CESDHCBase::BspESDHCIsCardPresent()
{
    DDK_GPIO_PORT CardDetectPort;
    DWORD dwCardDetectPin;
    UINT32 dwPinVal1 = 1, dwPinVal2 = 1, dwNumchecks = 0;
    BOOL fCardPresent = FALSE;
    DWORD dwTimeElapsed = 0;


    switch(m_dwControllerIndex)
    {
        case 1:
            CardDetectPort = ESDHC1_CD_GPIO_PORT;
            dwCardDetectPin = ESDHC1_CD_GPIO_PIN;
            break;

        case 2:
            //esdhc2 is connect to wifi chip on board, so it's always present
            if(bHost2CardDetectInsert == TRUE)
                RETAILMSG(0,(TEXT("CSR BspESDHCIsCardPresent\r\n")));
            else
                RETAILMSG(0,(TEXT("CSR Not BspESDHCIsCardPresent\r\n")));
            return bHost2CardDetectInsert;

            break;
        default:
            goto EXIT;
            break;
    }

    DDKGpioReadDataPin(CardDetectPort, dwCardDetectPin, &dwPinVal1);

    // debounce: timeout after 1s, or if same value is read twice (min of 2 debounce periods)
    for (dwNumchecks = 0; dwNumchecks < ESDHC_DEBOUNCE_CHECKS && dwTimeElapsed < ESDHC_DEBOUNCE_TIMEOUT; dwTimeElapsed += ESDHC_DEBOUNCE_PERIOD)
    {
        Sleep(ESDHC_DEBOUNCE_PERIOD);
        DDKGpioReadDataPin(CardDetectPort, dwCardDetectPin, &dwPinVal2);

        if (dwPinVal1 == dwPinVal2)
            dwNumchecks++;

        // if value changed in between reads, reset the count
        else
        {
            dwPinVal1 = dwPinVal2;
            dwNumchecks = 0;
        }
    }
    
#ifdef		EM9170
    // card is present when pin is high
    if (dwPinVal2)
#else		// ->iMX25PDK
    // card is present when pin is low
    if (!dwPinVal2)
#endif		//EM9170
    {
        fCardPresent = TRUE;
        RETAILMSG(1, (TEXT("SD% card is present\r\n"), m_dwControllerIndex)); 
    }

    // Then, clear interrupt status bit for GPIO 2_1
    DDKGpioClearIntrPin(CardDetectPort, dwCardDetectPin);


EXIT:
    return fCardPresent;

}


BOOL CESDHCBase::BspESDHCSlotStatusChanged()
{
    BOOL fSlotStatusChanged = FALSE;
    DDK_GPIO_PORT CardDetectPort;
    UINT32 dwCardDetectPin, dwCDIntrStatus;
    static bFirstChangeDetection = TRUE;

    switch(m_dwControllerIndex)
    {
        case 1:
            CardDetectPort = ESDHC1_CD_GPIO_PORT;
            dwCardDetectPin = ESDHC1_CD_GPIO_PIN;
            break;

        case 2:
            if(bHost2CardDetectInitialed == FALSE){
                if(bHost2CardDetectInsert == TRUE){
                    bHost2CardDetectInitialed = TRUE;
                    RETAILMSG(0,(TEXT("Host 2 Stauts change true\r\n"))); 
                    return TRUE;
                }
                else{
                    RETAILMSG(0,(TEXT("Host 2 Stauts change false\r\n"))); 
                    return FALSE;
                }
            }else{  
                RETAILMSG(0,(TEXT("Host 2 Stauts change false\r\n"))); 
                return FALSE;
            }
            break;
        default:
            goto EXIT;
            break;
    }

    // The first slot change status must return true to allow card detection (use if card inserted at system startup)
    if (bFirstChangeDetection == TRUE)
    {
        bFirstChangeDetection = FALSE;
        fSlotStatusChanged = TRUE;
        goto EXIT;
    }

    // if ISR of the GPIO pin is set, then some change occured (because interrupt is configured as both edge triggered)
    DDKGpioReadIntrPin(CardDetectPort, dwCardDetectPin, &dwCDIntrStatus);

    if (dwCDIntrStatus)
    {
        fSlotStatusChanged = TRUE;
        RETAILMSG(1, (TEXT("SD% some change occured\r\n"), m_dwControllerIndex)); 
    }

EXIT:
    return fSlotStatusChanged;
}


VOID CESDHCBase::BspESDHCCardDetectInt(BOOL fDetectInsertion)
{
    DDK_GPIO_PORT CardDetectPort;
    UINT32 dwCardDetectPin;
    
    switch(m_dwControllerIndex)
    {
        case 1:
            CardDetectPort = ESDHC1_CD_GPIO_PORT;
            dwCardDetectPin = ESDHC1_CD_GPIO_PIN;
            break;

        // on iMX25-3DS, ESDHC2 is not connected to any slot
        case 2:
        default:
            goto EXIT;
            break;

    }
    
    // First, clear interrupt status bit for GPIO 2_1
    DDKGpioClearIntrPin(CardDetectPort, dwCardDetectPin);
    
    if (fDetectInsertion == TRUE)
    {
        DDKGpioSetConfig(CardDetectPort, dwCardDetectPin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_BOTH_EDGE);
    }
    else
    {
        DDKGpioSetConfig(CardDetectPort, dwCardDetectPin, DDK_GPIO_DIR_IN, DDK_GPIO_INTR_BOTH_EDGE);
    }

    // First, clear interrupt status bit for GPIO 2_1
    DDKGpioClearIntrPin(CardDetectPort, dwCardDetectPin);

    // Signal interrupt to force a card insertion check, because if the card is already inserted no interrupt is triggered.
    SetInterruptEvent(m_dwControllerSysIntr);

EXIT:
    return;
}

BOOL CESDHCBase::BspESDHCIsWriteProtected()
{
#ifdef	EM9170
	//
	// CS&ZHL AUG-10-2011: WP pin is NOT used in EM9170
	//
	return FALSE;
#else	//iMX25PDK
    BOOL fWPStatus = FALSE;
    DDK_GPIO_PORT WPPort;
    UINT32 dwWPPin, dwWPStatus;

    switch(m_dwControllerIndex)
    {
        case 1:
            WPPort = ESDHC1_WP_GPIO_PORT;
            dwWPPin = ESDHC1_WP_GPIO_PIN;
            break;

        // on iMX25-3DS, ESDHC2 is not connected to any slot
        case 2:
            return FALSE;
            break;
        default:
            goto EXIT;
            break;
    }

    // if GPIO pin is 1, then card is write protected
    DDKGpioReadDataPin(WPPort, dwWPPin, &dwWPStatus);

    if (dwWPStatus)
        fWPStatus = TRUE;

EXIT:
    return fWPStatus;
#endif	//EM9170
}

