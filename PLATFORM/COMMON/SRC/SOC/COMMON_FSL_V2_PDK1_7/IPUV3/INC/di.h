//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  di.h
//
//  Definitions for the IPUv3 DI submodules (DI0 and DI1)
//
//-----------------------------------------------------------------------------

#ifndef __DI_H__
#define __DI_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define DI_ERROR  1

//*****************************
// DIGeneralConfigData defines
//*****************************

// dwClkPolarity, dwCS1Polarity, dwCS0Polarity, and dwPinPolarity[] values
#define IPU_DI_GENERAL_DI_POLARITY_ACTIVE_HIGH        1
#define IPU_DI_GENERAL_DI_POLARITY_ACTIVE_LOW         0

// dwSyncFlowErrorTreatment values
#define IPU_DI_GENERAL_DI_ERR_TREATMENT_WAIT          1
#define IPU_DI_GENERAL_DI_ERR_TREATMENT_DRIVE_LAST_COMPONENT 0

// dwWatchdogMode values
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_4_CYCLES      0
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_16_CYCLES     1
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_64_CYCLES     2
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_128_CYCLES    3

// dwClkSource values
#define IPU_DI_GENERAL_DI_CLK_EXT_EXTERNAL            1
#define IPU_DI_GENERAL_DI_CLK_EXT_INTERNAL            0

// dwVSyncSource values
#define IPU_DI_GENERAL_DI_VSYNC_EXT_EXTERNAL          1
#define IPU_DI_GENERAL_DI_VSYNC_EXT_INTERNAL          0

// dwMaskSelect values
#define IPU_DI_GENERAL_DI_MASK_SEL_MASK               1
#define IPU_DI_GENERAL_DI_MASK_SEL_COUNTER_2          0

// dwDisplayClockInitMode values
#define IPU_DI_GENERAL_DI_DISP_CLOCK_INIT_RUNNING     1
#define IPU_DI_GENERAL_DI_DISP_CLOCK_INIT_STOPPED     0

// dwClockStopMode values
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_NEXT_EDGE   0
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER1    1
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER2    2
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER3    3
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER4    4
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER5    5
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER6    6
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER7    7
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER8    8
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER9    9
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOL_NOW     12
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOF_NOW     13
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOL_NEXT_FRAME 14
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOF_NEXT_FRAME 15

//*****************************
// DIUpDownConfigData defines
//*****************************

// dwSet values
#define IPU_DI_DW_GEN_DI_PT_DW_SET0                   0
#define IPU_DI_DW_GEN_DI_PT_DW_SET1                   1
#define IPU_DI_DW_GEN_DI_PT_DW_SET2                   2
#define IPU_DI_DW_GEN_DI_PT_DW_SET3                   3

//*****************************
// DISyncConfigData defines
//*****************************

// dwOffsetResolution values
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_DISABLE   0
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_DISP_CLK  1
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_1 2
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_2 3
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_3 4
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_4 5
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_5 6
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_EXT_VSYNC 6
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_ON        7

// dwRunResolution values
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_DISABLE      0
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_DISP_CLK     1
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_1    2
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_2    3
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_3    4
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_4    5
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_5    6
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_EXT_VSYNC    6
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_ON           7

// dwPolarityClearSelect values
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED                   0
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_NOT_INVERTED               1
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_1_SET  2
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_2_SET  3
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_3_SET  4
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_4_SET  5
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_5_SET  6
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_6_SET  7

// dwToggleTriggerSelect values
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_DISABLE     0
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_DISP_CLK    1
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_1   2
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_2   3
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_3   4
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_4   5
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_5   6
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_EXT_VSYNC   6
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_ON          7

// dwCounterClearSelect values
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_DISABLE         0
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_DISP_CLK        1
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_1       2
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_2       3
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_3       4
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_4       5
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_5       6
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_EXT_VSYNC       6
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_ON              7

// dwPolarityGeneratorEnable values
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_DISABLE 0
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_TRIGGER 1
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_ENABLE  2
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_COUNTER 3


//*****************************
// DIUpDownConfigData defines
//*****************************
#define IPU_DI_POL_ACTIVE_HIGH                        1
#define IPU_DI_POL_ACTIVE_LOW                         0

//------------------------------------------------------------------------------
// Types

typedef enum {
    DI_POLARITY_SET_DRDY,
    DI_POLARITY_SET_CS0,
    DI_POLARITY_SET_CS1,
    DI_POLARITY_SET_CS0_BYTE_ENABLE,
    DI_POLARITY_SET_CS1_BYTE_ENABLE,
    DI_POLARITY_SET_WAIT,
} DI_POLARITY_SET;

// Info for setting the DI General Register
typedef struct {
    DWORD dwLineCounterSelect;
    DWORD dwClockStopMode;
    DWORD dwDisplayClockInitMode;
    DWORD dwMaskSelect;
    DWORD dwVSyncSource;
    DWORD dwClkSource;
    DWORD dwWatchdogMode;
    DWORD dwClkPolarity;
    DWORD dwSyncFlowCounterSelect;
    DWORD dwSyncFlowErrorTreatment;
    DWORD dwERMVSyncSelect;
    DWORD dwCS1Polarity;
    DWORD dwCS0Polarity;
    DWORD dwPinPolarity[8];
} DIGeneralConfigData, *PDIGeneralConfigData;

