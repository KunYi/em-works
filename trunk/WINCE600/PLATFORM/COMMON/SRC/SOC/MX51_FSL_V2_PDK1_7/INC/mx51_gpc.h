//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx51_gpc.h
//
//  Provides definitions for the GPC (Global Power Controller).
//
//------------------------------------------------------------------------------

#ifndef __MX51_GPC_H
#define __MX51_GPC_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

#define  CSP_BASE_REG_PA_GPC_DPTC_LP          (CSP_BASE_REG_PA_GPC + 0x0080)
#define  CSP_BASE_REG_PA_GPC_DPTC_GP          (CSP_BASE_REG_PA_GPC + 0x0100) 
#define  CSP_BASE_REG_PA_GPC_DVFS_CORE        (CSP_BASE_REG_PA_GPC + 0x0180) 
#define  CSP_BASE_REG_PA_GPC_DVFS_PER         (CSP_BASE_REG_PA_GPC + 0x01C0) 
#define  CSP_BASE_REG_PA_GPC_PGC_GPU2D        (CSP_BASE_REG_PA_GPC + 0x0200) 
#define  CSP_BASE_REG_PA_GPC_PGC_IPU          (CSP_BASE_REG_PA_GPC + 0x0220) 
#define  CSP_BASE_REG_PA_GPC_PGC_VPU          (CSP_BASE_REG_PA_GPC + 0x0240) 
#define  CSP_BASE_REG_PA_GPC_PGC_GPU          (CSP_BASE_REG_PA_GPC + 0x0260)
#define  CSP_BASE_REG_PA_GPC_SRPGC_NEON       (CSP_BASE_REG_PA_GPC + 0x0280) 
#define  CSP_BASE_REG_PA_GPC_SRPGC_ARM        (CSP_BASE_REG_PA_GPC + 0x02A0) 
#define  CSP_BASE_REG_PA_GPC_EMPGC0_ARM_L1    (CSP_BASE_REG_PA_GPC + 0x02C0) 
#define  CSP_BASE_REG_PA_GPC_EMPGC1_ARM_L2    (CSP_BASE_REG_PA_GPC + 0x02D0) 
#define  CSP_BASE_REG_PA_GPC_SRPGC_MEGAMIX    (CSP_BASE_REG_PA_GPC + 0x02E0)
#define  CSP_BASE_REG_PA_GPC_SRPGC_EMI        (CSP_BASE_REG_PA_GPC + 0x0300)

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32  DPTCCR;
    UINT32  DPTCDBG;
    UINT32  DCVR0;
    UINT32  DCVR1;
    UINT32  DCVR2;
    UINT32  DCVR3;
} CSP_DPTC_REGS, *PCSP_DPTC_REGS;

typedef struct
{
    UINT32  DVFSTHRS; 
    UINT32  DVFSCOUN; 
    UINT32  DVFSSIG1; 
    UINT32  DVFSSIG0; 
    UINT32  DVFSGPC0; 
    UINT32  DVFSGPC1; 
    UINT32  DVFSGPBT; 
    UINT32  DVFSEMAC; 
    UINT32  DVFSCNTR; 
    UINT32  DVFSLTR0_0; 
    UINT32  DVFSLTR0_1; 
    UINT32  DVFSLTR1_0; 
    UINT32  DVFSLTR1_1; 
    UINT32  DVFSPT0;
    UINT32  DVFSPT1;
    UINT32  DVFSPT2;
    UINT32  DVFSPT3;
}CSP_DVFS_CORE_REGS, *PCSP_DVFS_CORE_REGS;

typedef struct
{
    UINT32  reserver0;
    UINT32  LTR0;
    UINT32  LTR1;
    UINT32  LTR2;
    UINT32  LTR3;
    UINT32  LTBR0;
    UINT32  LTBR1;
    UINT32  PMCR0;
    UINT32  PMCR1;
} CSP_DVFS_PERIPH_REGS, *PCSP_DVFS_PERIPH_REGS;


typedef struct
{
    UINT32  PGCR;
    UINT32  PUPSCR;
    UINT32  PDNSCR;
    UINT32  PGSR;
} CSP_PGC_REGS, *PCSP_PGC_REGS;

typedef struct
{
    UINT32  SRPGCR;
    UINT32  PUPSCR;
    UINT32  PDNSCR;
    UINT32  SRPGSR;
    UINT32  SRPGDR;
} CSP_SRPGC_REGS, *PCSP_SRPGC_REGS;

