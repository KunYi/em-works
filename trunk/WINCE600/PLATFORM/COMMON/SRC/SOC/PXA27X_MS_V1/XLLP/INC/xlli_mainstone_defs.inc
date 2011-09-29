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
;*********************************************************************************
;
;        COPYRIGHT (c) 2002 - 2004 Intel Corporation
;
;   The information in this file is furnished for informational use only,
;   is subject to change without notice, and should not be construed as
;   a commitment by Intel Corporation. Intel Corporation assumes no
;   responsibility or liability for any errors or inaccuracies that may appear
;   in this document or any software that may be provided in association with
;   this document.
;
;*********************************************************************************
;
;  FILENAME:       xlli_Mainstone_defs.inc (Platform specific addresses and
;                  defalut values for Mainstone platform bring up)
;                  NOTE: - This file has a def to configure xlli for MCP and non-MCP processors
;
; LAST MODIFIED:   13-Feb-2004
;
;******************************************************************************
;
;
; Include file for Mainstone specific Cross Platform Low Level Initialization (XLLI)
;
;
; PLATFORM REGISTERS base address and register offsets from the base address
;
xlli_PLATFORM_REGISTERS              EQU    0x08000000

xlli_PLATFORM_HEXLED_DATA_offset     EQU    (0x10)   ; Hex LED Data Register
xlli_PLATFORM_LED_CONTROL_offset     EQU    (0x40)   ; LED Control Register
xlli_PLATFORM_SWITCH_offset          EQU    (0x60)   ; General Purpose Switch Register
xlli_PLATFORM_MISC_WRITE1_offset     EQU    (0x80)   ; Misc Write Register 1
xlli_PLATFORM_MISC_WRITE2_offset     EQU    (0x84)   ; Misc Write Register 2
xlli_PLATFORM_MISC_READ1_offset      EQU    (0x90)   ; Misc Read Register 1
xlli_PLATFORM_INTERR_ME_offset       EQU    (0xC0)   ; Platform Interrupt Mask/Enable Register 1
xlli_PLATFORM_INTERR_SC_offset       EQU    (0xD0)   ; Platform Interrupt Set/Clear Register 1
xlli_PLATFORM_PCMCIA0_SC_offset      EQU    (0xE0)   ; PCMCIA Socket 0 Status/Control Register
xlli_PLATFORM_PCMCIA1_SC_offset      EQU    (0xE4)   ; PCMCIA Socket 1 Status/Control Register

;
; Platform specific bits
;
xlli_SYS_RESET                       EQU    (0x01)   ; System reset bit

;
; platform GPIO pin settings (Bulverde/Mainstone)
;
xlli_GPSR0_value          EQU   (0x00008004)   ; Set registers
xlli_GPSR1_value          EQU   (0x00020080)
xlli_GPSR2_value          EQU   (0x16C14000)
xlli_GPSR3_value          EQU   (0x0003E000)

xlli_GPCR0_value          EQU   (0x0)          ; Clear registers
xlli_GPCR1_value          EQU   (0x00000380)   ; FFUART related
xlli_GPCR2_value          EQU   (0x0)
xlli_GPCR3_value          EQU   (0x0)

xlli_GRER0_value          EQU   (0x0)          ; Rising Edge Detect
xlli_GRER1_value          EQU   (0x0)
xlli_GRER2_value          EQU   (0x0)
xlli_GRER3_value          EQU   (0x0)

xlli_GFER0_value          EQU   (0x0)          ; Falling Edge Detect
xlli_GFER1_value          EQU   (0x0)
xlli_GFER2_value          EQU   (0x0)
xlli_GFER3_value          EQU   (0x0)

xlli_GPLR0_value          EQU   (0x0)          ; Pin Level Registers
xlli_GPLR1_value          EQU   (0x0)
xlli_GPLR2_value          EQU   (0x0)
xlli_GPLR3_value          EQU   (0x0)

xlli_GEDR0_value          EQU   (0x0)          ; Edge Detect Status
xlli_GEDR1_value          EQU   (0x0)
xlli_GEDR2_value          EQU   (0x0)
xlli_GEDR3_value          EQU   (0x0)

xlli_GPDR0_value          EQU   (0xCFE3BDE4)   ; Direction Registers
xlli_GPDR1_value          EQU   (0x003FAB81)
xlli_GPDR2_value          EQU   (0x1EC3FC00)
xlli_GPDR3_value          EQU   (0x018FFE8F)

