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
#include "descript.h"
#include "ufnmdd.h"


// Append data to the buffer and advance pointers.
static
DWORD
FillBuffer(
    PBYTE  *ppbDest,
    PDWORD  pcbDest,
    PCVOID  pvSource,
    DWORD   cbSource
    )
{
    SETFNAME();
    
    PREFAST_DEBUGCHK(ppbDest);
    DEBUGCHK(*ppbDest);
    PREFAST_DEBUGCHK(pcbDest);

    DWORD dwRet = ERROR_SUCCESS;

    if (pvSource) {
        DEBUGCHK(cbSource);
    
        if (cbSource > *pcbDest) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Buffer length is too short\r\n"), pszFname));
            dwRet = ERROR_INVALID_PARAMETER;
        }
        else {
            memcpy(*ppbDest, pvSource, cbSource);
            *ppbDest += cbSource;
            *pcbDest -= cbSource;
        }
    }

    return dwRet;
}


// Allocates room and copies an extended descriptor if it exists.
static
DWORD
CopyExtendedDesc(
    PVOID *ppvDest,
    PDWORD pcbDest,
    PVOID  pvSource,
    DWORD  cbSource
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(ppvDest);
    PREFAST_DEBUGCHK(pcbDest);
    
    DWORD dwRet = ERROR_SUCCESS;

    if (pvSource) {
        *ppvDest = LocalAlloc(0, cbSource);
        if (*ppvDest == NULL) {
            dwRet = GetLastError();
            DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
                dwRet));
            goto EXIT;
        }

        memcpy(*ppvDest, pvSource, cbSource);
        *pcbDest = cbSource;
        DEBUGMSG(ZONE_FUNCTION, (_T("%s Copied %d bytes of extended descriptor(s)\r\n"),
            pszFname, cbSource));
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}



CDescriptors::CDescriptors(
    ) : m_devHighSpeed(), m_devFullSpeed()
{
    m_pStringSets = NULL;
    m_cStringSets = 0;
    
    m_pbStringBuffer = NULL; 
}


CDescriptors::~CDescriptors(
    )
{
    if (m_pbStringBuffer) LocalFree(m_pbStringBuffer); 
}


DWORD CDescriptors::Init(
    PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
    PUFN_CONFIGURATION      pHighSpeedConfig,
    PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
    PUFN_CONFIGURATION      pFullSpeedConfig,
    PCUFN_STRING_SET        pStringSets,
    DWORD                   cStringSets
    )
{
    DEBUGCHK(ValidateDescriptor(pHighSpeedDeviceDesc,
            pHighSpeedConfig, pFullSpeedDeviceDesc, pFullSpeedConfig, pStringSets,
            cStringSets) == ERROR_SUCCESS);
    
    m_pStringSets = pStringSets;
    m_cStringSets = cStringSets;

    DWORD dwRet = m_devHighSpeed.Init(BS_HIGH_SPEED, pHighSpeedDeviceDesc, pHighSpeedConfig);
    if (dwRet == ERROR_SUCCESS) {
        dwRet = m_devFullSpeed.Init(BS_FULL_SPEED, pFullSpeedDeviceDesc, pFullSpeedConfig);
    }
    
    return dwRet;
}


DWORD CDescriptors::GenerateTotalConfigDescriptors(
    )
{
    DWORD dwRet = m_devFullSpeed.GenerateTotalConfigDescriptors();
    if (dwRet == ERROR_SUCCESS) {
        dwRet = m_devHighSpeed.GenerateTotalConfigDescriptors();
    }

    return dwRet;
}


DWORD CDescriptors::ValidateDescriptorAndChildren(
    PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
    PUFN_CONFIGURATION      pHighSpeedConfig,
    PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
    PUFN_CONFIGURATION      pFullSpeedConfig,
    PCUFN_STRING_SET        pStringSets,
    DWORD                   cStringSets
    )
{
    DWORD dwRet = ValidateDescriptor(pHighSpeedDeviceDesc,
        pHighSpeedConfig, pFullSpeedDeviceDesc, pFullSpeedConfig, pStringSets,
        cStringSets);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }
    
    dwRet = CDevice::ValidateDescriptorAndChildren(BS_HIGH_SPEED, pHighSpeedDeviceDesc, 
        pHighSpeedConfig);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }
    
    dwRet = CDevice::ValidateDescriptorAndChildren(BS_FULL_SPEED, pFullSpeedDeviceDesc, 
        pFullSpeedConfig);

EXIT:
    return dwRet;
}


DWORD CDescriptors::ValidateDescriptor(
    PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
    PUFN_CONFIGURATION      pHighSpeedConfig,
    PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
    PUFN_CONFIGURATION      pFullSpeedConfig,
    PCUFN_STRING_SET        pStringSets,
    DWORD                   cStringSets
    )
{
    SETFNAME();
    
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    
    if ( (pHighSpeedDeviceDesc == NULL) || (pFullSpeedDeviceDesc == NULL) || 
         (pHighSpeedConfig == NULL) || (pFullSpeedConfig == NULL) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid parameter\r\n"), pszFname));
        goto EXIT;
    }

    if ( (pStringSets && cStringSets == 0) || (pStringSets == NULL && cStringSets) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid String Sets\r\n"), pszFname));
        goto EXIT;
    }

    for (DWORD dwStringSet = 0; dwStringSet < cStringSets; ++dwStringSet) {
        PCUFN_STRING_SET pStringSet = &pStringSets[dwStringSet];
        if (pStringSet->ppszStrings == NULL || pStringSet->cStrings == 0) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Invalid String Set for LangID 0x%04x\r\n"), 
                pszFname, pStringSet->wLangId));
            goto EXIT;
        }
    }

    dwRet = ERROR_SUCCESS;

EXIT:
    return dwRet;
}


// Copy the user-supplied UFN structures to the pContext.
DWORD CDescriptors::Clone(
    CDescriptors **ppdescDst
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(ppdescDst);
    DEBUGCHK(*ppdescDst == NULL);

    DWORD dwRet;

    CDescriptors *pdescDst = new CDescriptors;
    if (pdescDst == NULL) {
        dwRet = ERROR_OUTOFMEMORY;
        goto EXIT;
    }

    dwRet = m_devFullSpeed.Clone(&pdescDst->m_devFullSpeed);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    dwRet = m_devHighSpeed.Clone(&pdescDst->m_devHighSpeed);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    // Copy string descriptors
    if (m_cStringSets != 0) {
        dwRet = CloneStrings(m_pStringSets, m_cStringSets, 
            (PUFN_STRING_SET*) &pdescDst->m_pbStringBuffer);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }

        pdescDst->m_cStringSets = m_cStringSets;
        pdescDst->m_pStringSets = (PUFN_STRING_SET) pdescDst->m_pbStringBuffer;
    }

    DEBUGCHK(ValidateDescriptorAndChildren(
        &pdescDst->m_devHighSpeed.m_DeviceDescCopy,
        pdescDst->m_devHighSpeed.m_pUfnConfigCopy, 
        &pdescDst->m_devFullSpeed.m_DeviceDescCopy,
        pdescDst->m_devFullSpeed.m_pUfnConfigCopy, 
        pdescDst->m_pStringSets, pdescDst->m_cStringSets) == ERROR_SUCCESS);

    dwRet = pdescDst->Init(
        &pdescDst->m_devHighSpeed.m_DeviceDescCopy,
        pdescDst->m_devHighSpeed.m_pUfnConfigCopy, 
        &pdescDst->m_devFullSpeed.m_DeviceDescCopy,
        pdescDst->m_devFullSpeed.m_pUfnConfigCopy, 
        pdescDst->m_pStringSets, pdescDst->m_cStringSets);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    dwRet = pdescDst->GenerateTotalConfigDescriptors();
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    *ppdescDst = pdescDst;