typedef struct
{
    UINT32  EMPGCR;
    UINT32  PUPSCR;
    UINT32  PDNSCR;
    UINT32  EMPGSR;
} CSP_EMPGC_REGS, *PCSP_EMPGC_REGS;


typedef struct
{
    UINT32  CNTR;               // 0x0000
    UINT32  PGR;                // 0x0004
    UINT32  VCR;                // 0x0008
    UINT32  ALL_PU;             // 0x000C
    UINT32  NEON;               // 0x0010
} CSP_GPC_REGS, *PCSP_GPC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
//GPC REGISTER OFFSET
#define  GPC_CNTR_OFFSET               0x0000
#define  GPC_PGR_OFFSET                0x0004
#define  GPC_VCR_OFFSET                0x0008
#define  GPC_ALL_PU_OFFSET             0x000C
#define  GPC_NEON_OFFSET               0x0010                                            

//DPTC REGISTER OFFSET
#define  DPTC_DPTCCR_OFFSET        0x0000
#define  DPTC_DPTCDBG_OFFSET       0x0004
#define  DPTC_DCVR0_OFFSET         0x0008
#define  DPTC_DCVR1_OFFSET         0x000C
#define  DPTC_DCVR2_OFFSET         0x0010
#define  DPTC_DCVR3_OFFSET         0x0014

//DVFS CORE REGISTER OFFSETS
#define  DVFS_CORE_DVFSTHRS_OFFSET               0x0000
#define  DVFS_CORE_DVFSCOUN_OFFSET               0x0004
#define  DVFS_CORE_DVFSSIG1_OFFSET               0x0008
#define  DVFS_CORE_DVFSSIG0_OFFSET               0x001C
#define  DVFS_CORE_DVFSGPC0_OFFSET               0x0010
#define  DVFS_CORE_DVFSGPC1_OFFSET               0x0014
#define  DVFS_CORE_DVFSGPBT_OFFSET               0x0018
#define  DVFS_CORE_DVFSEMAC_OFFSET               0x001C
#define  DVFS_CORE_DVFSCNTR_OFFSET               0x0020
#define  DVFS_CORE_DVFSLTR0_0_OFFSET             0x0024
#define  DVFS_CORE_DVFSLTR0_1_OFFSET             0x0028
#define  DVFS_CORE_DVFSLTR1_0_OFFSET             0x002C
#define  DVFS_CORE_DVFSLTR1_1_OFFSET             0x0030
#define  DVFS_CORE_DVFSPT0_OFFSET                0x0034
#define  DVFS_CORE_DVFSPT1_OFFSET                0x0038
#define  DVFS_CORE_DVFSPT2_OFFSET                0x003C
#define  DVFS_CORE_DVFSPT3_OFFSET                0x0040

//DVFS PER REGISTER OFFSET
#define  DVFS_PER_LTR0_OFFSET                   0x0004  
#define  DVFS_PER_LTR1_OFFSET                   0x0008
#define  DVFS_PER_LTR2_OFFSET                   0x000C
#define  DVFS_PER_LTR3_OFFSET                   0x0010
#define  DVFS_PER_LTBR0_OFFSET                  0x0014
#define  DVFS_PER_LTBR1_OFFSET                  0x0018
#define  DVFS_PER_PMCR0_OFFSET                  0x001C
#define  DVFS_PER_PMCR1_OFFSET                  0x0020


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//GPC REGISTER
//CNTR
#define  GPC_CNTR_HTRI_LSH                           0
#define  GPC_CNTR_FUPD_LSH                           13
#define  GPC_CNTR_STRT_LSH                           14
#define  GPC_CNTR_ADU_LSH                            15
#define  GPC_CNTR_DVFS0CR_LSH                        16
#define  GPC_CNTR_DVFS1CR_LSH                        17
#define  GPC_CNTR_DPTC0CR_LSH                        18
#define  GPC_CNTR_DPTC1CR_LSH                        19
#define  GPC_CNTR_GPCIRQ_LSH                         20
#define  GPC_CNTR_GPCIRQM_LSH                        21
#define  GPC_CNTR_IRQ2_LSH                           24
#define  GPC_CNTR_IRQ2M_LSH                          25
#define  GPC_CNTR_CSPI_LSH                           26
//PGR                                                
#define  GPC_PGR_IPUPG_LSH                           0
#define  GPC_PGR_VPUPG_LSH                           2
#define  GPC_PGR_GPUPG_LSH                           4
#define  GPC_PGR_ARMPG_LSH                           8
#define  GPC_PGR_IPCI_LSH                            16
#define  GPC_PGR_IPCO_LSH                            22
#define  GPC_PGR_IPCC_LSH                            28
#define  GPC_PGR_DRCIC_LSH                           29
                                                     
