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

/**
 * Key notes
 * 1. Wrapper doesn't generate interrupts for all the events, generates for HPD.
 *	  Using core interrupt instead.
 * 2. DVI mode is not configurable, operates in HDMI mode only, control in
 *	  HDMI_CTRL
 * 3. The Core system should operate as a SLAVE. MASTER/SLAVE mode depends on
 *	  core/wrapper integration.
 *
 */

/*
 * Open items
 * 1. Handle DDC bus hangups / lockups during EDID Read [Done]
 * 2. use copy to user and copy from user
 */


/* ========================================================================== */
/*	Include Files							      */
/* ========================================================================== */

#include <windows.h>
#include <oal_log.h>
#include <ceddk.h>
#include <devload.h>
#include <nkintr.h>
#include <windev.h>
#include <Oal_io.h>

#include "ti81xxhdmi.h"
#include "ti81xx_hdmi_cfg.h"
#include "ti81xx_hdmi_regoffsets.h"

#ifdef DEBUG

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_INFO           DEBUGZONE(4)
#define ZONE_IST            DEBUGZONE(5)
#define ZONE_IOCTL          DEBUGZONE(6)
#define ZONE_VERBOSE        DEBUGZONE(7)

DBGPARAM dpCurSettings = {
    L"HDMI Driver", {
        L"Errors",      L"Warnings",    L"Function",    L"Init",
        L"Info",        L"IST",         L"IOCTL",       L"Verbose",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#else
#if 0
#define ZONE_ERROR          1
#define ZONE_WARN           1
#define ZONE_FUNCTION       1
#define ZONE_INIT           1
#define ZONE_INFO           1
#define ZONE_IST            1
#define ZONE_IOCTL          1
#define ZONE_VERBOSE        1
#else
#define ZONE_ERROR          1
#define ZONE_WARN           1
#define ZONE_FUNCTION       0
#define ZONE_INIT           0
#define ZONE_INFO           0
#define ZONE_IST            0
#define ZONE_IOCTL          0
#define ZONE_VERBOSE        0
#endif
#endif

#if 0
#include <asm/io.h>
#endif

/* ========================================================================== */
/*	Local Configurations						      */
/* ========================================================================== */
#define HDMI_DDC_CMD_TIMEOUT (0xFFFFFu)
/* Timeout periods used to wait for a DDC operation to complete */
#define HDMI_WP_RESET_TIMEOUT (0xFFFFFu)
/* Timeout periods used to wait for a DDC opeation to complete */
#define HDMI_PHY_2_WP_PLL_LOCK_TIMEOUT (0xFFFFFu)
/* Timeout periods used to wait TCLK to stabilize - TCLK would be generated
   by PHY to operate wrapper */

/* ========================================================================== */
/*	Local Defines							      */
/* ========================================================================== */

#define HDMI_CTRL_PACKET_MODE_24BITS_PIXEL	(0x4u)
/* Defines used to configure the number of bits/pixel that would sent to
   packetizer */
#define HDMI_CTRL_PACKET_MODE_30BITS_PIXEL	(0x5u)
/* Defines used to configure the number of bits/pixel that would sent to
   packetizer */
#define HDMI_CTRL_PACKET_MODE_36BITS_PIXEL	(0x6u)
/* Defines used to configure the number of bits/pixel that would sent to
   packetizer */
#define HDMI_VID_MODE_DITHER_TO_24_BITS_MODE (0x0u)
/* Defines to used to determine the dithering width */
#define HDMI_VID_MODE_DITHER_TO_30_BITS_MODE (0x1u)
/* Defines to used to determine the dithering width */
#define HDMI_VID_MODE_DITHER_TO_36_BITS_MODE (0x2u)
/* Defines to used to determine the dithering width */
#define HDMI_TMDS_CTRL_IP_CLOCK_MULTIPLIER_AUDIO	(0x1u)
/* Defines the multiplier value used to multiply the input clock IDCK, in order
   to support higher sampling rates / channels audio */
#define HDMI_AVI_INFOFRAME_PKT_TYPE 	(0x82u)
/* AVI Info frame header - packet type - defined by standard */
#define HDMI_AVI_INFOFRAME_PKT_VER		(0x02)
/* AVI Info frame header - packet version - defined by standard */
#define HDMI_AVI_INFOFRAME_PKT_LEN		(0x0D)
/* AVI Info frame header - packet version - defined by standard */
#define HDMI_AVI_INFOFRAME_Y0_Y1_MASK		(0x60u)
/* Mask to set/extract Y0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_A0_MASK		(0x10u)
/* Mask to set/extract A0 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_B0_B1_MASK		(0x0Cu)
/* Mask to set/extract B0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_S0_S1_MASK		(0x03u)
/* Mask to set/extract S0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_C0_C1_MASK		(0xC0u)
/* Mask to set/extract C0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_M0_M1_MASK		(0x30u)
/* Mask to set/extract M0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_R0_R3_MASK		(0x0Fu)
/* Mask to set/extract R0-3 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_ITC_MASK 	(0x80u)
/* Mask to set/extract ITC bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_EC2_EC0_MASK 	(0x70u)
/* Mask to set/extract EC0-3 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_Q1_Q0_MASK		(0x0Cu)
/* Mask to set/extract Q0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_SC1_SC0_MASK 	(0x03u)
/* Mask to set/extract SC0-1 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_VIC6_VIC0_MASK	(0x7Fu)
/* Mask to set/extract VIC6-0 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_PR3_PR0_MASK 	(0x0Fu)
/* Mask to set/extract PR3-0 bit of first byte of AVI info packet */
#define HDMI_AVI_INFOFRAME_CONST_0x100		(0x100u)
/* Constant used to calculate AVI info frame checksum */
#define HDMI_MINIMUM_PIXELS_SEC 		(25000000u)
/* HDMI standard Mandates that at a minimum there should be 25 MPixels/sec. */
#define HDMI_PIXEL_REPLECATED_ONCE		(0x2)
/* Each pixel would be sent twice */
#define HDMI_PIXEL_REPLECATED_FOUR_TIMES	(0x4)
/* Each pixel would be sent four times */

/* Standard resolutions column X row X FPS */
#define HDMI_VIDEO_STAND_NTSC			(858 * 525 * 30)
#define HDMI_VIDEO_STAND_PAL			(858 * 625 * 25)
#define HDMI_VIDEO_STAND_720P60 		(1650 * 750 * 60)
#define HDMI_VIDEO_STAND_1080P60		(2200 * 1125 * 60)
#define HDMI_VIDEO_STAND_1080I60		(2200 * 1125 * 30)
#define HDMI_VIDEO_STAND_1080P30		(2200 * 1125 * 30)

/* Undef this to test HDMI */
//#define HDMI_TEST				(1)

/* ========================================================================== */
/*                Local Structure                                             */
/* ========================================================================== */

struct instance_cfg {
	UINT32 instance;
	UINT32 core_base_addr;
	UINT32 wp_base_addr;
	UINT32 phy_base_addr;
	UINT32 prcm_base_addr;
	UINT32 venc_base_addr;
	BOOL is_recvr_sensed;
	BOOL is_scl_clocked;
	BOOL is_streaming;
	struct hdmi_cfg_params config;
	UINT32 vSync_counter;
	BOOL is_interlaced;
	enum ti81xxhdmi_mode hdmi_mode;
	UINT32 hdmi_pll_base_addr;
};

/* ========================================================================== */
/*			   Local Function Declarations			      */
/* ========================================================================== */
static int configure_phy(struct instance_cfg *inst_context);
static int configure_wrapper(struct instance_cfg *inst_context);
static int configure_core_input(struct instance_cfg *inst_context);
static int configure_core_data_path(struct instance_cfg *inst_context);
static int configure_core(struct instance_cfg *inst_context);
static int configure_policies(struct instance_cfg *inst_context);
static int configure_avi_info_frame(struct instance_cfg *inst_context);
static int configure_ctrl_packets(struct instance_cfg *inst_context);
static int configure_csc_ycbcr_rgb(struct instance_cfg *inst_context);

static int validate_info_frame_cfg(struct hdmi_info_frame_cfg *config);
static int validate_core_config(struct hdmi_core_input_cfg *config);
static int validate_wp_cfg(struct hdmi_wp_config *config);
static int validate_path_config(struct hdmi_core_data_path *config);
static int check_copy_config(struct instance_cfg *inst_cntxt,
		struct hdmi_cfg_params *config);
int ti81xx_hdmi_set_mode(enum ti81xxhdmi_mode hdmi_mode,
		struct instance_cfg *cfg);
int ti81xx_hdmi_copy_mode_config(enum ti81xxhdmi_mode mode,
		struct instance_cfg *cfg);
static int determine_pixel_repeatation(struct instance_cfg *inst_context);


static int ti81xx_hdmi_lib_read_edid(void *handle,
		struct ti81xxdhmi_edid_params *r_params,
		void *args);
static int get_phy_status(struct instance_cfg *inst_context,
		struct ti81xxhdmi_phy_status *stat);
#if 0
static int ti81xx_hdmi_lib_get_cfg(void *handle,
		struct hdmi_cfg_params *config,
		void *args);
#endif
static void HDMI_ARGS_CHECK(UINT32 condition);
static int ti81xx_hdmi_lib_config(struct hdmi_cfg_params *config);

/* ========================================================================== */
/*                   Global Variables                                         */
/* ========================================================================== */
static struct instance_cfg hdmi_config;
/* Pool of HDMI objects */
static struct hdmi_cfg_params default_config = TI81XX_HDMI_8BIT_1080p_60_16_9_HD;
/* Default configuration to start with */

struct hdmi_cfg_params config_1080p60 = TI81XX_HDMI_8BIT_1080p_60_16_9_HD;
struct hdmi_cfg_params config_720p60 = TI81XX_HDMI_8BIT_720_60_16_9_HD;
struct hdmi_cfg_params config_1080i60 = TI81XX_HDMI_8BIT_1080i_60_16_9_HD;
struct hdmi_cfg_params config_1080p30 = TI81XX_HDMI_8BIT_1080p_30_16_9_HD;

struct hdmi_pll_ctrl gpll_ctrl[] = {
	{19, 1485, 10, 0x20021001},
	{19, 745,  10, 0x20021001}
};

/* ========================================================================== */
/*              Local Functions                                               */
/* ========================================================================== */

#ifndef CONFIG_ARCH_TI816X
static void udelay (int usec)
{
    Sleep(1);  // 1 ms
}
#endif

#ifndef CONFIG_ARCH_TI816X
// Centaurus begin
/* command
 * 0x0: Command to change LDO to OFF state
 * 0x1:	Command to change LDO to ON state
 * 0x2:	Command to go to LDO TXON Power
 */
static int wp_phy_pwr_ctrl(int wp_pwr_ctrl_addr, int command)
{
	volatile UINT32 reg_value;
	UINT32 cnt = 0;
	UINT32 max_count = 10000;
	int ret_val = 0;
	switch (command)
	{
		case 0x0:
			reg_value = INREG32(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PHY_PWR_CMD_MASK);
			OUTREG32(wp_pwr_ctrl_addr, reg_value);
			cnt = 0;
			do
			{
				reg_value = INREG32(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0 &&  (cnt < max_count));
			if (reg_value != 0)
			{
				ret_val = -1;
			}
			break;
		case 0x1:
			reg_value = INREG32(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PHY_PWR_CMD_MASK);
			reg_value |= 0x1 << HDMI_WP_PWR_CTRL_PHY_PWR_CMD_SHIFT;
			OUTREG32(wp_pwr_ctrl_addr, reg_value);
			cnt = 0;
			do
			{
				reg_value = INREG32(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while((reg_value >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x1 &&
					(cnt < max_count));
			if ((reg_value  >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x1)
			{
				ret_val = -1;
			}
			break;
		case  0x2:
			reg_value = INREG32(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PHY_PWR_CMD_MASK);
			reg_value |= 0x2 << HDMI_WP_PWR_CTRL_PHY_PWR_CMD_SHIFT;
			OUTREG32(wp_pwr_ctrl_addr, reg_value);
			cnt = 0;
			do
			{
				reg_value = INREG32(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while((reg_value  >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x2 &&  (cnt < max_count));
			if ((reg_value >> HDMI_WP_PWR_CTRL_PHY_PWR_STATUS_SHIFT) != 0x2)
			{
				ret_val = -1;
			}
			break;
		default:
			ret_val = -1;
	}
	return ret_val;
}

/* Command
 * 0x0: Command to change to OFF state
 * 0x1: Command to change to ON state for  PLL only (HSDIVISER is OFF)
 * 0x2: Command to change to ON state for both PLL and HSDIVISER
 * 0x3: Command to change to ON state for both PLL and HSDIVISER
 (no clock output to the DSI complex IO)
 */
static int wp_pll_pwr_ctrl(int wp_pwr_ctrl_addr, int command)
{
	volatile UINT32 reg_value;
	UINT32 cnt = 0;
	UINT32 max_count = 10000;
	int ret_val = 0;
	switch (command)
	{
		case 0x0:
			reg_value = INREG32(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			OUTREG32(wp_pwr_ctrl_addr, reg_value);
			cnt = 0;
			do
			{
				reg_value = INREG32(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0 &&  (cnt < max_count));
			if (reg_value != 0)
			{
				ret_val = -1;
			}
			break;
		case 0x1:
			reg_value = INREG32(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			reg_value |= 0x1 << HDMI_WP_PWR_CTRL_PLL_PWR_CMD_SHIFT;
			OUTREG32(wp_pwr_ctrl_addr, reg_value);
			cnt = 0;
			do
			{
				reg_value = INREG32(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0x1 &&  (cnt < max_count));
			if (reg_value != 0x1)
			{
				ret_val = -1;
			}
			break;
		case  0x2:
			reg_value = INREG32(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			reg_value |= 0x2 << HDMI_WP_PWR_CTRL_PLL_PWR_CMD_SHIFT;
			OUTREG32(wp_pwr_ctrl_addr, reg_value);
			cnt = 0;
			do
			{
				reg_value = INREG32(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0x2 &&  (cnt < max_count));
			if (reg_value != 0x2)
			{
				ret_val = -1;
			}
			break;
		case  0x3:
			reg_value = INREG32(wp_pwr_ctrl_addr);
			reg_value &= ~(HDMI_WP_PWR_CTRL_PLL_PWR_CMD_MASK);
			reg_value |= 0x3 << HDMI_WP_PWR_CTRL_PLL_PWR_CMD_SHIFT;
			OUTREG32(wp_pwr_ctrl_addr, reg_value);
			cnt = 0;
			do
			{
				reg_value = INREG32(wp_pwr_ctrl_addr);
				reg_value &= HDMI_WP_PWR_CTRL_PLL_PWR_STATUS_MASK;
				udelay(10);
				cnt++;
			}while(reg_value != 0x3 &&  (cnt < max_count));
			if (reg_value != 0x3)
			{
				ret_val = -1;
			}
			break;

		default:
			ret_val = -1;
	}
	return ret_val;
}
// Centaurus  end
#endif 

#ifdef CONFIG_ARCH_TI816X
// Netra  begin
/*
 *	   This function is expected to be called when initializing or
 *	   when re-configuring. After re-configuration its recomended to reset the
 *	   core and wrapper. To stabilize the clocks, it recomended to wait for a
 *	   period of time.
 */
static int configure_phy(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	int phy_base;
	volatile UINT32 temp;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_phy\r\n")));

	phy_base = inst_context->phy_base_addr;
	/* Steps
	 * 0. Power up if powered down
	 * 1. Determine the TCLK based in Deep color mode (Dither mode is used
	 *	  to get depth of the color) and Pixel repeatation (depends on deep
	 *	  color / resolution and audio). Turn OFF BIST pattern generator
	 * 2. Turn OFF BIST and DVI Encoder
	 * 3. Configure the source termination determination - we would require
	 *	  when the sink terminates the source - recomended by HDMI Spec 1.3A
	 *	  when operating higer frequencies
	 * 4. Enable the PHY
	 */
	temp = INREG32((phy_base + PHY_TMDS_CNTL3_OFFSET));
	if ((temp & HDMI_PHY_TMDS_CNTL3_PDB_MASK) !=
			HDMI_PHY_TMDS_CNTL3_PDB_MASK) {
		temp |= HDMI_PHY_TMDS_CNTL3_PDB_MASK;
		OUTREG32((phy_base + PHY_TMDS_CNTL3_OFFSET), temp);
	}
	/* BIST Pattern generator is disabled - leave it at that */
	temp = INREG32((phy_base + PHY_TMDS_CNTL3_OFFSET));
	temp &= (~((HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_MASK) |
				(HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_MASK) |
				(HDMI_PHY_TMDS_CNTL3_BIST_SEL_MASK)));

	/* Step 1.1 - Output width of the dither module in core, determines
	 *			  deep color or not
	 */
	if (inst_context->config.core_path_config.output_width ==
			hdmi_10_bits_chan_width) {
		temp |= (HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_10BITCHANNEL <<
				HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_SHIFT);

	} else if (inst_context->config.core_path_config.output_width ==
			hdmi_8_bits_chan_width) {
		temp |= (HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_NO <<
				HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_SHIFT);
	} else {
		temp |= (HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_12BITCHANNEL <<
				HDMI_PHY_TMDS_CNTL3_DPCOLOR_CTL_SHIFT);
	}

	rtn_value = determine_pixel_repeatation(inst_context);
	if (rtn_value == HDMI_PIXEL_REPLECATED_ONCE) {
		temp |= (HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_2_0X <<
				HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_SHIFT);
	} else if (rtn_value == HDMI_PIXEL_REPLECATED_FOUR_TIMES) {
		temp |= (HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_4_0X <<
				HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_SHIFT);
	} else if (rtn_value == 0x0) {
		temp |= (HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_1_0X <<
				HDMI_PHY_TMDS_CNTL3_CLKMULT_CTL_SHIFT);
	} else {
		ERRORMSG(TRUE, (_T("HDMIL: Could not calc pixel repeatation that would be required.\r\n")));
		goto exit_this_func;
	}
	rtn_value = 0x0;
	OUTREG32((phy_base + PHY_TMDS_CNTL3_OFFSET), temp);

	temp = INREG32((phy_base + PHY_BIST_CNTL_OFFSET));
	temp &= ~HDMI_PHY_BIST_CNTL_BIST_EN_MASK;
	temp |= HDMI_PHY_BIST_CNTL_ENC_BYP_MASK;
	OUTREG32((phy_base + PHY_BIST_CNTL_OFFSET), temp);

	/* Since the 10bit encode is done by the core, we would require to
	   disable 10bit encode in the PHY. Do So */
	OUTREG32((phy_base + PHY_TMDS_CNTL9_OFFSET), 0xE0);

	/************************ PHY BIST Test @ half clock rate *********************/

#ifdef TEST_PHY_SEND_OUT_0xAA_AT_HALF_CLOCK_RATE_ON_ALL_DATA_LINES
	OUTREG32((phy_base + PHY_BIST_CNTL_OFFSET), 0x40);
	OUTREG32((phy_base + PHY_TMDS_CNTL3_OFFSET), 0xE9);
	OUTREG32((phy_base + PHY_BIST_PATTERN_OFFSET), 0x00);
	/* Program the instruction, pattern, configuration registers */
	OUTREG32((phy_base + PHY_BIST_INST0_OFFSET), 0x81);
	OUTREG32((phy_base + PHY_BIST_CONF0_OFFSET), 0x00);
	OUTREG32((phy_base + PHY_BIST_INST1_OFFSET), 0x20);
	temp = 0xFF;
	/* Wait for few clocks (say 20 TMDS clocks) would require this. */
	while (temp)
		temp--;
	OUTREG32((phy_base + PHY_BIST_CNTL_OFFSET), 0x41);
#endif	/* TEST_PHY_SEND_OUT_0xAA_AT_HALF_CLOCK_RATE_ON_ALL_DATA_LINES */
	/************************PHY BIST Test @ half clock rate***********************/

	/* Step 3 and 4 */
	temp = INREG32((phy_base + PHY_TMDS_CNTL2_OFFSET));
	temp |=
		(HDMI_PHY_TMDS_CNTL2_TERM_EN_MASK |
		 HDMI_PHY_TMDS_CNTL2_OE_MASK);
	OUTREG32((phy_base + PHY_TMDS_CNTL2_OFFSET), temp);

exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_phy<<<<\r\n")));
	return (rtn_value);
}
// Netra  end
#else
// Centaurus  begin
static int configure_phy(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	int phy_base, wp_base;
	volatile UINT32 temp;
	int cmd, count;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_phy\r\n")));

	/* Steps
	 * LDOOn and TX Power ON
	 * Set the Transmit control register based on the pixel clock setting.
	 * Set the digital control register
	 * Set the power control
	 * Set the pad control register
	 * Disable Trim and Test Control
	 * Analog interface Control
	 * Digital interface control
	 * disable bist test.
	 */
	phy_base = inst_context->phy_base_addr;
	wp_base = inst_context->wp_base_addr;

	/* Power on the PLL and HSDivider */
	cmd = 0x2;
	rtn_value = wp_pll_pwr_ctrl(wp_base + HDMI_WP_PWR_CTRL_OFFSET, cmd);
	if (rtn_value)
	{
		rtn_value = -1;
		goto exit;
	}
	/* change LDO to on state */
	cmd = 1;
	rtn_value = wp_phy_pwr_ctrl(wp_base + HDMI_WP_PWR_CTRL_OFFSET, cmd);
	if (rtn_value)
	{
		rtn_value = -1;
		goto exit;
	}
	/* TXPower ON */
	cmd = 2;
	rtn_value = wp_phy_pwr_ctrl(wp_base + HDMI_WP_PWR_CTRL_OFFSET, cmd);
	if (rtn_value)
	{
		rtn_value = -1;
		goto exit;
	}
	/* read address 0 in order to get the SCPreset done completed */
	/* Dummy access performed to solve resetdone issue */
	INREG32(phy_base + HDMI_PHY_TX_CTRL_OFF);

	/* TX Control bit 30 set according to pixel clock frequencies*/
	temp = INREG32(phy_base + HDMI_PHY_TX_CTRL_OFF);
	switch (inst_context->config.display_mode)
	{
		case hdmi_1080P_30_mode:
		case hdmi_720P_60_mode:
		case hdmi_1080I_60_mode:
			temp |= 0x1 << 30;
			break;
		case hdmi_1080P_60_mode:
			temp |= 0x2 << 30;
			break;
		default:
			return -1;
	}
	/* Not programmed in OMA4 */
#if 0
	/* Enable de-emphasis on all the links D0, D1, D2 and CLK */
	temp |= 0x1 << 27;
	temp |= 0x1 << 26;
	temp |= 0x1 << 25;
	temp |= 0x1 << 24;
	/* Set the default de-emphasis value for all the links
	 * TODO: Get the proper de-emphasis value
	 */
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 21;
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 18;
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 15;
	temp |= HDMI_PHY_DEF_DE_EMPHASIS_VAL << 12;
	/* Configure the slow edge for the normal setting */
	temp |= 0x0 << 10;
	temp |= 0x0 << 8;
	temp |= 0x0 << 6;
	temp |= 0x0 << 4;
	/* Set the TMDS level for normal I/O of 3.3V */
	temp |= 0x0 << 3;
	/* Nominal current of 10ma used for signalling */
	temp |= 0x0 << 1;
#endif
	OUTREG32(phy_base + HDMI_PHY_TX_CTRL_OFF, temp);

	/* According to OMAP4 */
	/* Power Control */
	temp = INREG32(phy_base + HDMI_PHY_PWR_CTRL_OFF);
	/* setup max LDO voltage */
	temp |= HDMI_PHY_DEF_LDO_VOLTAGE_VAL << 0;
	OUTREG32(phy_base + HDMI_PHY_PWR_CTRL_OFF, temp);

	/* Pad configuration Control */
	temp = INREG32(phy_base + HDMI_PHY_PAD_CFG_CTRL_OFF);
	/* Normal polarity for all the links */
	temp |= 0x1 << 31;
	temp |= 0x0 << 30;
	temp |= 0x0 << 29;
	temp |= 0x0 << 28;
	temp |= 0x0 << 27;

	/* Channel assignement is 10101 – D2- D1 –D0-CLK */
	temp |=  0x21 << 22;
	OUTREG32(phy_base + HDMI_PHY_PAD_CFG_CTRL_OFF, temp);

	/* Digital control */
	temp = INREG32(phy_base + HDMI_PHY_DIGITAL_CTRL_OFF);;
	/* Use bit 30 from this register as the enable signal for the TMDS */
	temp |= 1 << 31;
	/* Enable TMDS signal. TODO*/
	temp |= 1 << 30;
	/* Use 28 pin as the TX valid from this register */
	temp |= 1  << 29;
	/* Tx Valid enable TODO*/
	temp |= 1 << 28;
	OUTREG32(phy_base + HDMI_PHY_DIGITAL_CTRL_OFF, temp);
#if 0
	/* Trim and Test Control */
	/* TODO Don't use the Bandgap values */
	temp |= 0x0 << 31;
	/* TODO Dont use cap trim settings */
	temp |= 0x0 << 15;
	/* TODO Dont enable the bandgap and switched cap current */
	temp |= 0x0 << 7;
	temp |= 0x0 << 6;
	OUTREG32(phy_base + HDMI_PHY_TRIM_TEST_CTRL_OFF, temp);

	/* Analog Interface control */
	temp = 0;
	/* TODO: Don't put AFE in debug mode */
	temp |= 0x0 << 16;
	/* TODO: Don't use the LDO prog register */
	temp |= 0x0 << 15;
	/* TODO: Don't override the value of the analog signal LDOPGD*/
	temp |= 0x0 << 14;
	/* TODO: Don't override the value of the analog signal BGON */
	temp |= 0x0 << 13;
	/* TODO: Don't override the value of the analog signal TXON */
	temp |= 0x0 << 12;
	/* TODO: Dont use the register to override the clock lane pos */
	temp |= 0x0 << 10;
	/* TODO: Analog characterization For now putting it to 0*/
	temp |= 0x0 << 0;
	OUTREG32(phy_base + HDMI_PHY_ANG_INT_CTRL_OFF, temp);

	/* Digital Interface Control */
	temp = 0;
	/* TODO: Don't use this register for data output */
	temp |= 0x0 << 31;
	OUTREG32(phy_base + HDMI_PHY_DATA_INT_CTRL_OFF, temp);

	/* BIST register */
	temp = 0;
	/* TODO: Don't use the LDO bist conrtol */
	temp |= 0x0 << 31;
	/* TODO: Don't use LB mode */
	temp |= 0x0 << 27;
	/* TODO: Don't use  the LB LANE SEL */
	temp |= 0x0 << 24;
	OUTREG32(phy_base + HDMI_PHY_BIST_OFF, temp);
#endif
exit:
	count = 0;
	while (count++ < 1000)
		;
	return rtn_value;
}

// Centaurus  end
#endif 

/*
 * Configure the wrapper with debouce data packing modes, timming
 *	parameters if operating as a master require timming generator also
 */
static int configure_wrapper(struct instance_cfg *inst_context)
{
	volatile UINT32 temp;
	UINT32 wp_base_addr = 0x0;
	int rtn_value = 0x0;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_wrapper\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));

	wp_base_addr = inst_context->wp_base_addr;
	/* Step 0 - Tweak if required */
	temp = ((inst_context->config.wp_config.debounce_rcv_detect <<
				HDMI_WP_DEBOUNCE_RXDET_SHIFT) &
			HDMI_WP_DEBOUNCE_RXDET_MASK);

	temp |= ((inst_context->config.wp_config.debounce_rcv_sens <<
				HDMI_WP_DEBOUNCE_LINE5VSHORT_SHIFT) &
			HDMI_WP_DEBOUNCE_LINE5VSHORT_MASK);

	OUTREG32((wp_base_addr + HDMI_WP_DEBOUNCE_OFFSET), temp);

	/* Dividing the 48MHz clock to 2 MHz for CEC and OCP different dividor */
	temp = INREG32(wp_base_addr + HDMI_WP_CLK_OFFSET);
	temp |= 0x00000218u;
	OUTREG32((wp_base_addr + HDMI_WP_CLK_OFFSET), temp);

	/* Following steps only applicable for a master generating the timmings
	   signal to core */
	if (inst_context->config.wp_config.is_slave_mode == 0x0) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Configuring as Master\r\n")));
		temp =
			((inst_context->config.wp_config.
			  hbp << HDMI_WP_VIDEO_TIMING_H_HBP_SHIFT) &
			 HDMI_WP_VIDEO_TIMING_H_HBP_MASK);
		temp |=
			((inst_context->config.wp_config.
			  hfp << HDMI_WP_VIDEO_TIMING_H_HFP_SHIFT) &
			 HDMI_WP_VIDEO_TIMING_H_HFP_MASK);
		temp |=
			((inst_context->config.wp_config.
			  hsw << HDMI_WP_VIDEO_TIMING_H_HSW_SHIFT) &
			 HDMI_WP_VIDEO_TIMING_H_HSW_MASK);

		OUTREG32((wp_base_addr +
				 HDMI_WP_VIDEO_TIMING_H_OFFSET),
                 temp);

		temp = ((inst_context->config.wp_config.vbp <<
					HDMI_WP_VIDEO_TIMING_V_VBP_SHIFT) &
				HDMI_WP_VIDEO_TIMING_V_VBP_MASK);
		temp |= ((inst_context->config.wp_config.vfp <<
					HDMI_WP_VIDEO_TIMING_V_VFP_SHIFT) &
				HDMI_WP_VIDEO_TIMING_V_VFP_MASK);
		temp |= ((inst_context->config.wp_config.vsw <<
					HDMI_WP_VIDEO_TIMING_V_VSW_SHIFT) &
				HDMI_WP_VIDEO_TIMING_V_VSW_MASK);
		OUTREG32((wp_base_addr +
				 HDMI_WP_VIDEO_TIMING_V_OFFSET),
                 temp);

		temp = INREG32
			(wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET);
		if (inst_context->config.wp_config.vSync_pol != 0x0) {
			temp |= HDMI_WP_VIDEO_CFG_VSYNC_POL_MASK;
		} else {
			temp &= ~(HDMI_WP_VIDEO_CFG_VSYNC_POL_MASK);
		}
		if (inst_context->config.wp_config.hSync_pol != 0x0) {
			temp |= HDMI_WP_VIDEO_CFG_HSYNC_POL_MASK;
		} else {
			temp &= ~(HDMI_WP_VIDEO_CFG_HSYNC_POL_MASK);
		}
		OUTREG32((wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET),
                 temp);
	}

	temp = INREG32(wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET);
	temp &= (~(HDMI_WP_VIDEO_CFG_PACKING_MODE_MASK));
	temp |= ((inst_context->config.wp_config.pack_mode <<
				HDMI_WP_VIDEO_CFG_PACKING_MODE_SHIFT) &
			HDMI_WP_VIDEO_CFG_PACKING_MODE_MASK);

	/* Invert if required - follows input otherwise */
	if (inst_context->config.wp_config.is_vSync_pol_inv != 0x0) {
		temp |= HDMI_WP_VIDEO_CFG_CORE_VSYNC_INV_MASK;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_CORE_VSYNC_INV_MASK));
	}
	if (inst_context->config.wp_config.is_hSync_pol_inv != 0x0) {
		temp |= HDMI_WP_VIDEO_CFG_CORE_HSYNC_INV_MASK;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_CORE_HSYNC_INV_MASK));
	}

	if (inst_context->is_interlaced == TRUE) {
		temp |= HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK));
	}

	if (inst_context->config.wp_config.is_slave_mode == 0x0) {
		temp |= (HDMI_WP_VIDEO_CFG_MODE_MASK);
		temp |= inst_context->config.wp_config.width;
	} else {
		temp &= (~(HDMI_WP_VIDEO_CFG_MODE_MASK));
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Operating as slave\r\n")));
	}
	OUTREG32((wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET), temp);

    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_wrapper<<<<\r\n")));
	return (rtn_value);
}


/*
 * Configures the interface between the wrapper and core.
 *
 * The number of lines/channel between in the core and the wrapper is not
 * configureable option.
 */
static int configure_core_input(struct instance_cfg *inst_context)
{
	volatile UINT32 temp;
	volatile UINT32 core_addr;
	struct hdmi_core_input_cfg *cfg = NULL;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_core_input\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));
	cfg = &(inst_context->config.core_config);
	core_addr = inst_context->core_base_addr;
	/*
	 * Step 1. Configure the width of the input bus.
	 * Step 2. Configure the sources for sync signals
	 *		   if hdmi_extract_syncs - VID_MODE.SYNCEX = 1
	 *		   if hdmi_generate_de - DE_CTRL.DE_GEN = 1 and de_top
	 *		   de_dly, etc...
	 *		   if hdmi_source_syncs - SYS_CTRL1.VEN/HEN = 1
	 * Step 3. Configure the edge to latch on.
	 */
	temp = INREG32(core_addr + HDMI_CORE_VID_ACEN_OFFSET);
	temp &= (~(HDMI_VID_ACEN_WIDE_BUS_MASK));
	temp |= ((cfg->data_bus_width << HDMI_VID_ACEN_WIDE_BUS_SHIFT) &
			HDMI_VID_ACEN_WIDE_BUS_MASK);
	OUTREG32((core_addr + HDMI_CORE_VID_ACEN_OFFSET), temp);

	temp = INREG32(core_addr + HDMI_CORE_SYS_CTRL1_OFFSET);
	temp &= (~(HDMI_SYS_CTRL1_BSEL_MASK | HDMI_SYS_CTRL1_EDGE_MASK));
	if (cfg->edge_pol != 0x0)
		temp |= HDMI_SYS_CTRL1_EDGE_MASK;

	temp |= HDMI_SYS_CTRL1_BSEL_MASK;
	OUTREG32((core_addr + HDMI_CORE_SYS_CTRL1_OFFSET), temp);

	if (cfg->sync_gen_cfg == hdmi_extract_syncs) {
		temp = INREG32(core_addr + HDMI_CORE_VID_MODE_OFFSET);
		temp &= (~(HDMI_VID_MODE_SYNCEX_MASK));
		temp |= HDMI_VID_MODE_SYNCEX_MASK;
		OUTREG32((core_addr + HDMI_CORE_VID_MODE_OFFSET), temp);
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Embedded syncs\r\n")));
	} else if (cfg->sync_gen_cfg == hdmi_generate_de) {
		temp = INREG32(core_addr + HDMI_CORE_DE_CTRL_OFFSET);
		temp &= (~(HDMI_DE_CTRL_DE_GEN_MASK));
		temp |= HDMI_DE_CTRL_DE_GEN_MASK;
		OUTREG32((core_addr + HDMI_CORE_DE_CTRL_OFFSET), temp);

		OUTREG32((core_addr + HDMI_CORE_DE_DLY_OFFSET),
                 (cfg->de_delay_cfg.
					DE_DLY & HDMI_DE_DLY_DE_DLY_MASK));
		OUTREG32((core_addr + HDMI_CORE_DE_TOP_OFFSET),
                 (cfg->de_delay_cfg.
					DE_TOP & HDMI_DE_TOP_DE_TOP_MASK));
		OUTREG32((core_addr + HDMI_CORE_DE_CNTL_OFFSET),
                 (cfg->de_delay_cfg.
					DE_CNTL & HDMI_DE_CNTL_DE_CNT_MASK));
		OUTREG32((core_addr + HDMI_CORE_DE_CNTH_OFFSET),
                 (cfg->de_delay_cfg.
					DE_CNTH & HDMI_DE_CNTH_DE_CNT_MASK));
		OUTREG32((core_addr + HDMI_CORE_DE_LINL_OFFSET),
                 (cfg->de_delay_cfg.
					DE_LINL & HDMI_DE_LINL_DE_LIN_MASK));
		OUTREG32((core_addr + HDMI_CORE_DE_LINH_OFFSET),
                 (cfg->de_delay_cfg.
					DE_LINH & HDMI_DE_LINH_DE_LIN_MASK));
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Sync being generated\r\n")));
	} else {
		OUTREG32((core_addr + HDMI_CORE_DE_CTRL_OFFSET), 0x1u);
		temp = INREG32(core_addr + HDMI_CORE_SYS_CTRL1_OFFSET);
		temp |= HDMI_SYS_CTRL1_VEN_MASK;
		temp |= HDMI_SYS_CTRL1_HEN_MASK;
		OUTREG32((core_addr + HDMI_CORE_SYS_CTRL1_OFFSET), temp);
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Descrete syncs and being sourced\r\n")));
	}
	OUTREG32((core_addr + HDMI_CORE_IADJUST_OFFSET), 0x0u);

    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_core_input<<<<\r\n")));
	return (0x0);
}

/*
 *	Configure sub-blocks
 */
static int configure_core_data_path(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	volatile UINT32 tempVidAcen;
	volatile UINT32 tempVidMode;
	volatile UINT32 tempVidDither;
	volatile UINT32 temp;
	volatile UINT32 core_addr;
	struct hdmi_core_data_path *pathCfg = NULL;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_core_data_path\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));
	core_addr = inst_context->core_base_addr;

	tempVidAcen = INREG32(core_addr + HDMI_CORE_VID_ACEN_OFFSET);
	tempVidMode = INREG32(core_addr + HDMI_CORE_VID_MODE_OFFSET);

	pathCfg = &(inst_context->config.core_path_config);
	tempVidMode &= (~(HDMI_VID_MODE_UPSMP_MASK |
				HDMI_VID_MODE_CSC_MASK |
				HDMI_VID_MODE_RANGE_MASK |
				HDMI_VID_MODE_DITHER_MASK));

	tempVidAcen &= (~(HDMI_VID_ACEN_RGB_2_YCBCR_MASK |
				HDMI_VID_ACEN_RANGE_CMPS_MASK |
				HDMI_VID_ACEN_DOWN_SMPL_MASK |
				HDMI_VID_ACEN_RANGE_CLIP_MASK |
				HDMI_VID_ACEN_CLIP_CS_ID_MASK));
	if (pathCfg->up_sampler_enable != 0x0)
		tempVidMode |= HDMI_VID_MODE_UPSMP_MASK;

	if (pathCfg->csc_YCbCr_2_RGB_enable != 0x0) {
		tempVidMode |= HDMI_VID_MODE_CSC_MASK;
		rtn_value = configure_csc_ycbcr_rgb(inst_context);
		if (rtn_value != 0x0)
			goto exit_this_func;
	}
	temp = INREG32(core_addr + HDMI_CORE_VID_CTRL_OFFSET);
	if (pathCfg->csc_convert_standard != 0x0)
		temp |= HDMI_VID_CTRL_CSCSEL_MASK;
	else
		temp &= (~(HDMI_VID_CTRL_CSCSEL_MASK));
	OUTREG32((core_addr + HDMI_CORE_VID_CTRL_OFFSET), temp);

	if (pathCfg->range_exp_RGB_enable != 0x0)
		tempVidMode |= HDMI_VID_MODE_RANGE_MASK;

	if (pathCfg->dither_enable != 0x0) {
		tempVidDither =
			INREG32(core_addr + HDMI_CORE_VID_DITHER_OFFSET);
		tempVidMode |= HDMI_VID_MODE_DITHER_MASK;
		tempVidDither &= (~(HDMI_VID_DITHER_M_D2_MASK |
					HDMI_VID_DITHER_UP2_MASK |
					HDMI_VID_DITHER_STR_422_EN_MASK |
					HDMI_VID_DITHER_D_BC_EN_MASK |
					HDMI_VID_DITHER_D_GC_EN_MASK |
					HDMI_VID_DITHER_D_RC_EN_MASK |
					HDMI_VID_DITHER_DRD_MASK));
		/* Configure dithering parameters */
		if (pathCfg->dither_config.M_D2 != 0x0)
			tempVidDither |= HDMI_VID_DITHER_M_D2_MASK;
		if (pathCfg->dither_config.UP2 != 0x0)
			tempVidDither |= HDMI_VID_DITHER_UP2_MASK;
		if (pathCfg->dither_config.STR_422_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_STR_422_EN_MASK;
		if (pathCfg->dither_config.D_BC_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_D_BC_EN_MASK;
		if (pathCfg->dither_config.D_GC_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_D_GC_EN_MASK;
		if (pathCfg->dither_config.D_RC_EN != 0x0)
			tempVidDither |= HDMI_VID_DITHER_D_RC_EN_MASK;
		if (pathCfg->dither_config.DRD != 0x0)
			tempVidDither |= HDMI_VID_DITHER_DRD_MASK;
		OUTREG32((core_addr + HDMI_CORE_VID_DITHER_OFFSET),
                 tempVidDither);
	}

	tempVidMode |=
		((pathCfg->output_width << HDMI_VID_MODE_DITHER_MODE_SHIFT) &
		 HDMI_VID_MODE_DITHER_MODE_MASK);
	OUTREG32((core_addr + HDMI_CORE_VID_MODE_OFFSET),
             tempVidMode);

	if (pathCfg->cscRGB_2_YCbCr_enable != 0x0)
		tempVidAcen |= HDMI_VID_ACEN_RGB_2_YCBCR_MASK;

	if (pathCfg->range_comp_enable != 0x0)
		tempVidAcen |= HDMI_VID_ACEN_RANGE_CMPS_MASK;

	if (pathCfg->down_sampler_enable != 0x0)
		tempVidAcen |= HDMI_VID_ACEN_DOWN_SMPL_MASK;

	if (pathCfg->range_clip_enable != 0x0) {
		tempVidAcen |= HDMI_VID_ACEN_RANGE_CLIP_MASK;
		if (pathCfg->clip_color_space != 0x0) {
			tempVidAcen |= HDMI_VID_ACEN_CLIP_CS_ID_MASK;
		}
	}
	OUTREG32((core_addr + HDMI_CORE_VID_ACEN_OFFSET),
             tempVidAcen);

exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_core_data_path<<<<\r\n")));
	return (rtn_value);
}

static int configure_core(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	volatile UINT32 temp;
	volatile UINT32 core_addr;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_core\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));
	core_addr = inst_context->core_base_addr;

	temp = INREG32(core_addr + HDMI_CORE_TEST_TXCTRL_OFFSET);
	temp &= (~(HDMI_TEST_TXCTRL_DIV_ENC_BYP_MASK));
	OUTREG32((core_addr + HDMI_CORE_TEST_TXCTRL_OFFSET), temp);

	if (inst_context->config.use_core_config != 0x0) {
		rtn_value = configure_core_input(inst_context);
		if (rtn_value != 0x0)
			goto exit_this_func;
	}
	if (inst_context->config.use_core_path_config != 0x0) {
		rtn_value = configure_core_data_path(inst_context);
		if (rtn_value != 0x0)
			goto exit_this_func;
	}
	rtn_value = configure_policies(inst_context);
	if (rtn_value != 0x0)
		goto exit_this_func;

	OUTREG32((core_addr + HDMI_CORE_ACR_CTRL_OFFSET), 0x0);

exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_core<<<<\r\n")));
	return (rtn_value);
}

static int configure_policies(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	volatile UINT32 temp;
	volatile UINT32 dither_mode_val;
	volatile UINT32 core_addr;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_policies\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));
	core_addr = inst_context->core_base_addr;

	temp = INREG32(core_addr + HDMI_CORE_VID_CTRL_OFFSET);

	/* No pixel repeatation by default */
	temp &= (~(HDMI_VID_CTRL_ICLK_MASK));

	rtn_value = determine_pixel_repeatation(inst_context);
	if (rtn_value == HDMI_PIXEL_REPLECATED_ONCE) {
		temp |= (((0x01u) << HDMI_VID_CTRL_ICLK_SHIFT) &
				HDMI_VID_CTRL_ICLK_MASK);
	} else if (rtn_value == HDMI_PIXEL_REPLECATED_FOUR_TIMES) {
		temp |= HDMI_VID_CTRL_ICLK_MASK;
	} else if (rtn_value == 0x0) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: No Pixel repeatation required\r\n")));
	} else {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not determine pixel that would be required\r\n")));
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}
	OUTREG32((core_addr + HDMI_CORE_VID_CTRL_OFFSET), temp);


	temp = INREG32(core_addr + HDMI_CORE_HDMI_CTRL_OFFSET);

	temp &=
		(~
		 (HDMI_HDMI_CTRL_DC_EN_MASK |
		  HDMI_HDMI_CTRL_PACKET_MODE_MASK));

	dither_mode_val =
		INREG32(core_addr + HDMI_CORE_VID_MODE_OFFSET);
	dither_mode_val =
		((dither_mode_val & HDMI_VID_MODE_DITHER_MODE_MASK)
		 >> HDMI_VID_MODE_DITHER_MODE_SHIFT);

	if (dither_mode_val != HDMI_VID_MODE_DITHER_TO_24_BITS_MODE) {
		temp |= HDMI_HDMI_CTRL_DC_EN_MASK;
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Deep color mode\r\n")));
	}
	temp |= ((HDMI_CTRL_PACKET_MODE_24BITS_PIXEL) <<
			HDMI_HDMI_CTRL_PACKET_MODE_SHIFT);
	if (dither_mode_val == HDMI_VID_MODE_DITHER_TO_30_BITS_MODE) {
		temp |= ((HDMI_CTRL_PACKET_MODE_30BITS_PIXEL) <<
				HDMI_HDMI_CTRL_PACKET_MODE_SHIFT);
	}
	if (dither_mode_val == HDMI_VID_MODE_DITHER_TO_36_BITS_MODE) {
		temp |= ((HDMI_CTRL_PACKET_MODE_36BITS_PIXEL) <<
				HDMI_HDMI_CTRL_PACKET_MODE_SHIFT);
	}
	/* TODO DVI mode is required - make this configureable also */
	temp |= HDMI_HDMI_CTRL_HDMI_MODE_MASK;

	OUTREG32((core_addr + HDMI_CORE_HDMI_CTRL_OFFSET), temp);

	temp = INREG32(core_addr + HDMI_CORE_TMDS_CTRL_OFFSET);

	temp |= (HDMI_TMDS_CTRL_TCLKSEL_MASK &
			(HDMI_TMDS_CTRL_IP_CLOCK_MULTIPLIER_AUDIO <<
			 HDMI_TMDS_CTRL_TCLKSEL_SHIFT));
	OUTREG32((core_addr + HDMI_CORE_TMDS_CTRL_OFFSET), temp);

exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_policies<<<<\r\n")));
	return (rtn_value);
}


static int configure_ctrl_packets(struct instance_cfg *inst_context)
{
	volatile UINT32 temp;

	temp = INREG32((inst_context->core_base_addr) +
			HDMI_CORE_DC_HEADER_OFFSET);
	temp = 0x03;
	OUTREG32(((inst_context->core_base_addr) +
			 HDMI_CORE_DC_HEADER_OFFSET), 
             temp);

	OUTREG32(((inst_context->core_base_addr) +
			 HDMI_CORE_CP_BYTE1_OFFSET), 
             HDMI_CP_BYTE1_SETAVM_MASK);

	return (0x0);
}


static int configure_avi_info_frame(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	UINT8 check_sum = 0x0;
	UINT8 byte_index = 0x0;
	volatile UINT8 data_byte = 0x0;
	volatile UINT32 dbyte_base;
	struct hdmi_avi_frame_cfg *infoPkt = NULL;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_avi_info_frame\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));

	infoPkt = &(inst_context->config.info_frame_config.aviData);
	dbyte_base = (UINT32) (inst_context->core_base_addr +
			HDMI_CORE_AVI_DBYTE_BASE_OFFSET);
	data_byte = (UINT8)
		(HDMI_AVI_TYPE_AVI_TYPE_MASK & HDMI_AVI_INFOFRAME_PKT_TYPE);
	OUTREG32(((inst_context->core_base_addr) +
			 HDMI_CORE_AVI_TYPE_OFFSET), 
             data_byte);
	check_sum = HDMI_AVI_INFOFRAME_PKT_TYPE;

	data_byte = (UINT8)
		(HDMI_AVI_VERS_AVI_VERS_MASK & HDMI_AVI_INFOFRAME_PKT_VER);
	OUTREG32(((inst_context->core_base_addr) +
			 HDMI_CORE_AVI_VERS_OFFSET), 
             data_byte);
	check_sum += HDMI_AVI_INFOFRAME_PKT_VER;

	data_byte = (UINT8)
		(HDMI_AVI_LEN_AVI_LEN_MASK & HDMI_AVI_INFOFRAME_PKT_LEN);
	OUTREG32(((inst_context->core_base_addr) +
			 HDMI_CORE_AVI_LEN_OFFSET), 
             data_byte);
	check_sum += HDMI_AVI_INFOFRAME_PKT_LEN;


	data_byte = (((UINT8) infoPkt->output_cs << 5) &
			HDMI_AVI_INFOFRAME_Y0_Y1_MASK);
	if (infoPkt->use_active_aspect_ratio == TRUE) {
		data_byte |= HDMI_AVI_INFOFRAME_A0_MASK;
	}
	/* Bar information B0 and B1 - if so require to update byte 6-13 */
	if (infoPkt->bar_info.barInfoValid != 0x0) {
		data_byte |= ((((UINT8) infoPkt->bar_info.barInfoValid) <<
					2) & HDMI_AVI_INFOFRAME_B0_B1_MASK);
	}
	data_byte |= (((UINT8) infoPkt->scan_info) &
			HDMI_AVI_INFOFRAME_S0_S1_MASK);

	/* First data byte of the packet */
	OUTREG32((dbyte_base + byte_index), data_byte);
	byte_index += 0x4;
	check_sum += data_byte;

	data_byte = (((UINT8) infoPkt->colorimetry_info << 6) &
			HDMI_AVI_INFOFRAME_C0_C1_MASK);

	data_byte |= (((UINT8) infoPkt->aspect_ratio << 4) &
			HDMI_AVI_INFOFRAME_M0_M1_MASK);
	if (infoPkt->use_active_aspect_ratio == TRUE) {
		data_byte |= (((UINT8) infoPkt->active_aspect_ratio) &
				HDMI_AVI_INFOFRAME_R0_R3_MASK);
	}

	/* Second data byte of the packet */
	OUTREG32((dbyte_base + byte_index), data_byte);
	byte_index += 0x4;
	check_sum += data_byte;

	data_byte = 0x0;
	if (infoPkt->it_content_present != 0x0) {
		data_byte = HDMI_AVI_INFOFRAME_ITC_MASK;
	}
	/* Extended colorimetry range EC3 to EC0 */
	data_byte |= (((UINT8) infoPkt->ext_colorimetry << 4) &
			HDMI_AVI_INFOFRAME_EC2_EC0_MASK);
	/* Quantization range range Q1 to Q0 */
	data_byte |= (((UINT8) infoPkt->quantization_range << 2)
			& HDMI_AVI_INFOFRAME_Q1_Q0_MASK);
	/* Non-Uniform scaling S0 and S1 */
	data_byte |= ((UINT8) infoPkt->non_uniform_sc &
			HDMI_AVI_INFOFRAME_SC1_SC0_MASK);
	/* Third data byte of the packet */
	OUTREG32((dbyte_base + byte_index), data_byte);
	byte_index += 0x4;
	check_sum += data_byte;
	/* Fourth data byte of the packet */
	switch (inst_context->config.display_mode) {
		case hdmi_720P_60_mode:
			infoPkt->format_identier = 4u;
			break;
		case hdmi_1080P_30_mode:
			infoPkt->format_identier = 34u;
			break;
		case hdmi_1080I_60_mode:
			infoPkt->format_identier = 5u;
			break;
		case hdmi_1080P_60_mode:
			infoPkt->format_identier = 16u;
			break;
		default:
			rtn_value = -EINVAL ;
			goto exit_this_func;
	}

	data_byte = (UINT8) infoPkt->format_identier;

	OUTREG32((dbyte_base + byte_index),
             ((UINT8) data_byte &
				HDMI_AVI_INFOFRAME_VIC6_VIC0_MASK));
	byte_index += 0x4;
	check_sum += data_byte;

	/* Pixel Repeatation */
	data_byte = (UINT8) (HDMI_VID_CTRL_ICLK_MASK &
			INREG32(inst_context->core_base_addr +
				HDMI_CORE_VID_CTRL_OFFSET));

	/* TODO - Why do we require to up the pixel repeatation when demux is
	   is used. */
	if ((INREG32(inst_context->core_base_addr +
					HDMI_CORE_VID_MODE_OFFSET) &
				HDMI_VID_MODE_DEMUX_MASK) == HDMI_VID_MODE_DEMUX_MASK){
		/* Do not worry about exceeding the upper limit.
		   Pixel could be repeated a maximum of 4 times (value 0x03).
		   The pixel repeatation has 4 bit space in info packet which could
		   be a maximum of 0x0F, but limited to 0x09 */
		data_byte++;
	}
	OUTREG32((dbyte_base + byte_index),
            (HDMI_AVI_INFOFRAME_PR3_PR0_MASK & data_byte));
	byte_index += 0x4;
	check_sum += data_byte;

	if (infoPkt->bar_info.barInfoValid != 0x0) {
		data_byte = (UINT8) (infoPkt->bar_info.topBar & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(UINT8) ((infoPkt->bar_info.topBar >> 8) & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;

		data_byte = (UINT8) (infoPkt->bar_info.bottomBar & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(UINT8) ((infoPkt->bar_info.bottomBar >> 8) & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;

		data_byte = (UINT8) (infoPkt->bar_info.leftBar & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(UINT8) ((infoPkt->bar_info.leftBar >> 8) & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;

		data_byte = (UINT8) (infoPkt->bar_info.rightBar & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;
		data_byte =
			(UINT8) ((infoPkt->bar_info.rightBar >> 8) & 0xFF);
		OUTREG32((dbyte_base + byte_index), data_byte);
		byte_index += 0x4;
		check_sum += data_byte;
	}

	OUTREG8((inst_context->core_base_addr + HDMI_CORE_AVI_CHSUM_OFFSET),
            (UINT8) (HDMI_AVI_INFOFRAME_CONST_0x100 - (UINT16) check_sum));

    PRINTMSG(ZONE_INIT, (_T("HDMIL: AVI - Computed check sum %d\r\n"), check_sum));
    PRINTMSG(ZONE_INIT, (_T("HDMIL: Check sum sent %d\r\n"), 
			INREG32(inst_context->core_base_addr +
				HDMI_CORE_AVI_CHSUM_OFFSET)));
    PRINTMSG(ZONE_INIT, (_T("HDMIL: Sent check sum + all bytes should 0x0\r\n")));
exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_avi_info_frame<<<\r\n")));
	return (rtn_value);
}

static int configure_csc_ycbcr_rgb(struct instance_cfg *inst_context)
{
	struct hdmi_csc_YCbCr_2_RGB_ctrl *ctrl = NULL;
	volatile UINT32 temp;
	volatile UINT32 core_addr;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>configure_csc_ycbcr_rgb\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));

	core_addr = inst_context->core_base_addr;
	ctrl =
		&(inst_context->config.core_path_config.
				csc_YCbCr_2_RGB_config);

	temp = INREG32(core_addr + HDMI_CORE_XVYCC2RGB_CTL_OFFSET);
	temp &= (~(HDMI_XVYCC2RGB_CTL_EXP_ONLY_MASK |
				HDMI_XVYCC2RGB_CTL_BYP_ALL_MASK |
				HDMI_XVYCC2RGB_CTL_SW_OVR_MASK |
				HDMI_XVYCC2RGB_CTL_FULLRANGE_MASK |
				HDMI_XVYCC2RGB_CTL_XVYCCSEL_MASK));
	if (ctrl->enableRngExp != 0x0)
		temp |= HDMI_XVYCC2RGB_CTL_EXP_ONLY_MASK;

	if (ctrl->enableFullRngExp != 0x0)
		temp |= HDMI_XVYCC2RGB_CTL_FULLRANGE_MASK;

	if (ctrl->srcCsSel != 0x0)
		temp |= HDMI_XVYCC2RGB_CTL_XVYCCSEL_MASK;

	if (ctrl->customCoEff != 0x0) {
		/* Load the custom coefficitents - using memcopy to load as the
		   structures maps to register */
		memcpy((void *) (core_addr +
					HDMI_CORE_Y2R_COEFF_LOW_OFFSET),
				((const void *) &(ctrl->coEff)),
				sizeof(struct hdmi_csc_YCbCr_2_RGB_coeff));
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Using custom co-effs\r\n")));
	}
	OUTREG32((core_addr + HDMI_CORE_XVYCC2RGB_CTL_OFFSET), temp);

    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_csc_ycbcr_rgb<<<<\r\n")));
	return (0x0);
}

static int validate_info_frame_cfg(struct hdmi_info_frame_cfg *config)
{
	int rtn_value = -EFAULT ;
	struct hdmi_avi_frame_cfg *aviData = NULL;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>validate_info_frame_cfg\r\n")));

	if (config == NULL)
		goto exit_this_func;

	aviData = &(config->aviData);
	if (aviData->output_cs >= hdmi_avi_max_op_cs) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: In correct color space\r\n")));
		goto exit_this_func;
	}
	if ((aviData->use_active_aspect_ratio != hdmi_avi_no_aspect_ratio)
			&& (aviData->use_active_aspect_ratio !=
				hdmi_avi_active_aspect_ratio)) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Wrong aspect ratio\r\n")));
		goto exit_this_func;
	}
	if (aviData->scan_info >= hdmi_avi_max_scan) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: In correct scan info\r\n")));
		goto exit_this_func;
	}
	if (aviData->colorimetry_info >= hdmi_avi_max_colorimetry) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Wrong colorimetry info\r\n")));
		goto exit_this_func;
	}
	if (aviData->aspect_ratio >= hdmi_avi_aspect_ratio_max) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Wrong aspect ratio info\r\n")));
		goto exit_this_func;
	}
	if ((aviData->active_aspect_ratio <
				hdmi_avi_active_aspect_ratio_same)
			&& (aviData->active_aspect_ratio >= hdmi_avi_aspect_ratio_max)) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Wrong active aspect ratio info\r\n")));
		goto exit_this_func;
	}
	if (aviData->non_uniform_sc >= hdmi_avi_non_uniform_scaling_max) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: In correct non-uniform scaling info\r\n")));
		goto exit_this_func;
	}
	rtn_value = 0x0;

exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: validate_info_frame_cfg<<<<\r\n")));
	return (rtn_value);
}

static int validate_core_config(struct hdmi_core_input_cfg *config)
{
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>validate_core_config\r\n")));
	HDMI_ARGS_CHECK((config != NULL));

	if (config->data_bus_width > hdmi_10_bits_chan_width) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Bus width should be <=30 bits/pixel\r\n")));
		return (-EFAULT );
	}

	if (config->sync_gen_cfg >= hdmi_max_syncs) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Incorrect meathods used for synchronization\r\n")));
		return (-EFAULT );
	}

    PRINTMSG(ZONE_INIT, (_T("HDMIL: validate_core_config<<<<\r\n")));
	return (0x0);
}

static int validate_wp_cfg(struct hdmi_wp_config *config)
{
	int rtn_value = -EFAULT ;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>validate_wp_cfg\r\n")));

	if ((config->debounce_rcv_detect < 0x01) ||
			(config->debounce_rcv_detect > 0x14)) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Debounce receiver detect incorrect\r\n")));
		goto exit_this_func;
	}
	if ((config->debounce_rcv_sens < 0x01) ||
			(config->debounce_rcv_sens > 0x14)) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Debounce receiver sens incorrect\r\n")));
		goto exit_this_func;
	}
	if (config->is_slave_mode == 0x0) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Warpper is not in SLAVE mode\r\n")));
        PRINTMSG(ZONE_INIT, (_T("HDMIL:  - Master mode cannot be supported\r\n")));
		goto exit_this_func;
	}
	if (config->width >= hdmi_12_bits_chan_width) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Bus width should be < 36 bits/pixel\r\n")));
        PRINTMSG(ZONE_INIT, (_T("HDMIL:  - 8 & 10 bits/channel is valid\r\n")));
		goto exit_this_func;
	}
	if (config->pack_mode >= hdmi_wp_no_pack) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Incorrect data packing mode\r\n")));
		goto exit_this_func;
	}
	rtn_value = 0x0;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: validate_wp_cfg<<<<\r\n")));

