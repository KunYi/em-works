/*
================================================================================
*             Texas Instruments AM389X(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#include <windows.h>
#include <oal_log.h>
#include <ceddk.h>
#include <devload.h>
#include <nkintr.h>
#include <windev.h>

#include "ti81xxhdmi.h"
#include "ti81xx_hdmi_cfg.h"

//------------------------------------------------------------------------------
//  Set debug zones names and initial setting for the EDMA driver. 
//
#define ZONE_ERROR          1
#define ZONE_WARN           1
#define ZONE_FUNCTION       0
#define ZONE_INIT           0
#define ZONE_INFO           0
#define ZONE_IST            0
#define ZONE_IOCTL          0
#define ZONE_VERBOSE        0

#define TI81XX_HDMI_DRIVER_NAME     "TI81XX_HDMI"

struct ti81xx_hdmi_params
{
	/* Handle to library */
	void* hdmi_lib_handle;
	/* Other parameters */
	UINT32 wp_v_addr;
	UINT32 core_v_addr;
	UINT32 phy_v_addr;
	UINT32 prcm_v_addr;
	UINT32 venc_v_addr;
	UINT32 hdmi_pll_v_addr; // Centaurus
	int i;
};

/* Global var */
struct ti81xx_hdmi_params hdmi_obj;
static struct ti81xx_hdmi_init_params initParams;

/* Module param */
static int  hdmi_mode = -1;

//------------------------------------------------------------------------------
//
//  Function:  DllEntry
//
//  Standard Windows DLL entry point.
//
BOOL WINAPI DllEntry(HANDLE hInstance, DWORD dwReason, VOID *pReserved)
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER((HMODULE)hInstance);
        DisableThreadLibraryCalls((HMODULE)hInstance);
        PRINTMSG(ZONE_INIT, (TEXT("HDMI: DLL_PROCESS_ATTACH\r\n")));
        break;

    case DLL_PROCESS_DETACH:
        PRINTMSG(ZONE_INIT, (TEXT("HDMI: DLL_PROCESS_DETACH\r\n")));
        break;
    }

    return TRUE;
}


/*
 * ti81xx_hdmi_open: This function opens hdmi driver.
 */
DWORD HDM_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
	int ret = 0;

    if (context != (DWORD)(&hdmi_obj))
        return FALSE;

	/* Call library to open HDMI */
	hdmi_obj.hdmi_lib_handle = ti81xx_hdmi_lib_open(0, &ret, 0x0);

	if ((ret == 0x0) && (hdmi_obj.hdmi_lib_handle != NULL)) 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: Opened\r\n")));
	}
	else
    {
        DEBUGMSG(ZONE_ERROR, (L"HDMI: ERROR: Could not open %d %X\r\n", 
           ret, hdmi_obj.hdmi_lib_handle));
	}

	return (DWORD)(hdmi_obj.hdmi_lib_handle);
}

/*
 * ti81xx_hdmi_release: This function releases hdmi driver.
 */
BOOL HDM_Close(DWORD context)
{
	int ret = 0;

    ret = ti81xx_hdmi_lib_close ((void *)context, (void *)NULL);
    
	/* Call close of the library */
	if (!ret) 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: Closed\r\n")));
	}
    else
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: Error in closing instance %08X.\r\n"), context));
	}

	return ret;
}

/*
 * ti81xx_hdmi_ioctl: This function will process IOCTL commands sent by
 * the application.
 */
BOOL HDM_IOControl(
    DWORD context, DWORD dwCode,
    BYTE *pInBuffer, DWORD inSize,
    BYTE *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    int ret;

    DEBUGMSG(ZONE_WARN, (L"HDMI: Ioctl\r\n"));
    ret = ti81xx_hdmi_lib_control((void *)context, (UINT32)dwCode, (void *)pInBuffer, NULL);
    return (ret == 0x0);
}

/**
 * ti81xx_hdmi_init() - Initialize TI81XX HDMI Driver
 */
