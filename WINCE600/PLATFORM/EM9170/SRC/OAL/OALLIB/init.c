//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File: init.c
//
// MX25 3DS board initialization code.
//
//-----------------------------------------------------------------------------

#include <bsp.h>
#include <kitl_cfg.h>

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// External Functions

extern VOID InitDebugSerial(void);

extern UINT32 OALTimerGetClkFreq(void);
extern UINT32 OALTimerGetClkPrescalar(void);
extern UINT32 OALTimerGetClkSrc();

extern BOOL OALPmicInit(void);
extern BOOL RNGB_Init(void);

extern BOOL OALInitRTC();
extern void DRYICE_WRITE(PCSP_DRYICE_REGS pDry,REG32* pAddr,DWORD dwValue);
extern DWORD DRYICE_READ(PCSP_DRYICE_REGS pDry,REG32* pAddr);

extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);
extern VOID OALInitializeClockGating(VOID);

//-----------------------------------------------------------------------------
// External Variables
extern UUID g_UUID;

extern PCSP_CRM_REGS g_pCRM;
extern PCSP_IOMUX_REGS g_pIOMUX;


extern PCSP_SDMA_REGS g_pSDMA;

UINT32 g_SREV;
UINT8 g_UPID[8];
UINT8 *g_pIIM;
DWORD g_dwResetCause = UNKNOWN_RESET;

// Define a static data structure that we can use to pass data between
// the OAL and KITL. We will assign the address of this data structure
// to the "pKitlInfo" data member in OEMGlobals.
//
// Note that we make a "static" declaration here so that all external
// accesses to this data structure will be forced to use the pKitlInfo
// pointer for consistency.
//
static _OALKITLSharedDataStruct g_OALKITLSharedData = { 0 };

//
//  Global:  dwOEMDrWatsonSize
//
//  Global variable which specify DrWatson buffer size. It can be fixed
//  in config.bib via FIXUPVAR.
//
#define DR_WATSON_SIZE_NOT_FIXEDUP (-1)
DWORD dwOEMDrWatsonSize = (DWORD) DR_WATSON_SIZE_NOT_FIXEDUP;



//-----------------------------------------------------------------------------
//
// Function: OEMHaltSystem
//
// Function is called before system halt.
//
// Parameters:
//
// Returns:
//
//
//-----------------------------------------------------------------------------
void OEMHaltSystem(void)
{
    
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInit
//
//  This is Windows CE OAL initialization function. It is called from kernel
//  after basic initialization is made.
//
void OEMInit()
{
    BSP_ARGS				*pBspArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_UA_START;
    UINT32						countsPerMsec;
    DWORD						dwREST;
	PEM9K_CPLD_REGS	pCPLD;
	UCHAR						VID[5];		

    // Set up the debug zones according to the fix-up variable
    // initialOALLogZones.
    OALLogSetZones(initialOALLogZones);

    //OALMSG(OAL_FUNC, (L"+OEMInit\r\n"));
    OALMSG(1, (L"+OEMInit\r\n"));		// CS&ZHL JUN-2-2011: debug

    //Initialize all global variables upfront here.
    if (g_pCRM == NULL)
    {
		g_pCRM = (PCSP_CRM_REGS) OALPAtoUA(CSP_BASE_REG_PA_CRM);
		if (g_pCRM == NULL)
		{
			OALMSG(1, (L"OEMInit:  g_pCRM null pointer, spin forever!\r\n"));
			for(;;); // never ending loop. We shouldn't go further if this failed
		}
    }

    // Configure CRM clock gating.
    OALInitializeClockGating();
    
    // Un-gate the required AHB and PER Clk
    // JJH : does it really impact power consumption to let it run even if nobody needs it ?
    //Enable some AHB clock
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_SDMA ,DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_FEC  ,DDK_CLOCK_GATE_MODE_ENABLED);
    //Enable some PER clock
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_UART ,DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_EPIT ,DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_GPT  ,DDK_CLOCK_GATE_MODE_ENABLED);

	//
    // CS&ZHL MAY-7-2011: Enable LCDC clock
	//
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);

    // Un-gate (enable) only the IPG Clcks required by the basic OAL.
    // The rest of the IPG clock (like FEC, UART2, etc.) will be enabled later by the 
    // drivers and the other optionnal OAL module (KITL, Profiler, etc.)

