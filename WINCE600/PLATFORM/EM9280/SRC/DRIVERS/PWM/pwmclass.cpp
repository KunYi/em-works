#include "bsp.h"
#include "pwmclass.h"

#define	MIN_PWM_FREQ		10					//10Hz
#define	MAX_PWM_FREQ		12000000			//12MHz

//-----------------------------------------------------
// PWMClass member function implementation
//-----------------------------------------------------


//-----------------------------------------------------
// GPIOClass public member functions
//-----------------------------------------------------
PWMClass::PWMClass(DWORD dwIndex)
{
    PHYSICAL_ADDRESS	phyAddr;
	UINT32				u32Div;

	m_dwIndex = dwIndex;
	if((m_dwIndex >= 1) && (m_dwIndex <= 4))
	{
        phyAddr.QuadPart = CSP_BASE_REG_PA_PWM;

        // Map peripheral physical address to virtual address
        pv_HWRegPWM = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);

        // Check if virtual mapping failed
        if (pv_HWRegPWM == NULL)
        {
            ERRORMSG(1, (TEXT("PWMClass: MmMapIoSpace failed!\r\n")));
            return;
        }

		switch(m_dwIndex)
		{
		case 1:			// PWM_5
			m_dwPwmIndex = 5;							
			m_dwPwmEnable = BM_PWM_CTRL_PWM5_ENABLE;
			break;

		case 2:			// PWM_6
			m_dwPwmIndex = 6;							
			m_dwPwmEnable = BM_PWM_CTRL_PWM6_ENABLE;
			break;

		case 3:			// PWM_3
			m_dwPwmIndex = 3;							
			m_dwPwmEnable = BM_PWM_CTRL_PWM3_ENABLE;
			break;

		case 4:			// PWM_4
			m_dwPwmIndex = 4;	
			m_dwPwmEnable = BM_PWM_CTRL_PWM4_ENABLE;
			break;
		}
	}
	else
	{
		pv_HWRegPWM = NULL;
        ERRORMSG(1, (TEXT("PWMClass: unsupport PWM%d!\r\n"), m_dwIndex));
        return;
	}

	m_dwFreq = 0;
	m_dwDuty = 0;
	m_dwResolution = 1;

	u32Div = 9;		// = 9, 18, 36, 72
	//set HSADC_CLK = ref_HSADC / u32Div => 480MHz / 9 = 53.333333MHz
	if(!DDKClockConfigBaud(DDK_CLOCK_SIGNAL_HSADC, DDK_CLOCK_BAUD_SOURCE_REF_HSADC, u32Div))
	{
        ERRORMSG(1, (TEXT("PWMClass: config HSADC clock failed\r\n")));
        return;
	}

	if(DDKClockGetGatingMode(DDK_CLOCK_GATE_HSADC_CLK))
	{
		//RETAILMSG(1, (_T("PWMClass: REF_HSADC enabled!\r\n")));            
		DDKClockSetGatingMode(DDK_CLOCK_GATE_HSADC_CLK, FALSE);
	}
	else
	{
		//RETAILMSG(1, (_T("PWMClass: REF_HSADC enabled already\r\n")));            
	}

	m_dwHsadcClock = 53340366;		// = 53.333333MHz calibrated to 53340366Hz
	m_dwXtalClock  = 24000000;		// = 24.000000MHz

	m_dwCDIV_Index  = 0;
	m_dwCDIV_TAB[0] = 1;
	m_dwCDIV_TAB[1] = 2;
	m_dwCDIV_TAB[2] = 4;
	m_dwCDIV_TAB[3] = 8;
	m_dwCDIV_TAB[4] = 16;
	m_dwCDIV_TAB[5] = 64;
	m_dwCDIV_TAB[6] = 256;
	m_dwCDIV_TAB[7] = 1024;
}


PWMClass::~PWMClass(void)
{
	if(pv_HWRegPWM != NULL)
	{
        MmUnmapIoSpace(pv_HWRegPWM, 0x1000);
		pv_HWRegPWM = NULL;
	}
}

