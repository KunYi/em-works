//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dc.h
//
//  Definitions for the IPUv3 DC submodule
//
//-----------------------------------------------------------------------------

#ifndef __DC_H__
#define __DC_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define DC_ERROR  1

//*****************************
// DCWriteChanConfData defines
//*****************************

// dwDINum values
#define IPU_DC_PROG_DI_ID_DI1                         1
#define IPU_DC_PROG_DI_ID_DI0                         0

// dwDispNum values
#define IPU_DC_PROG_DISP_ID_DISP0                     0
#define IPU_DC_PROG_DISP_ID_DISP1                     1
#define IPU_DC_PROG_DISP_ID_DISP2                     2
#define IPU_DC_PROG_DISP_ID_DISP3                     3

// dwWordSize values
#define IPU_DC_W_SIZE_8BITS                           0
#define IPU_DC_W_SIZE_16BITS                          1
#define IPU_DC_W_SIZE_24BITS                          2
#define IPU_DC_W_SIZE_32BITS                          3

// dwEventMask values
#define IPU_DC_CHAN_MASK_DEFAULT_NO_MASK              1
#define IPU_DC_CHAN_MASK_DEFAULT_HIGHEST_PRI          0

// dwChanMode values
#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_DISABLE                 0
#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_NORMAL_NO_ANTI_TEARING  4
#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_NORMAL_ANTI_TEARING     5
#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_WITH_COMMAND_CHAN       7

// dwFieldMode values
#define IPU_DC_WR_CH_CONF_FIELD_MODE_FIELD_MODE       1
#define IPU_DC_WR_CH_CONF_FIELD_MODE_FRAME_MODE       0

//*****************************
// DCGeneralData defines
//*****************************

// dwCh1SyncSel values
#define IPU_DC_GEN_SYNC_1_6_CH1_HANDLES_ASYNC         0
#define IPU_DC_GEN_SYNC_1_6_CH1_HANDLES_SYNC          2

// dwMaskChanEnable values
#define IPU_DC_GEN_MASK_EN_ENABLE                     1
#define IPU_DC_GEN_MASK_EN_DISABLE                    0

// dwMaskChanSelect values
#define IPU_DC_GEN_MASK4CHAN_5_DP                     1
#define IPU_DC_GEN_MASK4CHAN_5_DC                     0

// dwCh5Priority values
#define IPU_DC_GEN_SYNC_PRIORITY_5_HIGH               1
#define IPU_DC_GEN_SYNC_PRIORITY_5_LOW                0

// dwCh1Priority values
#define IPU_DC_GEN_SYNC_PRIORITY_1_HIGH               1
#define IPU_DC_GEN_SYNC_PRIORITY_1_LOW                0

// dwCh5SyncSel values
#define IPU_DC_GEN_DC_CH5_TYPE_ASYNC                  1
#define IPU_DC_GEN_DC_CH5_TYPE_SYNC                   0

// dwBlinkEnable values
#define IPU_DC_GEN_DC_BK_EN_ENABLE                    1
#define IPU_DC_GEN_DC_BK_EN_DISABLE                   0

//*****************************
// DCDispConfData defines
//*****************************

// dwDispType values
#define IPU_DC_DISP_CONF1_DISP_TYP_SERIAL             0
#define IPU_DC_DISP_CONF1_DISP_TYP_PARALLEL_NO_BYTE_EN 2
#define IPU_DC_DISP_CONF1_DISP_TYP_PARALLEL_BYTE_EN   3

// dwAddrIncrement values
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_1            0
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_2            1
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_3            2
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_4            3

// dwByteEnableAddrIncrement values
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_0             0
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_1             1
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_2             2
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_3             3

//*****************************
// DCUserEventConfData defines
//*****************************

// dwOddModeEnable values
#define IPU_DC_UGDE_0_ODD_EN_ENABLE                   1
#define IPU_DC_UGDE_0_ODD_EN_DISABLE                  0

// dwAutorestartEnable values
#define IPU_DC_UGDE_0_AUTO_RESTART_ENABLE             1
#define IPU_DC_UGDE_0_AUTO_RESTART_DISABLE            0

// dwCounterTriggerSelect values
#define IPU_DC_UGDE_0_NF_NL_NEW_LINE                  0
#define IPU_DC_UGDE_0_NF_NL_NEW_FRAME                 1
#define IPU_DC_UGDE_0_NF_NL_NEW_FIELD                 2

//*****************************
// MicrocodeConfigData defines
//*****************************

// dwStop values
#define DC_TEMPLATE_STOP_LAST_COMMAND                 1
#define DC_TEMPLATE_STOP_CONTINUE                     0


//*****************************
// DCConfigureMicrocodeEventMCU()
// region and readWriteSel defines
//*****************************
#define DC_READ_WRITE_SELECT_READ                     0
#define DC_READ_WRITE_SELECT_WRITE                    1

#define DC_REGION_SELECT_REGION1                      1
#define DC_REGION_SELECT_REGION2                      2

//------------------------------------------------------------------------------
// Types

typedef enum {
    DC_COMMAND_CHANNEL_3,
    DC_COMMAND_CHANNEL_4,
} DC_COMMAND_CHANNEL;

typedef enum {
    DC_USER_GENERAL_DATA_EVENT_0,
    DC_USER_GENERAL_DATA_EVENT_1,
    DC_USER_GENERAL_DATA_EVENT_2,
    DC_USER_GENERAL_DATA_EVENT_3
} DC_USER_GENERAL_DATA_EVENT;

