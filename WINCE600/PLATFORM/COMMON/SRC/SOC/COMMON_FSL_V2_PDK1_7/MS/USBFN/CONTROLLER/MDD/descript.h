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
#pragma once

#include <usbfntypes.h>

typedef struct UFN_MDD_CONTEXT *PUFN_MDD_CONTEXT;



enum CONTROL_RESPONSE {
    CR_SUCCESS = 0,
    CR_SUCCESS_TX,
    CR_SUCCESS_SEND_CONTROL_HANDSHAKE, // Use if no data stage
    CR_STALL_DEFAULT_PIPE,
    CR_UNHANDLED_REQUEST,
};



#define USB_REQUEST_RECIPIENT_MASK      0x1F
#define GET_REQUESET_RECIPIENT(x)       ((x) & USB_REQUEST_RECIPIENT_MASK)


class CMddEndpoint {
public:
    CMddEndpoint();
    ~CMddEndpoint();

#if 0
#ifdef DEBUG
    VOID Validate() {
        if (m_pbUfnEndpointBuffer) {
            DEBUGCHK(m_pUfnEndpoint == m_pbUfnEndpointBuffer);
        }
    }
#endif
#endif
    
    DWORD Init(
        UFN_BUS_SPEED           Speed,
        PUFN_ENDPOINT           pUfnEndpoint
        );

    DWORD Serialize(
        PBYTE   pbBuffer,
        DWORD   cbBuffer,
        PDWORD  pcbRequired
        );

    DWORD IsSupportable(
        PUFN_MDD_CONTEXT pContext,
        UFN_BUS_SPEED Speed,
        BYTE bConfiguration,
        BYTE bInterfaceNumber,
        BYTE bAlternateSetting
        );

#if 0
    DWORD Clone();
#endif

    static DWORD ValidateDescriptorAndChildren(
        UFN_BUS_SPEED           Speed,
        PUFN_ENDPOINT           pUfnEndpoint
        );

    static DWORD ValidateDescriptor(
        UFN_BUS_SPEED           Speed,
        PUFN_ENDPOINT           pUfnEndpoint
        );

    static DWORD IsEndpointZeroSupportable(
        PUFN_MDD_CONTEXT        pContext,
        UFN_BUS_SPEED           Speed,
        PUSB_DEVICE_DESCRIPTOR  pDeviceDesc
        );

    static VOID InitializeEndpointZeroDescriptor(
        BYTE bMaxPacketSize,
        PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc
        );
    
    PUFN_ENDPOINT   m_pUfnEndpoint;
    
private:
    // See if the packet size adheres to the USB standard for the speed and type.
    static BOOL ValidatePacketSize(
        UFN_BUS_SPEED            Speed,
        PUSB_ENDPOINT_DESCRIPTOR pEndpointDesc
        );
    
#if 0
    PBYTE           m_pbUfnEndpointBuffer;
#endif
};


class CInterface {
public:
    CInterface();
    ~CInterface();
    
    DWORD Init(
        UFN_BUS_SPEED           Speed,
        PUFN_INTERFACE          pUfnInterface
        );

    DWORD Serialize(
        PBYTE   pbBuffer,
        DWORD   cbBuffer,
        PDWORD  pcbRequired
        );

    DWORD IsSupportable(
        PUFN_MDD_CONTEXT pContext,
        UFN_BUS_SPEED Speed,
        BYTE bConfiguration
        );
    
    static DWORD ValidateDescriptorAndChildren(
        UFN_BUS_SPEED           Speed,
        PUFN_INTERFACE          pUfnInterface
        );

    static DWORD ValidateDescriptor(
        UFN_BUS_SPEED           Speed,
        PUFN_INTERFACE          pUfnInterface
        );
    
    static DWORD CountInterfaces(
        PUFN_CONFIGURATION      pUfnConfig
        );

    DWORD GetEndpointCount() const { return m_pUfnInterface->Descriptor.bNumEndpoints; }
    DWORD GetInterfaceNumber() const { return m_pUfnInterface->Descriptor.bInterfaceNumber; }
    DWORD GetAlternateSetting() const { return m_pUfnInterface->Descriptor.bAlternateSetting; }

    VOID SetInterface(
        DWORD dwAltSetting
        )
    {
        // Only store the current alternate setting in the default (0)
        DEBUGCHK(m_pUfnInterface->Descriptor.bAlternateSetting == 0);
        m_dwCurrAltSetting = dwAltSetting;
    }

