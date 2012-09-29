//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Ipu_common.h
//
//  Common IPU definitions
//
//-----------------------------------------------------------------------------

#ifndef __IPU_COMMON_H__
#define __IPU_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define SCREEN_PIX_WIDTH_VGA        640
#define SCREEN_PIX_HEIGHT_VGA       480

#define SCREEN_PIX_WIDTH_D1         720
#define SCREEN_PIX_HEIGHT_D1_NTSC   480
#define SCREEN_PIX_HEIGHT_D1_PAL    576
#define SCREEN_PIX_WIDTH_720P       1280
#define SCREEN_PIX_HEIGHT_720P      720

#define DISP_MIN_WIDTH                             8
#define DISP_MIN_HEIGHT                            8
#define DISP_BPP                                   16
#define DISP_BYTES_PP                              (DISP_BPP / 8)

#define DEBUG_FUNCTIONS                            0

#define IPU_FUNCTION_ENTRY() \
    DEBUGMSG(DEBUG_FUNCTIONS, (TEXT("++%s\r\n"), __WFUNCTION__))
#define IPU_FUNCTION_EXIT() \
    DEBUGMSG(DEBUG_FUNCTIONS, (TEXT("--%s\r\n"), __WFUNCTION__))

#define CUSTOMESCAPECODEBASE        100100
#define VF_GET_DISPLAY_INFO         (CUSTOMESCAPECODEBASE + 0)


// TV out driver escape codes and defines
#ifndef DISPLAY_GET_OUTPUT_FREQUENCY
#define DISPLAY_GET_OUTPUT_FREQUENCY     (CUSTOMESCAPECODEBASE + 7)
#endif
#ifndef DISPLAY_SET_OUTPUT_FREQUENCY
#define DISPLAY_SET_OUTPUT_FREQUENCY     (CUSTOMESCAPECODEBASE + 8)
#endif

//Allow customer to set the memory attribute fo frame buffer
#ifndef VM_SETATTEX
#define VM_SETATTEX            (CUSTOMESCAPECODEBASE + 11)
#endif

#ifndef DISPLAY_SET_BRIGHTNESS
#define DISPLAY_SET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 12) // Brightness is a DWORD %
#endif
#ifndef DISPLAY_GET_BRIGHTNESS
#define DISPLAY_GET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 13) // Brightness is a DWORD %
#endif

#ifndef DISPLAY_DLS_GET_CSC
#define DISPLAY_DLS_GET_CSC     (CUSTOMESCAPECODEBASE + 14)
#endif
#ifndef DISPLAY_DLS_SET_CSC
#define DISPLAY_DLS_SET_CSC     (CUSTOMESCAPECODEBASE + 15)
#endif

#ifndef DISPLAY_GET_GAMMA_VALUE
#define DISPLAY_GET_GAMMA_VALUE (CUSTOMESCAPECODEBASE + 16) // Gamma is a float %
#endif
#ifndef DISPLAY_SET_GAMMA_VALUE
#define DISPLAY_SET_GAMMA_VALUE (CUSTOMESCAPECODEBASE + 17) // Gamma is a float %
#endif

// Driver escape code to toggle between interlaced/non-interlaced overlay data
#ifndef DISPLAY_IS_VIDEO_INTERLACED
#define DISPLAY_IS_VIDEO_INTERLACED (CUSTOMESCAPECODEBASE + 18) 
#endif

// TVE driver escape codes and defines for CGMS-A
#ifndef CGMSA_GET_OUTPUT_MODE
#define CGMSA_GET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 28)
#endif
#ifndef CGMSA_SET_OUTPUT_MODE
#define CGMSA_SET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 29)
#endif

#ifndef DISPLAY_SET_PRIMARY
#define DISPLAY_SET_PRIMARY       (CUSTOMESCAPECODEBASE + 30) 
#endif

#ifndef DISPLAY_ENABLE_DMI
#define DISPLAY_ENABLE_DMI       (CUSTOMESCAPECODEBASE + 31) 
#endif
#ifndef DISPLAY_DISABLE_DMI
#define DISPLAY_DISABLE_DMI       (CUSTOMESCAPECODEBASE + 32) 
#endif

#ifndef CGMSA_MODE_NTSC
#define CGMSA_MODE_NTSC     0
#endif
#ifndef CGMSA_MODE_PAL
#define CGMSA_MODE_PAL      1
#endif

#define IPU_POLARITY_ACTIVE_HIGH        1
#define IPU_POLARITY_ACTIVE_LOW         0

//------------------------------------------------------------------------------
// Types

