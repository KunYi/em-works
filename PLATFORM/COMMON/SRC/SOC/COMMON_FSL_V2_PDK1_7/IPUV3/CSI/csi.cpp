//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  csi.cpp
//
//  Camera Sensor Interface functions
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"
#include "idmac.h"
#include "cm.h"
#include "csi.h"

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
static HANDLE g_hIPUBase = NULL;
PCSP_IPU_CSI_REGS g_pIPUV3_CSI0;
PCSP_IPU_CSI_REGS g_pIPUV3_CSI1;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: CSIRegsInit
//
// This function initializes the structures needed to access
// the IPUv3 Camera Sensor Interface.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL CSIRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    PHYSICAL_ADDRESS phyAddr;

    if ((g_pIPUV3_CSI0 == NULL) || (g_pIPUV3_CSI1 == NULL))
    {
        //  *** Use IPU_BASE driver to retrieve IPU Base Address ***

        // First, create handle to IPU_BASE driver
        g_hIPUBase = IPUV3BaseOpenHandle();
        if (g_hIPUBase == INVALID_HANDLE_VALUE)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        dwIPUBaseAddr = IPUV3BaseGetBaseAddr(g_hIPUBase);
        if (dwIPUBaseAddr == -1)
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        //********************************************
        // Create pointer to CSI0 registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_CSI0_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_CSI0 = (PCSP_IPU_CSI_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_CSI_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_CSI0 == NULL)
        {
            ERRORMSG(TRUE,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        //********************************************
        // Create pointer to CSI1 registers
        //********************************************
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_CSI1_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_CSI1 = (PCSP_IPU_CSI_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_CSI_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_CSI1 == NULL)
        {
            ERRORMSG(TRUE,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) CSIRegsCleanup();

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  CSIRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 Camera Sensor Interface.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSIRegsCleanup(void)
{
    if (!IPUV3BaseCloseHandle(g_hIPUBase))
    {
        ERRORMSG(TRUE,
            (_T("DI Cleanup: Failed closing IPU Base handle!\r\n")));
    }
    
    // Unmap peripheral registers
    if (g_pIPUV3_CSI0)
    {
        MmUnmapIoSpace(g_pIPUV3_CSI0, sizeof(PCSP_IPU_CSI_REGS));
        g_pIPUV3_CSI0 = NULL;
    }

    // Unmap peripheral registers
    if (g_pIPUV3_CSI1)
    {
        MmUnmapIoSpace(g_pIPUV3_CSI1, sizeof(PCSP_IPU_CSI_REGS));
        g_pIPUV3_CSI1 = NULL;
    }
   
}

//-----------------------------------------------------------------------------
//
// Function: CSIRegsEnable
//
// Enable the CSI0 or CSI1.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CS1 to enable
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSIRegsEnable(CSI_SELECT csi_sel)
{
    if (csi_sel == CSI_SELECT_CSI0)
    {
        // Call to IPU Base to turn on CSI0_EN in IPU_CONF reg.
        // This will also turn on IPU clocks if no other IPU
        // modules have already turned them on.
        if(!IPUV3EnableModule(g_hIPUBase, IPU_SUBMODULE_CSI0, IPU_DRIVER_OTHER))
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Failed to enable CSI0!\r\n"), __WFUNCTION__));
        }
    }
    else
    {
        // Call to IPU Base to turn on CSI1_EN in IPU_CONF reg.
        // This will also turn on IPU clocks if no other IPU
        // modules have already turned them on.
        if(!IPUV3EnableModule(g_hIPUBase, IPU_SUBMODULE_CSI1, IPU_DRIVER_OTHER))
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Failed to enable CSI1!\r\n"), __WFUNCTION__));
        }
    }

}

//-----------------------------------------------------------------------------
//
// Function: CSIRegsDisable
//
// Disable the CSI0 or CSI1.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to disable
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSIRegsDisable(CSI_SELECT csi_sel)
{
    if(csi_sel == CSI_SELECT_CSI0)
    {
        if(!IPUV3DisableModule(g_hIPUBase, IPU_SUBMODULE_CSI0, IPU_DRIVER_OTHER))
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Failed to disable CSI0!\r\n"), __WFUNCTION__));
        }
    }
    else
    {
        if(!IPUV3DisableModule(g_hIPUBase, IPU_SUBMODULE_CSI1, IPU_DRIVER_OTHER))
        {
            ERRORMSG(TRUE,
                (TEXT("%s: Failed to disable CSI1!\r\n"), __WFUNCTION__));
        }
    }
}


//-----------------------------------------------------------------------------
//
// Function: CSISetTestMode
//
// Set the CSI0 or CSI1 to test mode.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to set test mode.
//      bTestMode
//          [in] TRUE for active test mode,FALSE for inactive test mode.
//      iRed
//          [in] Test Pattern generator Red value.
//      iGreen
//          [in] Test Pattern generator Green value.
//      iBlue
//          [in] Test Pattern generator Blue value.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CSISetTestMode(CSI_SELECT csi_sel,BOOL bTestMode,UINT8 iRed,UINT8 iGreen,UINT8 iBlue)
{
    UINT32 sensClkDivider;
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    if(bTestMode)
    {
        // Turn on test pattern generation from the CSI
        INSREG32BF(&csi_regs->CSI_TST_CTRL, 
                   IPU_CSI_TST_CTRL_TEST_GEN_MODE, IPU_CSI_TST_CTRL_TEST_GEN_MODE_ACTIVE); 

        // Only for Test mode using:Get camera divider ratio
        sensClkDivider = 4;
    
        INSREG32BF(&csi_regs->CSI_SENS_CONF, 
           IPU_CSI_SENS_CONF_DIV_RATIO, sensClkDivider);        
            
        RETAILMSG(0,(TEXT("%s:sensClkDivider = %d\r\n"),__WFUNCTION__,sensClkDivider));
    }
    else
    {
        INSREG32BF(&csi_regs->CSI_TST_CTRL,
                   IPU_CSI_TST_CTRL_TEST_GEN_MODE, IPU_CSI_TST_CTRL_TEST_GEN_MODE_INACTIVE);
    }

    // Set test pattern color in CSI
    INSREG32BF(&csi_regs->CSI_TST_CTRL, 
        IPU_CSI_TST_CTRL_PG_R_VALUE, 
        iRed);
    INSREG32BF(&csi_regs->CSI_TST_CTRL, 
        IPU_CSI_TST_CTRL_PG_G_VALUE, 
        iGreen);
    INSREG32BF(&csi_regs->CSI_TST_CTRL, 
        IPU_CSI_TST_CTRL_PG_B_VALUE, 
        iBlue); 

}

//-----------------------------------------------------------------------------
//
// Function: CSISetDataDest
//
// This function Set the data destination from the coming CSI moduel.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to configure.
//      iDataDest
//          [in] The destination of the data coming from CSI.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISetDataDest(CSI_SELECT csi_sel,UINT8 iDataDest)
{  
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    switch(iDataDest)
    {
        case IPU_CSI_SENS_CONF_DATA_DEST_SMFC:
            // Set data destination as SMFC from CSI            
        case IPU_CSI_SENS_CONF_DATA_DEST_ISP:
            // Set data destination as ISP from CSI
        case IPU_CSI_SENS_CONF_DATA_DEST_IC:
            // Set data destination as IC from CSI
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_DEST,
                iDataDest);
            break;

        default:
            ERRORMSG(TRUE,(TEXT("%s: Set data destination is failed!\r\n"),__WFUNCTION__));
            return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CSISetForceEOF
//
// This function force the data end of frame.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to configure.
//      bForce
//          [in] TRUE to force the EOF,FALSE to no action.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISetForceEOF(CSI_SELECT csi_sel,BOOL bForce)
{  
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    if(bForce)
    {
        // Set force end of frame
        INSREG32BF(&csi_regs->CSI_SENS_CONF, 
            IPU_CSI_SENS_CONF_FORCE_EOF, 
            IPU_CSI_SENS_CONF_FORCE_EOF_ENABLE);
    }
    else
    {
        // No action
        INSREG32BF(&csi_regs->CSI_SENS_CONF, 
            IPU_CSI_SENS_CONF_FORCE_EOF, 
            IPU_CSI_SENS_CONF_FORCE_EOF_DISABLE);
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CSISetSensorPRTCL
//
// This function set the sensor protocol.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to configure.
//      iPrtclMode
//          [in] The input mode protocol structure point.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISetSensorPRTCL(CSI_SELECT csi_sel,CSI_PROTOCOL_INF *pPrtclInf)
{  
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    if (pPrtclInf->mode > 7)
    {
        ERRORMSG(TRUE,(TEXT("%s: Set CSI sensor protocol failed\r\n"),__WFUNCTION__));
    }
    
    INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_SENS_PRTCL,pPrtclInf->mode);

    if(pPrtclInf->mode == CSI_CCIR_PROGRESSIVE_BT656_MODE     ||
       pPrtclInf->mode == CSI_CCIR_PROGRESSIVE_BT1120DDR_MODE ||
       pPrtclInf->mode == CSI_CCIR_PROGRESSIVE_BT1120SDR_MODE)
    {
        //Write CCIR Pre Command
        INSREG32BF(&csi_regs->CSI_CCIR_CODE_3, 
                        IPU_CSI_CCIR_CODE_3_CCIR_PRECOM, pPrtclInf->PreCmd);

        //Write CCIR Code 1
        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_1ST, pPrtclInf->Field0FirstBlankStartCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_END_FLD0_ACTV, pPrtclInf->Field0ActiveEndCmd);            

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_ACTV, pPrtclInf->Field0ActiveStartCmd);     
    }

    if(pPrtclInf->mode == CSI_CCIR_INTERLACE_BT656_MODE      ||
       pPrtclInf->mode == CSI_CCIR_INTERLACE_BT1120DDR_MODE  ||
       pPrtclInf->mode == CSI_CCIR_INTERLACE_BT1120SDR_MODE)
    {
        //Write CCIR Pre Command
        INSREG32BF(&csi_regs->CSI_CCIR_CODE_3, 
                        IPU_CSI_CCIR_CODE_3_CCIR_PRECOM, pPrtclInf->PreCmd);

        //Write CCIR Code 1
        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_1ST, pPrtclInf->Field0FirstBlankEndCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_1ST, pPrtclInf->Field0FirstBlankStartCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_2ND, pPrtclInf->Field0SecondBlankEndCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_2ND, pPrtclInf->Field0SecondBlankStartCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_END_FLD0_ACTV, pPrtclInf->Field0ActiveEndCmd);            

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_1, 
                        IPU_CSI_CCIR_CODE_1_STRT_FLD0_ACTV, pPrtclInf->Field0ActiveStartCmd);  
        
        //Write CCIR Code 2
        INSREG32BF(&csi_regs->CSI_CCIR_CODE_2, 
                        IPU_CSI_CCIR_CODE_2_END_FLD1_BLNK_1ST, pPrtclInf->Field1FirstBlankEndCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_2, 
                        IPU_CSI_CCIR_CODE_2_STRT_FLD1_BLNK_1ST, pPrtclInf->Field1FirstBlankStartCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_2, 
                        IPU_CSI_CCIR_CODE_2_END_FLD1_BLNK_2ND, pPrtclInf->Field1SecondBlankEndCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_2, 
                        IPU_CSI_CCIR_CODE_2_STRT_FLD1_BLNK_2ND, pPrtclInf->Field1SecondBlankStartCmd);

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_2, 
                        IPU_CSI_CCIR_CODE_2_END_FLD1_ACTV, pPrtclInf->Field1ActiveEndCmd);            

        INSREG32BF(&csi_regs->CSI_CCIR_CODE_2, 
                        IPU_CSI_CCIR_CODE_2_STRT_FLD1_ACTV, pPrtclInf->Field1ActiveStartCmd);     
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CSISetVSYNCMode
//
// This function set the VSYNC mode.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to configure.
//      iMode
//          [in] VSYNC mode:0 for internal mode, 1 for external mode.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISetVSYNCMode(CSI_SELECT csi_sel,UINT8 iMode)
{  
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    if (iMode > 1)
    {
        ERRORMSG(TRUE,(TEXT("%s: Set CSI VSYNC mode failed\r\n"),__WFUNCTION__));
    }
    
    INSREG32BF(&csi_regs->CSI_SENS_CONF, 
            IPU_CSI_SENS_CONF_EXT_VSYNC, 
            iMode);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CSISetDataFormat
//
// This function set the CSI ouput data format.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to configure.
//      outFormat  
//          [in] Sensor output format
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISetDataFormat(CSI_SELECT csi_sel,csiSensorOutputFormat outFormat)
{
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    // Set CSI configuration parameters and sensor protocol based on the output format.
    switch (outFormat)
    {
        case csiSensorOutputFormat_RGB888:    
        case csiSensorOutputFormat_YUV444:
            
            // Set data format from the sensor.
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_DATA_FORMAT_RGB_YUV444);
            
            break;    
            
        case csiSensorOutputFormat_YUV422_YUYV:

            // Set data format from the sensor.
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_DATA_FORMAT_YUV422_YUYV);

            break;        

        case csiSensorOutputFormat_YUV422_UYVY:
            
            // Set data format from the sensor.
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_DATA_FORMAT_YUV422_UYVY);
        
            break;

        case csiSensorOutputFormat_Bayer:

            // Set data format from the sensor.
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_DATA_FORMAT_BAYER);

            break;

        case csiSensorOutputFormat_RGB565:

            // Set data format from the sensor.
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_DATA_FORMAT_RGB565);
            
            break;

        case csiSensorOutputFormat_RGB555:

            // Set data format from the sensor.
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_DATA_FORMAT_RGB555);
            
            break;

        case csiSensorOutputFormat_JPEG:

            // Set data format from the sensor.
            INSREG32BF(&csi_regs->CSI_SENS_CONF, 
                IPU_CSI_SENS_CONF_DATA_FORMAT, 
                IPU_CSI_SENS_CONF_DATA_FORMAT_JPEG);
            
            break;
            
        default:
            RETAILMSG(0,(TEXT("NO supported Sensor Format ! \r\n")));
            return FALSE;
    }
    
    // Data width should be 8 bits per color, regardless of the format
    INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DATA_WIDTH, 
                IPU_CSI_SENS_CONF_DATA_WIDTH_8BIT);
                
    INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_PACK_TIGHT, 
                IPU_CSI_SENS_CONF_PACK_TIGHT_16bit_WORD);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CSISetPolarity
