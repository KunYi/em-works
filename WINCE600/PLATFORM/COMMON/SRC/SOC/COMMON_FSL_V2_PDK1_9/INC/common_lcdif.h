//------------------------------------------------------------------------------
// Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  common_lcdif.h
//
//
//------------------------------------------------------------------------------

#ifndef __COMMON_LCDIF_H__
#define __COMMON_LCDIF_H__


#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// Defines

#define LCDIF_IRQ_VSYNC_EDGE 0x00000001
#define LCDIF_IRQ_FRAME_DONE 0x00000002
#define LCDIF_IRQ_UNDERFLOW  0x00000004
#define LCDIF_IRQ_OVERFLOW   0x00000008
#define LCDIF_IRQ_BM_ERROR       0x00000010

#define SCREEN_PIX_WIDTH_VGA        640
#define SCREEN_PIX_HEIGHT_VGA       480

#define SCREEN_PIX_WIDTH_QVGA        320
#define SCREEN_PIX_HEIGHT_QVGA       240

#define SCREEN_PIX_WIDTH_WQVGA        400
#define SCREEN_PIX_HEIGHT_WQVGA       272

#define SCREEN_PIX_WIDTH_D1         720
#define SCREEN_PIX_HEIGHT_D1_NTSC   480
#define SCREEN_PIX_HEIGHT_D1_PAL    576
#define SCREEN_PIX_WIDTH_720P       1280
#define SCREEN_PIX_HEIGHT_720P      720

#define DISP_MIN_WIDTH                             8
#define DISP_MIN_HEIGHT                            8
#define DISP_BPP                                   16
#define DISP_BYTES_PP                              (DISP_BPP / 8)

#define CUSTOMESCAPECODEBASE        100100

// Driver escape codes and defines
#ifndef DISPLAY_GET_OUTPUT_MODE
#define DISPLAY_GET_OUTPUT_MODE (CUSTOMESCAPECODEBASE + 9)
#endif
#ifndef DISPLAY_SET_OUTPUT_MODE
#define DISPLAY_SET_OUTPUT_MODE (CUSTOMESCAPECODEBASE + 10)
#endif

// display driver escape code for set video memory attribution
#ifndef VM_SETATTEX
#define VM_SETATTEX                 (CUSTOMESCAPECODEBASE + 11)
#endif

#ifndef DISPLAY_SET_BRIGHTNESS
#define DISPLAY_SET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 12) // Brightness is a DWORD %
#endif
#ifndef DISPLAY_GET_BRIGHTNESS
#define DISPLAY_GET_BRIGHTNESS (CUSTOMESCAPECODEBASE + 13) // Brightness is a DWORD %
#endif

#define DISPLAY_DLS_GET_CSC     (CUSTOMESCAPECODEBASE + 14)
#define DISPLAY_DLS_SET_CSC     (CUSTOMESCAPECODEBASE + 15)

#ifndef DISPLAY_GET_GAMMA_VALUE
#define DISPLAY_GET_GAMMA_VALUE (CUSTOMESCAPECODEBASE + 16) // Gamma is a float %
#endif
#ifndef DISPLAY_SET_GAMMA_VALUE
#define DISPLAY_SET_GAMMA_VALUE (CUSTOMESCAPECODEBASE + 17) // Gamma is a float %
#endif


#ifndef CGMSA_GET_OUTPUT_MODE
#define CGMSA_GET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 28)
#endif
#ifndef CGMSA_SET_OUTPUT_MODE
#define CGMSA_SET_OUTPUT_MODE     (CUSTOMESCAPECODEBASE + 29)
#endif


#ifndef CGMSA_MODE_NTSC
#define CGMSA_MODE_NTSC     0
#endif
#ifndef CGMSA_MODE_PAL
#define CGMSA_MODE_PAL      1
#endif

#define POWERON TRUE
#define POWEROFF FALSE

enum
{
     DISPLAY_MODE_DEVICE=0
    ,DISPLAY_MODE_NTSC
    ,DISPLAY_MODE_NTSC_J
    ,DISPLAY_MODE_PAL
    // NOTE: the following modes are supported by the
    // hardware but not implemented in software.
    // NTSC-M Mode
    // PAL-B Mode
    // PAL-M Mode
    // PAL-N Mode
    // PAL-CN Mode
    // NTSC with 700:300 scaling on "G"
    // PAL-60 Mode
    // NTSC Progressive
    ,DISPLAY_MODE_NONE=0x7f
};