xlli_GAFR0_L_value EQU  (0x84400000)   ; Alternate function registers
xlli_GAFR0_U_value EQU  (0xA5000510)
xlli_GAFR1_L_value EQU  (0x000A9558)
xlli_GAFR1_U_value EQU  (0x0005A1AA)
xlli_GAFR2_L_value EQU  (0x60000000)
xlli_GAFR2_U_value EQU  (0x00000802)
xlli_GAFR3_L_value EQU  (0x00000000)
xlli_GAFR3_U_value EQU  (0x00000000)

;
; MEMORY CONTROLLER SETTINGS FOR MAINSTONE
;

xlli_MDREFR_value  EQU   (0x0000001E)
        IF  :DEF:  xlli_FLASH_WIDTH_16_BIT
xlli_MSC0_DC_value EQU   (0x7FF07FFA)     ; Bulverde Card Flash value (MCP version)
        ELSE
xlli_MSC0_DC_value EQU   (0x7FF0B8F2)     ; Bulverde Card Flash value (Non-MCP version)
        ENDIF
xlli_MSC0_MS_value EQU   (0x23F2B8F2)     ; Mainstone Board Flash value
xlli_MSC1_value    EQU   (0x0000CCD1)
xlli_MSC2_value    EQU   (0x0000B884)
xlli_MECR_value    EQU   (0x00000001)
xlli_MCMEM0_value  EQU   (0x00014307)
xlli_MCMEM1_value  EQU   (0x00014307)
xlli_MCATT0_value  EQU   (0x0001C787)
xlli_MCATT1_value  EQU   (0x0001C787)
xlli_MCIO0_value   EQU   (0x000D4B3F)
xlli_MCIO1_value   EQU   (0x000D4B3F)
xlli_FLYCNFG_value EQU   (0x00010001)
xlli_MDMRSLP_value EQU   (0x0000C008)
xlli_SXCNFG_value  EQU   (0x40044004)   ; Default value at boot up


;
; Optimal values for MSCO for various MemClk frequencies are listed below
; These values are for L18 async flash
;
 IF :DEF: xlli_C0_BULVERDE

xlli_MSC0_13       EQU   (0x11101110)
xlli_MSC0_19       EQU   (0x11101110)
xlli_MSC0_26       EQU   (0x11201120)   ; 26 MHz setting
xlli_MSC0_32       EQU   (0x11201120)
xlli_MSC0_39       EQU   (0x11301130)   ; 39 MHz setting
xlli_MSC0_45       EQU   (0x11301130)
xlli_MSC0_52       EQU   (0x11401140)   ; 52 MHz setting
xlli_MSC0_58       EQU   (0x11401140)
xlli_MSC0_65       EQU   (0x11501150)   ; 65 MHz setting
xlli_MSC0_68       EQU   (0x11501150)
xlli_MSC0_71       EQU   (0x11501150)   ; 71.5 MHz setting
xlli_MSC0_74       EQU   (0x11601160)
xlli_MSC0_78       EQU   (0x12601260)   ; 78 MHz setting
xlli_MSC0_81       EQU   (0x12601260)
xlli_MSC0_84       EQU   (0x12601260)   ; 84.5 MHz setting
xlli_MSC0_87       EQU   (0x12701270)
xlli_MSC0_91       EQU   (0x12701270)   ; 91 MHz setting
xlli_MSC0_94       EQU   (0x12701270)   ; 94.2 MHz setting
xlli_MSC0_97       EQU   (0x12701270)   ; 97.5 MHz setting
xlli_MSC0_100      EQU   (0x12801280)   ; 100.7 MHz setting
xlli_MSC0_104      EQU   (0x12801280)   ; 104 MHz setting
xlli_MSC0_110      EQU   (0x12901290)
xlli_MSC0_117      EQU   (0x13901390)   ; 117 MHz setting
xlli_MSC0_124      EQU   (0x13A013A0)
xlli_MSC0_130      EQU   (0x13A013A0)   ; 130 MHz setting
xlli_MSC0_136      EQU   (0x13B013B0)
xlli_MSC0_143      EQU   (0x13B013B0)
xlli_MSC0_149      EQU   (0x13C013C0)
xlli_MSC0_156      EQU   (0x14C014C0)
xlli_MSC0_162      EQU   (0x14C014C0)
xlli_MSC0_169      EQU   (0x14C014C0)
xlli_MSC0_175      EQU   (0x14C014C0)
xlli_MSC0_182      EQU   (0x14C014C0)
xlli_MSC0_188      EQU   (0x14C014C0)
xlli_MSC0_195      EQU   (0x15C015C0)
xlli_MSC0_201      EQU   (0x15D015D0)
xlli_MSC0_208      EQU   (0x15D015D0)

    ELSE