//VCR                                                
#define  GPC_VCR_VCNT_LSH                            0
#define  GPC_VCR_VCNTU_LSH                           16
#define  GPC_VCR_VINC_LSH                            17

//ALL_PU
#define  GPC_ALL_PU_IPUPDR_LSH                       0
#define  GPC_ALL_PU_GPUPDR_LSH                       1
#define  GPC_ALL_PU_VPUPDR_LSH                       2
#define  GPC_ALL_PU_IPUPUR_LSH                       4
#define  GPC_ALL_PU_GPUPUR_LSH                       5
#define  GPC_ALL_PU_VPUPUR_LSH                       6
#define  GPC_ALL_PU_IPUSWSTATUS_LSH                  8
#define  GPC_ALL_PU_GPUSWSTATUS_LSH                  9
#define  GPC_ALL_PU_VPUSWSTATUS_LSH                  10

//NEON
#define  GPC_NEON_NEONPDR_LSH                        0
#define  GPC_NEON_NEONPUR_LSH                        1
#define  GPC_NEON_NEONFSMST_LSH                      4

//DPTC REGISTER
//DPTCCR
#define  DPTC_DPTCCR_DEN_LSH                         0
#define  DPTC_DPTCCR_VAI_LSH                         1
#define  DPTC_DPTCCR_VAIM_LSH                        3
#define  DPTC_DPTCCR_DPVV_LSH                        4
#define  DPTC_DPTCCR_DPNVCR_LSH                      5
#define  DPTC_DPTCCR_DSMM_LSH                        6
#define  DPTC_DPTCCR_DCR_LSH                        17
#define  DPTC_DPTCCR_DRCE0_LSH                      19
#define  DPTC_DPTCCR_DRCE1_LSH                      20
#define  DPTC_DPTCCR_DRCE2_LSH                      21
#define  DPTC_DPTCCR_DRCE3_LSH                      22
//DCVR0                                    
#define  DPTC_DCVR0_ELV_LSH                          0
#define  DPTC_DCVR0_LLV_LSH                         10
#define  DPTC_DCVR0_ULV_LSH                         21
//DCVR1                                     
#define  DPTC_DCVR1_ELV_LSH                          0
#define  DPTC_DCVR1_LLV_LSH                         10
#define  DPTC_DCVR1_ULV_LSH                         21
//DCVR2                                     
#define  DPTC_DCVR2_ELV_LSH                          0
#define  DPTC_DCVR2_LLV_LSH                         10
#define  DPTC_DCVR2_ULV_LSH                         21
//DCVR3                                     
#define  DPTC_DCVR3_ELV_LSH                          0
#define  DPTC_DCVR3_LLV_LSH                         10
#define  DPTC_DCVR3_ULV_LSH                         21
                                           
//DVFS CORE REGISTER                       
                                          
