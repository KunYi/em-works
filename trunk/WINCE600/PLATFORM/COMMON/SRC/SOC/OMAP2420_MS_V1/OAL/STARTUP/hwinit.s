;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;
; Portions Copyright (c) Texas Instruments.  All rights reserved.
; Portions Copyright (c) Intrinsyc Software.  All rights reserved.
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;-------------------------------------------------------------------------------
;
;  File:  hwinit.s
;
;  Hardware initialization routine for OMAP2420 boards. The code contains
;  references to board hardware specific initialization constants. It is
;  intended to be included by from other assembler file.
;
;  Constants used in configuration are defined in bsp_XXX_cfg.inc files
;  located in platform\%_TGTPLAT%\src\inc directory.
;
;  We also expect that the actual DPLL and its associated clock tree
;  frequencies including EMIF (memory interface Traffic Controller), CPU,
;  and peripheral are lower than the final and may even be in by-pass.
;
;  In normal situation this code is called from X-Loader code (for BSP
;  with NAND flash) or IPL/EBOOT code (for system with NOR flash).
;  This code can also be included from Windows CE OS startup code when
;  original TI EBOOT is used (it sets clocks back to 12/13MHz).
;

; ---------------------------------------------------------------------
;		T H E    E N D
; ---------------------------------------------------------------------
;11		ldr   r0, =0x08000016
;        ldr   r1, =0x3333 ;LED_REG_VAL1
;        strh  r1, [r0]
;	    b	%b11
; ---------------------------------------------------------------------
              
        ;---------------------------------------------------------------
        ; Set SVC mode & disable IRQ/FIQ
        ;---------------------------------------------------------------

        mrs     r0, cpsr                        ; Get current mode bits.
        bic     r0, r0, #0x1F                   ; Clear mode bits.
        orr     r0, r0, #0xD3                   ; Disable IRQs/FIQs, SVC mode
        msr     cpsr_c, r0                      ; Enter supervisor mode

        ;---------------------------------------------------------------
		; Disable Watchdog timer 
		;---------------------------------------------------------------
		ldr r0, =OMAP2420_WDOG2_REGS_PA                  
        ldr r1, =OMAP2420_WDOG_DISABLE_SEQ1		;Write 0xAAAA to WDOG start/stop register 
	    str r1,[r0,#OMAP2420_WDOG_WSPR_OA]
	    nop
10
    	ldr r2,[r0,#OMAP2420_WDOG_WWPS_OA]
    	cmp r2,#0
        bne %B10
        ldr r1, =OMAP2420_WDOG_DISABLE_SEQ2	        ;Write 0x5555 to WDOG start/stop register
        str r1,[r0,#OMAP2420_WDOG_WSPR_OA]
        nop
20
        ldr r2,[r0,#OMAP2420_WDOG_WWPS_OA]
        cmp r2,#0
        bne %B20

		;Ball configuration
        ldr   r0,=OMAP2420_SYSC1_REGS_PA      		

        ;SDRAM Ball configuration
;offset 0x30;ball D10,sdrc_ba1;ball D11,sdrc_a12;ball B4,sdrc_a13;ball B3,sdrc_a14			
        ldr   r1,=BSP_PADCONF_SDRC_A14		
		str   r1,[r0,#OMAP2420_CONTROL_PADCONF_SDRC_A14_OA]      

;offset 0x8C;ball N2,gpmc_cs3;ball E2,gpmc_cs2;ball N8,gpmc_cs1;ball L4,gpmc_cs0			
		ldr   r1, =BSP_PADCONF_GPMC_NCS0			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_GPMC_NCS0_OA]

;offset 0x9C;ball B14,sdrc_nclk;ball C14,sdrc_clk;ball P1,GPIO_35;ball M1,gpmc_wait2
		ldr   r1, =BSP_PADCONF_GPMC_WAIT2			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_GPMC_WAIT2_OA]

;offset 0xA0;ball B13,sdrc_ncs1;ball D13,sdrc_cke0;ball C12,sdrc_cke1;ball D12,sdrc_ncs0			
        ldr   r1,=BSP_PADCONF_SDRC_NCS0	 	
        str   r1,[r0,#OMAP2420_CONTROL_PADCONF_SDRC_NCS0_OA]    
		
;offset 0xB0;ball Y7,dss_d0	ball;B18,sdrc_dqs3;ball D16,sdrc_dqs2;ball G9,sdrc_dqs1			
		ldr   r1, =BSP_PADCONF_SDRC_DQS1			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_SDRC_DQS1_OA]

;offset 0xB4;ball W8,dss_d4;ball Y8,dss_d3;ball V8,dss_d2;ball P10,dss_d1			
		ldr   r1, =BSP_PADCONF_DSS_D1			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_DSS_D1_OA]

;offset 0xB8;ball W9,dss_d8;ball V9,dss_d7;ball Y9,dss_d6;ball R10,dss_d5			
		ldr   r1, =BSP_PADCONF_DSS_D5			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_DSS_D5_OA]

;offset 0xBC;ball W10,dss_d12;ball Y10,dss_d11;ball V10,dss_d10;ball P11,dss_d9			
		ldr   r1, =BSP_PADCONF_DSS_D9			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_DSS_D9_OA]

