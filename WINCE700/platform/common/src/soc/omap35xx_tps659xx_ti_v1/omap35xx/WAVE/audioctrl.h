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

#ifndef __AUDIOCTRL_H__
#define __AUDIOCTRL_H__

#include "linkqueue.h"

//------------------------------------------------------------------------------
//
//  Prototype
//
class CAudioLineBase;
class CAudioMixerManager;

//------------------------------------------------------------------------------
//
//  Base class for all audio control objects
//
class CAudioControlBase : private QUEUE_ENTRY
{
friend CAudioLineBase;
friend CAudioMixerManager;

    //--------------------------------------------------------------------------
    // member private variables
    //
private:

    CAudioLineBase             *m_pAudioLine;
    DWORD                       m_ControlId;   // assigned by the audio mgr
                                               // value is immutable

    //--------------------------------------------------------------------------
    // member protected variables
    //
protected:

    HANDLE                      m_hReference;   // may be used by audio lines
                                                // as a reference point

    //--------------------------------------------------------------------------
    // private methods
    //
private:

    void put_ControlId(DWORD ControlId, CAudioLineBase* pLine)
        {
        m_ControlId = ControlId;
        m_pAudioLine = pLine;
        }


    //--------------------------------------------------------------------------
    // constructor/destructor
    //
public:
    CAudioControlBase();


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    CAudioLineBase* get_AudioLine() const       { return m_pAudioLine; }

    DWORD get_ControlId() const                 { return m_ControlId; }
    HANDLE get_ReferenceHandle() const          { return m_hReference; }

    DWORD put_ReferenceHandle(HANDLE hRef)      { m_hReference = hRef; }


    //--------------------------------------------------------------------------
    // public methods
    //
public:

    virtual DWORD get_MultipleItemCount() const { return 0; }

    virtual DWORD get_StatusFlag() const
    {
        return MIXERCONTROL_CONTROLF_UNIFORM;
    }


    virtual void copy_ControlInfo(MIXERCONTROL *pControlInfo);

    virtual DWORD put_Value(PMIXERCONTROLDETAILS pDetail, DWORD dwFlags);
    virtual DWORD get_Value(PMIXERCONTROLDETAILS pDetail, DWORD dwFlags);


    //--------------------------------------------------------------------------
    // public pure virtual methods
    //
public:

    virtual WCHAR const* get_Name() const                                   =0;
    virtual WCHAR const* get_ShortName() const                              =0;
    virtual DWORD get_ControlType() const                                   =0;
};



template<DWORD ControlType>
class CAudioControlType : public CAudioControlBase
{
    //--------------------------------------------------------------------------
    // public pure virtual methods
    //
public:

    virtual DWORD get_ControlType() const   { return ControlType; }
};

#endif //__AUDIOCTRL_H__