// Info for setting the DI DW Gen Registers (Parallel mode)
typedef struct {
    DWORD dwPointer;
    DWORD dwAccessPeriod;  // in ns
    DWORD dwComponentPeriod;  // in ns
    DWORD dwCSPtr;
    DWORD dwPinPtr[7];
} DIPointerConfigData, *PDIPointerConfigData;

// Info for setting the DI DW Gen Registers (Serial mode)
typedef struct {
    DWORD dwPointer;
    DWORD dwSerialPeriod;  // in ns
    DWORD dwStartPeriod;  // in ns
    DWORD dwCSPtr;
    DWORD dwSerialValidBits;
    DWORD dwSerialRSPtr;
    DWORD dwSerialClkPtr;
} DIPointerConfigSerialData, *PDIPointerConfigSerialData;

// Info for setting the DI DW Set Registers
typedef struct {
    DWORD dwPointer;
    DWORD dwSet;
    DWORD dwUpPos; // in ns
    DWORD dwDownPos; // in ns
} DIUpDownConfigData, *PDIUpDownConfigData;

// Info for setting the DI SW Gen Registers
typedef struct {
    DWORD dwPointer;
    DWORD dwRunValue;
    DWORD dwRunResolution;
    DWORD dwOffsetValue;
    DWORD dwOffsetResolution;
    DWORD dwPolarityClearSelect;
    DWORD dwToggleTriggerSelect;
    DWORD dwCounterClearSelect;
    BOOL  bCounterAutoReload;
    DWORD dwPolarityGeneratorEnable;
    DWORD dwUpPos;   // in ns
    DWORD dwDownPos; // in ns
    DWORD dwStepRepeat;
    DWORD dwGentimeSelect;
} DISyncConfigData, *PDISyncConfigData;

// Info for setting the DI Polarity Register
typedef struct {
    DI_POLARITY_SET PolaritySetSelect;

    UINT8 DataPolarity;
    UINT8 PinPolarity[7]; // Polarity of pins when CS0/CS1/DRDY asserted
                          // Bits 6:0 map to pins 17:11

    // Additional miscellaneous polarities
    UINT8 WaitPolarity;
    UINT8 CS1ByteEnablePolarity;
    UINT8 CS0ByteEnablePolarity;
} DIPolarityConfigData, *PDIPolarityConfigData;

// Info for setting the DI Serial Control Register
typedef struct {
    DWORD dwRS1ReadPtr;
    DWORD dwRS0ReadPtr;
    DWORD dwRS1WritePtr;
    DWORD dwRS0WritePtr;
    DWORD dwDI0SerLatchDelay; // in DI clk cycles
    DWORD dwDirectSerLLA;
    DWORD dwSerClkPol;
    DWORD dwSerDataPol;
    DWORD dwSerRSPol;
    DWORD dwSerCSPol;
    DWORD dwWait4Serial;
} DISerialConfigData, *PDISerialConfigData;

// Info for setting the DI Active Window Register
typedef struct {
    UINT8 ActiveWindowTriggerSelector;
    UINT8 HorizontalCounterSelector;
    UINT8 VerticalCounterSelector;
    UINT16 HorizontalStart;
    UINT16 HorizontalEnd;
    UINT16 VerticalStart;
    UINT16 VerticalEnd;
} DIActiveWindowConfigData, *PDIActiveWindowConfigData;


//------------------------------------------------------------------------------
// Functions

// IPU Display Interface Data Initialization Functions
BOOL DIRegsInit();
void DIRegsCleanup();

// DI Submodule Enable/Disable Functions
void DIEnable(DI_SELECT);
void DIDisable(DI_SELECT);

// APIs to set the IPU and DI Clock frequencies
void DISetIPUClkFreq(UINT32);
BOOL DISetDIClkFreq(DI_SELECT di_sel, UINT32 dwDIClkFreq);


// DI Register Configuration Functions
void DIConfigureGeneral(DI_SELECT, PDIGeneralConfigData);
void DIConfigureBaseSyncClockGen(DI_SELECT,
    UINT32 dwClkOffset, UINT32 dwClkPeriod, UINT32 dwUpPos, UINT32 dwDownPos);
void DIConfigurePointer(DI_SELECT, PDIPointerConfigData);
void DIConfigureSerialPointer(DI_SELECT, PDIPointerConfigSerialData);
void DIConfigureUpDown(DI_SELECT, PDIUpDownConfigData);
void DIConfigureSync(DI_SELECT, PDISyncConfigData);
void DIConfigurePolarity(DI_SELECT, PDIPolarityConfigData);
void DIConfigureSerial(DI_SELECT, PDISerialConfigData);
void DIVSyncCounterSelect(DI_SELECT, DWORD);
void DISetSyncStart(DI_SELECT, DWORD);
void DISetScreenHeight(DI_SELECT, DWORD);


// Debug helper function
void DIDumpRegs();

#ifdef __cplusplus
}
#endif

#endif //__DI_H__

