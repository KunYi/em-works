//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  tve_sdk.h
//
//  Provides definitions for the TV encoder chip.
//
//------------------------------------------------------------------------------

#ifndef __TVE_SDK_H
#define __TVE_SDK_H

#if __cplusplus
extern "C" {
#endif

#define TVE_IOCTL_INIT                       CTL_CODE(FILE_DEVICE_UNKNOWN, 8000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_DEINIT                     CTL_CODE(FILE_DEVICE_UNKNOWN, 8001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_ENABLE                     CTL_CODE(FILE_DEVICE_UNKNOWN, 8002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_DISABLE                    CTL_CODE(FILE_DEVICE_UNKNOWN, 8003, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define TVE_IOCTL_SET_CC_FIELDS              CTL_CODE(FILE_DEVICE_UNKNOWN, 8004, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define TVE_IOCTL_SET_CGMS_WSS_NTSC          CTL_CODE(FILE_DEVICE_UNKNOWN, 8005, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_SET_CGMS_WSS_PAL           CTL_CODE(FILE_DEVICE_UNKNOWN, 8006, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define TVE_IOCTL_SET_OUTPUT_MODE            CTL_CODE(FILE_DEVICE_UNKNOWN, 8008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_SET_OUTPUT_STD_TYPE        CTL_CODE(FILE_DEVICE_UNKNOWN, 8009, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_SET_OUTPUT_RES_SIZE_TYPE   CTL_CODE(FILE_DEVICE_UNKNOWN, 8010, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define TVE_IOCTL_GET_OUTPUT_MODE            CTL_CODE(FILE_DEVICE_UNKNOWN, 8011, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_GET_OUTPUT_STD_TYPE        CTL_CODE(FILE_DEVICE_UNKNOWN, 8012, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TVE_IOCTL_GET_OUTPUT_RES_SIZE_TYPE   CTL_CODE(FILE_DEVICE_UNKNOWN, 8013, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef enum {
    TV_STAND_NTSC = 0,
    TV_STAND_PALM,
    TV_STAND_PALN,  // Combination PLAN
    TV_STAND_PAL,   // Normal PAL (B,D,G,H,I)
    TV_STAND_SECAM,
} TVE_TV_STAND;


typedef enum {
    TV_720X480_D1 = 0,   
    TV_720X576_D1 = 1,   
    TV_1280X720_720PI = 2, 
    TV_1920X1080_1080PI = 3, 
} TVE_TV_RES_SIZE;  // TV resolution size

typedef enum {
    TV_OUT_DISABLE = 0,                    // 0: TVE Standby
    TV_OUT_COMPOSITE_CH0,                  // 1: TVE Composite on Channel #0
    TV_OUT_COMPOSITE_CH2,                  // 2: TVE Composite on Channel #2
    TV_OUT_COMPOSITE_CH0_CH2,              // 3: TVE Composite on Channel #0 and #2
    TV_OUT_SVIDEO_CH0_CH1,                 // 4: TVE SVideo on Channel #0 and #1          
    TV_OUT_SVIDEO_CH0_CH1_COMPOSITE_CH2,   // 5: TVE S-video (on Channel #0 and #1) & Composite (on Channel #2)  
    TV_OUT_COMPONENT_YPRPB,                // 6: TVE Component YPrPb on Channel #0, #1, and #2.   
    TV_OUT_COMPONENT_RGB,                  // 7: TVE Component RGB on Channel #0, #1, and #2.                   
} TVE_TV_OUT_MODE;

// The Closed Caption data bits [6:0] defines "Non-display Control Byte", while CC data bits [14:8] defines
// "Display Control Byte". For each bit details, please refer to EIA-608 Closed Captioning specification.
//
typedef struct
{
    UINT32 cc_data_low:7;  // closed caption data bits [6:0]
    UINT32 reserved:1;     // must set to 0
    UINT32 cc_data_high:7; // closed caption data bits [14:8]
    UINT32 dummy:17;       // Dummy variable for alignment
} TVECC, *PTVECC;

