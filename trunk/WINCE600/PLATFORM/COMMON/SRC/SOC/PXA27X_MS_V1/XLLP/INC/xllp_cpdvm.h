//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

// Declares functions in xllp_cpdvm.s
//
#ifndef __XLLP_CPDVM_H__
#define __XLLP_CPDVM_H__

extern void XllpXSC1EnterTurbo(void);
extern void XllpXSC1ExitTurbo(void);
extern unsigned int XllpXSC1ReadCLKCFG(void );
extern void XllpXSC1WriteCLKCFG(int);
extern void XllpXSC1ChangeVoltage(void);
extern void XllpXSC1FreqChange(int);
extern unsigned long XSC1GetCPUId(void);
extern long XSC1GetCPSR();
extern long XSC1GetSPSR();

#endif
