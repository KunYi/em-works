//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CsiClass.cpp
//
//  Implementation of CMOS Sensor Interface Driver class
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
#include  "ipu_common.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "csi.h"
#include "CsiClass.h"
#include "cameradbg.h"
#include "cm.h"
#pragma warning(disable: 4100 4701)

//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPCSI0IOMUXConfig();
extern BOOL BSPCSI1IOMUXConfig();
extern void BSPSetupCamera(CSI_SELECT csi_sel, DWORD dwFramerate);
extern void BSPInitializeHSC();
extern void BSPDeleteCamera(CSI_SELECT csi_sel);
extern void BSPEnableCamera(CSI_SELECT csi_sel);
extern void BSPDisableCamera(CSI_SELECT csi_sel);
extern UINT32 BSPGetSensorClockRatio();
extern void BSPSetDigitalZoom(DWORD);
extern BOOL BSPCameraSetOutputResolution(DWORD dwFramerate, csiSensorOutputResolution);
extern BOOL BSPCameraSetOutputFormat(DWORD dwFramerate, csiSensorOutputFormat);

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: CsiClass
//
// CsiClass constructor.  Calls CsiInit to initialize module.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to initialize.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
CsiClass::CsiClass(CSI_SELECT csi_sel)
{
    CsiInit(csi_sel);
}

//-----------------------------------------------------------------------------
//
// Function: ~CsiClass
//
// CsiClass destructor.  Calls CsiDeinit to deinitialize module.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
CsiClass::~CsiClass()
{
    CsiDeinit();
}

//-----------------------------------------------------------------------------
//
// Function: CsiInit
//
// This function initializes the Camera Sensor Interface and
// Image Converter modules.
//
// Parameters:
//      csi_sel
//          [in] Selects either the CSI0 or CSI1 to initialize.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
void CsiClass::CsiInit(CSI_SELECT csi_sel)
{    
    CSI_FUNCTION_ENTRY();

    // Set CSI module interface is CSI0 or CSI1.    
    m_CsiSel = csi_sel;

    m_OutFormat = csiSensorOutputFormat_RGB888;
    m_OutResolution = csiSensorOutputResolution_QXGA;
    m_dwFramerate = 0;

    m_bCsi0Disable = TRUE;
    m_bCsi1Disable = TRUE;

    if(!CSIRegsInit())
    {
        ERRORMSG (TRUE, (TEXT("%s: Failed to enable CSI Regs!\r\n"), __WFUNCTION__));
        goto Error;
    }

    // Configure IOMUX pins for CSI0 interface
    if (csi_sel == CSI_SELECT_CSI0)
    {
        if (!BSPCSI0IOMUXConfig())
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Error configuring IOMUX for CSI.\r\n"), __WFUNCTION__));
            goto Error;
        }
    }
    else// Configure IOMUX pins for CSI1 interface
    {
        if (!BSPCSI1IOMUXConfig())
        {
            DEBUGMSG(ZONE_ERROR, 
                (TEXT("%s: Error configuring IOMUX for CSI.\r\n"), __WFUNCTION__));
            goto Error;
        }
    }

    CSI_FUNCTION_EXIT();

    return;

Error:
    CsiDeinit();
    return;
}