    DWORD GetInterface(
        ) const
    {
        // Only store the current alternate setting in the default (0)
        DEBUGCHK(m_pUfnInterface->Descriptor.bAlternateSetting == 0);
        return m_dwCurrAltSetting;
    }
    
    PUFN_INTERFACE      m_pUfnInterface;
    DWORD               m_dwCurrAltSetting;

    CMddEndpoint          *m_pEndpoints;
};


typedef class CConfiguration {
public:
    CConfiguration();
    ~CConfiguration();
    
    DWORD Init(
        UFN_BUS_SPEED           Speed,
        PUFN_CONFIGURATION      pUfnConfig
        );

    DWORD IsSupportable(
        PUFN_MDD_CONTEXT        pContext,
        UFN_BUS_SPEED           Speed,
        PUSB_DEVICE_DESCRIPTOR  pDeviceDesc
        );

    DWORD Serialize(
        PBYTE   pbBuffer,
        DWORD   cbBuffer,
        PDWORD  pcbRequired
        );

    DWORD GenerateTotalConfigDescriptor(
        );

    static DWORD ValidateDescriptorAndChildren(
        UFN_BUS_SPEED           Speed,
        PUFN_CONFIGURATION      pUfnConfig
        );
    
    static DWORD ValidateDescriptor(
        UFN_BUS_SPEED           Speed,
        PUFN_CONFIGURATION      pUfnConfig
        );

    // Copies a UFN config tree structure.
    static DWORD CopyUfnConfigTree(
        PUFN_CONFIGURATION  pConfigDest,
        PUFN_CONFIGURATION  pConfigSource
        );
    
    // Free all allocated data in a config tree structure.
    static VOID FreeUfnConfigTree(
        PUFN_CONFIGURATION pConfig
        );

    DWORD GetDefaultInterfaceCount(
        ) const
    {
        DWORD dwDefaultInterfaces = 0;
        for (DWORD dwInterface = 0; dwInterface < GetInterfaceCount(); ++dwInterface) {
            CInterface *pInterfaceCurr = &m_pInterfaces[dwInterface];
            if (pInterfaceCurr->GetAlternateSetting() == 0) {
                ++dwDefaultInterfaces;
            }
        }

        return dwDefaultInterfaces;
    }

    DWORD GetInterfaceCount() const { return m_pUfnConfig->Descriptor.bNumInterfaces; }
    DWORD GetConfigurationValue() const { return m_pUfnConfig->Descriptor.bConfigurationValue; }

    DWORD SetInterface(
        DWORD dwInterfaceIndex, 
        DWORD dwAltSetting
        )
    {
        DWORD dwRet = ERROR_INVALID_PARAMETER;

        CInterface *pInterfaceDefault = GetInterfaceClass(dwInterfaceIndex, 0);
        if (pInterfaceDefault == NULL) {
            goto EXIT;
        }
        
        CInterface *pInterface = GetInterfaceClass(dwInterfaceIndex, dwAltSetting);
        if (pInterface == NULL) {
            goto EXIT;
        }

        // The current alternate setting is stored in the default setting's
        // object.
        pInterfaceDefault->SetInterface(dwAltSetting);
        dwRet = ERROR_SUCCESS;
        
    EXIT:
        return dwRet;
    }

    // this function determines the current AlternateSetting value for the interface with 
    // index dwInterfaceIndex.  The alternate value will be returned in the are pointed by pdwAltSetting.
    DWORD GetInterface(
        DWORD dwInterfaceIndex, 
        PDWORD pdwAltSetting
        )
    {
        DWORD dwRet = ERROR_INVALID_PARAMETER;

        const CInterface *pInterfaceDefault = GetInterfaceClass(dwInterfaceIndex, 0);
        if (pInterfaceDefault == NULL) {
            goto EXIT;
        }

        *pdwAltSetting = pInterfaceDefault->GetInterface();
        dwRet = ERROR_SUCCESS;

    EXIT:
        return dwRet;
    }
    

    CInterface* GetInterfaceClass(
        DWORD dwInterfaceIndex, 
        DWORD dwAltSetting
        )
    {
        CInterface *pInterface = NULL;

        for (DWORD dwInterface = 0; dwInterface < GetInterfaceCount(); ++dwInterface) {
            CInterface *pInterfaceCurr = &m_pInterfaces[dwInterface];
            if ( (pInterfaceCurr->GetInterfaceNumber() == dwInterfaceIndex) &&
                 (pInterfaceCurr->GetAlternateSetting() == dwAltSetting) ) {
                pInterface = pInterfaceCurr;
            }
        }

        return pInterface;
    }


    VOID ResetInterfaces(
        )
    {
        for (DWORD dwInterface = 0; dwInterface < GetDefaultInterfaceCount(); ++dwInterface) {
            SetInterface( dwInterface, 0 );
        }
    }
        

    PUFN_CONFIGURATION       m_pUfnConfig;

    CInterface             *m_pInterfaces;

    PUSB_CONFIGURATION_DESCRIPTOR m_pConfigDesc;
    
} *PCConfiguration;


