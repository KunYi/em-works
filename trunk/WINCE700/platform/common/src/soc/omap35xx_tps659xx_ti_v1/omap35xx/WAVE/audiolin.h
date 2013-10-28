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

#ifndef __AUDIOLIN_H__
#define __AUDIOLIN_H__

#include "linkqueue.h"

//------------------------------------------------------------------------------
//
//  Prototype
//
class CAudioMixerManager;
class CAudioControlBase;

//------------------------------------------------------------------------------
//
//  Serves as a base class for all Audio Line objects.
//  - Allows subclasses to easily plug-in to the mixer manager
//  - Allows subclasses to inherit the built-in communication path between
//    audio-lines and audio-controls
//  - manages all audio controls associated with the audio line
//
class CAudioLineBase : private QUEUE_ENTRY
{
friend CAudioMixerManager;

    //--------------------------------------------------------------------------
    // public typedefs, enums, and structs
    //
public:

    typedef QUEUE_ENTRY         ControlList;


    //--------------------------------------------------------------------------
    // private member variables
    //  - made private because this value must be immutable
    //
private:
    WORD                        m_DestinationId;
    WORD                        m_SourceId;
    WORD                        m_LineId;
    WORD                        m_countConnections;
    WORD                        m_countControls;
    ControlList                *m_pAudioControls;


    //--------------------------------------------------------------------------
    // member variables
    //
protected:

    DWORD                       m_DeviceId;
    DWORD                       m_ComponentType;
    DWORD                       m_countChannels;
    DWORD                       m_TargetType;
    DWORD                       m_ffLineStatus;

    //--------------------------------------------------------------------------
    // private methods
    //
private:

    void increment_ConnectionCount();
    void put_AudioLineIds(WORD LineId, WORD DestinationId, WORD SourceId);


    //--------------------------------------------------------------------------
    // constructor/destructor
    //
protected:

    CAudioLineBase();


    //--------------------------------------------------------------------------
    // public inline methods
    //
public:

    WORD get_DestinationId() const          { return m_DestinationId; }
    WORD get_SourceId() const               { return m_SourceId; }
    WORD get_LineId() const                 { return m_LineId; }
    WORD get_ConnectionCount() const        { return m_countConnections; }
    DWORD get_ComponentType() const         { return m_ComponentType; }
    DWORD get_TargetType() const            { return m_TargetType; }
    WORD get_ControlCount() const           { return m_countControls; }
    DWORD get_ChannelCount() const          { return m_countChannels; }
    DWORD get_LineStatus() const            { return m_ffLineStatus; }
    DWORD get_DeviceId() const              { return m_DeviceId; }


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    DWORD register_AudioControl(CAudioControlBase *pControl);

    CAudioControlBase* query_ControlByIndex(int i) const;
    CAudioControlBase* query_ControlByControlId(DWORD ControlId) const;
    CAudioControlBase* query_ControlByControlType(DWORD ControlType) const;


    //--------------------------------------------------------------------------
    // public virtual methods
    //
public:

    virtual WCHAR const* get_ProductName() const               { return NULL; }

    virtual BOOL initialize_AudioLine(CAudioMixerManager *pAudioMixerManager)
    {
        return TRUE;
    }

    virtual DWORD get_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag) const
    {
        return MMSYSERR_INVALPARAM;
    }

    virtual DWORD put_AudioValue(
        CAudioControlBase *pControl,
        PMIXERCONTROLDETAILS pDetail,
        DWORD dwFlag)
    {
        return MMSYSERR_INVALPARAM;
    }

    virtual void copy_LineInfo(PMIXERLINE pMixerInfo) const;


    //--------------------------------------------------------------------------
    // public pure virtual methods
    //
public:

    virtual WCHAR const* get_Name() const                                   =0;
    virtual WCHAR const* get_ShortName() const                              =0;

};


//------------------------------------------------------------------------------
//
//  CAudioLine_WavOutVolume
//
class CAudioLine_WavOutVolume : public CAudioLineBase
{
protected:

    CAudioLine_WavOutVolume() : CAudioLineBase()
    {
        m_ComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
        m_countChannels = 2;
        m_TargetType = MIXERLINE_TARGETTYPE_WAVEOUT;
        m_ffLineStatus = MIXERLINE_LINEF_ACTIVE;
    }
};


//------------------------------------------------------------------------------
//
//  CAudioLine_WavInVolume
//
class CAudioLine_WavInVolume : public CAudioLineBase
{
protected:

    CAudioLine_WavInVolume() : CAudioLineBase()
    {
        m_ComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
        m_countChannels = 1;
        m_TargetType = MIXERLINE_TARGETTYPE_WAVEIN;
        m_ffLineStatus = MIXERLINE_LINEF_ACTIVE;
    }
};


#endif //__AUDIOLIN_H__