#if (DEBUG_PORT == DBG_UART1)
    // we need UART clock       
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_UART1,DDK_CLOCK_GATE_MODE_ENABLED);
#endif
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_EPIT1, DDK_CLOCK_GATE_MODE_ENABLED);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_SDMA,  DDK_CLOCK_GATE_MODE_ENABLED);
#ifndef		EM9170			// for iMX257PDK only
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_CSPI1, DDK_CLOCK_GATE_MODE_ENABLED); //used to communicate with the CPLD.
#endif		//EM9170
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_RTC,   DDK_CLOCK_GATE_MODE_ENABLED);


    // Set memory size for DrWatson kernel support.
    dwNKDrWatsonSize = 0;
    if (dwOEMDrWatsonSize != DR_WATSON_SIZE_NOT_FIXEDUP)
    {
        dwNKDrWatsonSize = dwOEMDrWatsonSize;
    }

    // Define optional kernel supported features.
    pOEMIsProcessorFeaturePresent = OALIsProcessorFeaturePresent;

    // Initialize system halt handler.
    g_pOemGlobal->pfnHaltSystem = OEMHaltSystem;

    // Give kernel access to the profiling functions.
    g_pOemGlobal->pfnProfileTimerEnable  = OEMProfileTimerEnable;
    g_pOemGlobal->pfnProfileTimerDisable = OEMProfileTimerDisable;

    // Expose the processor type.
    CEProcessorType = PROCESSOR_ARM920;

    // Initilize cache globals
    OALCacheGlobalsInit();

    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"   Cache:\r\n"));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1Flags:       0x%08x\r\n",
                                       g_oalCacheInfo.L1Flags));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1ISetsPerWay: 0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1ISetsPerWay,
                                       g_oalCacheInfo.L1ISetsPerWay));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1INumWays:    0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1INumWays,
                                       g_oalCacheInfo.L1INumWays));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1ILineSize:   0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1ILineSize,
                                       g_oalCacheInfo.L1ILineSize));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1ISize:       0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1ISize,
                                       g_oalCacheInfo.L1ISize));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DSetsPerWay: 0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DSetsPerWay,
                                       g_oalCacheInfo.L1DSetsPerWay));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DNumWays:    0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DNumWays,
                                       g_oalCacheInfo.L1DNumWays));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DLineSize:   0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DLineSize,
                                       g_oalCacheInfo.L1DLineSize));
    OALMSGS(OAL_CACHE && OAL_VERBOSE, (L"       L1DSize:       0x%08x (%d)\r\n",
                                       g_oalCacheInfo.L1DSize,
                                       g_oalCacheInfo.L1DSize));

    // Initialize the shared OAL+KITL data structure. This allows KITL.DLL
    // to access these pointers to the hardware control registers.
    g_OALKITLSharedData.g_pCRM   = g_pCRM;
    g_OALKITLSharedData.g_pIOMUX = g_pIOMUX;
    // Provide a pointer that KITL can use to access the shared data structure.
    g_pOemGlobal->pKitlInfo = (LPVOID)&g_OALKITLSharedData;

#ifdef		IMX257PDK_CPLD
    //Initialize the CPLD communication
    CPLDInit();					// CS&ZHL JUN-2-2011: for iMX257PDK only
#endif		//IMX257PDK_CPLD

    // get the cause of the reset     
    dwREST = EXTREG32BF(&g_pCRM->RCSR,CRM_RCSR_REST);
    if (dwREST == CRM_RCSR_REST_POR)
    {
        OALMSGS(OAL_INFO || OAL_VERBOSE,(TEXT("Reset cause : Power On Reset\r\n")));
        g_dwResetCause = POR_RESET;
    } else if (dwREST == CRM_RCSR_REST_RIR)
    {
        OALMSGS(OAL_INFO || OAL_VERBOSE,(TEXT("Reset cause : Reset In Reset\r\n")));
        g_dwResetCause = RESET_IN_RESET;
    } else if ((dwREST & CRM_RCSR_REST_SOFT) == CRM_RCSR_REST_SOFT)
    {
        OALMSGS(OAL_INFO || OAL_VERBOSE,(TEXT("Reset cause : Soft Reset\r\n")));
        g_dwResetCause = SOFT_RESET;
    } else if ((dwREST & CRM_RCSR_REST_JTAG) == CRM_RCSR_REST_JTAG)
    {
        OALMSGS(OAL_INFO || OAL_VERBOSE,(TEXT("Reset cause : JTAG Reset\r\n")));
        g_dwResetCause = JTAG_RESET;
    } else if ((dwREST & CRM_RCSR_REST_WDOG) == CRM_RCSR_REST_WDOG)
    {
        OALMSGS(OAL_INFO || OAL_VERBOSE,(TEXT("Reset cause : Watchdog Reset\r\n")));
        g_dwResetCause = WDOG_RESET;
    } 
    else
    {
        OALMSGS(OAL_INFO || OAL_VERBOSE,(TEXT("Reset cause : Unknown reset cause\r\n")));
        g_dwResetCause = UNKNOWN_RESET;
    }

    // Map access to IIM
    g_pIIM = (UINT8 *) OALPAtoUA(CSP_BASE_REG_PA_IIM);
    if (g_pIIM == NULL)
    {
        // Error message is all we can do since OEMInit has no return
        OALMSG(OAL_ERROR, (L"OEMInit:  IIM null pointer, spin forever!\r\n"));
        for(;;); // never ending loop. We shouldn't go further if this failed
    }

    // Set silicon version
    //
    //      Define      Silicon Rev
    //      -----------------------
    //      0x01        TO1.0
    //      0x02        TO1.1
    //
