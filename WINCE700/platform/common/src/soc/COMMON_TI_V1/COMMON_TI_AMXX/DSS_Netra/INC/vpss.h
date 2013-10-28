/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/

#ifndef __VPSS_H__
#define __VPSS_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define CONFIG_TI816X_VPSS_DEBUG_SUPPORT
#ifdef CONFIG_TI816X_VPSS_DEBUG_SUPPORT
#ifndef DEBUG
#define DEBUG
#endif  // DEBUG
#endif  // CONFIG_TI816X_VPSS_DEBUG_SUPPORT

extern unsigned int vpss_debug;

/* These flags are mutually exclusive. Only 1 should be set */
#ifdef CONFIG_HDMI_720P_DISPLAY
#define NETRA_DESKTOP_HDMI_OUTPUT       1   // Tested on Netra and Centaurus
#define NETRA_DESKTOP_COMPONENT_OUTPUT  0   // Tested on Netra. Not supported on Centaurus
#define NETRA_DESKTOP_COMPOSITE_OUTPUT  0   // Tested on Netra. Not supported on Centaurus
#define NETRA_DESKTOP_SVIDEO_OUTPUT     0   // Tested on Netra and Centaurus
#endif

#ifdef CONFIG_HDMI_1080P_DISPLAY
#define NETRA_DESKTOP_HDMI_OUTPUT       1   // Tested on Netra and Centaurus
#define NETRA_DESKTOP_COMPONENT_OUTPUT  0   // Tested on Netra. Not supported on Centaurus
#define NETRA_DESKTOP_COMPOSITE_OUTPUT  0   // Tested on Netra. Not supported on Centaurus
#define NETRA_DESKTOP_SVIDEO_OUTPUT     0   // Tested on Netra and Centaurus
#endif

#ifdef CONFIG_SVIDEO_DISPLAY
#define NETRA_DESKTOP_HDMI_OUTPUT       0   // Tested on Netra and Centaurus
#define NETRA_DESKTOP_COMPONENT_OUTPUT  0   // Tested on Netra. Not supported on Centaurus
#define NETRA_DESKTOP_COMPOSITE_OUTPUT  0   // Tested on Netra. Not supported on Centaurus
#define NETRA_DESKTOP_SVIDEO_OUTPUT     1   // Tested on Netra and Centaurus
#endif


/* When using composite/SVideo analog outputs use the following flag to enable PAL. Default is NTSC. */
#ifdef CONFIG_PAL_DISPLAY
#define NETRA_DESKTOP_PAL_OUTPUT        1   // Tested
#else
#define NETRA_DESKTOP_PAL_OUTPUT        0   // Tested
#endif

#if NETRA_DESKTOP_HDMI_OUTPUT

#ifdef CONFIG_HDMI_1080P_DISPLAY
#define GRPX_WIDTH                      (1920u)
#define GRPX_HEIGHT                     (1080u)
#endif

#ifdef CONFIG_HDMI_720P_DISPLAY
#define GRPX_WIDTH                      (1280u)
#define GRPX_HEIGHT                     (720u)
#endif

#else
#if NETRA_DESKTOP_COMPONENT_OUTPUT
#define GRPX_WIDTH                      (1280u)
#define GRPX_HEIGHT                     (720u)
#else
#if NETRA_DESKTOP_PAL_OUTPUT
// PAL SD resolution. Width/Height have to be multiple of 32.
#define GRPX_WIDTH                      (704u)
#define GRPX_HEIGHT                     (576u)
#else
// NTSC SD resolution. Width/Height have to be multiple of 32.
// 720x480 does not work since 720 is not a multiple of 32.
#define GRPX_WIDTH                      (704u)
#define GRPX_HEIGHT                     (480u)
#endif  // NETRA_DESKTOP_PAL_OUTPUT
#endif  // NETRA_DESKTOP_COMPONENT_OUTPUT
#endif  // NETRA_DESKTOP_HDMI_OUTPUT

#if 1
#define GRPX_BPP                        (32u)
#define GRPX_FORMAT                     (FVID2_DF_ARGB32_8888)
#else
#define GRPX_BPP                        (24u)
#define GRPX_FORMAT                     (FVID2_DF_RGB24_888)
#endif

/* Frame and input video parameters. */
#define REGION_WIDTH                    GRPX_WIDTH
#define REGION_HEIGHT                   GRPX_HEIGHT

#define NETRA_GRPX_FLIPPING_SUPPORT     1

void grpx_display_test(void);
void grpx_flipbuf_test(void);
void grpx_setup(void);
void grpx_setbuf(unsigned int paddr);
void grpx_flipbuf(unsigned int paddr);
void grpx_start();
void vpssm3_download(void);

#ifdef __cplusplus
}
#endif

#endif __VPSS_H__

