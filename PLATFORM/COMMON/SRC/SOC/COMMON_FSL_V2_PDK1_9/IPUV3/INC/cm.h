//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  cm.h
//
//  Definitions for the IPUv3 CM submodule (IPU Common registers)
//
//-----------------------------------------------------------------------------

#ifndef __CM_H__
#define __CM_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define CM_ERROR  1

//*****************************
// dwIPUMemoriesFlags values
//*****************************
#define IPU_RESET_MEM_SRM                  0x1
#define IPU_RESET_MEM_ALPHA                0x2
#define IPU_RESET_MEM_CPMEM                0x4
#define IPU_RESET_MEM_TPM                  0x8
#define IPU_RESET_MEM_MPM                  0x10
#define IPU_RESET_MEM_BM                   0x20
#define IPU_RESET_MEM_RM                   0x40
#define IPU_RESET_MEM_DSTM                 0x80
#define IPU_RESET_MEM_DSOM                 0x100
#define IPU_RESET_MEM_LUT0                 0x200
#define IPU_RESET_MEM_LUT1                 0x400
#define IPU_RESET_MEM_DC_TEMPLATE          0x100000
#define IPU_RESET_MEM_DMFC_RD              0x200000
#define IPU_RESET_MEM_DMFC_WR              0x400000

// Processing Task Status Values
#define IPU_PROC_TASK_STAT_IDLE            0
#define IPU_PROC_TASK_STAT_ACTIVE          1
#define IPU_PROC_TASK_STAT_WAIT4READY      2

// These values apply only to the MEM2PRP task
#define IPU_PROC_TASK_STAT_BOTH_ACTIVE     1
#define IPU_PROC_TASK_STAT_ENC_ACTIVE      2
#define IPU_PROC_TASK_STAT_VF_ACTIVE       3
#define IPU_PROC_TASK_STAT_BOTH_PAUSE      4

// Display Task Status Values (apply to DP_ASYNC and DC_ASYNCH2 tasks)
#define IPU_DISP_TASK_STAT_IDLE            0
#define IPU_DISP_TASK_STAT_PRIM_ACTIVE     1
#define IPU_DISP_TASK_STAT_ALT_ACTIVE      2
#define IPU_DISP_TASK_STAT_UPDATE_PARAM    3
#define IPU_DISP_TASK_STAT_PAUSE           4

// Display Task Status Values (apply to DC_ASYNC1 task)
#define IPU_DISP_TASK_STAT_ACTIVE          1
#define IPU_DISP_TASK_STAT_WAIT4READY      2

// Applies to DC_ASYNC2_CUR_FLOW and DP_ASYNC_CUR_FLOW
#define IPU_DISP_TASK_STAT_CUR_FLOW_ALTERNATE 1
#define IPU_DISP_TASK_STAT_CUR_FLOW_MAIN      0

// SRM Mode values
#define IPU_IPU_SRM_PRI_SRM_MODE_MCU_ACCESS_RAM                 0
#define IPU_IPU_SRM_PRI_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME  1
#define IPU_IPU_SRM_PRI_SRM_MODE_FSU_CONTROL_SWAP_CONTINUOUSLY  2
#define IPU_IPU_SRM_PRI_SRM_MODE_MCU_CONTROL_UPDATE_NOW         3

// CSI Data source values
#define IPU_IPU_CONF_CSI_DATA_SOURCE_PARALLEL  0
#define IPU_IPU_CONF_CSI_DATA_SOURCE_MIPI      1 

//------------------------------------------------------------------------------
// Types

// Enumeration to select the processing flow to set
typedef enum
{
    IPU_PROC_FLOW_PRPENC_ROT_SRC,
    IPU_PROC_FLOW_ALT_ISP_SRC,
    IPU_PROC_FLOW_PRPVF_ROT_SRC,
    IPU_PROC_FLOW_PP_SRC,
    IPU_PROC_FLOW_PP_ROT_SRC,
    IPU_PROC_FLOW_ISP_SRC,
    IPU_PROC_FLOW_PRP_SRC,
    IPU_PROC_FLOW_ENC_IN_VALID,
    IPU_PROC_FLOW_VF_IN_VALID,
    IPU_PROC_FLOW_VDI_SRC,
    IPU_PROC_FLOW_PRPENC_DEST,
    IPU_PROC_FLOW_PRPVF_DEST,
    IPU_PROC_FLOW_PRPVF_ROT_DEST,
    IPU_PROC_FLOW_PP_DEST,
    IPU_PROC_FLOW_PP_ROT_DEST,
    IPU_PROC_FLOW_PRPENC_ROT_DEST,
    IPU_PROC_FLOW_PRP_DEST,
    IPU_PROC_FLOW_PRP_ALT_DEST,
    IPU_PROC_FLOW_SMFC0_DEST,
    IPU_PROC_FLOW_SMFC1_DEST,
    IPU_PROC_FLOW_SMFC2_DEST,
    IPU_PROC_FLOW_SMFC3_DEST,
    IPU_PROC_FLOW_EXT_SRC1_DEST,
    IPU_PROC_FLOW_EXT_SRC2_DEST
}IPU_PROC_FLOW, *PIPU_PROC_FLOW;

