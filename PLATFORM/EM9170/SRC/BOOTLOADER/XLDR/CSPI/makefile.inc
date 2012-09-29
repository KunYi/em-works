!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.

Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT.

!endif
!if 0
Use of this source code is subject to the terms of the Microsoft end-user
license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
If you did not accept the terms of the EULA, you are not authorized to use
this source code. For a copy of the EULA, please see the LICENSE.RTF on your
install media.
!endif
!IF 0

Module Name:

    makefile.inc

Abstract:

    This file tells nmake the correct dependency relationships
    for objects created from c files in this directory which are
    wrappers for c files in another directory.
!ENDIF

xldr:
!IF "$(NOLINK)" == "" 
    romimage $(ROMIMAGE_FLAGS) xldr.bib

!IF "$(WINCEREL)"=="1"
    copy $(_PLATFORMROOT)\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)\xldr.nb0 $(_FLATRELEASEDIR)
    copy $(_PLATFORMROOT)\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)\xldr.bin $(_FLATRELEASEDIR)
!ENDIF
!ENDIF