EXIT:
    if (dwRet != ERROR_SUCCESS) {
        if (pdescDst) {
            delete pdescDst;
        }
    }
    
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD CDescriptors::RegisterDevice(
    PUFN_MDD_CONTEXT pContext
    )
{
    SETFNAME();

    DWORD dwRet;
    
    DEBUGCHK(ValidateDescriptorAndChildren(
        m_devHighSpeed.m_pDeviceDesc,
        m_devHighSpeed.m_pUfnConfig, 
        m_devFullSpeed.m_pDeviceDesc,
        m_devFullSpeed.m_pUfnConfig, 
        m_pStringSets, m_cStringSets) == ERROR_SUCCESS);

    struct CONFIG_DESC_BUFFER {
        CDevice *pDevice;
        PUSB_CONFIGURATION_DESCRIPTOR pConfigDescBuffer;
    } rgConfigDescBuffers[2] = {
        { &m_devHighSpeed },
        { &m_devFullSpeed }
    };

    // Create the set of configuration descriptors
    for (DWORD dwCDB = 0; dwCDB < dim(rgConfigDescBuffers); ++dwCDB) {
        CDevice *pDevice = rgConfigDescBuffers[dwCDB].pDevice;
        const DWORD c_dwConfigurations = pDevice->GetConfigurationCount();
        
        DWORD cbTotal = 0;
        for (DWORD dwConfig = 0; dwConfig < c_dwConfigurations; ++dwConfig) {
            PCUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
                pDevice->GetTotalConfigurationDescriptorByIndex(dwConfig);
            PREFAST_DEBUGCHK(pConfigDesc);
            cbTotal += pConfigDesc->wTotalLength;
        }

        PBYTE pbConfigDescBuffer = (PBYTE) LocalAlloc(0, cbTotal);
        rgConfigDescBuffers[dwCDB].pConfigDescBuffer = 
            (PUSB_CONFIGURATION_DESCRIPTOR) pbConfigDescBuffer;
        if (pbConfigDescBuffer == NULL) {
            dwRet = GetLastError();
            DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
                dwRet));
            goto EXIT;
        }
        
        for (DWORD dwConfig = 0; dwConfig < c_dwConfigurations; ++dwConfig) {
            PCUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
                pDevice->GetTotalConfigurationDescriptorByIndex(dwConfig);
            PREFAST_DEBUGCHK(pConfigDesc);
            DWORD dwErr = FillBuffer(&pbConfigDescBuffer, &cbTotal, pConfigDesc,
                pConfigDesc->wTotalLength);
            DEBUGCHK(dwErr == ERROR_SUCCESS);
        }

        DEBUGCHK(cbTotal == 0);
    }

    dwRet = pContext->PddInfo.pfnRegisterDevice(
        pContext->PddInfo.pvPddContext, 
        m_devHighSpeed.m_pDeviceDesc,
        m_devHighSpeed.m_pUfnConfig, 
        rgConfigDescBuffers[0].pConfigDescBuffer,
        m_devFullSpeed.m_pDeviceDesc,
        m_devFullSpeed.m_pUfnConfig, 
        rgConfigDescBuffers[1].pConfigDescBuffer,
        m_pStringSets, m_cStringSets);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Could not RegisterDevice with PDD\r\n"), pszFname));
    }

EXIT:
    for (DWORD dwCDB = 0; dwCDB < dim(rgConfigDescBuffers); ++dwCDB) {
        if (rgConfigDescBuffers[dwCDB].pConfigDescBuffer) {
            LocalFree(rgConfigDescBuffers[dwCDB].pConfigDescBuffer);
        }
    }

    return dwRet;
}


PCUSB_DEVICE_DESCRIPTOR CDescriptors::GetDeviceDescriptor(
    UFN_BUS_SPEED Speed
    )
{
    CDevice *pDevice = GetDevice(Speed);
    DEBUGCHK(pDevice->m_pDeviceDesc);
    return pDevice->m_pDeviceDesc;
}


// Copy string descriptors.
DWORD CDescriptors::CloneStrings(
    PCUFN_STRING_SET    pStringSetsSrc,
    DWORD               cStringSetsSrc,
    PUFN_STRING_SET    *ppStringSetsDst
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(pStringSetsSrc);
    PREFAST_DEBUGCHK(ppStringSetsDst);

    DWORD dwRet = ERROR_INVALID_PARAMETER;
    DWORD dwStringSet;
    DWORD dwString;
    DWORD cbStringSets;
    DWORD cbStringArray;
    DWORD cbStringArrays;
    DWORD cbBuffer;
    PBYTE pbBuffer = NULL;

    *ppStringSetsDst = NULL;

    // Get the count of bytes in the strings
    DWORD cbStrings = 0;
    DWORD cStrings = pStringSetsSrc->cStrings;
    for (dwStringSet = 0; dwStringSet < cStringSetsSrc; ++dwStringSet) {
        PCUFN_STRING_SET pStringSet = &pStringSetsSrc[dwStringSet];

        if (pStringSet->cStrings != cStrings) {
            DEBUGMSG(ZONE_ERROR, (_T("%s String count across locales do not match\r\n"), 
                pszFname));
            goto EXIT;
        }

        for (dwString = 0; dwString < pStringSet->cStrings; ++dwString) {
            LPCWSTR pszString = pStringSet->ppszStrings[dwString];
            DWORD cchString = wcslen(pszString);
            
            if (cchString >= MAX_STRING_DESC_WCHARS) {
                DEBUGMSG(ZONE_ERROR, (_T("%s String %s is too long\r\n"), pszFname,
                    pszString));
                goto EXIT;
            }
            
            cbStrings += (cchString + 1) * sizeof(WCHAR);
        }
    }

    cbStringSets = cStringSetsSrc * sizeof(UFN_STRING_SET);
    cbStringArray = pStringSetsSrc->cStrings * sizeof(*pStringSetsSrc->ppszStrings);
    cbStringArrays = cbStringArray * cStringSetsSrc;
    cbBuffer = cbStringSets + cbStringArrays + cbStrings;
    
    pbBuffer = (PBYTE) LocalAlloc(0, cbBuffer);
    if (pbBuffer == NULL) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
            dwRet));
        goto EXIT;
    }

    PUFN_STRING_SET pStringSetsDst = (PUFN_STRING_SET) pbBuffer;

    // Copy string sets
    memcpy(pStringSetsDst, pStringSetsSrc, cbStringSets);

    // Assign string pointer arrays
    pbBuffer += cbStringSets;
    for (dwStringSet = 0; dwStringSet < cStringSetsSrc; ++dwStringSet) {
        PUFN_STRING_SET pStringSetDst = &pStringSetsDst[dwStringSet];
        pStringSetDst->ppszStrings = (LPCWSTR *) pbBuffer;
        pbBuffer += cbStringArray;
    }

    // Copy each string
    for (dwStringSet = 0; dwStringSet < cStringSetsSrc; ++dwStringSet) {
        PCUFN_STRING_SET pStringSetSrc = &pStringSetsSrc[dwStringSet];
        PUFN_STRING_SET pStringSetDst = &pStringSetsDst[dwStringSet];

        for (dwString = 0; dwString < pStringSetSrc->cStrings; ++dwString) {
            LPCWSTR pszSrc = pStringSetSrc->ppszStrings[dwString];
            LPWSTR pszDst = (LPWSTR) pbBuffer;

#ifdef FSL_MERGE
            DWORD cchString = wcslen(pszSrc);
            StringCchCopyNW(pszDst, cchString + 1, pszSrc, cchString);
#else
            DWORD cchString = wcslen(pszSrc);
            wcsncpy(pszDst, pszSrc, cchString);
#endif
            pszDst[cchString] = 0;

            pStringSetDst->ppszStrings[dwString] = pszDst;
            pbBuffer += (cchString + 1) * sizeof(WCHAR);
        }
    }

    *ppStringSetsDst = pStringSetsDst;

    dwRet = ERROR_SUCCESS;
    
EXIT:
    if (dwRet != ERROR_SUCCESS) {
        if (pbBuffer) {
            LocalFree(pbBuffer);
        }

        DEBUGCHK(*ppStringSetsDst == NULL);
    }
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