//Selects LCDIF bus width.
typedef enum _LCDIF_DATABUS_WIDTH
{
    //brief Sets up 16 bit bus
    LCDIF_BUS_WIDTH_16BIT=0,

    //brief Sets up 8 bit bus
    LCDIF_BUS_WIDTH_8BIT,

    //brief Sets up 18 bit bus
    LCDIF_BUS_WIDTH_18BIT,

    //brief Sets up 24 bit bus
    LCDIF_BUS_WIDTH_24BIT,

} LCDIF_DATABUS_WIDTH;

// Selects the data output format.
typedef enum _LCDIF_DATASWIZZLE
{
    //No data swap, same as LITTLE_ENDIAN
    NO_SWAP=0,

    //No data swap, same as NO_SWAP
    LITTLE_ENDIAN=0,

    //Swap bytes 0,3 and 1,2, same as SWAP_ALL_BYTES
    BIG_ENDIAN_SWAP=1,

    //Swap bytes 0,3 and 1,2, same as BIG_ENDIAN_SWAP
    SWAP_ALL_BYTES=1,

    //brief Swap half-words
    HWD_SWAP=2,

    //brief Swap bytes within each half-word
    HWD_BYTE_SWAP=3,

} LCDIF_DATASWIZZLE;

// Selects the reset line high or low.
typedef enum _LCDIF_RESET
{
    //LCD_RESET output signal is low
    LCDRESET_LOW=0,

    //LCD_RESET output signal is high
    LCDRESET_HIGH=1

} LCDIF_RESET;

//Selects bus format.
typedef enum _LCDIF_BUSMODE
{
    //Data strobe is active low
    BUSMODE_8080=0,

    //Data strobe is active high
    BUSMODE_6800=1

} LCDIF_BUSMODE;

//Selects command or data mode for the buffer to be sent
typedef enum _LCDIF_COMMANDMODE
{
    //Sets up bus for command mode, DCn signal low
    CMD_MODE=0,

    //Sets up bus for data mode, DCn signal high
    DATA_MODE=1

} LCDIF_COMMANDMODE;

//Selects LCDIF bus width.
typedef enum _LCDIF_WORDLENGTH
{
    //brief Sets up 16 bit bus
    WORDLENGTH_16BITS=0,

    //brief Sets up 8 bit bus
    WORDLENGTH_8BITS,

    //brief Sets up 18 bit bus
    WORDLENGTH_18BITS,

    //brief Sets up 24 bit bus
    WORDLENGTH_24BITS

} LCDIF_WORDLENGTH;

typedef enum _LCDIF_POLARITY
{
    POLARITY_LOW = 0,
    POLARITY_HIGH
} LCDIF_POLARITY;

typedef enum _LCDIF_VSYNCUNIT
{
    VSYNC_UNIT_PIXCLK = 0,
    VSYNC_UNIT_HORZONTAL_LINE
} LCDIF_VSYNCUNIT;

typedef enum _LCDIF_DATASHIFTDIR
{
    DATA_SHIFT_LEFT = 0,
    DATA_SHIFT_RIGHT
} LCDIF_DATASHIFTDIR;

// Timing params
typedef union
{
    struct
    {
        //! Data bus setup time in XCLK cycles.
        //! Also the time that the data strobe is asserted in a cycle
        UINT8 u8DataSetup;
        //! Data bus hold time in XCLK cycles.
        //! Also the time that the data strobe is de-asserted in a cycle
        UINT8 u8DataHold;
        //! Number of XCLK cycles that DCn is active before CEn is asserted
        UINT8 u8CmdSetup;
        //! Number of XCLK cycles that DCn is active after CEn is de-asserted
        UINT8 u8CmdHold;
    } BYTE;
    UINT32 U;
} LCDIF_TIMING;