#pragma warning( disable : 4201 ) // nonstandard extension used : nameless struct/union
// If the TVE CC is in the progress mode, please set cc_f1_odd_field and set 0 to cc_f2_even_field.
// If the TVE CC is in the interleaved mode, please set both of cc_f1_odd_field and cc_f2_even_field.
typedef struct
{   
        TVECC  cc_f1_odd_field_data; 
        TVECC  cc_f2_even_field_data;
    
} TVECCInfo, *PTVECCInfo;
#pragma warning( default : 4201 )

// 625i Systems. 625i (576i) systems are based on ITU-R BT.1119 and ETSI EN 300 294. The Analogy Copy
// Generation Management System (CGMS-A) is also supported by the WSS signal. 
// For (B, D, G, H, I, N, NC) PAL, WSS data is normally on line 23. Please refer to ITU-R BT.1119 and 
// ETSI EN 300 294 specifications for details.
//
typedef struct 
{
    UINT32 aspect_ratio:4;    // aspect ratio:
                              //    b3 b2 b1 b0 | Aspect Ration Label             |Position
                              //    ------------|---------------------------------|-----------
                              //    1  0  0  0  | full format 4:3                 |
                              //    0  0  0  1  | box 14:9                        |centre
                              //    0  0  1  0  | box 14:9                        |top
                              //    1  0  1  1  | box 16:9                        |centre
                              //    0  1  0  0  | box 16:9                        |top
                              //    1  1  0  1  | box > 16:9                      |centre
                              //    1  1  1  0  | full format 4:3                 |centre
                              //    0  1  1  1  | full format 16:9 (anamorphic)   |  
                              // 
    UINT32 film:1;            // b4, 0 -- camera mode; 1 -- film mode
    UINT32 colour_coding:1;   // b5, 0 -- standard coding; 1 -- motion adaptive color plus
    UINT32 helper:1;          // b6, 0 -- no helper; 1 -- modulated helper
    UINT32 reserved:1;        // b7, should be set to "0"
    UINT32 subtitle:1;        // b8, 0 -- no subtitles within Teletext; 1 -- subtitles within Teletext
    UINT32 subtitle_mode:2;   // subtitle mode:
                              //    b10 b9      |  Subtitle in/out of active image area
                              //    ------------|--------------------------------------
                              //    0  0        |  no open subtitles
                              //    0  1        |  subtitles in active image area
                              //    1  0        |  subtitles out of active image area
                              //    1  1        |  reserved 
    UINT32 surround_sound:1;  // b11, 0 -- no surround sound info; 1 -- surround sound mode
    UINT32 copyright:1;       // b12, 0 -- no copyright asserted or status unkown; 1 -- copyright asserted    
    UINT32 generation:1;      // b13 (copy protection), 0 -- copying not restricted; 1 -- copying restricted
    UINT32 crc:6;             // b19-b14 -- CRC controlled by software
    UINT32 dummy:12;          // Dummy variable for alignment.
    
} TVECgmsWssPAL625, *PTVECgmsWssPAL625;

// 626-line PAL/SECAM will use CGMS_WSS_CONT_REG_0. (one line per frame of CGMS data : line 23)  
typedef union
{
    TVECgmsWssPAL625 tveCgmsWssPAL625;
    UINT32           tveCgmsWssF1Data;

} TVECgmsWssPAL625Info, *PTVECgmsWssPAL625Info;