DWORD CDescriptors::IsSupportable(
    PUFN_MDD_CONTEXT pContext
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(pContext);

    DWORD dwRet = m_devFullSpeed.IsSupportable(pContext, BS_FULL_SPEED);

    if ( (dwRet == ERROR_SUCCESS) && PddSupportsHighSpeed(pContext) ) {
        dwRet = m_devHighSpeed.IsSupportable(pContext, BS_HIGH_SPEED);
    }

    if (dwRet == ERROR_SUCCESS) {
        // At this point, the device descriptors have been validated and 
        // adjusted with for endpoint 0.
#pragma warning(push)
#pragma warning(disable: 4245)
        pContext->rgpPipes[0]->Reserve(BS_FULL_SPEED, TRUE, -1, -1, -1, NULL);
        pContext->rgpPipes[0]->Reserve(BS_HIGH_SPEED, TRUE, -1, -1, -1, NULL);
#pragma warning(pop)
    }

    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


#define GET_DESCRIPTOR_TYPE(x)          HIBYTE(x)
#define GET_DESCRIPTOR_INDEX(x)         LOBYTE(x)



CONTROL_RESPONSE CDescriptors::ProcessGetDescriptor(
    UFN_BUS_SPEED Speed,
    USB_DEVICE_REQUEST udr,
    DWORD dwMsg,
    BOOL fPddSupportsHighSpeed,
    PVOID *ppvBuffer,
    PDWORD pcbBuffer
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DEBUGCHK(IS_VALID_SPEED(Speed));
    PREFAST_DEBUGCHK(ppvBuffer);
    PREFAST_DEBUGCHK(pcbBuffer);

    CONTROL_RESPONSE response = CR_SUCCESS_TX;

    if (dwMsg == UFN_MSG_PREPROCESSED_SETUP_PACKET) {
        // Nothing to do.
        goto EXIT;
    }

    DWORD dwType = GET_DESCRIPTOR_TYPE(udr.wValue);
    DWORD dwIndex = GET_DESCRIPTOR_INDEX(udr.wValue);

    PVOID pvBuffer;
    DWORD cbBuffer;

    if ( (dwType != USB_STRING_DESCRIPTOR_TYPE) && (udr.wIndex != 0) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid wIndex value %u\r\n"), 
            pszFname, udr.wIndex));
        response = CR_STALL_DEFAULT_PIPE;
    }
    else if ( (dwType == USB_DEVICE_DESCRIPTOR_TYPE) && (dwIndex == 0) ) {
        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Get device descriptor request.\r\n"),
            pszFname));
        pvBuffer = (PVOID) GetDeviceDescriptor(Speed);
        cbBuffer = sizeof(USB_DEVICE_DESCRIPTOR);
    }
    else if ( (dwType == USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE) && (dwIndex == 0) ) {
        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Get device qualifier descriptor request.\r\n"),
            pszFname));
        PCUSB_DEVICE_DESCRIPTOR pDeviceDesc = NULL;

        if (Speed == BS_HIGH_SPEED) {
            pDeviceDesc = GetDeviceDescriptor(BS_FULL_SPEED);
        }
        else {            
            if (fPddSupportsHighSpeed) {
                pDeviceDesc = GetDeviceDescriptor(BS_FULL_SPEED);
            }
        }

        if (pDeviceDesc) {
            cbBuffer = sizeof(USB_DEVICE_QUALIFIER_DESCRIPTOR);
            memcpy(&m_DeviceQualifierDescToSend, pDeviceDesc, cbBuffer);

            // Copy over the values that do match
            m_DeviceQualifierDescToSend.bLength = 
                sizeof(USB_DEVICE_QUALIFIER_DESCRIPTOR);
            m_DeviceQualifierDescToSend.bDescriptorType = (BYTE) dwType;
            m_DeviceQualifierDescToSend.bNumConfigurations = 
                pDeviceDesc->bNumConfigurations;
            m_DeviceQualifierDescToSend.bReserved = 0;

            pvBuffer = &m_DeviceQualifierDescToSend;
        }
        else {
            // We are connected at full speed and there is no support 
            // for high speed.
            response = CR_STALL_DEFAULT_PIPE;
        }
    }
    else if ( (dwType == USB_CONFIGURATION_DESCRIPTOR_TYPE) || 
              (dwType == USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE) ) {
        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Get%s configuration descriptor (%u) request.\r\n"),
            pszFname,  
            (dwType == USB_CONFIGURATION_DESCRIPTOR_TYPE ? _T("") : _T(" other speed")),
            dwIndex));

        UFN_BUS_SPEED SpeedToUse;
        if (dwType == USB_CONFIGURATION_DESCRIPTOR_TYPE) {
            SpeedToUse = Speed;
        }
        else {
            DEBUGCHK(dwType == USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE);
            DEBUGCHK(fPddSupportsHighSpeed);
            if (Speed == BS_HIGH_SPEED) {
                SpeedToUse = BS_FULL_SPEED;
            }
            else {
                SpeedToUse = BS_HIGH_SPEED;
            }
        }
        
        PUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
            GetTotalConfigurationDescriptorByIndex(SpeedToUse, dwIndex);
        if (pConfigDesc == NULL) {
            DEBUGMSG(ZONE_ERROR, (_T("%s Invalid configuration index %u\r\n"), 
                pszFname, dwIndex));
            response = CR_STALL_DEFAULT_PIPE;
        }
        else
        {
            pConfigDesc->bDescriptorType = (BYTE) dwType;
            pvBuffer = pConfigDesc;
            cbBuffer = pConfigDesc->wTotalLength;
        }
    }
    else if (dwType == USB_STRING_DESCRIPTOR_TYPE) {
        WORD wLang = udr.wIndex;
        DEBUGMSG(ZONE_USB_EVENTS, (_T("%s Get string descriptor request. Lang 0x%04x Idx %u\r\n"),
            pszFname, wLang, dwIndex));

        if (m_cStringSets) {
            PUSB_STRING_DESCRIPTOR pStringDesc = (PUSB_STRING_DESCRIPTOR) 
                m_rgbStringDesc;
            pStringDesc->bDescriptorType = USB_STRING_DESCRIPTOR_TYPE;
            DWORD cbData;

            if (dwIndex == 0) {
                DEBUGCHK(m_cStringSets <= MAX_STRING_DESC_WCHARS);

                DWORD dwLangId;
                for (dwLangId = 0; dwLangId < m_cStringSets; ++dwLangId) {
                    pStringDesc->bString[dwLangId] = 
                        m_pStringSets[dwLangId].wLangId;
                }

                cbData = m_cStringSets * sizeof(WCHAR);
            }
            else {
                DWORD dwLangId;
                PCUFN_STRING_SET pStringSet;
                for (dwLangId = 0; dwLangId < m_cStringSets; ++dwLangId) {
                    pStringSet = &m_pStringSets[dwLangId];
                    if (pStringSet->wLangId == wLang) {
                        break;
                    }
                }

                if (dwLangId == m_cStringSets) {
                    // Perhaps the client wishes to process string requests.
                    DEBUGMSG(ZONE_WARNING, (_T("%s Invalid language id 0x%04x\r\n"), 
                        pszFname, wLang));
                    response = CR_UNHANDLED_REQUEST;
                }
                else {
                    --dwIndex;
                    
                    if (dwIndex < pStringSet->cStrings) {
                        LPCWSTR pszString = pStringSet->ppszStrings[dwIndex];
                        DWORD cchString = wcslen(pszString);
                        DEBUGCHK(cchString <= MAX_STRING_DESC_WCHARS);
                        // Do not copy NULL terminator.
#ifdef FSL_MERGE
                        StringCchCopyNW(pStringDesc->bString, cchString + 1, pszString, cchString);
#else
                        wcsncpy(pStringDesc->bString, pszString, cchString);
#endif
                        cbData = cchString * sizeof(WCHAR);
                    }
                    else {
                        // Perhaps the client wishes to process string requests.
                        DEBUGMSG(ZONE_WARNING, (_T("%s Invalid string index %u\r\n"), 
                            pszFname, dwIndex));
                        response = CR_UNHANDLED_REQUEST;
                    }
                }
            }

            if (response == CR_SUCCESS_TX) {
                pStringDesc->bLength = (UCHAR) (sizeof(USB_STRING_DESCRIPTOR) - 
                    sizeof(pStringDesc->bString) + cbData);
                pvBuffer = pStringDesc;
                cbBuffer = pStringDesc->bLength;
            }
        }
        else {
            // Perhaps the client wishes to process string requests.
            DEBUGMSG(ZONE_WARNING, (_T("%s Request for strings, but MDD has none. Index %u\r\n"), 
                pszFname, dwIndex));
            response = CR_UNHANDLED_REQUEST;
        }
    }
    else {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid GetDescriptor request\r\n"),
            pszFname));
        response = CR_STALL_DEFAULT_PIPE;
    }

    if (response == CR_SUCCESS_TX) {
        DEBUGCHK(ppvBuffer);
        *ppvBuffer = pvBuffer;
        *pcbBuffer = cbBuffer;
    }