//DVFSTHRS                       
#define  DVFS_CORE_DVFSTHRS_PNCTHR_LSH               0
#define  DVFS_CORE_DVFSTHRS_DNTHR_LSH               16
#define  DVFS_CORE_DVFSTHRS_UPTHR_LSH               22
//DVFSCOUN                       
#define  DVFS_CORE_DVFSCOUN_UPCNT_LSH                0
#define  DVFS_CORE_DVFSCOUN_DNCNT_LSH               16
//DVFSGPC0                       
#define  DVFS_CORE_DVFSGPC0_GPBC0_LSH                0
#define  DVFS_CORE_DVFSGPC0_C0ACT_LSH               30
#define  DVFS_CORE_DVFSGPC0_C0STRT_LSH              31
//DVFSGPC1                       
#define  DVFS_CORE_DVFSGPC1_GPBC1_LSH                0
#define  DVFS_CORE_DVFSGPC1_C1ACT_LSH               30
#define  DVFS_CORE_DVFSGPC1_C1STRT_LSH              31
//DVFSEMAC                       
#define  DVFS_CORE_DVFSEMAC_EMAC_LSH                 0
//DVFSCNTR                                 
#define  DVFS_CORE_DVFSCNTR_DVFEN_LSH                0   
#define  DVFS_CORE_DVFSCNTR_LTBRSR_LSH               3
#define  DVFS_CORE_DVFSCNTR_LTBRSH_LSH               5
#define  DVFS_CORE_DVFSCNTR_FPUS_LSH                 6
#define  DVFS_CORE_DVFSCNTR_FPUE_LSH                 9
#define  DVFS_CORE_DVFSCNTR_DIV_RATIO_LSH           11
#define  DVFS_CORE_DVFSCNTR_MINF_LSH                17
#define  DVFS_CORE_DVFSCNTR_MAXF_LSH                18
#define  DVFS_CORE_DVFSCNTR_WFIM_LSH                19
#define  DVFS_CORE_DVFSCNTR_FSVAI_LSH               20
#define  DVFS_CORE_DVFSCNTR_FSVAIM_LSH              22
#define  DVFS_CORE_DVFSCNTR_PIRQS_LSH               23
#define  DVFS_CORE_DVFSCNTR_DVFIS_LSH               24
#define  DVFS_CORE_DVFSCNTR_LBFL0_LSH               25
#define  DVFS_CORE_DVFSCNTR_LBFL1_LSH               26
#define  DVFS_CORE_DVFSCNTR_LBMI_LSH                27
#define  DVFS_CORE_DVFSCNTR_DVFEV_LSH               28
#define  DVFS_CORE_DVFSCNTR_DIV3CK_LSH              29
//DVFSPT0
#define  DVFS_CORE_DVFSPT0_FPTN0_LSH                0
#define  DVFS_CORE_DVFSPT0_PT0A_LSH                 17
//DVFSPT1
#define  DVFS_CORE_DVFSPT1_FPTN1_LSH                0
#define  DVFS_CORE_DVFSPT1_PT1A_LSH                 17
//DVFSPT2
#define  DVFS_CORE_DVFSPT2_FPTN2_LSH                0
#define  DVFS_CORE_DVFSPT2_PT2A_LSH                 17
#define  DVFS_CORE_DVFSPT2_P2THR_LSH                26
//DVFSPT3
#define  DVFS_CORE_DVFSPT3_FPTN3_LSH                0
#define  DVFS_CORE_DVFSPT3_PT3A_LSH                 17
                                           
//DVFS PER REGISTER     
//LTR0
#define  DVFS_PERIPH_LTR0_DIV3CK_LSH                0
#define  DVFS_PERIPH_LTR0_DWTHR_LSH                 16
#define  DVFS_PERIPH_LTR0_UPTHR_LSH                 22
//LTR1
#define  DVFS_PERIPH_LTR1_PNCTHR_LSH                0
#define  DVFS_PERIPH_LTR1_UPCNT_LSH                 6
#define  DVFS_PERIPH_LTR1_DWCNT_LSH                 14
#define  DVFS_PERIPH_LTR1_LTBRSR_LSH                22
#define  DVFS_PERIPH_LTR1_LTBRSH_LSH                23
#define  DVFS_PERIPH_LTR1_DIV_RATIO_LSH             26
//LTR2
#define  DVFS_PERIPH_LTR2_EMAC_LSH                  0
//PMCR0                                    
#define  DVFS_PER_PMCR0_DVFEN_LSH                    4
#define  DVFS_PER_PMCR0_WFIM_LSH                    10
#define  DVFS_PER_PMCR0_FSVAI_LSH                   13
#define  DVFS_PER_PMCR0_FSVAIM_LSH                  15
#define  DVFS_PER_PMCR0_LBCF_LSH                    18
#define  DVFS_PER_PMCR0_LBFL_LSH                    20
#define  DVFS_PER_PMCR0_LBMI_LSH                    21
#define  DVFS_PER_PMCR0_DVFIS_LSH                   22
#define  DVFS_PER_PMCR0_DVFEV_LSH                   23
#define  DVFS_PER_PMCR0_UDSC_LSH                    27
//PMCR1                                    
#define  DVFS_PER_PMCR1_DVGA_LSH                     0
#define  DVFS_PER_PMCR1_P2PM_LSH                    16
#define  DVFS_PER_PMCR1_P4PM_LSH                    17
#define  DVFS_PER_PMCR1_P1IFM_LSH                   18
#define  DVFS_PER_PMCR1_P1ISM_LSH                   19
#define  DVFS_PER_PMCR1_P1INM_LSH                   20

