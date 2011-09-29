/*****************************************************************************
   
    Copyright (c) 2005, SMSC. All rights reserved.
     
    File name   : lan9217.c 
    Description : LAN9217 EBOOT driver

    History     :
        ver 0.1     12/01/05    WH
            First WindowsCE 5.0 EBOOT driver for LAN9118

        ver 0.2     12/16/05    WH
            Work with VMINI

        ver 0.3     09/06/06    WH,NL 
            Minor changes to the RX Initialization

        ver 0.4     02/06/07    NL
            First WinCE 6.0 EBOOT Driver
*****************************************************************************/
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <ethdbg.h>
#include <oal.h>
#pragma warning(pop)

//------------------------------------------------------------------------------
// Types

// Prototypes for Ethernet controller I/O functions
typedef void (*PFN_EDBG_SETREGDW)(const UINT32 dwBase, const UINT32 dwOffset, 
                                    const UINT32 dwVal);
typedef UINT32 (*PFN_EDBG_GETREGDW)(const UINT32 dwBase, const UINT32 dwOffset);
typedef void (*PFN_EDBG_READFIFO)(const UINT32 dwBase, const UINT32 dwOffset, 
                                    UINT32 *pdwBuf, UINT32 dwDwordCount);
typedef void (*PFN_EDBG_WRITEFIFO)(const UINT32 dwBase, const UINT32 dwOffset, 
                                      const UINT32 *pdwBuf, UINT32 dwDwordCount);


//------------------------------------------------------------------------------
// LAN911X Defines 
#undef OALMSG
#undef OALMSGS

#define OALMSG(cond, exp)   ((void)((1)?(OALLog exp), TRUE : FALSE))
#define OALMSGS(cond, exp)  ((void)((1)?(OALLogSerial exp), TRUE : FALSE))

#define SetRegDW    g_pfnEdbgSetRegDW
#define GetRegDW    g_pfnEdbgGetRegDW
#define ReadFifo    g_pfnEdbgReadFIFO
#define WriteFifo   g_pfnEdbgWriteFIFO

/*
 ****************************************************************************
 ****************************************************************************
 *  TX/RX FIFO Port Register (Direct Address)
 *  Offset (from Base Address)
 *  and bit definitions
 ****************************************************************************
 ****************************************************************************
 */
#define RX_DATA_FIFO_PORT   (0x00UL)
#define TX_DATA_FIFO_PORT   (0x20UL)
    #define TX_CMD_A_INT_ON_COMP_       (0x80000000UL)
    #define TX_CMD_A_INT_BUF_END_ALGN_  (0x03000000UL)
    #define TX_CMD_A_INT_4_BYTE_ALGN_   (0x00000000UL)
    #define TX_CMD_A_INT_16_BYTE_ALGN_  (0x01000000UL)
    #define TX_CMD_A_INT_32_BYTE_ALGN_  (0x02000000UL)
    #define TX_CMD_A_INT_DATA_OFFSET_   (0x001F0000UL)
    #define TX_CMD_A_INT_FIRST_SEG_     (0x00002000UL)
    #define TX_CMD_A_INT_LAST_SEG_      (0x00001000UL)
    #define TX_CMD_A_BUF_SIZE_          (0x000007FFUL)

    #define TX_CMD_B_PKT_TAG_           (0xFFFF0000UL)
    #define TX_CMD_B_ADD_CRC_DISABLE_   (0x00002000UL)
    #define TX_CMD_B_DISABLE_PADDING_   (0x00001000UL)
    #define TX_CMD_B_PKT_BYTE_LENGTH_   (0x000007FFUL)

#define RX_STATUS_FIFO_PORT (0x00000040UL)
#define RX_STS_ES           (0x00008000UL)
#define RX_STS_LENGTH_ERR   (0x00001000UL)
#define RX_STS_MULTICAST    (0x00000400UL)
#define RX_STS_FRAME_TYPE   (0x00000020UL)
#define RX_FIFO_PEEK        (0x00000044UL)

#define TX_STS_ES           (0x00008000UL)

#define TX_STATUS_FIFO_PORT (0x00000048UL)
#define TX_FIFO_PEEK        (0x0000004CUL)

/*
 ****************************************************************************
 ****************************************************************************
 *  Slave Interface Module Control and Status Register (Direct Address)
 *  Offset (from Base Address)
 *  and bit definitions
 ****************************************************************************
 ****************************************************************************
 */
#define ID_REV              (0x50UL)
    #define ID_REV_CHIP_ID_     (0xFFFF0000UL)  // RO   default 0x011X
    #define ID_REV_REV_ID_      (0x0000FFFFUL)  // RO

#define INT_CFG             (0x54UL)
    #define INT_CFG_INT_DEAS_   (0xFF000000UL)  // R/W
    #define INT_CFG_IRQ_INT_    (0x00001000UL)  // RO
    #define INT_CFG_IRQ_EN_     (0x00000100UL)  // R/W
    #define INT_CFG_IRQ_POL_    (0x00000010UL)  // R/W Not Affected by SW Reset
    #define INT_CFG_IRQ_TYPE_   (0x00000001UL)  // R/W Not Affected by SW Reset
    #define INT_CFG_IRQ_RESERVED_   (0x00FFCEEEUL)  //Reserved bits mask

#define INT_STS             (0x58UL)
    #define INT_STS_SW_INT_     (0x80000000UL)  // R/WC
    #define INT_STS_TXSTOP_INT_ (0x02000000UL)  // R/WC
    #define INT_STS_RXSTOP_INT_ (0x01000000UL)  // R/WC
    #define INT_STS_RXDFH_INT_  (0x00800000UL)  // R/WC
    #define INT_STS_RXDF_INT_   (0x00400000UL)  // R/WC
    #define INT_STS_TX_IOC_     (0x00200000UL)  // R/WC
    #define INT_STS_RXD_INT_    (0x00100000UL)  // R/WC
    #define INT_STS_GPT_INT_    (0x00080000UL)  // R/WC
    #define INT_STS_PHY_INT_    (0x00040000UL)  // RO
    #define INT_STS_PME_INT_    (0x00020000UL)  // R/WC
    #define INT_STS_TXSO_       (0x00010000UL)  // R/WC
    #define INT_STS_RWT_        (0x00008000UL)  // R/WC
    #define INT_STS_RXE_        (0x00004000UL)  // R/WC
    #define INT_STS_TXE_        (0x00002000UL)  // R/WC
    #define INT_STS_ERX_        (0x00001000UL)  // R/WC
    #define INT_STS_TDFU_       (0x00000800UL)  // R/WC
    #define INT_STS_TDFO_       (0x00000400UL)  // R/WC
    #define INT_STS_TDFA_       (0x00000200UL)  // R/WC
    #define INT_STS_TSFF_       (0x00000100UL)  // R/WC
    #define INT_STS_TSFL_       (0x00000080UL)  // R/WC
    #define INT_STS_RDFO_       (0x00000040UL)  // R/WC
    #define INT_STS_RDFL_       (0x00000020UL)  // R/WC
    #define INT_STS_RSFF_       (0x00000010UL)  // R/WC
    #define INT_STS_RSFL_       (0x00000008UL)  // R/WC
    #define INT_STS_GPIO2_INT_  (0x00000004UL)  // R/WC
    #define INT_STS_GPIO1_INT_  (0x00000002UL)  // R/WC
    #define INT_STS_GPIO0_INT_  (0x00000001UL)  // R/WC