EXIT:
    FUNCTION_LEAVE_MSG();

    return response;
}






CDevice::CDevice(
    ) 
{
    m_pDeviceDesc = NULL;
    m_pUfnConfig = NULL;
    m_pConfigurations = NULL;
    m_dwConfigurationValue = 0;
    memset(&m_DeviceDescCopy, 0, sizeof(USB_DEVICE_DESCRIPTOR));
    m_pUfnConfigCopy = NULL;
}

CDevice::~CDevice(
    ) 
{ 
    if (m_pConfigurations) delete [] m_pConfigurations; 
    if (m_pUfnConfigCopy) {
        for (DWORD dwConfig = 0; dwConfig < GetConfigurationCount(); ++dwConfig) {
            PUFN_CONFIGURATION pConfig = &m_pUfnConfigCopy[dwConfig];
            CConfiguration::FreeUfnConfigTree(pConfig);
        }
        LocalFree(m_pUfnConfigCopy);
    }
}


DWORD CDevice::Init(
    UFN_BUS_SPEED           Speed,
    PUSB_DEVICE_DESCRIPTOR  pDeviceDesc,
    PUFN_CONFIGURATION      pUfnConfig
    )
{
    SETFNAME();
    
    DEBUGCHK(ValidateDescriptor(Speed, pDeviceDesc, pUfnConfig) == ERROR_SUCCESS);
        
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    
    m_pDeviceDesc = pDeviceDesc;
    m_pUfnConfig = pUfnConfig;

    const DWORD c_cConfigs = GetConfigurationCount();

    m_pConfigurations = new CConfiguration[c_cConfigs];
    if (m_pConfigurations == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s new failed.\r\n"), pszFname));
        dwRet = ERROR_OUTOFMEMORY;
        goto EXIT;
    }

    for (DWORD dwConf = 0; dwConf < c_cConfigs; ++dwConf) {
        CConfiguration *pConfigurationCurr = &m_pConfigurations[dwConf];
        PUFN_CONFIGURATION pUfnConfigCurr = &pUfnConfig[dwConf];
        dwRet = pConfigurationCurr->Init(Speed, pUfnConfigCurr);

        if (dwRet != ERROR_SUCCESS) {
            break;
        }
    }

EXIT:
    return dwRet;
}


DWORD CDevice::Clone(
    CDevice *pdevDst
    )
{
    SETFNAME();
    
    PREFAST_DEBUGCHK(pdevDst);

    DWORD dwRet;

    // Copy the device descriptor
    memcpy(&pdevDst->m_DeviceDescCopy, m_pDeviceDesc, sizeof(USB_DEVICE_DESCRIPTOR));
    pdevDst->m_pDeviceDesc = &pdevDst->m_DeviceDescCopy;

    DWORD cConfigs = GetConfigurationCount();
    DWORD cbConfigs = cConfigs * sizeof(UFN_CONFIGURATION);
    pdevDst->m_pUfnConfigCopy = (PUFN_CONFIGURATION) LocalAlloc(0, cbConfigs);
    if (pdevDst->m_pUfnConfigCopy == NULL) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
            dwRet));
        goto EXIT;
    }

    // Copy the config tree structure for each configuration
    for (DWORD dwConfig = 0; dwConfig < cConfigs; ++dwConfig) {
        PUFN_CONFIGURATION pUfnConfigDst = &pdevDst->m_pUfnConfigCopy[dwConfig];
        PUFN_CONFIGURATION pUfnConfigSrc = &m_pUfnConfig[dwConfig];
        dwRet = CConfiguration::CopyUfnConfigTree(pUfnConfigDst, pUfnConfigSrc);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }

EXIT:
    return dwRet;
}


DWORD CDevice::GenerateTotalConfigDescriptors(
    )
{
    DWORD dwRet = ERROR_INVALID_STATE;

    const DWORD c_cConfigs = GetConfigurationCount();

    for (DWORD dwConf = 0; dwConf < c_cConfigs; ++dwConf) {
        CConfiguration *pConfiguration = &m_pConfigurations[dwConf];
        dwRet = pConfiguration->GenerateTotalConfigDescriptor();
        if (dwRet != ERROR_SUCCESS) {
            break;
        }
    }

    return dwRet;
}