typedef enum DisplayPlaneEnum
{
    DisplayPlane_0,
    DisplayPlane_1,
} DisplayPlane;

typedef enum CSCEquationEnum
{
    CSCR2Y_A1,  //RGB to YUV equation A.1
    CSCR2Y_A0,  //RGB to YUV equation A.0
    CSCR2Y_B1,  //RGB to YUV equation B.1
    CSCR2Y_B0,  //RGB to YUV equation A.1
    CSCY2R_A1,  //YUV to RGB equation A.1
    CSCY2R_A0,  //YUV to RGB equation A.0
    CSCY2R_B1,  //YUV to RGB equation B.1
    CSCY2R_B0,  //YUV to RGB equation A.1
    CSCNoOp,
    CSCCustom
} CSCEQUATION;

//  RGB Formats
// Additional RGB formats has to be addede here
typedef enum {
  IPU_PIX_FMT_RGB666,
  IPU_PIX_FMT_RGB565,
  IPU_PIX_FMT_RGB24,
  IPU_PIX_FMT_YUV422,
}IPU_PIXEL_FORMAT;

typedef struct _DISPLAY_FG_CONFIG_DATA {
    int width;
    int height;
    int bpp;
    int stride;
    BOOL verticalFlip;
    UINT16 alpha;
    UINT32 colorKey;
    UINT32 plane;
} DISPLAY_FG_CONFIG_DATA, *PDISPLAY_FG_CONFIG_DATA;

// Bitfield of Sync Display Interface signal polarities.
typedef struct {
    UINT32 DATAMASK_EN:1;
    UINT32 CLKIDLE_EN :1;
    UINT32 CLKSEL_EN  :1;
    UINT32 VSYNC_POL  :1;
    UINT32 ENABLE_POL :1;
    UINT32 DATA_POL   :1;       // true = inverted
    UINT32 CLK_POL    :1;       // true = rising edge
    UINT32 HSYNC_POL  :1;       // true = active high
    UINT32 Dummy      :24;      // Dummy variable for alignment.
} SYNC_IPU_DI_SIGNAL_CFG;

// Bitfield of ASync Display Interface signal polarities.
typedef struct {
    UINT32 DISP_NUM   :2;
    UINT32 DATA_POL   :1;       // true = inverted
    UINT32 CS0_POL    :1;       // true = active high
    UINT32 CS1_POL    :1;       // true = active high
    UINT32 Dummy      :27;      // Dummy variable for alignment.
} ASYNC_IPU_DI_SIGNAL_CFG;

typedef enum {
    IPU_PANEL_SYNC_TYPE_INVALID,
    IPU_PANEL_SYNC_TYPE_SYNCHRONOUS,  // Dumb (Memoryless) Display Panel
    IPU_PANEL_SYNC_TYPE_ASYNCHRONOUS, // Smart Display Panel
} IPU_PANEL_SYNC_TYPE;

// Enumeration to select an interrupt type
typedef enum
{
    DP_INTR_TYPE_SF_START,  // Sync Flow Frame Start
    DP_INTR_TYPE_SF_END,    // Sync Flow Frame End
    DP_INTR_TYPE_ASF_START, // Async Flow Frame Start
    DP_INTR_TYPE_ASF_END,   // Async Flow Frame End
    DP_INTR_TYPE_SF_BRAKE,  // Sync Flow Brake
    DP_INTR_TYPE_ASF_BRAKE, // Sync Flow Brake
}DP_INTR_TYPE, *PDP_INTR_TYPE;

// DC channels enumeration
typedef enum {
    DC_CHANNEL_NONE = 0,
    DC_CHANNEL_0 = 0,
    DC_CHANNEL_READ = 0,
    DC_CHANNEL_1 = 1,
    DC_CHANNEL_DC_SYNC_OR_ASYNC = 1,
    DC_CHANNEL_2 = 2,
    DC_CHANNEL_DC_ASYNC = 2,
    DC_CHANNEL_3 = 3,
    DC_CHANNEL_COMMAND_1 = 3,
    DC_CHANNEL_4 = 4,
    DC_CHANNEL_COMMAND_2 = 4,
    DC_CHANNEL_5 = 5,
    DC_CHANNEL_DP_PRIMARY = 5,
    DC_CHANNEL_6 = 6,
    DC_CHANNEL_DP_SECONDARY = 6,
    DC_CHANNEL_8 = 8,
    DC_CHANNEL_9 = 9,
    DC_CHANNEL_INVALID
} DC_CHANNEL;

typedef enum {
    DC_DISPLAY_0,
    DC_DISPLAY_1,
    DC_DISPLAY_2,
    DC_DISPLAY_3
} DC_DISPLAY;

