@REM 
@REM Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
@REM THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
@REM AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
@REM 
@echo off & setlocal EnableDelayedExpansion

@REM Set path variables
set _TARGETIMGDIR=%_TARGETPLATROOT%\target\%_TGTCPU%\%WINCEDEBUG%

set _SUPPORT=SUPPORT_PDK1_9
set/a _FOUND_PDK=0

@REM Save current dir and goto tool dir
set _CURRENTDIR=%CD%
cd %_WINCEROOT%\%_SUPPORT%\TOOL\iMX28-EVK\SBIMAGE

@REM Copy xldr and eboot
copy /Y %_TARGETIMGDIR%\xldr.nb0
copy /Y %_TARGETIMGDIR%\eboot.nb0

@REM Generate eboot.sb
elftosb -z -c eboot.bd -o eboot.sb
elftosb -z -f imx28 -c eboot_ivt.bd -o eboot_ivt.sb

@REM Add manifest info
copy /Y eboot.sb eboot.msb
copy /Y eboot_ivt.sb eboot_ivt.msb
manifest eboot.msb
manifest eboot_ivt.msb

@REM Copy eboot.msb to target
copy /Y eboot.msb %_TARGETIMGDIR%
copy /Y eboot_ivt.msb %_TARGETIMGDIR%
copy /Y eboot.sb %_TARGETIMGDIR%
copy /Y eboot_ivt.sb %_TARGETIMGDIR%

@REM Copy eboot*.sb to flat release directory
copy /Y eboot.sb %_FLATRELEASEDIR%
copy /Y eboot_ivt.sb %_FLATRELEASEDIR%

@REM Copy eboot*.msb to flat release directory
copy /Y eboot.msb %_FLATRELEASEDIR%
copy /Y eboot_ivt.msb %_FLATRELEASEDIR%

@REM Clean up ...
del xldr.nb0
del eboot.nb0
del eboot.msb
del eboot_ivt.msb

@REM Go back to saved dir
cd %_CURRENTDIR%
