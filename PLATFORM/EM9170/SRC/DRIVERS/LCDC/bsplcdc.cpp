//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// FILE:    bsplcdc.c
//
//  This file contains BSP part of the LCDC driver implementation.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include files
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <winddi.h>
#include <gpe.h>
#pragma warning(pop)

#include "common_lcdc.h"
#include "lcdc_mode.h"

#include "bsp.h"
#include "bsplcdc.h"

//------------------------------------------------------------------------------
// Defines
#define VIDEO_REG_PATH                        TEXT("Drivers\\Display\\LCDC")
#define VIDEO_MEM_SIZE                        TEXT("VideoMemSize")
#define DEFAULT_VIDEO_MEM_SIZE                (5*1024*1024) // if can not got from registry, then return 5M bytes for default video RAM size

// CS&ZHL JUN-29-2011: get LCD physical base address
extern UINT32 LCDCGetBaseRegAddr(void);

//------------------------------------------------------------------------------
// Local Variables
static PanelType		dwPanelType;			// LCD Panel identifier, 0 for CHUNGHWA CLAA057VA01CT
static GPEMode*       pGPEModes;				// All supported mode, initialized during driver init
static DWORD           dwGPEModeNum;		// Number of GPE mode, initialized during driver init

//------------------------------------------------------------------------------
// Local Functions
static DWORD  GetDisplayModeFromRegistry(void);
static DWORD  InitializeGPEMode(VOID);
static BOOL   TurnOnLCD(PLCDC_MODE_DESCRIPTOR pModeDesc);
static BOOL   TurnOffLCD(PLCDC_MODE_DESCRIPTOR pModeDesc);

//------------------------------------------------------------------------------
// Description of all supported mode for all supported panel

// Mode id. Use this mode number in ModeArray.GpeMode table entry.
enum 
{
    modeid_1=0,
    modeid_2,
    modeid_3,
    modeid_4,
    modeid_5,
    modeid_6,
    modeid_7
};

