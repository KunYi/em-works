@REM 
@REM Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
@REM THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
@REM AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
@REM 
@echo off & setlocal EnableDelayedExpansion

@REM Skip the processing with debug build
if "%WINCEDEBUG%"=="debug" goto end_of_file

@REM Save current dir and goto tool dir
set _CURRENTDIR=%CD%

set _SUPPORT=SUPPORT
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

set _TOOLDIR=%_WINCEROOT%\%_SUPPORT%\TOOL\COMMON\FixNK
@REM echo _TOOLDIR = %_TOOLDIR% 

if not exist %_TOOLDIR%\fixnk.exe goto file_not_exist

cd %_TOOLDIR%

viewbin %_FLATRELEASEDIR%\nk.bin > nkinfo.txt
fixnk %_FLATRELEASEDIR%\nk.nb0 nkinfo.txt 

@REM Clean up ...
del nkinfo.txt

@REM Go to _FLATRELEASEDIR dir
cd %_FLATRELEASEDIR%
mk_mbr

@REM Go back to saved dir
cd %_CURRENTDIR%

goto end_of_file

:file_not_exist
echo ERROR : %_TOOLDIR% does not exist, please copy this folder from BSP package.

:end_of_file



