//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Ipu_base.h
//
//  IPU Base interface definitions
//
//-----------------------------------------------------------------------------

#ifndef __IPU_BASE_H__
#define __IPU_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define IPU_IOCTL_GET_BASE_ADDR                    0
#define IPU_IOCTL_ENABLE_SUBMODULE                 1
#define IPU_IOCTL_DISABLE_SUBMODULE                2
#define IPU_IOCTL_ENABLE_CLOCKS                    3
#define IPU_IOCTL_DISABLE_CLOCKS                   4
#define IPU_IOCTL_GET_VIDEO_MEMORY_SIZE            5
#define IPU_IOCTL_ALLOCATE_BUFFER                  6
#define IPU_IOCTL_DEALLOCATE_BUFFER                7
#define IPU_IOCTL_SET_PROC_FLOW                    8
#define IPU_IOCTL_SET_DISP_FLOW                    9
#define IPU_IOCTL_GET_VIDEO_MEMORY_BASE            10

// IDMAC interrupt events
#define IPU_PRP_INTR_EVENT                         L"PRP Interrupt"
#define IPU_PP_INTR_EVENT                          L"PP Interrupt"
#define IPU_DPBG_INTR_EVENT                        L"DP BG Interrupt"
#define IPU_DPFG_INTR_EVENT                        L"DP FG Interrupt"
#define IPU_DC_INTR_EVENT                          L"DC Interrupt"
#define IPU_DC_CH0_INTR_EVENT                      L"DC CH0 Interrupt"
#define IPU_DC_CH1_INTR_EVENT                      L"DC CH1 Interrupt"
#define IPU_DC_CH2_INTR_EVENT                      L"DC CH2 Interrupt"
#define IPU_SMFC_INTR_EVENT                        L"SMFC Interrupt"
#define IPU_VDI_INTR_EVENT                         L"VDI Interrupt"

// Non-IDMAC interrupt events
#define IPU_SNOOPING_INTR_EVENT                    L"Snooping Interrupt"
#define IPU_DP_FLOW_INTR_EVENT                     L"DP Flow Interrupt"
#define IPU_DC_FRAME_COMPLETE_INTR_EVENT           L"DC Frame Complete Interrupt"
#define IPU_DI_VSYNC_PRE_INTR_EVENT                L"DI VSync Pre Interrupt"
#define IPU_DI_COUNTER_INTR_EVENT                  L"DI Counter Interrupt"

// Error interrupt events
#define IPU_DC_TEARING_ERROR_INTR_EVENT            L"DC Tearing Error Interrupt"
#define IPU_DI_SYNC_ERROR_INTR_EVENT               L"DI Sync Error Interrupt"
#define IPU_DI_TIMEOUT_ERROR_INTR_EVENT            L"DI Timeout Error Interrupt"
#define IPU_IC_FRAME_LOST_ERROR_INTR_EVENT         L"IC Frame Lost Error Interrupt"
#define IPU_AXI_ERROR_INTR_EVENT                   L"AXI Error Interrupt"


//------------------------------------------------------------------------------
// Types

typedef enum IPU_SUBMODULE_ENUM
{
    IPU_SUBMODULE_CSI0,  // Camera Sensor Interface 0 submodule
    IPU_SUBMODULE_CSI1,  // Camera Sensor Interface 1 submodule
    IPU_SUBMODULE_IC,    // Image Converter Submodule
    IPU_SUBMODULE_IRT,   // Image Rotation Submodule
    IPU_SUBMODULE_ISP,   // Image Rotation Submodule
    IPU_SUBMODULE_DP,    // Display Processor Submodule
    IPU_SUBMODULE_DI0,   // Display Interface 0 Submodule
    IPU_SUBMODULE_DI1,   // Display Interface 1 Submodule
    IPU_SUBMODULE_SMFC,  // Sensor Multi-FIFO Controller Submodule
    IPU_SUBMODULE_DC,    // Display Controller Submodule
    IPU_SUBMODULE_DMFC,  // Display Multi-FIFO Submodule
    IPU_SUBMODULE_SISG,  // Still Image Synchronization Generator Submodule
    IPU_SUBMODULE_VDI,   // Video De-Interlacer Submodule
    maxNumSubmodules,
} IPU_SUBMODULE;

typedef enum IPU_DRIVER_ENUM
{
    IPU_DRIVER_PP,       // Post-Processor driver
    IPU_DRIVER_PRP_VF,   // Pre-Processor (viewfinding) driver
    IPU_DRIVER_PRP_ENC,  // Pre-Processor (encoding) driver
    IPU_DRIVER_OTHER,    // Pre-Processor (encoding) driver
} IPU_DRIVER;

typedef struct IPUBufferInfoStruct
{
    UINT32 dwBufferSize;
    MEM_TYPE MemType;
} IPUBufferInfo, *pIPUBufferInfo;


//------------------------------------------------------------------------------
// Functions
HANDLE IPUV3BaseOpenHandle(void);
BOOL IPUV3BaseCloseHandle(HANDLE hIPUBase);
DWORD IPUV3BaseGetBaseAddr(HANDLE hIPUBase);
BOOL IPUV3EnableModule(HANDLE hIPUBase, IPU_SUBMODULE submodule, IPU_DRIVER driver);
BOOL IPUV3DisableModule(HANDLE hIPUBase, IPU_SUBMODULE submodule, IPU_DRIVER driver);
BOOL IPUV3EnableClocks(HANDLE hIPUBase);
BOOL IPUV3DisableClocks(HANDLE hIPUBase);
DWORD IPUV3GetVideoMemorySize(HANDLE hIPUBase);
DWORD IPUV3GetVideoMemoryBase(HANDLE hIPUBase);
BOOL IPUV3AllocateBuffer(HANDLE hIPUBase, IPUBufferInfo *pBufferInfo, IpuBuffer *pIpuBuffer);
BOOL IPUV3DeallocateBuffer(HANDLE hIPUBase, IpuBuffer *pIpuBuffer);

#ifdef __cplusplus
}
#endif

#endif //__IPU_BASE_H__

