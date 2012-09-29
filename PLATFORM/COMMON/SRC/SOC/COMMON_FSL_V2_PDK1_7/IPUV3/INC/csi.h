//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  csi.h
//
//  Camera Sensosr interface definitions
//
//-----------------------------------------------------------------------------

#ifndef __CSI_H__
#define __CSI_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

// The data source for CSI
typedef enum csiConnectedDataSourceEnum
{
    csiConnectedDataSource_PARALLEL = 0,
    csiConnectedDataSource_MIPI,
}csiConnectedDataSource;
// The data format from the sensor
typedef enum csiSensorOutputFormatEnum
{
    csiSensorOutputFormat_RGB888,
    csiSensorOutputFormat_YUV444,
    csiSensorOutputFormat_YUV422_YUYV,
    csiSensorOutputFormat_YUV422_UYVY,
    csiSensorOutputFormat_Bayer,
    csiSensorOutputFormat_RGB565,
    csiSensorOutputFormat_RGB555,
    csiSensorOutputFormat_RGB444,
    csiSensorOutputFormat_JPEG,
} csiSensorOutputFormat;

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionEnum
{
    csiSensorOutputResolution_D1_PAL,//720*576
    csiSensorOutputResolution_D1_NTSC,//720*480
    csiSensorOutputResolution_VGA,   //640*480
    csiSensorOutputResolution_QVGA,  //320*240
    csiSensorOutputResolution_CIF,   //352*288
    csiSensorOutputResolution_QCIF,  //176*144
    csiSensorOutputResolution_QQVGA, //160*120
    csiSensorOutputResolution_SVGA,  //800*600
    csiSensorOutputResolution_XGA,   //1024*768
    csiSensorOutputResolution_SXGA,  //1280*1024
    csiSensorOutputResolution_UXGA,  //1600*1200
    csiSensorOutputResolution_QXGA,  //2048*1536
} csiSensorOutputResolution;  

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionWidthEnum
{
    VGA_Width = 640,    //640*480
    QVGA_Width = 320,   //320*240
    CIF_Width = 352,    //352*288
    QCIF_Width = 176,   //176*144
    QQVGA_Width = 160,  //160*120
    SVGA_Width = 800,   //800*600
    XGA_Width = 1024,   //1024*768
    SXGA_Width = 1280,  //1280*1024
    UXGA_Width = 1600,  //1600*1200
    QXGA_Width = 2048,  //2048*1536
    D1_Width = 720, //720*576 720*480
} csiSensorOutputResolutionWidth;  

// The image resolution from the sensor
typedef enum csiSensorOutputResolutionHeightEnum
{
    VGA_Height = 480,    //640*480
    QVGA_Height = 240,   //320*240
    CIF_Height = 288,    //352*288
    QCIF_Height = 144,   //176*144
    QQVGA_Height = 120,  //160*120
    SVGA_Height = 600,   //800*600
    XGA_Height = 768,    //1024*768
    SXGA_Height = 1024,  //1280*1024
    UXGA_Height = 1200,  //1600*1200
    QXGA_Height = 1536,  //2048*1536
    D1_PAL_Height = 576, //720*576
    D1_NTSC_Height = 480, //720*480
} csiSensorOutputResolutionHeight;  

// The data identifier num
typedef enum csiDataIdentifierEnum
{
    CSI_MIPI_DI0,
    CSI_MIPI_DI1,
    CSI_MIPI_DI2,
    CSI_MIPI_DI3,    
}csiDataIdentifier;

//CSI Protocol Mode
typedef enum csiModeEnum
{
    CSI_GATED_CLOCK_MODE = 0,
    CSI_NONGATED_CLOCK_MODE,
    CSI_CCIR_PROGRESSIVE_BT656_MODE,
    CSI_CCIR_INTERLACE_BT656_MODE,
    CSI_CCIR_PROGRESSIVE_BT1120DDR_MODE,
    CSI_CCIR_PROGRESSIVE_BT1120SDR_MODE,
    CSI_CCIR_INTERLACE_BT1120DDR_MODE,
    CSI_CCIR_INTERLACE_BT1120SDR_MODE,
}csiMode;  

