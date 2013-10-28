REM --------------------------------------------------------------------------
REM Build Environment
REM --------------------------------------------------------------------------

REM Always copy binaries to flat release directory
set WINCEREL=1
REM Generate .cod, .lst files
set WINCECOD=1

REM ----OS SPECIFIC VERSION SETTINGS----------

if "%SG_OUTPUT_ROOT%" == "" (set SG_OUTPUT_ROOT=%_PROJECTROOT%\cesysgen) 

set _PLATCOMMONLIB=%_PLATFORMROOT%\common\lib
set _KITLLIBS=%SG_OUTPUT_ROOT%\oak\lib

set BSP_WCE=1

REM This is to be used in COMMON_TI_V1\COMMON_TI\BOOT\SDHC\sdhc.c
set BSP_AM33X=1

REM TI BSP builds its own ceddk.dll. Setting this IMG var excludes default CEDDK from the OS image.
rem set IMGNODFLTDDK=1

REM --------------------------------------------------------------------------
REM Initial Operating Point - VDD1 voltage, MPU (CPU) speeds
REM --------------------------------------------------------------------------

REM Select initial operating point (CPU speed, VDD1 voltage).
REM Note that this controls the operating point selected by the bootloader.
REM If the power management subsystem is enabled, the initial operating point 
REM it uses is controlled by registry entries.
REM The following are choices for AM33x family
REM Use 4 for MPU[720Mhz @ 1.26V], 	L3/L4[200/100Mhz],	CORE @ 1.1V (SRTurbo)
REM Use 3 for MPU[600Mhz @ 1.2V], 	L3/L4[200/100Mhz],	CORE @ 1.1V (OPM120)
REM Use 2 for MPU[500Mhz @ 1.1V], 	L3/L4[200/100Mhz],	CORE @ 1.1V (OPM100)
REM Use 1 for MPU[275Mhz @ 0.95V],	L3/L4[200/100Mhz],	CORE @ 1.1V (OPM50)
set BSP_OPM_SELECT=4