typedef enum {
    DI_SELECT_DI0,
    DI_SELECT_DI1,
} DI_SELECT;

typedef enum {
    CSI_SELECT_CSI0,
    CSI_SELECT_CSI1,
    CSI_SELECT_CSIAll,
} CSI_SELECT;

typedef enum TopFieldSelectEnum
{
    TopFieldSelect_Even,
    TopFieldSelect_Odd
} TopFieldSelect;

// Info for configuring video de-interlacing.
// Used with DISPLAY_IS_VIDEO_INTERLACED escape code
typedef struct {
    BOOL           bIsInterlaced;
    TopFieldSelect topField;
} InterlacedVideoData, *PInterlacedVideoData;

// Info for configuring bus mapping
typedef struct {
    DWORD dwComponent2Offset;
    DWORD dwComponent2Mask;
    DWORD dwComponent1Offset;
    DWORD dwComponent1Mask;
    DWORD dwComponent0Offset;
    DWORD dwComponent0Mask;
} BusMappingData, *PBusMappingData;

// Info for configuring serial communication timing
typedef struct {
    DWORD dwSerClkPeriod;
    DWORD dwCSUp;
    DWORD dwCSDown;
    DWORD dwClkUp;
    DWORD dwClkDown;
    DWORD dwRSUp;
    DWORD dwRSDown;
} SerialTimingData, *PSerialTimingData;

// Panel Information can be filled as a struct variable to extend the driver
typedef struct PANEL_INFO_ST {
    // General panel info
    PUCHAR NAME;                      // Panel string name
    IPU_PANEL_SYNC_TYPE SYNC_TYPE;    // Select synchronous (dumb) or asynchronous (smart) display panel
    DI_SELECT  DI_NUM;                // Is panel connected to Display Interface 0 or 1 (DI0 or DI1)
    UINT32 PANELID;                   // Panel ID as defined in platform-level IPU_PANEL_ID enumeration
    UINT32 PORTID;                    // Port ID as defined in platform-level IPU_PORT_TYPE enumeration
    IPU_PIXEL_FORMAT PIXEL_FMT;       // Select the output pixel format to the panel
    UINT32 WIDTH;                     // Screen width in pixels
    UINT32 HEIGHT;                    // Screen height in pixels
    UINT32 FREQUENCY;                 // Refresh rate frequency
    INT32 TRANSFER_CYCLES;            // Number of cycles required to refresh pixel data to display panel
    PBusMappingData MAPPINGS;         // Pointer to an array of bus mappings.  There must be one bus mapping for each Transfer cycle specified
                                      // in TRANSFER_CYLCES.  This determines how the RGB888 data internal the IPU is mapped and
                                      // transmitted to the display panel

    // Synchronous Panel Info
    UINT32 VSYNCWIDTH;                // VSync width in lines
    UINT32 VSTARTWIDTH;               // VStart width in lines
    UINT32 VENDWIDTH;                 // VEnd width in lines
    UINT32 HSYNCWIDTH;                // HSync width in pixel clock cycles
    UINT32 HSTARTWIDTH;               // HStart width in pixel clock cycles
    UINT32 HENDWIDTH;                 // HEnd width in pixel clock cycles
    UINT32 PIX_CLK_FREQ;              // Pixel clock frequency, in Hz
    UINT32 PIX_DATA_POS;              // Pixel data position, in ns
    INT PIX_CLK_UP;                   // Pixel clock up position, in ns
    INT PIX_CLK_DOWN;                 // Pixel clock down position, in ns
    SYNC_IPU_DI_SIGNAL_CFG SYNC_SIG_POL;   // Signal polarities for a synchronous panel (set to 0 if these settings do not apply)
    // TV only sync panel info
    UINT32 VSTARTWIDTH_FIELD0;        // Field0 VStart width in lines
    UINT32 VENDWIDTH_FIELD0;          // Field0 VEnd width in lines
    UINT32 VSTARTWIDTH_FIELD1;        // Field1 VStart width in lines
    UINT32 VENDWIDTH_FIELD1;          // Field1 VEnd width in lines
    BOOL   ISINTERLACE;               // TRUE if TV output data is interlaced; FALSE if progressive

    // Asynchronous Panel Info
    UINT32 RD_CYCLE_PER;              // Cycle length for read operation, in ns
    UINT32 RD_UP_POS;                 // Up position for read operation, in ns
    UINT32 RD_DOWN_POS;               // Down position for read operation, in ns
    UINT32 WR_CYCLE_PER;              // Cycle length for write operation, in ns
    UINT32 WR_UP_POS;                 // Up position for write operation, in ns
    UINT32 WR_DOWN_POS;               // Down position for write operation, in ns
    ASYNC_IPU_DI_SIGNAL_CFG ASYNC_SIG_POL; // Signal polarities for an asynchronous panel (set to 0 if these settings do not apply)

    // Serial Configuration
    BOOL   USESERIALPORT;             // Set to TRUE if panel uses IPU serial to communicate with panel
    INT32 SERIAL_TRANSFER_CYCLES;     // Number of cycles required to write serial data to display panel
    PBusMappingData SERIAL_MAPPINGS;  // Pointer to an array of bus mappings.  There must be one bus mapping for each serial communication transfer 
                                      // cycle specified in SERIAL_TRANSFER_CYLCES.  This determines how the RGB888 data internal the 
                                      // IPU is mapped and transmitted to the display panel via the serial port
    SerialTimingData SERIAL_TIMING;   // Structure containing timing data for serial communication with panel 

    // Dynamic panel status information
    UINT32 MODEID;                    // Select a unique mode number (MODEID may be changed as mode array is
                                      // first being built, but should not change thereafter)
    BOOL   ISINITIALIZED;             // Variable to track one-time initialization of bus mapping info
    BOOL   ISACTIVE;                  // Dynamic panel status info...always intialized to FALSE
    DC_DISPLAY DISPLAY_NUM;           // Select IPU display number that panel is connected to
                                      // This is configured during first-time initialization
} PANEL_INFO;