;offset 0xC0;ball P12,dss_d16;ball W11,dss_d15;ball V11,dss_d14;ball R11,dss_d13			
		ldr   r1, =BSP_PADCONF_DSS_D13			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_DSS_D13_OA]

;offset 0xC4;ball L20,uart1_tx;ball H21,uart1_rts;ball D21,uart1_cts;ball R12,dss_d17			
		ldr   r1, =BSP_PADCONF_DSS_D17			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_DSS_D17_OA]

;offset 0xC8;ball W6,dss_pclk;ball P21,mcbsp2_clkx;ball M21,mcbsp2_dr;ball T21,uart1_rx			
		ldr   r1, =BSP_PADCONF_UART1_RX			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_UART1_RX_OA]

;offset 0xCC;ball V6,cam_d9;ball W7,dss_acbias;ball Y6,dss_hsync;ball V7,dss_vsync			
		ldr   r1, =BSP_PADCONF_DSS_VSYNC			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_DSS_VSYNC_OA]

;offset 0xD0;ball V4,cam_d5;ball W3,cam_d6;ball Y2,cam_d7;ball Y4,cam_d8
		ldr   r1, =BSP_PADCONF_CAM_D8			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_CAM_D8_OA]

;offset 0xD4;ball V2,cam_d1;ball V3,cam_d2;ball U4,cam_d3;ball W2,cam_d4
		ldr   r1, =BSP_PADCONF_CAM_D4			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_CAM_D4_OA]

;offset 0xD8;ball V5,cam_lclk;ball U2,cam_vs;ball T3,cam_hs;ball T4,cam_d0
		ldr   r1, =BSP_PADCONF_CAM_D0			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_CAM_D0_OA]

;offset 0xDC;ball P13,GPIO.61;ball V12,GPIO.25;ball W12,ssi1_dat_tx;ball U3,cam_xclk
		ldr   r1, =BSP_PADCONF_CAM_XCLK			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_CAM_XCLK_OA]

;offset 0xE0;ball V13,EAC.MD_DOUT;ball W13,EAC.MD_DIN;ball Y12,EAC.MD_SCLK;ball R13,gpio_62
		ldr   r1, =BSP_PADCONF_GPIO_62			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_GPIO_62_OA]

;offset 0xE4;ball AA4,GPIO.15;ball AA6,vlynq_rx1;ball AA10,GPIO.13;ball Y13,EAC.MD_FS
		ldr   r1, =BSP_PADCONF_SSI1_WAKE			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_SSI1_WAKE_OA]