#define INT_EN              (0x5CUL)
    #define INT_EN_SW_INT_EN_       (0x80000000UL)  // R/W
    #define INT_EN_TXSTOP_INT_EN_   (0x02000000UL)  // R/W
    #define INT_EN_RXSTOP_INT_EN_   (0x01000000UL)  // R/W
    #define INT_EN_RXDFH_INT_EN_    (0x00800000UL)  // R/W
    #define INT_EN_RXDF_INT_EN_     (0x00400000UL)  // R/W
    #define INT_EN_TIOC_INT_EN_     (0x00200000UL)  // R/W
    #define INT_EN_RXD_INT_EN_      (0x00100000UL)  // R/W
    #define INT_EN_GPT_INT_EN_      (0x00080000UL)  // R/W
    #define INT_EN_PHY_INT_EN_      (0x00040000UL)  // R/W
    #define INT_EN_PME_INT_EN_      (0x00020000UL)  // R/W
    #define INT_EN_TXSO_EN_         (0x00010000UL)  // R/W
    #define INT_EN_RWT_EN_          (0x00008000UL)  // R/W
    #define INT_EN_RXE_EN_          (0x00004000UL)  // R/W
    #define INT_EN_TXE_EN_          (0x00002000UL)  // R/W
    #define INT_EN_ERX_EN_          (0x00001000UL)  // R/W
    #define INT_EN_TDFU_EN_         (0x00000800UL)  // R/W
    #define INT_EN_TDFO_EN_         (0x00000400UL)  // R/W
    #define INT_EN_TDFA_EN_         (0x00000200UL)  // R/W
    #define INT_EN_TSFF_EN_         (0x00000100UL)  // R/W
    #define INT_EN_TSFL_EN_         (0x00000080UL)  // R/W
    #define INT_EN_RDFO_EN_         (0x00000040UL)  // R/W
    #define INT_EN_RDFL_EN_         (0x00000020UL)  // R/W
    #define INT_EN_RSFF_EN_         (0x00000010UL)  // R/W
    #define INT_EN_RSFL_EN_         (0x00000008UL)  // R/W
    #define INT_EN_GPIO2_INT_       (0x00000004UL)  // R/W
    #define INT_EN_GPIO1_INT_       (0x00000002UL)  // R/W
    #define INT_EN_GPIO0_INT_       (0x00000001UL)  // R/W

#define DMA_CFG             (0x60UL)
    #define DMA_CFG_DRQ1_DEAS_      (0xFF000000UL)  // R/W
    #define DMA_CFG_DMA1_MODE_      (0x00200000UL)  // R/W
    #define DMA_CFG_DMA1_FUNC_      (0x00180000UL)  // R/W
        #define DMA_CFG_DMA1_FUNC_DISABLED_     (0x000000000UL) // R/W
        #define DMA_CFG_DMA1_FUNC_TX_DMA_       (0x000800000UL) // R/W
        #define DMA_CFG_DMA1_FUNC_RX_DMA_       (0x001000000UL) // R/W
    #define DMA_CFG_DRQ1_BUFF_      (0x00040000UL)  // R/W
    #define DMA_CFG_DRQ1_POL_       (0x00020000UL)  // R/W
    #define DMA_CFG_DAK1_POL_       (0x00010000UL)  // R/W
    #define DMA_CFG_DRQ0_DEAS_      (0x0000FF00UL)  // R/W
    #define DMA_CFG_DMA0_MODE_      (0x00000020UL)  // R/W
    #define DMA_CFG_DMA0_FUNC_      (0x00000018UL)  // R/W
        #define DMA_CFG_DMA0_FUNC_FIFO_SEL_     (0x000000000UL) // R/W
        #define DMA_CFG_DMA0_FUNC_TX_DMA_       (0x000000008UL) // R/W
        #define DMA_CFG_DMA0_FUNC_RX_DMA_       (0x000000010UL) // R/W
    #define DMA_CFG_DRQ0_BUFF_      (0x00000004UL)  // R/W
    #define DMA_CFG_DRQ0_POL_       (0x00000002UL)  // R/W
    #define DMA_CFG_DAK0_POL_       (0x00000001UL)  // R/W

#define BYTE_TEST           (0x64UL)    // RO default 0x87654321

#define FIFO_INT            (0x68UL)
    #define FIFO_INT_TX_AVAIL_LEVEL_    (0xFF000000UL)  // R/W
    #define FIFO_INT_TX_STS_LEVEL_      (0x00FF0000UL)  // R/W
    #define FIFO_INT_RX_AVAIL_LEVEL_    (0x0000FF00UL)  // R/W
    #define FIFO_INT_RX_STS_LEVEL_      (0x000000FFUL)  // R/W

#define RX_CFG              (0x6CUL)
    #define RX_CFG_RX_END_ALGN_     (0xC0000000UL)  // R/W
        #define RX_CFG_RX_END_ALGN4_        (0x00000000UL)  // R/W
        #define RX_CFG_RX_END_ALGN16_       (0x40000000UL)  // R/W
        #define RX_CFG_RX_END_ALGN32_       (0x80000000UL)  // R/W
    #define RX_CFG_RX_DMA_CNT_      (0x0FFF0000UL)  // R/W
    #define RX_CFG_RX_DUMP_         (0x00008000UL)  // R/W
    #define RX_CFG_RXDOFF_          (0x00001F00UL)  // R/W
    #define RX_CFG_RXBAD_           (0x00000001UL)  // R/W

#define TX_CFG              (0x70UL)
    #define TX_CFG_TX_DMA_LVL_      (0xE0000000UL)  // R/W
    #define TX_CFG_TX_DMA_CNT_      (0x0FFF0000UL)  // R/W Self Clearing
    #define TX_CFG_TXS_DUMP_        (0x00008000UL)  // Self Clearing
    #define TX_CFG_TXD_DUMP_        (0x00004000UL)  // Self Clearing
    #define TX_CFG_TXSAO_           (0x00000004UL)  // R/W
    #define TX_CFG_TX_ON_           (0x00000002UL)  // R/W
    #define TX_CFG_STOP_TX_         (0x00000001UL)  // Self Clearing

#define HW_CFG              (0x74UL)
    #define HW_CFG_TTM_             (0x00200000UL)  // R/W
    #define HW_CFG_SF_              (0x00100000UL)  // R/W
    #define HW_CFG_TX_FIF_SZ_       (0x000F0000UL)  // R/W
    #define HW_CFG_TR_              (0x00003000UL)  // R/W
    #define HW_CFG_PHY_CLK_SEL_     (0x00000060UL)  // R/W
    #define   HW_CFG_PHY_CLK_SEL_INT_PHY_   (0x00000000UL)  // R/W
    #define   HW_CFG_PHY_CLK_SEL_EXT_PHY_   (0x00000020UL)  // R/W
    #define   HW_CFG_PHY_CLK_SEL_CLK_DIS_   (0x00000040UL)  // R/W
    #define HW_CFG_SMI_SEL_         (0x00000010UL)  // R/W
    #define HW_CFG_EXT_PHY_DET_     (0x00000008UL)  // RO
    #define HW_CFG_EXT_PHY_EN_      (0x00000004UL)  // R/W
    #define HW_CFG_SRST_TO_         (0x00000002UL)  // RO
    #define HW_CFG_SRST_            (0x00000001UL)  // Self Clearing

#define RX_DP_CTL           (0x78UL)
    #define RX_DP_CTL_FFWD_BUSY_    (0x80000000UL)  // R/?
    #define RX_DP_CTL_RX_FFWD_      (0x00000FFFUL)  // R/W

#define RX_FIFO_INF         (0x7CUL)
    #define RX_FIFO_INF_RXSUSED_    (0x00FF0000UL)  // RO
    #define RX_FIFO_INF_RXDUSED_    (0x0000FFFFUL)  // RO

#define TX_FIFO_INF         (0x80UL)
    #define TX_FIFO_INF_TSFREE_     (0x00FF0000UL)  // RO for PAS V.1.3
    #define TX_FIFO_INF_TSUSED_     (0x00FF0000UL)  // RO for PAS V.1.4
    #define TX_FIFO_INF_TDFREE_     (0x0000FFFFUL)  // RO

#define PMT_CTRL            (0x84UL)
    #define PMT_CTRL_PM_MODE_           (0x00003000UL)  // Self Clearing
        #define PMT_CTRL_PM_MODE_GP_        (0x00003000UL)  // Self Clearing
        #define PMT_CTRL_PM_MODE_ED_        (0x00002000UL)  // Self Clearing
        #define PMT_CTRL_PM_MODE_WOL_       (0x00001000UL)  // Self Clearing
    #define PMT_CTRL_PHY_RST_           (0x00000400UL)  // Self Clearing
    #define PMT_CTRL_WOL_EN_            (0x00000200UL)  // R/W
    #define PMT_CTRL_ED_EN_             (0x00000100UL)  // R/W
    #define PMT_CTRL_PME_TYPE_          (0x00000040UL)  // R/W Not Affected by SW Reset
    #define PMT_CTRL_WUPS_              (0x00000030UL)  // R/WC
        #define PMT_CTRL_WUPS_NOWAKE_       (0x00000000UL)  // R/WC
        #define PMT_CTRL_WUPS_ED_           (0x00000010UL)  // R/WC
        #define PMT_CTRL_WUPS_WOL_          (0x00000020UL)  // R/WC
        #define PMT_CTRL_WUPS_MULTI_        (0x00000030UL)  // R/WC
    #define PMT_CTRL_PME_IND_       (0x00000008UL)  // R/W
    #define PMT_CTRL_PME_POL_       (0x00000004UL)  // R/W
    #define PMT_CTRL_PME_EN_        (0x00000002UL)  // R/W Not Affected by SW Reset
    #define PMT_CTRL_READY_         (0x00000001UL)  // RO