typedef enum _IPU_DRIVE_TYPE{
    UNKNOWN,
    eIPU_SYNC,
    eIPU_ASYNC
}IPU_DRIVE_TYPE, *PIPU_DRIVE_TYPE;

typedef struct _DISPLAY_CHARACTERISTIC{
    INT32    width;
    INT32    height;
    INT32    offsetX;
    INT32    offsetY;
    INT32    bpp;

}DISPLAY_CHARACTERISTIC, *PDISPLAY_CHARACTERISTIC;


typedef enum _DISPLAY_ID_TYPE{
    DISPLAY_ID_MAIN_DISPLAY = 0,
    DISPLAY_ID_SECONDARY_DISPLAY,
}DISPLAY_ID, *PDISPLAY_ID;

//The structure is used to set the primary surface 
// physical address directly through extend API.
typedef struct _DISPLAY_PRIMARY{
    DISPLAY_ID id; //identify for main display or secondary display
    UINT32 phyAddr;
}DISPLAY_PRIMARY, *PDISPLAY_PRIMARY;

typedef enum _DISPLAY_GPUID_TYPE{
    DMI_GPU3D = 0,
    DMI_GPU2D = 1
}DISPLAY_GPUID, *PDISPLAY_GPUID;

typedef enum _DISPLAY_GPU_COLOR_TYPE{
    DMI_RGB565 = 0,
    DMI_RGB888
}DISPLAY_GPU_COLOR, *PDISPLAY_GPU_COLOR;

typedef enum _DISPLAY_GPU_BUFFERMODE_TYPE{
    DMI_DOUBLE_BUFFERED = 0,
    DMI_TRIPLE_BUFFERED
}DISPLAY_GPU_BUFFERMODE, *PDISPLAY_GPU_BUFFERMODE;

typedef enum _VM_ATTRIBUTE_TYPE{
    VMATTR_NONCACHED = 0,              //non-cached, write buffer disabled either 
    VMATTR_ONLYBUFFERED = 1,           //non-cached, write buffer enabled only
    VMATTR_CACHED_WRITETHROUGH = 2,    //write through mdoe           
    VMATTR_CACHED_WRITEBACK = 3        //write back mode          
}VM_ATTRIBUTE, *PVM_ATTRIBUTE;

typedef struct _DISPLAY_DMI{
    DISPLAY_GPUID eGPUID; 
    DISPLAY_GPU_COLOR eColorFormat;
    UINT32 uiStride;
    DISPLAY_GPU_BUFFERMODE eBufferMode;
    void* pBuffer1;
    void* pBuffer2;
    void* pBuffer3;
}DISPLAY_DMI, *PDISPLAY_DMI;

// structure for VirtualSetAttributesEx
typedef struct VMSetAttributeExDataStruct{
  LPVOID lpvAddress;    // starting address of virtual memory
  DWORD cbSize;         // size of virtual memory
  VM_ATTRIBUTE vmAttr;  //The attribute of memory
}VMSetAttributeExData, *pVMSetAttributeExData;


//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif //__IPU_COMMON_H__