DWORD CDevice::IsSupportable(
    PUFN_MDD_CONTEXT pContext,
    UFN_BUS_SPEED Speed
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    const DWORD c_cConfigs = GetConfigurationCount();
    DEBUGCHK(c_cConfigs > 0); // Should already be checked in ValidateDescriptor

    DWORD dwRet = ERROR_GEN_FAILURE;

    if ( (c_cConfigs > 1) && !PddSupportsMultipleConfigurations(pContext) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s This PDD does not support multiple configurations\r\n"),
            pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

    for (DWORD dwConf = 0; dwConf < c_cConfigs; ++dwConf) {
        CConfiguration *pConfiguration = &m_pConfigurations[dwConf];
        // Check EP0 in the first configuration
        PUSB_DEVICE_DESCRIPTOR pDeviceDesc = (dwConf == 0) ? m_pDeviceDesc : NULL;
        dwRet = pConfiguration->IsSupportable(pContext, Speed, pDeviceDesc);

        if (dwRet != ERROR_SUCCESS) {
            break;
        }
    }

EXIT:
    FUNCTION_LEAVE_MSG();
    return dwRet;
}


DWORD CDevice::ValidateDescriptorAndChildren(
    UFN_BUS_SPEED           Speed,
    PUSB_DEVICE_DESCRIPTOR  pDeviceDesc,
    PUFN_CONFIGURATION      pUfnConfig
    )
{
    SETFNAME();
    
    BOOL rgfConfigValueUsed[256] = { 0 };
    DWORD dwRet = ValidateDescriptor(Speed, pDeviceDesc, pUfnConfig);

    if (dwRet == ERROR_SUCCESS) {
        const DWORD c_cConfigs = pDeviceDesc->bNumConfigurations;
        
        for (DWORD dwConf = 0; dwConf < c_cConfigs; ++dwConf) {
            PUFN_CONFIGURATION pUfnConfigCurr = &pUfnConfig[dwConf];

            // Verify that this configuration's unique value has not already
            // been used.
            BYTE bConfigurationValue = pUfnConfigCurr->Descriptor.bConfigurationValue;
            if (rgfConfigValueUsed[bConfigurationValue] == TRUE) {
                DEBUGMSG(ZONE_ERROR, (_T("%s bConfigurationValue 0x%02x is used more than once\r\n"),
                    pszFname, bConfigurationValue));
                dwRet = ERROR_INVALID_PARAMETER;
                break;
            }

            rgfConfigValueUsed[bConfigurationValue] = TRUE;
            
            dwRet = CConfiguration::ValidateDescriptorAndChildren(Speed, pUfnConfigCurr);
            if (dwRet != ERROR_SUCCESS) {
                break;
            }
        }
    }

    return dwRet;
}


DWORD CDevice::ValidateDescriptor(
    UFN_BUS_SPEED           Speed,
    PUSB_DEVICE_DESCRIPTOR  pDeviceDesc,
    PUFN_CONFIGURATION      pUfnConfig
    )
{
    SETFNAME();
    
    DWORD dwRet = ERROR_INVALID_PARAMETER;

    const DWORD c_cConfigs = pDeviceDesc->bNumConfigurations;
    
    if (c_cConfigs == 0) {
        DEBUGMSG(ZONE_ERROR, (_T("%s No configurations!\r\n"),
            pszFname));
    }
    else {
        dwRet = ERROR_SUCCESS;
    }

    return dwRet;
}


CConfiguration::CConfiguration(
    )
{
    m_pUfnConfig = NULL;
    m_pInterfaces = NULL;
    m_pConfigDesc = NULL;
}


CConfiguration::~CConfiguration(
    )
{ 
    if (m_pInterfaces) delete [] m_pInterfaces; 
    if (m_pConfigDesc) LocalFree(m_pConfigDesc);
}


DWORD CConfiguration::Init(
    UFN_BUS_SPEED           Speed,
    PUFN_CONFIGURATION      pUfnConfig
    )
{
    SETFNAME();
    
    DEBUGCHK(ValidateDescriptor(Speed, pUfnConfig) == ERROR_SUCCESS);
    
    DWORD dwRet = ERROR_INVALID_PARAMETER;

    m_pUfnConfig = pUfnConfig;

    const DWORD c_cInterfaces = GetInterfaceCount();

    // Allocate the CInterfaces
    m_pInterfaces = new CInterface[c_cInterfaces];
    if (m_pInterfaces == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s new failed.\r\n"), pszFname));
        dwRet = ERROR_OUTOFMEMORY;
        goto EXIT;
    }

    // Now initialize each CInterface
    for (DWORD dwInterface = 0; dwInterface < c_cInterfaces; ++dwInterface) {
        CInterface *pInterface = &m_pInterfaces[dwInterface];
        PUFN_INTERFACE pUfnInterface = &m_pUfnConfig->pInterfaces[dwInterface];
        dwRet = pInterface->Init(Speed, pUfnInterface);

        if (dwRet != ERROR_SUCCESS) {
            break;
        }
    }

EXIT:
    return dwRet;    
}


DWORD CConfiguration::IsSupportable(
    PUFN_MDD_CONTEXT        pContext,
    UFN_BUS_SPEED           Speed,
    PUSB_DEVICE_DESCRIPTOR  pDeviceDesc
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    // Make sure that the PDD can handle the configuration as a whole before giving it
    // each endpoint at a time.
    DWORD dwRet = pContext->PddInfo.pfnIsConfigurationSupportable(
        pContext->PddInfo.pvPddContext, Speed, m_pUfnConfig);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s PDD cannot support the configuration\r\n"),
            pszFname));
        goto EXIT;
    }

    if (pDeviceDesc) {
        // Ensure that the specified endpoint 0 can be supported
        dwRet = CMddEndpoint::IsEndpointZeroSupportable(pContext, Speed, pDeviceDesc);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }

    // Verify that the endpoints in every interface can be supported.
    DWORD cEndpoints = 0;
    DWORD cMaxEndpointsPerInterface = 0;
    for (DWORD dwInterface = 0; dwInterface < GetInterfaceCount(); ++dwInterface) {
        CInterface *pInterface = &m_pInterfaces[dwInterface];
        PUFN_INTERFACE pUfnInterface = pInterface->m_pUfnInterface;

        if( pUfnInterface->Descriptor.bAlternateSetting == 0 )
        {
            cEndpoints += cMaxEndpointsPerInterface;
            if (cEndpoints >= pContext->PddInfo.dwEndpointCount) {
                DEBUGMSG(ZONE_ERROR, (_T("%s Not enough endpoints for client\r\n"),
                    pszFname));
                dwRet = ERROR_INVALID_PARAMETER;
                goto EXIT;
            }
            cMaxEndpointsPerInterface = 0;
        }

        if( cMaxEndpointsPerInterface < pUfnInterface->Descriptor.bNumEndpoints ) {
            cMaxEndpointsPerInterface = pUfnInterface->Descriptor.bNumEndpoints;
        }

        if( dwInterface == ( GetInterfaceCount() - 1 ) )
        {
            cEndpoints += cMaxEndpointsPerInterface;
            if (cEndpoints >= pContext->PddInfo.dwEndpointCount) {
                DEBUGMSG(ZONE_ERROR, (_T("%s Not enough endpoints for client\r\n"),
                    pszFname));
                dwRet = ERROR_INVALID_PARAMETER;
                goto EXIT;
            }
            cMaxEndpointsPerInterface = 0;
        }

        dwRet = pInterface->IsSupportable(pContext, Speed, m_pUfnConfig->Descriptor.bConfigurationValue);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }

EXIT:
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}



DWORD CConfiguration::ValidateDescriptorAndChildren(
    UFN_BUS_SPEED           Speed,
    PUFN_CONFIGURATION      pUfnConfig
    )
{
    DWORD dwRet = ValidateDescriptor(Speed, pUfnConfig);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    const DWORD c_cInts = pUfnConfig->Descriptor.bNumInterfaces;  

    // Now validate each CInterface
#pragma warning(push)
#pragma warning(disable: 4245)
    DWORD dwPrevInterfaceNumber = -1;
#pragma warning(pop)
    DWORD dwPrevAltSetting = 0;
    for (DWORD dwInterface = 0; dwInterface < c_cInts; ++dwInterface) {
        PUFN_INTERFACE pUfnInterface = &pUfnConfig->pInterfaces[dwInterface];

        // First validate interface numbering
        DWORD dwCurrInterfaceNumber = pUfnInterface->Descriptor.bInterfaceNumber;
        DWORD dwCurrAltSetting = pUfnInterface->Descriptor.bAlternateSetting;

        if (dwCurrInterfaceNumber != dwPrevInterfaceNumber) {
            // New interface or bad numbering?
            if (dwCurrInterfaceNumber == dwPrevInterfaceNumber + 1) {
                // New interface
                if (dwCurrAltSetting != 0) {
                    DEBUGMSG(ZONE_ERROR, 
                        (_T("%s Interface(%u) has invalid alternate setting of %u. It should be 0.\r\n"),
                        dwCurrInterfaceNumber, dwCurrAltSetting));
                    dwRet = ERROR_INVALID_PARAMETER;
                    goto EXIT;
                }

                ++dwPrevInterfaceNumber;
                dwPrevAltSetting = 0;
            }
            else {
                DEBUGMSG(ZONE_ERROR, 
                    (_T("%s Interface number %u is invalid. It should be %u + 1 = %u.\r\n"),
                    dwCurrInterfaceNumber, dwPrevInterfaceNumber, dwPrevInterfaceNumber + 1));
                dwRet = ERROR_INVALID_PARAMETER;
                goto EXIT;
            }
        }
        else {
            // Same interface. Validate alternate setting numbering.
            if (dwCurrAltSetting != dwPrevAltSetting + 1) {
                DEBUGMSG(ZONE_ERROR, 
                    (_T("%s Interface(%u) has invalid alternate setting of %u. It should be %u + 1 = %u.\r\n"),
                    dwCurrInterfaceNumber, dwCurrAltSetting, dwPrevAltSetting, dwPrevAltSetting + 1));
                dwRet = ERROR_INVALID_PARAMETER;
                goto EXIT;
            }

            ++dwPrevAltSetting;
        }
        
        dwRet = CInterface::ValidateDescriptorAndChildren(Speed, pUfnInterface);

        if (dwRet != ERROR_SUCCESS) {
            break;
        }
    }

EXIT:
    return dwRet;
}


DWORD CConfiguration::ValidateDescriptor(
    UFN_BUS_SPEED           Speed,
    PUFN_CONFIGURATION      pUfnConfig
    )
{
    SETFNAME();

    DWORD dwRet = ERROR_INVALID_PARAMETER;
    
    const DWORD c_cInts = pUfnConfig->Descriptor.bNumInterfaces;
    
    if ( (c_cInts == 0) || (pUfnConfig->pInterfaces == NULL) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s No interfaces in config!\r\n"), pszFname));
        goto EXIT;
    }
    
    if ( (pUfnConfig->pvExtended == NULL && pUfnConfig->cbExtended) ||
         (pUfnConfig->pvExtended && pUfnConfig->cbExtended == 0) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid cbExtended/pvExtended pair!\r\n"), pszFname));
        goto EXIT;
    }

    dwRet = ERROR_SUCCESS;

EXIT:
    return dwRet;
}