//CSI Protocol command for CCIR register
typedef struct
{
    csiMode mode;                          //Sensor protocol. Sensor timing/data mode protocol.
    
    //The fellow fields only used in BT.656 or BT1120 mode
    UINT32  PreCmd;                        //CCIR pre command. This field defines the sequence which comes before the CCIR command.
    UINT8   Field0FirstBlankStartCmd;      //Start of field 0 first blanking line command (interlaces mode). (In progressive mode this field indicates start of blanking line command).
    UINT8   Field0FirstBlankEndCmd;        //End of field 0 first blanking line command (interlaces mode).
    UINT8   Field0SecondBlankStartCmd;     //Start of field 0 second blanking line command (interlaces mode). (In progressive mode this field is ignored).
    UINT8   Field0SecondBlankEndCmd;       //End of field 0 second blanking line command (interlaces mode). (In progressive mode this field is ignored).
    UINT8   Field0ActiveStartCmd;          //Start of field 0 active line command (interlaces mode). (In progressive mode, start of active line command mode).
    UINT8   Field0ActiveEndCmd;            //End of field 0 active line command (interlaces mode). (In progressive mode, end of active line command mode).

    //The fellow fields only used in interlace mode
    UINT8   Field1FirstBlankStartCmd;      //Start of field 1 first blanking line command (interlaces mode). (In progressive mode this field is ignored).
    UINT8   Field1FirstBlankEndCmd;        //End of field 1 first blanking line command (interlaces mode). (In progressive mode this field is ignored).
    UINT8   Field1SecondBlankStartCmd;     //Start of field 1 second blanking line command (interlaces mode). (In progressive mode this field is ignored).
    UINT8   Field1SecondBlankEndCmd;       //End of field 1 second blanking line command (interlaces mode).(In progressive mode this field is ignored).
    UINT8   Field1ActiveStartCmd;          //Start of field 1 active line command (interlaces mode). (In progressive mode this field is ignored).
    UINT8   Field1ActiveEndCmd;            //End of field 1 active line command (interlaces mode). (In progressive mode this field is ignored).
}CSI_PROTOCOL_INF,*PCSI_PROTOCOL_INF;
//------------------------------------------------------------------------------
// Functions

BOOL CSIRegsInit();
void CSIRegsCleanup();
void CSIRegsEnable(CSI_SELECT csi_sel);
void CSIRegsDisable(CSI_SELECT csi_sel);
void CSISetTestMode(CSI_SELECT csi_sel,BOOL bTestMode,UINT8 iRed,UINT8 iGreen,UINT8 iBlue);
BOOL CSISetDataDest(CSI_SELECT csi_sel,UINT8 iDataDest)    ;
BOOL CSISetForceEOF(CSI_SELECT csi_sel,BOOL bForce);
BOOL CSISetSensorPRTCL(CSI_SELECT csi_sel,CSI_PROTOCOL_INF *pPrtclInf);
BOOL CSISetVSYNCMode(CSI_SELECT csi_sel,UINT8 iMode);
BOOL CSISetDataFormat(CSI_SELECT csi_sel,csiSensorOutputFormat outFormat);
void CSISetPolarity(CSI_SELECT csi_sel,BOOL bClkPol,BOOL bDataPol,BOOL bHSYNCPol,BOOL bVSYNCPol,BOOL bDataENPol);
BOOL CSIConfigureFrmSize(CSI_SELECT csi_sel, csiSensorOutputResolution outResolution,csiMode prtclmode);
BOOL CSISetVscHscSkip(CSI_SELECT csi_sel,DWORD dwVscSkip, DWORD dwHscSkip);
BOOL CSISetVscHscDownSize(CSI_SELECT csi_sel,BOOL bVert, BOOL bHorz);
BOOL CSISelectDI(CSI_SELECT csi_sel,UINT8 iDI,UINT8 iDIValue);
void CSIDumpRegs(CSI_SELECT csi_sel);

#ifdef __cplusplus
}
#endif

#endif //__CSI_H__