#define GPIO_CFG            (0x88UL)
    #define GPIO_CFG_LED3_EN_       (0x40000000UL)  // R/W
    #define GPIO_CFG_LED2_EN_       (0x20000000UL)  // R/W
    #define GPIO_CFG_LED1_EN_       (0x10000000UL)  // R/W
    #define GPIO_CFG_GPIO2_INT_POL_ (0x04000000UL)  // R/W
    #define GPIO_CFG_GPIO1_INT_POL_ (0x02000000UL)  // R/W
    #define GPIO_CFG_GPIO0_INT_POL_ (0x01000000UL)  // R/W
    #define GPIO_CFG_EEPR_EN_       (0x00700000UL)  // R/W
    #define GPIO_CFG_GPIOBUF2_      (0x00040000UL)  // R/W
    #define GPIO_CFG_GPIOBUF1_      (0x00020000UL)  // R/W
    #define GPIO_CFG_GPIOBUF0_      (0x00010000UL)  // R/W
    #define GPIO_CFG_GPIODIR2_      (0x00000400UL)  // R/W
    #define GPIO_CFG_GPIODIR1_      (0x00000200UL)  // R/W
    #define GPIO_CFG_GPIODIR0_      (0x00000100UL)  // R/W
    #define GPIO_CFG_GPIOD4_        (0x00000020UL)  // R/W
    #define GPIO_CFG_GPIOD3_        (0x00000010UL)  // R/W
    #define GPIO_CFG_GPIOD2_        (0x00000004UL)  // R/W
    #define GPIO_CFG_GPIOD1_        (0x00000002UL)  // R/W
    #define GPIO_CFG_GPIOD0_        (0x00000001UL)  // R/W

#define GPT_CFG             (0x8CUL)
    #define GPT_CFG_TIMER_EN_       (0x20000000UL)  // R/W
    #define GPT_CFG_GPT_LOAD_       (0x0000FFFFUL)  // R/W

#define GPT_CNT             (0x90UL)
    #define GPT_CNT_GPT_CNT_        (0x0000FFFFUL)  // RO

#define FPGA_REV            (0x94UL)
    #define FPGA_REV_FPGA_REV_      (0x0000FFFFUL)  // RO

#define ENDIAN              (0x98UL)    // R/W Not Affected by SW Reset

#define FREE_RUN            (0x9CUL)    // RO

#define RX_DROP             (0xA0UL)    // R/WC

#define MAC_CSR_CMD         (0xA4UL)
    #define MAC_CSR_CMD_CSR_BUSY_   (0x80000000UL)  // Self Clearing
    #define MAC_CSR_CMD_R_NOT_W_    (0x40000000UL)  // R/W
    #define MAC_CSR_CMD_CSR_ADDR_   (0x000000FFUL)  // R/W

#define MAC_CSR_DATA        (0xA8UL)    // R/W

#define AFC_CFG             (0xACUL)
    #define AFC_CFG_AFC_HI_         (0x00FF0000UL)  // R/W
    #define AFC_CFG_AFC_LO_         (0x0000FF00UL)  // R/W
    #define AFC_CFG_BACK_DUR_       (0x000000F0UL)  // R/W
    #define AFC_CFG_FCMULT_         (0x00000008UL)  // R/W
    #define AFC_CFG_FCBRD_          (0x00000004UL)  // R/W
    #define AFC_CFG_FCADD_          (0x00000002UL)  // R/W
    #define AFC_CFG_FCANY_          (0x00000001UL)  // R/W

#define E2P_CMD             (0xB0UL)
    #define E2P_CMD_EPC_BUSY_       (0x80000000UL)  // Self Clearing
    #define E2P_CMD_EPC_CMD_        (0x70000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_READ_   (0x00000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_EWDS_   (0x10000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_EWEN_   (0x20000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_WRITE_  (0x30000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_WRAL_   (0x40000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_ERASE_  (0x50000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_ERAL_   (0x60000000UL)  // R/W
        #define E2P_CMD_EPC_CMD_RELOAD_ (0x70000000UL)  // R/W
    #define E2P_CMD_EPC_TIMEOUT_    (0x08000000UL)  // RO
    #define E2P_CMD_E2P_PROG_GO_    (0x00000200UL)  // WO
    #define E2P_CMD_E2P_PROG_DONE_  (0x00000100UL)  // RO
    #define E2P_CMD_EPC_ADDR_       (0x000000FFUL)  // R/W

#define E2P_DATA            (0xB4UL)
    #define E2P_DATA_EEPROM_DATA_   (0x000000FFUL)  // R/W

#define TEST_REG_A          (0xC0UL)
    #define TEST_REG_A_FR_CNT_BYP_  (0x00000008UL)  // R/W
    #define TEST_REG_A_PME50M_BYP_  (0x00000004UL)  // R/W
    #define TEST_REG_A_PULSE_BYP_   (0x00000002UL)  // R/W
    #define TEST_REG_A_PS_BYP_      (0x00000001UL)  // R/W

#define LAN_REGISTER_EXTENT         (0x00000100UL)
/*
 ****************************************************************************
 ****************************************************************************
 *  MAC Control and Status Register (Indirect Address)
 *  Offset (through the MAC_CSR CMD and DATA port)
 ****************************************************************************
 ****************************************************************************
 *
 */
#define MAC_CR              (0x01UL)    // R/W

    /* MAC_CR - MAC Control Register */
    #define MAC_CR_RXALL_       (0x80000000UL)
    #define MAC_CR_HBDIS_       (0x10000000UL)
    #define MAC_CR_RCVOWN_      (0x00800000UL)
    #define MAC_CR_LOOPBK_      (0x00200000UL)
    #define MAC_CR_FDPX_        (0x00100000UL)
    #define MAC_CR_MCPAS_       (0x00080000UL)
    #define MAC_CR_PRMS_        (0x00040000UL)
    #define MAC_CR_INVFILT_     (0x00020000UL)
    #define MAC_CR_PASSBAD_     (0x00010000UL)
    #define MAC_CR_HFILT_       (0x00008000UL)
    #define MAC_CR_HPFILT_      (0x00002000UL)
    #define MAC_CR_LCOLL_       (0x00001000UL)
    #define MAC_CR_BCAST_       (0x00000800UL)
    #define MAC_CR_DISRTY_      (0x00000400UL)
    #define MAC_CR_PADSTR_      (0x00000100UL)
    #define MAC_CR_BOLMT_MASK_  (0x000000C0UL)
    #define MAC_CR_DFCHK_       (0x00000020UL)
    #define MAC_CR_TXEN_        (0x00000008UL)
    #define MAC_CR_RXEN_        (0x00000004UL)

#define ADDRH               (0x02UL)    // R/W mask 0x0000FFFFUL
#define ADDRL               (0x03UL)    // R/W mask 0xFFFFFFFFUL
#define HASHH               (0x04UL)    // R/W
#define HASHL               (0x05UL)    // R/W

#define MII_ACC             (0x06UL)    // R/W
    #define MII_ACC_PHY_ADDR_   (0x0000F800UL)
    #define MII_ACC_MIIRINDA_   (0x000007C0UL)
    #define MII_ACC_MII_WRITE_  (0x00000002UL)
    #define MII_ACC_MII_BUSY_   (0x00000001UL)

#define MII_DATA            (0x07UL)    // R/W mask 0x0000FFFFUL

#define FLOW                (0x08UL)    // R/W
    #define FLOW_FCPT_          (0xFFFF0000UL)
    #define FLOW_FCPASS_        (0x00000004UL)
    #define FLOW_FCEN_          (0x00000002UL)
    #define FLOW_FCBSY_         (0x00000001UL)

#define VLAN1               (0x09UL)    // R/W mask 0x0000FFFFUL
#define VLAN2               (0x0AUL)    // R/W mask 0x0000FFFFUL

