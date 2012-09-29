@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this source code is subject to the terms of the Microsoft end-user
@REM license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
@REM If you did not accept the terms of the EULA, you are not authorized to use
@REM this source code. For a copy of the EULA, please see the LICENSE.RTF on your
@REM install media.
@REM

@REM 
@REM Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
@REM THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
@REM AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
@REM 
@ECHO OFF

REM
REM Select target hardware revision
REM   0 - Red board with TO1 and mDDR
REM   1 - Blue/Green board with TO1.1 and DDR2
REM
set _TGTHWREV=1

REM Auto define (Don't edit the lines below!)
if "%_TGTHWREV%" == "1" goto none_set
set BSP_BOARD_RED=1
set BSP_CPU_TO1=1
:none_set

REM
REM BSP_NOXXX logics
REM Define "1" to exclude the driver despite the SYSGEN dependency.
REM
set BSP_NOAUDIO=
set BSP_NOBATTERY=
set BSP_NOCSPDDK=
set BSP_NODISPLAY=
set BSP_NOFEC=
set BSP_NOESDHC=
set BSP_NOKEYPAD=
set BSP_NONANDFMD=
set BSP_NOTOUCH=
set BSP_NOUSB=
set BSP_NOSIM=