; This is a hack to get around some B0 timing issue where it doesn't like the optimal
; values according to it's own SPEC!?!  These timing values are relaxed from the above optimal
; but they work for B-step Bulverde... ugh....

xlli_MSC0_13       EQU   (0x12101210)
xlli_MSC0_19       EQU   (0x12101210)
xlli_MSC0_26       EQU   (0x12201220)   ; 26 MHz setting
xlli_MSC0_32       EQU   (0x12201220)
xlli_MSC0_39       EQU   (0x13301330)   ; 39 MHz setting
xlli_MSC0_45       EQU   (0x13301330)
xlli_MSC0_52       EQU   (0x13401340)   ; 52 MHz setting
xlli_MSC0_58       EQU   (0x13601360)
xlli_MSC0_65       EQU   (0x13501350)   ; 65 MHz setting
xlli_MSC0_68       EQU   (0x13501350)
xlli_MSC0_71       EQU   (0x14601460)   ; 71.5 MHz setting
xlli_MSC0_74       EQU   (0x14601460)
xlli_MSC0_78       EQU   (0x14601460)   ; 78 MHz setting
xlli_MSC0_81       EQU   (0x14701470)
xlli_MSC0_84       EQU   (0x14701470)   ; 84.5 MHz setting
xlli_MSC0_87       EQU   (0x14701470)
xlli_MSC0_91       EQU   (0x14701470)   ; 91 MHz setting
xlli_MSC0_94       EQU   (0x14801480)   ; 94.2 MHz setting
xlli_MSC0_97       EQU   (0x14801480)   ; 97.5 MHz setting
xlli_MSC0_100      EQU   (0x15801580)   ; 100.7 MHz setting
xlli_MSC0_104      EQU   (0x15801580)   ; 104 MHz setting
xlli_MSC0_110      EQU   (0x15901590)
xlli_MSC0_117      EQU   (0x15A015A0)   ; 117 MHz setting
xlli_MSC0_124      EQU   (0x15A015A0)
xlli_MSC0_130      EQU   (0x15B015B0)   ; 130 MHz setting
xlli_MSC0_136      EQU   (0x16B016B0)
xlli_MSC0_143      EQU   (0x16C016C0)
xlli_MSC0_149      EQU   (0x16C016C0)
xlli_MSC0_156      EQU   (0x16C016C0)
xlli_MSC0_162      EQU   (0x16C016C0)
xlli_MSC0_169      EQU   (0x17D017D0)   ; Given that the optimal value would be 13 (RDF), but according to B0 manual, it's different
xlli_MSC0_175      EQU   (0x17C017C0)
xlli_MSC0_182      EQU   (0x17C017C0)
xlli_MSC0_188      EQU   (0x17D017D0)
xlli_MSC0_195      EQU   (0x17E017E0)
xlli_MSC0_201      EQU   (0x18E018E0)
xlli_MSC0_208      EQU   (0x18E018E0)

    ENDIF ; xlli_C0_BULVERDE

;
; Optimal values for DTC settings for various MemClk settings (MDCNFG)
;
    IF  :DEF:  xlli_SDRAM_WIDTH_16_BIT

; Really, we should have yet another flag that is dependant on the SDRAM part, but since only
; the MCP's SDRAM parameters are different than the MSII platform, I'm choosing to pick the 16-bit
; width to trigger for the different timing values

