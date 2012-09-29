//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#ifndef __IPU_BASE_PRIV_H__
#define __IPU_BASE_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

// Masks for IPU modules
#define IPU_MODULE_CSI0         (CSP_BITFMASK(IPU_IPU_CONF_CSI0_EN))
#define IPU_MODULE_CSI1         (CSP_BITFMASK(IPU_IPU_CONF_CSI1_EN))
#define IPU_MODULE_IC           (CSP_BITFMASK(IPU_IPU_CONF_IC_EN))
#define IPU_MODULE_IRT          (CSP_BITFMASK(IPU_IPU_CONF_IRT_EN))
#define IPU_MODULE_ISP          (CSP_BITFMASK(IPU_IPU_CONF_ISP_EN))
#define IPU_MODULE_DP           (CSP_BITFMASK(IPU_IPU_CONF_DP_EN))
#define IPU_MODULE_DI0          (CSP_BITFMASK(IPU_IPU_CONF_DI0_EN))
#define IPU_MODULE_DI1          (CSP_BITFMASK(IPU_IPU_CONF_DI1_EN))
#define IPU_MODULE_SMFC         (CSP_BITFMASK(IPU_IPU_CONF_SMFC_EN))
#define IPU_MODULE_DC           (CSP_BITFMASK(IPU_IPU_CONF_DC_EN))
#define IPU_MODULE_DMFC         (CSP_BITFMASK(IPU_IPU_CONF_DMFC_EN))
#define IPU_MODULE_SISG         (CSP_BITFMASK(IPU_IPU_CONF_SISG_EN))
#define IPU_MODULE_VDI          (CSP_BITFMASK(IPU_IPU_CONF_VDI_EN))
// The following is a special mask used when it is directly
// requested that the IPU HSP clock be enabled/disabled
#define IPU_DIRECT_REQUEST      (1 << 14)


// Macros for generating 64-bit IRQ masks
#define IPU_INTMASK(intr) (((ULONG) 1)  << intr)


// Masks for IPU submodule interrupt groups
#define PRP_DMA_CHA_MASK       IPU_INTMASK(IPU_INT_IDMAC_12) | IPU_INTMASK(IPU_INT_IDMAC_14) | \
                               IPU_INTMASK(IPU_INT_IDMAC_17)

#define PRP_ENC_DMA_CHA_MASK   IPU_INTMASK(IPU_INT_IDMAC_20)

#define PRP_VF_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_IDMAC_21)

#define PRP_ENC_ROT_DMA_CHA_MASK IPU_INTMASK(IPU_INT_IDMAC_45) | IPU_INTMASK(IPU_INT_IDMAC_48)

#define PRP_VF_ROT_DMA_CHA_MASK  IPU_INTMASK(IPU_INT_IDMAC_46) | IPU_INTMASK(IPU_INT_IDMAC_49)

#define PP_INPUT_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_IDMAC_11)

#define PP_OUTPUT_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_IDMAC_22)

#define PP_ROT_INPUT_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_IDMAC_47)

#define PP_ROT_OUTPUT_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_IDMAC_50)

#define PP_DMA_CHA_MASK        IPU_INTMASK(IPU_INT_IDMAC_11) | IPU_INTMASK(IPU_INT_IDMAC_15) | \
                               IPU_INTMASK(IPU_INT_IDMAC_18) | IPU_INTMASK(IPU_INT_IDMAC_22)

#define PP_ROT_DMA_CHA_MASK    IPU_INTMASK(IPU_INT_IDMAC_47) | IPU_INTMASK(IPU_INT_IDMAC_50)

#define DP_DMA_CHA_MASK_LOWER  IPU_INTMASK(IPU_INT_IDMAC_23) | IPU_INTMASK(IPU_INT_IDMAC_24) | \
                               IPU_INTMASK(IPU_INT_IDMAC_27) | IPU_INTMASK(IPU_INT_IDMAC_29) | \
                               IPU_INTMASK(IPU_INT_IDMAC_31)

