@REM 
@REM Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
@REM THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
@REM AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
@REM 
@echo off & setlocal EnableDelayedExpansion

@REM Skip the processing with debug build
if "%WINCEDEBUG%"=="debug" goto end_of_file

@REM Set path variables
set _TARGETIMGDIR=%_TARGETPLATROOT%\target\%_TGTCPU%\%WINCEDEBUG%

@REM Save current dir and goto tool dir
set _CURRENTDIR=%CD%

set _SUPPORT=SUPPORT_PDK1_9
set/a _FOUND_PDK=0

@REM Check if it's PDK version 
for /l %%i in (0,1,50) do if "!_TGTPLAT:~%%i,3!"=="PDK" set strstart=%%i && set/a _FOUND_PDK=1

if %_FOUND_PDK% EQU 1 (
    for /l %%i in (0,1,50) do if "!_TGTPLAT:~%%i,1!"=="" set strlen=%%i && goto :_end_for     
:_end_for
    set/a strlen=%strlen%-1
    @REM echo strstart=%strstart% strlen=%strlen%
    set _SUPPORT=SUPPORT_
    for /l %%i in (%strstart%,1,%strlen%) do (
        set _SUPPORT=!_SUPPORT!!_TGTPLAT:~%%i,1!
    )
) 

@REM if it is UUT, goto make_nk_ivt_sb
if "%IMGUUT%" == "1" goto make_nk_ivt_sb

@REM make MBR, em9280_mk_mbr.exe is at "%_WINCEROOT%\%_SUPPORT%\TOOL\COMMON\FixNK"
set _TOOLDIR=%_WINCEROOT%\%_SUPPORT%\TOOL\COMMON\FixNK

if not exist %_TOOLDIR%\em9280_mk_mbr.exe goto file_not_exist

cd %_FLATRELEASEDIR%

@REM copy the tool to release directory
copy /Y %_TOOLDIR%\em9280_mk_mbr.exe

@REM if it is case of BinFS, run without parameter
if "%BSP_SUPPORT_DEMAND_PAGING%" == "1" goto case_of_binfs

@REM case of single NK, run with parameter, generate mbr.nb0 in "%_FLATRELEASEDIR%"
em9280_mk_mbr 1
goto end_of_mkmbr

:case_of_binfs
@REM case of BinFS, run without parameter, generate mbr.nb0 in "%_FLATRELEASEDIR%"
em9280_mk_mbr

:end_of_mkmbr

@REM Go back to saved dir
cd %_CURRENTDIR%

@REM if it is case of BinFS, goto make_binfs_files
if "%BSP_SUPPORT_DEMAND_PAGING%" == "1" goto make_binfs_files

@REM it is case of single NK, fix NK.NB0 to actual size
set _TOOLDIR=%_WINCEROOT%\%_SUPPORT%\TOOL\COMMON\FixNK

if not exist %_TOOLDIR%\fixnk.exe goto file_not_exist

cd %_TOOLDIR%

viewbin %_FLATRELEASEDIR%\nk.bin > nkinfo.txt
fixnk %_FLATRELEASEDIR%\nk.nb0 nkinfo.txt 

@REM Clean up ...
del nkinfo.txt

@REM Go back to saved dir
cd %_CURRENTDIR%

if "%BSP_EM9280%" == "1"  set _MFGTOOL=%_WINCEROOT%\%_SUPPORT%\TOOL\MfgTools\Profiles\MX28 WinCE Update\OS firmware\EM9280
if "%BSP_EM9283%" == "1"  set _MFGTOOL=%_WINCEROOT%\%_SUPPORT%\TOOL\MfgTools\Profiles\MX28 WinCE Update\OS firmware\EM9283

cd %_MFGTOOL%

@REM copy eboot_ivt.sb & nk.nb0 to MfgTools workspace
copy /Y %_FLATRELEASEDIR%\eboot_ivt.sb
copy /Y %_FLATRELEASEDIR%\mbr.nb0
copy /Y %_FLATRELEASEDIR%\nk.nb0

@REM Go back to saved dir
cd %_CURRENTDIR%

goto end_of_file


:make_binfs_files
@REM it is case of BinFS, just copy xip.nb0 to nk.nb0
if "%BSP_EM9280%" == "1"  set _MFGTOOL=%_WINCEROOT%\%_SUPPORT%\TOOL\MfgTools\Profiles\MX28 WinCE Update\OS firmware\EM9280
if "%BSP_EM9283%" == "1"  set _MFGTOOL=%_WINCEROOT%\%_SUPPORT%\TOOL\MfgTools\Profiles\MX28 WinCE Update\OS firmware\EM9283

cd %_MFGTOOL%

@REM copy eboot_ivt.sb & xip.nb0 to MfgTools workspace
copy /Y %_FLATRELEASEDIR%\eboot_ivt.sb
copy /Y %_FLATRELEASEDIR%\mbr.nb0
copy /Y %_FLATRELEASEDIR%\xip.nb0 nk.nb0

@REM Go back to saved dir
cd %_CURRENTDIR%

goto end_of_file


:make_nk_ivt_sb
@REM this is case of IMGUUT
set _TOOLDIR=%_WINCEROOT%\%_SUPPORT%\TOOL\iMX28-EVK\SBIMAGE

if not exist %_TOOLDIR%\elftosb.exe goto file_not_exist

cd %_TOOLDIR%

@REM Copy xldr.nb0 and nk.nb0
copy /Y %_TARGETIMGDIR%\xldr.nb0
copy /Y %_FLATRELEASEDIR%\nk.nb0

@REM Generate nk.sb
elftosb -z -c nk.bd -o nk.sb
elftosb -z -f imx28 -c nk_ivt.bd -o nk_ivt.sb

@REM Copy nk.sb to flat release directory
copy /Y nk.sb %_FLATRELEASEDIR%
copy /Y nk_ivt.sb %_FLATRELEASEDIR%

@REM Clean up ...
del xldr.nb0
del nk.nb0

set _MFGTOOL=%_WINCEROOT%\%_SUPPORT%\TOOL\MfgTools\Profiles\MX28 WinCE Update\OS firmware
cd %_MFGTOOL%

copy /Y %_FLATRELEASEDIR%\nk_ivt.sb

@REM Go back to saved dir
cd %_CURRENTDIR%

goto end_of_file


:file_not_exist
echo ERROR : %_TOOLDIR% does not exist, please copy this folder from BSP package.

:end_of_file