typedef enum {
    DC_EVENT_NEW_LINE,
    DC_EVENT_NEW_FRAME,
    DC_EVENT_NEW_FIELD,
    DC_EVENT_END_OF_FRAME,
    DC_EVENT_END_OF_FIELD,
    DC_EVENT_END_OF_LINE,
    DC_EVENT_NEW_CHAN,
    DC_EVENT_NEW_ADDR,
    DC_EVENT_NEW_DATA,
} DC_EVENT_TYPE;

typedef enum {
    DC_MICROCODE_COMMAND_HLG,
    DC_MICROCODE_COMMAND_WRG,
    DC_MICROCODE_COMMAND_HLOA,
    DC_MICROCODE_COMMAND_WROA,
    DC_MICROCODE_COMMAND_HLOD,
    DC_MICROCODE_COMMAND_WROD,
    DC_MICROCODE_COMMAND_HLOAR,
    DC_MICROCODE_COMMAND_WROAR,
    DC_MICROCODE_COMMAND_HLODR,
    DC_MICROCODE_COMMAND_WRODR,
    DC_MICROCODE_COMMAND_WRBC,
    DC_MICROCODE_COMMAND_WCLK,
    DC_MICROCODE_COMMAND_WSTSI,
    DC_MICROCODE_COMMAND_WSTSII,
    DC_MICROCODE_COMMAND_WSTSIII,
    DC_MICROCODE_COMMAND_RD,
    DC_MICROCODE_COMMAND_WACK,
    DC_MICROCODE_COMMAND_MSK,
    DC_MICROCODE_COMMAND_HMA,
    DC_MICROCODE_COMMAND_HMA1,
    DC_MICROCODE_COMMAND_BMA
} DC_MICROCODE_COMMAND;

// Info for setting the Write Channel Configuration Register
typedef struct {
    DWORD dwStartTime;
    DWORD dwChanMode;
    DWORD dwEventMask;
    DWORD dwFieldMode;
    DWORD dwDispNum;
    DWORD dwDINum;
    DWORD dwWordSize;
} DCWriteChanConfData, *PDCWriteChanConfData;

// Info for setting the DC General Register
typedef struct {
    DWORD dwBlinkEnable;
    DWORD dwBlinkRate;
    DWORD dwCh5SyncSel;
    DWORD dwCh1Priority;
    DWORD dwCh5Priority;
    DWORD dwMaskChanSelect;
    DWORD dwMaskChanEnable;
    DWORD dwCh1SyncSel;
} DCGeneralData, *PDCGeneralData;

// Info for setting the DC Display Configuration Registers
typedef struct {
    DWORD dwPollingValueAndMaskSelect;   // Ignored
    DWORD dwAddrCompareMode;             // Ignored
    DWORD dwByteEnableAddrIncrement;
    DWORD dwAddrIncrement;
    DWORD dwDispType;
} DCDispConfData, *PDCDispConfData;

// Info for setting the DC User Event Registers
typedef struct {
    DWORD dwCounterCompare;
    DWORD dwCounterOffset;
    DWORD dwEventNumber;
    DWORD dwCounterTriggerSelect;
    DWORD dwAutorestartEnable;
    DWORD dwOddModeEnable;
    DWORD dwOddModeMicrocodeStartAddr;
    DWORD dwMicrocodeStartAddr;
    DWORD dwEventPriority;
    DWORD dwEventDCChan;
} DCUserEventConfData, *PDCUserEventConfData;

// Info for configuring a microcode instruction
typedef struct {
    DWORD dwWord;
    DWORD dwStop;
    DC_MICROCODE_COMMAND Command;
    DWORD dwLf;
    DWORD dwAf;
    DWORD dwOperand;
    DWORD dwMapping;
    DWORD dwWaveform;
    DWORD dwGluelogic;
    DWORD dwSync;
} MicrocodeConfigData, *PMicrocodeConfigData;

// Info for configuring a microcode instruction
typedef struct {
    DC_CHANNEL dcChan;
    DWORD dwRSNum;  // RS0-RS3
    DWORD dwWord;   // Microcode word address
} LLAConfigData, *PLLAConfigData;

//------------------------------------------------------------------------------
// Functions

// IDMAC Functions
BOOL DCRegsInit(void);
void DCRegsCleanup(void);
void DCEnable(void);
void DCDisable(void);

void DCConfigureWriteChannel(DC_CHANNEL, PDCWriteChanConfData);
void DCSetWriteChannelAddress(DC_CHANNEL, DWORD);
void DCConfigureGeneralData(PDCGeneralData);
void DCConfigureDisplay(DC_DISPLAY, PDCDispConfData);
void DCSetDisplayStride(DC_DISPLAY, DWORD);
void DCConfigureUserEventData(DC_USER_GENERAL_DATA_EVENT, PDCUserEventConfData);
void DCDisableUserEventData(DC_USER_GENERAL_DATA_EVENT);
void DCConfigureMicrocode(PMicrocodeConfigData);
void DCConfigureMicrocodeEvent(DC_CHANNEL, DC_EVENT_TYPE, DWORD, DWORD);
void DCConfigureMicrocodeEventMCU(DC_CHANNEL, DC_EVENT_TYPE, DWORD, DWORD, DWORD, DWORD);
BOOL DCConfigureDataMapping(DWORD, PBusMappingData);
DWORD DCGetStatus();
void DCLLA(PLLAConfigData);
void DCWait4TripleBufEmpty(DI_SELECT di_sel);
void DCChangeChannelMode(DC_CHANNEL writeCh, DWORD dwChanMode);

DC_CHANNEL DCIDMACtoDCChannel();

// Debug helper function
void DCDumpRegs();

/*
void DCSetRoutineLinkData();
void DCConfigureReadChannel(DC_CHANNEL);
void DCConfigureCommandChannel(DC_COMMAND_CHANNEL);
void DCConfigureDIData();
*/

#ifdef __cplusplus
}
#endif

#endif //__DC_H__