typedef struct _LCDIF_INIT
{
    //! Enable use of the busy line signal from the LCD
    BOOL bBusyEnable;
    //! Indicate output bytesex
    LCDIF_DATASWIZZLE eDataSwizzle;
    //!
    LCDIF_DATASWIZZLE eCscSwizzle;
    //! Set the reset line low or high
    LCDIF_RESET eReset;
    //! Set mode to 8080 or 6800 interface type
    LCDIF_BUSMODE eBusMode;
    //! Set wordlength 8,16,18,24
    LCDIF_WORDLENGTH eWordLength;
    //! Set buswidth 8,16,18,24
    LCDIF_DATABUS_WIDTH eBusWidth;
    //! Bus timing info
    LCDIF_TIMING Timing;

} LCDIF_INIT;

typedef struct _LCDIFVSYNC
{
    BOOL bOEB;
    LCDIF_POLARITY ePolarity;
    LCDIF_VSYNCUNIT eVSyncPeriodUnit;
    LCDIF_VSYNCUNIT eVSyncPulseWidthUnit;
    UINT32 u32PulseWidth;
    UINT32 u32Period;
    UINT32 WaitCount;

} LCDIFVSYNC;

typedef struct _LCDIFDOTCLK
{
    BOOL bEnablePresent;
    LCDIF_POLARITY eHSyncPolarity;
    LCDIF_POLARITY eEnablePolarity;
    LCDIF_POLARITY eDotClkPolarity;
    UINT32 u32HsyncPulseWidth;
    UINT32 u32HsyncPeriod;
    UINT32 u32HsyncWaitCount;
    UINT32 u32DotclkWaitCount;

} LCDIFDOTCLK;

typedef struct _LCDIFDVI
{
    BOOL   bStartTRS;
    UINT32 u32HactiveCount;
    UINT32 u32HblankingCount;
    UINT32 u32VlinesCount;
    UINT32 u32Field1StartLine;
    UINT32 u32Field1EndLine;
    UINT32 u32Field2StartLine;
    UINT32 u32Field2EndLine;
    UINT32 u32V1BlankStartLine;
    UINT32 u32V1BlankEndLine;
    UINT32 u32V2BlankStartLine;
    UINT32 u32V2BlankEndLine;
    UINT32 u32YFillValue;
    UINT32 u32CBFillValue;
    UINT32 u32CRFillValue;
    UINT32 u32HFillCount;
} LCDIFDVI;

BOOL LCDIFInit(LCDIF_INIT *pLCDIFInit,BOOL bReset,BOOL bTVReset);
VOID LCDIFSetDataShift(LCDIF_DATASHIFTDIR dir, UINT8 NoBits);
VOID LCDIFSetBytePacking(UINT8 ValidBytes);
BOOL LCDIFSetupVsync(LCDIFVSYNC *pLCDIFVsync);
BOOL LCDIFSetupDotclk(LCDIFDOTCLK *pLCDIFDotclk );
BOOL LCDIFSetupDvi(LCDIFDVI *pLCDIFDvi);
VOID LCDIFSetIrqEnable(UINT32 IrqMask);
VOID LCDIFSetDotclkMode(BOOL bEnable);
VOID LCDIFSetVsyncMode(BOOL bEnable);
VOID LCDIFSetDviMode(BOOL bEnable);
VOID LCDIFSetSyncSignals(BOOL bEnable);
VOID LCDIFSetTransferCount(UINT32 WidthInBytes, UINT32 Height);
VOID LCDIFSetBusMasterMode(BOOL bEnable);
UINT32 LCDIFActiveIrq(VOID);
VOID LCDIFClearIrq(UINT32 irq_mask);
VOID LCDIFPowerDown(BOOL bPowerOff);
VOID LCDIFResetController(LCDIF_RESET reset);

BOOL LCDIFDisplayFrameBuffer(const void* pData);
VOID LCDIFFlush(VOID);
VOID LCDIFSetInterlace(BOOL bEnable);

VOID LCDIFSetupLCDIFClock(UINT32 PixFreq);
// ZXW JUN11-2012
//VOID LCDIFSetupLCDIFClock(UINT32 PixFreq , BOOL bMode );
//
// LQK JUL05-2012: add function to support dot_lcd
//
BOOL LCDIFDisplayFrameBufferEx(const void* pData, int nDataSelect );
#ifdef __cplusplus
}
#endif

#endif // __COMMON_LCDIF_H__