exit_this_func:
	return (rtn_value);
}

static int validate_path_config(struct hdmi_core_data_path *config)
{
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>validate_path_config\r\n")));
	HDMI_ARGS_CHECK((config != NULL));
	if (config->output_width >= hdmi_max_bits_chan_width) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: In valid output channel width\r\n"), 
                 config->output_width));
		return (-EFAULT );
	}
    PRINTMSG(ZONE_INIT, (_T("HDMIL: validate_path_config<<<<\r\n")));
	return (0x0);
}


static int check_copy_config(struct instance_cfg *inst_cntxt,
		struct hdmi_cfg_params *config)
{
	int rtn_value = 0x0;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>check_copy_config\r\n")));

	if (config->use_display_mode != 0x0) {
		if (config->display_mode >= hdmi_max_mode) {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Incorrect mode id\r\n")));
			rtn_value = -EINVAL ;
			goto exit_this_func;
		}
		inst_cntxt->config.display_mode = config->display_mode;
	}
	if (config->use_wp_config != 0x0) {
		rtn_value = validate_wp_cfg(&(config->wp_config));
		if (rtn_value != 0x0) {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Wrapper config incorrect\r\n")));
			goto exit_this_func;
		}
		memcpy((void *) (&(inst_cntxt->config.wp_config)),
				((const void *) &(config->wp_config)),
				sizeof(struct hdmi_wp_config));
	}
	if (config->use_core_config != 0x0) {
		rtn_value = validate_core_config(&(config->core_config));
		if (rtn_value != 0x0) {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Core config incorrect\r\n")));
			goto exit_this_func;
		}
		memcpy((void *) (&(inst_cntxt->config.core_config)),
				((const void *) &(config->core_config)),
				sizeof(struct hdmi_core_input_cfg));
	}
	if (config->use_core_path_config != 0x0) {
		rtn_value =
			validate_path_config(&(config->core_path_config));
		if (rtn_value != 0x0) {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Core data path config incorrect\r\n")));
			goto exit_this_func;
		}
		memcpy((void *) (&(inst_cntxt->config.core_path_config)),
				((const void *) &(config->core_path_config)),
				sizeof(struct hdmi_core_data_path));
	}
	if (config->use_info_frame_config != 0x0) {
		if (config->info_frame_config.use_avi_info_data != 0x0) {
			rtn_value = validate_info_frame_cfg
				(&(config->info_frame_config));
			if (rtn_value != 0x0) {
                PRINTMSG(ZONE_INIT, (_T("HDMIL: Bad AVI Info frame data\r\n")));
				goto exit_this_func;
			}
			memcpy((void
						*) (&(inst_cntxt->config.
								info_frame_config)),
					((const void *)
					 &(config->info_frame_config)),
					sizeof(struct hdmi_info_frame_cfg));
		}
	}
    PRINTMSG(ZONE_INIT, (_T("HDMIL: check_copy_config<<<< \r\n")));

exit_this_func:
	return (rtn_value);
}