xlli_DTC_13        EQU   (0x00000000)   ; 13 MHz setting
xlli_DTC_19        EQU   (0x00000000)   ; 19 MHz setting
xlli_DTC_26        EQU   (0x00000000)   ; 26 MHz setting
xlli_DTC_32        EQU   (0x00000000)   ; 32 MHz setting
xlli_DTC_39        EQU   (0x01000100)   ; 39 MHz setting
xlli_DTC_45        EQU   (0x01000100)   ; 45 MHz setting
xlli_DTC_52        EQU   (0x01000100)   ; 52 MHz setting
xlli_DTC_58        EQU   (0x01000100)   ; 58 MHz setting
xlli_DTC_65        EQU   (0x01000100)   ; 65 MHz setting
xlli_DTC_68        EQU   (0x02000200)   ; 68 MHz setting
xlli_DTC_71        EQU   (0x02000200)   ; 71 MHz setting
xlli_DTC_74        EQU   (0x02000200)   ; 74 MHz setting
xlli_DTC_78        EQU   (0x02000200)   ; 78 MHz setting
xlli_DTC_81        EQU   (0x02000200)   ; 81 MHz setting
xlli_DTC_84        EQU   (0x02000200)   ; 84 MHz setting
xlli_DTC_87        EQU   (0x02000200)   ; 87 MHz setting
xlli_DTC_91        EQU   (0x02000200)   ; 91 MHz setting
xlli_DTC_94        EQU   (0x02000200)   ; 94 MHz setting
xlli_DTC_97        EQU   (0x02000200)   ; 97 MHz setting
xlli_DTC_100       EQU   (0x03000300)   ; 100 MHz setting
xlli_DTC_104       EQU   (0x03000300)   ; 104 MHz setting
xlli_DTC_110       EQU   (0x01000100)   ; 110 MHz setting - SDCLK Halved
xlli_DTC_117       EQU   (0x01000100)   ; 117 MHz setting - SDCLK Halved
xlli_DTC_124       EQU   (0x01000100)   ; 124 MHz setting - SDCLK Halved
xlli_DTC_130       EQU   (0x02000200)   ; 130 MHz setting - SDCLK Halved
xlli_DTC_136       EQU   (0x02000200)   ; 136 MHz setting - SDCLK Halved
xlli_DTC_143       EQU   (0x02000200)   ; 143 MHz setting - SDCLK Halved
xlli_DTC_149       EQU   (0x02000200)   ; 149 MHz setting - SDCLK Halved
xlli_DTC_156       EQU   (0x02000200)   ; 156 MHz setting - SDCLK Halved
xlli_DTC_162       EQU   (0x02000200)   ; 162 MHz setting - SDCLK Halved
xlli_DTC_169       EQU   (0x02000200)   ; 169 MHz setting - SDCLK Halved
xlli_DTC_175       EQU   (0x02000200)   ; 175 MHz setting - SDCLK Halved
xlli_DTC_182       EQU   (0x02000200)   ; 182 MHz setting - SDCLK Halved
xlli_DTC_188       EQU   (0x02000200)   ; 188 MHz setting - SDCLK Halved
xlli_DTC_195       EQU   (0x02000200)   ; 195 MHz setting - SDCLK Halved
xlli_DTC_201       EQU   (0x03000300)   ; 201 MHz setting - SDCLK Halved
xlli_DTC_208       EQU   (0x03000300)   ; 208 MHz setting - SDCLK Halved

;
; Optimal values for DRI settings for various MemClk settings (MDREFR)
;
xlli_DRI_13        EQU   (0x002)   ; 13 MHz setting
xlli_DRI_19        EQU   (0x003)
xlli_DRI_26        EQU   (0x004)   ; 26 MHz setting
xlli_DRI_32        EQU   (0x006)
xlli_DRI_39        EQU   (0x007)   ; 39 MHz setting
xlli_DRI_45        EQU   (0x009)
xlli_DRI_52        EQU   (0x00A)   ; 52 MHz setting
xlli_DRI_58        EQU   (0x00C)
xlli_DRI_65        EQU   (0x00D)   ; 65 MHz setting
xlli_DRI_68        EQU   (0x00E)
xlli_DRI_71        EQU   (0x00F)   ; 71 MHz setting
xlli_DRI_74        EQU   (0x010)
xlli_DRI_78        EQU   (0x010)   ; 78 MHz setting
xlli_DRI_81        EQU   (0x011)
xlli_DRI_84        EQU   (0x012)   ; 84 MHz setting
xlli_DRI_87        EQU   (0x013)
xlli_DRI_91        EQU   (0x013)   ; 91 MHz setting
xlli_DRI_94        EQU   (0x014)   ; 94 MHz setting
xlli_DRI_97        EQU   (0x015)   ; 97 MHz setting
xlli_DRI_100       EQU   (0x016)   ; 100 MHz setting
xlli_DRI_104       EQU   (0x016)   ; 104 MHz setting
xlli_DRI_110       EQU   (0x018)
xlli_DRI_117       EQU   (0x019)   ; 117 MHz setting
xlli_DRI_124       EQU   (0x01B)
xlli_DRI_130       EQU   (0x01C)   ; 130 MHz setting
xlli_DRI_136       EQU   (0x01E)
xlli_DRI_143       EQU   (0x01F)
xlli_DRI_149       EQU   (0x021)
xlli_DRI_156       EQU   (0x022)
xlli_DRI_162       EQU   (0x024)
xlli_DRI_169       EQU   (0x025)   ; 169 MHz setting
xlli_DRI_175       EQU   (0x027)
xlli_DRI_182       EQU   (0x028)
xlli_DRI_188       EQU   (0x02A)
xlli_DRI_195       EQU   (0x02B)
xlli_DRI_201       EQU   (0x02D)
xlli_DRI_208       EQU   (0x02E)   ; 208 MHz setting


    ELSE ; ELSE not MCP package

