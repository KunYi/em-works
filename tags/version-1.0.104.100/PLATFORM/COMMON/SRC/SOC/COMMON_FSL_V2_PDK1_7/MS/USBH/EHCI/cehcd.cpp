//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//------------------------------------------------------------------------------
// Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------
//
// Module Name:
//     cehcd.cpp
// Abstract:
//     This file contains the CEhcd object, which is the main entry
//     point for all HCDI calls by USBD
//
// Notes:
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214 6262)
#include <windows.h>
#include "cehcd.h"
#include "cpipe.h"
#pragma warning(pop)

// ****************************************************************
// PUBLIC FUNCTIONS
// ****************************************************************

// ******************************************************************
#ifdef QFE_MERGE /*070630*/ /*CE5QFE*/
CEhcd::CEhcd( IN LPVOID pvUhcdPddObject,
                        IN CPhysMem * pCPhysMem,
                        IN LPCWSTR lpRegPath, // szDriverRegistryKey ignored for now
                        IN REGISTER portBase,
                        IN DWORD dwOffset,
                        IN DWORD dwSysIntr)
:CHW( portBase, dwOffset, dwSysIntr, pCPhysMem, pvUhcdPddObject,lpRegPath)
,m_pMem(pCPhysMem)
#else
CEhcd::CEhcd( IN LPVOID pvUhcdPddObject,
                        IN CPhysMem * pCPhysMem,
                        IN LPCWSTR, // szDriverRegistryKey ignored for now
                        IN REGISTER portBase,
                        IN DWORD dwSysIntr)
:CHW( portBase,dwSysIntr, pCPhysMem, pvUhcdPddObject )
,m_pMem(pCPhysMem)
#endif
// Purpose: Initialize variables associated with this class
//
// Parameters: None
//
// Returns: Nothing (Constructor can NOT fail!)
//
// Notes: *All* initialization which could possibly fail should be done
//        via the Initialize() routine, which is called right after
//        the constructor
// ******************************************************************
{
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("+CEhcd::CEhcd\n")));
    m_dwSysIntr = dwSysIntr;
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("-CEhcd::CEhcd\n")));
}

// ******************************************************************
CEhcd::~CEhcd()
//
// Purpose: Destroy all memory and objects associated with this class
//
// Parameters: None
//
// Returns: Nothing
//
// Notes:
// ******************************************************************
{
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("+CEhcd::~CEhcd\n")));

    // make the API set inaccessible
    CRootHub *pRoot = SetRootHub(NULL);
    // signal root hub to close
    if ( pRoot ) {
        pRoot->HandleDetach();
        delete pRoot;
    }
    CHW::StopHostController();

    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("-CEhcd::~CEhcd\n")));
}
void CEhcd::DeviceDeInitialize( void )
{
    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("+CEhcd::DeInitialize\n")));

    CHW::StopHostController();

    // make the API set inaccessible
    CRootHub *pRoot = SetRootHub(NULL);
    // signal root hub to close
    if ( pRoot ) {
        pRoot->HandleDetach();
        delete pRoot;
    }
    // this is safe because by now all clients have been unloaded
    //DeleteCriticalSection ( &m_csHCLock );

    CHW::DeInitialize();
    CDeviceGlobal::DeInitialize();

    DEBUGMSG(ZONE_UHCD && ZONE_VERBOSE, (TEXT("-CEhcd::DeInitialize\n")));

}