;offset 0xE8;ball V19,uart2_cts;ball AA8,GPIO.58;ball AA12,GPIO.17;ball Y11,GPIO.16
		ldr   r1, =BSP_PADCONF_VLYNQ_TX1			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_VLYNQ_TX1_OA]

;offset 0xEC;ball R8,eac_bt_sclk;ball P15,uart2_rx;ball N14,uart2_tx;ball W20,uart2_rts
		ldr   r1, =BSP_PADCONF_UART2_RTS			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_UART2_RTS_OA]

;offset 0xF0;ball G19,mmc_clko;ball W4,eac_bt_dout;ball Y3,eac_bt_din;ball P9,eac_bt_fs
		ldr   r1, =BSP_PADCONF_EAC_BT_FS			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_EAC_BT_FS_OA]

;offset 0xF4;ball E19,mmc_dat2;ball H14,mmc_dat1;ball F20,mmc_dat0;ball H18,mmc_cmd
		ldr   r1, =BSP_PADCONF_MMC_CMD			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_MMC_CMD_OA]

;offset 0xF8;ball F18,mmc_dat2;ball E20,mmc_dat1;ball F19,mmc_dat0;ball D19,mmc_cmd
		ldr   r1, =BSP_PADCONF_MMC_DAT3			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_MMC_DAT3_OA]

;offset 0xFC;ball U18,spi1_clk;ball H15,mmc_clki;ball G18,mmc_cmd_dir;ball E18,mmc_dat_dir3
		ldr   r1, =BSP_PADCONF_MMC_DAT_DIR3			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_MMC_DAT_DIR3_OA]

;offset 0x100;ball N15,spi1_cs1;ball U19,spi1_cs0;ball T18,spi1_somi;ball V20,spi1_simo
		ldr   r1, =BSP_PADCONF_SPI1_SIMO			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_SPI1_SIMO_OA]

;offset 0x104;ball R19,GPIO.89;ball T19,GPIO.88;ball U21,spi1_cs3;ball R18,spi1_cs2
		ldr   r1, =BSP_PADCONF_SPI1_NCS2			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_SPI1_NCS2_OA]

;offset 0x108;ball P20,GPIO.93;ball M15,GPIO.92;ball M14,GPIO.91;ball R20,GPIO.90
		ldr   r1, =BSP_PADCONF_SPI2_SOMI			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_SPI2_SOMI_OA]

;offset 0x10C;ball L14,GPIO.97;ball M18,GPIO.96;ball P18,GPIO.95;ball P19,mcbsp1_dx
		ldr   r1, =BSP_PADCONF_MCBSP1_DX			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_MCBSP1_DX_OA]

;offset 0x110;ball J15,i2c2_scl;ball L15,i2c1_sda;ball M19,i2c1_scl;ball N19,GPIO.98
		ldr   r1, =BSP_PADCONF_MCBSP1_CLKX			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_MCBSP1_CLKX_OA]

;offset 0x114;ball L19,GPIO.103;ball L18,GPIO.102;ball N18,HDQ;ball H19,i2c2_sda
		ldr   r1, =BSP_PADCONF_I2C2_SDA			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_I2C2_SDA_OA]

;offset 0x118;ball AA18,tv_vref  ball AA16,tv_cvbs  ball K14,uart3_rx_irrx;ball K15,uart3_tx_irrx			
		ldr   r1, =BSP_PADCONF_UART3_TX_IRTX			
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_UART3_TX_IRTX_OA]

; offset 0x124;ball V15,eac_ac_dout;ball W15,eac_ac_din;ball R14,eac_ac_fs;ball Y15,eac_ac_sclk
		ldr   r1, =BSP_PADCONF_EAC_AC_SCLK
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_EAC_AC_SCLK_OA]

; offset 0x128;ball Y16,sys_nreswarm;ball Y14,sys_nrespwron;ball W16,GPIO.118		  ball V14,GPIO.117
		ldr   r1, =BSP_PADCONF_EAC_AC_MCLK
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_EAC_AC_MCLK_OA]

