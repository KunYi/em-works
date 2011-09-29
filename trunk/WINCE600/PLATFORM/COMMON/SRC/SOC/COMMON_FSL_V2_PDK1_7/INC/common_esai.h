//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_esai.h
//
//  Provides definitions for the ESAI module
//  that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------
#ifndef __COMMON_ESAI_H
#define __COMMON_ESAI_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 ETDR;    //0x00
    UINT32 ERDR;    //0x04
    UINT32 ECR;     //0x08
    UINT32 ESR;     //0x0c
    UINT32 TFCR;    //0x10
    UINT32 TFSR;    //0x14
    UINT32 RFCR;    //0x18
    UINT32 RFSR;    //0x1c
    UINT32 PAD0[24];//0X20 ~ 0X7C reserved
    UINT32 TX0;     // 0x80
    UINT32 TX1;     // 0x84
    UINT32 TX2;     // 0x88
    UINT32 TX3;     // 0x8c
    UINT32 TX4;     // 0x90
    UINT32 TX5;     // 0x94
    UINT32 TSR;     // 0x98
    UINT32 PAD1;    //0x9c reserved
    UINT32 RX0;     // 0xA0
    UINT32 RX1;     // 0xA4
    UINT32 RX2;     //  0xA8
    UINT32 RX3;     //  0xAC
    UINT32 PAD2[7]; //0xb0 ~0xc8 reserved      
    UINT32 SAISR;   // 0xCC
    UINT32 SAICR;   //  0xD0
    UINT32 TCR;     //  0xD4
    UINT32 TCCR;    // 0xD8
    UINT32 RCR;     //  0xDC
    UINT32 RCCR;    //  0xE0
    UINT32 TSMA;    //  0xE4
    UINT32 TSMB;    //  0xE8
    UINT32 RSMA;    //  0xEC
    UINT32 RSMB;    //  0xF0
    UINT32 PDRC;    //  0xF4
    UINT32 PRRC;    //  0xF8
    UINT32 PCRC;    //  0xFC  
} CSP_ESAI_REG, *PCSP_ESAI_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define ESAI_ETDR_OFFSET           0x00
#define ESAI_ERDR_OFFSET           0x04
#define ESAI_ECR_OFFSET            0x08
#define ESAI_ESR_OFFSET            0x0c
#define ESAI_TFCR_OFFSET           0x10
#define ESAI_TFSR_OFFSET           0x14
#define ESAI_RFCR_OFFSET           0x18
#define ESAI_RFSR_OFFSET           0x1c
#define ESAI_TX0_OFFSET            0x80
#define ESAI_TX1_OFFSET            0x84
#define ESAI_TX2_OFFSET            0x88
#define ESAI_TX3_OFFSET            0x8c
#define ESAI_TX4_OFFSET            0x90
#define ESAI_TX5_OFFSET            0x94
#define ESAI_TSR_OFFSET            0x98
#define ESAI_RX0_OFFSET            0xA0
#define ESAI_RX1_OFFSET            0xA4
#define ESAI_RX2_OFFSET            0xA8
#define ESAI_RX3_OFFSET            0xAC
#define ESAI_SAISR_OFFSET          0xCC
#define ESAI_SAICR_OFFSET          0xD0
#define ESAI_TCR_OFFSET            0xD4
#define ESAI_TCCR_OFFSET           0xD8
#define ESAI_RCR_OFFSET            0xDC
#define ESAI_RCCR_OFFSET           0xE0
#define ESAI_TSMA_OFFSET           0xE4
#define ESAI_TSMB_OFFSET           0xE8
#define ESAI_RSMA_OFFSET           0xEC
#define ESAI_RSMB_OFFSET           0xF0
#define ESAI_PDRC_OFFSET           0xF4
#define ESAI_PRRC_OFFSET           0xF8
#define ESAI_PCRC_OFFSET           0xFC


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//
#define ECR_ESAIEN_LSH 0
#define ECR_ERST_LSH   1
#define ECR_ERO_LSH    16
#define ECR_ERI_LSH    17
#define ECR_ETO_LSH    18
#define ECR_ETI_LSH    19

#define ECR_ESAIEN_WID 1
#define ECR_ERST_WID   1
#define ECR_ERO_WID    1
#define ECR_ERI_WID    1
#define ECR_ETO_WID    1
#define ECR_ETI_WID    1


#define ESR_RD_LSH     0
#define ESR_RED_LSH    1
#define ESR_RDE_LSH    2
#define ESR_RLS_LSH    3
#define ESR_TD_LSH     4 
#define ESR_TED_LSH    5
#define ESR_TDE_LSH    6
#define ESR_TLS_LSH    7 
#define ESR_TFE_LSH    8
#define ESR_RFF_LSH    9
#define ESR_TINIT_LSH  10