#define DP_DMA_CHA_MASK_UPPER  IPU_INTMASK(IPU_INT_IDMAC_33) | IPU_INTMASK(IPU_INT_IDMAC_51) | \
                               IPU_INTMASK(IPU_INT_IDMAC_52)

#define DC_DMA_CHA_MASK_LOWER  IPU_INTMASK(IPU_INT_IDMAC_28)

#define DC_DMA_CHA_MASK_UPPER  IPU_INTMASK(IPU_INT_IDMAC_40) | IPU_INTMASK(IPU_INT_IDMAC_41) | \
                               IPU_INTMASK(IPU_INT_IDMAC_42) | IPU_INTMASK(IPU_INT_IDMAC_43) | \
                               IPU_INTMASK(IPU_INT_IDMAC_44)

#define SMFC0_DMA_CHA_MASK     IPU_INTMASK(IPU_INT_IDMAC_0)

#define SMFC1_DMA_CHA_MASK     IPU_INTMASK(IPU_INT_IDMAC_1)

#define SMFC2_DMA_CHA_MASK     IPU_INTMASK(IPU_INT_IDMAC_2)

#define SMFC3_DMA_CHA_MASK     IPU_INTMASK(IPU_INT_IDMAC_3)

#define VDI_DMA_CHA_MASK       IPU_INTMASK(IPU_INT_IDMAC_8) | IPU_INTMASK(IPU_INT_IDMAC_9) | \
                               IPU_INTMASK(IPU_INT_IDMAC_10) | IPU_INTMASK(IPU_INT_IDMAC_13) 

#define SNOOPING_INTR_MASK                       0x3
#define DP_FLOW_INTR_MASK                       0xFC
#define DC_FRAME_COMPLETE_INTR_MASK           0x3F00
#define DI_VSYNC_PRE_INTR_MASK                0xC000
#define DI_COUNTER_INTR_MASK              0xDFF80000

#define DC_TEARING_ERROR_INTR_MASK           0x70000
#define DI_SYNC_ERROR_INTR_MASK             0x180000
#define DI_TIMEOUT_ERROR_INTR_MASK          0x600000
#define IC_FRAME_LOST_ERROR_INTR_MASK      0x7000000
#define IC_AXI_ERROR_INTR_MASK            0x60000000

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNCTION        3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNCTION      (1 << ZONEID_FUNCTION)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNCTION          DEBUGZONE(ZONEID_FUNCTION)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

#endif  // DEBUG

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Functions
BOOL InitIPU(void);
void DeinitIPU(void);
BOOL IpuAlloc(void);
void IpuDealloc(void);
BOOL IpuIntrInit(void);
void IpuIntrThread(LPVOID);
void IpuISRLoop(UINT32);
void IpuModuleEnable(DWORD);
void IpuModuleDisable(DWORD);
void EnableCSI0();
void DisableCSI0();
void EnableCSI1();
void DisableCSI1();
void EnableIC(IPU_DRIVER);
void DisableIC(IPU_DRIVER);
void EnableIRT(IPU_DRIVER);
void DisableIRT(IPU_DRIVER);
void EnableISP();
void DisableISP();
void EnableDP();
void DisableDP();
void EnableDI0();
void DisableDI0();
void EnableDI1();
void DisableDI1();
void EnableDC();
void DisableDC();
void EnableDMFC();
void DisableDMFC();
void EnableSMFC();
void DisableSMFC();
void EnableSISG();
void DisableSISG();
void EnableVDI();
void DisableVDI();
void EnableHSPClock();
void DisableHSPClock();
void IPUAllocateBuffer(pIPUBufferInfo, IpuBuffer*);
void IPUDeallocateBuffer(IpuBuffer*);

BOOL GetVMemSizeFromRegistry(VOID);
BOOL IpuBufferManagerInit(BOOL bCachedWT);
void IpuBufferManagerDeinit(void);
DWORD IPUGetVideoMemorySize();
DWORD IPUGetVideoMemoryBase();

#ifdef __cplusplus
}
#endif

#endif //__IPU_BASE_PRIV_H__