; offset 0x12C;ball Y5,gpio_120;ball W5,gpio_119;ball Y20,sys_nvmode;ball W19,sys_nirq
		ldr   r1, =BSP_PADCONF_SYS_NIRQ
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_SYS_NIRQ_OA]

; offset 0x130;ball Y18,sys_xtalin;ball Y17,sys_32k;ball P8,GPIO.122;ball R9,gpio_121
		ldr   r1, =BSP_PADCONF_GPIO_121
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_GPIO_121_OA]

; offset 0x134;ball W14,sys_clkout;ball AA17,sys_clkreq;ball V17,GPIO.36;ball V16,sys_xtalout
		ldr   r1, =BSP_PADCONF_SYS_XTALOUT
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_SYS_XTALOUT_OA]

; offset 0x138;ball AA21,jtag_emu1;ball P14,GPIO.125;ball V18,GPIO.88;ball E5,GPIO.6
		ldr   r1, =BSP_PADCONF_GPIO_6
		str   r1, [r0,#OMAP2420_CONTROL_PADCONF_GPIO_6_OA]

		;GPIO configuration
        ldr   r0,=OMAP2420_GPIO1_REGS_PA      		
        ldr   r1,=BSP_GPIO_OE_1		
		str   r1,[r0,#OMAP2420_GPIO_OE]      

        ldr   r0,=OMAP2420_GPIO2_REGS_PA      		
        ldr   r1,=BSP_GPIO_OE_2		
		str   r1,[r0,#OMAP2420_GPIO_OE]      

        ldr   r0,=OMAP2420_GPIO3_REGS_PA      		
        ldr   r1,=BSP_GPIO_OE_3		
		str   r1,[r0,#OMAP2420_GPIO_OE]      

        ldr   r0,=OMAP2420_GPIO4_REGS_PA      		
        ldr   r1,=BSP_GPIO_OE_4		
		str   r1,[r0,#OMAP2420_GPIO_OE]      

		;---------------------------------------------------------------
		; SDRAM Initialization
		;---------------------------------------------------------------
		
        ; perform a reset to sdrc.
        ldr    r0, =OMAP2420_SDRC_REGS_PA  
	    ldr    r1, =BSP_SDRC_RESET_VAL1	
	    str    r1, [r0,#OMAP2420_SDRC_SYSCONFIG_OA]
	    nop
	    nop
	    nop
	    nop
	    nop
		
	    ldr    r1, =BSP_SDRC_RESET_VAL2
	    str    r1, [r0,#OMAP2420_SDRC_SYSCONFIG_OA]

	    ; check the status and wait till reset is over.
30		ldr r2, [r0,#OMAP2420_SDRC_SYSSTATUS_OA]
		and r2, r2, #BSP_SDRC_RESET_STATUS_MASK 
		cmp r2, #BSP_SDRC_RESET_STATUS_MASK
		bne %B30
		
		; configure the SDRAM Sharing register for
		; both combo and discrete device.
		ldr	r1, =BSP_SDRC_SHARING_VAL	   		
		str	r1, [r0,#OMAP2420_SDRC_SHARING_OA]
		
		;------------------------------------------------------
		; Configure SDRC CS0/1 here.
		;------------------------------------------------------
		
		; configure SDRC power
		ldr     r1, =BSP_SDRC_PWR_VAL   ;Self refresh mode enabled during reset,
		                                     ;High Power High Bandwidth mode set
											 ;Power down for CKE1 disabled.
		str     r1, [r0,#OMAP2420_SDRC_POWER_OA]
		
		; configure the mcfg_0 register for CS0/1.
		ldr		r1, =BSP_SDRC_MCFG_VAL         ; MUX9 config, 32bit 
		str     r1, [r0,#OMAP2420_SDRC_MCFG_0_OA]         
		str     r1, [r0,#OMAP2420_SDRC_MCFG_1_OA]         
		
		;configure AC Timing Control A register for CS0/1
		ldr     r1, =BSP_SDRC_ACTIM_CTRLA_VAL   
		str     r1, [r0,#OMAP2420_SDRC_ACTIM_CTRLA_0_OA]
		str     r1, [r0,#OMAP2420_SDRC_ACTIM_CTRLA_1_OA]
		
		;configure AC Timing Control B register for CS0/1
		ldr     r1, =BSP_SDRC_ACTIM_CTRLB_VAL
		str     r1, [r0,#OMAP2420_SDRC_ACTIM_CTRLB_0_OA]
		str     r1, [r0,#OMAP2420_SDRC_ACTIM_CTRLB_1_OA]
		
		;configure Referesh control register for CS0/1
		ldr     r1, =BSP_SDRC_RFR_CTRL_VAL  
		str     r1, [r0,#OMAP2420_SDRC_RFR_CTRL_0_OA]
		str     r1, [r0,#OMAP2420_SDRC_RFR_CTRL_1_OA]
				
		; write a specific sequence to manual command 
		; register before configuring MRC register - CS0/1.
		ldr     r1, =BSP_SDRC_MANUAL_NOP_CMD 		  ; NOP comand
		str     r1, [r0,#OMAP2420_SDRC_MANUAL_0_OA]	
		str     r1, [r0,#OMAP2420_SDRC_MANUAL_1_OA]	
		
		ldr     r1, =BSP_SDRC_MANUAL_PRECHARGE_CMD	  ; Precharge
		str     r1, [r0,#OMAP2420_SDRC_MANUAL_0_OA]
		str     r1, [r0,#OMAP2420_SDRC_MANUAL_1_OA]
		
		ldr     r1, =BSP_SDRC_MANUAL_AUTOREFRESH_CMD ; Auto-refresh command.
		str     r1, [r0,#OMAP2420_SDRC_MANUAL_0_OA]
		str     r1, [r0]                                  ; Auto-refresh command again.
		str     r1, [r0,#OMAP2420_SDRC_MANUAL_1_OA]
		str     r1, [r0]                                  ; Auto-refresh command again.
						                    
		; configure mr register for CS0/1
		ldr     r1, =BSP_SDRC_MR_VAL
		str     r1, [r0,#OMAP2420_SDRC_MR_0_OA]
		str     r1, [r0,#OMAP2420_SDRC_MR_1_OA]
		
		ldr     r1, =BSP_SDRC_DLL_VAL1	               
		str     r1, [r0,#OMAP2420_SDRC_DLLA_CTRL_OA]
		str     r1, [r0,#OMAP2420_SDRC_DLLB_CTRL_OA]
		
		ldr     r1, =BSP_SDRC_DLL_VAL2		        
		str     r1, [r0,#OMAP2420_SDRC_DLLA_CTRL_OA]
		str     r1, [r0,#OMAP2420_SDRC_DLLB_CTRL_OA]
		
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		
	   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	   ; CONFIGURE GPMC for CS0
	   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		
       ldr   r0, =OMAP2420_GPMC_REGS_PA

       ldr   r1, =BSP_GPMC_TIMEOUT_CONTROL_VAL
       str   r1, [r0,#OMAP2420_GPMC_TIMEOUT_CONTROL_OA]

       ldr   r1, =BSP_GPMC_SYSCONFIG_VAL
       str	 r1, [r0,#OMAP2420_GPMC_SYSCONFIG_OA]


       ldr   r1, =BSP_GPMC_IRQENABLE_VAL 
       str	 r1, [r0,#OMAP2420_GPMC_IRQENABLE_OA]

       ldr   r1, =BSP_GPMC_CONFIG_VAL 
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG_OA]
       ; Make sure CS0 is not pointing to the same place.

       ;****Warning****
	   ;The config value to CONFIG7 register is valid only for
	   ;NAND flash. If hwinit.s is used for NOR then we should not 
	   ;configure CONFIG7 register.
  
       ldr   r1, =BSP_GPMC_CONFIG1_0_VAL   
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG1_0_OA]

       ldr   r1, =BSP_GPMC_CONFIG2_0_VAL   
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG2_0_OA]

       ldr   r1, =BSP_GPMC_CONFIG3_0_VAL   
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG3_0_OA]

       ldr   r1, =BSP_GPMC_CONFIG4_0_VAL   
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG4_0_OA]

       ldr   r1, =BSP_GPMC_CONFIG5_0_VAL   
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG5_0_OA]

       ldr   r1, =BSP_GPMC_CONFIG6_0_VAL   
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG6_0_OA]

       ldr   r1, =BSP_GPMC_CONFIG7_0_VAL   
       str	 r1, [r0,#OMAP2420_GPMC_CONFIG7_0_OA]

       ;-------------------------------------------------------------------
	   ; GPMC settings for MPDB CS1
	   ;-------------------------------------------------------------------

       ldr   r1, =BSP_GPMC_CONFIG1_1_VAL
       str   r1, [r0,#OMAP2420_GPMC_CONFIG1_1_OA]
			
       ldr   r1, =BSP_GPMC_CONFIG2_1_VAL
       str   r1, [r0,#OMAP2420_GPMC_CONFIG2_1_OA]
		
       ldr   r1, =BSP_GPMC_CONFIG3_1_VAL
       str   r1, [r0,#OMAP2420_GPMC_CONFIG3_1_OA]

       ldr   r1, =BSP_GPMC_CONFIG4_1_VAL  
       str   r1, [r0,#OMAP2420_GPMC_CONFIG4_1_OA]

       ldr   r1, =BSP_GPMC_CONFIG5_1_VAL
       str   r1, [r0,#OMAP2420_GPMC_CONFIG5_1_OA]
		
       ldr   r1, =BSP_GPMC_CONFIG6_1_VAL
       str   r1, [r0,#OMAP2420_GPMC_CONFIG6_1_OA]
				
       ldr   r1, =BSP_GPMC_CONFIG7_1_VAL	
       str   r1, [r0,#OMAP2420_GPMC_CONFIG7_1_OA]
         
	   ;SDRC and GPMC configuration ends here


        ; Clean & invalidate D cache
;110
;		mrc     p15, 0, r15, c7, c14, 3
;        bne     %b110
        
        ; Invalidate I cache
        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 0
       
        ; Disable I and D caches and write buffer
        mrc     p15, 0, r0, c1, c0, 0
        ldr		r1, =0xFFFFEFF3	        
        and     r0, r0, r1           ; I-cache, D-cache, write buffer
        mcr     p15, 0, r0, c1, c0, 0

;11		ldr   r0, =0x08000016
		ldr   r0, =0x08000016
        ldr   r1, =0xFF00 ;LED_REG_VAL1
        strh  r1, [r0]
;	    b	%b11

       ;*-------------------------------------------
	   ; Configure PRCM registers
	   ;*-------------------------------------------

	   ; Configure the Core clock source to be DPLL, by default it is DPLL * 2.
	   ; Set Core clock = DPLL
	   ldr r0, =OMAP2420_PRCM_REGS_PA
	   ; Allow the core clk to use the default value of DPLL * 2.
  
       ldr r1, [r0,#OMAP2420_PRCM_CM_CLKSEL2_PLL_OA]
       bic r1, r1,#BSP_PRCM_CM_CLKSEL2_PLL_MASK     ; Clear bits 0,1
       orr r1, r1,#BSP_PRCM_CM_CLKSEL2_PLL_DPLL_SET ; core = dpll * 1
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL2_PLL_OA]
                
	   ;Put DPLL in low power bypass.
       ;Configure the value for CM_CLKEN_PLL[1:0] (EN_DPLL)
       ldr r1,[r0,#OMAP2420_PRCM_CM_CLKEN_PLL_OA]
       bic r1,r1,#BSP_PRCM_CM_CLKEN_PLL_MASK1
       orr r1,r1,#BSP_PRCM_CM_CLKEN_PLL_DPLL_SET1
       str r1,[r0,#OMAP2420_PRCM_CM_CLKEN_PLL_OA]
        
       ;Select MPU clock
       ;configure the value for CM_CLKSEL_MPU[4:0]
       ldr r1,[r0,#OMAP2420_PRCM_CM_CLKSEL_MPU_OA]
       bic r1,r1,#BSP_PRCM_CM_CLKSEL_MPU_MASK
       orr r1,r1,#BSP_PRCM_CM_CLKSEL_MPU_SET      ; mpu clock = core clock / 2
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL_MPU_OA]
        
       ;Select DSP clock
       ;configure the value for CM_CLKSEL_DSP[13:0]
       ldr r1, =BSP_PRCM_CM_CLKSEL_DSP_VAL	
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL_DSP_OA]	
        
       ;Select GFX clock
       ;configure the value for CM_CLKSEL_GFX[2:0]
       ldr r1, =BSP_PRCM_CM_CLKSEL_GFX_VAL	
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL_GFX_OA]
        
       ;Select 
       ;configure the value for CM_CLKSEL1_CORE
       ldr r1, =BSP_PRCM_CM_CLKSEL1_CORE_VAL
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL1_CORE_OA]
        
	   ;Now check for clock validity and make all the PRCM changes effective.
       ldr r1,[r0,#OMAP2420_PRCM_CLKCFG_STATUS_OA]	
       and r1, r1, #0x01 	; Check if bit 0 is ONE.
       cmp r1,#0x01	
       bne %F50          	; If it is 1 then stay in a forever loop,
                            ; otherwise proceed to prcm_validate.
40      nop
       b %B40     			; stay in a forever loop.         
         
50
       mov r1,#0x01		    ; write 1 into bit 0 of PRCM_CLKCFG_CTRL register
       str r1,[r0,#OMAP2420_PRCM_CLKCFG_CTRL_OA]
		
       ; Start configuring the Level 0 registers
       ; DPLL is 300
	   ; Selects 12MHz for the APLL
       ldr r1, =BSP_PRCM_CM_CLKSEL1_PLL_VAL 
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL1_PLL_OA]
       
       ;Enable APLL 96MHz.
       ;Retain the default value for CM_CLKEN_PLL register EN_96_PLL
       ;Retain the default value for CM_AUTOIDLE_PLL[3:2] (AUTO_96M)
        
       ;Enable APLL 54MHz.
       ;Configure the value for CM_CLKEN_PLL[7:6] (EN_54M_PLL)
       ldr r1,[r0,#OMAP2420_PRCM_CM_CLKEN_PLL_OA]
       bic r1,r1,#BSP_PRCM_CM_CLKEN_PLL_MASK2
       orr r1,r1,#BSP_PRCM_CM_CLKEN_PLL_APLL_SET
       str r1,[r0,#OMAP2420_PRCM_CM_CLKEN_PLL_OA]

       ;Retain the default value for CM_AUTOIDLE_PLL[7:6] (AUTO_54M)
	   ;Scale up the processor speed.
       ;Configure CM_CLKSEL1_PLL[21:12] - multiplication factor
       ;CM_CLKSEL1_PLL[11:8] - division factor
 	   
       ;Configure the value for CM_CLKEN_PLL[1:0] (EN_DPLL)
       bic r1,r1,#BSP_PRCM_CM_CLKEN_PLL_MASK1
       orr r1,r1,#BSP_PRCM_CM_CLKEN_PLL_DPLL_SET2
        
       ;* loop for 4000 times *;
       mov r6, #0xFF0  ; load 1024 in r0
60
       nop
       subs r6, r6, #1 ; decrement by one
       cmp r6, #0x0    ; check if its equal to 0
	   bne %B60
		str r1,[r0,#OMAP2420_PRCM_CM_CLKEN_PLL_OA]
        
       mov r6, #0xFF0  ; load 1024 in r0
70
       nop
       subs r6, r6, #1 ; decrement by one
       cmp r6, #0x0    ; check if its equal to 0
       bne %B70
        
       nop
       nop
       nop
       nop
                
       ;Configuration of Level 0 registers ends here.
        
       ;Start configuring the Level 1 registers.
       ;Retain the Default value of CM_CLKSEL1_PLL1[3] (48M_source)
       ;and CM_CLKSEL1_PLL1[5] (54M_source) which means the source for these
       ;clocks is from 96 MHz APLL, 54 MHz APLL respectively. 
        
       ;Configure the MPU clock. CM_CLKSEL_MPU [4:0] is by default 1
       ;which means MPU_CLK = Core_clk/1. So it is 12 MHz.
        
       ;PRCM_CLKOUT_CTRL[1:0](ClkOut_source) must be made 1. 
       ;since the clock out is connected to Codec, which requires 12 MHz MCLK, 
       ;make the source as system clock which is 12MHz
       ldr r1,[r0, #OMAP2420_PRCM_CLKOUT_CTRL_OA] 
       bic r1,r1,#BSP_PRCM_CLKOUT_CTRL_MASK	;clear the bits 0,1
       orr r1,r1,#BSP_PRCM_CLKOUT_CTRL_SET	;enable bit number 0 for 12MHz sys clock out, 7 to enable sys clock out
       str r1,[r0, #OMAP2420_PRCM_CLKOUT_CTRL_OA] 
                
       ;Configuration of Level 1 registers ends here.
        	        	
       ;--------------------------------------------
	   ;         PRCM Modules Clock Enable
       ;--------------------------------------------
        
       ldr r0, =OMAP2420_PRCM_REGS_PA
       ldr r1, =BSP_PRCM_CLKEMUL_CTRL_VAL 
       str r1,[r0,#OMAP2420_PRCM_CLKEMUL_CTRL_OA]
       ldr r1, =BSP_PRCM_CM_xCLKENx_CORE_VAL
       str r1,[r0,#OMAP2420_PRCM_CM_FCLKEN1_CORE_OA] 
       str r1,[r0,#OMAP2420_PRCM_CM_FCLKEN2_CORE_OA]
       str r1,[r0,#OMAP2420_PRCM_CM_ICLKEN1_CORE_OA]
       str r1,[r0,#OMAP2420_PRCM_CM_ICLKEN2_CORE_OA]
       ldr r1, =BSP_PRCM_CM_CLKSEL2_CORE_VAL
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL2_CORE_OA] ;set GPTn clk to sysclk (only one available)
       ldr r1, =BSP_PRCM_CM_CLKSEL_WKUP_VAL ;0x01 
       str r1,[r0,#OMAP2420_PRCM_CM_CLKSEL_WKUP_OA]  ;set GPT1 clk to sysclk (only one available)
       ldr r1, =BSP_PRCM_CM_FCLKEN_WKUP_VAL ;0x0F 
       str r1,[r0,#OMAP2420_PRCM_CM_FCLKEN_WKUP_OA]
       ldr r1, =BSP_PRCM_CM_ICLKEN_WKUP_VAL ;0x3F 
       str r1,[r0,#OMAP2420_PRCM_CM_ICLKEN_WKUP_OA]
    	
       nop
    	
       ;------Coprocessor Control reset-------
       ;--------------------------------------
       MOV r0, #0x00050000
       ADD r0, r0, #0x78
       MCR p15, 0, r0, c1, c0, 0 ; reset Control Coprocessor register
       ;--------------------------------------
	
	   ;--------Peripheral port mapping-------
	   ;--------------------------------------
       LDR r0,=0x480FE003                    ;INTH_SETUP (Need to check this register, Not available in 2420 TRM)
       MCR p15, 0, r0, c15, c2, 4

       LDR r3,=0x00008000                    ; L4 MASK
       MCR p15, 0, r3, c1, c0, 0
  	 
;-------------------------------------------------------------------------------

        END