#define ESR_RD_WID     1
#define ESR_RED_WID    1
#define ESR_RDE_WID    1
#define ESR_RLS_WID    1
#define ESR_TD_WID     1
#define ESR_TED_WID    1
#define ESR_TDE_WID    1
#define ESR_TLS_WID    1 
#define ESR_TFE_WID    1
#define ESR_RFF_WID    1
#define ESR_TINIT_WID  1

//Transmit fifo configuration reg
#define TFCR_TFE_LSH    0
#define TFCR_TFR_LSH    1
#define TFCR_TE0_LSH    2
#define TFCR_TE1_LSH    3
#define TFCR_TE2_LSH    4
#define TFCR_TE3_LSH    5
#define TFCR_TE4_LSH    6
#define TFCR_TE5_LSH    7
#define TFCR_TFWM_LSH   8
#define TFCR_TWA_LSH    16
#define TFCR_TIEN_LSH   19

#define TFCR_TFE_WID    1
#define TFCR_TFR_WID    1
#define TFCR_TE0_WID    1
#define TFCR_TE1_WID    1
#define TFCR_TE2_WID    1
#define TFCR_TE4_WID    1
#define TFCR_TE5_WID    1
#define TFCR_TFWM_WID   8
#define TFCR_TWA_WID    3
#define TFCR_TIEN_WID   1

//Transmit fifo status reg
#define TFSR_TFCNT_LSH  0
#define TFSR_NTFI_LSH   8
#define TFSR_NTFO_LSH   12

#define TFSR_TFCNT_WID  8
#define TFSR_NTFI_WID   3
#define TFSR_NTFO_WID   3

//Receive fifo configure reg
#define RFCR_RFE_LSH    0
#define RFCR_RFR_LSH    1
#define RFCR_RE0_LSH    2
#define RFCR_RE1_LSH    3
#define RFCR_RE2_LSH    4
#define RFCR_RE3_LSH    5
#define RFCR_RFWM_LSH   8
#define RFCR_RWA_LSH    16
#define RFCR_REXT_LSH   19

#define RFCR_RFE_WID    1
#define RFCR_RFR_WID    1
#define RFCR_RE0_WID    1
#define RFCR_RE1_WID    1
#define RFCR_RE2_WID    1
#define RFCR_RE3_WID    1
#define RFCR_RFWM_WID   8
#define RFCR_RWA_WID    3
#define RFCR_REXT_WID   1

//Receive FIFO status reg
#define RFSR_RFCNT_LSH  0
#define RFSR_NRFO_LSH   8
#define RFSR_NRFI_LSH   12

#define RFSR_TFCNT_WID  8
#define RFSR_NRFO_WID   3
#define RFSR_NRFI_WID   3

//esai status reg
#define SAISR_IF0_LSH   0
#define SAISR_IF1_LSH   1 
#define SAISR_IF2_LSH   2 
#define SAISR_RFS_LSH   6
#define SAISR_ROE_LSH   7
#define SAISR_RDF_LSH   8
#define SAISR_REDF_LSH  9 
#define SAISR_RODF_LSH  10
#define SAISR_TFS_LSH   13
#define SAISR_TUE_LSH   14
#define SAISR_TDE_LSH   15
#define SAISR_TEDE_LSH  16
#define SAISR_TODFE_LSH 17

#define SAISR_IF0_WID   1
#define SAISR_IF1_WID   1 
#define SAISR_IF2_WID   1 
#define SAISR_RFS_WID   1
#define SAISR_ROE_WID   1
#define SAISR_RDF_WID   1
#define SAISR_REDF_WID  1 
#define SAISR_RODF_WID  1
#define SAISR_TFS_WID   1
#define SAISR_TUE_WID   1
#define SAISR_TDE_WID   1
#define SAISR_TEDE_WID  1
#define SAISR_TODFE_WID 1


//ESAI Common control reg
#define SAICR_OF0_LSH  0
#define SAICR_OF1_LSH  1
#define SAICR_OF2_LSH  2
#define SAICR_SYN_LSH  6
#define SAICR_TEBE_LSH 7
#define SAICR_ALC_LSH  8


#define SAICR_OF0_WID  1
#define SAICR_OF1_WID  1
#define SAICR_OF2_WID  1
#define SAICR_SYN_WID  1
#define SAICR_TEBE_WID 1
#define SAICR_ALC_WID  1