// 525i Systems. EIA-J CPR-1204 and IEC 61880 define a widescreen signalling standard for 525i (480i) systems.
// Lines 20 and 283 are used to transmit the WSS info. Please refer to the EIA-J CPR-1204 and IEC 61880 
// specifications for details.
//
typedef struct 
{
    UINT32 aspect_ratio:2;          // b1 b0  | means              | comments
                                    // -------|--------------------|-------------
                                    // 0  0   | 4:3 aspect ratio   | normal
                                    // 0  1   | 16:9 aspect ration | anamorphic
                                    // 1  0   | 4:3 aspect ratio   | letterbox
                                    // 1  1   | reserved           |
    
    UINT32 transmit_copy_control:4; // b5 b4 b3 b2 |
                                    //-------------|-----------------------------
                                    // 0  0   0  0 |      
                                    // 1  1   1  1 |
                                    //
                                    // Note: copy control info is transmitted in b7b6 when b5b4b3b2 is "0000".
                                    // When copy control info is not to be transferred, b5b4b3b2 data must be set 
                                    // to the default value "1111".
    
    UINT32 copy_control:2;          // b7 b6 | means
                                    // ------|--------------------------
                                    // 0  0  | copying permitted
                                    // 0  1  | one copy permitted
                                    // 1  0  | reserved
                                    // 1  1  | no copying permitted
                                 
    UINT32 copy_protection_reserved:2;   // b9 b8
    UINT32 analog_prerecorded_medium:1;     // b10, 0 -- not analog pre-recorded medium; 1 -- analog pre-recorded medium
    
    UINT32 reserved:3;                      // Bits b13, b12, b11 are reserved and are "000".
    UINT32 crc:6;                           // b19 - b14 -- CRC controlled by software
    UINT32 dummy:12;                        // Dummy variable for alignment.
    
} TVECgmsWssNTSC525, *PTVECgmsWssNTSC525;

#pragma warning( disable : 4201 ) // nonstandard extension used : nameless struct/union
// 525-line  NTSC will use CGMS_CONT_REG_1 and CGMS_WSS_CONT_REG_0. 
typedef struct
{   union {
        TVECgmsWssNTSC525 tveCgmsWssF1NTSC525;
        UINT32            tveCgmsWssF1Data;
    }; 
    
    union {
        TVECgmsWssNTSC525 tveCgmsF2NTSC525;
        UINT32            tveCgmsF2Data;
    };
    
} TVECgmsWssNTSC525Info, *PTVECgmsWssNTSC525Info;
#pragma warning( default : 4201 )



// TVE Output Mode Parameter
typedef struct
{
    UINT16 iTVOutputMode;
} TVEOutputModeInfo, *PTVEOutputModeInfo;

// TVE Output Standard Parameter
typedef struct
{
    UINT16 iTVOutputStd;
} TVEOutputStdInfo, *PTVEOutputStdInfo;

// TVE Output Resuolution Size Parameter
typedef struct
{
    UINT16 iTVOutputResSize;
} TVEOutputResSizeInfo, *PTVEOutputResSizeInfo;

HANDLE TVEOpenHandle(void);
BOOL   TVECloseHandle(HANDLE hTVE);
BOOL   TVEEnable(HANDLE hTVE);
BOOL   TVEDisable(HANDLE hTVE);
BOOL   TVESetClosedCaptionFields(HANDLE hTVE, TVECCInfo *pCCInfo);
BOOL   TVESetCgmsWssPAL(HANDLE hTVE, TVECgmsWssPAL625Info *pCgmsWssInfo);
BOOL   TVESetCgmsWssNTSC(HANDLE hTVE, TVECgmsWssNTSC525Info *pCgmsInfo);
BOOL   TVESetOutputMode(HANDLE hTVE, TVEOutputModeInfo *pOutputModeInfo);
BOOL   TVESetOutputStdType(HANDLE hTVE, TVEOutputStdInfo *pOutputStdInfo);
BOOL   TVESetOutputResSizeType(HANDLE hTVE, TVEOutputResSizeInfo *pOutputResSizeInfo);

TVE_TV_OUT_MODE  TVEGetOutputMode(HANDLE hTVE); 
TVE_TV_STAND     TVEGetOutputStdType(HANDLE hTVE);
TVE_TV_RES_SIZE  TVEGetOutputResSizeType(HANDLE hTVE);

#if __cplusplus
    }
#endif

#endif // __TVE_SDK_H