; Now on a discrete MS II platform with slightly better performing SDRAM

xlli_DTC_13        EQU   (0x00000000)   ; 13 MHz setting
xlli_DTC_19        EQU   (0x00000000)   ; 19 MHz setting
xlli_DTC_26        EQU   (0x00000000)   ; 26 MHz setting
xlli_DTC_32        EQU   (0x00000000)   ; 32 MHz setting
xlli_DTC_39        EQU   (0x00000000)   ; 39 MHz setting
xlli_DTC_45        EQU   (0x00000000)   ; 45 MHz setting
xlli_DTC_52        EQU   (0x00000000)   ; 52 MHz setting
xlli_DTC_58        EQU   (0x01000100)   ; 58 MHz setting
xlli_DTC_65        EQU   (0x01000100)   ; 65 MHz setting
xlli_DTC_68        EQU   (0x01000100)   ; 68 MHz setting
xlli_DTC_71        EQU   (0x01000100)   ; 71 MHz setting
xlli_DTC_74        EQU   (0x01000100)   ; 74 MHz setting
xlli_DTC_78        EQU   (0x01000100)   ; 78 MHz setting
xlli_DTC_81        EQU   (0x01000100)   ; 81 MHz setting
xlli_DTC_84        EQU   (0x01000100)   ; 84 MHz setting
xlli_DTC_87        EQU   (0x01000100)   ; 87 MHz setting
xlli_DTC_91        EQU   (0x02000200)   ; 91 MHz setting
xlli_DTC_94        EQU   (0x02000200)   ; 94 MHz setting
xlli_DTC_97        EQU   (0x02000200)   ; 97 MHz setting
xlli_DTC_100       EQU   (0x02000200)   ; 100 MHz setting
xlli_DTC_104       EQU   (0x02000200)   ; 104 MHz setting
xlli_DTC_110       EQU   (0x01000100)   ; 110 MHz setting - SDCLK Halved
xlli_DTC_117       EQU   (0x01000100)   ; 117 MHz setting - SDCLK Halved
xlli_DTC_124       EQU   (0x01000100)   ; 124 MHz setting - SDCLK Halved
xlli_DTC_130       EQU   (0x01000100)   ; 130 MHz setting - SDCLK Halved
xlli_DTC_136       EQU   (0x01000100)   ; 136 MHz setting - SDCLK Halved
xlli_DTC_143       EQU   (0x01000100)   ; 143 MHz setting - SDCLK Halved
xlli_DTC_149       EQU   (0x01000100)   ; 149 MHz setting - SDCLK Halved
xlli_DTC_156       EQU   (0x01000100)   ; 156 MHz setting - SDCLK Halved
xlli_DTC_162       EQU   (0x01000100)   ; 162 MHz setting - SDCLK Halved
xlli_DTC_169       EQU   (0x01000100)   ; 169 MHz setting - SDCLK Halved
xlli_DTC_175       EQU   (0x01000100)   ; 175 MHz setting - SDCLK Halved
xlli_DTC_182       EQU   (0x02000200)   ; 182 MHz setting - SDCLK Halved - Close to edge, so bump up
xlli_DTC_188       EQU   (0x02000200)   ; 188 MHz setting - SDCLK Halved - Close to edge, so bump up
xlli_DTC_195       EQU   (0x02000200)   ; 195 MHz setting - SDCLK Halved - Close to edge, so bump up
xlli_DTC_201       EQU   (0x02000200)   ; 201 MHz setting - SDCLK Halved - Close to edge, so bump up
xlli_DTC_208       EQU   (0x02000200)   ; 208 MHz setting - SDCLK Halved - Close to edge, so bump up

