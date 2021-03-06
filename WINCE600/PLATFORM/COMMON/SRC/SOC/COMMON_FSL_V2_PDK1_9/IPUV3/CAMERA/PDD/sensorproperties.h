//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#ifndef __SENSORPROPERTIES_H
#define __SENSORPROPERTIES_H

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG BrightnessRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        750,                // Minimum in (IRE * 100) units
        750                 // Maximum in (IRE * 100) units
    }
};

const static LONG BrightnessDefault = 750;

static CSPROPERTY_MEMBERSLIST BrightnessMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),       /*MembersSize*/
            SIZEOF_ARRAY (BrightnessRangeAndStep),   /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) BrightnessRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (BrightnessDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &BrightnessDefault,
    }    
};

static CSPROPERTY_VALUES BrightnessValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (BrightnessMembersList),
    BrightnessMembersList
};

// ------------------------------------------------------------------------
// The contrast value is expressed as a gain factor multiplied by 100. 
static CSPROPERTY_STEPPING_LONG ContrastRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        100,                // Minimum as a gain factor multiplied by 100
        100                 // Maximum as a gain factor multiplied by 100
    }
};

const static LONG ContrastDefault = 100;

static CSPROPERTY_MEMBERSLIST ContrastMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),       /*MembersSize*/
            SIZEOF_ARRAY (ContrastRangeAndStep),     /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) ContrastRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (ContrastDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ContrastDefault,
    }    
};

static CSPROPERTY_VALUES ContrastValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ContrastMembersList),
    ContrastMembersList
};

// ------------------------------------------------------------------------
// The value of the hue setting is expressed in degrees multiplied by 100. 
static CSPROPERTY_STEPPING_LONG HueRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum in degrees multiplied by 100. 
        0                   // Maximum in degrees multiplied by 100. 
    }
};

const static LONG HueDefault = 0;

static CSPROPERTY_MEMBERSLIST HueMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (HueRangeAndStep),
            0
        },
        (PVOID) HueRangeAndStep
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (HueDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &HueDefault,
    }    
};

static CSPROPERTY_VALUES HueValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (HueMembersList),
    HueMembersList
};

// ------------------------------------------------------------------------
// The value of the saturation setting is expressed as gain multiplied by 100.

static CSPROPERTY_STEPPING_LONG SaturationRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        100,                // Minimum in (gain * 100) units
        100                 // Maximum in (gain * 100) units
    }
};

const static LONG SaturationDefault = 100;

static CSPROPERTY_MEMBERSLIST SaturationMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (SaturationRangeAndStep),
            0
        },
        (PVOID) SaturationRangeAndStep
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (SaturationDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &SaturationDefault,
    }    
};

static CSPROPERTY_VALUES SaturationValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SaturationMembersList),
    SaturationMembersList
};

// ------------------------------------------------------------------------
// Sharpness is expressed in arbitrary units
static CSPROPERTY_STEPPING_LONG SharpnessRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum
        100                 // Maximum
    }
};

const static LONG SharpnessDefault = 50;

static CSPROPERTY_MEMBERSLIST SharpnessMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (SharpnessRangeAndStep),
            0
        },
        (PVOID) SharpnessRangeAndStep
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (SharpnessDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &SharpnessDefault,
    }    
};

static CSPROPERTY_VALUES SharpnessValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SharpnessMembersList),
    SharpnessMembersList
};

// ------------------------------------------------------------------------
// The white balance value is expressed as a color temperature, in degrees Kelvin
static CSPROPERTY_STEPPING_LONG WhiteBalanceRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        273,                // Minimum in (degrees Kelvin)
        310                 // Maximum in degrees Kelvin)
    }
};

const static LONG WhiteBalanceDefault = 298;

static CSPROPERTY_MEMBERSLIST WhiteBalanceMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (WhiteBalanceRangeAndStep),
            0
        },
        (PVOID) WhiteBalanceRangeAndStep
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (WhiteBalanceDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WhiteBalanceDefault,
    }    
};

static CSPROPERTY_VALUES WhiteBalanceValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (WhiteBalanceMembersList),
    WhiteBalanceMembersList
};

// ------------------------------------------------------------------------
// The value of the gamma setting is expressed in gamma multiplied by 100
static CSPROPERTY_STEPPING_LONG GammaRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        100,                // Minimum (in gamma multiplied by 100)
        100                 // Maximum (in gamma multiplied by 100)
    }
};

const static LONG GammaDefault = 100; // gamma = 1

