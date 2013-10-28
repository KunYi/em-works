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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//

#ifndef _DMTAUDIOPORT_H_
#define _DMTAUDIOPORT_H_

#include "audiostrmport.h"
#include "memtxapi.h"


//------------------------------------------------------------------------------
// Direct memory transfer audio stream port interface definition
//
class DMTAudioStreamPort : public AudioStreamPort
{
    //--------------------------------------------------------------------------
    // member variables
    //
public:
    typedef enum
        {
        DMTProfile_I2SSlave,
        DMTProfile_I2SMaster,
        DMTProfile_TDM
        }
    DMTProfile_e;

protected:
    HANDLE                                  m_hDMTPort;
    DMTProfile_e                            m_DMTProfile;
    void                                   *m_pCallbackData;
    DataTransfer_Command                    m_fnRxCommand;
    DataTransfer_Command                    m_fnTxCommand;
    PortConfigInfo_t                        m_PlayPortConfigInfo;
    PortConfigInfo_t                        m_RecPortConfigInfo;


public:
    AudioStreamPort_Client                 *m_pPortClient;


    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:
    DMTAudioStreamPort() :
        m_hDMTPort(NULL),
        m_DMTProfile(DMTProfile_I2SSlave),
        m_pPortClient(NULL),
        m_fnRxCommand(NULL),
        m_fnTxCommand(NULL),
        m_pCallbackData(NULL)
    {
    }

    ~DMTAudioStreamPort()
    {
        if (m_hDMTPort)
            {
            ::CloseHandle(m_hDMTPort);
            }
    }


    //--------------------------------------------------------------------------
    // protected methods
    //
protected:

    BOOL RegisterDirectMemoryTransferCallbacks();
    BOOL UnregisterDirectMemoryTransferCallbacks();


    //--------------------------------------------------------------------------
    // inline public methods
    //
public:

    virtual BOOL register_PORTHost(AudioStreamPort_Client *pClient)
    {
        m_pPortClient = pClient;
        return TRUE;
    }

    virtual void unregister_PORTHost(AudioStreamPort_Client *pClient)
    {
        m_pPortClient = NULL;
        UnregisterDirectMemoryTransferCallbacks();
    }


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    virtual BOOL signal_Port(DWORD SignalId,
                             HANDLE hStreamContext,
                             DWORD dwContextData);
    virtual BOOL open_Port(WCHAR const* szPort,
                           HANDLE hPlayPortConfigInfo,
                           HANDLE hRecPortConfigInfo);

    virtual void set_DMTProfile(DMTProfile_e dmtProfile)
    {
        m_DMTProfile = dmtProfile;
        m_PlayPortConfigInfo.portProfile = m_DMTProfile;
        m_RecPortConfigInfo.portProfile = m_DMTProfile;
    }

    virtual DMTProfile_e get_DMTProfile()
    {
        return m_DMTProfile;
    }

};




#endif //_DMTAUDIOPORT_H_