class CDevice {
public:
    CDevice();
    ~CDevice();

    DWORD Init(
        UFN_BUS_SPEED           Speed,
        PUSB_DEVICE_DESCRIPTOR  pDeviceDesc,
        PUFN_CONFIGURATION      pUfnConfig
        );

    DWORD GenerateTotalConfigDescriptors(
        );

    DWORD Clone(
        CDevice *pdevDst
        );

    DWORD IsSupportable(
        PUFN_MDD_CONTEXT pContext,
        UFN_BUS_SPEED Speed
        );

    DWORD GetConfiguration(
        ) const
    {
        DWORD dwConfigurationValue = m_dwConfigurationValue;
        DEBUGCHK( (dwConfigurationValue == 0) ||
            (GetConfigurationClass(dwConfigurationValue) != NULL) );
        return dwConfigurationValue;
    }

    DWORD SetConfiguration(
        DWORD dwConfigurationValue
        ) 
    {
        DWORD dwRet = ERROR_SUCCESS;

        if (dwConfigurationValue == 0) {
            // Back to addressed state
            m_dwConfigurationValue = dwConfigurationValue;
        }
        else {
            CConfiguration *pConfiguration = GetConfigurationClass(dwConfigurationValue);
            if (pConfiguration == NULL) {
                dwRet = ERROR_INVALID_PARAMETER;
            }
            else {
                m_dwConfigurationValue = dwConfigurationValue;
                pConfiguration->ResetInterfaces();
            }
        }
        
        return dwRet;
    }

    DWORD GetDefaultInterfaceCount(
        ) const
    {
        DWORD dwCurrConfigValue = GetConfiguration();
        CConfiguration *pConfiguration = GetConfigurationClass(dwCurrConfigValue);
        return pConfiguration->GetDefaultInterfaceCount();
    }    

    // this function determines the current AlternateSetting value for the interface with 
    // index dwInterfaceIndex in currently selected configuration.  The alternate value 
    // will be returned in the are pointed by pdwAltSetting.
    DWORD GetInterface(
        DWORD dwInterfaceIndex,
        PDWORD pdwCurrentAltSetting
        )
    {
        DEBUGCHK(m_dwConfigurationValue != 0);
        CConfiguration *pConfiguration = 
            GetConfigurationClass(m_dwConfigurationValue);
        PREFAST_DEBUGCHK(pConfiguration);
        return pConfiguration->GetInterface(dwInterfaceIndex, pdwCurrentAltSetting);
    }

    DWORD SetInterface(
        DWORD dwInterfaceIndex,
        DWORD dwAltSetting
        )
    {
        DEBUGCHK(m_dwConfigurationValue != 0);
        CConfiguration *pConfiguration = GetConfigurationClass(m_dwConfigurationValue);
        PREFAST_DEBUGCHK(pConfiguration);
        return pConfiguration->SetInterface(dwInterfaceIndex, dwAltSetting);
    }

    
#if 0
    static DWORD IsSupportable(
        PUFN_MDD_CONTEXT        pContext,
        PUSB_DEVICE_DESCRIPTOR  pDeviceDesc,
        PUFN_CONFIGURATION      pConfig,
        UFN_BUS_SPEED           Speed
        );
#endif

    static DWORD ValidateDescriptorAndChildren(
        UFN_BUS_SPEED           Speed,
        PUSB_DEVICE_DESCRIPTOR  pDeviceDesc,
        PUFN_CONFIGURATION      pUfnConfig
        );