//PGC REGISTER
//PGCR
#define  PGC_PGCR_PCR_LSH                            0
//PUPSCR                                             
#define  PGC_PUPSCR_SW_LSH                           0
#define  PGC_PUPSCR_SW2ISO_LSH                       8
//PDNSCR                                             
#define  PGC_PDNSCR_ISO_LSH                          0
#define  PGC_PDNSCR_ISO2SW_LSH                       8
//PGSR                                               
#define  PGC_PGSR_PSR_LSH                            0              
                                                     
//SRPGC REGISTER                                         
//SRPGCR                                                 
#define  SRPGC_SRPGCR_PCR_LSH                        0 
//PUPSCR                                            
#define  SRPGC_PUPSCR_SW_LSH                         0 
#define  SRPGC_PUPSCR_SW2SH_LSH                      8      
#define  SRPGC_PUPSCR_SH2PG_LSH                     16
#define  SRPGC_PUPSCR_PG2ISO_LSH                    24

//PDNSCR                                              
#define  SRPGC_PDNSCR_ISO_LSH                        0 
#define  SRPGC_PDNSCR_ISO2PG_LSH                     8 
#define  SRPGC_PDNSCR_PG2SH_LSH                     16 
#define  SRPGC_PDNSCR_SH2SW_LSH                     24 
//SRPGSR                                                
#define  SRPGC_SRPGSR_PSR_LSH                        0 
    
//EMPGC REGISTER
//EMPGCR
#define  EMPGC_EMPGCR_PCR_LSH                        0
//PUPSCR                                           
#define  EMPGC_PUPSCR_PUP_LSH                        0
//PDNSCR                                           
#define  EMPGC_PDNSCR_PDN_LSH                        0
//EMPGSR                                             
#define  EMPGC_EMPGSR_PSR_LSH                        0  

   
//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
//GPC REGISTER
//CNTR
#define  GPC_CNTR_HTRI_WID                           4
#define  GPC_CNTR_FUPD_WID                           1
#define  GPC_CNTR_STRT_WID                           1
#define  GPC_CNTR_ADU_WID                            1
#define  GPC_CNTR_DVFS0CR_WID                        1
#define  GPC_CNTR_DVFS1CR_WID                        1
#define  GPC_CNTR_DPTC0CR_WID                        1
#define  GPC_CNTR_DPTC1CR_WID                        1
#define  GPC_CNTR_GPCIRQ _WID                        1
#define  GPC_CNTR_GPCIRQM_WID                        1
#define  GPC_CNTR_IRQ2_WID                           1
#define  GPC_CNTR_IRQ2M_WID                          1
#define  GPC_CNTR_CSPI_WID                           1
//PGR                                                
#define  GPC_PGR_IPUPG_WID                           2
#define  GPC_PGR_VPUPG_WID                           2
#define  GPC_PGR_GPUPG_WID                           2
#define  GPC_PGR_ARMPG_WID                           2
#define  GPC_PGR_IPCI_WID                            6
#define  GPC_PGR_IPCO_WID                            6
#define  GPC_PGR_IPCC_WID                            1
#define  GPC_PGR_DRCIC_WID                           2
                                                     
//VCR                                                
#define  GPC_VCR_VCNT_WID                            15
#define  GPC_VCR_VCNTU_WID                           1
#define  GPC_VCR_VINC_WID                            1

//ALL_PU
#define  GPC_ALL_PU_IPUPDR_WID                       1
#define  GPC_ALL_PU_GPUPDR_WID                       1
#define  GPC_ALL_PU_VPUPDR_WID                       1
#define  GPC_ALL_PU_IPUPUR_WID                       1
#define  GPC_ALL_PU_GPUPUR_WID                       1
#define  GPC_ALL_PU_VPUPUR_WID                       1
#define  GPC_ALL_PU_IPUSWSTATUS_WID                  1
#define  GPC_ALL_PU_GPUSWSTATUS_WID                  1
#define  GPC_ALL_PU_VPUSWSTATUS_WID                  1

//NEON
#define  GPC_NEON_NEONPDR_WID                        1
#define  GPC_NEON_NEONPUR_WID                        1
#define  GPC_NEON_NEONFSMST_WID                      2


