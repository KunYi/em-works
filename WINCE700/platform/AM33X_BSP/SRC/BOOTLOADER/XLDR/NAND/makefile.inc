# All rights reserved ADENEO EMBEDDED 2010
!if 0 
;  Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

;================================================================================
;             Texas Instruments OMAP(TM) Platform Software
; (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
;
; Use of this software is controlled by the terms and conditions found
; in the license agreement under which this software has been supplied.
;
;================================================================================
!endif

#
# Take xldr.nb0, ebootnd.nb0 and iplnand.nb0 (WM only) to generate raw files
#
# Boot ROM requires xldr to be preceeded with two words containing image 
# size and destination address.  The xldrtocnand.raw file contains these
# two words and is prepended to the xldr image.
# 
# Note that boot ROM expects xldr to be copied into 4 consecutive blocks
# to support data redundancy.  Blocks are 128K while xldr is only 16K
# so there will be gaps in each block.  For ease of creating the image
# the xldr nb0 file has been configured to be a full 128K minus the 
# table of contents.

# _TGT=C:\WINCE700\OSDesigns\AM387X_OS\AM387X_OS\Wince700\AM387X_BSP_ARMV7_Retail\cesysgen\platform\AM387X_BSP\target\ARMV7\retail

_FILES=$(_PLATFORMROOT)\$(_TGTPLAT)\files
_SRC=$(_PLATFORMROOT)\$(_TGTPLAT)\src\bootloader\xldr\nand
_TGT=$(SG_OUTPUT_ROOT)\platform\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)
_BIN=$(_PLATFORMROOT)\$(_TGTPLAT)\bin
_PLATNAME=AM33X

xldr_romimage_raw:
!if "$(NOLINK)" == ""
    romimage $(ROMIMAGE_FLAGS) xldrnand.bib
#   The OMAP boot ROM will not load the NAND XLDR unless some bytes are modified near the image start
    $(_FILES)\fileset.exe $(_TGT)\xldrnand.nb0 0x4 0x3c 0xff
	copy /Y $(_FILES)\Logo.bmp $(_TGT)\logo.nb0 
!if "$(WINCEREL)"=="1"
    xcopy /Q /I $(_TGT)\xldrnand.* $(_FLATRELEASEDIR)
	xcopy /Q /I $(_TGT)\logo.nb0 $(_FLATRELEASEDIR)
!endif
!endif

!if "$(NOLINK)" == ""
    copy /b $(_SRC)\xldrtocnand.raw + $(_TGT)\xldrnand.nb0 $(_TGT)\xldrnand.1block
    copy /b $(_TGT)\xldrnand.1block + $(_TGT)\xldrnand.1block + $(_TGT)\xldrnand.1block + $(_TGT)\xldrnand.1block $(_TGT)\xldrnand.4blocks
#    del $(_TGT)\xldrnand.1block >nul

    copy /b $(_TGT)\xldrnand.4blocks + $(_TGT)\ebootnd.nb0 $(_TGT)\$(_PLATNAME)-nand.raw

    copy /b $(_TGT)\$(_PLATNAME)-nand.raw + $(_FILES)\Logo.bmp $(_TGT)\$(_PLATNAME)-nand-logo.raw
    del $(_TGT)\xldrnand.4blocks >nul

!if "$(WINCEREL)"=="1"
    xcopy /Q /I $(_TGT)\xldrnand.1block $(_FLATRELEASEDIR)
    xcopy /Q /I $(_TGT)\*.raw $(_FLATRELEASEDIR)
!endif
!endif        
