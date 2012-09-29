//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  vdi_priv.h
//
//  Common definitions for IPU's Video De-Interlacer module
//  This is includes APIs between the Stream interface and the
//  configuration code, and also the APIs to the VDI registers.
//
//------------------------------------------------------------------------------

#ifndef __VDI_PRIV_H__
#define __VDI_PRIV_H__

//------------------------------------------------------------------------------
// Defines

//******************************************************
// VDI configuration defines
//******************************************************

#define VDI_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define VDI_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

#ifdef DEBUG
// Debug zone bit positions
#define ZONEID_INIT       0
#define ZONEID_DEINIT     1
#define ZONEID_IOCTL      2
#define ZONEID_DEVICE     3

#define ZONEID_INFO       12
#define ZONEID_FUNCTION   13
#define ZONEID_WARN       14
#define ZONEID_ERROR      15

// Debug zone masks
#define ZONEMASK_INIT     (1<<ZONEID_INIT)
#define ZONEMASK_DEINIT   (1<<ZONEID_DEINIT)
#define ZONEMASK_IOCTL    (1<<ZONEID_IOCTL)
#define ZONEMASK_DEVICE   (1<<ZONEID_DEVICE)

#define ZONEMASK_INFO     (1<<ZONEID_INFO)
#define ZONEMASK_FUNCTION (1<<ZONEID_FUNCTION)
#define ZONEMASK_WARN     (1<<ZONEID_WARN)
#define ZONEMASK_ERROR    (1<<ZONEID_ERROR)

// Debug zone args to DEBUGMSG
#define ZONE_INIT         DEBUGZONE(ZONEID_INIT)
#define ZONE_DEINIT       DEBUGZONE(ZONEID_DEINIT)
#define ZONE_IOCTL        DEBUGZONE(ZONEID_IOCTL)
#define ZONE_DEVICE       DEBUGZONE(ZONEID_DEVICE)

#define ZONE_INFO         DEBUGZONE(ZONEID_INFO)
#define ZONE_FUNCTION     DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_WARN         DEBUGZONE(ZONEID_WARN)
#define ZONE_ERROR        DEBUGZONE(ZONEID_ERROR)
#endif

#define VDI_MIN_INPUT_WIDTH     16
#define VDI_MIN_INPUT_HEIGHT    32
#define VDI_MAX_INPUT_WIDTH     720
#define VDI_MAX_INPUT_HEIGHT    1024
#define VDI_MIN_OUTPUT_WIDTH    16
#define VDI_MIN_OUTPUT_HEIGHT   32
#define VDI_MAX_OUTPUT_WIDTH    720
#define VDI_MAX_OUTPUT_HEIGHT   1024


//******************************************************
// VDI low-level register access defines
//******************************************************

// VDI pixel format select defines, used in VDISetPixelFormat()
#define IPU_VDI_C_VDI_CH_422_FORMAT_422               1
#define IPU_VDI_C_VDI_CH_422_FORMAT_420               0

// VDI motion select defines, used in VDIMotionSelect()
#define IPU_VDI_C_VDI_MOT_SEL_ROM1                    0
#define IPU_VDI_C_VDI_MOT_SEL_ROM2                    1
#define IPU_VDI_C_VDI_MOT_SEL_FULL_MOTION             2

// VDI motion select defines, used in VDISetWatermarkLevel() and VDIClearWatermarkLevel()
#define IPU_VDI_C_WATERMARK_FIFO_1_8TH                0
#define IPU_VDI_C_WATERMARK_FIFO_2_8TH                1
#define IPU_VDI_C_WATERMARK_FIFO_3_8TH                2
#define IPU_VDI_C_WATERMARK_FIFO_4_8TH                3
#define IPU_VDI_C_WATERMARK_FIFO_5_8TH                4
#define IPU_VDI_C_WATERMARK_FIFO_6_8TH                5
#define IPU_VDI_C_WATERMARK_FIFO_7_8TH                6
#define IPU_VDI_C_WATERMARK_FIFO_FULL                 7

// VDI top field select values, used in VDISetTopField()
#define IPU_VDI_C_VDI_TOP_FIELD_FIELD0                0
#define IPU_VDI_C_VDI_TOP_FIELD_FIELD1                1


//------------------------------------------------------------------------------
// Types

//******************************************************
// VDI configuration types
//******************************************************


//******************************************************
// VDI low-level register access types
//******************************************************

typedef enum {
    VDI_CHANNEL_1 = 0,
    VDI_CHANNEL_4 = 0,
    VDI_CHANNEL_2 = 1,
    VDI_CHANNEL_3 = 2,
} VDI_CHANNEL;

//------------------------------------------------------------------------------
// Functions

//******************************************************
// VDI configuration functions
//******************************************************

// Public exposed functions
BOOL VDIConfigure(pVdiConfigData);
BOOL VDIStartChannel(pVdiStartParams);

// Local functions to VDI operation
BOOL VDIInit();
void VDIDeinit();
void VDIIDMACChannelConfig(pVdiConfigData);
BOOL VDIStopChannel();
void VDIEnable(void);
void VDIDisable(void);
BOOL VDIParamsCheck(pVdiConfigData);
void VDIIntrEnable(void);
BOOL VDIWaitForEOF(UINT32);
static void VDIIntrThread(LPVOID);
void VDIISRLoop(UINT32);

//******************************************************
// VDI low-level register access functions
//******************************************************

// Low-level register accesses
BOOL VDIRegsInit();
void VDIRegsCleanup();
void VDIModuleEnable(void);
void VDIModuleDisable(void);

void VDISetFieldDimensions(DWORD width, DWORD height);
void VDISetPixelFormat(DWORD);
void VDIMotionSelect(DWORD);
void VDISetBurstSize(VDI_CHANNEL, DWORD);
void VDISetWatermarkLevel(VDI_CHANNEL, DWORD);
void VDIClearWatermarkLevel(VDI_CHANNEL, DWORD);
void VDISetTopField(VDI_INPUT_SOURCE, DWORD);

// Debug helper function
void VDIDumpRegs();

#endif  // __VDI_PRIV_H__