    static DWORD ValidateDescriptor(
        UFN_BUS_SPEED           Speed,
        PUSB_DEVICE_DESCRIPTOR  pDeviceDesc,
        PUFN_CONFIGURATION      pUfnConfig
        );

    DWORD GetConfigurationCount() const { return m_pDeviceDesc->bNumConfigurations; }

    PUSB_CONFIGURATION_DESCRIPTOR GetTotalConfigurationDescriptorByIndex(
        DWORD dwIndex
        )
    {
        PUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = NULL;

        if (dwIndex < GetConfigurationCount()) {
            CConfiguration *pConfig = &m_pConfigurations[dwIndex];
            pConfigDesc = pConfig->m_pConfigDesc;
        }

        return pConfigDesc;
    }

    PCUSB_CONFIGURATION_DESCRIPTOR GetTotalConfigurationDescriptorByValue(
        DWORD dwValue
        )
    {
        PCUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = NULL;
        const CConfiguration *pConfig = GetConfigurationClass(dwValue);
        if (pConfig) {
            pConfigDesc = pConfig->m_pConfigDesc;
        }

        return pConfigDesc;
    }


    CConfiguration* GetConfigurationClass(
        DWORD dwConfigurationValue
        ) const
    {
        CConfiguration *pConfiguration = NULL;
        
        for (DWORD dwConfig = 0; dwConfig < GetConfigurationCount(); ++dwConfig) {
            CConfiguration *pConfigurationCurr = &m_pConfigurations[dwConfig];
            if (pConfigurationCurr->GetConfigurationValue() == dwConfigurationValue) {
                pConfiguration = pConfigurationCurr;
                break;
            }
        }

        return pConfiguration;
    }
    
    PUSB_DEVICE_DESCRIPTOR  m_pDeviceDesc;
    PUFN_CONFIGURATION      m_pUfnConfig;

    CConfiguration         *m_pConfigurations;
    DWORD                   m_dwConfigurationValue; // Currently selected configuration

    USB_DEVICE_DESCRIPTOR   m_DeviceDescCopy;
    PUFN_CONFIGURATION      m_pUfnConfigCopy;
};


class CDescriptors {
public:
    CDescriptors();
    ~CDescriptors();
    
    DWORD Init(
        PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
        PUFN_CONFIGURATION      pHighSpeedConfig,
        PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
        PUFN_CONFIGURATION      pFullSpeedConfig,
        PCUFN_STRING_SET        pStringSets,
        DWORD                   cStringSets);

    DWORD IsSupportable(
        PUFN_MDD_CONTEXT pContext
        );

    DWORD GenerateTotalConfigDescriptors(
        );

    DWORD Clone(
        CDescriptors  **ppdescDst
        );

    DWORD RegisterDevice(
        PUFN_MDD_CONTEXT pContext
        );

    PCUSB_DEVICE_DESCRIPTOR GetDeviceDescriptor(
        UFN_BUS_SPEED Speed
        );

    static DWORD ValidateDescriptorAndChildren(
        PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
        PUFN_CONFIGURATION      pHighSpeedConfig,
        PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
        PUFN_CONFIGURATION      pFullSpeedConfig,
        PCUFN_STRING_SET        pStringSets,
        DWORD                   cStringSets
        );

    static DWORD ValidateDescriptor(
        PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
        PUFN_CONFIGURATION      pHighSpeedConfig,
        PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
        PUFN_CONFIGURATION      pFullSpeedConfig,
        PCUFN_STRING_SET        pStringSets,
        DWORD                   cStringSets
        );

    // Free all allocated UFN structures in pContext.
    static VOID FreeUfnStructs(
        PUFN_MDD_CONTEXT pContext
        );

    CONTROL_RESPONSE ProcessGetDescriptor(
        UFN_BUS_SPEED Speed,
        USB_DEVICE_REQUEST udr,
        DWORD dwMsg,
        BOOL fPddSupportsHighSpeed,
        PVOID *ppvBuffer,
        PDWORD pcbBuffer
        );

