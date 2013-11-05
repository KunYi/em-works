//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#include <bsp.h>
#include "omap_cpuver.h"

#include "am33x.h"
#include "bsp_padcfg.h"


#if (_WINCEOSVER >= 700)
	#include <vfpSupport.h>
#endif

//------------------------------------------------------------------------------
//  External functions
//
extern DWORD GetCp15ControlRegister(void);
extern DWORD GetCp15AuxiliaryControlRegister(void);
extern LPCWSTR g_oalIoCtlPlatformManufacturer;
extern LPCWSTR g_oalIoCtlPlatformName;
VOID OEMDeinitDebugSerial();
extern BOOL detect_board_id_and_profile_info();


//------------------------------------------------------------------------------
//  Global FixUp variables
//
//  Note: This is workaround for makeimg limitation - no fixup on variables
//        initialized to zero, they must also be const.
//
const volatile DWORD dwOEMFailPowerPaging  = 1;
//const volatile DWORD dwOEMTargetProject    = OEM_TARGET_PROJECT_CEBASE;
const volatile DWORD dwOEMDrWatsonSize     = 0x0004B000;
const volatile DWORD dwOEMHighSecurity     = OEM_HIGH_SECURITY_GP;
const volatile DWORD dwRamdiskEnabled   = (DWORD)-1;

//------------------------------------------------------------------------------
//  Global variables

//-----------------------------------------------------------------------------
//
//  Global:  g_CpuFamily
//
//  Set during OEMInit to indicate CPU family.
//
DWORD g_dwCpuFamily = CPU_FAMILY_AM33X;

//-----------------------------------------------------------------------------
//
//  Global:  g_CpuFamily
//
//  Set during OEMInit to indicate CPU family.
//

DWORD g_dwCpuRevision = (DWORD)CPU_REVISION_UNKNOWN;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMSRAMStartOffset
//
//  offset to start of SRAM where SRAM routines will be copied to. 
//
DWORD dwOEMSRAMStartOffset = 0x00002000;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMMPUContextRestore
//
//  location to store context restore information from off mode (PA)
//
const volatile DWORD dwOEMMPUContextRestore = AM33X_OCMC0_PA;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMVModeSetupTime
//
//  Setup time for DVS transitions. Reinitialized in config.bib (FIXUPVAR)
//
DWORD dwOEMVModeSetupTime = 2;

#if 0
//------------------------------------------------------------------------------
//  Time the PRCM waits for system clock stabilization. 
//  Reinitialized in config.bib (FIXUPVAR)
const volatile DWORD dwOEMPRCMCLKSSetupTime = 0x140;//0x2;

//------------------------------------------------------------------------------
//  location to store context restore information from off mode
const volatile DWORD dwOEMMPUContextRestore = CPU_INFO_ADDR_PA;  // ??
#endif
//------------------------------------------------------------------------------
//  maximum idle period during OS Idle in milliseconds
DWORD dwOEMMaxIdlePeriod = 1000;

//------------------------------------------------------------------------------
//  Save kitl state
DWORD g_oalKitlEnabled;

//-----------------------------------------------------------------------------
//
//  Global:  g_oalRetailMsgEnable
//
//  Used to enable retail messages
//
BOOL   g_oalRetailMsgEnable = FALSE;

//-----------------------------------------------------------------------------
//
//  Global:  g_ResumeRTC
//
//  Used to inform RTC code that a resume occured
//
BOOL g_ResumeRTC = FALSE;

static RamTable   am33evm_RAM;
RAMTableEntry ramEntry[2];

PCRamTable OEMGetOEMRamTable(void)
{  

	am33evm_RAM.dwVersion = MAKELONG(0, 7);
	am33evm_RAM.dwNumEntries = 0; 
    am33evm_RAM.pRamEntries = ramEntry;

	/* Ram entry for second bank */
	{
	    ramEntry[am33evm_RAM.dwNumEntries].ShiftedRamBase = IMAGE_WINCE_DRAM_EXT_PA >> 8;
		ramEntry[am33evm_RAM.dwNumEntries].RamSize = IMAGE_WINCE_DRAM_EXT_SIZE;
		ramEntry[am33evm_RAM.dwNumEntries].RamAttributes = 0;
		
		am33evm_RAM.dwNumEntries++;
	}
	/* if ramdisk is disabled, then add it to system memory */
    if(dwRamdiskEnabled !=1)
	{
	    ramEntry[am33evm_RAM.dwNumEntries].ShiftedRamBase = OALVAtoPA(IMAGE_WINCE_RAM_DISK_CA)>>8;
		ramEntry[am33evm_RAM.dwNumEntries].RamSize = IMAGE_WINCE_RAM_DISK_SIZE;
		ramEntry[am33evm_RAM.dwNumEntries].RamAttributes = 0;
		am33evm_RAM.dwNumEntries++;
	}
	
	OALMSG(OAL_INFO, (L"OEMGetOEMRamTable, dwNumEntries=%d\r\n", am33evm_RAM.dwNumEntries));
    return &am33evm_RAM;
}