#ifdef BSP_CPU_TO1
    g_SREV = 0x01;
#else
    g_SREV = 0x02;
#endif
    OALMSG(OAL_INFO, (L"OEMInit:  silicon rev = 0x%x\r\n", g_SREV));
    
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_IIM, DDK_CLOCK_GATE_MODE_ENABLED);
    // Read 64-bit unique part ID from Fuse Bank 0
    g_UPID[0] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x0020]) & 0xFF);
    g_UPID[1] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x0024]) & 0xFF);
    g_UPID[2] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x0028]) & 0xFF);
    g_UPID[3] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x002C]) & 0xFF);
    g_UPID[4] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x0030]) & 0xFF);
    g_UPID[5] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x0034]) & 0xFF);
    g_UPID[6] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x0038]) & 0xFF);
    g_UPID[7] = (UINT8) (INREG32(&g_pIIM[0x0800 + 0x003C]) & 0xFF);
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_IIM, DDK_CLOCK_GATE_MODE_DISABLED);
    

    // Copy UPID into the high 8 bytes of UUID which will be queried by
    // OAL_ARGS_QUERY_UUID in OALArgsQuery().
    memcpy(&g_UUID.Data4, &g_UPID, 8);
 


    // Tell filesys.exe that we want a clean boot.
    NKForceCleanBoot();

    // Initialize interrupts
    if (!OALIntrInit())
    {
        OALMSG(OAL_ERROR, 
            (L"ERROR: OEMInit: failed to initialize interrupts\r\n"));
        goto cleanUp;
    }

    // Initialize the system clock.
    countsPerMsec = OALTimerGetClkFreq() / ((OALTimerGetClkPrescalar() + 1) * 1000);
    if (!OALTimerInit(RESCHED_PERIOD, countsPerMsec, countsPerMsec / 100 + 2))
    {
        OALMSG(OAL_ERROR, 
            (L"ERROR: OEMInit: Failed to initialize system clock\r\n"));
        goto cleanUp;
    }

   // Initialize SDMA with address of shared region
    g_pSDMA = OALPAtoUA(CSP_BASE_REG_PA_SDMA);
    if (g_pSDMA == NULL)
    {
        // Error message is all we can do since OEMInit has no return
        OALMSG(OAL_ERROR, (L"ERROR: OEMInit:  SDMA null pointer!\r\n"));
        goto cleanUp;
    }
    else
    {
        // Set the channel 0 pointer to the shared region physical address
        OUTREG32(&g_pSDMA->MC0PTR, BSP_SDMA_MC0PTR);

        // Configure SDMA/AHB clock ratio
        if (pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_AHB] == 
            pBspArgs->clockFreq[DDK_CLOCK_SIGNAL_IPG])
        {
            INSREG32BF(&g_pSDMA->CONFIG, SDMA_CONFIG_ACR, SDMA_CONFIG_ACR_AHB1X);
        }
        else
        {
            INSREG32BF(&g_pSDMA->CONFIG, SDMA_CONFIG_ACR, SDMA_CONFIG_ACR_AHB2X);
        }

        // Configure SDMA for static context switch
        INSREG32BF(&g_pSDMA->CONFIG, SDMA_CONFIG_CSM, SDMA_CONFIG_CSM_STATIC);
    }
    
    // Initialize the PMIC interface
    OALPmicInit();		

    OALInitRTC();

     // Initialize the KITL connection if required
    KITLIoctl(IOCTL_KITL_STARTUP, NULL, 0, NULL, 0, NULL);

    // Initialize RNGC module
    RNGB_Init();

    goto cleanUp;
cleanUp:
	//
	// CS&ZHL Jul-23 2011: check EM9170 OS platform
	//
	
	pCPLD = (PEM9K_CPLD_REGS)OALPAtoUA(CSP_BASE_MEM_PA_CS5);
    if (pCPLD == NULL)
    {
		OALMSG(OAL_ERROR, (L"EM9170: CS5 mapping failed!\r\n"));
		//while(1);
		for( ; ; )
		{
		}
    }

	VID[0] = INREG8(&pCPLD->VID[3]);		// OFFSET = 0x58, Vendor ID = "EM9k"	
	VID[1] = INREG8(&pCPLD->VID[2]);
	VID[2] = INREG8(&pCPLD->VID[1]);
	VID[3] = INREG8(&pCPLD->VID[0]);
	VID[4] = '\0';
	
	if( (VID[0]!='E')||(VID[1]!='M')||(VID[2]!='9')||(VID[3]!='k') )
	{
		OALMSG(1, (L"Board Check Failed!\r\n"));
		for( ; ; )
		{
		}
	}

	//OALMSG(OAL_FUNC, (L"-OEMInit\r\n"));
	OALMSG(1, (L"-OEMInit\r\n"));
}

//------------------------------------------------------------------------------