static CSPROPERTY_MEMBERSLIST GammaMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),       /*MembersSize*/
            SIZEOF_ARRAY (GammaRangeAndStep),        /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) GammaRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (GammaDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &GammaDefault,
    }    
};

static CSPROPERTY_VALUES GammaValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (GammaMembersList),
    GammaMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG ColorEnableRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum (Color is disabled)
        1                   // Maximum (Color is enabled)
    }
};

const static LONG ColorEnableDefault = 1;

static CSPROPERTY_MEMBERSLIST ColorEnableMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),       /*MembersSize*/
            SIZEOF_ARRAY (ColorEnableRangeAndStep),  /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) ColorEnableRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (ColorEnableDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ColorEnableDefault,
    }    
};

static CSPROPERTY_VALUES ColorEnableValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ColorEnableMembersList),
    ColorEnableMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG BackLightCompensationRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum ( BackLight Compensation is off
        1                   // Maximum ( BackLight Compensation is on )
    }
};

const static LONG BackLightCompensationDefault = 1;

static CSPROPERTY_MEMBERSLIST BackLightCompensationMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),        /*MembersSize*/
            SIZEOF_ARRAY (BackLightCompensationRangeAndStep),    /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) BackLightCompensationRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (BackLightCompensationDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &BackLightCompensationDefault,
    }    
};

static CSPROPERTY_VALUES BackLightCompensationValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (BackLightCompensationMembersList),
    BackLightCompensationMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG GainRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        1,                  // Minimum 
        1                   // Maximum 
    }
};

const static LONG GainDefault = 1;

static CSPROPERTY_MEMBERSLIST GainMembersList [] = 
{
    {
        /*CSPROPERTY_MEMBERSHEADER*/
        {    
            CSPROPERTY_MEMBER_RANGES,                /*MembersFlags*/
            sizeof (CSPROPERTY_STEPPING_LONG),       /*MembersSize*/
            SIZEOF_ARRAY (GainRangeAndStep),         /*MembersCount*/
            0                                        /*flags 0 or CSPROPERTY_MEMBER_FLAG_DEFAULT*/
        },
        /*Members*/
        (PVOID) GainRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (GainDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &GainDefault,
    }    
};

static CSPROPERTY_VALUES GainValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (GainMembersList),
    GainMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG PanRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum in Degrees
        0                   // Maximum in Degrees
    }
};

const static LONG PanDefault = 0;

static CSPROPERTY_MEMBERSLIST PanMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (PanRangeAndStep),
            0
        },
        (PVOID) PanRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (PanDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &PanDefault,
    }    
};

static CSPROPERTY_VALUES PanValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (PanMembersList),
    PanMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG TiltRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        -180,               // Minimum in Degrees
        180                 // Maximum in Degrees
    }
};

const static LONG TiltDefault = 0;

static CSPROPERTY_MEMBERSLIST TiltMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (TiltRangeAndStep),
            0
        },
        (PVOID) TiltRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (TiltDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &TiltDefault,
    }    
};

static CSPROPERTY_VALUES TiltValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (TiltMembersList),
    TiltMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG RollRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum in Degrees
        0                   // Maximum in Degrees
    }
};

const static LONG RollDefault = 0;

static CSPROPERTY_MEMBERSLIST RollMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (RollRangeAndStep),
            0
        },
        (PVOID) RollRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (RollDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &RollDefault,
    }    
};

static CSPROPERTY_VALUES RollValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (RollMembersList),
    RollMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG ZoomRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        1,                  // Minimum in Millimeters
        10                  // Maximum in Millimeters 
    }
};

const static LONG ZoomDefault = 1;

static CSPROPERTY_MEMBERSLIST ZoomMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (ZoomRangeAndStep),
            0
        },
        (PVOID) ZoomRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (ZoomDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ZoomDefault,
    }    
};

static CSPROPERTY_VALUES ZoomValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ZoomMembersList),
    ZoomMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG IrisRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        2500,                  // Minimum in units of fstop * 10
        2500                // Maximum in units of fstop * 10
    }
};

const static LONG IrisDefault = 2500;

static CSPROPERTY_MEMBERSLIST IrisMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (IrisRangeAndStep),
            0
        },
        (PVOID) IrisRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (IrisDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &IrisDefault,
    }    
};

static CSPROPERTY_VALUES IrisValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (IrisMembersList),
    IrisMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG ExposureRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        -9,                 // Minimum (This value is expressed in log base 2 seconds, thus, 
                            //         for values less than zero, the exposure time is 1/2n seconds. For positive values and zero, the exposure time is 2n seconds)
        4                   // Maximum (This value is expressed in log base 2 seconds, thus, 
                            //         for values less than zero, the exposure time is 1/2n seconds. For positive values and zero, the exposure time is 2n seconds)
    }
};