//
// This function set the CSI polarity.
//
// Parameters:
//      bClkPol
//          [in] TRUE is for DISP_PIX_CLK_INVERT, FALSE is for DISP_PIX_CLK_NO_INVERT.
//      bDataPol
//          [in] TRUE is for DATA_POL_INVERT, FALSE is for DATA_POL_NO_INVERT.
//      bHSYNCPol
//          [in] TRUE is for HSYNC_POL_INVERT, FALSE is for HSYNC_POL_NO_INVERT.
//      bVSYNCPol
//          [in] TRUE is for VSYNC_POL_INVERT, FALSE is for VSYNC_POL_NO_INVERT.
//      bDataENPol
//          [in] TRUE is for DATA_EN_POL_INVERT, FALSE is for DATA_EN_POL_NO_INVERT.
//
// Returns:
//      NULL.
//
//-----------------------------------------------------------------------------
void CSISetPolarity(CSI_SELECT csi_sel,BOOL bClkPol,BOOL bDataPol,BOOL bHSYNCPol,BOOL bVSYNCPol,BOOL bDataENPol)
{
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;
    
    // Set the polarity    
    if (bClkPol)
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DISP_PIX_CLK_POL, 
                        IPU_CSI_SENS_CONF_DISP_PIX_CLK_INVERT);
    else
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DISP_PIX_CLK_POL, 
                        IPU_CSI_SENS_CONF_DISP_PIX_CLK_NO_INVERT);

    if (bDataPol)
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DATA_POL, 
                    IPU_CSI_SENS_CONF_DATA_POL_INVERT);
    else
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DATA_POL, 
                    IPU_CSI_SENS_CONF_DATA_POL_NO_INVERT);

    if (bHSYNCPol)
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_HSYNC_POL, 
                    IPU_CSI_SENS_CONF_HSYNC_POL_INVERT);   
    else
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_HSYNC_POL, 
                    IPU_CSI_SENS_CONF_HSYNC_POL_NO_INVERT); 

    if (bVSYNCPol)
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_VSYNC_POL, 
                    IPU_CSI_SENS_CONF_VSYNC_POL_INVERT);  
    else
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_VSYNC_POL, 
                    IPU_CSI_SENS_CONF_VSYNC_POL_NO_INVERT);

    if (bDataENPol)
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DATA_EN_POL, 
                    IPU_CSI_SENS_CONF_DATA_EN_POL_INVERT);
    else
        INSREG32BF(&csi_regs->CSI_SENS_CONF, IPU_CSI_SENS_CONF_DATA_EN_POL, 
                    IPU_CSI_SENS_CONF_DATA_EN_POL_NO_INVERT);

}

