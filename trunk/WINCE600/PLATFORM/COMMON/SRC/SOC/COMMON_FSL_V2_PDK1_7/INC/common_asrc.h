//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_asrc.h
//
//  Provides definitions for the ASRC module
//  that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------
#ifndef __COMMON_ASRC_H
#define __COMMON_ASRC_H

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
    UINT32 ASRCTR;  //0x00
    UINT32 ASRIER;
    UINT32 RESERV;
    UINT32 ASRCNCR;
    UINT32 ASRCFG;  //0x10
    UINT32 ASRCSR;
    UINT32 ASRCDR1;
    UINT32 ASRCDR2;
    UINT32 ASRSTR;  //0x20
    UINT32 ASRRA;
    UINT32 ASRRB;
    UINT32 ASRRC;
    UINT32 ASRMAA;  //0X30
    UINT32 ASRMAD;
    UINT32 ASRDCR;
    UINT32 ASRDCR1;
    UINT32 ASRPM1;  //0X40
    UINT32 ASRPM2;
    UINT32 ASRPM3;
    UINT32 ASRPM4;
    UINT32 ASRPM5;  //0x50
    UINT32 ASRTFR1;
    UINT32 ASRTFR2;
    UINT32 ASRCCR;
    UINT32 ASRDIA;  //0x60
    UINT32 ASRDOA;
    UINT32 ASRDIB;
    UINT32 ASRDOB;
    UINT32 ASRDIC;  //0X70
    UINT32 ASRDOC;
    UINT32 RESERV2;
    UINT32 RESERV3;
    UINT32 ASRIDRHA;  //0X80
    UINT32 ASRIDRLA;
    UINT32 ASRIDRHB;
    UINT32 ASRIDRLB;
    UINT32 ASRIDRHC;  //0X90
    UINT32 ASRIDRLC;
    UINT32 ASR76K;
    UINT32 ASR56K;    
} CSP_ASRC_REG, *PCSP_ASRC_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define ASRC_ASRCTR_OFFSET 0x00
#define ASRC_ASRIER_OFFSET 0x04
#define ASRC_RESERV_OFFSET 0x08
#define ASRC_ASRCNCR_OFFSET 0x0C
#define ASRC_ASRCFG_OFFSET 0x10
#define ASRC_ASRCSR_OFFSET 0x1C
#define ASRC_ASRCDR1_OFFSET 0x18
#define ASRC_ASRCDR2_OFFSET 0x1C
#define ASRC_ASRSTR_OFFSET 0x20
#define ASRC_ASRRA_OFFSET 0x24
#define ASRC_ASRRB_OFFSET 0x28
#define ASRC_ASRRC_OFFSET 0x2C
#define ASRC_ASRMAA_OFFSET 0x30
#define ASRC_ASRMAD_OFFSET 0x34
#define ASRC_ASRDCR_OFFSET 0x38
#define ASRC_ASRDCR1_OFFSET 0x3C
#define ASRC_ASRPM1_OFFSET 0x40
#define ASRC_ASRPM2_OFFSET 0x44
#define ASRC_ASRPM3_OFFSET 0x48
#define ASRC_ASRPM4_OFFSET 0x4C
#define ASRC_ASRPM5_OFFSET 0x50
#define ASRC_ASRTFR1_OFFSET 0x54
#define ASRC_ASRTFR2_OFFSET 0x58
#define ASRC_ASRCCR_OFFSET 0x5C
#define ASRC_ASRDIA_OFFSET 0x60
#define ASRC_ASRDOA_OFFSET 0x64
#define ASRC_ASRDIB_OFFSET 0x68
#define ASRC_ASRDOB_OFFSET 0x6C
#define ASRC_ASRDIC_OFFSET 0x70   //0X70
#define ASRC_ASRDOC_OFFSET 0x74
#define ASRC_RESERV2_OFFSET 0X78
#define ASRC_ESERV3_OFFSET 0x7C
#define ASRC_ASRIDRHA_OFFSET 0x80  //0X80
#define ASRC_ASRIDRLA_OFFSET 0x84
#define ASRC_ASRIDRHB_OFFSET 0x88
#define ASRC_ASRIDRLB_OFFSET 0x8C
#define ASRC_ASRIDRHC_OFFSET 0x90  //0X90
#define ASRC_ASRIDRLC_OFFSET 0x94
#define ASRC_ASR76K_OFFSET 0x98
#define ASRC_ASR56K_OFFSET 0x9C


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define ASRCTR_ASRCEN_LSH 0
#define ASRCTR_ASREA_LSH 1
#define ASRCTR_ASREB_LSH 2
#define ASRCTR_ASREC_LSH 3
#define ASRCTR_IDRA_LSH 13
#define ASRCTR_USRA_LSH 14
#define ASRCTR_IDRB_LSH 15
#define ASRCTR_USRB_LSH 16
#define ASRCTR_IDRC_LSH 17
#define ASRCTR_USRC_LSH 18
#define ASRCTR_ATSA_LSH 20
#define ASRCTR_ATSB_LSH 21
#define ASRCTR_ATSC_LSH 22