// All mode supported description.
LCDC_MODE_DESCRIPTOR ModeArray[] =
{
    {
        DISPLAY_CHUNGHWA_CLAA057VA01CT,
        {modeid_1, 640, 480, 16, 60, gpe16Bpp},
        20,41,6,10,9,7,										//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_16BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },

	{
        DISPLAY_SHARP_LQ057,
        {modeid_1, 320, 240, 16, 60, gpe16Bpp},
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 16;  
		//                                     Hwait1 + 1 = 32; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 32; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 400
		//                                     Vwidth = 2;  
		//                                     Vwait1 = 8;  
		//                                     Vwait2 = 12;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 262
		//
        16, 32, 32, 2, 8, 12,			//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_16BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },

	{
        DISPLAY_CHIMEI_LR430LC9001,
        {modeid_1, 480, 272, 16, 60, gpe16Bpp},
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 37;  
		//                                     Hwait1 + 1 = 4; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 4; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 525
		//                                     Vwidth = 10;  
		//                                     Vwait1 = 2;  
		//                                     Vwait2 = 2;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 286
		//
        37, 4, 4, 10, 2, 2,				//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_16BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },

	{
        DISPLAY_INNOLUX_AT056TN52,
        {modeid_1, 640, 480, 16, 60, gpe16Bpp},
		//
		// CS&ZHL JUN-1-2011: Hwidth + 1 = 64;  
		//                                   Hwait1 + 1 = 40; -> horinzontal front porch (HFP)
		//                                   Hwait2 + 3 = 56; -> horinzontal back porch (HBP)
		//                                   Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 800
		//                                   Vwidth = 2;  
		//                                   Vwait1 = 10;  
		//                                   Vwait2 = 33;  
		//                                   Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 525
		//
        64, 40, 56, 2, 10, 33,		//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,														// Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,				//LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,			//LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_16BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },

	{
        DISPLAY_INNOLUX_AT070TN83V1,
        {modeid_1, 800, 480, 16, 68, gpe16Bpp},
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 48;  
		//                                     Hwait1 + 1 = 40; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 40; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 928
		//                                     Vwidth = 3;  
		//                                     Vwait1 = 13;  
		//                                     Vwait2 = 29;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 525
		//
        48, 40, 40, 3, 13, 29,		//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_16BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },

	{
        DISPLAY_AUO_G084SN03,
        {modeid_1, 800, 600, 16, 50, gpe16Bpp},
		//
		// CS&ZHL JUN-29-2011: Hwidth + 1 = 64;  
		//                                     Hwait1 + 1 = 40; -> horinzontal front porch (HFP)
		//                                     Hwait2 + 3 = 40; -> horinzontal back porch (HBP)
		//                                     Htotal = (Hwait1 + 1) + (Hwidth + 1) + (Hwait2 + 3) + Width = 944
		//                                     Vwidth = 4;  
		//                                     Vwait1 = 1;  
		//                                     Vwait2 = 20;  
		//                                     Vtotal = (Vwait1) + (Vwidth) + (Hwait2) + Height = 625
		//
        64, 40, 40, 4, 1, 20,		//Hwidth, Hwait1, Hwait2, Vwidth, Vwait1, Vwait2
        {
            0,                              // Pixel clock divier compute by common driver layer
            LCDC_PCR_SHARP_DISABLE,          //LCDC_PCR_SHARP_ENABLE
            LCDC_PCR_SCLKSEL_ENABLE,        //LCDC_PCR_SCLKSEL_ENABLE
            0,                              // Not used for TFT panel
            LCDC_PCR_ACDSEL_USE_LPHSYNC,    //LCDC_PCR_ACDSEL_USE_LPHSYNC
            LCDC_PCR_REV_VS_NORMAL,         //LCDC_PCR_REV_VS_NORMAL
            LCDC_PCR_SWAP_SEL_16BPP,        //LCDC_PCR_SWAP_SEL_16BPP
            LCDC_PCR_END_SEL_LITTLE_ENDIAN, //LCDC_PCR_END_SEL_LITTLE_ENDIAN
            LCDC_PCR_SCLKIDLE_ENABLE,      //LCDC_PCR_SCLKIDLE_DISABLE
            LCDC_PCR_OEPOL_ACTIVE_HIGH,     //LCDC_PCR_OEPOL_ACTIVE_HIGH
            LCDC_PCR_CLKPOL_POS_EDGE,       //LCDC_PCR_CLKPOL_NEG_EDGE
            LCDC_PCR_LPPOL_ACTIVE_LOW,   //LCDC_PCR_LPPOL_ACTIVE_LOW
            LCDC_PCR_FLMPOL_ACTIVE_LOW,  //LCDC_PCR_FLMPOL_ACTIVE_HIGH
            LCDC_PCR_PIXPOL_ACTIVE_HIGH,    //LCDC_PCR_PIXPOL_ACTIVE_HIGH
            0,
            LCDC_PCR_PBSIZ_1BIT,            //LCDC_PCR_PBSIZ_8BIT
            LCDC_PCR_COLOR_COLOR,           //LCDC_PCR_COLOR_COLOR
            LCDC_PCR_TFT_ACTIVE             //LCDC_PCR_TFT_ACTIVE
        }
    },

    // Table is terminated by a GPE mode id = -1
    {
        {(DWORD)-1}
    }
    // Put other mode and panel info here in future expansion
    // Caution, modeid in GPEMode must be define once
    // ... ...
};

//--------------------------------------------------------------------------
//
// Function: BSPGetModes
//
// This function return the list of all GPE modes supported by the driver.
//
// Parameters:
//      None.
//
// Returns:
//      GPE modes array.
//
//--------------------------------------------------------------------------
GPEMode * BSPGetModes(VOID)
{
    return pGPEModes;
}

//--------------------------------------------------------------------------
//
// Function: BSPGetModeNum
//
// This function returns the number of display modes supported by a driver. 
//
// Parameters:
//      None.
//
// Returns:
//      Returns the number of display modes supported by a driver.
//
//--------------------------------------------------------------------------
ULONG BSPGetModeNum(void)
{
    // numPanel for all TFT panel supported
    return dwGPEModeNum;
}

//--------------------------------------------------------------------------
//
// Function: BSPGetDisplayMode
//
// This function return the mode descriptor for the coreponding modeid.
//
// Parameters:
//      modeid
//          [in] Modeid of the wanted mode descriptor
//
//      pContext
//          [out] This argument will be filled with LCDC BSP parameters and
//          used by CSP.
//
// Returns:
//      Returns TRUE if the function return a valid context. 
//      Returns FALSE if not.
//
//--------------------------------------------------------------------------
BOOL BSPGetModeDescriptor(int modeid, PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    int i=0;
    PLCDC_MODE_DESCRIPTOR pSelectedMode = NULL;
   
    if(!pModeDesc)
    {
        return FALSE;
    }

    while(ModeArray[i].dwPanelType != -1)
    {
        // If mode id and panel type match
        if (  (ModeArray[i].GPEModeInfo.modeId == modeid) 
           && (ModeArray[i].dwPanelType == (DWORD)dwPanelType))
        {
            pSelectedMode = &ModeArray[i];
            break;
        }
        i++;
    }

    if (pSelectedMode == NULL)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT(" BSPGetModeDescriptor: No mode matching find\r\n")));
        return FALSE;
    }

    // Copy the mode descriptor
    *pModeDesc = *pSelectedMode;

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: BSPInitLCDC
//
// This function initializes the LCDC BSP-secific hardware.
// Read panel type in registy, create the gpe mode supported list for the 
// coresponding panel type and configure the configure the PIO.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE.
//
//------------------------------------------------------------------------------
BOOL BSPInitLCDC(VOID)
{
	RETAILMSG(1, (TEXT("->BSPInitLCDC\r\n")));

    // Get the panel type inregistry
    dwPanelType = (PanelType)GetDisplayModeFromRegistry();

#ifdef	EM9170
	//
	// CS&ZHL JUN-29-2011: get LCD type according to the resolution of splash screen
	//
	{
		PCSP_LCDC_REGS	pLCDC;
		PHYSICAL_ADDRESS phyAddr;
		DWORD						dwLCDSizeReg;
		DWORD						dwWidth, dwHeight;

		//Mem maps the LCDC module space for user access
		phyAddr.QuadPart = LCDCGetBaseRegAddr();
		pLCDC = (PCSP_LCDC_REGS)MmMapIoSpace(phyAddr, sizeof(CSP_LCDC_REGS), FALSE);
		if (pLCDC  ==  NULL)   
		{
			RETAILMSG(1, (TEXT("BSPInitLCDC:: pLCDC map failed!\r\n")));
			return FALSE;
		}
		
		dwLCDSizeReg = INREG32(&pLCDC->SR);	// get current state of LCDC Size Register

		MmUnmapIoSpace(pLCDC, sizeof(CSP_LCDC_REGS));

		dwHeight = dwLCDSizeReg & 0x3FF;
		dwWidth = ((dwLCDSizeReg >> 20) & 0x3F) * 16;
		if((dwWidth == 320) && (dwHeight == 240))
		{
			dwPanelType = DISPLAY_SHARP_LQ057;
			RETAILMSG(1, (TEXT("BSPInitLCDC:: PanelTyepe%d -> 320*240\r\n"), dwPanelType));
		}
		else if((dwWidth == 480) && (dwHeight == 272))
		{
			dwPanelType = DISPLAY_CHIMEI_LR430LC9001;
			RETAILMSG(1, (TEXT("BSPInitLCDC:: PanelTyepe%d -> 480*272\r\n"), dwPanelType));
		}
		else if((dwWidth == 640) && (dwHeight == 480))
		{
			dwPanelType = DISPLAY_INNOLUX_AT056TN52;
			RETAILMSG(1, (TEXT("BSPInitLCDC:: PanelTyepe%d -> 640*480\r\n"), dwPanelType));
		}
		else if((dwWidth == 800) && (dwHeight == 480))
		{
			dwPanelType = DISPLAY_INNOLUX_AT070TN83V1;
			RETAILMSG(1, (TEXT("BSPInitLCDC:: PanelTyepe%d -> 800*480\r\n"), dwPanelType));
		}
		else if((dwWidth == 800) && (dwHeight == 600))
		{
			dwPanelType = DISPLAY_AUO_G084SN03;
			RETAILMSG(1, (TEXT("BSPInitLCDC:: PanelTyepe%d -> 800*600\r\n"), dwPanelType));
		}
		else
		{	// default setting: 640 * 480
			dwPanelType = DISPLAY_INNOLUX_AT056TN52;
			RETAILMSG(1, (TEXT("BSPInitLCDC:: default PanelTyepe%d -> 640*480\r\n"), dwPanelType));
		}
	}
#endif	//EM9170

    // Create the GPEMode table. This table is used by gwes to know all 
    // available mode supported by the driver.
    dwGPEModeNum = InitializeGPEMode();


    //Enable LCDC module pins
    //LCDC_D0
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD0, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD0, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD1, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD1, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD2, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD2, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD3, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD3, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD4, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD4, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD5, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD5, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD6, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD6, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD7, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD7, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD8, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD8, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD9, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD9, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD10, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD10, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD11, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD11, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD12, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD12, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD13, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD13, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD14, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD14, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LD15, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LD15, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_HSYNC, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_HSYNC, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_VSYNC, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_VSYNC, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_LSCLK, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_LSCLK, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_OE_ACD, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_OE_ACD, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CONTRAST, DDK_IOMUX_PIN_MUXMODE_ALT0, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CONTRAST, DDK_IOMUX_PAD_SLEW_FAST, DDK_IOMUX_PAD_DRIVE_NORMAL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

#ifdef	EM9170
    // CS&ZHL JUN-1-2011: LCDC_EN -> LCD_PWR: select GPIO3[20], output, active low
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_CS4, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_CS4, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_1V8);

	// CS&ZHL JUN-1-2011: supporting EM9170
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D15, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D16
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D15, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_D14, DDK_IOMUX_PIN_MUXMODE_ALT1, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D17
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_D14, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

#else	//->iMX257PDK
	// LCDC_EN: control LCD power, active low
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_VSTBY_REQ, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_VSTBY_REQ, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_E, DDK_IOMUX_PIN_MUXMODE_ALT2, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D16
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_E, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPinMux(DDK_IOMUX_PIN_GPIO_F, DDK_IOMUX_PIN_MUXMODE_ALT2, DDK_IOMUX_PIN_SION_REGULAR); // LCD_D17
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_GPIO_F, DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_NORMAL, 
										DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_100K, 
										DDK_IOMUX_PAD_HYSTERESIS_DISABLE, DDK_IOMUX_PAD_VOLTAGE_3V3);
#endif	//EM9170
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: BSPDeinitLCDC
//
// This function deinitiaizes the LCDC BSP-secific hardware.
// This function un-configure the PIO.
//
// Parameters:
//      pContext
//          [in] This argument contains the display context
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
BOOL BSPDeinitLCDC(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);
    
    // Disable LCDC module pins

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: BSPTurnOnDisplay
//
// This function turn on the display peripheral (LCD or TVOut).
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void BSPTurnOnDisplay(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    TurnOnLCD(pModeDesc);
}

//------------------------------------------------------------------------------
//
// Function: BSPTurnOffDisplay
//
// This function turn off the Display (LCD or TVOut).
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void BSPTurnOffDisplay(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    TurnOffLCD(pModeDesc);
}

//------------------------------------------------------------------------------
//
// Function: BSPPowerOnDisplay
//
// This function power on the Display (LCD or TVOut).
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void BSPPowerOnDisplay(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);
}

//------------------------------------------------------------------------------
//
// Function: BSPPowerOffDisplay
//
// This function power off the Display (LCD or TVOut).
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void BSPPowerOffDisplay(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);
}