static int determine_pixel_repeatation(struct instance_cfg *inst_context)
{
	int rtn_value = 0x0;
	UINT32 mPixelPerSec = 0x0;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>determine_pixel_repeatation\r\n")));
	HDMI_ARGS_CHECK((inst_context != NULL));

	switch (inst_context->config.display_mode) {
		case hdmi_ntsc_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_NTSC;
			inst_context->is_interlaced = TRUE;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: NTSC Standard\r\n")));
			break;
		case hdmi_pal_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_PAL;
			inst_context->is_interlaced = TRUE;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: PAL Standard\r\n")));
			break;
		case hdmi_720P_60_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_720P60;
			inst_context->is_interlaced = FALSE;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: 720P60 format\r\n")));
			break;
		case hdmi_1080P_60_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_1080P60;
			inst_context->is_interlaced = FALSE;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: 1080P60 format\r\n")));
			break;
		case hdmi_1080P_30_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_1080P30;
			inst_context->is_interlaced = FALSE;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: 1080P30 format\r\n")));
			break;
		case hdmi_1080I_60_mode:
			mPixelPerSec = HDMI_VIDEO_STAND_1080I60;
			inst_context->is_interlaced = FALSE;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: 1080I60 format\r\n")));
			break;
		default:
			/* This should not happen */
			rtn_value = -EINVAL ;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: The display format is not supported\r\n")));
			break;
	}
	if (rtn_value == 0x0) {
		if (mPixelPerSec < HDMI_MINIMUM_PIXELS_SEC) {
			if ((mPixelPerSec * HDMI_PIXEL_REPLECATED_ONCE) >=
					HDMI_MINIMUM_PIXELS_SEC) {
				rtn_value = HDMI_PIXEL_REPLECATED_ONCE;
                PRINTMSG(ZONE_INIT, (_T("HDMIL: Pixel Repeating 1 time\r\n")));
				goto exit_this_func;
			}

			if ((mPixelPerSec *
						HDMI_PIXEL_REPLECATED_FOUR_TIMES) >=
					HDMI_MINIMUM_PIXELS_SEC) {
				rtn_value =
					HDMI_PIXEL_REPLECATED_FOUR_TIMES;
                PRINTMSG(ZONE_INIT, (_T("HDMIL: Pixel Repeating 4 time\r\n")));
				goto exit_this_func;
			}
			/* We could not still meet the HDMI needs - let the
			   caller know */
			rtn_value = -EINVAL ;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Resolution too low Could not reach 25 MHz\r\n")));
			goto exit_this_func;
		}
	}

exit_this_func:

    PRINTMSG(ZONE_INIT, (_T("HDMIL: determine_pixel_repeatation<<<<\r\n")));
	return (rtn_value);
}