#define ASRCTR_ASRCEN_WID 1
#define ASRCTR_ASREA_WID  1
#define ASRCTR_ASREB_WID  1
#define ASRCTR_ASREC_WID  1
#define ASRCTR_IDRA_WID 1
#define ASRCTR_USRA_WID 1
#define ASRCTR_IDRB_WID 1
#define ASRCTR_USRB_WID 1
#define ASRCTR_IDRC_WID 1
#define ASRCTR_USRC_WID 1
#define ASRCTR_ATSA_WID 1
#define ASRCTR_ATSB_WID 1
#define ASRCTR_ATSC_WID 1

#define ASRIER_ADIEA_LSH  0
#define ASRIER_ADIEB_LSH  1
#define ASRIER_ADIEC_LSH  2
#define ASRIER_ADOEA_LSH  3
#define ASRIER_ADOEB_LSH  4
#define ASRIER_ADOEC_LSH  5
#define ASRIER_AOLIE_LSH  6
#define ASRIER_AFPWE_LSH  7

#define ASRIER_ADIEA_WID  1
#define ASRIER_ADIEB_WID  1
#define ASRIER_ADIEC_WID  1
#define ASRIER_ADOEA_WID  1
#define ASRIER_ADOEB_WID  1
#define ASRIER_ADOEC_WID  1
#define ASRIER_AOLIE_WID  1
#define ASRIER_AFPWE_WID  1

#define ASRCNCR_V1_ANCA_LSH  0
#define ASRCNCR_V1_ANCB_LSH  3
#define ASRCNCR_V1_ANCC_LSH  6

#define ASRCNCR_V1_ANCA_WID  3
#define ASRCNCR_V1_ANCB_WID  3
#define ASRCNCR_V1_ANCC_WID  3

//New ASRC extended the bit width to 4 bits
#define ASRCNCR_V2_ANCA_LSH  0
#define ASRCNCR_V2_ANCB_LSH  4
#define ASRCNCR_V2_ANCC_LSH  8

#define ASRCNCR_V2_ANCA_WID  4
#define ASRCNCR_V2_ANCB_WID  4
#define ASRCNCR_V2_ANCC_WID  4

#define ASRCFG_PREMODA_LSH 6
#define ASRCFG_POSTMODA_LSH 8
#define ASRCFG_PREMODB_LSH 10
#define ASRCFG_POSTMODB_LSH 12
#define ASRCFG_PREMODC_LSH 14
#define ASRCFG_POSTMODC_LSH 16
#define ASRCFG_NDPRA_LSH 18 
#define ASRCFG_NDPRB_LSH 19
#define ASRCFG_NDPRC_LSH 20
#define ASRCFG_INIRQA_LSH 21
#define ASRCFG_INIRQB_LSH 22
#define ASRCFG_INIRQC_LSH 23

#define ASRCFG_PREMODA_WID 2
#define ASRCFG_POSTMODA_WID 2
#define ASRCFG_PREMODB_WID 2
#define ASRCFG_POSTMODB_WID 2
#define ASRCFG_PREMODC_WID 2
#define ASRCFG_POSTMODC_WID 2
#define ASRCFG_NDPRA_WID 1
#define ASRCFG_NDPRB_WID 1
#define ASRCFG_NDPRC_WID 1
#define ASRCFG_INIRQA_WID 1
#define ASRCFG_INIRQB_WID 1
#define ASRCFG_INIRQC_WID 1