//------------------------------------------------------------------------------
//
// Function: BSPGetVideoMemorySize
//
// This function can return a specified framebuffer size. If the return is less
// than the minimum requiered size, this value is not used.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor.
//
// Returns:
//      Return wanted memory size.
//
//------------------------------------------------------------------------------
DWORD BSPGetVideoMemorySize(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    BOOL result = TRUE;
    LONG  error;
    HKEY  hKey;
    DWORD dwSize;
    ULONG iAlignment;
    UINT32 nVideoMemorySize = 0;

    UNREFERENCED_PARAMETER(pModeDesc);

    // Open registry key for display driver
    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, VIDEO_REG_PATH, 0 , 0, &hKey);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(1, (TEXT("%s: Failed to open reg path:%s [Error:0x%x]\r\n"), __WFUNCTION__, VIDEO_REG_PATH, error));
        result = FALSE;
        goto _doneVMem;
    }

    // Retrieve Video Memory Size from registry
    dwSize = sizeof(int);
    error = RegQueryValueEx(hKey, VIDEO_MEM_SIZE, NULL, NULL,(LPBYTE)&nVideoMemorySize, (LPDWORD)&dwSize);
    if (error != ERROR_SUCCESS)
    {
        DEBUGMSG(1, (TEXT("DDIPU GetVMemSizeFromRegistry: Failed to get the video RAM size [Error:0x%x].  Setting to default.\r\n"),error));
        result = FALSE;
        nVideoMemorySize = DEFAULT_VIDEO_MEM_SIZE;
        goto _doneVMem;
    }

    // Aligned to 64k bytes
    iAlignment = (1UL << 0xF) - 1;
    nVideoMemorySize = (nVideoMemorySize + iAlignment) & (~iAlignment);