#ifndef CONFIG_ARCH_TI816X
// Centaurus begin
int get_phy_status(struct instance_cfg *inst_context,
		struct ti81xxhdmi_phy_status *stat)
{
	int rtn_value = 0;
	int phy_base;
	UINT32 temp;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>get_phy_status\r\n")));
	phy_base = inst_context->phy_base_addr;
	if (!stat)
	{
		rtn_value = -EFAULT;
		goto exit_this_func;
	}

	temp = INREG32(phy_base + HDMI_PHY_PWR_CTRL_OFF);
	stat->rst_done_pclk = (temp & HDMI_PHY_RESETDONEPIXELCLK_MASK) <<
		HDMI_PHY_RESETDONEPIXELCLK_SHIFT;
	stat->rst_done_pwrclk = (temp & HDMI_PHY_RESETDONEPWRCLK_MASK) <<
		HDMI_PHY_RESETDONEPWRCLK_SHIFT;
	stat->rst_done_scpclk = (temp & HDMI_PHY_RESETDONESCPCLK_MASK) <<
		HDMI_PHY_RESETDONESCPCLK_SHIFT;
	stat->rst_done_refclk = (temp & HDMI_PHY_RESETDONEREFCLK_MASK) <<
		HDMI_PHY_RESETDONEREFCLK_SHIFT;
	temp = INREG32(phy_base + HDMI_PHY_PAD_CFG_CTRL_OFF);
	stat->dct_5v_short_clk = (temp & HDMI_PHY_DET5VSHT_CLK_MASK) <<
		HDMI_PHY_DET5VSHT_CLK_SHIFT;
	stat->rx_detect = (temp & HDMI_PHY_RXDET_LINE_MASK) >>
		HDMI_PHY_RXDET_LINE_SHIFT;
	stat->dct_5v_short_data = (temp & HDMI_PHY_DET5VSHT_DATA_MASK) >>
		HDMI_PHY_DET5VSHT_DATA_SHIFT;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: get_phy_status<<<<<\r\n")));
exit_this_func:
	return rtn_value;
}
// Centaurus end
#else
// Netra
int get_phy_status(struct instance_cfg *inst_context,
		struct ti81xxhdmi_phy_status *stat)
{
	return -EINVAL;
}
#endif


#ifdef CONFIG_ARCH_TI816X
// Netra begin
int enable_hdmi_clocks(UINT32 prcm_base)
{
	UINT32 temp, repeatCnt;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: HDMI Clk enable in progress\r\n")));
	temp = 2;
	/*Enable Power Domain Transition for HDMI */
	OUTREG32((prcm_base + CM_HDMI_CLKSTCTRL_OFF), temp);
	/*Enable HDMI Clocks*/
	OUTREG32((prcm_base + CM_ACTIVE_HDMI_CLKCTRL_OFF), temp);

	/*Check clocks are active*/
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + CM_HDMI_CLKSTCTRL_OFF)) >> 8;
		repeatCnt++;
	}
	while((temp != 0x3) &&(repeatCnt < VPS_PRCM_MAX_REP_CNT));

    if (temp != 0x3)
    {
        DEBUGMSG(ZONE_ERROR, (L"HDMIL: >>> ERROR: HDMI_CLKSTCTRL enable FAILED <<<\r\n"));
        return -EFAULT;
    }

	/* Check to see module is functional */
	repeatCnt = 0;
	do
	{
		temp = ((INREG32(prcm_base + CM_ACTIVE_HDMI_CLKCTRL_OFF) &
					0x70000)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < VPS_PRCM_MAX_REP_CNT));

    if (temp != 0)
    {
        DEBUGMSG(ZONE_ERROR, (L"HDMIL: >>> ERROR: ACTIVE_HDMI_CLKCTRL enable FAILED <<<\r\n"));
        return -EFAULT;
    }

    PRINTMSG(ZONE_INIT, (_T("HDMIL: HDMI Clocks enabled successfully\r\n")));
    return 0;
}
// Netra end
#else
// Centaurus begin
int enable_hdmi_clocks(UINT32 prcm_base)
{
	UINT32 temp, repeatCnt;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: HDMI Clk enable in progress, prcm_base 0x%08X\r\n"), prcm_base));

	/*Enable Power Domain Transition for HDMI */
	temp = 2;
	OUTREG32((prcm_base + CM_ALWON_SDIO_CLKCTRL), temp);
	/*Check clocks are active*/
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + CM_ALWON_SDIO_CLKCTRL)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < VPS_PRCM_MAX_REP_CNT));

    if(temp == 0) PRINTMSG(ZONE_INIT, (_T("HDMIL: CM_ALWON_SDIO_CLKCTRL enabled\r\n")));

    /* CM_HDVPSS_CLKCTRL */
	temp = 2;
	OUTREG32((prcm_base + 0x0800), temp);
	/*Check clocks are active*/
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + 0x0800)) >> 8;
		repeatCnt++;
	}
	while((temp != 1) && (repeatCnt < 200000 /*VPS_PRCM_MAX_REP_CNT*/));

    if(temp == 1) PRINTMSG(1, (_T("HDMIL: CM_HDVPSS_CLKCTRL enabled\r\n")));

    /* CM_HDVPSS_HDVPSS_CLKCTRL */
	temp = 2;
	OUTREG32((prcm_base + 0x0820), temp);
	/*Check clocks are active*/
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + 0x0820)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < 200000 /*VPS_PRCM_MAX_REP_CNT*/));

    if (temp == 0x0) PRINTMSG(1, (_T("HDMIL: CM_HDVPSS_HDVPSS_CLKCTRL enabled\r\n")));


    /* CM_HDVPSS_HDMI_CLKCTRL */
	temp = 2;
	OUTREG32((prcm_base + CM_HDMI_CLKCTRL_OFF), temp);
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + CM_HDMI_CLKCTRL_OFF)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < 200000 /*VPS_PRCM_MAX_REP_CNT*/));

    if (temp == 0) PRINTMSG(1, (_T("HDMIL: CM_HDVPSS_HDMI_CLKCTRL enabled\r\n")));


    /* Check CM_HDVPSS_CLKCTRL again */
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + 0x0800)) >> 8;
		repeatCnt++;
	}
	while((temp != 1) && (repeatCnt < 200000 /*VPS_PRCM_MAX_REP_CNT*/));

    if(temp != 1)
    {
        DEBUGMSG(ZONE_ERROR, (L"HDMIL: >>> ERROR: CM_HDVPSS_CLKCTRL enable FAILED <<<\r\n"));
        return -EFAULT;
    }
    else
        PRINTMSG(ZONE_INIT, (_T("HDMIL: CM_HDVPSS_CLKCTRL enabled\r\n")));

    /* Check CM_HDVPSS_HDVPSS_CLKCTRL again */
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + 0x0820)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < 200000 /*VPS_PRCM_MAX_REP_CNT*/));

    if (temp == 0x0)
    {
        DEBUGMSG(ZONE_ERROR, (L"HDMIL: >>> ERROR: CM_HDVPSS_HDVPSS_CLKCTRL enable FAILED <<<\r\n"));
        return -EFAULT;
    }
    else
        PRINTMSG(ZONE_INIT, (_T("HDMIL: CM_HDVPSS_HDVPSS_CLKCTRL enabled\r\n")));


    /*Deasserting resets: RM_HDVPSS_RSTCTRL */
	OUTREG32((prcm_base + 0x0E10), 0);

    /* Check CM_HDMI_CLKCTRL again */
	repeatCnt = 0;
	do
	{
		temp = (INREG32(prcm_base + CM_HDMI_CLKCTRL_OFF)) >> 16;
		repeatCnt++;
	}
	while((temp != 0) && (repeatCnt < 200000 /*VPS_PRCM_MAX_REP_CNT*/));

    if (temp != 0)
    {
        DEBUGMSG(ZONE_ERROR, (L"HDMIL: >>> ERROR: CM_HDVPSS_HDMI_CLKCTRL enable FAILED <<<\r\n"));
        return -EFAULT;
    }
    else
        PRINTMSG(ZONE_INIT, (_T("HDMIL: CM_HDVPSS_HDMI_CLKCTRL enabled\r\n")));


    PRINTMSG(ZONE_INIT, (_T("HDMIL: HDMI Clocks enabled successfully\r\n")));
    return 0;
}
// Centaurus end
#endif