//DPTC REGISTER
//DPTCCR
#define  DPTC_DPTCCR_DEN_WID                          1
#define  DPTC_DPTCCR_VAI_WID                          2
#define  DPTC_DPTCCR_VAIM_WID                         1
#define  DPTC_DPTCCR_DPVV_WID                         1
#define  DPTC_DPTCCR_DPNVCR_WID                       1
#define  DPTC_DPTCCR_DSMM_WID                         1
#define  DPTC_DPTCCR_DCR_WID                          1
#define  DPTC_DPTCCR_DRCE0_WID                        1
#define  DPTC_DPTCCR_DRCE1_WID                        1
#define  DPTC_DPTCCR_DRCE2_WID                        1
#define  DPTC_DPTCCR_DRCE3_WID                        1
//DCVR0                                     
#define  DPTC_DCVR0_ELV_WID                          10
#define  DPTC_DCVR0_LLV_WID                          11
#define  DPTC_DCVR0_ULV_WID                          11
//DCVR1                                     
#define  DPTC_DCVR1_ELV_WID                          10
#define  DPTC_DCVR1_LLV_WID                          11
#define  DPTC_DCVR1_ULV_WID                          11
//DCVR2                                     
#define  DPTC_DCVR2_ELV_WID                          10
#define  DPTC_DCVR2_LLV_WID                          11
#define  DPTC_DCVR2_ULV_WID                          11
//DCVR3                                     
#define  DPTC_DCVR3_ELV_WID                          10
#define  DPTC_DCVR3_LLV_WID                          11
#define  DPTC_DCVR3_ULV_WID                          11
                                            
//DVFS CORE REGISTER                        
                                            
//DVFSTHRS                        
#define  DVFS_CORE_DVFSTHRS_PNCTHR_WID                6
#define  DVFS_CORE_DVFSTHRS_DNTHR_WID                 6
#define  DVFS_CORE_DVFSTHRS_UPTHR_WID                 6
//DVFSCOUN                                  
#define  DVFS_CORE_DVFSCOUN_UPCNT_WID                 8
#define  DVFS_CORE_DVFSCOUN_DNCNT_WID                 8
//DVFSGPC0                        
#define  DVFS_CORE_DVFSGPC0_GPBC0_WID                17
#define  DVFS_CORE_DVFSGPC0_C0ACT_WID                 1
#define  DVFS_CORE_DVFSGPC0_C0STRT_WID                1
//DVFSGPC1                        
#define  DVFS_CORE_DVFSGPC1_GPBC1_WID                17
#define  DVFS_CORE_DVFSGPC1_C1ACT_WID                 1
#define  DVFS_CORE_DVFSGPC1_C1STRT_WID                1
//DVFSEMAC                                  
#define  DVFS_CORE_DVFSEMAC_EMAC_WID                  9
//DVFSCNTR                                  
#define  DVFS_CORE_DVFSCNTR_DVFEN_WID                 1   
#define  DVFS_CORE_DVFSCNTR_DIV3CK_WID                2
#define  DVFS_CORE_DVFSCNTR_LTBRSR_WID                2
#define  DVFS_CORE_DVFSCNTR_LTBRSH_WID                1
#define  DVFS_CORE_DVFSCNTR_FPUS_WID                  3
#define  DVFS_CORE_DVFSCNTR_FPUE_WID                  1
#define  DVFS_CORE_DVFSCNTR_FPUSTRT_WID               1
#define  DVFS_CORE_DVFSCNTR_DIV_RATIO_WID             3
#define  DVFS_CORE_DVFSCNTR_MINF_WID                  1
#define  DVFS_CORE_DVFSCNTR_MAXF_WID                  1
#define  DVFS_CORE_DVFSCNTR_WFIM_WID                  1
#define  DVFS_CORE_DVFSCNTR_FSVAI_WID                 2
#define  DVFS_CORE_DVFSCNTR_FSVAIM_WID                1
#define  DVFS_CORE_DVFSCNTR_PIRQS_WID                 1
#define  DVFS_CORE_DVFSCNTR_DVFIS_WID                 1
#define  DVFS_CORE_DVFSCNTR_LBFL0_WID                 1
#define  DVFS_CORE_DVFSCNTR_LBFL1_WID                 1
#define  DVFS_CORE_DVFSCNTR_LBMI_WID                  1
#define  DVFS_CORE_DVFSCNTR_DVFEV_WID                 1
#define  DVFS_CORE_DVFSCNTR_DIV3CK_WID                2
//DVFSPT0
#define  DVFS_CORE_DVFSPT0_FPTN0_WID                17
#define  DVFS_CORE_DVFSPT0_PT0A_WID                 1
//DVFSPT1
#define  DVFS_CORE_DVFSPT1_FPTN1_WID                17
#define  DVFS_CORE_DVFSPT1_PT1A_WID                 1
//DVFSPT2
#define  DVFS_CORE_DVFSPT2_FPTN2_WID                17
#define  DVFS_CORE_DVFSPT2_PT2A_WID                 1
#define  DVFS_CORE_DVFSPT2_P2THR_WID                6
//DVFSPT3
#define  DVFS_CORE_DVFSPT3_FPTN3_WID                0
#define  DVFS_CORE_DVFSPT3_PT3A_WID                 1
                                            