_doneVMem:
    // Close registry key
    RegCloseKey(hKey);

    //DEBUGMSG(1, (TEXT("GetVMemSizeFromRegistry: %s, size is %d!\r\n"), result ? L"succeeds" : L"fails", nVideoMemorySize));
    RETAILMSG(1, (TEXT("GetVMemSizeFromRegistry: %s, size is %d!\r\n"), result ? L"succeeds" : L"fails", nVideoMemorySize));

    return nVideoMemorySize;
}


//------------------------------------------------------------------------------
//
// Function: BSPGetCacheMode
//
// Return whether the Cache mode is cached, write-through
// or non-cached, bufferable.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if cache mode is cached, write-through.
//      FALSE if non-cached, bufferable.
//
//------------------------------------------------------------------------------
BOOL BSPGetCacheMode()
{
    return BSP_VID_MEM_CACHE_WRITETHROUGH;
}

//------------------------------------------------------------------------------
//
// Function: TurnOnLCD
//
// This function enables the LCD display panel by turning on the
// LCD through the Peripheral Bus Controller.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TurnOnLCD(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);

#ifdef	EM9170
	// CS&ZHL JUN-1-2011: use GPIO3_20 as LCD_PWR, active low
    DDKGpioSetConfig(DDK_GPIO_PORT3,20,DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT3,20,0);