;
; Optimal values for DRI settings for various MemClk settings (MDREFR)
;
xlli_DRI_13        EQU   (0x002)   ; 13 MHz setting
xlli_DRI_19        EQU   (0x003)
xlli_DRI_26        EQU   (0x005)   ; 26 MHz setting
xlli_DRI_32        EQU   (0x006)
xlli_DRI_39        EQU   (0x008)   ; 39 MHz setting
xlli_DRI_45        EQU   (0x00A)
xlli_DRI_52        EQU   (0x00B)   ; 52 MHz setting
xlli_DRI_58        EQU   (0x00D)
xlli_DRI_65        EQU   (0x00E)   ; 65 MHz setting
xlli_DRI_68        EQU   (0x00F)
xlli_DRI_71        EQU   (0x010)   ; 71 MHz setting
xlli_DRI_74        EQU   (0x011)
xlli_DRI_78        EQU   (0x012)   ; 78 MHz setting
xlli_DRI_81        EQU   (0x012)
xlli_DRI_84        EQU   (0x013)   ; 84 MHz setting
xlli_DRI_87        EQU   (0x014)
xlli_DRI_91        EQU   (0x015)   ; 91 MHz setting
xlli_DRI_94        EQU   (0x016)   ; 94 MHz setting
xlli_DRI_97        EQU   (0x016)   ; 97 MHz setting
xlli_DRI_100       EQU   (0x017)   ; 100 MHz setting
xlli_DRI_104       EQU   (0x018)   ; 104 MHz setting
xlli_DRI_110       EQU   (0x01A)
xlli_DRI_117       EQU   (0x01B)   ; 117 MHz setting
xlli_DRI_124       EQU   (0x01D)
xlli_DRI_130       EQU   (0x01E)   ; 130 MHz setting
xlli_DRI_136       EQU   (0x020)
xlli_DRI_143       EQU   (0x021)
xlli_DRI_149       EQU   (0x023)
xlli_DRI_156       EQU   (0x025)
xlli_DRI_162       EQU   (0x026)
xlli_DRI_169       EQU   (0x028)   ; 169 MHz setting
xlli_DRI_175       EQU   (0x029)
xlli_DRI_182       EQU   (0x02B)
xlli_DRI_188       EQU   (0x02D)
xlli_DRI_195       EQU   (0x02E)
xlli_DRI_201       EQU   (0x030)
xlli_DRI_208       EQU   (0x031)   ; 208 MHz setting

    ENDIF   ; xlli_SDRAM_WIDTH_16_BIT

; SDRAM Settings
    IF  :DEF:  xlli_SDRAM_WIDTH_16_BIT

; Again, using the 16-bit flag since the SDRAM in question is MCP specific and currently
; the only 16-bit bus is planned on a MCP
xlli_MDCNFG_value  EQU   (0x00002BCC)   ; SDRAM Config Reg (MCP Version)

    ELSE
xlli_MDCNFG_value  EQU   (0x00000AC8)   ; SDRAM Config Reg (Non-MCP Version)
    ENDIF

xlli_MDMRS_value   EQU   (0x00000000)   ; SDRAM Mode Reg Set Config Reg

;
; MEMORY PHYSICAL BASE ADDRESS(S)
;

xlli_SRAM_PHYSICAL_BASE      EQU       (0X5C000000)  ; Physical base address for SRAM
xlli_SDRAM_PHYSICAL_BASE     EQU       (0xA0000000)  ; Physical base address for SDRAM

;
; CORE, SYSTEM BUS, MEMORY BUS Default frequency setting for Mainstone
;
xlli_CCCR_value      EQU     (0x00000107)  ; Bulverde (HW reset value to start)
;
; Clock Enable Register (CKEN) setting
;
xlli_CKEN_value      EQU     (0x00400200)  ; Data to be set into the clock enable register
                                           ; bit 9 enables OS timers
                                           ; Bit 22 enables memory clock
;
; Address where system configuration data is stored
;
xlli_SCR_data        EQU     (0x5C03FFFC) ; Address in SRAM where system config data is stored

;
; Misc constants
;
xlli_MemSize_1Mb     EQU     (0x00100000)
        IF  :DEF: xlli_SDRAM_SIZE_32_MB
xlli_p_PageTable      EQU     (0xA1FFC000)   ; Base address for memory Page Table (MCP version)
        ELSE
xlli_p_PageTable      EQU     (0xA3FFC000)   ; Base address for memory Page Table (Non-MCP version)
        ENDIF
xlli_s_PageTable      EQU     (0x00004000)   ; Page Table size (4K words - 16 Kb)

    IF :DEF: POST_BUILD
xlli_v_xbBOOTROM          EQU     (0x04000000)   ; (0x04000000 for POST)
    ELSE
xlli_v_xbBOOTROM          EQU     (0x00000000)
    ENDIF;

      END