//DVFS PER REGISTER                         
//LTR0
#define  DVFS_PERIPH_LTR0_DIV3CK_WID                3
#define  DVFS_PERIPH_LTR0_DWTHR_WID                 6
#define  DVFS_PERIPH_LTR0_UPTHR_WID                 6
//LTR1
#define  DVFS_PERIPH_LTR1_PNCTHR_WID                6
#define  DVFS_PERIPH_LTR1_UPCNT_WID                 8
#define  DVFS_PERIPH_LTR1_DWCNT_WID                 8
#define  DVFS_PERIPH_LTR1_LTBRSR_WID                1
#define  DVFS_PERIPH_LTR1_LTBRSH_WID                1
#define  DVFS_PERIPH_LTR1_DIV_RATIO_WID             6
//LTR2
#define  DVFS_PERIPH_LTR2_EMAC_WID                  9

//PMCR0                                     
#define  DVFS_PER_PMCR0_DVFEN_WID                     1
#define  DVFS_PER_PMCR0_WFIM_WID                      1
#define  DVFS_PER_PMCR0_FSVAI_WID                     2
#define  DVFS_PER_PMCR0_FSVAIM_WID                    1
#define  DVFS_PER_PMCR0_LBCF_WID                      2
#define  DVFS_PER_PMCR0_LBFL_WID                      1
#define  DVFS_PER_PMCR0_LBMI_WID                      1
#define  DVFS_PER_PMCR0_DVFIS_WID                     1
#define  DVFS_PER_PMCR0_DVFEV_WID                     1
#define  DVFS_PER_PMCR0_UDSC_WID                      1
//PMCR1                                     
#define  DVFS_PER_PMCR1_DVGA_WID                     16
#define  DVFS_PER_PMCR1_P2PM_WID                      1
#define  DVFS_PER_PMCR1_P4PM_WID                      1
#define  DVFS_PER_PMCR1_P1IFM_WID                     1
#define  DVFS_PER_PMCR1_P1ISM_WID                     1
#define  DVFS_PER_PMCR1_P1INM_WID                     1
  

//PGC REGISTER        
//PGCR                
#define  PGC_PGCR_PCR_WID                            1 
//PUPSCR                                              
#define  PGC_PUPSCR_SW_WID                           6
#define  PGC_PUPSCR_SW2ISO_WID                       6 
//PDNSCR                                              
#define  PGC_PDNSCR_ISO_WID                          6 
#define  PGC_PDNSCR_ISO2SW_WID                       6 
//PGSR                                                
#define  PGC_PGSR_PSR_WID                            1 
                                                      
//SRPGC REGISTER                                      
//SRPGCR                                              
#define  SRPGC_SRPGCR_PCR_WID                        1 
//PUPSCR                                              
#define  SRPGC_PUPSCR_SW_WID                         6 
#define  SRPGC_PUPSCR_SW2SH_WID                      6     
#define  SRPGC_PUPSCR_SH2PG_WID                      6 
#define  SRPGC_PUPSCR_PG2ISO_WID                     2 
                                                      
//PDNSCR                                              
#define  SRPGC_PDNSCR_ISO_WID                        6 
#define  SRPGC_PDNSCR_ISO2PG_WID                     6 
#define  SRPGC_PDNSCR_PG2SH_WID                      2
#define  SRPGC_PDNSCR_SH2SW_WID                      6 
//SRPGSR                                              
#define  SRPGC_SRPGSR_PSR_WID                        1 
                                                      