#else	//->iMX257PDK
    DDKGpioSetConfig(DDK_GPIO_PORT3,17,DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT3,17,0);
#endif	//EM9170

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: TurnOffLCD
//
// This function disables the display panel by turning off the
// LCD through the Peripheral Bus Controller.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      TRUE if success; FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL TurnOffLCD(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);

#ifdef	EM9170
	// CS&ZHL JUN-1-2011: use GPIO3_20 as LCD_PWR, active low
    DDKGpioSetConfig(DDK_GPIO_PORT3,20,DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT3,20,1);
#else	//->iMX257PDK
    DDKGpioSetConfig(DDK_GPIO_PORT3,17,DDK_GPIO_DIR_OUT, DDK_GPIO_INTR_NONE);
    DDKGpioWriteDataPin(DDK_GPIO_PORT3,17,1);
#endif	//EM9170

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: PowerOnLCD
//
// This function powers on the LCD.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PowerOnLCD(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);
}

//------------------------------------------------------------------------------
//
// Function: PowerOffLCD
//
// This function powers off the LCD.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PowerOffLCD(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);
}


//------------------------------------------------------------------------------
//
// Function: PowerOnTVOut
//
// This function powers on the TV out encoder.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PowerOnTVOut(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);
}


//------------------------------------------------------------------------------
//
// Function: PowerOffTVOut
//
// This function powers off the TV out encoder.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void PowerOffTVOut(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UNREFERENCED_PARAMETER(pModeDesc);
}


