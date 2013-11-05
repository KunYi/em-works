TI_COMMON=common_ti_AM3xx
TARGETTYPE=LIBRARY
WARNLEVEL=4

CDEFINES=$(CDEFINES) -DCEDDK_USEDDKMACRO -D_WINCEOSVER=$(_WINCEOSVER)
CDEFINES=$(CDEFINES) -DOAL_BSP_CALLBACKS
CDEFINES=$(CDEFINES) -DOAL_ILTIMING

!IF "$(BSP_OMAP_SDMA)" == "1"
CDEFINES=$(CDEFINES) -DBSP_OMAP_SDMA
!ELSEIF "$(BSP_EDMA)" == "1"
CDEFINES=$(CDEFINES) -DBSP_EDMA
!ENDIF

!IF "$(_WINCEOSVER)" >= "700"
CDEFINES=$(CDEFINES) -DASSEMBLER_ARM_ARCH7_SUPPORT
!ENDIF

INCLUDES=$(INCLUDES);$(_PLATFORMROOT)\COMMON\src\soc\COMMON_TI_V1\COMMON_TI_AMXX\inc
INCLUDES=$(INCLUDES);$(_PLATFORMROOT)\COMMON\src\soc\COMMON_TI_V1\COMMON_TI\inc