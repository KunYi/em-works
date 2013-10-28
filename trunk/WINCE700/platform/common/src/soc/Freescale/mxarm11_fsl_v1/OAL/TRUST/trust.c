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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  trust.c
//
//  Interface to OAL trust services.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "oal_log.h"
#include "mxarm11.h"

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
//
// Type: KnownFileDescriptor
//
// Tracks the file names and predefined checksums of files that do not require
// a certificate to run as a trusted application.
//
//-----------------------------------------------------------------------------
typedef  struct {
    LPWSTR filename;
} KnownFileDescriptor;

//-----------------------------------------------------------------------------
//
// Type: SignedFileStatus
//
// Tracks the current status of a file that is being loaded.
//
//-----------------------------------------------------------------------------
typedef enum {UNDEFINED, SIGNED_FILE, UNSIGNED_FILE} SignedFileStatus;

//-----------------------------------------------------------------------------
// External Functions

// Loadauth library routines.
extern BOOL CertifyModuleInit(void);
extern BOOL CertifyModule(PBYTE pbBlock, DWORD cbBlock);
extern BOOL CertifyModuleFinal(PBYTE *ppbSignData, PDWORD pcbSignData);

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static SignedFileStatus sfStatus;

// The following files are excluded from signature checking.  The files in this
// list are part of the remote tools available in Platform Builder.
static KnownFileDescriptor knownFiles[] = {
        L"CEMGRC.EXE",
        L"cetlkitl.dll",
        L"\\Windows\\CEFWCLI.EXE",
        L"CETLSTUB.DLL",
        L"\\Windows\\PSPubSubCE.dll",
        L"\\Windows\\rtsce.dll",
        L"\\Windows\\ConPMon.exe",
        L"msitlogr.dll",
        L"rtsce.dll",
        L"PSPubSubCE",
        L"profilepub",
        L"CETLSTUB.DLL",
        L"ProfilePub.dll",
        L"celog.dll",
        L"\\Windows\\CEHWCLI.EXE",
        L"\\Windows\\RTH.EXE",
        L"ktpub.dll",
        L"\\Windows\\KTPub.dll",
        L"\\Windows\\CEPerfMon.dll",
        L"CePerfMon.dll",
        L"\\Windows\\CEPWCLI.EXE",
        L"\\Windows\\CEREGCLI.EXE",
        L"\\Windows\\CESPYCLI.EXE",
        L"\\Windows\\CESysInfo.dll",
        L"\\Windows\\CEZCLI.EXE",
        NULL,
};

static KnownFileDescriptor *pKnownFile;

//-----------------------------------------------------------------------------
// Local Functions
static BOOL CompareStrings(LPWSTR str1, LPWSTR str2);

//-----------------------------------------------------------------------------
//
// Function: OEMCertifyModuleInit
//
// This function is called when a file is loaded from a location other than
// ROM.  This function checks to see if the file name is the same as a
// recognized file or if the file needs to be validated.  If the file is
// recognized, this function will will store a pointer to the recognized file
// entry in the knownFiles array.
//
// Parameters:
//      lpModuleName
//          [in] The name of the module that is about to be loaded.
//
// Returns:
//      TRUE if the file should continue with the validation or FALSE if the
//      load should fail before the validation.
//
//-----------------------------------------------------------------------------
BOOL
OEMCertifyModuleInit(LPWSTR lpModuleName)
{
    OALMSG(OAL_FUNC, (_T("+OEMCertifyModuleInit(%s)\r\n"), lpModuleName));

    // Some files may not need to be signed
    // if you know about them in advance.
    for (pKnownFile = knownFiles; pKnownFile->filename != NULL;  pKnownFile++)
    {
        if (CompareStrings(pKnownFile->filename, lpModuleName))
        {
            OALMSG(OAL_LOG_FUNC,
                (_T("-OEMCertifyModuleInit() Found one we recognize(%s)\r\n"),
                pKnownFile->filename));
           return TRUE;
        }
    }

    // You will need to verify the signature on this file.
    pKnownFile = NULL;
    sfStatus = UNDEFINED;

    OALMSG(OAL_FUNC,
        (_T("-OEMCertifyModuleInit() Did not recognize file name\r\n")));

    return CertifyModuleInit();
}