DWORD CConfiguration::Serialize(
    PBYTE   pbBuffer,
    DWORD   cbBuffer,
    PDWORD  pcbRequired
    )
{
    DWORD dwRet = ERROR_SUCCESS;
    
    if (pbBuffer) {
        dwRet = FillBuffer(&pbBuffer, &cbBuffer, &m_pUfnConfig->Descriptor, 
            sizeof(m_pUfnConfig->Descriptor));
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }
    else {
        DEBUGCHK(cbBuffer == 0);
    }

    if (pbBuffer) {
        dwRet = FillBuffer(&pbBuffer, &cbBuffer, m_pUfnConfig->pvExtended, 
            m_pUfnConfig->cbExtended);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }
    
    *pcbRequired = sizeof(m_pUfnConfig->Descriptor) + m_pUfnConfig->cbExtended;

    const DWORD c_cInterfaces = GetInterfaceCount();

    for (DWORD dwInterface = 0; dwInterface < c_cInterfaces; ++dwInterface) {
        CInterface *pInterface = &m_pInterfaces[dwInterface];
        DWORD cbRequired;
        dwRet = pInterface->Serialize(pbBuffer, cbBuffer, &cbRequired);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }

        pbBuffer += cbRequired;
        cbBuffer -= cbRequired;
        *pcbRequired += cbRequired;
    }

    DEBUGCHK(*pcbRequired == m_pUfnConfig->Descriptor.wTotalLength);
    
EXIT:
    return dwRet;
}


DWORD CConfiguration::GenerateTotalConfigDescriptor(
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();
    
    DEBUGCHK(m_pConfigDesc == NULL);

    DWORD dwRet;

    DWORD cbConfig = m_pUfnConfig->Descriptor.wTotalLength;
    PBYTE pbBuffer = (PBYTE) LocalAlloc(0, cbConfig);
    if (pbBuffer == NULL) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
            dwRet));
        goto EXIT;
    }

    DWORD cbRequired;
    dwRet = Serialize(pbBuffer, cbConfig, &cbRequired);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }
    
    DEBUGCHK(cbRequired == cbConfig);
    m_pConfigDesc = (PUSB_CONFIGURATION_DESCRIPTOR) pbBuffer;
    dwRet = ERROR_SUCCESS;

EXIT:
    if (dwRet != ERROR_SUCCESS) {
        if (pbBuffer) LocalFree(pbBuffer);
    }

    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


// Copies a UFN config tree structure.
DWORD CConfiguration::CopyUfnConfigTree(
    PUFN_CONFIGURATION  pConfigDest,
    PUFN_CONFIGURATION  pConfigSource
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(pConfigDest);
    PREFAST_DEBUGCHK(pConfigSource);

    DWORD dwRet;
    
    memcpy(pConfigDest, pConfigSource, sizeof(UFN_CONFIGURATION));

    // Clear out the configuration's pointers
    pConfigDest->pInterfaces = NULL;
    pConfigDest->pvExtended = NULL;
    
    // Copy the config's extended descriptor    
    dwRet = CopyExtendedDesc(&pConfigDest->pvExtended, &pConfigDest->cbExtended,
        pConfigSource->pvExtended, pConfigSource->cbExtended);
    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    // Copy the interface structure
    DEBUGCHK(pConfigSource->Descriptor.bNumInterfaces);
    PREFAST_DEBUGCHK(pConfigSource->pInterfaces);
    DWORD cbInterfaces = 
        pConfigDest->Descriptor.bNumInterfaces * sizeof(UFN_INTERFACE);
    pConfigDest->pInterfaces = (PUFN_INTERFACE) LocalAlloc(0, cbInterfaces);
    if (pConfigDest->pInterfaces == NULL) {
        dwRet = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
            dwRet));
        goto EXIT;
    }
    
    memcpy(pConfigDest->pInterfaces, pConfigSource->pInterfaces, cbInterfaces);

    DWORD cInterfaces = pConfigDest->Descriptor.bNumInterfaces;
    // Clear out each interface's pointers
    for (DWORD dwInterface = 0; dwInterface < cInterfaces; ++dwInterface) {
        PUFN_INTERFACE pInterfaceDest = &pConfigDest->pInterfaces[dwInterface];
        pInterfaceDest->pEndpoints = NULL;
        pInterfaceDest->pvExtended = NULL;
    }
    
    // Copy each interface's extended descriptor
    for (DWORD dwInterface = 0; dwInterface < cInterfaces; ++dwInterface) {
        PUFN_INTERFACE pInterfaceSource = &pConfigSource->pInterfaces[dwInterface];
        PUFN_INTERFACE pInterfaceDest = &pConfigDest->pInterfaces[dwInterface];

        dwRet = CopyExtendedDesc(&pInterfaceDest->pvExtended, 
            &pInterfaceDest->cbExtended,
            pInterfaceSource->pvExtended,
            pInterfaceSource->cbExtended);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }

        // Copy the endpoint structures
        DWORD cEndpoints = pInterfaceSource->Descriptor.bNumEndpoints;

        if (cEndpoints > 0) {
            DWORD cbEndpoints;
            cbEndpoints = cEndpoints * sizeof(UFN_ENDPOINT);
            pInterfaceDest->pEndpoints = (PUFN_ENDPOINT)
                LocalAlloc(0, cbEndpoints);
            if (pInterfaceDest->pEndpoints == NULL) {
                dwRet = GetLastError();
                DEBUGMSG(ZONE_ERROR, (_T("%s LocalAlloc failed. Error: %d\r\n"), pszFname, 
                    dwRet));
                goto EXIT;
            }

            memcpy(pInterfaceDest->pEndpoints, pInterfaceSource->pEndpoints, 
                cbEndpoints);

            // Clear out each endpoint's pointers
            for (DWORD dwEndpoint = 0; dwEndpoint < cEndpoints; ++dwEndpoint) {
                PUFN_ENDPOINT pDestEndpoint = 
                    &pInterfaceDest->pEndpoints[dwEndpoint];
                pDestEndpoint->pvExtended = NULL;
            }
            
            // Copy each endpoint's extended descriptors
            for (DWORD dwEndpoint = 0; dwEndpoint < cEndpoints; ++dwEndpoint) {
                PUFN_ENDPOINT pSourceEndpoint = 
                    &pInterfaceSource->pEndpoints[dwEndpoint];
                PUFN_ENDPOINT pDestEndpoint = 
                    &pInterfaceDest->pEndpoints[dwEndpoint];
                dwRet = CopyExtendedDesc(&pDestEndpoint->pvExtended, &pDestEndpoint->cbExtended,
                    pSourceEndpoint->pvExtended, pSourceEndpoint->cbExtended);
                if (dwRet != ERROR_SUCCESS) {
                    goto EXIT;
                }
            }
        }
    }

    dwRet = ERROR_SUCCESS;

EXIT:
    if (dwRet != ERROR_SUCCESS) {
        if (pConfigDest) {
            FreeUfnConfigTree(pConfigDest);
        }
    }
    
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