//--------------------------------------------------------------------------
//
// Function: GetDisplayModeFromRegistry
//
// This function gets the current panel type from registry.
//
// Parameters:
//      None.
//
// Returns:
//      Panel type number find in registry.
//
//--------------------------------------------------------------------------
DWORD  GetDisplayModeFromRegistry(void) 
{
    LONG result;
    HKEY  hKey = NULL;
    DWORD dwSize;
    DWORD dwPanelTypeReg = PANEL_TYPE_DEFAULT;

    DEBUGMSG(GPE_ZONE_ENTER, (TEXT("+GetDisplayModeFromRegistry()\r\n")));

    // Get LCDC key
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_LCDC_PATH, 0 , 0, &hKey);
    if(result != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT(" GetDisplayModeFromRegistry:Open the RegKey failed\r\n")));
        goto cleanup;
    }

    // Get LCD panel type
    dwSize = sizeof(dwPanelTypeReg);
    result = RegQueryValueEx(hKey, REG_PANEL_TYPE, NULL, NULL,(LPBYTE)&dwPanelTypeReg, 
        (LPDWORD) &dwSize);
    if(result != ERROR_SUCCESS)
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT(" GetDisplayModeFromRegistry: Get PanelType failed\r\n")));
        dwPanelTypeReg = PANEL_TYPE_DEFAULT;
    }

cleanup:
    // Close the video key
    if (hKey != NULL)
        RegCloseKey(hKey);   

    DEBUGMSG(GPE_ZONE_ENTER, (TEXT("-GetDisplayModeFromRegistry()\r\n")));  
    return dwPanelTypeReg;
}

//--------------------------------------------------------------------------
//
// Function: InitializeGPEMode
//
// This function initialize pGPEModes variable with all GPE mode available for
// the selected panel.
//
// Parameters:
//      None.
//
// Returns:
//      Returns the number GPEMode supported for the selected panel.
//
// Note:
//      For future extenstion, TV modes wanted must be added in this GPE 
//      mode list for dynamic mode change.
//
//--------------------------------------------------------------------------
DWORD InitializeGPEMode(VOID)
{
    DWORD dwGPEModeNumber = 0;
    int i=0, j=0;

    // Count number of mode
    while(ModeArray[i].dwPanelType != -1)
    {
        // If panel type match
        if (ModeArray[i].dwPanelType == (DWORD)dwPanelType)
        {
            dwGPEModeNumber++;
        }
        i++;
    }

    // Allocated GPEMode table
    pGPEModes = (GPEMode*)LocalAlloc(LPTR, dwGPEModeNumber * sizeof(GPEMode));

    // Reinit counter
    i=0;
    j=0;

    // Count number of mode
    while(ModeArray[i].dwPanelType != -1)
    {
        // If panel type match
        if (ModeArray[i].dwPanelType == (DWORD)dwPanelType)
        {
            pGPEModes[j] = ModeArray[i].GPEModeInfo;            
            j++;
        }
        i++;
    }

    return dwGPEModeNumber;    
}