DWORD OALMux_UpdateOnDeviceStateChange( UINT devId, UINT oldState, UINT newState, BOOL bPreStateChange )
{
	UNREFERENCED_PARAMETER(devId);
    UNREFERENCED_PARAMETER(oldState);
    UNREFERENCED_PARAMETER(newState);
    UNREFERENCED_PARAMETER(bPreStateChange);
    return (DWORD) -1;
}


extern DEVICE_IFC_GPIO Am3xx_Gpio;
void BSPGpioInit()
{
    BSPInsertGpioDevice(0,&Am3xx_Gpio,L"GIO1:");   // Am3xx GPIOs
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInit
//
//  This is Windows CE OAL initialization function. It is called from kernel
//  after basic initialization is made.
//
static UCHAR allocationPool[2048];
VOID OEMInit()
{
    BOOL           *pColdBoot;
	AM33X_SYSC_PADCONFS_REGS *padconf_r;
	AM33X_PRCM_REGS          *prcm_r;
    
    BOOL           *pRetailMsgEnable;

    //----------------------------------------------------------------------
    // Initialize OAL log zones
    //----------------------------------------------------------------------

    OALLogSetZones( 
    //           (1<<OAL_LOG_VERBOSE)  |
    //           (1<<OAL_LOG_INFO)     |
               (1<<OAL_LOG_ERROR)    |
               (1<<OAL_LOG_WARN)     |
    //           (1<<OAL_LOG_IOCTL)    | 
    //           (1<<OAL_LOG_FUNC)     |
    //           (1<<OAL_LOG_INTR)     |
               0);
    OALMSG(OAL_FUNC, (L"+OEMInit\r\n"));

    //----------------------------------------------------------------------
    // Initialize the OAL memory allocation system (TI code)
    //----------------------------------------------------------------------
    OALLocalAllocInit(allocationPool,sizeof(allocationPool));

    //----------------------------------------------------------------------
    // Update kernel variables
    //----------------------------------------------------------------------
    CEProcessorType = PROCESSOR_ARM_CORTEX;
    CEInstructionSet = PROCESSOR_ARM_V7_INSTRUCTION;

    dwNKDrWatsonSize = dwOEMDrWatsonSize;
  //  gdwFailPowerPaging = dwOEMFailPowerPaging;
  //  cbNKPagingPoolSize = (dwOEMPagingPoolSize == -1) ? 0 : dwOEMPagingPoolSize;
    
    // Alarm has resolution 1 second
    dwNKAlarmResolutionMSec = 1000;

    // Set extension functions
    pOEMIsProcessorFeaturePresent = OALIsProcessorFeaturePresent;
    pfnOEMSetMemoryAttributes     = OALSetMemoryAttributes;

    // Profiling support
    g_pOemGlobal->pfnProfileTimerEnable  = OEMProfileTimerEnable;
    g_pOemGlobal->pfnProfileTimerDisable = OEMProfileTimerDisable;
	g_pOemGlobal->pfnGetOEMRamTable      = OEMGetOEMRamTable;

	// Set values of globals used in IOCTL_HAL_GET_DEVICE_INFO handler
    g_oalIoCtlPlatformManufacturer  = L"Texas Instruments";
    g_oalIoCtlPlatformName          = L"BSP_AM33X";

    CEProcessorType = PROCESSOR_ARM_CORTEX;
    CEInstructionSet = PROCESSOR_ARM_V7_INSTRUCTION;

    //----------------------------------------------------------------------
    // Initialize cache globals
    //----------------------------------------------------------------------

    OALCacheGlobalsInit();
    
 //   #ifdef DEBUG
        OALMSG(1, (L"CPU CP15 Control Register = 0x%x\r\n", GetCp15ControlRegister()));
        OALMSG(1, (L"CPU CP15 Auxiliary Control Register = 0x%x\r\n", GetCp15AuxiliaryControlRegister()));
 //   #endif
        
    //----------------------------------------------------------------------
    // Initialize Power Domains
    //----------------------------------------------------------------------
    
    // OALPowerInit();
    PrcmInit();
	OALI2CInit(AM_DEVICE_I2C0);
	OALI2CInit(AM_DEVICE_I2C1);
	OALI2CInit(AM_DEVICE_I2C2);

    detect_board_id_and_profile_info(); 

    //----------------------------------------------------------------------
    // Initialize Vector Floating Point co-processor
    //----------------------------------------------------------------------

#if (_WINCEOSVER >= 700)
    VfpOemInit(g_pOemGlobal, VFP_AUTO_DETECT_FPSID);
#else
    OALVFPInitialize(g_pOemGlobal);
#endif

    //----------------------------------------------------------------------
    // Initialize interrupt
    //----------------------------------------------------------------------
    if (!OALIntrInit()) {
        OALMSG(OAL_ERROR, (L"ERROR: OEMInit: failed to initialize interrupts\r\n"));
        goto cleanUp;
    }
    INTERRUPTS_ON();
    //----------------------------------------------------------------------
    // Initialize system clock
    //----------------------------------------------------------------------
	PrcmClockSetParent(kTIMER3_GCLK, kSYS_CLKIN_CK);
	if (!OALTimerInit(1, 24000, 200)){
        OALMSG(OAL_ERROR, (L"ERROR: OEMInit: Failed to initialize system clock\r\n"));
        goto cleanUp;
    }    

	OAL3XX_RTCInit(AM33X_RTCSS_REGS_PA, IRQ_RTCALARM);

    //----------------------------------------------------------------------
    // Initialize PAD cfg
    //----------------------------------------------------------------------
    OALPadCfgInit();

    //----------------------------------------------------------------------
    // configure pin mux
    //----------------------------------------------------------------------
    ConfigurePadArray(BSPGetAllPadsInfo());
    if (!RequestDevicePads(AM_DEVICE_I2C1)) OALMSG(OAL_ERROR, (TEXT("Failed to request pads for I2C2\r\n")));

    GPIOInit();
   // Set GPIOs default values (like the buffers' OE)
    // OALGPIOSetDefaultValues();

    //----------------------------------------------------------------------
    // Initialize SRAM Functions
    //----------------------------------------------------------------------
    OALSRAMFnInit();

	//----------------------------------------------------------------------
    // Initialize high performance counter
    //----------------------------------------------------------------------
    PrcmClockSetParent(kTIMER2_GCLK, kSYS_CLKIN_CK);
    OALPerformanceTimerInit(0, 0);

	if (CurMSec == 0){
		OALMSG(1,(L"!!!!!!!!!! LOOKS LOOKS TIMER IS NOT RUNNING %d %d\r\n", __LINE__, CurMSec));
	}

    //----------------------------------------------------------------------
    // Initialize the KITL
    //----------------------------------------------------------------------
    g_oalKitlEnabled = KITLIoctl(IOCTL_KITL_STARTUP, NULL, 0, NULL, 0, NULL);

    //----------------------------------------------------------------------
    // Initialize the watchdog
    //----------------------------------------------------------------------
    
#ifdef BSP_AM33X_WATCHDOG
	OALMSG(1,(L"OALWatchdogInit is about to be called\r\n"));
    OALWatchdogInit(BSP_WATCHDOG_PERIOD_MILLISECONDS,BSP_WATCHDOG_THREAD_PRIORITY);
#endif

    //----------------------------------------------------------------------
    // Check for retail messages enabled
    //----------------------------------------------------------------------
    pRetailMsgEnable = OALArgsQuery(OAL_ARGS_QUERY_OALFLAGS);
    if (pRetailMsgEnable && (*pRetailMsgEnable & OAL_ARGS_OALFLAGS_RETAILMSG_ENABLE))
        g_oalRetailMsgEnable = TRUE;

    //----------------------------------------------------------------------
    // Deinitialize serial debug
    //----------------------------------------------------------------------

    if (!g_oalRetailMsgEnable)
        OEMDeinitDebugSerial();

// not available under CE6
#if (_WINCEOSVER >= 700)
    //----------------------------------------------------------------------
    // Make Page Tables walk L2 cacheable. There are 2 new fields in OEMGLOBAL
    // that we need to update:
    // dwTTBRCacheBits - the bits to set for TTBR to change page table walk
    //                   to be L2 cacheable. (Cortex-A8 TRM, section 3.2.31)
    //                   Set this to be "Outer Write-Back, Write-Allocate".
    // dwPageTableCacheBits - bits to indicate cacheability to access Level
    //                   L2 page table. We need to set it to "inner no cache,
    //                   outer write-back, write-allocate. i.e.
    //                      TEX = 0b101, and C=B=0.
    //                   (ARM1176 TRM, section 6.11.2, figure 6.7, small (4k) page)
    //----------------------------------------------------------------------
    g_pOemGlobal->dwTTBRCacheBits = 0x8;            // TTBR RGN set to 0b01 - outer write back, write-allocate
    g_pOemGlobal->dwPageTableCacheBits = 0x140;     // Page table cacheability uses 1BB/AA format, where AA = 0b00 (inner non-cached)
#endif
    //----------------------------------------------------------------------
    // Check for a clean boot of device
    //----------------------------------------------------------------------
    pColdBoot = OALArgsQuery(OAL_ARGS_QUERY_COLDBOOT);
    if ((pColdBoot == NULL)|| ((pColdBoot != NULL) && *pColdBoot))
        NKForceCleanBoot();

cleanUp:
    OALMSG(OAL_FUNC, (L"-OEMInit\r\n"));
}