//-----------------------------------------------------------------------------
//
// Function: CSIConfigureFrmSize
//
// This function configures the CSI module frame size.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to config.
//      outResolution
//          [in] Sensor output resolution:QXGA,UXGA,SXGA,XGA,SVGA,VGA,QVGA,QQVGA,CIF,QIF
//                                        D1_PAL,D1_NTSC  
//      prtclmode
//          [in] Input data Protocol Mode
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSIConfigureFrmSize(CSI_SELECT csi_sel, csiSensorOutputResolution outResolution, csiMode prtclmode)
{

    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    UINT32 iInterlaceScale = 1;

    if(prtclmode == CSI_CCIR_INTERLACE_BT656_MODE      ||
       prtclmode == CSI_CCIR_INTERLACE_BT1120DDR_MODE  ||
       prtclmode == CSI_CCIR_INTERLACE_BT1120SDR_MODE)
    {
        //if use interlace mode, we will downsize the image height
        iInterlaceScale = 2;
    }
    
    // Set up CSI width and height configuration parameters and 
    // CSI sensor and actual frame size from the output resolution.
    // Note: Subtract one from frame height and width values 
    // before writing to CSI registers.
    switch (outResolution)
    {
        case csiSensorOutputResolution_QXGA:
            // Set up CSI sensor frame and actual frame for QXGA resolution.
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, QXGA_Width - 1); 
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, QXGA_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, QXGA_Width - 1 );
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, QXGA_Height - 1 );
            break;
            
        case csiSensorOutputResolution_UXGA:
            // Set up CSI sensor frame and actual frame for UXGA resolution.
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, UXGA_Width - 1); 
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, UXGA_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, UXGA_Width - 1 );
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, UXGA_Height - 1 );
            break;

        case csiSensorOutputResolution_SXGA:

            // Set up CSI sensor frame and actual frame for SXGA resolution.
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, SXGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, SXGA_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, SXGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, SXGA_Height - 1);

            break;

        case csiSensorOutputResolution_XGA:

            // Set up CSI sensor frame and actual frame for XGA resolution.
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, XGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, XGA_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, XGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, XGA_Height - 1);

            break;

        case csiSensorOutputResolution_SVGA:

            // Set up CSI sensor frame and actual frame for SVGA resolution.
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, SVGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, SVGA_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, SVGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, SVGA_Height - 1);

            break;

        case csiSensorOutputResolution_VGA:

            // Set up CSI sensor frame and actual frame for VGA resolution
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, VGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, VGA_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, VGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, VGA_Height - 1);

            break;

        case csiSensorOutputResolution_QVGA:

            // Set up CSI sensor frame and actual frame for QVGA resolution
            INSREG32(&csi_regs->CSI_SENS_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH),  
                CSP_BITFVAL(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, QVGA_Width - 1));
                
            INSREG32(&csi_regs->CSI_SENS_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT),  
                CSP_BITFVAL(IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, QVGA_Height - 1));

            INSREG32(&csi_regs->CSI_ACT_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH),  
                CSP_BITFVAL(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, QVGA_Width - 1));
            INSREG32(&csi_regs->CSI_ACT_FRM_SIZE, 
                CSP_BITFMASK(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT),  
                CSP_BITFVAL(IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, QVGA_Height - 1));

            break;

        case csiSensorOutputResolution_CIF:

            // Set up CSI sensor frame and actual frame for CIF resolution
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, CIF_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, CIF_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, CIF_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, CIF_Height - 1);

            break;

        case csiSensorOutputResolution_QCIF:

            // Set up CSI sensor frame and actual frame for QCIF resolution
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, QCIF_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE,
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, QCIF_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, QCIF_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, QCIF_Height - 1);

            break;

        case csiSensorOutputResolution_QQVGA:

            // Set up CSI sensor frame and actual frame for QQVGA resolution
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, QQVGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, QQVGA_Height - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, QQVGA_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, QQVGA_Height - 1);

            break;

        case csiSensorOutputResolution_D1_PAL:

            // Set up CSI sensor frame and actual frame for QQVGA resolution
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, D1_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, D1_PAL_Height/iInterlaceScale - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, D1_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, D1_PAL_Height/iInterlaceScale - 1);

            break;

        case csiSensorOutputResolution_D1_NTSC:

            // Set up CSI sensor frame and actual frame for QQVGA resolution
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH, D1_Width - 1);
            INSREG32BF(&csi_regs->CSI_SENS_FRM_SIZE, 
                IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT, D1_NTSC_Height/iInterlaceScale - 1);

            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE, 
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH, D1_Width - 1);
            INSREG32BF(&csi_regs->CSI_ACT_FRM_SIZE,
                IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT, D1_NTSC_Height/iInterlaceScale - 1);

            break;

        default:
            return FALSE;
    } 

    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: CSISetVscHscSkip