#define ASRCSR_AOCSC_LSH 20
#define ASRCSR_AOCSB_LSH 16
#define ASRCSR_AOCSA_LSH 12
#define ASRCSR_AICSC_LSH 8
#define ASRCSR_AICSB_LSH 4
#define ASRCSR_AICSA_LSH 0

#define ASRCSR_AOCSC_WID 4
#define ASRCSR_AOCSB_WID 4
#define ASRCSR_AOCSA_WID 4
#define ASRCSR_AICSC_WID 4
#define ASRCSR_AICSB_WID 4
#define ASRCSR_AICSA_WID 4

#define ASRCDR1_AICPA_LSH 0
#define ASRCDR1_AICDA_LSH 3
#define ASRCDR1_AICPB_LSH 6
#define ASRCDR1_AICDB_LSH 9
#define ASRCDR1_AOCPA_LSH 12
#define ASRCDR1_AOCDA_LSH 15
#define ASRCDR1_AOCPB_LSH 18
#define ASRCDR1_AOCDB_LSH 21

#define ASRCDR1_AICPA_WID 3
#define ASRCDR1_AICDA_WID 3
#define ASRCDR1_AICPB_WID 3
#define ASRCDR1_AICDB_WID 3
#define ASRCDR1_AOCPA_WID 3
#define ASRCDR1_AOCDA_WID 3
#define ASRCDR1_AOCPB_WID 3
#define ASRCDR1_AOCDB_WID 3

#define ASRCDR2_AICPC_LSH 0
#define ASRCDR2_AICDC_LSH 3
#define ASRCDR2_AOCPC_LSH 6
#define ASRCDR2_AOCDC_LSH 9

#define ASRCDR2_AICPC_WID 3
#define ASRCDR2_AICDC_WID 3
#define ASRCDR2_AOCPC_WID 3
#define ASRCDR2_AOCDC_WID 3

#define ASRSTR_AIDEA_LSH 0
#define ASRSTR_AIDEB_LSH 1
#define ASRSTR_AIDEC_LSH 2
#define ASRSTR_AODFA_LSH 3
#define ASRSTR_AODFB_LSH 4
#define ASRSTR_AODFC_LSH 5
#define ASRSTR_AOLE_LSH 6
#define ASRSTR_FPWT_LSH 7
#define ASRSTR_AIDUA_LSH 8
#define ASRSTR_AIDUB_LSH 9
#define ASRSTR_AIDUC_LSH 10
#define ASRSTR_AODOA_LSH 11
#define ASRSTR_AODOB_LSH 12
#define ASRSTR_AODOC_LSH 13
#define ASRSTR_AIOLA_LSH 14
#define ASRSTR_AIOLB_LSH 15
#define ASRSTR_AIOLC_LSH 16
#define ASRSTR_AOOLA_LSH 17
#define ASRSTR_AOOLB_LSH 18
#define ASRSTR_AOOLC_LSH 19
#define ASRSTR_ATQOL_LSH 20
#define ASRSTR_DSLCNT_LSH 21

#define ASRSTR_AIDEA_WID 1
#define ASRSTR_AIDEB_WID 1
#define ASRSTR_AIDEA_WID 1
#define ASRSTR_AIDEB_WID 1
#define ASRSTR_AIDEC_WID 1
#define ASRSTR_AODFA_WID 1
#define ASRSTR_AODFB_WID 1
#define ASRSTR_AODFC_WID 1
#define ASRSTR_AOLE_WID 1
#define ASRSTR_FPWT_WID 1
#define ASRSTR_AIDUA_WID 1
#define ASRSTR_AIDUB_WID 1
#define ASRSTR_AIDUC_WID 1
#define ASRSTR_AODOA_WID 1
#define ASRSTR_AODOB_WID 1
#define ASRSTR_AODOC_WID 1
#define ASRSTR_AIOLA_WID 1
#define ASRSTR_AIOLB_WID 1
#define ASRSTR_AIOLC_WID 1
#define ASRSTR_AOOLA_WID 1
#define ASRSTR_AOOLB_WID 1
#define ASRSTR_AOOLC_WID 1
#define ASRSTR_ATQOL_WID 1
#define ASRSTR_DSLCNT_WID 1
#ifdef __cplusplus
}
#endif

#endif // __COMMON_ASRC_H