//EMPGC REGISTER                                      
//EMPGCR                                              
#define  EMPGC_EMPGCR_PCR_WID                        1                                                       
//PUPSCR                                                                                                    
#define  EMPGC_PUPSCR_PUP_WID                        8                                                       
//PDNSCR                                                                                                    
#define  EMPGC_PDNSCR_PDN_WID                        8                                                       
//EMPGSR                                                                                                    
#define  EMPGC_EMPGSR_PSR_WID                        1                                                       

  
//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//GPC REGISTER
//CNTR                                                
#define  GPC_CNTR_FUPD_FREQ_UPDATE_ENABLE            1
#define  GPC_CNTR_FUPD_FREQ_UPDATE_DISABLE           1

#define  GPC_CNTR_STRT_CNTL_FINISHED                 0
#define  GPC_CNTR_STRT_CNTL_INPROGRESS               1

#define  GPC_CNTR_ADU_PER_FREQ_UPDATE                0
#define  GPC_CNTR_ADU_ARM_FREQ_UPDATE                1

#define  GPC_CNTR_DVFS0CR_NOUPDATE_REQ               0
#define  GPC_CNTR_DVFS0CR_UPDATE_REQ                 1

#define  GPC_CNTR_DVFS1CR_NOUPDATE_REQ               0
#define  GPC_CNTR_DVFS1CR_UPDATE_REQ                 1

#define  GPC_CNTR_DPTC0CR_NOUPDATE_REQ               0
#define  GPC_CNTR_DPTC0CR_UPDATE_REQ                 1

#define  GPC_CNTR_DPTC1CR_NOUPDATE_REQ               0
#define  GPC_CNTR_DPTC1CR_UPDATE_REQ                 1

#define  GPC_CNTR_GPCIRQ_SDMA_EVENT                  0
#define  GPC_CNTR_GPCIRQ_ARM_IRQ                     1

#define  GPC_CNTR_GPCIRQM_UNMASKED                   0
#define  GPC_CNTR_GPCIRQM_MASKED                     1

#define  GPC_CNTR_IRQ2M_UNMASKED                     0
#define  GPC_CNTR_IRQ2M_MASKED                       1
                                                     
#define  GPC_CNTR_CSPI_I2C                           0
#define  GPC_CNTR_CSPI_CSPI                          1


//PGR                                                
#define  GPC_PGR_IPUPG_PG_NEVER                      0
#define  GPC_PGR_IPUPG_PG_WAIT                       1
#define  GPC_PGR_IPUPG_PG_DOZE                       2
#define  GPC_PGR_IPUPG_PG_STOP                       3

#define  GPC_PGR_VPUPG_PG_NEVER                      0
#define  GPC_PGR_VPUPG_PG_WAIT                       1
#define  GPC_PGR_VPUPG_PG_DOZE                       2
#define  GPC_PGR_VPUPG_PG_STOP                       3

#define  GPC_PGR_ARMPG_PG_NEVER                      0
#define  GPC_PGR_ARMPG_PG_WAIT                       1
#define  GPC_PGR_ARMPG_PG_DOZE                       2
#define  GPC_PGR_ARMPG_PG_STOP                       3
                                             
//VCR                                                 
#define  GPC_VCR_VCNTU_NOT_USED                      0
#define  GPC_VCR_VCNTU_USED                          1

#define  GPC_VCR_VINC_VOLTAGE_DECREASE               0
#define  GPC_VCR_VINC_VOLTAGE_INCREASE               1

//PGC                      
#define  PGC_PGCR_PCR_POWER_ON                             0      
#define  PGC_PGCR_PCR_POWER_OFF                            1     
                                                     
#define  PGC_PGSR_PSR_POWER_ON                             0
#define  PGC_PGSR_PSR_POWER_OFF                            1      
                                                        
//SRPGC REGISTER                                        
#define  SRPGC_SRPGCR_PCR_POWER_ON                         0      
#define  SRPGC_SRPGCR_PCR_POWER_OFF                        1     
                                                     
#define  SRPGC_SRPGSR_PSR_POWER_ON                         0
#define  SRPGC_SRPGSR_PSR_POWER_OFF                        1 
                                                        
//EMPGC REGISTER                                        
#define  EMPGC_EMPGCR_PCR_POWER_ON                         0      
#define  EMPGC_EMPGCR_PCR_POWER_OFF                        1     
                                                     
#define  EMPGC_EMPGSR_PSR_POWER_ON                         0
#define  EMPGC_EMPGSR_PSR_POWER_OFF                        1 
  
  
//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
  


#ifdef __cplusplus
}
#endif

#endif // __MX51_GPC_H