const static LONG ExposureDefault = -4;

static CSPROPERTY_MEMBERSLIST ExposureMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (ExposureRangeAndStep),
            0
        },
        (PVOID) ExposureRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (ExposureDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ExposureDefault,
    }    
};

static CSPROPERTY_VALUES ExposureValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ExposureMembersList),
    ExposureMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG FocusRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        100,                // Minimum in millimeters
        100                 // Maximum in millimeters
    }
};

const static LONG FocusDefault = 100;

static CSPROPERTY_MEMBERSLIST FocusMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (FocusRangeAndStep),
            0
        },
        (PVOID) FocusRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (FocusDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &FocusDefault,
    }    
};

static CSPROPERTY_VALUES FocusValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (FocusMembersList),
    FocusMembersList
};

// ------------------------------------------------------------------------
static CSPROPERTY_STEPPING_LONG FlashRangeAndStep [] = 
{
    {
        1,                  // SteppingDelta (range / steps)
        0,                  // Reserved
        0,                  // Minimum (means flash is turned off)
        0                   // Maximum (in millimeter units. Values greater than 0 means 
                            // flash is turned on and the value reflects the distance of the object from the camera.
    }
};

const static LONG FlashDefault = 0; // By default flash is turned off

static CSPROPERTY_MEMBERSLIST FlashMembersList [] = 
{
    {
        {
            CSPROPERTY_MEMBER_RANGES,
            sizeof (CSPROPERTY_STEPPING_LONG),
            SIZEOF_ARRAY (FlashRangeAndStep),
            0
        },
        (PVOID) FlashRangeAndStep,
     },
     {
        {
            CSPROPERTY_MEMBER_VALUES,
            sizeof (FlashDefault),
            1,
            CSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &FlashDefault,
    }    
};

static CSPROPERTY_VALUES FlashValues =
{
    {
        STATICGUIDOF (CSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (FlashMembersList),
    FlashMembersList
};


// ------------------------------------------------------------------------

DEFINE_CSPROPERTY_TABLE(VideoProcAmpProperties)
{
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &BrightnessValues,                      // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_CONTRAST,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &ContrastValues,                        // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_HUE,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &HueValues,                             // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),    

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_SATURATION,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &SaturationValues,                      // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_SHARPNESS,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &SharpnessValues,                       // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_GAMMA,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &GammaValues,                           // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_COLORENABLE,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &ColorEnableValues,                     // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_WHITEBALANCE,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &WhiteBalanceValues,                    // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &BackLightCompensationValues,           // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_VIDEOPROCAMP_GAIN,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(CSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &GainValues,                            // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    )

};

DEFINE_CSPROPERTY_TABLE(CameraControlProperties)
{
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_PAN,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &PanValues,                             // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_TILT,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &TiltValues,                            // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_ROLL,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &RollValues,                            // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_ZOOM,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &ZoomValues,                            // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_IRIS,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &IrisValues,                            // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_EXPOSURE,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &ExposureValues,                        // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_FOCUS,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &FocusValues,                           // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_CSPROPERTY_ITEM
    (
        CSPROPERTY_CAMERACONTROL_FLASH,
        TRUE,                                   // GetSupported or Handler
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(CSPROPERTY_CAMERACONTROL_S),     // MinData
        TRUE,                                   // SetSupported or Handler
        &FlashValues,                           // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    )
};


// ------------------------------------------------------------------------
// Array of all of the property sets supported by the adapter
// ------------------------------------------------------------------------

DEFINE_CSPROPERTY_SET_TABLE(AdapterPropertyTable)
{
    DEFINE_CSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEOPROCAMP,
        SIZEOF_ARRAY(VideoProcAmpProperties),
        VideoProcAmpProperties,
        0, 
        NULL
    ),

    DEFINE_CSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_CAMERACONTROL,
        SIZEOF_ARRAY(CameraControlProperties),
        CameraControlProperties,
        0, 
        NULL
    )
};

#define NUMBER_OF_ADAPTER_PROPERTY_SETS (SIZEOF_ARRAY (AdapterPropertyTable))

// ----------------------------------------------------------------------------
// Default Video Control Caps 
// ----------------------------------------------------------------------------
ULONG DefaultVideoControlCaps[] = {
    0x0                               /*CAPTURE*/,
    CS_VideoControlFlag_ExternalTriggerEnable | CS_VideoControlFlag_Trigger /*STILL*/,
    0x0                              /*PREVIEW*/
    };


#endif