//-----------------------------------------------------------------------------
//
// Function: OEMCertifyModule
//
// This function is called to validate a file to load. This function will be
// called multiple times for a single file that is being loaded.  When cbData
// is zero, the end of the file has been reached.
//
// Parameters:
//      lpData
//          [in] A chunk of data that makes up the file.
//
//      cbData
//          [in] The size of the chunk of data to process. 0 indicates the
//          end of the file.
//
// Returns:
//      OEM_CERTIFY_TRUST if the file should be run as a trusted file.
//      OEM_CERTIFY_RUN if the file should be run, but not as a trusted file.
//      OEM_CERTIFY_FALSE if the file should not be allowed to run.
//
//-----------------------------------------------------------------------------
DWORD
OEMCertifyModule(LPBYTE lpData, DWORD cbData)
{
    OALMSG(OAL_FUNC, (_T("+OEMCertifyModule(%d)\r\n"), cbData));

    // First see if you need to check the signature on this file or just a checksum.
    if (pKnownFile != NULL)
    {
        OALMSG(OAL_FUNC,
            (_T("-OEMCertifyModule() returns OEM_CERTIFY_TRUST\r\n")));

        return OEM_CERTIFY_TRUST;
    }

    if (sfStatus == UNSIGNED_FILE)
    {
        OALMSG(OAL_FUNC,
            (_T("-OEMCertifyModule() unsigned file so run \r\n")));

        return OEM_CERTIFY_RUN;
    }

    if (cbData)
    {
        if (CertifyModule(lpData, cbData))
        {
            sfStatus = SIGNED_FILE;
            OALMSG(OAL_FUNC, (_T("-OEMCertifyModule() signed file\r\n")));

            return OEM_CERTIFY_TRUST;
        } else {
            sfStatus = UNSIGNED_FILE;
            OALMSG(OAL_FUNC,
                (_T("-OEMCertifyModule() unsigned file so run \r\n")));

            return OEM_CERTIFY_RUN;
        }
    } else {
        // Final call.
        DWORD dwTrustLevel = OEM_CERTIFY_FALSE;
        LPBYTE pSignedData;
        DWORD cbSignedData;

        BOOL fRet = CertifyModuleFinal(&pSignedData,
                                       &cbSignedData);

        if (fRet)
        {
            if (cbSignedData < sizeof(CHAR))
            {
                dwTrustLevel = OEM_CERTIFY_TRUST;
            } else {
                switch (*pSignedData)
                {
                case 'T' :
                    dwTrustLevel = OEM_CERTIFY_TRUST;
                    break;
                case 'R' :
                    dwTrustLevel = OEM_CERTIFY_RUN;
                    break;
                default:
                    dwTrustLevel = OEM_CERTIFY_FALSE;
                    break;
                }
            }
        }

        OALMSG(OAL_FUNC, (_T("-OEMCertifyModule()returns %d\r\n"),
            dwTrustLevel));

        // Return one of the OEM_CERTIFY levels:
        return dwTrustLevel;
    }
}

//-----------------------------------------------------------------------------
//
// Function: CompareStrings
//
// Compares two strings for equality.  This is a case sensitive comparison.
//
// Parameters:
//      str1
//          [in] The first string to compare.
//
//      str2
//          [in] The second string to compare.
//
// Returns:
//      TRUE if the strings are compared or FALSE if the strings are not equal.
//
//-----------------------------------------------------------------------------
static BOOL
CompareStrings(LPWSTR str1, LPWSTR str2)
{
    int index = 0;

    for (;;)
    {
        if (str1[index] != str2[index])
        {
            return FALSE;
        }

        if (str1[index] == 0x0000)
        {
            return TRUE;
        }

        index++;
    }
}