    PCUSB_CONFIGURATION_DESCRIPTOR GetTotalConfigurationDescriptorByValue(
        UFN_BUS_SPEED Speed,
        DWORD dwValue
        )
    {
        CDevice* pDevice = GetDevice(Speed);
        PCUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
            pDevice->GetTotalConfigurationDescriptorByValue(dwValue);
        return pConfigDesc;
    }

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
    PUSB_CONFIGURATION_DESCRIPTOR GetTotalConfigurationDescriptorByIndex(
        UFN_BUS_SPEED Speed,
        DWORD dwIndex
        )
    {
        CDevice* pDevice = GetDevice(Speed);
        PUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
            pDevice->GetTotalConfigurationDescriptorByIndex(dwIndex);
        return pConfigDesc;
    }
#endif

    // Returns current configuration number
    DWORD GetConfiguration(
        UFN_BUS_SPEED   Speed
        )
    {
        const CDevice* pDevice = GetDevice(Speed);
        return pDevice->GetConfiguration();
    }

    DWORD SetConfiguration(
        UFN_BUS_SPEED   Speed,
        DWORD           dwConfigurationValue
        )
    {
        CDevice* pDevice = GetDevice(Speed);
        return pDevice->SetConfiguration(dwConfigurationValue);
    }

    DWORD GetDefaultInterfaceCount(
        UFN_BUS_SPEED   Speed
        )
    {
        CDevice *pDevice = GetDevice(Speed);
        return pDevice->GetDefaultInterfaceCount();
    }

    // Returns error code
    DWORD GetInterface(
        UFN_BUS_SPEED Speed,
        DWORD dwInterfaceIndex, 
        PDWORD pdwAltSetting
        )
    {
        CDevice* pDevice = GetDevice(Speed);
        return pDevice->GetInterface(dwInterfaceIndex, pdwAltSetting);
    }

    DWORD SetInterface(
        UFN_BUS_SPEED Speed,
        DWORD dwInterfaceIndex, 
        DWORD dwAltSetting
        )
    {
        CDevice* pDevice = GetDevice(Speed);
        return pDevice->SetInterface(dwInterfaceIndex, dwAltSetting);
    }
    
#if 0
    // Copy the user-supplied UFN structures to the pContext.
    static DWORD CopyUfnStructs(
        PUFN_MDD_CONTEXT        pContext,
        PUSB_DEVICE_DESCRIPTOR  pHighSpeedDeviceDesc,
        PUFN_CONFIGURATION      pHighSpeedConfig,
        PUSB_DEVICE_DESCRIPTOR  pFullSpeedDeviceDesc,
        PUFN_CONFIGURATION      pFullSpeedConfig,
        PCUFN_STRING_SET        pStringSets,
        DWORD                   cStringSets
        );
#endif
    // Copy string descriptors.
    static DWORD CloneStrings(
        PCUFN_STRING_SET    pStringSetsSrc,
        DWORD               cStringSetsSrc,
        PUFN_STRING_SET    *ppStringSetsDst
        );

    // Free string descriptors in pContext.
    static DWORD FreeStrings(
        PUFN_MDD_CONTEXT pContext
        );

    PCUFN_STRING_SET        m_pStringSets;
    DWORD                   m_cStringSets;

    CDevice m_devHighSpeed;
    CDevice m_devFullSpeed;

    PBYTE m_pbStringBuffer;

private:
    CDevice* GetDevice(
        UFN_BUS_SPEED Speed
        )
    {
        DEBUGCHK(IS_VALID_SPEED(Speed));
        CDevice* pDevice;
        
        if (Speed == BS_FULL_SPEED) {
            pDevice = &m_devFullSpeed;
        }
        else {
            pDevice = &m_devHighSpeed;
        }
        
        return pDevice;
    }

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
    PUSB_CONFIGURATION_DESCRIPTOR GetTotalConfigurationDescriptorByIndex(
        UFN_BUS_SPEED Speed,
        DWORD dwIndex
        )
    {
        CDevice* pDevice = GetDevice(Speed);
        PUSB_CONFIGURATION_DESCRIPTOR pConfigDesc = 
            pDevice->GetTotalConfigurationDescriptorByIndex(dwIndex);
        return pConfigDesc;
    }
#endif
    // Buffers for data to send to the host. Note that the host may
    // only ask for one of these at a time.
    union {
        BYTE m_rgbBuffer[2];
        USB_DEVICE_QUALIFIER_DESCRIPTOR m_DeviceQualifierDescToSend;
        BYTE m_rgbStringDesc[256];
    };    
};