// Enumeration to select the display flow to set
typedef enum
{
    IPU_DISP_FLOW_DP_SYNC0_SRC,
    IPU_DISP_FLOW_DP_SYNC1_SRC,
    IPU_DISP_FLOW_DP_ASYNC0_SRC,
    IPU_DISP_FLOW_DP_ASYNC1_SRC,
    IPU_DISP_FLOW_DC2_SRC,
    IPU_DISP_FLOW_DC1_SRC,
    IPU_DISP_FLOW_DP_ASYNC1_ALT_SRC,
    IPU_DISP_FLOW_DP_ASYNC0_ALT_SRC,
    IPU_DISP_FLOW_DC2_ALT_SRC
}IPU_DISP_FLOW, *PIPU_DISP_FLOW;


// Enumeration to select the processing task to query
typedef enum
{
    IPU_PROC_TASK_ENC,
    IPU_PROC_TASK_VF,
    IPU_PROC_TASK_PP,
    IPU_PROC_TASK_ENC_ROT,
    IPU_PROC_TASK_VF_ROT,
    IPU_PROC_TASK_PP_ROT,
    IPU_PROC_TASK_MEM2PRP,
    IPU_PROC_TASK_CSI2MEM_SMFC0,
    IPU_PROC_TASK_CSI2MEM_SMFC1,
    IPU_PROC_TASK_CSI2MEM_SMFC2,
    IPU_PROC_TASK_CSI2MEM_SMFC3
}IPU_PROC_TASK, *PIPU_PROC_TASK;

// Enumeration to select the display task to query
typedef enum
{
    IPU_DISP_TASK_DP_ASYNC,
    IPU_DISP_TASK_DP_ASYNC_CUR_FLOW,
    IPU_DISP_TASK_DC_ASYNC1,
    IPU_DISP_TASK_DC_ASYNC2,
    IPU_DISP_TASK_DC_ASYNC2_CUR_FLOW
}IPU_DISP_TASK, *PIPU_DISP_TASK;

// Enumeration to set SRM priority for a selected IPU submodule
typedef enum
{
    IPU_SRM_PRI_MODULE_CSI1,
    IPU_SRM_PRI_MODULE_CSI0,
    IPU_SRM_PRI_MODULE_ISP,
    IPU_SRM_PRI_MODULE_DP,
    IPU_SRM_PRI_MODULE_DC,
    IPU_SRM_PRI_MODULE_DI0,
    IPU_SRM_PRI_MODULE_DI1
}IPU_SRM_PRI_MODULE, *PIPU_SRM_PRI_MODULE;

// Enumeration to set SRM priority for a selected IPU submodule
typedef enum
{
    IPU_SRM_MODE_MODULE_CSI1,
    IPU_SRM_MODE_MODULE_CSI0,
    IPU_SRM_MODE_MODULE_ISP,
    IPU_SRM_MODE_MODULE_DP_SYNC,
    IPU_SRM_MODE_MODULE_DP_ASYNC0,
    IPU_SRM_MODE_MODULE_DP_ASYNC1,
    IPU_SRM_MODE_MODULE_DC_2,
    IPU_SRM_MODE_MODULE_DC_6,
    IPU_SRM_MODE_MODULE_DI0,
    IPU_SRM_MODE_MODULE_DI1
}IPU_SRM_MODE_MODULE, *PIPU_SRM_MODE_MODULE;

//------------------------------------------------------------------------------
// Functions

// IPU Display Interface Data Initialization Functions
BOOL CMRegsInit();
void CMRegsCleanup();

// CM Register Configuration Functions
void CMSetProcFlow(IPU_PROC_FLOW procFlowType, DWORD procFlowVal);
void CMSetDispFlow(IPU_DISP_FLOW dispFlowType, DWORD dispFlowVal);
void CMResetIPUMemories(DWORD dwIPUMemoriesFlags);
void CMSetMCU_T(DWORD);
void CMSetPathIC2DP(BOOL bIsDMFC, BOOL bIsSync);

// CM Register Status Functions
DWORD CMGetProcTaskStatus(IPU_PROC_TASK);
DWORD CMGetDispTaskStatus(IPU_DISP_TASK);

// DP interrupt control functions
void CMDPFlowClearIntStatus(DP_INTR_TYPE IntrType);
void CMDPFlowIntCntrl(DP_INTR_TYPE IntrType, BOOL enable);

// DC interrupt control functions
void CMDCFrameCompleteClearIntStatus(DC_CHANNEL dcChan);
void CMDCFrameCompleteIntCntrl(DC_CHANNEL dcChan, BOOL enable);

// DI Start/Stop internal counter functions
void CMStartDICounters(DI_SELECT);
void CMStopDICounters(DI_SELECT);

// SRM mode/priority configuration functions
BOOL CMSetSRMPriority(IPU_SRM_PRI_MODULE module, DWORD priority);
BOOL CMSetSRMMode(IPU_SRM_MODE_MODULE module, DWORD mode);

// CSI source configuration
void CMSetCSIDataSource(CSI_SELECT csi_sel, DWORD source);

// Debug helper function
void CMDumpRegs();

#ifdef __cplusplus
}
#endif

#endif //__CM_H__