// Free all allocated data in a config tree structure.
VOID CConfiguration::FreeUfnConfigTree(
    PUFN_CONFIGURATION pConfig
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    PREFAST_DEBUGCHK(pConfig);

    if (pConfig->pInterfaces) {
        const DWORD cInterfaces = pConfig->Descriptor.bNumInterfaces;
        for (DWORD dwInterface = 0; dwInterface < cInterfaces; ++dwInterface) {
            PUFN_INTERFACE pInterface = &pConfig->pInterfaces[dwInterface];
            if (pInterface->pEndpoints) {
                DWORD cEndpoints = pInterface->Descriptor.bNumEndpoints;
                for (DWORD dwEndpoint = 0; dwEndpoint < cEndpoints; ++dwEndpoint) {
                    PUFN_ENDPOINT pSourceEndpoint = 
                        &pInterface->pEndpoints[dwEndpoint];

                    // Free this endpoint's extended descriptor
                    if (pSourceEndpoint->pvExtended) {
                        LocalFree(pSourceEndpoint->pvExtended);
                    }
                }

                // Free the endpoint structures
                LocalFree(pInterface->pEndpoints);
            }

            // Free the interface's extended descriptor
            if (pInterface->pvExtended) {
                LocalFree(pInterface->pvExtended);
            }
        }

        // Free the interface structure
        LocalFree(pConfig->pInterfaces);
        pConfig->pInterfaces = NULL;
    }
    
    // Free the config's extended descriptor
    if (pConfig->pvExtended) {
        LocalFree(pConfig->pvExtended);
        pConfig->pvExtended = NULL;
    }

    FUNCTION_LEAVE_MSG();
}



CInterface::CInterface(
    ) 
{
    m_pUfnInterface = NULL;
    m_dwCurrAltSetting = 0;
    m_pEndpoints = NULL;
}


CInterface::~CInterface() 
{ 
    if (m_pEndpoints) delete [] m_pEndpoints; 
}

DWORD CInterface::CountInterfaces(
    PUFN_CONFIGURATION      pUfnConfig
    )
{
    const DWORD c_cInts = pUfnConfig->Descriptor.bNumInterfaces;

    DWORD dwInterfaces = 1;
    PUFN_INTERFACE pIntPrev = &pUfnConfig->pInterfaces[0];
    for (DWORD dwCurrInt = 1; dwCurrInt < c_cInts; ++dwCurrInt) {
        PUFN_INTERFACE pIntCurr = &pUfnConfig->pInterfaces[dwCurrInt];
        if (pIntPrev->Descriptor.bInterfaceNumber != 
                pIntCurr->Descriptor.bInterfaceNumber) {
            // New interface
            ++dwInterfaces;
        }

        pIntPrev = pIntCurr;
    }

    return dwInterfaces;
}


DWORD CInterface::Init(
    UFN_BUS_SPEED           Speed,
    PUFN_INTERFACE          pUfnInterface
    )
{
    SETFNAME();
    
    DEBUGCHK(ValidateDescriptor(Speed, pUfnInterface) == ERROR_SUCCESS);

    m_pUfnInterface = pUfnInterface;

    const DWORD c_cEndpoints = GetEndpointCount();
    
    DWORD dwRet = ERROR_SUCCESS;

    m_pEndpoints = new CMddEndpoint[c_cEndpoints];
    if (m_pEndpoints == NULL) {
        DEBUGMSG(ZONE_ERROR, (_T("%s new failed.\r\n"), pszFname));
        dwRet = ERROR_OUTOFMEMORY;
        goto EXIT;
    }

    for (DWORD dwEndpoint = 0; dwEndpoint < c_cEndpoints; ++dwEndpoint) {
        dwRet = m_pEndpoints[dwEndpoint].Init(Speed, &m_pUfnInterface->pEndpoints[dwEndpoint]);
        if (dwRet != ERROR_SUCCESS) {
            break;
        }
    }

EXIT:
    return dwRet;
}


DWORD CInterface::ValidateDescriptorAndChildren(
    UFN_BUS_SPEED           Speed,
    PUFN_INTERFACE          pUfnInterface
    )
{    
    DWORD dwRet = ValidateDescriptor(Speed, pUfnInterface);

    if (dwRet != ERROR_SUCCESS) {
        goto EXIT;
    }

    const DWORD c_cEndpoints = pUfnInterface->Descriptor.bNumEndpoints;

    for (DWORD dwEndpoint = 0; dwEndpoint < c_cEndpoints; ++dwEndpoint) {
        dwRet = CMddEndpoint::ValidateDescriptor(Speed, &pUfnInterface->pEndpoints[dwEndpoint]);
        if (dwRet != ERROR_SUCCESS) {
            break;
        }
    }

EXIT:
    return dwRet;
}


DWORD CInterface::ValidateDescriptor(
    UFN_BUS_SPEED           Speed,
    PUFN_INTERFACE          pUfnInterface
    )
{
    SETFNAME();

    DWORD dwRet = ERROR_INVALID_PARAMETER;
    
    const DWORD c_cEndpoints = pUfnInterface->Descriptor.bNumEndpoints;
    
    if ( c_cEndpoints && (pUfnInterface->pEndpoints == NULL) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s No endpoint structure provided\r\n"),
            pszFname));
        goto EXIT;
    }

    if ( (pUfnInterface->pvExtended == NULL && pUfnInterface->cbExtended) ||
         (pUfnInterface->pvExtended && pUfnInterface->cbExtended == 0) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid cbExtended/pvExtended pair!\r\n"), pszFname));
        goto EXIT;
    }

    dwRet = ERROR_SUCCESS;

EXIT:
    return dwRet;
}


DWORD CInterface::Serialize(
    PBYTE   pbBuffer,
    DWORD   cbBuffer,
    PDWORD  pcbRequired
    )
{
    DWORD dwRet = ERROR_SUCCESS;
    
    if (pbBuffer) {
        dwRet = FillBuffer(&pbBuffer, &cbBuffer, &m_pUfnInterface->Descriptor, 
            sizeof(m_pUfnInterface->Descriptor));
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }
    else {
        DEBUGCHK(cbBuffer == 0);
    }

    if (pbBuffer) {
        dwRet = FillBuffer(&pbBuffer, &cbBuffer, m_pUfnInterface->pvExtended, 
            m_pUfnInterface->cbExtended);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }
    
    *pcbRequired = sizeof(m_pUfnInterface->Descriptor) + m_pUfnInterface->cbExtended;

    const DWORD c_cEndpoints = GetEndpointCount();

    for (DWORD dwEndpoint = 0; dwEndpoint < c_cEndpoints; ++dwEndpoint) {
        CMddEndpoint *pEndpoint = &m_pEndpoints[dwEndpoint];
        DWORD cbRequired;
        dwRet = pEndpoint->Serialize(pbBuffer, cbBuffer, &cbRequired);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }

        pbBuffer += cbRequired;
        cbBuffer -= cbRequired;
        *pcbRequired += cbRequired;
    }
    
EXIT:
    return dwRet;
}


// Finds out if an entire interface is supportable.
DWORD CInterface::IsSupportable(
    PUFN_MDD_CONTEXT pContext,
    UFN_BUS_SPEED Speed,
    BYTE bConfiguration
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_INVALID_PARAMETER;
    DWORD cEndpoints = GetEndpointCount();

    if( m_pUfnInterface->Descriptor.bAlternateSetting > 0 )
    {
        ASSERT( PddSupportsAlternateInterfaces( pContext ) );
    }
    
    for (DWORD dwEndpoint = 0; dwEndpoint < cEndpoints; ++dwEndpoint) {
        CMddEndpoint *pEndpoint = &m_pEndpoints[dwEndpoint];
        dwRet = pEndpoint->IsSupportable(pContext, Speed, bConfiguration, 
            m_pUfnInterface->Descriptor.bInterfaceNumber, 
            m_pUfnInterface->Descriptor.bAlternateSetting);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }

    dwRet = ERROR_SUCCESS;

EXIT:
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Cannot support provided %s speed interface\r\n"),
            pszFname, GetSpeedString(Speed)));
    }
    
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


CMddEndpoint::CMddEndpoint(
    ) 
{
    m_pUfnEndpoint = NULL;
}

CMddEndpoint::~CMddEndpoint(
    ) 
{
}
    

DWORD CMddEndpoint::Init(
    UFN_BUS_SPEED           Speed,
    PUFN_ENDPOINT           pUfnEndpoint
    )
{
    DEBUGCHK(ValidateDescriptor(Speed, pUfnEndpoint) == ERROR_SUCCESS);

    m_pUfnEndpoint = pUfnEndpoint;

    return ERROR_SUCCESS;
}