// ******************************************************************
BOOL CEhcd::DeviceInitialize()
//
// Purpose: Set up the Host Controller hardware, associated data structures,
//          and threads so that schedule processing can begin.
//
// Parameters: pvUhcdPddObject - pointer to the PDD object for this driver
//
//             pCPhysMem - pointer to class for managing physical memory
//
//             szDriverRegistryKey - unused ?
//
//             portBase - base address for USB registers
//
//             dwSysIntr - interrupt identifier for USB interrupts
//
// Returns: TRUE - if initializes successfully and is ready to process
//                 the schedule
//          FALSE - if setup fails
//
// Notes: This function is called by right after the constructor.
//        It is the starting point for all initialization.
//
//        This needs to be implemented for HCDI
// ******************************************************************
{
    DEBUGMSG(ZONE_INIT,(TEXT("+CEhcd::Initialize. Entry\r\n")));


    // All Initialize routines must be called, so we can't write
    // if ( !CDevice::Initialize() || !CPipe::Initialize() etc )
    // due to short circuit eval.
    {
        m_pMem ->ReInit();
        BOOL fCDeviceInitOK = CDeviceGlobal::Initialize(this);
        BOOL fCHWInitOK = CHW::Initialize(); //UsbInterruptThreadStub is created here

        if ( !fCDeviceInitOK || !fCHWInitOK ) {
            DEBUGMSG(ZONE_ERROR, (TEXT("-CEhcd::Initialize. Error - could not initialize device/pipe/hw classes\n")));
            ASSERT(FALSE);
            return FALSE;
        }
    }

    // set up the root hub object
    {
        USB_DEVICE_INFO deviceInfo;
        USB_HUB_DESCRIPTOR usbHubDescriptor;

        deviceInfo.dwCount = sizeof( USB_DEVICE_INFO );
        deviceInfo.lpConfigs = NULL;
        deviceInfo.lpActiveConfig = NULL;
        deviceInfo.Descriptor.bLength = sizeof( USB_DEVICE_DESCRIPTOR );
        deviceInfo.Descriptor.bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE;
        deviceInfo.Descriptor.bcdUSB = 0x200; // USB spec 200
        deviceInfo.Descriptor.bDeviceClass = USB_DEVICE_CLASS_HUB;
        deviceInfo.Descriptor.bDeviceSubClass = 0xff;
        deviceInfo.Descriptor.bDeviceProtocol = 0xff;
        deviceInfo.Descriptor.bMaxPacketSize0 = 0;
        deviceInfo.Descriptor.bNumConfigurations = 0;

        usbHubDescriptor.bDescriptorType = USB_HUB_DESCRIPTOR_TYPE;
        usbHubDescriptor.bDescriptorLength = USB_HUB_DESCRIPTOR_MINIMUM_SIZE;
        usbHubDescriptor.bNumberOfPorts = (BYTE)GetNumberOfPort();
        usbHubDescriptor.wHubCharacteristics = USB_HUB_CHARACTERISTIC_NO_POWER_SWITCHING |
                                               USB_HUB_CHARACTERISTIC_NOT_PART_OF_COMPOUND_DEVICE |
                                               USB_HUB_CHARACTERISTIC_NO_OVER_CURRENT_PROTECTION;
        usbHubDescriptor.bPowerOnToPowerGood = 0;
        usbHubDescriptor.bHubControlCurrent = 0;
        DEBUGCHK( usbHubDescriptor.bNumberOfPorts < 8 );
        usbHubDescriptor.bRemoveAndPowerMask[0] = 0; // all devices on hub are removable
        usbHubDescriptor.bRemoveAndPowerMask[1] = 0xFF; // must be 0xFF, USB spec 1.1, table 11-8

        // FALSE indicates root hub is not low speed
        // (though, this is ignored for hubs anyway)
        SetRootHub( new CRootHub( deviceInfo, FALSE,TRUE, usbHubDescriptor,this ));
    }
    if ( !GetRootHub() ) {
        DEBUGMSG( ZONE_ERROR, (TEXT("-CEhcd::Initialize - unable to create root hub object\n")) );
        return FALSE;
    }

    // Signal root hub to start processing port changes
    // The root hub doesn't have any pipes, so we pass NULL as the
    // endpoint0 pipe
    if ( !GetRootHub()->EnterOperationalState( NULL ) ) {
        DEBUGMSG(ZONE_ERROR, (TEXT("-CEhcd::Initialize. Error initializing root hub\n")));
        return FALSE;
    }
    // Start processing frames
    CHW::EnterOperationalState();

    DEBUGMSG(ZONE_INIT,(TEXT("-CEhcd::Initialize. Success!!\r\n")));
    return TRUE;
}

CHcd * CreateHCDObject(IN LPVOID pvUhcdPddObject,
                     IN CPhysMem * pCPhysMem,
                     IN DWORD dwOffset,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwSysIntr)
{
    return new CEhcd (pvUhcdPddObject, pCPhysMem,szDriverRegistryKey,portBase, dwOffset, dwSysIntr);
}