//
// This function set vertical or Horizontal skip.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to config.
//      dwVscSkip
//          [in] The vertical skipped number of rows.
//      dwHscSkip
//          [in] The horizontal skipped number of columns.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISetVscHscSkip(CSI_SELECT csi_sel,DWORD dwVscSkip, DWORD dwHscSkip)
{
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;
    
    //Vertical skip dwVscSkip number of rows          
    INSREG32BF(&csi_regs->CSI_OUT_FRM_CTRL, IPU_CSI_OUT_FRM_CTRL_VSC, 
                dwVscSkip);

    //Horizontal skip dwHscSkip number of columns
    INSREG32BF(&csi_regs->CSI_OUT_FRM_CTRL, IPU_CSI_OUT_FRM_CTRL_HSC, 
                dwHscSkip); 

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: CSISetDownSize
//
// This function set CSI downsize for horizotal or vertical.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to config.
//      bVert
//          [in] TRUE for enable vertical downsize by 2.FALSE for disable.
//      bHorz
//          [in] TRUE for enable horz downsize by 2.FALSE for disable.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISetVscHscDownSize(CSI_SELECT csi_sel,BOOL bVert, BOOL bHorz)
{
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;
    
    // Vertical     
    if(bVert)
    {
        INSREG32BF(&csi_regs->CSI_OUT_FRM_CTRL, IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS, 
                    IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_ENABLE);
    }
    else
    {
        
        INSREG32BF(&csi_regs->CSI_OUT_FRM_CTRL, IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS, 
                    IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_DISABLE);
    }

    // horizontal
    if(bHorz)
    {
        INSREG32BF(&csi_regs->CSI_OUT_FRM_CTRL, IPU_CSI_OUT_FRM_CTRL_VERT_DWNS, 
                    IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_ENABLE);
    }
    else
    {
        INSREG32BF(&csi_regs->CSI_OUT_FRM_CTRL, IPU_CSI_OUT_FRM_CTRL_VERT_DWNS, 
                    IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_DISABLE);

    }
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CSISelectDI
//
// This function configure the Data Identifier handled by CSI.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to config.
//      iDI
//          [in] The data identifier:0~3.
//      iDIValue
//          [in] The data identifier value.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CSISelectDI(CSI_SELECT csi_sel,UINT8 iDI,UINT8 iDIValue)
{
    PCSP_IPU_CSI_REGS csi_regs = (csi_sel == CSI_SELECT_CSI0) ? g_pIPUV3_CSI0 : g_pIPUV3_CSI1;

    switch(iDI)
    {
        case CSI_MIPI_DI0:
            INSREG32BF(&csi_regs->CSI_DI, IPU_CSI_DI_MIPI_DI0, 
                            iDIValue); 
            break;
            
        case CSI_MIPI_DI1:
            INSREG32BF(&csi_regs->CSI_DI, IPU_CSI_DI_MIPI_DI1, 
                            iDIValue); 
            break;
            
        case CSI_MIPI_DI2:
            INSREG32BF(&csi_regs->CSI_DI, IPU_CSI_DI_MIPI_DI2, 
                            iDIValue); 
            break;
            
        case CSI_MIPI_DI3:
            INSREG32BF(&csi_regs->CSI_DI, IPU_CSI_DI_MIPI_DI3, 
                            iDIValue); 
            break;

        default:
            ERRORMSG(TRUE,(TEXT("%s:CSI Set Data identifier failed\r\n"),__WFUNCTION__));
            return FALSE;
    }
    
    return TRUE;
}

void CSIDumpRegs(CSI_SELECT csi_sel)
{
    int i,num;
    UINT32* ptr;

    if (csi_sel == CSI_SELECT_CSI0)
    {
        ptr = (UINT32 *)&g_pIPUV3_CSI0->CSI_SENS_CONF;
        num = 0;
    }
    else
    {
        ptr = (UINT32 *)&g_pIPUV3_CSI1->CSI_SENS_CONF;
        num = 1;
    }

    RETAILMSG (1, (TEXT("\n\nCSI %d Registers\n"),num));
    for (i = 0; i <= (sizeof(CSP_IPU_CSI_REGS) / 16); i++)
    {
        RETAILMSG (1, (TEXT("Address %08x  %08x %08x %08x %08x\n"), (UINT32)ptr, INREG32(ptr), INREG32(ptr+1), INREG32(ptr+2), INREG32(ptr+3)));
        ptr += 4;
    }
}