DWORD HDM_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
//	int result;
    HKEY hKey;
    DWORD dwValueType;
    DWORD dwDataSize;
    PHYSICAL_ADDRESS physicalAddress;

    PRINTMSG(ZONE_INIT,  (L"+HDM_Init\r\n"));

    // Read registry settings
    hKey = OpenDeviceKey((LPCTSTR)szContext);
    if (!hKey) 
    {
        DEBUGMSG(ZONE_ERROR, (L"HDMI: ERROR: Failed to open registry key\r\n"));
        goto err_exit;
    }

    dwDataSize = sizeof(hdmi_mode);
    if (RegQueryValueEx(hKey, L"HdmiMode", NULL, &dwValueType,
                    (LPBYTE)&hdmi_mode, &dwDataSize))
    {
        DEBUGMSG(ZONE_WARN,
             (L"HDMI: WARNING: Failed to read hdmi mode from registry\r\n"));
        hdmi_mode = hdmi_1080P_60_mode; // Default value
    }
    RegCloseKey(hKey);

    memset (&(hdmi_obj), 0, sizeof(struct ti81xx_hdmi_params));

    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = PRCM_0_REGS /*AM38X_PRCM_REGS_PA*/;
	hdmi_obj.prcm_v_addr = (UINT32)MmMapIoSpace(physicalAddress, 0x500, FALSE);
	if (hdmi_obj.prcm_v_addr == 0x0)
    {
		ERRORMSG(TRUE, (_T("HDMI: HDM_Init: Unable to map memory region for PRCM.\r\n")));
		goto err_exit;
	} 
    else 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: PRCM addr @ %08X\r\n"), hdmi_obj.prcm_v_addr));
	}

	/* Initialize the global strucutres... */
	hdmi_obj.hdmi_lib_handle = NULL;

    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = HDMI_WP_0_REGS /*0x46C00000*/ /*AM38X_HDMI_WP_0_REGS_PA*/;
	hdmi_obj.wp_v_addr = (UINT32)MmMapIoSpace(physicalAddress, 512, FALSE);
	if (hdmi_obj.wp_v_addr == 0x0)
    {
		ERRORMSG(TRUE, (_T("HDMI: HDM_Init: Unable to map memory region for WP.\r\n")));
		goto err_exit;
	} 
    else 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: Wrapper addr @ %08X\r\n"), hdmi_obj.wp_v_addr));
	}

    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = HDMI_CORE_0_REGS /*0x46C00400*/ /*AM38X_HDMI_CORE_0_REGS_PA*/;
	hdmi_obj.core_v_addr = (UINT32)MmMapIoSpace(physicalAddress, 2560, FALSE);
	if (hdmi_obj.core_v_addr == 0x0)
    {
		ERRORMSG(TRUE, (_T("HDMI: HDM_Init: Unable to map memory region for Core.\r\n")));
		goto err_exit;
	} 
    else 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: Core addr @ %08X\r\n"), hdmi_obj.core_v_addr));
	}

    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = HDMI_PHY_0_REGS; /*0x48122000*/  /*AM389X_HDMI_PHY_0_REGS_PA*/
	hdmi_obj.phy_v_addr = (UINT32)MmMapIoSpace(physicalAddress, 64, FALSE);
	if (hdmi_obj.phy_v_addr == 0x0)
    {
		ERRORMSG(TRUE, (_T("HDMI: HDM_Init: Unable to map memory region for PHY.\r\n")));
		goto err_exit;
	} 
    else 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: PHY addr @ %08X\r\n"), hdmi_obj.phy_v_addr));
	}

    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = 0x48106000;
	hdmi_obj.venc_v_addr = (UINT32)MmMapIoSpace(physicalAddress, 0x80, FALSE);
	if (hdmi_obj.venc_v_addr == 0x0)
    {
		ERRORMSG(TRUE, (_T("HDMI: HDM_Init: Unable to map memory region for VENC.\r\n")));
		goto err_exit;
	} 
    else 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: VENC addr @ %08X\r\n"), hdmi_obj.venc_v_addr));
	}

#ifndef CONFIG_ARCH_TI816X
// Centaurus
    physicalAddress.HighPart = 0;
    physicalAddress.LowPart  = 0x481c5200;
	hdmi_obj.hdmi_pll_v_addr = (UINT32)MmMapIoSpace(physicalAddress, 0x80, FALSE);
	if (hdmi_obj.hdmi_pll_v_addr == 0x0)
    {
		ERRORMSG(TRUE, (_T("HDMI: HDM_Init: Unable to map memory region for HDMI PLL.\r\n")));
		goto err_exit;
	} 
    else 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMI: HDMI PLL addr @ %08X\r\n"), hdmi_obj.hdmi_pll_v_addr));
	}
#endif

	/* Initialize the HDMI library */
	initParams.wp_base_addr       =   (UINT32) hdmi_obj.wp_v_addr;
	initParams.core_base_addr     =   (UINT32) hdmi_obj.core_v_addr;
	initParams.phy_base_addr      =   (UINT32) hdmi_obj.phy_v_addr;
	initParams.prcm_base_addr     =   (UINT32) hdmi_obj.prcm_v_addr;
	initParams.venc_base_addr     =   (UINT32) hdmi_obj.venc_v_addr;
	initParams.hdmi_pll_base_addr =   (UINT32) hdmi_obj.hdmi_pll_v_addr; // Centaurus

	if (ti81xx_hdmi_lib_init(&initParams, hdmi_mode) != 0x0)
    {
		ERRORMSG(TRUE, (_T("HDMI: HDM_Init: Failed.\r\n")));
		goto err_exit;
	}

    PRINTMSG(1, (_T("HDMI: Initialized.  hdmi_mode %d\r\n"), hdmi_mode));
	return ((DWORD)(&hdmi_obj));

err_exit:
	return FALSE;
}

/**
 * ti81xx_hdmi_exit() - Perform clean before unload
 */
BOOL HDM_Deinit(DWORD context)
{
    if (context != (DWORD)(&hdmi_obj))
        return FALSE;

	ti81xx_hdmi_lib_deinit(NULL);

    if (hdmi_obj.wp_v_addr   != 0) MmUnmapIoSpace((PVOID)(hdmi_obj.wp_v_addr),   512);
    if (hdmi_obj.core_v_addr != 0) MmUnmapIoSpace((PVOID)(hdmi_obj.core_v_addr), 2560);
    if (hdmi_obj.phy_v_addr  != 0) MmUnmapIoSpace((PVOID)(hdmi_obj.phy_v_addr),  64);
    if (hdmi_obj.prcm_v_addr != 0) MmUnmapIoSpace((PVOID)(hdmi_obj.prcm_v_addr), 0x500);
    if (hdmi_obj.venc_v_addr != 0) MmUnmapIoSpace((PVOID)(hdmi_obj.venc_v_addr), 0x80);
    if (hdmi_obj.hdmi_pll_v_addr != 0) MmUnmapIoSpace((PVOID)(hdmi_obj.hdmi_pll_v_addr), 0x80);  // Centaurus

    return TRUE;
}