DWORD CMddEndpoint::IsSupportable(
    PUFN_MDD_CONTEXT pContext,
    UFN_BUS_SPEED Speed,
    BYTE bConfiguration,
    BYTE bInterfaceNumber,
    BYTE bAlternateSetting
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_SUCCESS;
    
    // Do not check endoint 0.
    for (DWORD dwPipe = 1; dwPipe < pContext->PddInfo.dwEndpointCount; ++dwPipe) {
        PCPipeBase pPipe = pContext->rgpPipes[dwPipe];
        
        if (pPipe->IsReserved(Speed, bConfiguration, bInterfaceNumber, bAlternateSetting)) {
            continue;
        }

        if (pContext->PddInfo.pfnIsEndpointSupportable(pContext->PddInfo.pvPddContext, 
                pPipe->GetPhysicalEndpoint(), Speed,
                &m_pUfnEndpoint->Descriptor, bConfiguration, bInterfaceNumber,
                bAlternateSetting) == ERROR_SUCCESS) {
            DEBUGMSG(ZONE_INIT,
                (_T("%s Endpoint can be supported by physical endpoint %u\r\n"), 
                pszFname, pPipe->GetPhysicalEndpoint()));

            pPipe->Reserve(Speed, TRUE, bConfiguration, bInterfaceNumber, bAlternateSetting, &m_pUfnEndpoint->Descriptor);
            break;
        }
    }

    if (dwPipe == pContext->PddInfo.dwEndpointCount) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Endpoint descriptor cannot be supported\r\n"),
            pszFname));
        dwRet = ERROR_INVALID_PARAMETER;
        goto EXIT;
    }

EXIT:
    FUNCTION_LEAVE_MSG();
    
    return dwRet;
}


DWORD CMddEndpoint::ValidateDescriptorAndChildren(
    UFN_BUS_SPEED           Speed,
    PUFN_ENDPOINT           pUfnEndpoint
    )
{
    return ValidateDescriptor(Speed, pUfnEndpoint);
}


DWORD CMddEndpoint::ValidateDescriptor(
    UFN_BUS_SPEED           Speed,
    PUFN_ENDPOINT           pUfnEndpoint
    )
{
    SETFNAME();
    
    DWORD dwRet = ERROR_INVALID_PARAMETER;

    if (!ValidatePacketSize(Speed, &pUfnEndpoint->Descriptor)) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid packet size for endpoint 0x%x\r\n"),
            pszFname, pUfnEndpoint->Descriptor.bEndpointAddress));
        goto EXIT;
    }

    if ( (pUfnEndpoint->pvExtended == NULL && pUfnEndpoint->cbExtended) ||
         (pUfnEndpoint->pvExtended && pUfnEndpoint->cbExtended == 0) ) {
        DEBUGMSG(ZONE_ERROR, (_T("%s Invalid cbExtended/pvExtended pair!\r\n"), pszFname));
        goto EXIT;
    }

    dwRet = ERROR_SUCCESS;

EXIT:
    return dwRet;
}

// See if the packet size adheres to the USB standard for the speed and type.
BOOL CMddEndpoint::ValidatePacketSize(
    UFN_BUS_SPEED            Speed,
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc
    )
{
    PREFAST_DEBUGCHK(pEndpointDesc);
    
    BOOL fRet = TRUE;

    WORD wPacketSize = 
        (pEndpointDesc->wMaxPacketSize & USB_ENDPOINT_MAX_PACKET_SIZE_MASK);
    
    switch (pEndpointDesc->bmAttributes & USB_ENDPOINT_TYPE_MASK) {
    case USB_ENDPOINT_TYPE_CONTROL:
        if ( (wPacketSize != 8) && (wPacketSize != 16) &&
             (wPacketSize != 32) && (wPacketSize != 64) ) {
            fRet = FALSE;
        }
        break;
        
    case USB_ENDPOINT_TYPE_BULK:
        if ( (wPacketSize != 8) && (wPacketSize != 16) &&
             (wPacketSize != 32) && (wPacketSize != 64) &&
             (wPacketSize != 512) ) {
            fRet = FALSE;
        }
        else if ( (wPacketSize == 512) && (Speed != BS_HIGH_SPEED) ) {
            fRet = FALSE;
        }
        break;
        
    case USB_ENDPOINT_TYPE_INTERRUPT:
        if ( (Speed == BS_FULL_SPEED) && (wPacketSize > 64) ) {
            fRet = FALSE;
        }
        else if (wPacketSize > 1024) {
            DEBUGCHK(Speed == BS_HIGH_SPEED);
            fRet = FALSE;
        }
        break;
        
    case USB_ENDPOINT_TYPE_ISOCHRONOUS:
        if ( (Speed == BS_FULL_SPEED) && (wPacketSize > 1023) ) {
            fRet = FALSE;
        }
        else if (wPacketSize > 1024) {
            DEBUGCHK(Speed == BS_HIGH_SPEED);
            fRet = FALSE;
        }
        break;

    default:
        // We should net get here.
        fRet = FALSE;
        break;
    }

    return fRet;
}


DWORD CMddEndpoint::Serialize(
    PBYTE   pbBuffer,
    DWORD   cbBuffer,
    PDWORD  pcbRequired
    )
{
    DWORD dwRet = ERROR_SUCCESS;
    
    if (pbBuffer) {
        dwRet = FillBuffer(&pbBuffer, &cbBuffer, &m_pUfnEndpoint->Descriptor, 
            sizeof(m_pUfnEndpoint->Descriptor));
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }
    else {
        DEBUGCHK(cbBuffer == 0);
    }

    if (pbBuffer) {
        dwRet = FillBuffer(&pbBuffer, &cbBuffer, m_pUfnEndpoint->pvExtended, 
            m_pUfnEndpoint->cbExtended);
        if (dwRet != ERROR_SUCCESS) {
            goto EXIT;
        }
    }
    
    *pcbRequired = sizeof(m_pUfnEndpoint->Descriptor) + m_pUfnEndpoint->cbExtended;
    
EXIT:
    return dwRet;
}


// Finds out if endpoint 0 is supportable for these parameters.
DWORD CMddEndpoint::IsEndpointZeroSupportable(
    PUFN_MDD_CONTEXT        pContext,
    UFN_BUS_SPEED           Speed,
    PUSB_DEVICE_DESCRIPTOR  pDeviceDesc
    )
{
    SETFNAME();
    FUNCTION_ENTER_MSG();

    ValidateContext(pContext);
    PREFAST_DEBUGCHK(pDeviceDesc);

    DWORD dwRet = ERROR_SUCCESS;

    USB_ENDPOINT_DESCRIPTOR EndpointDesc;
    InitializeEndpointZeroDescriptor(pDeviceDesc->bMaxPacketSize0, &EndpointDesc);
    
    if ( !ValidatePacketSize(Speed, &EndpointDesc) ||
         (pContext->PddInfo.pfnIsEndpointSupportable(pContext->PddInfo.pvPddContext, 0, Speed, 
            &EndpointDesc, 0, 0, 0) != ERROR_SUCCESS) ) {
        dwRet = ERROR_INVALID_PARAMETER;
        DEBUGMSG(ZONE_ERROR, (_T("%s Cannot satisfy %s speed EP0 packet size requirement of %u\r\n"), 
            pszFname, GetSpeedString(Speed), pDeviceDesc->bMaxPacketSize0));
        goto EXIT;
    }
    pDeviceDesc->bMaxPacketSize0 = (BYTE) EndpointDesc.wMaxPacketSize;

EXIT:
    FUNCTION_LEAVE_MSG();

    return dwRet;
}


VOID CMddEndpoint::InitializeEndpointZeroDescriptor(
    BYTE bMaxPacketSize,
    PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc
    )
{
    PREFAST_DEBUGCHK(pEndpointDesc);
    
    pEndpointDesc->bEndpointAddress = 0;
    pEndpointDesc->bmAttributes = USB_ENDPOINT_TYPE_CONTROL;
    pEndpointDesc->wMaxPacketSize = bMaxPacketSize;
}