#ifndef CONFIG_ARCH_TI816X
// Centaurus begin
static void configure_hdmi_pll(volatile UINT32  b_addr,
		UINT32 __n,
		UINT32 __m,
		UINT32 __m2,
		UINT32 clkctrl_val)
{
	UINT32 m2nval, mn2val, read_clkctrl;
	UINT32 read_m2nval, read_mn2val;
	volatile UINT32 repeatCnt = 0;
	/* Put PLL in idle bypass mode */
	read_clkctrl = INREG32(b_addr + HDMI_PLL_CLKCTRL_OFF);
    PRINTMSG(1, (_T("HDMI: >>> configure_hdmi_pll: read_clkctrl=0x%08X <<<\r\n"), read_clkctrl));
	read_clkctrl |= 0x1 << 23;
    read_clkctrl &= ~0x1;
	OUTREG32(b_addr + HDMI_PLL_CLKCTRL_OFF, read_clkctrl);

	/* poll for the bypass acknowledgement */
	repeatCnt = 0u;
	while (repeatCnt < VPS_PRCM_MAX_REP_CNT)
	{
		if (((INREG32(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000101) == 0x00000101)
		{
			break;
		}
		/* Wait for the 100 cycles */
		udelay(100);
		repeatCnt++;
	}

	if (((INREG32(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000101) == 0x00000101)
	{
		;
	}
	else
	{
		ERRORMSG(TRUE, (_T("HDMIL: Not able to Keep PLL in bypass state!!!\r\n")));
	}
	m2nval = (__m2 << 16) | __n;
	mn2val =  __m;
	/*ref_clk     = OSC_FREQ/(__n+1);
	  clkout_dco  = ref_clk*__m;
	  clk_out     = clkout_dco/__m2;
	 */

	OUTREG32((b_addr+HDMI_PLL_M2NDIV_OFF), m2nval);
	read_m2nval = INREG32((b_addr+HDMI_PLL_M2NDIV_OFF));

	OUTREG32((b_addr+HDMI_PLL_MN2DIV_OFF), mn2val);
	read_mn2val = INREG32((b_addr+HDMI_PLL_MN2DIV_OFF));


	OUTREG32((b_addr+HDMI_PLL_TENABLEDIV_OFF), 0x1);

	OUTREG32((b_addr+HDMI_PLL_TENABLEDIV_OFF), 0x0);

	OUTREG32((b_addr+HDMI_PLL_TENABLE_OFF), 0x1);

	OUTREG32((b_addr+HDMI_PLL_TENABLE_OFF), 0x0);

	read_clkctrl = INREG32(b_addr+HDMI_PLL_CLKCTRL_OFF);

	/*configure the TINITZ(bit0) and CLKDCO bits if required */
	OUTREG32(b_addr+HDMI_PLL_CLKCTRL_OFF, (read_clkctrl & 0xff7fe3ff) | clkctrl_val); 

	read_clkctrl = INREG32(b_addr+HDMI_PLL_CLKCTRL_OFF);


	/* poll for the freq,phase lock to occur */
	repeatCnt = 0u;

	while (repeatCnt < VPS_PRCM_MAX_REP_CNT)
	{
		if (((INREG32(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000600) == 0x00000600)
		{
			break;
		}
		/* Wait for the 100 cycles */
		udelay(100);
		repeatCnt++;
	}

	if (((INREG32(b_addr+HDMI_PLL_STATUS_OFF)) & 0x00000600) == 0x00000600)
	{
        /*PRINTMSG(ZONE_INFO, (_T("HDMIL: PLL Locked\r\n")));*/
	}
	else
	{
        PRINTMSG(ZONE_INFO, (_T("HDMIL: PLL Not Getting Locked!!!\r\n")));
	}

	/*wait fot the clocks to get stabized */
	udelay(100);
}

// Centaurus end
#endif

/* Ideally vencs should be configured from the HDVPSS drivers.	But in case
 * we want to test the HDMI this fuction can be used to generate the test
 * pattern on venc and HDMI can be tested in absence of HDVPSS drivers
 */
/*******************************************************************************
 *			Venc Configurations 				       *
 ******************************************************************************/
#ifdef HDMI_TEST
static void configure_venc_1080p30(UINT32 *venc_base, int useEmbeddedSync)
{
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_venc_1080p30 \r\n")));
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A033;
	}
	venc_base++;
	*venc_base = 0x003F0275;
	venc_base++;
	*venc_base = 0x1EA500BB;
	venc_base++;
	*venc_base = 0x1F9901C2;
	venc_base++;
	*venc_base = 0x1FD71E67;
	venc_base++;
	*venc_base = 0x004001C2;
	venc_base++;
	*venc_base = 0x00200200;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x84465898;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F000028;
	venc_base++;
	*venc_base = 0x587800BF;
	venc_base++;
	*venc_base = 0x00000460;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x58780118;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x58780110;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;

}

void configure_venc_1080p60(UINT32 *venc_base, int useEmbeddedSync)
{
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_venc_1080p60 \r\n")));
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A033;
	}
	venc_base++;
	*venc_base = 0x003F0275;
	venc_base++;
	*venc_base = 0x1EA500BB;
	venc_base++;
	*venc_base = 0x1F9901C2;
	venc_base++;
	*venc_base = 0x1FD71E67;
	venc_base++;
	*venc_base = 0x004001C2;
	venc_base++;
	*venc_base = 0x00200200;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x84465898;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F000028;
	venc_base++;
	*venc_base = 0x587800BF;
	venc_base++;
	*venc_base = 0x00000460;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x58780118;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x58780110;
	venc_base++;
	*venc_base = 0x0002A86D;
	venc_base++;
	*venc_base = 0x00438000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;

}


void configure_venc_1080i60(UINT32 *venc_base, int useEmbeddedSync)
{
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_venc_1080i60 \r\n")));
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A03A;
	}

	venc_base++;
	*venc_base = 0x003F0275;
	venc_base++;
	*venc_base = 0x1EA500BB;
	venc_base++;
	*venc_base = 0x1F9901C2;
	venc_base++;
	*venc_base = 0x1FD71E67;
	venc_base++;
	*venc_base = 0x004001C2;
	venc_base++;
	*venc_base = 0x00200200;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x84465898;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F245013;
	venc_base++;
	*venc_base = 0x587800C0;
	venc_base++;
	*venc_base = 0x00000230;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x587800C1;
	venc_base++;
	*venc_base = 0x0001586D;
	venc_base++;
	*venc_base = 0x0021C247;
	venc_base++;
	*venc_base = 0x0500021C;
	venc_base++;
	*venc_base = 0x05001232;
	venc_base++;
	*venc_base = 0x00234234;
	venc_base++;
	*venc_base = 0x587800C0;
	venc_base++;
	*venc_base = 0x0001586D;
	venc_base++;
	*venc_base = 0x0021C247;
	venc_base++;
	*venc_base = 0x0500021C;
	venc_base++;
	*venc_base = 0x05001232;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;
}

void configure_venc_720p60(UINT32* venc_base, int useEmbeddedSync)
{
    PRINTMSG(ZONE_INIT, (_T("HDMIL: configure_venc_720p60 \r\n")));
	if (useEmbeddedSync != 0x0)
	{
		*venc_base = 0x4002A033;
	}
	else
	{
		*venc_base = 0x4003A033;
	}

	venc_base++;
	*venc_base = 0x1FD01E24;
	venc_base++;
	*venc_base = 0x02DC020C;
	venc_base++;
	*venc_base = 0x00DA004A;
	venc_base++;
	*venc_base = 0x020C1E6C;
	venc_base++;
	*venc_base = 0x02001F88;
	venc_base++;
	*venc_base = 0x00200000;
	venc_base++;
	*venc_base = 0x1B6C0C77;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x1C0C0C30;
	venc_base++;
	*venc_base = 0x842EE672;	/* 0x28 */
	venc_base++;
	*venc_base = 0x3F000018;
	venc_base++;
	*venc_base = 0x50500103;
	venc_base++;
	*venc_base = 0x000002E8;
	venc_base++;
	*venc_base = 0x000C39E7;
	venc_base++;
	*venc_base = 0x50500172;
	venc_base++;
	*venc_base = 0x0001A64B;
	venc_base++;
	*venc_base = 0x002D0000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x5050016A;
	venc_base++;
	*venc_base = 0x0001A64B;
	venc_base++;
	*venc_base = 0x002D0000;
	venc_base++;
	*venc_base = 0x05000000;
	venc_base++;
	*venc_base = 0x00003000;
	venc_base++;
	*venc_base = 0x00000000;
	venc_base++;
	*venc_base = 0x00000000;
}
#endif
/* ========================================================================== */
/*			  Global Functions				      */
/* ========================================================================== */

int ti81xx_hdmi_lib_init(struct ti81xx_hdmi_init_params *init_param,
		enum ti81xxhdmi_mode hdmi_mode)
{
	int rtn_value = 0x0;
    int ret;
	if (init_param == NULL) {
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
    PRINTMSG(ZONE_INIT, (_T("HDMIL: hdmi Mode passed = %d\r\n"), hdmi_mode));
	hdmi_config.is_recvr_sensed = FALSE;
	hdmi_config.is_streaming = FALSE;
	hdmi_config.is_scl_clocked = FALSE;
	hdmi_config.vSync_counter = 0x0;
	hdmi_config.is_interlaced = FALSE;

	hdmi_config.core_base_addr = init_param->core_base_addr;
	hdmi_config.wp_base_addr = init_param->wp_base_addr;
	hdmi_config.phy_base_addr = init_param->phy_base_addr;
	hdmi_config.prcm_base_addr = init_param->prcm_base_addr;
	hdmi_config.venc_base_addr = init_param->venc_base_addr;
	hdmi_config.hdmi_pll_base_addr = init_param->hdmi_pll_base_addr;

	ret = enable_hdmi_clocks(hdmi_config.prcm_base_addr);
    if (ret)
    {
		rtn_value = -EFAULT ;
		goto exit_this_func;
    }

	if (-1 != hdmi_mode)
	{
		ti81xx_hdmi_set_mode(hdmi_mode, &hdmi_config);
		ti81xx_hdmi_lib_config(&hdmi_config.config);
		ti81xx_hdmi_lib_start(&hdmi_config, NULL);
	}
	else
	{
		memcpy(((void *) &(hdmi_config.config)),
				((void *) &default_config),
				sizeof(struct hdmi_cfg_params));
	}
exit_this_func:
	return (rtn_value);
}


int ti81xx_hdmi_copy_mode_config(enum ti81xxhdmi_mode hdmi_mode,
		struct instance_cfg *cfg)
{
	int ret_val = 0;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_copy_mode_config - hdmi_mode = %d \r\n"), 
              hdmi_mode));
	switch (hdmi_mode)
	{
		case hdmi_1080P_60_mode:
			memcpy(&cfg->config, &config_1080p60,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_1080p60(
					(UINT32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		case hdmi_720P_60_mode:
			memcpy(&cfg->config, &config_720p60,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_720p60(
					(UINT32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		case hdmi_1080I_60_mode:
			memcpy(&cfg->config, &config_1080i60,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_1080i60(
					(UINT32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		case hdmi_1080P_30_mode:
			memcpy(&cfg->config, &config_1080p30,
					sizeof(struct hdmi_cfg_params));
#ifdef HDMI_TEST
			configure_venc_1080p30(
					(UINT32 *)hdmi_config.venc_base_addr, 0);
#endif
			break;
		default:
			ret_val = -1;;
	}
	if (!ret_val)
		cfg->hdmi_mode = hdmi_mode;
	return ret_val;
}


int ti81xx_hdmi_lib_deinit(void *args)
{
	ti81xx_hdmi_lib_stop(&hdmi_config, NULL);
	hdmi_config.is_recvr_sensed = FALSE;
	hdmi_config.is_streaming = FALSE;
	hdmi_config.is_scl_clocked = FALSE;
	hdmi_config.vSync_counter = 0x0;
	hdmi_config.is_interlaced = FALSE;
	hdmi_config.core_base_addr = 0;
	hdmi_config.wp_base_addr = 0;
	hdmi_config.phy_base_addr = 0;
	hdmi_config.prcm_base_addr = 0;
	hdmi_config.venc_base_addr = 0;
	return 0;
}

/* Open 	- Power up the clock for the DDC and keeps it ON.
 *			- Register the int, update HPD if required
 */
void *ti81xx_hdmi_lib_open(UINT32 instance, int *status, void *args)
{
	struct instance_cfg *inst_context = NULL;
	*status = 0;
	inst_context = &(hdmi_config);
	return inst_context;
}

int ti81xx_hdmi_lib_config(struct hdmi_cfg_params *config)
{
	struct instance_cfg *inst_context = NULL;
	int rtn_value = 0x0;
	volatile UINT32 reset_time_out;
	volatile UINT32 temp;

	inst_context = &(hdmi_config);
	if (config != NULL) {
		if (check_copy_config(&(hdmi_config), config) != 0x0) {
			rtn_value = -EFAULT ;
			goto exit_this_func;
		}
	}

	reset_time_out = HDMI_WP_RESET_TIMEOUT;

	temp = INREG32(inst_context->wp_base_addr + HDMI_WP_SYSCONFIG_OFFSET);
	temp |= HDMI_WP_SYSCONFIG_SOFTRESET_MASK;
	OUTREG32((inst_context->wp_base_addr + HDMI_WP_SYSCONFIG_OFFSET), temp);

	while (
      ((INREG32 (inst_context->wp_base_addr + HDMI_WP_SYSCONFIG_OFFSET)) & 
                 HDMI_WP_SYSCONFIG_SOFTRESET_MASK) == HDMI_WP_SYSCONFIG_SOFTRESET_MASK) 
    {
		reset_time_out--;
		if (reset_time_out == 0x0) 
        {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not reset wrapper\r\n")));
			rtn_value = -EFAULT ;
			goto exit_this_func;
		}
	}

	rtn_value = configure_phy(inst_context);
	if (rtn_value != 0x0) 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not configure PHY\r\n")));
		goto exit_this_func;
	}

	rtn_value = configure_wrapper(inst_context);
	if (rtn_value != 0x0) 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not configure wrapper\r\n")));
		rtn_value = -EINVAL ;
	}

	temp = INREG32(inst_context->wp_base_addr + HDMI_WP_AUDIO_CTRL_OFFSET);
	temp &= (~(HDMI_WP_AUDIO_CTRL_DISABLE_MASK));
	OUTREG32((inst_context->wp_base_addr + HDMI_WP_AUDIO_CTRL_OFFSET), temp);
	OUTREG32((inst_context->wp_base_addr + HDMI_WP_AUDIO_CFG_OFFSET), 0x0);

	temp = INREG32(inst_context->core_base_addr + HDMI_CORE_SYS_STAT_OFFSET);
	temp = (temp & HDMI_SYS_STAT_HPD_MASK) >> HDMI_SYS_STAT_HPD_SHIFT;

	if (temp)
    {
			inst_context->is_recvr_sensed = TRUE;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Detected a sink\r\n")));
	} 
    else 
    {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Sink not detected\r\n")));
	}

	temp = INREG32(inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET);
	temp |= HDMI_SRST_SWRST_MASK;
	OUTREG32((inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET), temp);

	temp = INREG32(inst_context->core_base_addr + HDMI_CORE_SYS_CTRL1_OFFSET);
	temp |= HDMI_SYS_CTRL1_PD_MASK;
	OUTREG32((inst_context->core_base_addr + HDMI_CORE_SYS_CTRL1_OFFSET), temp);

	rtn_value = configure_core(inst_context);
	if (rtn_value != 0x0) 
    {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not cfg core\r\n")));
		goto exit_this_func;
	}
	OUTREG32((inst_context->core_base_addr + HDMI_CORE_AUD_MODE_OFFSET), 0x0);

exit_this_func:
	return (rtn_value);
}


int ti81xx_hdmi_set_mode(enum ti81xxhdmi_mode hdmi_mode,
		struct instance_cfg *cfg)
{
	int rtn_value = 0;
#ifndef CONFIG_ARCH_TI816X
// Centaurus
	struct hdmi_pll_ctrl *pll_ctrl;
#endif
	if (!cfg)
	{
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	if (hdmi_mode < hdmi_ntsc_mode || hdmi_mode >= hdmi_max_mode)
	{
		rtn_value = -EINVAL;
		goto exit_this_func;
	}
	ti81xx_hdmi_copy_mode_config(hdmi_mode, cfg);
#ifndef CONFIG_ARCH_TI816X
// Centaurus begin
	/* Set the PLL according to the mode selected */
	switch (hdmi_mode)
	{
		case hdmi_1080P_30_mode:
		case hdmi_1080I_60_mode:
		case hdmi_720P_60_mode:
			pll_ctrl = &gpll_ctrl[1];
			break;
		case hdmi_1080P_60_mode:
			pll_ctrl = &gpll_ctrl[0];
			break;
		default:
            PRINTMSG(ZONE_INFO, (_T("HDMIL: Mode passed is incorrect\r\n")));
			pll_ctrl = &gpll_ctrl[1];
	}

	configure_hdmi_pll(cfg->hdmi_pll_base_addr, 
                       pll_ctrl->__n, 
                       pll_ctrl->__m,
                       pll_ctrl->__m2, 
                       pll_ctrl->clk_ctrl_value);
// Centaurus end
#endif

	rtn_value = ti81xx_hdmi_lib_config(&cfg->config);
exit_this_func:
	return rtn_value;

}

int ti81xx_hdmi_lib_close(void *handle, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>ti81xx_hdmi_lib_close\r\n")));
	HDMI_ARGS_CHECK((args == NULL));

	if (handle == NULL) {
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	if (inst_context->is_streaming == FALSE) {
		rtn_value = -EBUSY;
		goto exit_this_func;
	}
	rtn_value = -EFAULT ;
exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_lib_close<<<<\r\n")));
	return (rtn_value);
}

/* TODO Not supported for now */
#if 0
static int ti81xx_hdmi_lib_get_cfg(void *handle,
		struct hdmi_cfg_params *config, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>ti81xx_hdmi_lib_get_cfg\r\n")));
	HDMI_ARGS_CHECK((args == NULL));
	if ((handle == NULL) || (config == NULL)) {
		rtn_value = -EFAULT ;
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Invalid handle/config pointer\r\n")));
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	/* Copy the configurations */
	memcpy((void *) config,
			((const void *) &(inst_context->config)),
			sizeof(struct hdmi_cfg_params));
	/* Turn OFF the config update flags */
	config->use_display_mode = 0x0;
	config->use_wp_config = 0x0;
	config->use_core_config = 0x0;
	config->use_core_path_config = 0x0;
	config->use_info_frame_config = 0x0;

exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_lib_get_cfg<<<<\r\n")));
	return (rtn_value);
}
static int ti81xx_hdmi_lib_set_cfg(void *handle,
		struct hdmi_cfg_params *config, void *args)
{
	struct instance_cfg *inst_context = NULL;
	int rtn_value = 0x0;
	volatile UINT32 temp;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>ti81xx_hdmi_lib_set_cfg\r\n")));
	HDMI_ARGS_CHECK((args == NULL));

	inst_context = (struct instance_cfg *) handle;
	HDMI_ARGS_CHECK(
			(inst_context->coreRegOvrlay != NULL));
	if (inst_context->is_streaming == TRUE) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Streaming - cannot re-configure\r\n")));
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}
	if (config == NULL) {
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}

	rtn_value = check_copy_config(inst_context, config);
	if (rtn_value != 0x0) {
		goto exit_this_func;
	}

	rtn_value = configure_phy(inst_context);
	if (rtn_value != 0x0) {
		goto exit_this_func;
	}

	if (config->use_wp_config != 0x0) {
		rtn_value = configure_wrapper(inst_context);
		if (rtn_value != 0x0) {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not configure wrapper\r\n")));
			rtn_value = -EINVAL ;
			goto exit_this_func;
		}
	}

	inst_context->coreRegOvrlay->SRST |=
		CSL_HDMI_SRST_SWRST_MASK;
	inst_context->coreRegOvrlay->SYS_CTRL1 &=
		(~(CSL_HDMI_SYS_CTRL1_PD_MASK));

	if ((config->use_core_config != 0x0) ||
			(config->use_core_path_config != 0x0)) {
		rtn_value = configure_core(inst_context);
		if (rtn_value != 0x0) {
			goto exit_this_func;
		}
	} else {
		rtn_value =
			determine_pixel_repeatation(inst_context);
		/* No pixel repetation - by default */
		inst_context->coreRegOvrlay->VID_CTRL &=
			(~(CSL_HDMI_VID_CTRL_ICLK_MASK));
		if (rtn_value == HDMI_PIXEL_REPLECATED_ONCE) {
			/* Repeat once */
			inst_context->coreRegOvrlay->VID_CTRL |=
				(((0x01u) <<
				  CSL_HDMI_VID_CTRL_ICLK_SHIFT) &
				 CSL_HDMI_VID_CTRL_ICLK_MASK);
		} else if (rtn_value ==
				HDMI_PIXEL_REPLECATED_FOUR_TIMES) {
			inst_context->coreRegOvrlay->VID_CTRL |=
				CSL_HDMI_VID_CTRL_ICLK_MASK;
		} else if (rtn_value == 0x0) {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: No Pixel repeatation required\r\n")));
		} else {
			/* Error let the caller know */
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not determine pixel rate that would be required\r\n")));
			goto exit_this_func;
		}
		/* Power up core and bring it out of reset. */
		inst_context->coreRegOvrlay->SYS_CTRL1 |=
			CSL_HDMI_SYS_CTRL1_PD_MASK;
		inst_context->coreRegOvrlay->SRST &=
			(~(CSL_HDMI_SRST_SWRST_MASK));
	}
	/*
	 * Step 4
	 * Re-configure the wrapper with the scan type. It might have changed.
	 */
	temp =
		INREG32(inst_context->wp_base_addr +
				HDMI_WP_VIDEO_CFG_OFFSET);
	if (inst_context->is_interlaced == TRUE) {
		temp |=
			HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK;
	} else {
		temp &=
			(~
			 (HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK));
	}
	OUTREG32((inst_context->wp_base_addr +
			 HDMI_WP_VIDEO_CFG_OFFSET),
             temp);

	/* Step 4 - Configure AVI Info frame and enable them to be transmitted
	   every frame */
	if (config->use_info_frame_config != 0x0) {
		rtn_value = configure_avi_info_frame(inst_context);
		if (rtn_value != 0x0) {
			goto exit_this_func;
		}
		/*
		 * Policy
		 * 1. Enabling continious transmission of AVI Information packets
		 */
		inst_context->coreRegOvrlay->PB_CTRL1 |=
			(CSL_HDMI_PB_CTRL1_AVI_EN_MASK |
			 CSL_HDMI_PB_CTRL1_AVI_RPT_MASK);
	}
exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_lib_set_cfg<<<<\r\n")));
	return (rtn_value);
}
#endif

int ti81xx_hdmi_lib_start(void *handle, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	volatile UINT32 temp;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>ti81xx_hdmi_lib_start\r\n")));

	if (handle == NULL) {
		rtn_value = -EFAULT ;
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Invalid handle/config pointer\r\n")));
		goto exit_this_func;
	}

	inst_context = (struct instance_cfg *) handle;
	if (inst_context->is_streaming == FALSE)
    {
		temp  = INREG32(inst_context->core_base_addr + HDMI_CORE_SYS_STAT_OFFSET);
		temp &= HDMI_SYS_STAT_HPD_MASK;
		if (!temp)
		{
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Sink not detected\r\n")));
		}

        PRINTMSG(ZONE_INIT, (_T("HDMIL: Trying to start the port\r\n")));

		temp = INREG32(inst_context->core_base_addr + HDMI_CORE_SYS_CTRL1_OFFSET);
		temp |= HDMI_SYS_CTRL1_PD_MASK;
		OUTREG32((inst_context->core_base_addr + HDMI_CORE_SYS_CTRL1_OFFSET), temp);

		temp = INREG32(inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET);
		temp &= (~(HDMI_SRST_SWRST_MASK));
		OUTREG32((inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET), temp);

		/*
		 * Configure core would have updated the global member to
		 * specify the scan type update the wrapper with same info
		 */
		temp = INREG32(inst_context->wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET);

		if (inst_context->is_interlaced == TRUE) 
        {
			temp |= HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK;
		}
        else 
        {
			temp &= (~(HDMI_WP_VIDEO_CFG_PROGRESSIVE_INTERLACE_MASK));
		}
		OUTREG32((inst_context->wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET), temp);

		rtn_value = configure_avi_info_frame(inst_context);
		if (rtn_value != 0x0) 
        {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not configure AVI Info frames\r\n")));
			goto exit_this_func;
		}

		rtn_value = configure_ctrl_packets(inst_context);
		if (rtn_value != 0x0) 
        {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not cfg control packets\r\n")));
			goto exit_this_func;
		}

		temp = INREG32(inst_context->core_base_addr + HDMI_CORE_PB_CTRL1_OFFSET);
		temp = (HDMI_PB_CTRL1_AVI_EN_MASK | HDMI_PB_CTRL1_AVI_RPT_MASK);
		OUTREG32((inst_context->core_base_addr + HDMI_CORE_PB_CTRL1_OFFSET), temp);

		temp = INREG32(inst_context->core_base_addr + HDMI_CORE_VID_MODE_OFFSET);
		temp = ((temp & HDMI_VID_MODE_DITHER_MODE_MASK) >> HDMI_VID_MODE_DITHER_MODE_SHIFT);
		/* General control packets are required only in deep color mode,
		 *  as packing phase would require to be indicated,
		 *  else bypass this
		 */
		if (temp != HDMI_VID_MODE_DITHER_TO_24_BITS_MODE) 
        {
			OUTREG32((inst_context->core_base_addr + HDMI_CORE_PB_CTRL2_OFFSET),
                     (HDMI_PB_CTRL2_GEN_EN_MASK | HDMI_PB_CTRL2_GEN_RPT_MASK));
		}
        else 
        {
			OUTREG32((inst_context->core_base_addr + HDMI_CORE_PB_CTRL2_OFFSET),
                     (HDMI_PB_CTRL2_CP_EN_MASK |
						HDMI_PB_CTRL2_CP_RPT_MASK |
						HDMI_PB_CTRL2_GEN_EN_MASK |
						HDMI_PB_CTRL2_GEN_RPT_MASK));
		}

#ifndef CONFIG_ARCH_TI816X
        // For Centaurus some secret bit needs to be set
		temp = INREG32((inst_context->wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET));
		temp |= (0x1 << 2);
		OUTREG32((inst_context->wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET), temp);
#endif

		temp = INREG32((inst_context->wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET));
		temp |= HDMI_WP_VIDEO_CFG_ENABLE_MASK;
		OUTREG32((inst_context->wp_base_addr + HDMI_WP_VIDEO_CFG_OFFSET), temp);
		inst_context->is_streaming = TRUE;
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Started the port\r\n")));
	}
    else
    {
		if (inst_context->is_recvr_sensed == TRUE){
			rtn_value = -EFAULT ;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: No Sinks dected-not starting\r\n")));
		}
	}
exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_lib_start<<<<\r\n")));
	return (rtn_value);
}

int ti81xx_hdmi_lib_stop(void *handle, void *args)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	volatile UINT32 temp;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>ti81xx_hdmi_lib_stop\r\n")));

	if (handle == NULL) {
		rtn_value = -EFAULT ;
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Invalid handle/config pointer\r\n")));
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	if (inst_context->is_streaming == TRUE) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Trying to stop the port\r\n")));
		temp = INREG32(inst_context->core_base_addr +
				HDMI_CORE_SRST_OFFSET);
		temp |= HDMI_SRST_SWRST_MASK;
		OUTREG32((inst_context->core_base_addr + HDMI_CORE_SRST_OFFSET),
                 temp);
		temp = INREG32((inst_context->wp_base_addr +
						HDMI_WP_VIDEO_CFG_OFFSET));
		temp &= (~(HDMI_WP_VIDEO_CFG_ENABLE_MASK));
		OUTREG32((inst_context->wp_base_addr +
				 HDMI_WP_VIDEO_CFG_OFFSET),
                 temp);
		inst_context->is_streaming = FALSE;
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Stopped the port\r\n")));
	}
exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_lib_stop<<<<\r\n")));
	return (rtn_value);
}

int ti81xx_hdmi_lib_control(void *handle,
		UINT32 cmd, void *cmdArgs, void *additionalArgs)
{
	int rtn_value = 0x0;
	struct instance_cfg *inst_context = NULL;
	volatile unsigned int temp;
	struct ti81xxhdmi_status  *status;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>ti81xx_hdmi_lib_control\r\n")));
	/* Validate the handle and execute the command. */
	if (handle == NULL) {
		rtn_value = -EFAULT ;
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Invalid handle/cmdArgs pointer\r\n")));
		goto exit_this_func;
	}
	inst_context = (struct instance_cfg *) handle;
	switch (cmd) {
		case IOCTL_TI81XXHDMI_START:
			rtn_value = ti81xx_hdmi_lib_start(handle, NULL);
			break;

		case IOCTL_TI81XXHDMI_STOP:
			rtn_value = ti81xx_hdmi_lib_stop(handle, NULL);
			break;
		case IOCTL_TI81XXHDMI_GET_STATUS:
			rtn_value = -EFAULT ;
			if (cmdArgs) {
				status = (struct ti81xxhdmi_status *)cmdArgs;
				status->is_hdmi_streaming =
						inst_context->is_streaming;
				temp  = INREG32(inst_context->core_base_addr
					+ HDMI_CORE_SYS_STAT_OFFSET);
				temp &= HDMI_SYS_STAT_HPD_MASK;
				status->is_hpd_detected =
					temp >> HDMI_SYS_STAT_HPD_SHIFT;
				rtn_value = 0x0;
			}
			break;
		case IOCTL_TI81XXHDMI_READ_EDID:
			rtn_value = ti81xx_hdmi_lib_read_edid(handle,
					(struct ti81xxdhmi_edid_params *)
					cmdArgs,
					NULL);
			break;
			/* TODO Not supported for now */
#if 0
		case TI81XXHDMI_GET_CONFIG:
			rtn_value = ti81xx_hdmi_lib_get_cfg(handle,
					(struct hdmi_cfg_params *)
					cmdArgs,
					NULL);
			break;
		case TI81XXHDMI_SET_CONFIG:
			{
				if (NULL != cmdArgs)
				{
					rtn_value =
						(ti81xx_hdmi_lib_config((struct hdmi_cfg_params *)cmdArgs));
				}
				else
				{
					rtn_value = -EFAULT ;
				}

			}
#endif
		case IOCTL_TI81XXHDMI_SET_MODE:
			rtn_value =  (ti81xx_hdmi_set_mode((enum ti81xxhdmi_mode)cmdArgs,
						inst_context));
			break;
		case IOCTL_TI81XXHDMI_GET_MODE:
			return (inst_context->hdmi_mode);
		case IOCTL_TI81XXHDMI_TEST_HDMI:
            PRINTMSG(ZONE_INIT, (_T("HDMIL: In HDMI TEST venc_base = %d\r\n"), 
                     inst_context->venc_base_addr));
#ifdef HDMI_TEST
			switch ((enum ti81xxhdmi_mode)cmdArgs)
			{
				case hdmi_1080P_30_mode:
					configure_venc_1080p30((UINT32 *)inst_context->venc_base_addr, 0);
					break;
				case hdmi_1080P_60_mode:
					configure_venc_1080p60((UINT32 *)inst_context->venc_base_addr, 0);
					break;
				case hdmi_1080I_60_mode:
					configure_venc_1080i60((UINT32 *)inst_context->venc_base_addr, 0);
					break;
				case hdmi_720P_60_mode:
					configure_venc_720p60((UINT32 *)inst_context->venc_base_addr, 0);
					break;
				default :
					rtn_value = -EINVAL;
			}
#endif
			rtn_value =  ti81xx_hdmi_set_mode((enum ti81xxhdmi_mode)cmdArgs,
					inst_context);
			break;
		case IOCTL_TI81XXHDMI_GET_PHY_STAT:
			if (cmdArgs)
			{
				get_phy_status(inst_context, cmdArgs);
			}
			else
			{
				rtn_value = -EFAULT;
			}
			break;
		default:
			rtn_value = -EINVAL;
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Un-recoganized command\r\n")));
			break;
	}
exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_lib_control<<<<\r\n")));
	return (rtn_value);
}
static int ti81xx_hdmi_lib_read_edid(void *handle,
		struct ti81xxdhmi_edid_params *r_params,
		void *args)
{
	int rtn_value = 0x0;
	UINT32 r_byte_cnt = 0x0;
	volatile UINT32 io_timeout = 0x0;
	volatile UINT32 timeout;
	volatile UINT32 cmd_status;
	volatile UINT32 temp;
	UINT8 *buf_ptr = NULL;
	struct instance_cfg *inst_context = NULL;

    PRINTMSG(ZONE_INIT, (_T("HDMIL: >>>>ti81xx_hdmi_lib_read_edid\r\n")));

	if ((handle == NULL) || (r_params == NULL)) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Invalid params\r\n")));
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	inst_context = handle;
	buf_ptr = (UINT8 *) r_params->buffer_ptr;
	if (buf_ptr == NULL) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Invalid buffer pointer\r\n")));
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	/* 10 bits to hold the count - which would be 3FF */
	if ((r_params->no_of_bytes == 0x0)
			|| (r_params->no_of_bytes > 0x3FF)) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Invalid byte count\r\n")));
		rtn_value = -EFAULT ;
		goto exit_this_func;
	}
	r_params->no_of_bytes_read = 0x0;
	if (inst_context->is_recvr_sensed != TRUE) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: HPD not detected - HAL not opened\r\n")));
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}
	if (r_params->timeout == 0x0){
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not read in given time\r\n")));
		rtn_value = -ETIME ;
		goto exit_this_func;
	}

	temp = INREG32((inst_context->core_base_addr +
				HDMI_CORE_RI_STAT_OFFSET));
	if ((temp & HDMI_RI_STAT_RI_STARTED_MASK) ==
			HDMI_RI_STAT_RI_STARTED_MASK) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: RI Check enbled - DDC bus busy\r\n")));
		rtn_value = -EINVAL ;
		goto exit_this_func;
	}

	if (inst_context->is_scl_clocked == FALSE) {
		OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET),
                 HDMI_DDC_CMD_CLOCK_SCL);

		timeout = HDMI_DDC_CMD_TIMEOUT;
		temp = INREG32((inst_context->core_base_addr +
					HDMI_CORE_DDC_STATUS_OFFSET));
		while ((temp & HDMI_DDC_STATUS_IN_PROG_MASK)
				== HDMI_DDC_STATUS_IN_PROG_MASK) {
			timeout--;
			temp = INREG32((inst_context->core_base_addr +
						HDMI_CORE_DDC_STATUS_OFFSET));
		}
		if (timeout == 0x0) {
            PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not clock SCL before read\r\n")));
			rtn_value = -ETIME ;
			goto exit_this_func;
		}
		inst_context->is_scl_clocked = TRUE;
	}

	OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_ADDR_OFFSET),
            (HDMI_DDC_ADDR_DDC_ADDR_MASK & r_params->slave_address));

	OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_SEGM_OFFSET),
             (HDMI_DDC_SEGM_DDC_SEGM_MASK & r_params->segment_ptr));

	OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_OFFSET_OFFSET),
             (HDMI_DDC_OFFSET_DDC_OFFSET_MASK & r_params->offset));

	OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_COUNT1_OFFSET),
             (HDMI_DDC_COUNT1_DDC_COUNT_MASK & r_params->no_of_bytes));

	OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_COUNT2_OFFSET),
             (HDMI_DDC_COUNT2_DDC_COUNT_MASK &
				(r_params->no_of_bytes >> 0x08)));

	OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET),
             HDMI_DDC_CMD_CLEAR_FIFO);

	timeout = HDMI_DDC_CMD_TIMEOUT;
	temp = INREG32((inst_context->core_base_addr +
				HDMI_CORE_DDC_STATUS_OFFSET));
	while ((temp & HDMI_DDC_STATUS_IN_PROG_MASK)
			== HDMI_DDC_STATUS_IN_PROG_MASK) {
		timeout--;
		temp = INREG32((inst_context->core_base_addr +
					HDMI_CORE_DDC_STATUS_OFFSET));
	}
	if (timeout == 0x0) {
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Could not clear FIFOs\r\n")));
		rtn_value = -ETIME ;
		goto abort_exit_this_func;
	}

	io_timeout = r_params->timeout;
	if (r_params->use_eddc_read == 0x0){
		OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET),
                 HDMI_DDC_CMD_SEQ_R_NO_ACK_ON_LAST_BYTE);
	}else{
		OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET),
                 HDMI_DDC_CMD_EDDC_R_NO_ACK_ON_LAST_BYTE);
	}
	temp = INREG32((inst_context->core_base_addr +
				HDMI_CORE_DDC_FIFOCNT_OFFSET));
	while (temp == 0x0) {
		if (io_timeout == 0x0){
			rtn_value = -ETIME ;
			goto abort_exit_this_func;
		}
		temp = INREG32((inst_context->core_base_addr +
					HDMI_CORE_DDC_FIFOCNT_OFFSET));
		io_timeout--;
	}
	/* Check for errors */
	cmd_status = INREG32((inst_context->core_base_addr +
				HDMI_CORE_DDC_STATUS_OFFSET));

	if ((cmd_status & HDMI_DDC_STATUS_BUS_LOW_MASK) ==
			HDMI_DDC_STATUS_BUS_LOW_MASK) {
		/* Bus is being held by the slave / others...
		   Ultra Slow slaves? */
        PRINTMSG(ZONE_INIT, (_T("HDMIL: Bus being held low\r\n")));
		rtn_value = -EINVAL ;
		goto abort_exit_this_func;
	}
	if ((cmd_status & HDMI_DDC_STATUS_NO_ACK_MASK) ==
			HDMI_DDC_STATUS_NO_ACK_MASK) {
		/* UnPlugged TV? */
        PRINTMSG(ZONE_INIT, (_T("HDMIL: No ACK from the device\r\n")));
		rtn_value = -EINVAL ;
		goto abort_exit_this_func;
	}
	while (r_byte_cnt < r_params->no_of_bytes){
		if (inst_context->is_recvr_sensed != TRUE) {
			rtn_value = -ETIME ;
			goto abort_exit_this_func;
		}
		temp = INREG32((inst_context->core_base_addr +
					HDMI_CORE_DDC_FIFOCNT_OFFSET));
		if (temp == 0x0){
			while (temp == 0x0)
			{
				if (io_timeout == 0x0){
					rtn_value = -ETIME ;
					goto abort_exit_this_func;
				}
				io_timeout--;
				temp = INREG32(
						(inst_context->core_base_addr +
						 HDMI_CORE_DDC_FIFOCNT_OFFSET));
			}
		}

		*buf_ptr = (UINT8) ((INREG32((inst_context->core_base_addr
							+ HDMI_CORE_DDC_DATA_OFFSET))) &
				HDMI_DDC_DATA_DDC_DATA_MASK);
		buf_ptr++;
		r_byte_cnt++;
	}
	/*
	 * Aborting the READ command.
	 * In case we have completed as expected - no of bytes to read is read
	 *	- No issues, aborting on completion is OK
	 * If device was unplugged before read could be complete,
	 *	- Abort should leave the bus clean
	 * If any other error
	 *	- Ensure bus is clean
	 */
	r_params->no_of_bytes_read = r_byte_cnt;

abort_exit_this_func:
	OUTREG32((inst_context->core_base_addr + HDMI_CORE_DDC_CMD_OFFSET),
             HDMI_DDC_CMD_ABORT);

	temp = INREG32((inst_context->core_base_addr +
				HDMI_CORE_DDC_STATUS_OFFSET));
	while ((temp & HDMI_DDC_STATUS_IN_PROG_MASK)
			== HDMI_DDC_STATUS_IN_PROG_MASK) {
		timeout--;
		temp = INREG32((inst_context->core_base_addr +
					HDMI_CORE_DDC_STATUS_OFFSET));
	}
exit_this_func:
    PRINTMSG(ZONE_INIT, (_T("HDMIL: ti81xx_hdmi_lib_read_edid<<<<\r\n")));
	return (rtn_value);
}
static void HDMI_ARGS_CHECK(UINT32 condition)
{
	return;
}