//ESAI Transmit control reg
#define TCR_TE0_LSH  0
#define TCR_TE1_LSH  1
#define TCR_TE2_LSH  2 
#define TCR_TE3_LSH  3
#define TCR_TE4_LSH  4 
#define TCR_TE5_LSH  5
#define TCR_TSHFD_LSH 6
#define TCR_TWA_LSH  7
#define TCR_TMOD_LSH  8
#define TCR_TSWS_LSH  10 
#define TCR_TFSL_LSH  15
#define TCR_TFSR_LSH  16
#define TCR_PADC_LSH 17
#define TCR_TPR_LSH  19
#define TCR_TEIE_LSH  20
#define TCR_TDEIE_LSH 21 
#define TCR_TIE_LSH  22
#define TCR_TLIE_LSH 23

#define TCR_TE0_WID  1
#define TCR_TE1_WID  1
#define TCR_TE2_WID  1 
#define TCR_TE3_WID  1
#define TCR_TE4_WID  1 
#define TCR_TE5_WID  1
#define TCR_TSHFD_WID 1
#define TCR_TWA_WID  1
#define TCR_TMOD_WID  2
#define TCR_TSWS_WID  5 
#define TCR_TFSL_WID  1
#define TCR_TFSR_WID  1
#define TCR_PADC_WID 1
#define TCR_TPR_WID  1
#define TCR_TEIE_WID  1
#define TCR_TDEIE_WID 1 
#define TCR_TIE_WID  1
#define TCR_TLIE_WID 1

//esai transmit clock control reg
#define TCCR_TPM_LSH 0
#define TCCR_TPSR_LSH 8
#define TCCR_TDC_LSH 9
#define TCCR_TFP_LSH 14
#define TCCR_TCKP_LSH 18
#define TCCR_TFSP_LSH 19
#define TCCR_THCKP_LSH 20
#define TCCR_TCKD_LSH 21
#define TCCR_TFSD_LSH 22
#define TCCR_THCKD_LSH 23

#define TCCR_TPM_WID 8
#define TCCR_TPSR_WID 1
#define TCCR_TDC_WID 5
#define TCCR_TFP_WID 4
#define TCCR_TCKP_WID 1
#define TCCR_TFSP_WID 1
#define TCCR_THCKP_WID 1
#define TCCR_TCKD_WID 1
#define TCCR_TFSD_WID 1
#define TCCR_THCKD_WID 1

//ESAI Receive control reg
#define RCR_RE0_LSH 0
#define RCR_RE1_LSH 1
#define RCR_RE2_LSH 2
#define RCR_RE3_LSH 3
#define RCR_RSHFD_LSH 6
#define RCR_RWA_LSH 7
#define RCR_RMOD_LSH 8
#define RCR_RSWS_LSH 10
#define RCR_RFSL_LSH 15
#define RCR_RFSR_LSH 16
#define RCR_RPR_LSH 19
#define RCR_REIE_LSH 20
#define RCR_RDEIE_LSH 21
#define RCR_RIE_LSH 22
#define RCR_RLIE_LSH 23

#define RCR_RE0_WID 1
#define RCR_RE1_WID 1
#define RCR_RE2_WID 1
#define RCR_RE3_WID 1
#define RCR_RSHFD_WID 1
#define RCR_RWA_WID 1
#define RCR_RMOD_WID 2
#define RCR_RSWS_WID 5
#define RCR_RFSL_WID 1
#define RCR_RFSR_WID 1
#define RCR_RPR_WID  1
#define RCR_REIE_WID 1
#define RCR_RDEIE_WID 1
#define RCR_RIE_WID 1
#define RCR_RLIE_WID 1

//esai receive clock control reg
#define RCCR_RPM_LSH 0
#define RCCR_RPSR_LSH 8
#define RCCR_RDC_LSH 9
#define RCCR_RFP_LSH 14
#define RCCR_RCKP_LSH 18
#define RCCR_RFSP_LSH 19
#define RCCR_RHCKP_LSH 20
#define RCCR_RCKD_LSH 21
#define RCCR_RFSD_LSH 22
#define RCCR_RHCKD_LSH 23

#define RCCR_RPM_WID 8
#define RCCR_RPSR_WID 1
#define RCCR_RDC_WID 5
#define RCCR_RFP_WID 4
#define RCCR_RCKP_WID 1
#define RCCR_RFSP_WID 1
#define RCCR_RHCKP_WID 1
#define RCCR_RCKD_WID 1
#define RCCR_RFSD_WID 1
#define RCCR_RHCKD_WID 1


#ifdef __cplusplus
}
#endif

#endif // __COMMON_ESAI_H
