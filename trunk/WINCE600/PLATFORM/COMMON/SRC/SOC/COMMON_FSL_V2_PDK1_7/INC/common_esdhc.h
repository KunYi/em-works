//------------------------------------------------------------------------------
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Header: common_esdhc.h
//
//  Provides definitions for the Freescale eSDHC module.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_ESDHC_H
#define __COMMON_ESDHC_H

#if __cplusplus
extern "C" {
#endif

#include "common_types.h"

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------

typedef struct {

    REG32 DSADDR;                    // 0x00 DMA system address register
    REG32 BLKATTR;                   // 0x04 Block attributes register         
    REG32 CMDARG;                   // 0x08: Command argument register
    REG32 XFERTYP;                  // 0x0C: Transfer type register
    REG32 CMDRSP0;                  // 0x10: Command response 0 register
    REG32 CMDRSP1;                  // 0x14: Command response 1 register
    REG32 CMDRSP2;                  // 0x18: Command response 2 register
    REG32 CMDRSP3;                  // 0x1C: Command response 3 register
    REG32 DATPORT;                  // 0x20: Buffer data port register
    REG32 PRSSTAT;                  // 0x24: Present state register
    REG32 PROCTL;                   // 0x28: Protocol control register
    REG32 SYSCTL;                   // 0x2C: System control register
    REG32 IRQSTAT;                  // 0x30: Interrupt status register
    REG32 IRQSTATEN;                // 0x34: Interrupt status enable register
    REG32 IRQSIGEN;                 // 0x38: Interrupt signal enable register
    REG32 AUTOC12ERR;               // 0x3C: Auto CMD12 error status register
    REG32 HOSTCAPBLT;               // 0x40: Host controller capabilities register
    REG32 WML;                      // 0x44: Watermark level register
    REG32 pad0;                     // 0x48: resevered
    REG32 pad1;                     // 0x4c: resevered
    REG32 FEVT;                     // 0x50: Force event register
    REG32 ADMAES;                   // 0x54: ADMA error status register
    REG32 ADSADDR;                  // 0x58: ADMA system address register
    REG32 pad2[25];                   // 0x5c - 0xBC: reserved area
    REG32 VENDOR;                    // 0xc0: Vendor specific register
    REG32 pad3[14];                   // 0xc4 - 0xfc: reserved area
    REG32 HOSTVER;                  // 0xfc: Host controller version register
     
    } CSP_ESDHC_REG, *PCSP_ESDHC_REG;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define   ESDHC_DSADDR_OFFSET             (0x0000)
#define   ESDHC_BLKATTR_OFFSET            (0x0004)
#define   ESDHC_CMDARG_OFFSET             (0x0008)
#define   ESDHC_XFERTYP_OFFSET            (0x000C)
#define   ESDHC_CMDRSP0_OFFSET             (0x0010)
#define   ESDHC_CMDRSP1_OFFSET            (0x0014)
#define   ESDHC_CMDRSP2_OFFSET             (0x0018)
#define   ESDHC_CMDRSP3_OFFSET             (0x001C)
#define   ESDHC_DATPORT_OFFSET             (0x0020)
#define   ESDHC_PRSSTAT_OFFSET            (0x0024)
#define   ESDHC_PROCTL_OFFSET             (0x0028)
#define   ESDHC_SYSCTL_OFFSET            (0x002C)
#define   ESDHC_IRQSTAT_OFFSET            (0x0030)
#define   ESDHC_IRQSTATEN_OFFSET        (0x0034)
#define   ESDHC_IRQSIGEN_OFFSET          (0x0038)
#define   ESDHC_AUTOC12ERR_OFFSET        (0x003C)
#define   ESDHC_HOSTCAPBLT_OFFSET       (0x0040)
#define   ESDHC_WML_OFFSET              (0x0044)
#define   ESDHC_FEVT_OFFSET              (0x0050)
#define   ESDHC_ADMAES_OFFSET            (0x0054)
#define   ESDHC_ADSADDR_OFFSET          (0x0058)
#define   ESDHC_VENDOR_OFFSET            (0x00C0)
#define   ESDHC_HOSTVER_OFFSET          (0x00FC)

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//Block Attributes register
#define ESDHC_BLKATTR_BLKSIZE_LSH        0
#define ESDHC_BLKATTR_BLKCNT_LSH        16

#define ESDHC_BLKATTR_BLKSIZE_WID        13
#define ESDHC_BLKATTR_BLKCNT_WID        16

//Transfer Type register
#define ESDHC_XFERTYP_DMAEN_LSH            0
#define ESDHC_XFERTYP_BCEN_LSH                1
#define ESDHC_XFERTYP_AC12EN_LSH            2
#define ESDHC_XFERTYP_DTDSEL_LSH            4
#define ESDHC_XFERTYP_MSBSEL_LSH            5
#define ESDHC_XFERTYP_RSPTYP_LSH            16
#define ESDHC_XFERTYP_CCCEN_LSH            19
#define ESDHC_XFERTYP_CICEN_LSH            20
#define ESDHC_XFERTYP_DPSEL_LSH            21
#define ESDHC_XFERTYP_CMDTYP_LSH            22
#define ESDHC_XFERTYP_CMDINX_LSH            24

#define ESDHC_XFERTYP_DMAEN_WID            1
#define ESDHC_XFERTYP_BCEN_WID                1
#define ESDHC_XFERTYP_AC12EN_WID            1
#define ESDHC_XFERTYP_DTDSEL_WID            1
#define ESDHC_XFERTYP_MSBSEL_WID            1
#define ESDHC_XFERTYP_RSPTYP_WID            2
#define ESDHC_XFERTYP_CCCEN_WID            1
#define ESDHC_XFERTYP_CICEN_WID            1
#define ESDHC_XFERTYP_DPSEL_WID            1
#define ESDHC_XFERTYP_CMDTYP_WID            2
#define ESDHC_XFERTYP_CMDINX_WID            6

//Present State register
#define ESDHC_PRSSTAT_CIHB_LSH                0
#define ESDHC_PRSSTAT_CDIHB_LSH                1
#define ESDHC_PRSSTAT_DLA_LSH                2
#define ESDHC_PRSSTAT_SDSTB_LSH             3
#define ESDHC_PRSSTAT_IPGOFF_LSH            4
#define ESDHC_PRSSTAT_HCKOFF_LSH            5
#define ESDHC_PRSSTAT_PEROFF_LSH            6
#define ESDHC_PRSSTAT_SDOFF_LSH                7
#define ESDHC_PRSSTAT_WTA_LSH                8
#define ESDHC_PRSSTAT_RTA_LSH                9
#define ESDHC_PRSSTAT_BWEN_LSH                10
#define ESDHC_PRSSTAT_BREN_LSH                11
#define ESDHC_PRSSTAT_CINS_LSH                16
#define ESDHC_PRSSTAT_CDPL_LSH                18
#define ESDHC_PRSSTAT_WPSPL_LSH                19
#define ESDHC_PRSSTAT_CLSL_LSH                23
#define ESDHC_PRSSTAT_DLSL_LSH                24

#define ESDHC_PRSSTAT_CIHB_WID                1
#define ESDHC_PRSSTAT_CDIHB_WID                1
#define ESDHC_PRSSTAT_DLA_WID                1
#define ESDHC_PRSSTAT_SDSTB_WID             1
#define ESDHC_PRSSTAT_IPGOFF_WID            1
#define ESDHC_PRSSTAT_HCKOFF_WID            1
#define ESDHC_PRSSTAT_PEROFF_WID            1
#define ESDHC_PRSSTAT_SDOFF_WID                1
#define ESDHC_PRSSTAT_WTA_WID                1
#define ESDHC_PRSSTAT_RTA_WID                1
#define ESDHC_PRSSTAT_BWEN_WID                1
#define ESDHC_PRSSTAT_BREN_WID                1
#define ESDHC_PRSSTAT_CINS_WID                1
#define ESDHC_PRSSTAT_CDPL_WID                1
#define ESDHC_PRSSTAT_WPSPL_WID                1
#define ESDHC_PRSSTAT_CLSL_WID                1
#define ESDHC_PRSSTAT_DLSL_WID                8


//Protocol Control register
#define ESDHC_PROCTL_LCTL_LSH            0
#define ESDHC_PROCTL_DTW_LSH            1
#define ESDHC_PROCTL_D3CD_LSH            3
#define ESDHC_PROCTL_EMODE_LSH            4
#define ESDHC_PROCTL_CDTL_LSH            6
#define ESDHC_PROCTL_CDSS_LSH            7
#define ESDHC_PROCTL_DMAS_LSH            8
#define ESDHC_PROCTL_SABGREQ_LSH        16
#define ESDHC_PROCTL_CREQ_LSH            17
#define ESDHC_PROCTL_RWCTL_LSH            18
#define ESDHC_PROCTL_IABG_LSH            19
#define ESDHC_PROCTL_WECINT_LSH            24
#define ESDHC_PROCTL_WECINS_LSH            25
#define ESDHC_PROCTL_WECRM_LSH            26

#define ESDHC_PROCTL_LCTL_WID            1
#define ESDHC_PROCTL_DTW_WID            2
#define ESDHC_PROCTL_D3CD_WID            1
#define ESDHC_PROCTL_EMODE_WID            2
#define ESDHC_PROCTL_CDTL_WID            1
#define ESDHC_PROCTL_CDSS_WID            1
#define ESDHC_PROCTL_DMAS_WID            2
#define ESDHC_PROCTL_SABGREQ_WID        1
#define ESDHC_PROCTL_CREQ_WID            1
#define ESDHC_PROCTL_RWCTL_WID            1
#define ESDHC_PROCTL_IABG_WID            1
#define ESDHC_PROCTL_WECINT_WID            1
#define ESDHC_PROCTL_WECINS_WID            1
#define ESDHC_PROCTL_WECRM_WID            1

//System Control register
#define ESDHC_SYSCTL_IPGEN_LSH            0
#define ESDHC_SYSCTL_HCKEN_LSH            1
#define ESDHC_SYSCTL_PEREN_LSH            2
#define ESDHC_SYSCTL_SDCLKEN_LSH        3
#define ESDHC_SYSCTL_DVS_LSH            4
#define ESDHC_SYSCTL_SDCLKFS_LSH        8
#define ESDHC_SYSCTL_DTOCV_LSH            16
#define ESDHC_SYSCTL_RSTA_LSH            24
#define ESDHC_SYSCTL_RSTC_LSH            25
#define ESDHC_SYSCTL_RSTD_LSH            26
#define ESDHC_SYSCTL_INITA_LSH            27

#define ESDHC_SYSCTL_IPGEN_WID            1
#define ESDHC_SYSCTL_HCKEN_WID            1
#define ESDHC_SYSCTL_PEREN_WID            1
#define ESDHC_SYSCTL_SDCLKEN_WID        1
#define ESDHC_SYSCTL_DVS_WID            4
#define ESDHC_SYSCTL_SDCLKFS_WID        8
#define ESDHC_SYSCTL_DTOCV_WID            4
#define ESDHC_SYSCTL_RSTA_WID            1
#define ESDHC_SYSCTL_RSTC_WID            1
#define ESDHC_SYSCTL_RSTD_WID            1
#define ESDHC_SYSCTL_INITA_WID            1

//Interrupt Status register
#define ESDHC_IRQSTAT_CC_LSH            0
#define ESDHC_IRQSTAT_TC_LSH            1
#define ESDHC_IRQSTAT_BGE_LSH            2
#define ESDHC_IRQSTAT_DINT_LSH            3
#define ESDHC_IRQSTAT_BWR_LSH            4
#define ESDHC_IRQSTAT_BRR_LSH            5
#define ESDHC_IRQSTAT_CINS_LSH            6
#define ESDHC_IRQSTAT_CRM_LSH            7
#define ESDHC_IRQSTAT_CINT_LSH            8
#define ESDHC_IRQSTAT_CTOE_LSH            16
#define ESDHC_IRQSTAT_CCE_LSH            17
#define ESDHC_IRQSTAT_CEBE_LSH            18
#define ESDHC_IRQSTAT_CIE_LSH            19
#define ESDHC_IRQSTAT_DTOE_LSH            20
#define ESDHC_IRQSTAT_DCE_LSH            21
#define ESDHC_IRQSTAT_DEBE_LSH            22
#define ESDHC_IRQSTAT_AC12E_LSH            24
#define ESDHC_IRQSTAT_DMAE_LSH            28

#define ESDHC_IRQSTAT_CC_WID            1
#define ESDHC_IRQSTAT_TC_WID            1
#define ESDHC_IRQSTAT_BGE_WID            1
#define ESDHC_IRQSTAT_DINT_WID            1
#define ESDHC_IRQSTAT_BWR_WID            1
#define ESDHC_IRQSTAT_BRR_WID            1
#define ESDHC_IRQSTAT_CINS_WID            1
#define ESDHC_IRQSTAT_CRM_WID            1
#define ESDHC_IRQSTAT_CINT_WID            1
#define ESDHC_IRQSTAT_CTOE_WID            1
#define ESDHC_IRQSTAT_CCE_WID            1
#define ESDHC_IRQSTAT_CEBE_WID            1
#define ESDHC_IRQSTAT_CIE_WID            1
#define ESDHC_IRQSTAT_DTOE_WID            1
#define ESDHC_IRQSTAT_DCE_WID            1
#define ESDHC_IRQSTAT_DEBE_WID            1
#define ESDHC_IRQSTAT_AC12E_WID            1
#define ESDHC_IRQSTAT_DMAE_WID            1

//Interrupt Status Enable register
#define ESDHC_IRQSTATEN_CCSEN_LSH            0
#define ESDHC_IRQSTATEN_TCSEN_LSH            1
#define ESDHC_IRQSTATEN_BGESEN_LSH            2
#define ESDHC_IRQSTATEN_DINTSEN_LSH            3
#define ESDHC_IRQSTATEN_BWRSEN_LSH            4
#define ESDHC_IRQSTATEN_BRRSEN_LSH            5
#define ESDHC_IRQSTATEN_CINSSEN_LSH            6
#define ESDHC_IRQSTATEN_CRMSEN_LSH            7
#define ESDHC_IRQSTATEN_CINTSEN_LSH            8
#define ESDHC_IRQSTATEN_CTOESEN_LSH            16
#define ESDHC_IRQSTATEN_CCESEN_LSH            17
#define ESDHC_IRQSTATEN_CEBESEN_LSH            18
#define ESDHC_IRQSTATEN_CICSEN_LSH            19
#define ESDHC_IRQSTATEN_DTOESEN_LSH            20
#define ESDHC_IRQSTATEN_DCESEN_LSH            21
#define ESDHC_IRQSTATEN_DEBESEN_LSH            22
#define ESDHC_IRQSTATEN_AC12ESEN_LSH        24
#define ESDHC_IRQSTATEN_DMAESEN_LSH            28

#define ESDHC_IRQSTATEN_CCSEN_WID            1
#define ESDHC_IRQSTATEN_TCSEN_WID            1
#define ESDHC_IRQSTATEN_BGESEN_WID            1
#define ESDHC_IRQSTATEN_DINTSEN_WID            1
#define ESDHC_IRQSTATEN_BWRSEN_WID            1
#define ESDHC_IRQSTATEN_BRRSEN_WID            1
#define ESDHC_IRQSTATEN_CINSSEN_WID            1
#define ESDHC_IRQSTATEN_CRMSEN_WID            1
#define ESDHC_IRQSTATEN_CINTSEN_WID            1
#define ESDHC_IRQSTATEN_CTOESEN_WID            1
#define ESDHC_IRQSTATEN_CCESEN_WID            1
#define ESDHC_IRQSTATEN_CEBESEN_WID            1
#define ESDHC_IRQSTATEN_CICSEN_WID            1
#define ESDHC_IRQSTATEN_DTOESEN_WID            1
#define ESDHC_IRQSTATEN_DCESEN_WID            1
#define ESDHC_IRQSTATEN_DEBESEN_WID            1
#define ESDHC_IRQSTATEN_AC12ESEN_WID        1
#define ESDHC_IRQSTATEN_DMAESEN_WID            1

//Interrupt Signal Enable register
#define ESDHC_IRQSIGEN_CCIEN_LSH            0
#define ESDHC_IRQSIGEN_TCIEN_LSH            1
#define ESDHC_IRQSIGEN_BGEIEN_LSH            2
#define ESDHC_IRQSIGEN_DINTIEN_LSH            3
#define ESDHC_IRQSIGEN_BWRIEN_LSH            4
#define ESDHC_IRQSIGEN_BRRIEN_LSH            5
#define ESDHC_IRQSIGEN_CINSIEN_LSH            6
#define ESDHC_IRQSIGEN_CRMIEN_LSH            7
#define ESDHC_IRQSIGEN_CINTIEN_LSH            8
#define ESDHC_IRQSIGEN_CTOEIEN_LSH            16
#define ESDHC_IRQSIGEN_CCEIEN_LSH            17
#define ESDHC_IRQSIGEN_CEBEIEN_LSH            18
#define ESDHC_IRQSIGEN_CICIEN_LSH            19
#define ESDHC_IRQSIGEN_DTOEIEN_LSH            20
#define ESDHC_IRQSIGEN_DCEIEN_LSH            21
#define ESDHC_IRQSIGEN_DEBEIEN_LSH            22
#define ESDHC_IRQSIGEN_AC12EIEN_LSH        24
#define ESDHC_IRQSIGEN_DMAEIEN_LSH            28

#define ESDHC_IRQSIGEN_CCIEN_WID            1
#define ESDHC_IRQSIGEN_TCIEN_WID            1
#define ESDHC_IRQSIGEN_BGEIEN_WID            1
#define ESDHC_IRQSIGEN_DINTIEN_WID            1
#define ESDHC_IRQSIGEN_BWRIEN_WID            1
#define ESDHC_IRQSIGEN_BRRIEN_WID            1
#define ESDHC_IRQSIGEN_CINSIEN_WID            1
#define ESDHC_IRQSIGEN_CRMIEN_WID            1
#define ESDHC_IRQSIGEN_CINTIEN_WID            1
#define ESDHC_IRQSIGEN_CTOEIEN_WID            1
#define ESDHC_IRQSIGEN_CCEIEN_WID            1
#define ESDHC_IRQSIGEN_CEBEIEN_WID            1
#define ESDHC_IRQSIGEN_CICIEN_WID            1
#define ESDHC_IRQSIGEN_DTOEIEN_WID            1
#define ESDHC_IRQSIGEN_DCEIEN_WID            1
#define ESDHC_IRQSIGEN_DEBEIEN_WID            1
#define ESDHC_IRQSIGEN_AC12EIEN_WID            1
#define ESDHC_IRQSIGEN_DMAEIEN_WID            1

// Auto CMD12 Error Status register
#define ESDHC_AUTOC12ERR_AC12NE_LSH            0
#define ESDHC_AUTOC12ERR_AC12TOE_LSH        1
#define ESDHC_AUTOC12ERR_AC12EBE_LSH        2
#define ESDHC_AUTOC12ERR_AC12CE_LSH            3
#define ESDHC_AUTOC12ERR_AC12IE_LSH            4
#define ESDHC_AUTOC12ERR_CNIBAC12E_LSH        7

#define ESDHC_AUTOC12ERR_AC12NE_WID            1
#define ESDHC_AUTOC12ERR_AC12TOE_WID        1
#define ESDHC_AUTOC12ERR_AC12EBE_WID        1
#define ESDHC_AUTOC12ERR_AC12CE_WID            1
#define ESDHC_AUTOC12ERR_AC12IE_WID            1
#define ESDHC_AUTOC12ERR_CNIBAC12E_WID        1

//Host Controller Capability register
#define ESDHC_HOSTCAPBLT_MBL_LSH        16
#define ESDHC_HOSTCAPBLT_ADMAS_LSH        20
#define ESDHC_HOSTCAPBLT_HSS_LSH        21
#define ESDHC_HOSTCAPBLT_DMAS_LSH        22
#define ESDHC_HOSTCAPBLT_SRS_LSH        23
#define ESDHC_HOSTCAPBLT_VS33_LSH        24
#define ESDHC_HOSTCAPBLT_VS30_LSH        25
#define ESDHC_HOSTCAPBLT_VS18_LSH        26

#define ESDHC_HOSTCAPBLT_MBL_WID        3
#define ESDHC_HOSTCAPBLT_ADMAS_WID        1
#define ESDHC_HOSTCAPBLT_HSS_WID        1
#define ESDHC_HOSTCAPBLT_DMAS_WID        1
#define ESDHC_HOSTCAPBLT_SRS_WID        1
#define ESDHC_HOSTCAPBLT_VS33_WID        1
#define ESDHC_HOSTCAPBLT_VS30_WID        1
#define ESDHC_HOSTCAPBLT_VS18_WID        1

//Watermark Level register
#define ESDHC_WML_RDWML_LSH                0
#define ESDHC_WML_WRWML_LSH                16

#define ESDHC_WML_RDWML_WID                8
#define ESDHC_WML_WRWML_WID                8

//Force Event register
#define ESDHC_FEVT_FEVTAC12NE_LSH        0
#define ESDHC_FEVT_FEVTAC12TOE_LSH        1
#define ESDHC_FEVT_FEVTAC12CE_LSH        2
#define ESDHC_FEVT_FEVTAC12EBE_LSH        3
#define ESDHC_FEVT_FEVTAC12IE_LSH        4
#define ESDHC_FEVT_FEVTCNIBAC12E_LSH    7
#define ESDHC_FEVT_FEVTCTOE_LSH            16
#define ESDHC_FEVT_FEVTCCE_LSH            17
#define ESDHC_FEVT_FEVTCEBE_LSH            18
#define ESDHC_FEVT_FEVTCIE_LSH            19
#define ESDHC_FEVT_FEVTDTOE_LSH            20
#define ESDHC_FEVT_FEVTDCE_LSH            21
#define ESDHC_FEVT_FEVTDEBE_LSH            22
#define ESDHC_FEVT_FEVTAC12E_LSH        24
#define ESDHC_FEVT_FEVTDMAE_LSH            28
#define ESDHC_FEVT_FEVTCINT_LSH            31

#define ESDHC_FEVT_FEVTAC12NE_WID        1
#define ESDHC_FEVT_FEVTAC12TOE_WID        1
#define ESDHC_FEVT_FEVTAC12CE_WID        1
#define ESDHC_FEVT_FEVTAC12EBE_WID        1
#define ESDHC_FEVT_FEVTAC12IE_WID        1
#define ESDHC_FEVT_FEVTCNIBAC12E_WID    1
#define ESDHC_FEVT_FEVTCTOE_WID            1
#define ESDHC_FEVT_FEVTCCE_WID            1
#define ESDHC_FEVT_FEVTCEBE_WID            1
#define ESDHC_FEVT_FEVTCIE_WID            1
#define ESDHC_FEVT_FEVTDTOE_WID            1
#define ESDHC_FEVT_FEVTDCE_WID            1
#define ESDHC_FEVT_FEVTDEBE_WID            1
#define ESDHC_FEVT_FEVTAC12E_WID        1
#define ESDHC_FEVT_FEVTDMAE_WID            1
#define ESDHC_FEVT_FEVTCINT_WID            1

//ADMA Error Status register
#define ESDHC_ADMAES_ADMAES_LSH            0
#define ESDHC_ADMAES_ADMALME_LSH            2
#define ESDHC_ADMAES_ADMADCE_LSH          3

#define ESDHC_ADMAES_ADMAES_WID            2
#define ESDHC_ADMAES_ADMALME_WID          1
#define ESDHC_ADMAES_ADMADCE_WID          1

// Vendor Specific register
#define ESDHC_VENDOR_EXTDMAEN_LSH          0
#define ESDHC_VENDOR_VOLTSEL_LSH            1
#define ESDHC_VENDOR_INTSTVAL_LSH           16
#define ESDHC_VENDOR_DBGSEL_LSH            24

#define ESDHC_VENDOR_EXTDMAEN_WID          1
#define ESDHC_VENDOR_VOLTSEL_WID            1
#define ESDHC_VENDOR_INTSTVAL_WID           8
#define ESDHC_VENDOR_DBGSEL_WID              4

//Host Controller Version register
#define ESDHC_HOSTVER_SVN_LSH            0
#define ESDHC_HOSTVER_VVN_LSH            8

#define ESDHC_HOSTVER_SVN_WID            8
#define ESDHC_HOSTVER_VVN_WID            8

typedef enum
{
    ESDHC_CMD_NORMAL = 0,
    ESDHC_CMD_SUSPEND = 1,
    ESDHC_CMD_RESUME = 2,
    ESDHC_CMD_ABORT = 3,
}ESDHCCmdType;

typedef enum
{
    ESDHC_RSPLEN_0 = 0,
    ESDHC_RSPLEN_136 = 1,
    ESDHC_RSPLEN_48 = 2,
    ESDHC_RSPLEN_48B = 3,
}ESDHCRspType;

typedef enum
{
    ESDHC_EMODE_BE = 0,
    ESDHC_EMODE_HWBE = 1,
    ESDHC_EMODE_LE = 2,
}ESDHCEndianMode;

typedef enum
{
    ESDHC_DTW_1BIT = 0,
    ESDHC_DTW_4BIT = 1,
    ESDHC_DTW_8BIT = 2,
}ESDHCDataBusWidth;

typedef enum
{
    ESDHC_VVN_FSL10 = 0,    //Freescale eSDHC V1.0
    ESDHC_VVN_FSL20 = 0x10, //Freescale eSDHC V2.0
    ESDHC_VVN_FSL21 = 0x11, //Freescale eSDHC V2.1
    ESDHC_VVN_FSL22 = 0x12  //Freescale eSDHC V2.2
}ESDHCVendorVersion;    

typedef enum
{
    ESDHC_DMAS_SIMPLE_DMA = 0,  //Simple DMA or No DMA
    ESDHC_DMAS_32BIT_ADMA = 1,  // 32bit ADMA    (data needs to be 4KB aligned)
    ESDHC_DMAS_32BIT_ADMA2 = 2  // 32bit ADMA2 (data only needs to be DWORD aligned)
}ESDHCDMASMode;

#define ESDHC_MAX_CLOCK_RATE        52000000        // 52 MHz max
#define ESDHC_MAX_NUMBLOCKS         65535
#define ESDHC_MAX_BLK_LENGTH        4096
#define ESDHC_MIN_BLK_LENGTH         1
#define ESDHC_MAX_DATA_BUFFER_SIZE    512                 // Max number of bytes in data buffer port
#define ESDHC_MAX_DATA_TIMEOUT_COUNTER_VAL  0xE   // data timeout will occur after SDCLK * 2^27 ticks
#define ESDHC_MAX_POWER_SUPPLY_RAMP_UP               250 // SD Phys Layer 6.6

#define ESDHC_ERROR_BITS        0x117F0000
#define ESDHC_CMD_ERROR_BITS        0x000F0000
#define ESDHC_DAT_ERROR_BITS        0x00700000

#ifdef __cplusplus
}
#endif

#endif      // #ifndef __COMMON_ESDHC_H