//-----------------------------------------------------------------------------
//
// Function: CsiDeinit
//
// This function deinitializes the Camera Sensor Interface module.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiDeinit()
{
    CSI_FUNCTION_ENTRY();

    if(m_CsiSel == CSI_SELECT_CSI0)
    {
        // Disable CSI0
        CsiDisable();
    }
    else
    {
        // Disable CSI1
        CsiDisable(); 
    }

    CSIRegsCleanup();
    
    BSPDeleteCamera(m_CsiSel);

    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiEnable
//
// Enable the Camera Sensor Interface.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiEnable()
{
    CSI_FUNCTION_ENTRY();

    IPU_SUBMODULE csi_module = (m_CsiSel == CSI_SELECT_CSI0) ? IPU_SUBMODULE_CSI0 : IPU_SUBMODULE_CSI1;

    // Enable CSI
    if (csi_module == IPU_SUBMODULE_CSI0)
    {
        if(!m_bCsi0Disable)
        {
            CSI_FUNCTION_ENTRY();
            return;
        }
    }
    else
    {
        if(!m_bCsi1Disable)
        {
            CSI_FUNCTION_ENTRY();
            return;
        }
    }

    CSIRegsEnable(m_CsiSel);

    if (csi_module == IPU_SUBMODULE_CSI0)
    {
        m_bCsi0Disable = FALSE;
        DEBUGMSG(ZONE_FUNCTION, (_T("Enabling CSI0\r\n")));
    }
    else
    {
        m_bCsi1Disable = FALSE;
        DEBUGMSG(ZONE_FUNCTION, (_T("Enabling CSI1\r\n")));
    }
       
    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiDisable
//
// Disable the Camera Sensor Interface.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiDisable()
{
    CSI_FUNCTION_ENTRY();

    IPU_SUBMODULE csi_module = (m_CsiSel == CSI_SELECT_CSI0) ? IPU_SUBMODULE_CSI0 : IPU_SUBMODULE_CSI1;

    if (csi_module == IPU_SUBMODULE_CSI0)
    {
        if(m_bCsi0Disable)
        {
            CSI_FUNCTION_EXIT();
            return;
        }
    }
    else
    {
        if(m_bCsi1Disable)
        {
            CSI_FUNCTION_EXIT();
            return;
        }  
    }
        
    // Disable CSI

    CSIRegsDisable(m_CsiSel);

    if (csi_module == IPU_SUBMODULE_CSI0)
    {
        m_bCsi0Disable = TRUE;
        DEBUGMSG(ZONE_FUNCTION, (_T("Disabling CSI0\r\n")));
    }
    else
    {
        m_bCsi1Disable = TRUE;    
        DEBUGMSG(ZONE_FUNCTION, (_T("Disabling CSI1\r\n")));
    }
    
    CSI_FUNCTION_EXIT();
}



//-----------------------------------------------------------------------------
//
// Function: CsiConfigure
//
// This function configures the CSI module.
//
// Parameters:
//      outFormat  
//          [in] Sensor output format
//      outResolution
//          [in] Sensor output resolution:VGA,QVGA,QQVGA,CIF,QIF
//      bTestMode
//          [in] TRUE to set test mode active,FALSE to inactive.
//
// Returns:
//      TRUE if success
//      FALSE if failure
//
//-----------------------------------------------------------------------------
BOOL CsiClass::CsiConfigure(csiSensorOutputFormat outFormat, csiSensorOutputResolution outResolution, CSI_PROTOCOL_INF *pPrtclInf, BOOL bTestMode)
{   
    CSI_FUNCTION_ENTRY();

    //-------------------------------------------------------------
    // Set sensor format and resolution
    //-------------------------------------------------------------
    // Set the output resolution for the BSP-specific camera module.

    if (!BSPCameraSetOutputResolution(m_dwFramerate,outResolution)) 
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: Invalid camera sensor output resolution.\r\n"), __WFUNCTION__));
        CSI_FUNCTION_EXIT();
        return FALSE;
    }

    m_OutResolution = outResolution;

    // Set the output format for the BSP-specific camera module.
    if (!BSPCameraSetOutputFormat(m_dwFramerate,outFormat)) 
    {
        DEBUGMSG(ZONE_ERROR, (_T("%s: Invalid camera sensor output format.\r\n"), __WFUNCTION__));
        CSI_FUNCTION_EXIT();
        return FALSE;
    }

    m_OutFormat = outFormat;

    //-------------------------------------------------------------
    // Config CSI setting
    //-------------------------------------------------------------
    // Set CSI data source:parallel
    CsiSourceSel(csiConnectedDataSource_PARALLEL);

    // Set data destination from CSI
    CSISetDataDest(m_CsiSel,IPU_CSI_SENS_CONF_DATA_DEST_SMFC);

    // Set force end of frame
    CSISetForceEOF(m_CsiSel,TRUE);

    if(bTestMode)
    {   
        // Set CSI Test mode active
        CSISetTestMode(m_CsiSel,bTestMode,0xff,0,0);
    
        // Set internal VSYNC mode.
        CSISetVSYNCMode(m_CsiSel,IPU_CSI_SENS_CONF_EXT_VSYNC_INTERNAL);

        // Set output data format as RGB
        CSISetDataFormat(m_CsiSel,csiSensorOutputFormat_RGB888);

        // Set Polarity
        CSISetPolarity(m_CsiSel,FALSE,FALSE,FALSE,FALSE,FALSE);

        // Set timing and data protocol to Un-Gated mode.
        // if use test mode, force change protocol mode to Un-Gate mode
        pPrtclInf->mode = CSI_NONGATED_CLOCK_MODE;
        CSISetSensorPRTCL(m_CsiSel, pPrtclInf);
    }
    else
    {
        // Set CSI Test mode inactive
        CSISetTestMode(m_CsiSel,bTestMode,0,0,0);
        
        // Set external VSYNC mode.
        CSISetVSYNCMode(m_CsiSel,IPU_CSI_SENS_CONF_EXT_VSYNC_EXTERNAL);

        // Set output data format
        CSISetDataFormat(m_CsiSel,outFormat);

        // Set Polarity
        CSISetPolarity(m_CsiSel,FALSE,FALSE,FALSE,FALSE,FALSE);

        // Set timing and data protocol
        // If Gated mode and Non-Gated mode, only check if set the parameter:
            //mode
                //CSI_GATED_CLOCK_MODE ,CSI_NONGATED_CLOCK_MODE,
        // If CCIR mode, please check the folloing parameters whether setting
            //mode
                //CSI_CCIR_PROGRESSIVE_BT656_MODE,
                //CSI_CCIR_INTERLACE_BT656_MODE,
                //CSI_CCIR_PROGRESSIVE_BT1120DDR_MODE,
                //CSI_CCIR_PROGRESSIVE_BT1120SDR_MODE,
                //CSI_CCIR_INTERLACE_BT1120DDR_MODE,
                //CSI_CCIR_INTERLACE_BT1120SDR_MODE,
            //PreCmd;                        
            //Field0FirstBlankStartCmd;      
            //Field0FirstBlankEndCmd;        
            //Field0SecondBlankStartCmd;     
            //Field0SecondBlankEndCmd;       
            //Field0ActiveStartCmd;          
            //Field0ActiveEndCmd;
        CSISetSensorPRTCL(m_CsiSel,pPrtclInf);
    }

    // Set data output resolution
    CSIConfigureFrmSize(m_CsiSel,outResolution,pPrtclInf->mode);

    // Select CSIn_DI
    CSISelectDI(m_CsiSel,CSI_MIPI_DI0,0xff);

    // Configure CSIn_OUT_FRM_CTRL            
    CSISetVscHscSkip(m_CsiSel,0,0);
    CSISetVscHscDownSize(m_CsiSel,FALSE,FALSE);

    CSI_FUNCTION_EXIT();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: CsiZoom
//
// This function sets the camera zoom value.
//
// Parameters:
//      zoomVal
//          [in] zoom value.  
//                If 2, zoom by 2x.  If 1, zoom by 1x.  
//                All other values are invalid.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiZoom(DWORD zoomVal)
{
    CSI_FUNCTION_ENTRY();


    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiSourceSel
//
// This function sets CSI data source:parallel or MIPI.
//
// Parameters:
//      csi_sourcesel
//          [in] Selects data source:MIPI or Parellel.  
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiSourceSel(csiConnectedDataSource csi_sourcesel)
{
    CSI_FUNCTION_ENTRY();

    if (m_CsiSel == CSI_SELECT_CSI0)
    {
        switch(csi_sourcesel)
        {
            case csiConnectedDataSource_MIPI:

                CMSetCSIDataSource(CSI_SELECT_CSI0, IPU_IPU_CONF_CSI0_DATA_SOURCE_MIPI);
                break;
                
            case csiConnectedDataSource_PARALLEL :

                CMSetCSIDataSource(CSI_SELECT_CSI0, IPU_IPU_CONF_CSI0_DATA_SOURCE_PARALLEL);
                default:
                break;
        }
    }
    else
    {
        switch(csi_sourcesel)
        {
            case csiConnectedDataSource_MIPI:

                CMSetCSIDataSource(CSI_SELECT_CSI1, IPU_IPU_CONF_CSI1_DATA_SOURCE_MIPI);
                break;
                
            case csiConnectedDataSource_PARALLEL :

                CMSetCSIDataSource(CSI_SELECT_CSI1, IPU_IPU_CONF_CSI1_DATA_SOURCE_PARALLEL);
                default:
                break;
        }
    }

    CSI_FUNCTION_EXIT();
}


//-----------------------------------------------------------------------------
//
// Function: CsiConfigureSensor
//
// This function configures the camera sensor registers and HSC module.
//
// Parameters:
//      dwFramerate
//          [in] Sensor frame rate
//
// Returns:
//      NULL
//
//-----------------------------------------------------------------------------
void CsiClass::CsiConfigureSensor(DWORD dwFramerate)
{    
    CSI_FUNCTION_ENTRY();

    // Initialize sensor setting: set sensor output framerate as 30fps
    m_dwFramerate = dwFramerate;
    BSPSetupCamera(m_CsiSel,m_dwFramerate);

    // For the issue:cannot initialize sensor register every time
    // If Initialize sensor,the Format and Resolution will be set to 
    // RGB888 and QXGA.
    m_OutFormat = csiSensorOutputFormat_RGB888;
    m_OutResolution = csiSensorOutputResolution_QXGA;
    
    // Set HSC
    BSPInitializeHSC();

    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiChangeFrameRate
//
// This function change the sensor frame rate.
//
// Parameters:
//      rate
//          [in] Wanted sensor frame rate.
//
// Returns:
//      CSI module interface: CSI0 or CSI1.
//
//-----------------------------------------------------------------------------
void CsiClass::CsiChangeFrameRate(DWORD rate)
{
    CSI_FUNCTION_ENTRY();

    if (rate == m_dwFramerate)
        return;

    
    m_dwFramerate = rate;

    BSPSetupCamera(m_CsiSel,m_dwFramerate);

    CSI_FUNCTION_EXIT();
}

//-----------------------------------------------------------------------------
//
// Function: CsiGetModuleInterface
//
// This function get the CSI interface.
//
// Parameters:
//      None.
//
// Returns:
//      CSI module interface: CSI0 or CSI1.
//
//-----------------------------------------------------------------------------
CSI_SELECT CsiClass::CsiGetModuleInterface()
{
    return m_CsiSel;
}


void CsiClass::DumpCSIRegs(void)
{
    CSIDumpRegs(m_CsiSel);
}