#define WUFF                (0x0BUL)    // WO
    #define FILTER3_COMMAND     (0x0F000000UL)
    #define FILTER2_COMMAND     (0x000F0000UL)
    #define FILTER1_COMMAND     (0x00000F00UL)
    #define FILTER0_COMMAND     (0x0000000FUL)
        #define FILTER3_ADDR_TYPE     (0x04000000UL)
        #define FILTER3_ENABLE     (0x01000000UL)
        #define FILTER2_ADDR_TYPE     (0x00040000UL)
        #define FILTER2_ENABLE     (0x00010000UL)
        #define FILTER1_ADDR_TYPE     (0x00000400UL)
        #define FILTER1_ENABLE     (0x00000100UL)
        #define FILTER0_ADDR_TYPE     (0x00000004UL)
        #define FILTER0_ENABLE     (0x00000001UL)
    #define FILTER3_OFFSET      (0xFF000000UL)
    #define FILTER2_OFFSET      (0x00FF0000UL)
    #define FILTER1_OFFSET      (0x0000FF00UL)
    #define FILTER0_OFFSET      (0x000000FFUL)

    #define FILTER3_CRC         (0xFFFF0000UL)
    #define FILTER2_CRC         (0x0000FFFFUL)
    #define FILTER1_CRC         (0xFFFF0000UL)
    #define FILTER0_CRC         (0x0000FFFFUL)

#define WUCSR               (0x0CUL)    // R/W
    #define WUCSR_GUE_          (0x00000200UL)
    #define WUCSR_WUFR_         (0x00000040UL)
    #define WUCSR_MPR_          (0x00000020UL)
    #define WUCSR_WAKE_EN_      (0x00000004UL)
    #define WUCSR_MPEN_         (0x00000002UL)


/*
 ****************************************************************************
 *  Chip Specific MII Defines
 ****************************************************************************
 *
 *  Phy register offsets and bit definitions
 *
 */
#define LAN911X_PHY_ID  (0x00C0001CUL)

#define PHY_BCR     ((UINT32)0U)
#define PHY_BCR_RESET_              ((UINT16)0x8000U)
#define PHY_BCR_SPEED_SELECT_       ((UINT16)0x2000U)
#define PHY_BCR_AUTO_NEG_ENABLE_    ((UINT16)0x1000U)
#define PHY_BCR_POWER_DOWN_         ((UINT16)0x0800U)
#define PHY_BCR_RESTART_AUTO_NEG_   ((UINT16)0x0200U)
#define PHY_BCR_DUPLEX_MODE_        ((UINT16)0x0100U)

#define PHY_BSR     ((UINT32)1U)
    #define PHY_BSR_LINK_STATUS_    ((UINT16)0x0004U)
    #define PHY_BSR_REMOTE_FAULT_   ((UINT16)0x0010U)
    #define PHY_BSR_AUTO_NEG_COMP_  ((UINT16)0x0020U)
    #define PHY_BSR_ANEG_ABILITY_   ((UINT16)0x0008U)

#define PHY_ID_1    ((UINT32)2U)
#define PHY_ID_2    ((UINT32)3U)

#define PHY_ANEG_ADV    ((UINT32)4U)
#define PHY_ANEG_ADV_PAUSE_OP_      ((UINT16)0x0C00)
#define PHY_ANEG_ADV_ASYM_PAUSE_    ((UINT16)0x0800)
#define PHY_ANEG_ADV_SYM_PAUSE_     ((UINT16)0x0400)
#define PHY_ANEG_ADV_10H_           ((UINT16)0x0020)
#define PHY_ANEG_ADV_10F_           ((UINT16)0x0040)
#define PHY_ANEG_ADV_100H_          ((UINT16)0x0080)
#define PHY_ANEG_ADV_100F_          ((UINT16)0x0100)
#define PHY_ANEG_ADV_SPEED_         ((UINT16)0x01E0)

#define PHY_ANEG_LPA    ((UINT32)5U)
#define PHY_ANEG_LPA_100FDX_    ((UINT16)0x0100)
#define PHY_ANEG_LPA_100HDX_    ((UINT16)0x0080)
#define PHY_ANEG_LPA_10FDX_     ((UINT16)0x0040)
#define PHY_ANEG_LPA_10HDX_     ((UINT16)0x0020)

#define PHY_ANEG_EXP    ((UINT32)6U)
#define PHY_ANEG_EXP_PDF_           ((UINT16)0x0010)
#define PHY_ANEG_EXP_LPNPA_         ((UINT16)0x0008)
#define PHY_ANEG_EXP_NPA_           ((UINT16)0x0004)
#define PHY_ANEG_EXP_PAGE_RX_       ((UINT16)0x0002)
#define PHY_ANEG_EXP_LPANEG_ABLE_   ((UINT16)0x0001)

#define PHY_MODE_CTRL_STS       ((UINT32)17) // Mode Control/Status Register
    #define MODE_CTRL_STS_FASTRIP_      ((UINT16)0x4000U)
    #define MODE_CTRL_STS_EDPWRDOWN_    ((UINT16)0x2000U)
    #define MODE_CTRL_STS_LOWSQEN_      ((UINT16)0x0800U)
    #define MODE_CTRL_STS_MDPREBP_      ((UINT16)0x0400U)
    #define MODE_CTRL_STS_FARLOOPBACK_  ((UINT16)0x0200U)
    #define MODE_CTRL_STS_FASTEST_      ((UINT16)0x0100U)
    #define MODE_CTRL_STS_REFCLKEN_     ((UINT16)0x0010U)
    #define MODE_CTRL_STS_PHYADBP_      ((UINT16)0x0008U)
    #define MODE_CTRL_STS_FORCE_G_LINK_ ((UINT16)0x0004U)
    #define MODE_CTRL_STS_ENERGYON_     ((UINT16)0x0002U)

#define PHY_INT_SRC         ((UINT32)29)
#define PHY_INT_SRC_ENERGY_ON_          ((UINT16)0x0080U)
#define PHY_INT_SRC_ANEG_COMP_          ((UINT16)0x0040U)
#define PHY_INT_SRC_REMOTE_FAULT_       ((UINT16)0x0020U)
#define PHY_INT_SRC_LINK_DOWN_          ((UINT16)0x0010U)

#define PHY_INT_MASK        ((UINT32)30)
#define PHY_INT_MASK_ENERGY_ON_     ((UINT16)0x0080U)
#define PHY_INT_MASK_ANEG_COMP_     ((UINT16)0x0040U)
#define PHY_INT_MASK_REMOTE_FAULT_  ((UINT16)0x0020U)
#define PHY_INT_MASK_LINK_DOWN_     ((UINT16)0x0010U)

#define PHY_SPECIAL         ((UINT32)31)
#define PHY_SPECIAL_SPD_    ((UINT16)0x001CU)
#define PHY_SPECIAL_SPD_10HALF_     ((UINT16)0x0004U)
#define PHY_SPECIAL_SPD_10FULL_     ((UINT16)0x0014U)
#define PHY_SPECIAL_SPD_100HALF_    ((UINT16)0x0008U)
#define PHY_SPECIAL_SPD_100FULL_    ((UINT16)0x0018U)

#define TIMEOUT_VALUE           2000        // 2 seconds.

#define LINK_NO_LINK            (0UL)
#define LINK_10MPS_HALF         (1UL)
#define LINK_10MPS_FULL         (2UL)
#define LINK_100MPS_HALF        (3UL)
#define LINK_100MPS_FULL        (4UL)


//------------------------------------------------------------------------------
// Local Variables 
static const char date_code[] = "020607";
static UINT32 g_pLAN911x;
static UINT32 dwPhyAddr;
static UINT32 g_chipRevision;

static UINT32 g_etherTimerFreqKhz;


BOOL   LAN911xInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3]);
UINT16 LAN911xSendFrame(UINT8 *pBuffer, UINT32 length);
UINT16 LAN911xGetFrame(UINT8 *pBuffer, UINT16 *pLength);
VOID   LAN911xEnableInts();
VOID   LAN911xDisableInts();
VOID   LAN911xCurrentPacketFilter(UINT32 filter);
BOOL   LAN911xMulticastList(UINT8 *pAddresses, UINT32 count);

void SetRegDW_32bit(const UINT32 dwBase, const UINT32 dwOffset, const UINT32 dwVal)
{
    (*(volatile UINT32 *)(dwBase + dwOffset)) = dwVal;
}

void SetRegDW_16bit(const UINT32 dwBase, const UINT32 dwOffset, const UINT32 dwVal)
{
    (*(volatile UINT16 *)(dwBase + dwOffset )) =  (UINT16)  (dwVal & 0xFFFF);
    (*(volatile UINT16 *)(dwBase + dwOffset + 2)) = (UINT16) ((dwVal >> 16) & 0xFFFF);
}

UINT32 GetRegDW_32bit(const UINT32 dwBase, const UINT32 dwOffset)
{
    return (UINT32)(*(volatile UINT32 *)(dwBase + dwOffset));
}