//
// PWM functions
//
BOOL PWMClass::PinConfig()
{
	BOOL bRet = TRUE;

	switch(m_dwIndex)
	{
	case 1:	// -> iMX283.PWM_5 - GPIO3_22
		DDKIomuxSetPinMux(DDK_IOMUX_PWM5, DDK_IOMUX_MODE_01);
		DDKIomuxSetPadConfig(DDK_IOMUX_PWM5, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		break;

	case 2:	// -> iMX283.PWM_6 - GPIO3_23
		DDKIomuxSetPinMux(DDK_IOMUX_PWM6, DDK_IOMUX_MODE_01);
		DDKIomuxSetPadConfig(DDK_IOMUX_PWM6, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		break;

	case 3:	// -> iMX283.PWM_3 - GPIO3_28
		DDKIomuxSetPinMux(DDK_IOMUX_PWM3_1, DDK_IOMUX_MODE_00);
		DDKIomuxSetPadConfig(DDK_IOMUX_PWM3_1, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		break;

	case 4:	// -> iMX283.PWM_4 - GPIO3_29
		DDKIomuxSetPinMux(DDK_IOMUX_PWM4_1, DDK_IOMUX_MODE_00);
		DDKIomuxSetPadConfig(DDK_IOMUX_PWM4_1, DDK_IOMUX_PAD_DRIVE_8MA, DDK_IOMUX_PAD_PULL_ENABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
		break;

	default:
		bRet = FALSE;
		ERRORMSG(1, (TEXT("PWMClass::PinConfig: unsupport PWM%d!\r\n"), m_dwIndex));
	}

	return bRet;
}

BOOL PWMClass::OutputConfig(DWORD dwFreq, DWORD dwDuty, DWORD dwResolution)
{
	BOOL	bRet = TRUE;
	DWORD	dwDiv24MHz, dwDiv53MHz;
	DWORD	dwActualFreq24MHz, dwActualFreq53MHz;
	DWORD	dwError24MHz, dwError53MHz;
	BOOL	bUseXtalClock;
	DWORD	dwDiv;
	DWORD	dwInactiveCount;
	DWORD	dwTmp;
	__int64	i64Tmp;

	// stop PWM output if required
	if(!dwFreq)
	{
		HW_PWM_CTRL_CLR(m_dwPwmEnable);
		goto exit;
	}

	// check input parameters
	switch(dwResolution)
	{
	case 1:
	case 10:
	case 100:
		// good resolution
		break;
	default:
		ERRORMSG(1, (TEXT("PWMClass::OutputConfig: invalid resolution = %d\r\n"), dwResolution));
		bRet = FALSE;
		goto exit;
	}

	dwTmp = dwFreq / dwResolution;
	if((dwTmp < MIN_PWM_FREQ) || (dwTmp > MAX_PWM_FREQ))
	{
		ERRORMSG(1, (TEXT("PWMClass::OutputConfig: freq %dHz out of range \r\n"), dwTmp));
		bRet = FALSE;
		goto exit;
	}

	// save resolution
	m_dwResolution = dwResolution;

	// compute divider
	i64Tmp = m_dwXtalClock * dwResolution;
	dwDiv24MHz = (DWORD)(i64Tmp / dwFreq);
	dwTmp = (DWORD)(i64Tmp * 10 / dwFreq);
	if((dwTmp % 10) >= 5)
	{
		dwDiv24MHz++;
	}

	i64Tmp = m_dwHsadcClock * dwResolution;
	dwDiv53MHz = (DWORD)(i64Tmp / dwFreq);
	dwTmp = (DWORD)(i64Tmp * 10 / dwFreq);
	if((dwTmp % 10) >= 5)
	{
		dwDiv53MHz++;
	}

	// compute actual clock freq with the same resolution
	i64Tmp = m_dwXtalClock * dwResolution;
	dwActualFreq24MHz = (DWORD)(i64Tmp / dwDiv24MHz);
	i64Tmp = m_dwHsadcClock * dwResolution;
	dwActualFreq53MHz = (DWORD)(i64Tmp / dwDiv53MHz);
	//RETAILMSG(1, (TEXT("PWMClass::OutputConfig: ActualFreq24MHz =(%d/%d)Hz, Div=%d\r\n"), dwActualFreq24MHz, m_dwResolution, dwDiv24MHz));
	//RETAILMSG(1, (TEXT("PWMClass::OutputConfig: ActualFreq53MHz =(%d/%d)Hz, Div=%d\r\n"), dwActualFreq53MHz, m_dwResolution, dwDiv53MHz));

	// compute error
	if(dwActualFreq24MHz >= dwFreq)
	{
		dwError24MHz = dwActualFreq24MHz - dwFreq;
	}
	else
	{
		dwError24MHz = dwFreq - dwActualFreq24MHz;
	}

	if(dwActualFreq53MHz >= dwFreq)
	{
		dwError53MHz = dwActualFreq53MHz - dwFreq;
	}
	else
	{
		dwError53MHz = dwFreq - dwActualFreq53MHz;
	}

	// select source clock
	if(dwError24MHz <= dwError53MHz)
	{
		// use XTAL = 24.000000MHz as PWM's clock source
		m_dwFreq = dwActualFreq24MHz;
		bUseXtalClock = TRUE;
		dwDiv = dwDiv24MHz;
		RETAILMSG(1, (TEXT("PWMClass::OutputConfig: select XTAL24M Clock -> freq=(%d/%d)Hz, Div=%d\r\n"), m_dwFreq, m_dwResolution, dwDiv));
	}
	else
	{
		// use HSADC Clock = 53.333333MHz as PWM's clock source
		m_dwFreq = dwActualFreq53MHz;
		bUseXtalClock = FALSE;
		dwDiv = dwDiv53MHz;
		RETAILMSG(1, (TEXT("PWMClass::OutputConfig: select HSADC Clock -> freq=(%d/%d)Hz, Div=%d\r\n"), m_dwFreq, m_dwResolution, dwDiv));
	}

	// config pre-scaler of PWM divider
	for(m_dwCDIV_Index = 0; (m_dwCDIV_Index < 8) && (dwDiv > 0x10000); m_dwCDIV_Index++)
	{
		if((dwDiv / m_dwCDIV_TAB[m_dwCDIV_Index]) <= 0x10000)
		{
			break;
		}
	}
	dwDiv = dwDiv / m_dwCDIV_TAB[m_dwCDIV_Index];

	// compute actual duty 
	dwInactiveCount = (dwDiv * dwDuty) / 100;
	m_dwDuty = (dwInactiveCount * 100) / dwDiv;

	// ACTIVE segment start from 0, INACTIVE segment start from dwInactiveCount
	dwTmp = BF_PWM_ACTIVEn_INACTIVE(dwInactiveCount) | BF_PWM_ACTIVEn_ACTIVE(0);
	HW_PWM_ACTIVEn_WR(m_dwPwmIndex, dwTmp);

	if(bUseXtalClock)
	{
		dwTmp = BF_PWM_PERIODn_MATT_SEL(1)											// select 24MHz main clock as source clock
			  | BF_PWM_PERIODn_CDIV(m_dwCDIV_Index)									// clock = mainclock / m_dwCDIV_TAB[m_dwCDIV_Index]
			  | BF_PWM_PERIODn_INACTIVE_STATE(BV_PWM_PERIODn_INACTIVE_STATE__0)		// INACTIVE_STATE = 0 
			  | BF_PWM_PERIODn_ACTIVE_STATE(BV_PWM_PERIODn_ACTIVE_STATE__1)			// ACTIVE_STATE = 1
			  | BF_PWM_PERIODn_PERIOD((dwDiv - 1));									// number of divided clock = dwDiv
	}
	else
	{
		dwTmp = BF_PWM_PERIODn_HSADC_CLK_SEL(1)										// select HSADC input clock as source clock
			  | BF_PWM_PERIODn_CDIV(m_dwCDIV_Index)									// clock = mainclock / m_dwCDIV_TAB[m_dwCDIV_Index]
			  | BF_PWM_PERIODn_INACTIVE_STATE(BV_PWM_PERIODn_INACTIVE_STATE__0)		// INACTIVE_STATE = 0 
			  | BF_PWM_PERIODn_ACTIVE_STATE(BV_PWM_PERIODn_ACTIVE_STATE__1)			// ACTIVE_STATE = 1
			  | BF_PWM_PERIODn_PERIOD((dwDiv - 1));									// number of divided clock = dwDiv
	}
	HW_PWM_PERIODn_WR(m_dwPwmIndex, dwTmp);

	// Enable current PWM channel
	HW_PWM_CTRL_SET(m_dwPwmEnable);

exit:
	return bRet;
}

