//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  display_vf.h
//
//  local header file.
//
//------------------------------------------------------------------------------

#ifndef __DISPLAY_VF_H__
#define __DISPLAY_VF_H__

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// Defines
#define CUSTOMESCAPECODEBASE        100100
#define VF_GET_DISPLAY_INFO         (CUSTOMESCAPECODEBASE + 0)
#define VF_CONFIG                   (CUSTOMESCAPECODEBASE + 1)
#define VF_SET_OFFSET               (CUSTOMESCAPECODEBASE + 2)
#define VF_BUF_SET                  (CUSTOMESCAPECODEBASE + 3)
#define VF_ENABLE                   (CUSTOMESCAPECODEBASE + 4)
#define VF_DISABLE                  (CUSTOMESCAPECODEBASE + 5)
#define VF_PEEKPOKE                 (CUSTOMESCAPECODEBASE + 6)

// TV out driver escape codes and defines
#ifndef DISPLAY_GET_OUTPUT_MODE
#define DISPLAY_GET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 7)
#endif
#ifndef DISPLAY_SET_OUTPUT_MODE
#define DISPLAY_SET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 8)
#endif

#ifndef DISPLAY_UPDATE_OUTPUT_MODE
#define DISPLAY_UPDATE_OUTPUT_MODE  (CUSTOMESCAPECODEBASE + 9)
#endif

#ifndef DISPLAY_MODE_DEVICE
#define DISPLAY_MODE_DEVICE     0
#endif
#ifndef DISPLAY_MODE_NTSC
#define DISPLAY_MODE_NTSC       1
#endif
#ifndef DISPLAY_MODE_PAL
#define DISPLAY_MODE_PAL        2
#endif
#ifndef DISPLAY_MODE_NONE
#define DISPLAY_MODE_NONE       3
#endif

#define VF_GET_FRAMEBUFFER_PTR      (CUSTOMESCAPECODEBASE + 10)
#define VM_SETATTEX                 (CUSTOMESCAPECODEBASE + 11)

#ifndef DISPLAY_SET_BRIGHTNESS
#define DISPLAY_SET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 12) // Brightness is a DWORD %
#endif
#ifndef DISPLAY_GET_BRIGHTNESS
#define DISPLAY_GET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 13) // Brightness is a DWORD %
#endif

//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------

typedef enum _IPU_DRIVE_TYPE{
    UNKNOWN,
    eIPU_SDC,
    eIPU_ADC
}IPU_DRIVE_TYPE, *PIPU_DRIVE_TYPE;

typedef enum _DISP_BUF_TYPE{
    eBUF_0,
    eBUF_1
}DISP_BUF_TYPE;


typedef struct _DISPLAY_CHARACTERISTIC{
    IPU_DRIVE_TYPE eType;
    int    width;
    int    height;
    int    offsetX;
    int    offsetY;
    int    bpp;

}DISPLAY_CHARACTERISTIC, *PDISPLAY_CHARACTERISTIC;


typedef struct _DISPLAY_BUFFER{
    DISP_BUF_TYPE    eSrcBuf;   // eBUF_0 or eBUF_1
    PHYSICAL_ADDRESS paBuf;     // For SDC driver, the source must be
                                // system buffer
}DISPLAY_BUFFER, *PDISPLAY_BUFFER;

typedef struct _DISPLAY_SDC_FG_CONFIG_DATA {
    int width;
    int height;
    int bpp;
    int stride;
    BOOL verticalFlip;
    UINT16 alpha;
    UINT32 colorKey;
    UINT32 plane;
} DISPLAY_SDC_FG_CONFIG_DATA, *PDISPLAY_SDC_FG_CONFIG_DATA;

// TODO: Deprecate
typedef struct _DISPLAY_VF_SOURCE_SDC{
    int input_channel;          // If the data is directly transmitted
                                // from a DMA channel, this parameter
                                // indicates the IDMAC channel number.
                                // This parameter is only used by ADC
                                // driver.
    BOOL bRotate;               // Bool flag to indicate rotation is desired
    int  width;                 // width of the viewfinder frame
    int  height;                // height of the viewfinder frame
    int  left;                  // x coordinate of the viewfinder
    int  top;                   // y coordinate of the viewfinder 

}DISPLAY_VF_SOURCE_SDC, *PDISPLAY_VF_SOURCE_SDC;


typedef struct _DEBUG_INFO{
    unsigned long   uReg;
    UINT32          uValue;
    BOOL            bWriteFlag;
}DEBUG_INFO, *PDEBUG_INFO;

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif // __DISPLAY_VF_H__