UINT32 GetRegDW_16bit(const UINT32 dwBase, const UINT32 dwOffset)
{
       UINT16 hi, lo;

       lo = *(volatile UINT16 *)(dwBase + dwOffset);
       hi = *(volatile UINT16 *)(dwBase + dwOffset + 2);

    return (UINT32)((hi << 16) + lo);
}

void WriteFifo_32bit(const UINT32 dwBase, const UINT32 dwOffset, const UINT32 *pdwBuf, UINT32 dwDwordCount)
{
    volatile UINT32 * pdwReg;
    pdwReg = (volatile UINT32 *)(dwBase + dwOffset);
    
    while (dwDwordCount)
    {
        *pdwReg = *pdwBuf++;
        dwDwordCount--;
    }
}

void WriteFifo_16bit(const UINT32 dwBase, const UINT32 dwOffset, const UINT32 *pdwBuf, UINT32 dwDwordCount)
{
    volatile UINT16 *pwRegLo, *pwRegHi, *pwBuf;

    pwRegLo = (volatile UINT16 *)(dwBase + dwOffset);
    pwRegHi = (volatile UINT16 *)(dwBase + dwOffset +2);
       pwBuf = (UINT16*)pdwBuf;
    
    while (dwDwordCount)
    {
        *pwRegLo = *pwBuf++;
        *pwRegHi = *pwBuf++;
        dwDwordCount--;
    }
}


void ReadFifo_32bit(const UINT32 dwBase, const UINT32 dwOffset, UINT32 *pdwBuf, UINT32 dwDwordCount)
{
    const volatile UINT32 * const pdwReg = (const volatile UINT32 * const)(dwBase + dwOffset);
    
    while (dwDwordCount)
    {
        *pdwBuf++ = *pdwReg;
        dwDwordCount--;
    }
}

void ReadFifo_16bit(const UINT32 dwBase, const UINT32 dwOffset, UINT32 *pdwBuf, UINT32 dwDwordCount)
{
    const volatile UINT16 * const pwRegLo = (const volatile UINT16 * const)(dwBase + dwOffset);
    const volatile UINT16 * const pwRegHi = (const volatile UINT16 * const)(dwBase + dwOffset + 2);
       UINT16 *pwBuf = (UINT16*)pdwBuf;
    
    while (dwDwordCount)
    {
        *pwBuf++ = *pwRegLo;
        *pwBuf++ = *pwRegHi;
        dwDwordCount--;
    }
}

// Default Ethernet controller I/O support is for 16-bit local bus.  Users of
// this LIB can override the function pointers if an alternate I/O method is
// required (i.e. SPI-based Ethernet controller).
PFN_EDBG_SETREGDW g_pfnEdbgSetRegDW = (PFN_EDBG_SETREGDW) SetRegDW_16bit;
PFN_EDBG_GETREGDW g_pfnEdbgGetRegDW = (PFN_EDBG_GETREGDW) GetRegDW_16bit;
PFN_EDBG_READFIFO g_pfnEdbgReadFIFO = (PFN_EDBG_READFIFO) ReadFifo_16bit;
PFN_EDBG_WRITEFIFO g_pfnEdbgWriteFIFO = (PFN_EDBG_WRITEFIFO) WriteFifo_16bit;

//------------------------------------------------------------------------------

BOOL Lan_MacNotBusy(const UINT32 dwLanBase)
{
    int i=0;

    // wait for MAC not busy, w/ timeout
    for(i=0;i<40;i++)
    {
        if((GetRegDW(dwLanBase, MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)==(0UL)) {
            return TRUE;
        }
    }

    OALMSGS(OAL_ERROR, (L"timeout waiting for MAC not BUSY. MAC_CSR_CMD = 0x%08lX\n", (UINT32)(*(volatile UINT32 *)(dwLanBase + MAC_CSR_CMD))));
    return FALSE;
}

void Lan_SetMacRegDW(const UINT32 dwLanBase, const UINT32 dwOffset, const UINT32 dwVal)
{
    if (GetRegDW(dwLanBase, MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)
    {
        OALMSGS(OAL_ERROR, (L"LanSetMacRegDW() failed MAC already busy at entry\r\n"));
        return;
    }

    // send the data to write
    SetRegDW(dwLanBase, MAC_CSR_DATA, dwVal);

    // do the actual write
    SetRegDW(dwLanBase, MAC_CSR_CMD, 
        ((dwOffset & 0x000000FFUL) | MAC_CSR_CMD_CSR_BUSY_));

    // wait for the write to complete, w/ timeout
    if (!Lan_MacNotBusy(dwLanBase))
    {
        OALMSGS(OAL_ERROR, (L"LanSetMacRegDW() failed waiting for MAC not busy after write\r\n"));
    }
}

UINT32 Lan_GetMacRegDW(const UINT32 dwLanBase, const UINT32 dwOffset)
{
    UINT32  dwRet;

    // wait until not busy, w/ timeout
    if (GetRegDW(dwLanBase, MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY_)
    {
        OALMSGS(OAL_ERROR, (L"LanGetMacRegDW() failed MAC already busy at entry\n"));
        return 0xFFFFFFFFUL;
    }

    // send the MAC Cmd w/ offset
    SetRegDW(dwLanBase, MAC_CSR_CMD, 
        ((dwOffset & 0x000000FFUL) | MAC_CSR_CMD_CSR_BUSY_ | MAC_CSR_CMD_R_NOT_W_));

    // wait for the read to happen, w/ timeout
    if (!Lan_MacNotBusy(dwLanBase))
    {
        OALMSGS(OAL_ERROR, (L"LanGetMacRegDW() failed waiting for MAC not busy after read\n"));
        dwRet = 0xFFFFFFFFUL;
    }
    else
    {
        // finally, return the read data
        dwRet = GetRegDW(dwLanBase, MAC_CSR_DATA);
    }

    return dwRet;
}

void Lan_SetPhyRegW(const UINT32 dwLanBase, const UINT32 dwPhyAddress, const UINT32 dwMiiIndex, const UINT16 wVal)
{
    UINT32 dwAddr;
    int i=0;

    // confirm MII not busy
    if ((Lan_GetMacRegDW(dwLanBase, MII_ACC) & MII_ACC_MII_BUSY_) != 0UL)
    {
        OALMSGS(OAL_ERROR, (L"MII is busy in MiiGetReg???\r\n"));
        return;
    }

    // put the data to write in the MAC
    Lan_SetMacRegDW(dwLanBase, MII_DATA, (UINT32)wVal);

    // set the address, index & direction (write to PHY)
    dwAddr = ((dwPhyAddress & 0x1FUL)<<11) | ((dwMiiIndex & 0x1FUL)<<6) | MII_ACC_MII_WRITE_;
    Lan_SetMacRegDW(dwLanBase, MII_ACC, dwAddr);

    // wait for write to complete w/ timeout
    for(i=0;i<100;i++) {
        // see if MII is finished yet
        if ((Lan_GetMacRegDW(dwLanBase, MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
        {
            return;
        }
    }

    OALMSGS(OAL_ERROR, (L"timeout waiting for MII write to finish\r\n"));
    return;
}

UINT16 Lan_GetPhyRegW(const UINT32 dwLanBase, const UINT32 dwPhyAddress, const UINT32 dwMiiIndex)
{
    UINT32 dwAddr;
    UINT16 wRet = (UINT16)0xFFFF;
    int i=0;

    // confirm MII not busy
    if ((Lan_GetMacRegDW(dwLanBase, MII_ACC) & MII_ACC_MII_BUSY_) != 0UL)
    {
        OALMSGS(OAL_ERROR, (L"MII is busy in MiiGetReg???\r\n"));
        return (UINT16)0;
    }

    // set the address, index & direction (read from PHY)
    dwAddr = ((dwPhyAddress & 0x1FUL)<<11) | ((dwMiiIndex & 0x1FUL)<<6);
    Lan_SetMacRegDW(dwLanBase, MII_ACC, dwAddr);

    // wait for read to complete w/ timeout
    for(i=0;i<100;i++) {
        // see if MII is finished yet
        if ((Lan_GetMacRegDW(dwLanBase, MII_ACC) & MII_ACC_MII_BUSY_) == 0UL)
        {
            // get the read data from the MAC & return i
            wRet = ((UINT16)Lan_GetMacRegDW(dwLanBase, MII_DATA));
            break;
        }
    }
    if (i == 100) {
        OALMSGS(OAL_ERROR, (L"timeout waiting for MII write to finish\n"));
        wRet = ((UINT16)0xFFFFU);
    }

    return wRet;
}

typedef struct  _SHOW_REG   {
    WCHAR   wszName[20];
    DWORD   dwOffset;
}   SHOW_REG;
static const SHOW_REG   sysCsr[] = {
    { L"ID_REV",        ID_REV      },
    { L"INT_CFG",       INT_CFG     },
    { L"INT_STS",       INT_STS     },
    { L"INT_EN",        INT_EN      },
    { L"DMA_CFG",       DMA_CFG     },
    { L"BYTE_TEST",     BYTE_TEST   },
    { L"FIFO_INT",      FIFO_INT    },
    { L"RX_CFG",        RX_CFG      },
    { L"TX_CFG",        TX_CFG      },
    { L"HW_CFG",        HW_CFG      },
    { L"RX_DP_CTRL",    RX_DP_CTL   },
    { L"RX_FIFO_INF",   RX_FIFO_INF },
    { L"TX_FIFO_INF",   TX_FIFO_INF },
    { L"PMT_CTRL",      PMT_CTRL    },
    { L"GPIO_CFG",      GPIO_CFG    },
    { L"GPT_CFG",       GPT_CFG     },
    { L"GPT_CNT",       GPT_CNT     },
    { L"FPGA_REV",      FPGA_REV    },
    { L"ENDIAN",        ENDIAN      },
    { L"FREE_RUN",      FREE_RUN    },
    { L"RX_DROP",       RX_DROP     },
    { L"MAC_CSR_CMD",   MAC_CSR_CMD },
    { L"MAC_CSR_DATA",  MAC_CSR_DATA},
    { L"AFC_CFG",       AFC_CFG     },
    { L"E2P_CMD",       E2P_CMD     },
    { L"E2P_DATA",      E2P_DATA    },
    { L"TEST_REG_A",    TEST_REG_A  }
};

static const SHOW_REG macCsr[] = {
    { L"MAC_CR",        MAC_CR      },
    { L"MAC_ADDRH",     ADDRH       },
    { L"MAC_ADDRL",     ADDRL       },
    { L"MAC_HASHH",     HASHH       },
    { L"MAC_HASHL",     HASHL       },
    { L"MAC_MII_ACC",   MII_ACC     },
    { L"MAC_MII_DATA",  MII_DATA    },
    { L"MAC_FLOW",      FLOW        },
    { L"MAC_VLAN1",     VLAN1       },
    { L"MAC_VLAN2",     VLAN2       },
    { L"MAC_WUFF",      WUFF        },
    { L"MAC_WUCSR",     WUCSR       }
};

static const SHOW_REG phyCsr[] = {
    { L"PHY_BCR",       PHY_BCR     },
    { L"PHY_BSR",       PHY_BSR     },
    { L"PHY_ID_1",      PHY_ID_1    },
    { L"PHY_ID_2",      PHY_ID_2    },
    { L"PHY_ANEG_ADV",  PHY_ANEG_ADV},
    { L"PHY_ANEG_LPA",  PHY_ANEG_LPA},
    { L"PHY_ANEG_EXP",  6UL         },
    { L"PHY_SI_REV",    16UL        },
    { L"PHY_MODE_CTRL_STS", PHY_MODE_CTRL_STS   },
    { L"PHY_SPECIAL_MODE",  18UL    },
    { L"PHY_TSTCNTL",       20UL    },
    { L"PHY_TSTREAD1",      21UL    },
    { L"PHY_TSTREAD1",      22UL    },
    { L"PHY_TSTWRITE",      23UL    },
    { L"PHY_CONTROL",       27UL    },
    { L"PHY_SITC",          28UL    },
    { L"PHY_INT_SRC",       PHY_INT_SRC         },
    { L"PHY_INT_MASK",      PHY_INT_MASK        },
    { L"PHY_SPECIAL",       PHY_SPECIAL         },
};


static void DumpSIMRegs()
{
    UINT    i;

    OALMSGS(1, (TEXT("Dump 911X Slave Interface Module Registers\r\n")));
    for (i=0U;i<(sizeof(sysCsr)/sizeof(SHOW_REG));i++) {
        OALMSGS(1, (TEXT("%20s = 0x%08x\r\n"), 
                    sysCsr[i].wszName, 
                    GetRegDW(g_pLAN911x, sysCsr[i].dwOffset)));
    }
}

static void DumpMACRegs()
{
    UINT    i;

    OALMSGS(1, (TEXT("Dump 911X MAC Registers\r\n")));
    for (i=0U;i<(sizeof(macCsr)/sizeof(SHOW_REG));i++) {
        OALMSGS(1, (TEXT("%20s = 0x%08x\r\n"), 
                    macCsr[i].wszName, 
                    Lan_GetMacRegDW(g_pLAN911x, macCsr[i].dwOffset)));
    }
}

static void DumpPHYRegs()
{
    UINT    i;

    OALMSGS(1, (TEXT("Dump 911X PHY Registers\r\n")));
    for (i=0U;i<(sizeof(phyCsr)/sizeof(SHOW_REG));i++) {
        OALMSGS(1, (TEXT("%20s = 0x%04x\r\n"), 
                    phyCsr[i].wszName, 
                    Lan_GetPhyRegW(g_pLAN911x, 1, phyCsr[i].dwOffset)));
    }
}

static void DumpAllRegs()
{
    OALMSGS(1, (L"Dump All 911X Registers\r\n"));
    DumpSIMRegs();
    DumpMACRegs();
    DumpPHYRegs();
}

UINT32 Lan_GetLinkMode(const UINT32 dwLanBase)
{
    DWORD result = LINK_NO_LINK;
    const WORD wRegBSR = Lan_GetPhyRegW(dwLanBase, dwPhyAddr, PHY_BSR);

    if(wRegBSR & PHY_BSR_LINK_STATUS_) 
    {
        WORD wTemp;
        const WORD wRegADV = Lan_GetPhyRegW(dwLanBase, dwPhyAddr, PHY_ANEG_ADV);
        const WORD wRegLPA = Lan_GetPhyRegW(dwLanBase, dwPhyAddr, PHY_ANEG_LPA);
        wTemp = (WORD)(wRegLPA & wRegADV);

        if(wTemp & PHY_ANEG_LPA_100FDX_) 
        {
            result = LINK_100MPS_FULL;
        } 
        else if(wTemp & PHY_ANEG_LPA_100HDX_) 
        {
            result = LINK_100MPS_HALF;
        } 
        else if(wTemp & PHY_ANEG_LPA_10FDX_) 
        {
            result = LINK_10MPS_FULL;
        } 
        else if(wTemp & PHY_ANEG_LPA_10HDX_) 
        {
            result = LINK_10MPS_HALF;
        }
    }

    return result;
}

//------------------------------------------------------------------------------
//  *NOTE*
//      All H/W settings like timing, bus width, Chip Select and etc
//      should be set before calling this routine.
//------------------------------------------------------------------------------
BOOL LAN911xInit(UINT8 *pAddress, UINT32 offset, UINT16 mac[3])
{
    BOOL rc = FALSE;
    UINT32  dwRegVal, dwTemp, dwTimeOut;
    UINT16  wTemp;
    UINT32  dwADDR;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(offset);

    // Save address
    g_pLAN911x = (UINT32)pAddress;

    // Chip settle time.
    OALStall(750);

    // Reset the Chip
    SetRegDW(g_pLAN911x, HW_CFG, HW_CFG_SRST_);
    dwTimeOut = 100000UL;
    do {
        OALStall(10);
        dwTemp = GetRegDW(g_pLAN911x, HW_CFG);
        dwTimeOut--;
    } while((dwTimeOut-- > 0UL) && (dwTemp & HW_CFG_SRST_));

    if (dwTemp & HW_CFG_SRST_) {
        OALMSGS(OAL_ERROR, (L"ERROR: Failed to Initialize LAN911x\r\n"));
        goto cleanUp;
    }

    // read the chip ID and revision.
    g_chipRevision = GetRegDW(g_pLAN911x, ID_REV);
    if (((g_chipRevision & 0x0FF00000) == 0x01100000) ||
        ((g_chipRevision & 0xFF000000) == 0x11000000))
    {
        OALMSGS(TRUE, (L"LAN9118: Chip Id %x Revision %d\r\n", 
            (g_chipRevision >> 16) & 0xFFFF, (g_chipRevision & 0xFFFF)));
    }
    else
    {
        OALMSGS(TRUE, (L"Error! Not a LAN9118 Family. Init Failed.\r\n"));
        goto cleanUp;
    }

       // enable LED indicator
       dwTemp = GetRegDW(g_pLAN911x, GPIO_CFG);
       dwTemp |= GPIO_CFG_LED3_EN_ |GPIO_CFG_LED2_EN_ | GPIO_CFG_LED1_EN_;
    SetRegDW(g_pLAN911x, GPIO_CFG, dwTemp);

    // NOTE
    // Internal PHY only
    // If want to support External Phy, 
    //  assign External Phy address to dwPhyAddr
    dwPhyAddr = 1UL;

    // reset PHY
    Lan_SetPhyRegW(g_pLAN911x, dwPhyAddr, PHY_BCR, PHY_BCR_RESET_);
    dwTimeOut = 1000UL;
    do {
        OALStall(10000);            // 10mSec
        dwTemp = (UINT32)Lan_GetPhyRegW(g_pLAN911x, dwPhyAddr, PHY_BCR);
        dwTimeOut--;
    } while ((dwTimeOut>0UL) && (dwTemp & (UINT32)PHY_BCR_RESET_));

    if (dwTemp & (UINT32)PHY_BCR_RESET_) {
        OALMSGS(OAL_ERROR, (L"PHY reset failed to complete.\r\n"));
        goto cleanUp;
    }


    // Auto Neg
    Lan_SetPhyRegW(g_pLAN911x, dwPhyAddr, PHY_BCR, PHY_BCR_AUTO_NEG_ENABLE_);
    wTemp = Lan_GetPhyRegW(g_pLAN911x, dwPhyAddr, PHY_ANEG_ADV);
    wTemp &= ~(PHY_ANEG_ADV_PAUSE_OP_ | PHY_ANEG_ADV_SPEED_);

       // 10/100 Mhz
    wTemp |= PHY_ANEG_ADV_10H_| PHY_ANEG_ADV_10F_| PHY_ANEG_ADV_100H_| PHY_ANEG_ADV_100F_;

       // 10Mhz only
    //wTemp |= PHY_ANEG_ADV_10H_| PHY_ANEG_ADV_10F_;

    Lan_SetPhyRegW(g_pLAN911x, dwPhyAddr, PHY_ANEG_ADV, wTemp);

    Lan_SetPhyRegW(g_pLAN911x, dwPhyAddr, PHY_BCR, PHY_BCR_AUTO_NEG_ENABLE_ | PHY_BCR_RESTART_AUTO_NEG_);

    dwTimeOut = 10000;
    do {
        OALStall(1000);         // 1mSec
        wTemp = Lan_GetPhyRegW(g_pLAN911x, dwPhyAddr, PHY_BSR);
    } while ((dwTimeOut-- > 0UL) && 
            !((wTemp & (WORD)(PHY_BSR_REMOTE_FAULT_|PHY_BSR_AUTO_NEG_COMP_))));

    if (wTemp & (WORD)PHY_BSR_AUTO_NEG_COMP_) 
    {
        OALMSGS(OAL_ERROR, (L"Auto Negotiation done\r\n"));
        // We are done
    }
    else
    {
        OALMSGS(OAL_ERROR, (L"Auto Negotiation Failed\r\n"));
        goto cleanUp;
    }

    dwTemp = Lan_GetLinkMode(g_pLAN911x);

    dwRegVal = Lan_GetMacRegDW(g_pLAN911x, MAC_CR);
    dwRegVal &= ~(MAC_CR_FDPX_|MAC_CR_RCVOWN_);

    switch(dwTemp)
    {
        case LINK_NO_LINK:
                     OALMSGS(OAL_ETHER&&OAL_FUNC, (L"Auto Negotiation done LINK_NO_LINK\r\n"));
            break;
        case LINK_10MPS_HALF:
            dwRegVal|=MAC_CR_RCVOWN_;
                     OALMSGS(OAL_ETHER&&OAL_FUNC, (L"Auto Negotiation done LINK_10MPS_HALF\r\n"));
            break;
        case LINK_10MPS_FULL:
            dwRegVal|=MAC_CR_FDPX_;
                     OALMSGS(OAL_ETHER&&OAL_FUNC, (L"Auto Negotiation done LINK_10MPS_FULL\r\n"));
            break;
        case LINK_100MPS_HALF:
            dwRegVal|=MAC_CR_RCVOWN_;
                     OALMSGS(OAL_ETHER&&OAL_FUNC, (L"Auto Negotiation done LINK_100MPS_HALF\r\n"));
            break;
        case LINK_100MPS_FULL:
            dwRegVal|=MAC_CR_FDPX_;
                     OALMSGS(OAL_ETHER&&OAL_FUNC, (L"Auto Negotiation done LINK_100MPS_FULL\r\n"));
            break;
    }
    Lan_SetMacRegDW(g_pLAN911x, MAC_CR, dwRegVal);


    // initialize TX
    dwTemp = Lan_GetMacRegDW(g_pLAN911x, MAC_CR);
    dwTemp |= (MAC_CR_TXEN_ | MAC_CR_HBDIS_);
    Lan_SetMacRegDW(g_pLAN911x, MAC_CR, dwTemp);

    //setup TLI store-and-forward, and preserve TxFifo size
    dwTemp = GetRegDW(g_pLAN911x, HW_CFG);
    dwTemp &= (HW_CFG_TX_FIF_SZ_ | 0xFFFUL);
    dwTemp |= HW_CFG_SF_;
    SetRegDW(g_pLAN911x, HW_CFG, dwTemp);
    dwTemp = GetRegDW(g_pLAN911x, HW_CFG);

    SetRegDW(g_pLAN911x, TX_CFG, TX_CFG_TX_ON_);

    // initialize RX
    SetRegDW(g_pLAN911x, RX_CFG, 0x00000000UL);
    dwTemp = Lan_GetMacRegDW(g_pLAN911x, MAC_CR);
    dwTemp |= MAC_CR_RXEN_;
    Lan_SetMacRegDW(g_pLAN911x, MAC_CR, dwTemp);
          

    // If caller wants MAC to be read from EEPROM
    if ((mac[0] == 0xFFFF) && (mac[1] == 0xFFFF) && (mac[2] == 0xFFFF))
    {
        mac[0] = 0;
        mac[1] = 0;
        mac[2] = 0;
        
        // Try to get MAC from  EEPROM
        if (GetRegDW(g_pLAN911x, E2P_CMD) & E2P_CMD_E2P_PROG_DONE_)
        {
            dwADDR = Lan_GetMacRegDW(g_pLAN911x, ADDRL);
            mac[0] = (UINT16)(dwADDR & 0xFFFF);
            mac[1] = (UINT16)((dwADDR >> 16)& 0xFFFF);
            dwADDR = Lan_GetMacRegDW(g_pLAN911x, ADDRH);
            mac[2] = (UINT16)(dwADDR & 0xFFFF);
        }

        // Check if EEPROM MAC was valid
        if ((mac[0] == 0) && (mac[1] == 0) && (mac[2] == 0))
        {
            OALMSGS(OAL_ERROR, (L"MAC could not be loaded from EEPROM\r\n"));
            goto cleanUp;
        }
        else
        {
            OALMSGS(OAL_ERROR, (L"MAC loaded from EEPROM\r\n"));
        }


    }

    // Use MAC provided by caller
    else
    {
        // Set the MAC
        dwADDR = (mac[1] << 16) | mac[0] ;
        Lan_SetMacRegDW(g_pLAN911x, ADDRL, dwADDR);
        dwADDR = mac[2];
        Lan_SetMacRegDW(g_pLAN911x, ADDRH, dwADDR);

        // Get the MAC
        dwADDR = Lan_GetMacRegDW(g_pLAN911x, ADDRL);
        mac[0] = (UINT16)(dwADDR & 0xFFFF);
        mac[1] = (UINT16)((dwADDR >> 16)& 0xFFFF);
        dwADDR = Lan_GetMacRegDW(g_pLAN911x, ADDRH);
        mac[2] = (UINT16)(dwADDR & 0xFFFF);

        OALMSGS(OAL_ETHER&&OAL_FUNC, (L"MAC provided by caller.\r\n"));
    }

    rc = TRUE;

cleanUp:
    OALMSGS(OAL_ETHER&&OAL_FUNC, (
        L"-LAN91CInit(mac = %02x:%02x:%02x:%02x:%02x:%02x, rc = %d)\r\n",
        mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8, mac[2]&0xFF, mac[2]>>8,
        rc
    ));

    return rc;
}

//------------------------------------------------------------------------------

UINT16 LAN911xSendFrame(UINT8 *pBuffer, UINT32 length)
{
    UINT16 rc = 0;
    UINT32 dwCounter;
    UINT32 dwTemp;
    UINT32 dwTxCmdA, dwTxCmdB;
    UINT32 dwSpcNeed, dwTxSpc;
    static UINT32   dwSeqNum;

    // wait until space available
    dwCounter = 0;
    dwSpcNeed = (length+3)&(~0x03UL);
    for(;;)
    {
        dwTxSpc = GetRegDW(g_pLAN911x, TX_FIFO_INF) & TX_FIFO_INF_TDFREE_;
        if (dwSpcNeed <= dwTxSpc)
        {
            break;
        }
        OALStall(10);
        if (dwCounter++ > 100)
        {
            OALMSGS(OAL_ERROR, (L"ERROR: Tx Timeout Error. (no FIFO available)\r\n"));
            OALMSGS(TRUE, (L"ERROR: Tx Timeout Error. (no FIFO available)\r\n"));
            return 1;
        }
    }

    dwTxCmdA = (((UINT32)pBuffer & 0x03) << 16) | length;
    dwTxCmdA |= (TX_CMD_A_INT_FIRST_SEG_ | TX_CMD_A_INT_LAST_SEG_);
    dwTxCmdB = length;
    dwTxCmdB |= (dwSeqNum << 16);
    dwSeqNum ++;
    SetRegDW(g_pLAN911x, TX_DATA_FIFO_PORT, dwTxCmdA);
    SetRegDW(g_pLAN911x, TX_DATA_FIFO_PORT, dwTxCmdB);

    WriteFifo(g_pLAN911x, TX_DATA_FIFO_PORT, (UINT32 *)((UINT32)pBuffer & ~0x03UL), (length+3+(((UINT32)pBuffer)&0x03UL))>>2);

    dwTemp = GetRegDW(g_pLAN911x, TX_FIFO_INF) & TX_FIFO_INF_TSUSED_;
    if ((dwTemp >> 16) & 0xFFFF)
    {
        dwTemp = GetRegDW(g_pLAN911x, TX_STATUS_FIFO_PORT);

        if (dwTemp & TX_STS_ES)
        {
            OALMSGS(OAL_ERROR, (L"ERROR: Tx Error (TxStatus = 0x%x)\r\n", dwTemp));
            // Failure.
            rc = 1;
        }
    }
    return rc;
}

//------------------------------------------------------------------------------

UINT16 LAN911xGetFrame(UINT8 *pBuffer, UINT16 *pLength)
{
    UINT32  dwTemp, RxStatus, pkt_len = 0;

    *pLength = 0;

    if (GetRegDW(g_pLAN911x, INT_STS) & INT_STS_RSFL_)
    {
        dwTemp = GetRegDW(g_pLAN911x, RX_FIFO_INF);
        if ((dwTemp & 0x00FF0000) >> 16)
        {
            RxStatus = GetRegDW(g_pLAN911x, RX_STATUS_FIFO_PORT);
            if (!(RxStatus & RX_STS_ES))
            {
                pkt_len = (RxStatus & 0x3FFF0000) >>16;
                ReadFifo(g_pLAN911x, RX_DATA_FIFO_PORT, (UINT32 *)(((UINT32)pBuffer)&~0x03), (((pkt_len+3)>>2)));

                *pLength = (UINT16)pkt_len;
            }
            else
            {
                OALMSGS(TRUE, ((L"Rx Error! RxStatus = 0x%08lx\r\n"), RxStatus));
            }
            // clear pending Intr
            SetRegDW(g_pLAN911x, INT_STS, INT_STS_RSFL_);
        }
    }

    if (GetRegDW(g_pLAN911x, INT_STS) & INT_STS_RXE_)
    {
        OALMSGS(TRUE, ((L"RX Error! States = %X\r\n"), GetRegDW(g_pLAN911x, INT_STS)));
        // clear RXE Int
        SetRegDW(g_pLAN911x, INT_STS, INT_STS_RXE_);
        *pLength = 0;
    }

    return (*pLength);
}


//------------------------------------------------------------------------------

VOID LAN911xEnableInts()
{
    UINT32  dwTemp;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+LAN911xEnableInts\r\n"));

    // enable master IRQ_EN
    dwTemp = GetRegDW(g_pLAN911x, INT_CFG);
    dwTemp |= INT_CFG_IRQ_EN_;
    SetRegDW(g_pLAN911x, INT_CFG, dwTemp);

    // clear pending one before enable
    SetRegDW(g_pLAN911x, INT_STS, INT_STS_RSFL_);

    // Only enable receive interrupts (we poll for Tx completion)
    dwTemp = GetRegDW(g_pLAN911x, INT_EN);
    dwTemp |= INT_EN_RSFL_EN_;
    SetRegDW(g_pLAN911x, INT_EN, dwTemp);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-LAN911xEnableInts\r\n"));
}

//------------------------------------------------------------------------------

VOID LAN911xDisableInts()
{
    UINT32  dwTemp;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+LAN911xDisableInts\r\n"));

    // Disable all interrupts
    dwTemp = GetRegDW(g_pLAN911x, INT_EN);
    dwTemp &= ~INT_EN_RSFL_EN_;
    SetRegDW(g_pLAN911x, INT_EN, dwTemp);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-LAN911xDisableInts\r\n"));
}

//------------------------------------------------------------------------------
static UINT32 ComputeCrc(IN const UINT8 *pBuffer, IN const UINT32 uiLength)
{
    UINT i;
    UINT32 crc = 0xFFFFFFFFUL;
    UINT32 result = 0UL;
    const UINT32 poly = 0xEDB88320UL;

    for(i=0U; i<uiLength; i++) 
    {
        int bit;
        UINT32 data=((UINT32)pBuffer[i]);
        for(bit=0; bit<8; bit++) 
        {
            const UINT32 p = (crc^((UINT32)data))&1UL;
            crc >>= 1;
            if(p != 0UL) {
                crc ^= poly;
            }
            data >>=1;
        }
    }
    result=((crc&0x01UL)<<5)|
           ((crc&0x02UL)<<3)|
           ((crc&0x04UL)<<1)|
           ((crc&0x08UL)>>1)|
           ((crc&0x10UL)>>3)|
           ((crc&0x20UL)>>5);

    return (result);
}

//------------------------------------------------------------------------------
VOID LAN911xCurrentPacketFilter(UINT32 filter)
{
    UINT32  dwTemp;

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+LAN911xCurrentPacketFilter(0x%08x)\r\n", filter));

    dwTemp = Lan_GetMacRegDW(g_pLAN911x, MAC_CR);
    dwTemp &= ~MAC_CR_RXEN_;
    Lan_SetMacRegDW(g_pLAN911x, MAC_CR, dwTemp);

    dwTemp = Lan_GetMacRegDW(g_pLAN911x, MAC_CR);
    if ((filter & PACKET_TYPE_ALL_MULTICAST) != 0) 
        dwTemp |= MAC_CR_MCPAS_;
    if ((filter & PACKET_TYPE_PROMISCUOUS) != 0) 
        dwTemp |= MAC_CR_PRMS_;

    dwTemp |= MAC_CR_RXEN_;
    Lan_SetMacRegDW(g_pLAN911x, MAC_CR, dwTemp);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-LAN911xCurrentPacketFilter\r\n"));
}

//------------------------------------------------------------------------------
static void GetMulticastBit(const UINT8 * const ucAddress, UINT8 * const pTable, UINT32 * const pValue)
{
    UINT32 uiBitNumber;

    uiBitNumber = ComputeCrc(ucAddress, 6);
    *pTable = (UCHAR)(((uiBitNumber & 0x20UL) >> 5) & 1UL);
    *pValue = (UINT32)(1UL << (uiBitNumber & 0x1FUL));
}

//------------------------------------------------------------------------------
BOOL LAN911xMulticastList(UINT8 *pAddresses, UINT32 count)
{
    UINT32 i, MCReg[2];
    UINT8  Table;
    UINT32 dwBit;   

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+LAN911xMulticastList(0x%08x, %d)\r\n", pAddresses, count));

    // Calculate hash bits       
    MCReg[0] = MCReg[1] = 0;
    for (i = 0; i < count; i++) {
        GetMulticastBit(pAddresses,  &Table, &dwBit);
        MCReg[Table] |= dwBit;
        pAddresses += 6;
    }

    // Write it to hardware
    Lan_SetMacRegDW(g_pLAN911x, HASHL, MCReg[0]);
    Lan_SetMacRegDW(g_pLAN911x, HASHH, MCReg[1]);

    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-LAN911xMulticastList(rc = 1)\r\n"));
    return TRUE;
}

