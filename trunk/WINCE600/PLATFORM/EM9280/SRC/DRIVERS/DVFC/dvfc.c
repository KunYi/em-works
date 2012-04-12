//-----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  dvfc.c
//
//  Provides BSP-specific configuration routines for the DVFS driver.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include "bsp.h"
#include "dvfs.h"
#include "pmu.h"
#include "common_regsdcp.h"
#include "regsbch.h"
// Need to disable function pointer cast warnings.  We use memcpy to move 
// the bus scaling calibration routine into IRAM and then cast this address 
// as a function pointer.
#pragma warning(disable: 4054 4055)

//-----------------------------------------------------------------------------
// External Functions
extern BOOL DvfcUpdateSupplyVoltage(UINT32 mV, UINT32 mV_BO, DDK_DVFC_DOMAIN domain);
extern PDDK_CLK_CONFIG DDKClockGetSharedConfig(VOID);
extern VOID DDKClockLock(VOID);
extern VOID DDKClockUnlock(VOID);
extern void EnableIrqInterrupt(VOID);
extern void DisableIrqInterrupt(VOID);


//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define DVFC_VERBOSE            FALSE
#define DVFC_IST_PRIORITY       250

#define CSPDDK_LOCK()           DDKClockLock()
#define CSPDDK_UNLOCK()         DDKClockUnlock()

#define DRAMCTRLREGNUM          190

//-----------------------------------------------------------------------------
// Types
typedef enum
{
    // CPU setpoint parameters
    DVFC_PARM_ARM_FRAC_CPUFRAC  = 0, 
    DVFC_PARM_ARM_CLK_DIV       = 1,
    DVFC_PARM_AHB_CLK_DIV       = 2,
    DVFC_PARM_EMI_FRAC_EMIFRAC  = 3, 
    DVFC_PARM_EMI_CLK_DIV       = 4,    
    DVFC_PARM_ENUM_END          = 5
} DVFC_SETPOINT_PARM;

typedef struct
{
    UINT32 parm[DVFC_PARM_ENUM_END];
    DDK_DVFC_SETPOINT_INFO setPointInfo;    
} DVFC_SETPOINT_CONFIG, *PDVFC_SETPOINT_CONFIG;   

typedef enum
{
    EMI_DEV_MOBILE_DDR = 0,
    EMI_DEV_DDR1       = 1
}EMI_MEM_TYPE;


typedef struct {
    PVOID   pv_HwRegDRAM;
    DWORD   DDRTimingoptimized_Low[DRAMCTRLREGNUM];    
    DWORD   DDRTimingoptimized_High[DRAMCTRLREGNUM];    
} DVFC_TIMING_OPTIMIZED, *PDVFC_TIMING_OPTIMIZED;

typedef void (*FUNC_EMI_CONTROL) (UINT32 NewClock, UINT32 EMIFracDiv, UINT32 EMIDiv);
//typedef void (*FUNC_DEBUG_CONTROL) (UINT8 ch);

//-----------------------------------------------------------------------------
// Global Variables
DWORD g_DvfcIrq = IRQ_DVFC;

//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWregCLKCTRL = NULL;
PVOID pv_HWregPOWER = NULL;
PVOID pv_HWregDIGCTL = NULL;
PVOID pv_HWregDRAM = NULL;
PVOID pv_HWregUARTDbg = NULL;
PVOID pv_HWregAPBH = NULL;
PVOID pv_HWregAPBX = NULL;
PVOID pv_HWregDCP = NULL;
PVOID pv_HWregBCH = NULL;
PVOID pv_HWregLCDIF = NULL;

FUNC_EMI_CONTROL pEMIClockChangeFunc = NULL;
//FUNC_DEBUG_CONTROL pDEBUGChange = NULL;
PDVFC_TIMING_OPTIMIZED pDDRTiming;

//static BOOL lowFreq = FALSE;
static UINT32 DRAM_REG[DRAMCTRLREGNUM];
static HANDLE g_hSetPointEvent;
static int g_IstPriority;
static HANDLE g_hDvfcWorkerThread;
static HANDLE g_hDvfcWorkerEvent;
//static UINT32 DDR2ChangeCount = 0;
static CEDEVICE_POWER_STATE g_dxCurrent[DDK_DVFC_DOMAIN_ENUM_END];
static DDK_DVFC_SETPOINT g_SetpointLoad[DDK_DVFC_DOMAIN_ENUM_END];

static DVFC_SETPOINT_CONFIG g_SetPointConfig[DDK_DVFC_DOMAIN_ENUM_END][DDK_DVFC_SETPOINT_ENUM_END] = 
{
    // CPU SETPOINTS
    {
        // HIGH SETPOINT
        {
            {
                BSP_DVFS_CPU_HIGH_FRAC_CPUFRAC,                 
                BSP_DVFS_CPU_HIGH_ARM_CLK_DIV,
                BSP_DVFS_CPU_HIGH_AHB_CLK_DIV,
                BSP_DVFS_CPU_HIGH_FRAC_EMIFRAC,
                BSP_DVFS_CPU_HIGH_EMI_CLK_DIV
            },
            {
                BSP_DVFS_CPU_HIGH_mV, 
                BSP_DVFS_CPU_HIGH_BO_mV,    
                {
                    BSP_DVFS_CPU_HIGH_ARM_FREQ,
                    BSP_DVFS_CPU_HIGH_AHB_FREQ,
                    BSP_DVFS_CPU_HIGH_EMI_FREQ
                }
            }
        },

        // MEDIUM SETPOINT
        {
            {
                BSP_DVFS_CPU_MED_FRAC_CPUFRAC,                 
                BSP_DVFS_CPU_MED_ARM_CLK_DIV,
                BSP_DVFS_CPU_MED_AHB_CLK_DIV,
                BSP_DVFS_CPU_MED_FRAC_EMIFRAC,
                BSP_DVFS_CPU_MED_EMI_CLK_DIV
            },
            {
                BSP_DVFS_CPU_MED_mV, 
                BSP_DVFS_CPU_MED_BO_mV,    
                {
                    BSP_DVFS_CPU_MED_ARM_FREQ,
                    BSP_DVFS_CPU_MED_AHB_FREQ,
                    BSP_DVFS_CPU_MED_EMI_FREQ
                }
            }
        },

        // LOW SETPOINT
        {
            {
                BSP_DVFS_CPU_LOW_FRAC_CPUFRAC,    
                BSP_DVFS_CPU_LOW_ARM_CLK_DIV,
                BSP_DVFS_CPU_LOW_AHB_CLK_DIV,
                BSP_DVFS_CPU_LOW_FRAC_EMIFRAC,
                BSP_DVFS_CPU_LOW_EMI_CLK_DIV
            },
            {
                BSP_DVFS_CPU_LOW_mV,  
                BSP_DVFS_CPU_LOW_BO_mV,    
                {
                    BSP_DVFS_CPU_LOW_ARM_FREQ,
                    BSP_DVFS_CPU_LOW_AHB_FREQ,
                    BSP_DVFS_CPU_LOW_EMI_FREQ
                }
            }
        }
        
    }
};


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcGetThreadPriority
//
//  This function provides the thread priority for the DVFC IST.
//
//  Parameters:
//      None.
//
//  Returns:
//      DVFC thread priority.
//
//-----------------------------------------------------------------------------
int BSPDvfcGetThreadPriority(void)
{
    return g_IstPriority;
}

//-----------------------------------------------------------------------------
//
//  Function:  DvfcMapDevPwrStateToSetpoint
//
//  This function maps the specified device power state into a DVFC setpoint.
//
//  Parameters:
//      dx
//          [in] Device power state to be mapped.
//
//      domain
//          [in] Specifies DVFC domain.
//
//  Returns:
//      DVFC setpoint associated with the device power state.
//
//-----------------------------------------------------------------------------
DDK_DVFC_SETPOINT DvfcMapDevPwrStateToSetpoint(CEDEVICE_POWER_STATE dx,
                                               DDK_DVFC_DOMAIN domain)
{
    DDK_DVFC_SETPOINT setpoint;
    
    switch(domain)
    {

    default:
        switch(dx)
        {
        case D0:
            setpoint = DDK_DVFC_SETPOINT_HIGH;
            break;

        case D1:
            setpoint = DDK_DVFC_SETPOINT_MEDIUM;
            break;

        case D2:
            setpoint = DDK_DVFC_SETPOINT_LOW;
            break;

        case D4:
            // Force HIGH setpoint for suspend/resume
            setpoint = DDK_DVFC_SETPOINT_HIGH;
            break;

        default:
            setpoint = DDK_DVFC_SETPOINT_LOW;
            break;
        }
        break;
    }

    return setpoint;
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcGetSupportedDx
//
//  This function returns the supported device states for the DVFC driver.
//
//  Parameters:
//      None.
//
//  Returns:
//      Bitmask indicating supported device power states.
//
//-----------------------------------------------------------------------------
UCHAR BSPDvfcGetSupportedDx(void)
{
    return (DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D2) | DX_MASK(D4));
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcPowerGet
//
//  This function responds to a IOCTL_POWER_GET request from the WinCE 
//  Power Manager.
//
//  Parameters:
//      domainIndex
//          [in] DVFC domain index
//
//  Returns:
//      Device power state for the specified domain.
//
//-----------------------------------------------------------------------------
CEDEVICE_POWER_STATE BSPDvfcPowerGet(UINT32 domainIndex)
{
    DDK_DVFC_DOMAIN domain;
    
    if ((domainIndex > 0) && (domainIndex <= DDK_DVFC_DOMAIN_ENUM_END))
    {
        // Convert domain index into DDK_DVFC_DOMAIN enum
        domain = domainIndex - 1;
        return g_dxCurrent[domain];
    }
    else
    {
        return PwrDeviceUnspecified;
    }   
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcPowerSet
//
//  This function responds to a IOCTL_POWER_SET request from the WinCE 
//  Power Manager.
//
//  Parameters:
//      domainIndex
//          [in] DVFC domain index
//
//      dx
//          [in] Requested device state.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPDvfcPowerSet(UINT32 domainIndex, CEDEVICE_POWER_STATE dx)
{
    DDK_DVFC_SETPOINT setpoint;
    DDK_DVFC_DOMAIN domain;

    if ((domainIndex == 0) || (domainIndex > DDK_DVFC_DOMAIN_ENUM_END))
    {
        return FALSE;
    }

    // Convert domain index into DDK_DVFC_DOMAIN enum
    domain = domainIndex -1;
    
    //RETAILMSG(DVFC_VERBOSE, (_T("DOM%d = D%d\r\n"), domain, dx));    

    if (g_dxCurrent[domain] != PwrDeviceUnspecified)
    {
        setpoint = DvfcMapDevPwrStateToSetpoint(g_dxCurrent[domain], domain);
        DDKClockSetpointRelease(setpoint);
    }
           
    if (dx != PwrDeviceUnspecified)
    {
        setpoint = DvfcMapDevPwrStateToSetpoint(dx, domain);
        // If we are suspending the system, block on the setpoint
        // change to ensure we will resume with the required
        // setpoint configuration.
        if (dx == D4)
        {
            DDKClockSetpointRequest(setpoint, TRUE);
        }
        else
        {
            DDKClockSetpointRequest(setpoint, FALSE);
        }
    }

    g_dxCurrent[domain] = dx;
  
    return TRUE;
}


#if (DVFC_VERBOSE == TRUE)
//-----------------------------------------------------------------------------
//
//  Function:  DumpSetpoint
//
//  Dumps setpoint information.
//
//  Parameters:
//      domain
//          [in] Specifies DVFC domain.
//
//      setpoint
//          [in] Specifies current setpoint.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DumpSetpoint(PDVFC_SETPOINT_CONFIG pNewCfg)
{
    switch (pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_CPU])
    {
    case BSP_DVFS_CPU_HIGH_ARM_FREQ:
        RETAILMSG(TRUE, (_T("Setpoint HIGH\r\n")));
        break;

    case BSP_DVFS_CPU_MED_ARM_FREQ:
        RETAILMSG(TRUE, (_T("Setpoint MED\r\n")));
        break;

    case BSP_DVFS_CPU_LOW_ARM_FREQ:
        RETAILMSG(TRUE, (_T("Setpoint LOW\r\n")));
        break;
    }
}
#endif


//-----------------------------------------------------------------------------
//
//  Function:  DDR2EmiController_EDE1116_133MHz
//
//  This function is to initial EMI controlller for DDR2 133MHz.
//
//  Parameters:
//          None.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void DDR2EmiController_EDE1116_133MHz(void)
{
    DRAM_REG[0] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_0(RW) 
    DRAM_REG[1] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_1(RW) 
    DRAM_REG[2] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_2(RW) 
    DRAM_REG[3] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_3(RW) 
    DRAM_REG[4] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_4(RW) 
    DRAM_REG[5] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_5(RW) 
    DRAM_REG[6] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_6(RW) 
    DRAM_REG[7] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_7(RW) 
    DRAM_REG[8] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_0(RD) 
    DRAM_REG[9] =   0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_1(RD) 
    DRAM_REG[10] =  0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_2(RD) 
    DRAM_REG[11] =  0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_3(RD) 
    DRAM_REG[12] =  0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_4(RD) 
    DRAM_REG[13] =  0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_5(RD) 
    DRAM_REG[14] =  0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_6(RD) 
    DRAM_REG[15] =  0x00000000  ;  //00000000000000000000000000000000 user_def_reg_ro_7(RD) 
    DRAM_REG[16] =  0x00000000  ;  //0000000_0 write_modereg(WR) 0000000_0 power_down(RW) 000000000000000_0 start(RW) 
    DRAM_REG[17] =  0x00000100  ;  //0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_1 enable_quick_srefresh(RW) 0000000_0 srefresh(RW+) 
    DRAM_REG[18] =  0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[19] =  0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[20] =  0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[21] =  0x00000000  ;  //00000_000 cke_delay(RW) 00000000 dll_lock(RD) 0000000_0 dlllockreg(RD) 0000000_0 dll_bypass_mode(RW) 
    DRAM_REG[22] =  0x00000000  ;  //000000000000_0000 lowpower_refresh_enable(RW) 000_00000 lowpower_control(RW) 000_01000 lowpower_auto_enable(RW) 
    DRAM_REG[23] =  0x00000000  ;  //0000000000000000 lowpower_internal_cnt(RW) 0000000000000000 lowpower_external_cnt(RW) 
    DRAM_REG[24] =  0x00000000  ;  //0000000000000000 lowpower_self_refresh_cnt(RW) 0000000000000000 lowpower_refresh_hold(RW) 
    DRAM_REG[25] =  0x00000000  ;  //00000000000000000000000000100000 lowpower_power_down_cnt(RW) 
    DRAM_REG[26] =  0x00010101  ;  //000000000000000_1 priority_en(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 placement_en(RW) 
    DRAM_REG[27] =  0x01010101  ;  //0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_1 bank_split_en(RW) 0000000_1 rw_same_en(RW) 
    DRAM_REG[28] =  0x000f0f01  ;  //00000_000 q_fullness(RW) 0000_1111 age_count(RW) 0000_1111 command_age_count(RW) 0000000_1 active_aging(RW) 
    DRAM_REG[29] =  0x0f02020a  ;  //0000_1111 cs_map(RW) 00000_010 column_size(RW) 00000_010 addr_pins(RW) 0000_1010 aprebit(RW) 
    DRAM_REG[30] =  0x00000000  ;  //0000000000000_000 max_cs_reg(RD) 0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 
    DRAM_REG[31] =  0x00010101  ;  //000000000000000_1 eight_bank_mode(RW) 0000000_1 drive_dq_dqs(RW) 0000000_1 dqs_n_en(RW) 
    DRAM_REG[32] =  0x00000100  ;  //00000000000000000000000_1 reduc(RW) 0000000_0 reg_dimm_enable(RW) 
    DRAM_REG[33] =  0x00000100  ;  //00000000000000000000000_1 concurrentap(RW) 0000000_0 ap(RW) 
    DRAM_REG[34] =  0x00000000  ;  //0000000_0 writeinterp(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
    DRAM_REG[35] =  0x00000002  ;  //000000000000000_0 pwrup_srefresh_exit(RW) 0000000_0 no_cmd_init(RW) 0000_0010 initaref(RW) 
    DRAM_REG[36] =  0x01010000  ;  //0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 000000000000000_0 fast_write(RW) 
    DRAM_REG[37] =  0x07080403  ;  //0000_0111 caslat_lin_gate(RW) 0000_1000 caslat_lin(RW) 00000_100 caslat(RW) 0000_0011 wrlat(RW) 
    DRAM_REG[38] =  0x04003603  ;  //000_00100 tdal(RW) 0000000000110110 tcpd(RW) 00000_011 tcke(RW) 
    DRAM_REG[39] =  0x070000c8  ;  //00_000111 tfaw(RW) 000000000000000011001000 tdll(RW) 
    DRAM_REG[40] =  0x0200682b  ;  //000_00010 tmrd(RW) 000000000110100000101011 tinit(RW) 
    DRAM_REG[41] =  0x00020208  ;  //0000000000000010 tpdex(RW) 00000010 trcd_int(RW) 00_001000 trc(RW) 
    DRAM_REG[42] =  0x00246c06  ;  //000000000010010001101100 tras_max(RW) 00000110 tras_min(RW) 
    DRAM_REG[43] =  0x02110409  ;  //0000_0010 trp(RW) 00010001 trfc(RW) 00_00010000001001 tref(RW) 
    DRAM_REG[44] =  0x01020202  ;  //0000_0001 twtr(RW) 000_00010 twr_int(RW) 00000_010 trtp(RW) 00000_010 trrd(RW) 
    DRAM_REG[45] =  0x00c80013  ;  //0000000011001000 txsr(RW) 0000000000010011 txsnr(RW) 
    DRAM_REG[46] =  0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[47] =  0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[48] =  0x00012100  ;  //0_0000000 axi0_current_bdw(RD) 0000000_1 axi0_bdw_ovflow(RW) 0_0100001 axi0_bdw(RW) 000000_00 axi0_fifo_type_reg(RW) 
    DRAM_REG[49] =  0xffff0303  ;  //0101010101010101 axi0_en_size_lt_width_instr(RW) 00000_011 axi0_w_priority(RW) 00000_011 axi0_r_priority(RW) 
    DRAM_REG[50] =  0x00012100  ;  //0_0000000 axi1_current_bdw(RD) 0000000_1 axi1_bdw_ovflow(RW) 0_0100001 axi1_bdw(RW) 000000_00 axi1_fifo_type_reg(RW) 
    DRAM_REG[51] =  0xffff0303  ;  //1111111100000000 axi1_en_size_lt_width_instr(RW) 00000_011 axi1_w_priority(RW) 00000_011 axi1_r_priority(RW) 
    DRAM_REG[52] =  0x00012100  ;  //0_0000000 axi2_current_bdw(RD) 0000000_1 axi2_bdw_ovflow(RW) 0_0100001 axi2_bdw(RW) 000000_00 axi2_fifo_type_reg(RW) 
    DRAM_REG[53] =  0xffff0303  ;  //0000000000000001 axi2_en_size_lt_width_instr(RW) 00000_011 axi2_w_priority(RW) 00000_011 axi2_r_priority(RW) 
    DRAM_REG[54] =  0x00012100  ;  //0_0000000 axi3_current_bdw(RD) 0000000_1 axi3_bdw_ovflow(RW) 0_0100001 axi3_bdw(RW) 000000_00 axi3_fifo_type_reg(RW) 
    DRAM_REG[55] =  0xffff0303  ;  //0000000000000001 axi3_en_size_lt_width_instr(RW) 00000_011 axi3_w_priority(RW) 00000_011 axi3_r_priority(RW) 
    DRAM_REG[56] =  0x00000003  ;  //00000000000000000000000000000_011 arb_cmd_q_threshold(RW) 
    DRAM_REG[57] =  0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[58] =  0x00000000  ;  //00000_00000000000 int_status(RD) 00000_00000000000 int_mask(RW) 
    DRAM_REG[59] =  0x00000000  ;  //00000000000000000000000000000000 out_of_range_addr(RD) 
    DRAM_REG[60] =  0x00000000  ;  //000000000000000000000000000000_00
    DRAM_REG[61] =  0x00000000  ;  //00_000000 out_of_range_type(RD) 0_0000000 out_of_range_length(RD) 000_0000000000000 out_of_range_source_id(RD) 
    DRAM_REG[62] =  0x00000000  ;  //00000000000000000000000000000000 port_cmd_error_addr(RD) 
    DRAM_REG[63] =  0x00000000  ;  //000000000000000000000000000000_00
    DRAM_REG[64] =  0x00000000  ;  //00000000000_0000000000000 port_cmd_error_id(RD) 0000_0000 port_cmd_error_type(RD) 
    DRAM_REG[65] =  0x00000000  ;  //00000000000_0000000000000 port_data_error_id(RD) 00000_000 port_data_error_type(RD) 
    DRAM_REG[66] =  0x00000409  ;  //000000000000_0000 tdfi_ctrlupd_min(RD) 00_00010000001001 tdfi_ctrlupd_max(RW) 
    DRAM_REG[67] =  0x01000f02  ;  //0000_0001 tdfi_dram_clk_enable(RW) 00000_000 tdfi_dram_clk_disable(RW) 0000_0000 dram_clk_disable(RW) 0000_0010 tdfi_ctrl_delay(RW) 
    DRAM_REG[68] =  0x04090409  ;  //00_00010000001001 tdfi_phyupd_type0(RW) 00_00010000001001 tdfi_phyupd_resp(RW) 
    DRAM_REG[69] =  0x00000200  ;  //00000000000000000000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 
    DRAM_REG[70] =  0x00020006  ;  //000000000000_0010 tdfi_rddata_en_base(RW) 0000_0000 tdfi_rddata_en(RD) 0000_0110 tdfi_phy_rdlat(RW) 
    DRAM_REG[71] =  0xf4004a27  ;
    DRAM_REG[72] =  0xf4004a27  ;
    DRAM_REG[73] =  0xf4004a27  ;
    DRAM_REG[74] =  0xf4004a27  ;   
    DRAM_REG[75] =  0x07000300  ;  //00000111000000000000001100000000 phy_ctrl_reg_1_0(RW) 
    DRAM_REG[76] =  0x07000300  ;  //00000111000000000000001100000000 phy_ctrl_reg_1_1(RW) 
    DRAM_REG[77] =  0x07400300  ;  //00000111010000000000001100000000 phy_ctrl_reg_1_2(RW) 
    DRAM_REG[78] =  0x07400300  ;  //00000111010000000000001100000000 phy_ctrl_reg_1_3(RW) 
    DRAM_REG[79] =  0x00000005  ;   
    DRAM_REG[80] =  0x00000000  ;  //00000000000000000000000000000000 dft_ctrl_reg(RW) 
    DRAM_REG[81] =  0x00000000  ;  //0000000000000000000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 
    DRAM_REG[82] =  0x01000000  ;  //0000000_1 odt_alt_en(RW) 000000000000000000000000
    DRAM_REG[83] =  0x01020408  ;  //0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
    DRAM_REG[84] =  0x08040201  ;  //0000_1000 odt_wr_map_cs3(RW) 0000_0100 odt_wr_map_cs2(RW) 0000_0010 odt_wr_map_cs1(RW) 0000_0001 odt_wr_map_cs0(RW) 
    DRAM_REG[85] =  0x000f1133  ;  //00000000000011110001000100110011 pad_ctrl_reg_0(RW) 
    DRAM_REG[86] =  0x00000000  ;  //00000000000000000000000000000000 version(RD) 
    DRAM_REG[87] =  0x00001f04  ; 
    DRAM_REG[88] =  0x00001f04  ; 
    DRAM_REG[89] =  0x00001f04  ; 
    DRAM_REG[90] =  0x00001f04  ; 
    DRAM_REG[91] =  0x00001f04  ; 
    DRAM_REG[92] =  0x00001f04  ; 
    DRAM_REG[93] =  0x00001f04  ; 
    DRAM_REG[94] =  0x00001f04  ;    
    DRAM_REG[95] =  0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_0_0(RD) 
    DRAM_REG[96] =  0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_0_1(RD) 
    DRAM_REG[97] =  0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_0_2(RD) 
    DRAM_REG[98] =  0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_0_3(RD) 
    DRAM_REG[99] =  0x00000000  ;  //00000000000000000000000000000000 phy_obs_reg_0_0(RD) 
    DRAM_REG[100] = 0x00000000  ;  //00000000000000000000000000000000 phy_obs_reg_0_1(RD) 
    DRAM_REG[101] = 0x00000000  ;  //00000000000000000000000000000000 phy_obs_reg_0_2(RD) 
    DRAM_REG[102] = 0x00000000  ;  //00000000000000000000000000000000 phy_obs_reg_0_3(RD) 
    DRAM_REG[103] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_1_0(RD) 
    DRAM_REG[104] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[105] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[106] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[107] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[108] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_1_1(RD) 
    DRAM_REG[109] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[110] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[111] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[112] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[113] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_1_2(RD) 
    DRAM_REG[114] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[115] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[116] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[117] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[118] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_1_3(RD) 
    DRAM_REG[119] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[120] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[121] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[122] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[123] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_2_0(RD) 
    DRAM_REG[124] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[125] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[126] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[127] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[128] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_2_1(RD) 
    DRAM_REG[129] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[130] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[131] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[132] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[133] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_2_2(RD) 
    DRAM_REG[134] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[135] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[136] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[137] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[138] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_2_3(RD) 
    DRAM_REG[139] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[140] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[141] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[142] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[143] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_3_0(RD) 
    DRAM_REG[144] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[145] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[146] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[147] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[148] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_3_1(RD) 
    DRAM_REG[149] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[150] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[151] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[152] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[153] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_3_2(RD) 
    DRAM_REG[154] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[155] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[156] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[157] = 0x00000000  ;  //00000000000000000000000_000000000
    DRAM_REG[158] = 0x00000000  ;  //00000000000000000000000000000000 dll_obs_reg_3_3(RD) 
    DRAM_REG[159] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[160] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[161] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[162] = 0x00010000  ;  //00000_000 w2r_samecs_dly(RW) 00000_001 w2r_diffcs_dly(RW) 0000000_000000000
    DRAM_REG[163] = 0x00030404  ;  //00000000 dll_rst_adj_dly(RW) 0000_0011 wrlat_adj(RW) 0000_0100 rdlat_adj(RW) 0000_0100 dram_class(RW) 
    DRAM_REG[164] = 0x00000002  ;  //00000000000000_0000000000 int_ack(WR) 00000010 tmod(RW) 
    DRAM_REG[165] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[166] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[167] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[168] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[169] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[170] = 0x00000000  ;  //00000000000000000000000000000000
    DRAM_REG[171] = 0x01010000  ;  //0000000_1 axi5_bdw_ovflow(RW) 0000000_1 axi4_bdw_ovflow(RW) 0000000000000000 dll_rst_delay(RW) 
    DRAM_REG[172] = 0x01000000  ;  //0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 concurrentap_wr_only(RW) 0000000_0 cke_status(RD) 
    DRAM_REG[173] = 0x03030000  ;  //00000_011 axi4_w_priority(RW) 00000_011 axi4_r_priority(RW) 000000_00 axi5_fifo_type_reg(RW) 000000_00 axi4_fifo_type_reg(RW) 
    DRAM_REG[174] = 0x00010303  ;  //00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_011 axi5_w_priority(RW) 00000_011 axi5_r_priority(RW) 
    DRAM_REG[175] = 0x01020202  ;  //00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 
    DRAM_REG[176] = 0x00000000  ;  //0000_0000 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 0000_0000 add_odt_clk_difftype_diffcs(RW) 00000_000 w2w_samecs_dly(RW) 
    DRAM_REG[177] = 0x02030303  ;  //000_00010 tccd(RW) 0000_0011 trp_ab(RW) 0000_0011 cksrx(RW) 0000_0011 cksre(RW) 
    DRAM_REG[178] = 0x21002103  ;  //0_0100001 axi5_bdw(RW) 0_0000000 axi4_current_bdw(RD) 0_0100001 axi4_bdw(RW) 000_00011 tckesr(RW) 
    DRAM_REG[179] = 0x00040900  ;  //0000000000_00010000001001 tdfi_phyupd_type1(RW) 0_0000000 axi5_current_bdw(RD) 
    DRAM_REG[180] = 0x04090409  ;  //00_00010000001001 tdfi_phyupd_type3(RW) 00_00010000001001 tdfi_phyupd_type2(RW) 
    DRAM_REG[181] = 0x02420242  ;  //0_000001001000010 mr0_data_1(RW) 0_000001001000010 mr0_data_0(RW) 
    DRAM_REG[182] = 0x02420242  ;  //0_000001001000010 mr0_data_3(RW) 0_000001001000010 mr0_data_2(RW) 
    DRAM_REG[183] = 0x00040004  ;  //0_000000000000100 mr1_data_1(RW) 0_000000000000100 mr1_data_0(RW) 
    DRAM_REG[184] = 0x00040004  ;  //0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 
    DRAM_REG[185] = 0x00000000  ;  //0_000000000000000 mr2_data_1(RW) 0_000000000000000 mr2_data_0(RW) 
    DRAM_REG[186] = 0x00000000  ;  //0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 
    DRAM_REG[187] = 0x00000000  ;  //0_000000000000000 mr3_data_1(RW) 0_000000000000000 mr3_data_0(RW) 
    DRAM_REG[188] = 0x00000000  ;  //0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 
    DRAM_REG[189] = 0xffffffff  ;  //0000000000000001 axi5_en_size_lt_width_instr(RW) 0000000000000001 axi4_en_size_lt_width_instr(RW) 

}

//-----------------------------------------------------------------------------
//
//  Function:  DDR2EmiController_EDE1116_166MHz
//
//  This function is to initial EMI controlller for DDR2 166MHz.
//
//  Parameters:
//          None.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void DDR2EmiController_EDE1116_166MHz(void)
{

    DRAM_REG[0] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_0(RW) 
    DRAM_REG[1] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_1(RW) 
    DRAM_REG[2] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_2(RW) 
    DRAM_REG[3] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_3(RW) 
    DRAM_REG[4] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_4(RW) 
    DRAM_REG[5] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_5(RW) 
    DRAM_REG[6] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_6(RW) 
    DRAM_REG[7] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_7(RW) 
    DRAM_REG[8] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_0(RD) 
    DRAM_REG[9] =   0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_1(RD) 
    DRAM_REG[10] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_2(RD) 
    DRAM_REG[11] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_3(RD) 
    DRAM_REG[12] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_4(RD) 
    DRAM_REG[13] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_5(RD) 
    DRAM_REG[14] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_6(RD) 
    DRAM_REG[15] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_7(RD) 
    DRAM_REG[16] =  0x00000000 ; //0000000_0 write_modereg(WR) 0000000_0 power_down(RW) 000000000000000_0 start(RW) 
    DRAM_REG[17] =  0x00000100 ; //0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_1 enable_quick_srefresh(RW) 0000000_0 srefresh(RW+) 
    DRAM_REG[18] =  0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[19] =  0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[20] =  0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[21] =  0x00000000 ; //00000_000 cke_delay(RW) 00000000 dll_lock(RD) 0000000_0 dlllockreg(RD) 0000000_0 dll_bypass_mode(RW) 
    DRAM_REG[22] =  0x00000000 ; //000000000000_0000 lowpower_refresh_enable(RW) 000_00000 lowpower_control(RW) 000_00000 lowpower_auto_enable(RW) 
    DRAM_REG[23] =  0x00000000 ; //0000000000000000 lowpower_internal_cnt(RW) 0000000000000000 lowpower_external_cnt(RW) 
    DRAM_REG[24] =  0x00000000 ; //0000000000000000 lowpower_self_refresh_cnt(RW) 0000000000000000 lowpower_refresh_hold(RW) 
    DRAM_REG[25] =  0x00000000 ; //00000000000000000000000000000000 lowpower_power_down_cnt(RW) 
    DRAM_REG[26] =  0x00010101 ; //000000000000000_1 priority_en(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 placement_en(RW) 
    DRAM_REG[27] =  0x01010101 ; //0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_1 bank_split_en(RW) 0000000_1 rw_same_en(RW) 
    DRAM_REG[28] =  0x000f0f01 ; //00000_000 q_fullness(RW) 0000_1111 age_count(RW) 0000_1111 command_age_count(RW) 0000000_1 active_aging(RW) 
    DRAM_REG[29] =  0x0f02020a ; //0000_1111 cs_map(RW) 00000_010 column_size(RW) 00000_010 addr_pins(RW) 0000_1010 aprebit(RW) 
    DRAM_REG[30] =  0x00000000 ; //0000000000000_000 max_cs_reg(RD) 0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 
    DRAM_REG[31] =  0x00010101 ; //000000000000000_1 eight_bank_mode(RW) 0000000_1 drive_dq_dqs(RW) 0000000_1 dqs_n_en(RW) 
    DRAM_REG[32] =  0x00000100 ; //00000000000000000000000_1 reduc(RW) 0000000_0 reg_dimm_enable(RW) 
    DRAM_REG[33] =  0x00000100 ; //00000000000000000000000_1 concurrentap(RW) 0000000_0 ap(RW) 
    DRAM_REG[34] =  0x00000000 ; //0000000_0 writeinterp(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
    DRAM_REG[35] =  0x00000002 ; //000000000000000_0 pwrup_srefresh_exit(RW) 0000000_0 no_cmd_init(RW) 0000_0010 initaref(RW) 
    DRAM_REG[36] =  0x01010000 ; //0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 000000000000000_0 fast_write(RW) 
    DRAM_REG[37] =  0x07080403 ; //0000_0111 caslat_lin_gate(RW) 0000_1000 caslat_lin(RW) 00000_100 caslat(RW) 0000_0011 wrlat(RW) 
    DRAM_REG[38] =  0x06004303 ; //000_00110 tdal(RW) 0000000001000011 tcpd(RW) 00000_011 tcke(RW) 
    DRAM_REG[39] =  0x090000c8 ; //00_001001 tfaw(RW) 000000000000000011001000 tdll(RW) 
    DRAM_REG[40] =  0x02008236 ; //000_00010 tmrd(RW) 000000001000001000110110 tinit(RW) 
    DRAM_REG[41] =  0x0002030a ; //0000000000000010 tpdex(RW) 00000011 trcd_int(RW) 00_001010 trc(RW) 
    DRAM_REG[42] =  0x002d8908 ; //000000000010110110001001 tras_max(RW) 00001000 tras_min(RW) 
    DRAM_REG[43] =  0x0316050e ; //0000_0011 trp(RW) 00010110 trfc(RW) 00_00010100001110 tref(RW) 
    DRAM_REG[44] =  0x02030202 ; //0000_0010 twtr(RW) 000_00011 twr_int(RW) 00000_010 trtp(RW) 00000_010 trrd(RW) 
    DRAM_REG[45] =  0x00c80017 ; //0000000011001000 txsr(RW) 0000000000010111 txsnr(RW) 
    DRAM_REG[46] =  0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[47] =  0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[48] =  0x00012100 ; //0_0000000 axi0_current_bdw(RD) 0000000_1 axi0_bdw_ovflow(RW) 0_0100001 axi0_bdw(RW) 000000_00 axi0_fifo_type_reg(RW) 
    DRAM_REG[49] =  0xffff0303 ; //0101010101010101 axi0_en_size_lt_width_instr(RW) 00000_011 axi0_w_priority(RW) 00000_011 axi0_r_priority(RW) 
    DRAM_REG[50] =  0x00012100 ; //0_0000000 axi1_current_bdw(RD) 0000000_1 axi1_bdw_ovflow(RW) 0_0100001 axi1_bdw(RW) 000000_00 axi1_fifo_type_reg(RW) 
    DRAM_REG[51] =  0xff000303 ; //1111111100000000 axi1_en_size_lt_width_instr(RW) 00000_011 axi1_w_priority(RW) 00000_011 axi1_r_priority(RW) 
    DRAM_REG[52] =  0x00012100 ; //0_0000000 axi2_current_bdw(RD) 0000000_1 axi2_bdw_ovflow(RW) 0_0100001 axi2_bdw(RW) 000000_00 axi2_fifo_type_reg(RW) 
    DRAM_REG[53] =  0xffff0303 ; //0000000000000001 axi2_en_size_lt_width_instr(RW) 00000_011 axi2_w_priority(RW) 00000_011 axi2_r_priority(RW) 
    DRAM_REG[54] =  0x00012100 ; //0_0000000 axi3_current_bdw(RD) 0000000_1 axi3_bdw_ovflow(RW) 0_0100001 axi3_bdw(RW) 000000_00 axi3_fifo_type_reg(RW) 
    DRAM_REG[55] =  0xffff0303 ; //0000000000000001 axi3_en_size_lt_width_instr(RW) 00000_011 axi3_w_priority(RW) 00000_011 axi3_r_priority(RW) 
    DRAM_REG[56] =  0x00000003 ; //00000000000000000000000000000_011 arb_cmd_q_threshold(RW) 
    DRAM_REG[57] =  0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[58] =  0x00000000 ; //00000_00000000000 int_status(RD) 00000_00000000000 int_mask(RW) 
    DRAM_REG[59] =  0x00000000 ; //00000000000000000000000000000000 out_of_range_addr(RD) 
    DRAM_REG[60] =  0x00000000 ; //000000000000000000000000000000_00
    DRAM_REG[61] =  0x00000000 ; //00_000000 out_of_range_type(RD) 0_0000000 out_of_range_length(RD) 000_0000000000000 out_of_range_source_id(RD) 
    DRAM_REG[62] =  0x00000000 ; //00000000000000000000000000000000 port_cmd_error_addr(RD) 
    DRAM_REG[63] =  0x00000000 ; //000000000000000000000000000000_00
    DRAM_REG[64] =  0x00000000 ; //00000000000_0000000000000 port_cmd_error_id(RD) 0000_0000 port_cmd_error_type(RD) 
    DRAM_REG[65] =  0x00000000 ; //00000000000_0000000000000 port_data_error_id(RD) 00000_000 port_data_error_type(RD) 
    DRAM_REG[66] =  0x0000050e ; //000000000000_0000 tdfi_ctrlupd_min(RD) 00_00010100001110 tdfi_ctrlupd_max(RW) 
    DRAM_REG[67] =  0x01000f02 ; //0000_0001 tdfi_dram_clk_enable(RW) 00000_000 tdfi_dram_clk_disable(RW) 0000_0000 dram_clk_disable(RW) 0000_0010 tdfi_ctrl_delay(RW) 
    DRAM_REG[68] =  0x050e050e ; //00_00010100001110 tdfi_phyupd_type0(RW) 00_00010100001110 tdfi_phyupd_resp(RW) 
    DRAM_REG[69] =  0x00000200 ; //00000000000000000000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 
    DRAM_REG[70] =  0x00020007 ; //000000000000_0010 tdfi_rddata_en_base(RW) 0000_0000 tdfi_rddata_en(RD) 0000_0111 tdfi_phy_rdlat(RW) 
    DRAM_REG[71] =  0xf5004a27 ;
    DRAM_REG[72] =  0xf5004a27 ;
    DRAM_REG[73] =  0xf5004a27 ;
    DRAM_REG[74] =  0xf5004a27 ;       
    DRAM_REG[75] =  0x07000300 ; //00000111000000000000001100000000 phy_ctrl_reg_1_0(RW) 
    DRAM_REG[76] =  0x07000300 ; //00000111000000000000001100000000 phy_ctrl_reg_1_1(RW) 
    DRAM_REG[77] =  0x07400300 ; //00000111010000000000001100000000 phy_ctrl_reg_1_2(RW) 
    DRAM_REG[78] =  0x07400300 ; //00000111010000000000001100000000 phy_ctrl_reg_1_3(RW) 
    //DRAM_REG[79] =  0x00000005 ; //00000000000000000000000000000101 phy_ctrl_reg_2(RW) 
    DRAM_REG[79] =  0x00000006 ;
    DRAM_REG[80] =  0x00000000 ; //00000000000000000000000000000000 dft_ctrl_reg(RW) 
    DRAM_REG[81] =  0x00000000 ; //0000000000000000000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 
    DRAM_REG[82] =  0x01000000 ; //0000000_1 odt_alt_en(RW) 000000000000000000000000
    DRAM_REG[83] =  0x01020408 ; //0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
    DRAM_REG[84] =  0x08040201 ; //0000_1000 odt_wr_map_cs3(RW) 0000_0100 odt_wr_map_cs2(RW) 0000_0010 odt_wr_map_cs1(RW) 0000_0001 odt_wr_map_cs0(RW) 
    DRAM_REG[85] =  0x000f1133 ; //00000000000011110001000100110011 pad_ctrl_reg_0(RW) 
    DRAM_REG[86] =  0x00000000 ; //00000000000000000000000000000000 version(RD)     
    DRAM_REG[87] =  0x00001f04 ; 
    DRAM_REG[88] =  0x00001f04 ; 
    DRAM_REG[89] =  0x00001f04 ; 
    DRAM_REG[90] =  0x00001f04 ; 
    DRAM_REG[91] =  0x00001f04 ; 
    DRAM_REG[92] =  0x00001f04 ; 
    DRAM_REG[93] =  0x00001f04 ; 
    DRAM_REG[94] =  0x00001f04 ;    
    DRAM_REG[95] =  0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_0(RD) 
    DRAM_REG[96] =  0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_1(RD) 
    DRAM_REG[97] =  0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_2(RD) 
    DRAM_REG[98] =  0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_3(RD) 
    DRAM_REG[99] =  0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_0(RD) 
    DRAM_REG[100] = 0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_1(RD) 
    DRAM_REG[101] = 0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_2(RD) 
    DRAM_REG[102] = 0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_3(RD) 
    DRAM_REG[103] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_0(RD) 
    DRAM_REG[104] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[105] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[106] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[107] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[108] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_1(RD) 
    DRAM_REG[109] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[110] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[111] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[112] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[113] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_2(RD) 
    DRAM_REG[114] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[115] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[116] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[117] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[118] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_3(RD) 
    DRAM_REG[119] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[120] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[121] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[122] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[123] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_0(RD) 
    DRAM_REG[124] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[125] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[126] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[127] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[128] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_1(RD) 
    DRAM_REG[129] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[130] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[131] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[132] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[133] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_2(RD) 
    DRAM_REG[134] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[135] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[136] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[137] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[138] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_3(RD) 
    DRAM_REG[139] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[140] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[141] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[142] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[143] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_0(RD) 
    DRAM_REG[144] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[145] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[146] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[147] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[148] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_1(RD) 
    DRAM_REG[149] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[150] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[151] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[152] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[153] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_2(RD) 
    DRAM_REG[154] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[155] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[156] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[157] = 0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[158] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_3(RD) 
    DRAM_REG[159] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[160] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[161] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[162] = 0x00010000 ; //00000_000 w2r_samecs_dly(RW) 00000_001 w2r_diffcs_dly(RW) 0000000_000000000
    DRAM_REG[163] = 0x00030404 ; //00000000 dll_rst_adj_dly(RW) 0000_0011 wrlat_adj(RW) 0000_0100 rdlat_adj(RW) 0000_0100 dram_class(RW) 
    DRAM_REG[164] = 0x00000002 ; //00000000000000_0000000000 int_ack(WR) 00000010 tmod(RW) 
    DRAM_REG[165] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[166] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[167] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[168] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[169] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[170] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[171] = 0x01010000 ; //0000000_1 axi5_bdw_ovflow(RW) 0000000_1 axi4_bdw_ovflow(RW) 0000000000000000 dll_rst_delay(RW) 
    DRAM_REG[172] = 0x01000000 ; //0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 concurrentap_wr_only(RW) 0000000_0 cke_status(RD) 
    DRAM_REG[173] = 0x03030000 ; //00000_011 axi4_w_priority(RW) 00000_011 axi4_r_priority(RW) 000000_00 axi5_fifo_type_reg(RW) 000000_00 axi4_fifo_type_reg(RW) 
    DRAM_REG[174] = 0x00010303 ; //00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_011 axi5_w_priority(RW) 00000_011 axi5_r_priority(RW) 
    DRAM_REG[175] = 0x01020202 ; //00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 
    DRAM_REG[176] = 0x00000000 ; //0000_0000 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 0000_0000 add_odt_clk_difftype_diffcs(RW) 00000_000 w2w_samecs_dly(RW) 
    DRAM_REG[177] = 0x02040303 ; //000_00010 tccd(RW) 0000_0100 trp_ab(RW) 0000_0011 cksrx(RW) 0000_0011 cksre(RW) 
    DRAM_REG[178] = 0x21002103 ; //0_0100001 axi5_bdw(RW) 0_0000000 axi4_current_bdw(RD) 0_0100001 axi4_bdw(RW) 000_00011 tckesr(RW) 
    DRAM_REG[179] = 0x00050e00 ; //0000000000_00010100001110 tdfi_phyupd_type1(RW) 0_0000000 axi5_current_bdw(RD) 
    DRAM_REG[180] = 0x050e050e ; //00_00010100001110 tdfi_phyupd_type3(RW) 00_00010100001110 tdfi_phyupd_type2(RW) 
    DRAM_REG[181] = 0x04420442 ; //0_000010001000010 mr0_data_1(RW) 0_000010001000010 mr0_data_0(RW) 
    DRAM_REG[182] = 0x04420442 ; //0_000010001000010 mr0_data_3(RW) 0_000010001000010 mr0_data_2(RW) 
    DRAM_REG[183] = 0x00040004 ; //0_000000000000100 mr1_data_1(RW) 0_000000000000100 mr1_data_0(RW) 
    DRAM_REG[184] = 0x00040004 ; //0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 
    DRAM_REG[185] = 0x00000000 ; //0_000000000000000 mr2_data_1(RW) 0_000000000000000 mr2_data_0(RW) 
    DRAM_REG[186] = 0x00000000 ; //0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 
    DRAM_REG[187] = 0x00000000 ; //0_000000000000000 mr3_data_1(RW) 0_000000000000000 mr3_data_0(RW) 
    DRAM_REG[188] = 0x00000000 ; //0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 
    DRAM_REG[189] = 0xffffffff ; //0000000000000001 axi5_en_size_lt_width_instr(RW) 0000000000000001 axi4_en_size_lt_width_instr(RW) 

}

//-----------------------------------------------------------------------------
//
//  Function:  DDR2EmiController_EDE1116_200MHz
//
//  This function is to initial EMI controlller for DDR2 200MHz.
//
//  Parameters:
//          None.
//  Returns:
//          None.
//
//-----------------------------------------------------------------------------
void DDR2EmiController_EDE1116_200MHz(void)
{
    DRAM_REG[0] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_0(RW) 
    DRAM_REG[1] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_1(RW) 
    DRAM_REG[2] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_2(RW) 
    DRAM_REG[3] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_3(RW) 
    DRAM_REG[4] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_4(RW) 
    DRAM_REG[5] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_5(RW) 
    DRAM_REG[6] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_6(RW) 
    DRAM_REG[7] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_7(RW) 
    DRAM_REG[8] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_0(RD) 
    DRAM_REG[9] =  0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_1(RD) 
    DRAM_REG[10] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_2(RD) 
    DRAM_REG[11] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_3(RD) 
    DRAM_REG[12] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_4(RD) 
    DRAM_REG[13] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_5(RD) 
    DRAM_REG[14] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_6(RD) 
    DRAM_REG[15] = 0x00000000 ; //00000000000000000000000000000000 user_def_reg_ro_7(RD) 
    DRAM_REG[16] = 0x00000000 ; //0000000_0 write_modereg(WR) 0000000_0 power_down(RW) 000000000000000_0 start(RW) 
    DRAM_REG[17] = 0x00000100 ; //0000000_0 auto_refresh_mode(RW) 0000000_0 arefresh(WR) 0000000_1 enable_quick_srefresh(RW) 0000000_0 srefresh(RW+) 
    DRAM_REG[18] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[19] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[20] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[21] = 0x00000000 ; //00000_000 cke_delay(RW) 00000000 dll_lock(RD) 0000000_0 dlllockreg(RD) 0000000_0 dll_bypass_mode(RW) 
    DRAM_REG[22] = 0x00000000 ; //000000000000_0000 lowpower_refresh_enable(RW) 000_00000 lowpower_control(RW) 000_01000 lowpower_auto_enable(RW) 
    DRAM_REG[23] = 0x00000000 ; //0000000000000000 lowpower_internal_cnt(RW) 0000000000000000 lowpower_external_cnt(RW) 
    DRAM_REG[24] = 0x00000000 ; //0000000000000000 lowpower_self_refresh_cnt(RW) 0000000000000000 lowpower_refresh_hold(RW) 
    DRAM_REG[25] = 0x00000000 ; //00000000000000000000000000100000 lowpower_power_down_cnt(RW) 
    DRAM_REG[26] = 0x00010101 ; //000000000000000_1 priority_en(RW) 0000000_1 addr_cmp_en(RW) 0000000_1 placement_en(RW) 
    DRAM_REG[27] = 0x01010101 ; //0000000_1 swap_port_rw_same_en(RW) 0000000_1 swap_en(RW) 0000000_1 bank_split_en(RW) 0000000_1 rw_same_en(RW) 
    DRAM_REG[28] = 0x000f0f01 ; //00000_000 q_fullness(RW) 0000_1111 age_count(RW) 0000_1111 command_age_count(RW) 0000000_1 active_aging(RW) 
    DRAM_REG[29] = 0x0f02020a ; //0000_1111 cs_map(RW) 00000_010 column_size(RW) 00000_010 addr_pins(RW) 0000_1010 aprebit(RW) 
    DRAM_REG[30] = 0x00000000 ; //0000000000000_000 max_cs_reg(RD) 0000_0000 max_row_reg(RD) 0000_0000 max_col_reg(RD) 
    DRAM_REG[31] = 0x00010101 ; //000000000000000_1 eight_bank_mode(RW) 0000000_1 drive_dq_dqs(RW) 0000000_1 dqs_n_en(RW) 
    DRAM_REG[32] = 0x00000100 ; //00000000000000000000000_1 reduc(RW) 0000000_0 reg_dimm_enable(RW) 
    DRAM_REG[33] = 0x00000100 ; //00000000000000000000000_1 concurrentap(RW) 0000000_0 ap(RW) 
    DRAM_REG[34] = 0x00000000 ; //0000000_0 writeinterp(RW) 0000000_0 intrptwritea(RW) 0000000_0 intrptreada(RW) 0000000_0 intrptapburst(RW) 
    DRAM_REG[35] = 0x00000002 ; //000000000000000_0 pwrup_srefresh_exit(RW) 0000000_0 no_cmd_init(RW) 0000_0010 initaref(RW) 
    DRAM_REG[36] = 0x01010000 ; //0000000_1 tref_enable(RW) 0000000_1 tras_lockout(RW) 000000000000000_0 fast_write(RW) 
    DRAM_REG[37] = 0x07080403 ; //0000_0111 caslat_lin_gate(RW) 0000_1000 caslat_lin(RW) 00000_100 caslat(RW) 0000_0011 wrlat(RW) 
    DRAM_REG[38] = 0x06005003 ; //000_00110 tdal(RW) 0000000001010000 tcpd(RW) 00000_011 tcke(RW) 
    DRAM_REG[39] = 0x0a0000c8 ; //00_001010 tfaw(RW) 000000000000000011001000 tdll(RW) 
    DRAM_REG[40] = 0x02009c40 ; //000_00010 tmrd(RW) 000000000111010100100010 tinit(RW) 
    DRAM_REG[41] = 0x0002030c ; //0000000000000010 tpdex(RW) 00000011 trcd_int(RW) 00_001100 trc(RW) 
    DRAM_REG[42] = 0x0036a609 ; //000000000011011010100110 tras_max(RW) 00001001 tras_min(RW) 
    DRAM_REG[43] = 0x031a0612 ; //0000_0011 trp(RW) 00011010 trfc(RW) 00_00011000010010 tref(RW) 
    DRAM_REG[44] = 0x02030202 ; //0000_0010 twtr(RW) 000_00011 twr_int(RW) 00000_010 trtp(RW) 00000_010 trrd(RW) 
    DRAM_REG[45] = 0x00c8001c ; //0000000011001000 txsr(RW) 0000000000011100 txsnr(RW) 
    DRAM_REG[46] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[47] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[48] = 0x00012100 ; //0_0000000 axi0_current_bdw(RD) 0000000_1 axi0_bdw_ovflow(RW) 0_0100001 axi0_bdw(RW) 000000_00 axi0_fifo_type_reg(RW) 
    //DRAM_REG[49] = 0x55550303 ; //0101010101010101 axi0_en_size_lt_width_instr(RW) 00000_011 axi0_w_priority(RW) 00000_011 axi0_r_priority(RW) 
    DRAM_REG[49] = 0xffff0303 ; //0101010101010101 axi0_en_size_lt_width_instr(RW) 00000_011 axi0_w_priority(RW) 00000_011 axi0_r_priority(RW) 
    DRAM_REG[50] = 0x00012100 ; //0_0000000 axi1_current_bdw(RD) 0000000_1 axi1_bdw_ovflow(RW) 0_0100001 axi1_bdw(RW) 000000_00 axi1_fifo_type_reg(RW) 
    DRAM_REG[51] = 0xffff0303 ; //1111111100000000 axi1_en_size_lt_width_instr(RW) 00000_011 axi1_w_priority(RW) 00000_011 axi1_r_priority(RW) 
    DRAM_REG[52] = 0x00012100 ; //0_0000000 axi2_current_bdw(RD) 0000000_1 axi2_bdw_ovflow(RW) 0_0100001 axi2_bdw(RW) 000000_00 axi2_fifo_type_reg(RW) 
    DRAM_REG[53] = 0xffff0303 ; //0000000000000001 axi2_en_size_lt_width_instr(RW) 00000_011 axi2_w_priority(RW) 00000_011 axi2_r_priority(RW) 
    DRAM_REG[54] = 0x00012100 ; //0_0000000 axi3_current_bdw(RD) 0000000_1 axi3_bdw_ovflow(RW) 0_0100001 axi3_bdw(RW) 000000_00 axi3_fifo_type_reg(RW) 
    DRAM_REG[55] = 0xffff0303 ; //0000000000000001 axi3_en_size_lt_width_instr(RW) 00000_011 axi3_w_priority(RW) 00000_011 axi3_r_priority(RW) 
    DRAM_REG[56] = 0x00000003 ; //00000000000000000000000000000_011 arb_cmd_q_threshold(RW) 
    DRAM_REG[57] = 0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[58] = 0x00000000 ; //00000_00000000000 int_status(RD) 00000_00000000000 int_mask(RW) 
    DRAM_REG[59] = 0x00000000 ; //00000000000000000000000000000000 out_of_range_addr(RD) 
    DRAM_REG[60] = 0x00000000 ; //000000000000000000000000000000_00
    DRAM_REG[61] = 0x00000000 ; //00_000000 out_of_range_type(RD) 0_0000000 out_of_range_length(RD) 000_0000000000000 out_of_range_source_id(RD) 
    DRAM_REG[62] = 0x00000000 ; //00000000000000000000000000000000 port_cmd_error_addr(RD) 
    DRAM_REG[63] = 0x00000000 ; //000000000000000000000000000000_00
    DRAM_REG[64] = 0x00000000 ; //00000000000_0000000000000 port_cmd_error_id(RD) 0000_0000 port_cmd_error_type(RD) 
    DRAM_REG[65] = 0x00000000 ; //00000000000_0000000000000 port_data_error_id(RD) 00000_000 port_data_error_type(RD) 
    DRAM_REG[66] = 0x00000612 ; //000000000000_0000 tdfi_ctrlupd_min(RD) 00_00011000010010 tdfi_ctrlupd_max(RW) 
    //DRAM_REG[67] = 0x01000002 ; //0000_0001 tdfi_dram_clk_enable(RW) 00000_000 tdfi_dram_clk_disable(RW) 0000_0000 dram_clk_disable(RW) 0000_0010 tdfi_ctrl_delay(RW) 
    DRAM_REG[67] = 0x01000f02 ; //0000_0001 tdfi_dram_clk_enable(RW) 00000_000 tdfi_dram_clk_disable(RW) 0000_0000 dram_clk_disable(RW) 0000_0010 tdfi_ctrl_delay(RW) 
    DRAM_REG[68] = 0x06120612 ; //00_00011000010010 tdfi_phyupd_type0(RW) 00_00011000010010 tdfi_phyupd_resp(RW) 
    DRAM_REG[69] = 0x00000200 ; //00000000000000000000_0010 tdfi_phy_wrlat_base(RW) 0000_0000 tdfi_phy_wrlat(RD) 
    DRAM_REG[70] = 0x00020007 ; //000000000000_0010 tdfi_rddata_en_base(RW) 0000_0000 tdfi_rddata_en(RD) 0000_0111 tdfi_phy_rdlat(RW) 
    DRAM_REG[71] = 0xf4004a27 ;
    DRAM_REG[72] = 0xf4004a27 ;
    DRAM_REG[73] = 0xf4004a27 ;
    DRAM_REG[74] = 0xf4004a27 ;
    DRAM_REG[75] = 0x07000300 ; //00000111000000000000001100000000 phy_ctrl_reg_1_0(RW) 
    DRAM_REG[76] = 0x07000300 ; //00000111000000000000001100000000 phy_ctrl_reg_1_1(RW) 
    DRAM_REG[77] = 0x07400300 ; //00000111010000000000001100000000 phy_ctrl_reg_1_2(RW) 
    DRAM_REG[78] = 0x07400300 ; //00000111010000000000001100000000 phy_ctrl_reg_1_3(RW) 
    DRAM_REG[79] = 0x00000005 ; //00000000000000000000000000000101 phy_ctrl_reg_2(RW) 
    DRAM_REG[80] = 0x00000000 ; //00000000000000000000000000000000 dft_ctrl_reg(RW) 
    DRAM_REG[81] = 0x00000000 ; //0000000000000000000_00000 ocd_adjust_pup_cs_0(RW) 000_00000 ocd_adjust_pdn_cs_0(RW) 
    DRAM_REG[82] = 0x01000000 ; //0000000_1 odt_alt_en(RW) 000000000000000000000000
    DRAM_REG[83] = 0x01020408 ; //0000_0001 odt_rd_map_cs3(RW) 0000_0010 odt_rd_map_cs2(RW) 0000_0100 odt_rd_map_cs1(RW) 0000_1000 odt_rd_map_cs0(RW) 
    DRAM_REG[84] = 0x08040201 ; //0000_1000 odt_wr_map_cs3(RW) 0000_0100 odt_wr_map_cs2(RW) 0000_0010 odt_wr_map_cs1(RW) 0000_0001 odt_wr_map_cs0(RW) 
    DRAM_REG[85] = 0x000f1133 ; //00000000000011110001000100110011 pad_ctrl_reg_0(RW) 
    DRAM_REG[86] = 0x00000000 ; //00000000000000000000000000000000 version(RD) 
    DRAM_REG[87] = 0x00001f04 ;
    DRAM_REG[88] = 0x00001f04 ;
    DRAM_REG[89] = 0x00001f04 ;
    DRAM_REG[90] = 0x00001f04 ;
    DRAM_REG[91] = 0x00001f04 ;
    DRAM_REG[92] = 0x00001f04 ;
    DRAM_REG[93] = 0x00001f04 ;
    DRAM_REG[94] = 0x00001f04 ;
    DRAM_REG[95] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_0(RD) 
    DRAM_REG[96] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_1(RD) 
    DRAM_REG[97] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_2(RD) 
    DRAM_REG[98] = 0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_0_3(RD) 
    DRAM_REG[99] = 0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_0(RD) 
    DRAM_REG[100] =0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_1(RD) 
    DRAM_REG[101] =0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_2(RD) 
    DRAM_REG[102] =0x00000000 ; //00000000000000000000000000000000 phy_obs_reg_0_3(RD) 
    DRAM_REG[103] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_0(RD) 
    DRAM_REG[104] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[105] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[106] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[107] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[108] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_1(RD) 
    DRAM_REG[109] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[110] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[111] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[112] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[113] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_2(RD) 
    DRAM_REG[114] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[115] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[116] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[117] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[118] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_1_3(RD) 
    DRAM_REG[119] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[120] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[121] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[122] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[123] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_0(RD) 
    DRAM_REG[124] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[125] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[126] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[127] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[128] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_1(RD) 
    DRAM_REG[129] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[130] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[131] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[132] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[133] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_2(RD) 
    DRAM_REG[134] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[135] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[136] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[137] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[138] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_2_3(RD) 
    DRAM_REG[139] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[140] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[141] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[142] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[143] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_0(RD) 
    DRAM_REG[144] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[145] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[146] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[147] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[148] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_1(RD) 
    DRAM_REG[149] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[150] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[151] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[152] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[153] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_2(RD) 
    DRAM_REG[154] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[155] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[156] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[157] =0x00000000 ; //00000000000000000000000_000000000
    DRAM_REG[158] =0x00000000 ; //00000000000000000000000000000000 dll_obs_reg_3_3(RD) 
    DRAM_REG[159] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[160] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[161] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[162] =0x00010000 ; //00000_000 w2r_samecs_dly(RW) 00000_001 w2r_diffcs_dly(RW) 0000000_000000000
    DRAM_REG[163] =0x00030404 ; //00000000 dll_rst_adj_dly(RW) 0000_0011 wrlat_adj(RW) 0000_0100 rdlat_adj(RW) 0000_0100 dram_class(RW) 
    DRAM_REG[164] =0x00000003 ; //00000000000000_0000000000 int_ack(WR) 00000011 tmod(RW) 
    DRAM_REG[165] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[166] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[167] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[168] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[169] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[170] =0x00000000 ; //00000000000000000000000000000000
    DRAM_REG[171] =0x01010000 ; //0000000_1 axi5_bdw_ovflow(RW) 0000000_1 axi4_bdw_ovflow(RW) 0000000000000000 dll_rst_delay(RW) 
    DRAM_REG[172] =0x01000000 ; //0000000_1 resync_dll_per_aref_en(RW) 0000000_0 resync_dll(WR) 0000000_0 concurrentap_wr_only(RW) 0000000_0 cke_status(RD) 
    DRAM_REG[173] =0x03030000 ; //00000_011 axi4_w_priority(RW) 00000_011 axi4_r_priority(RW) 000000_00 axi5_fifo_type_reg(RW) 000000_00 axi4_fifo_type_reg(RW) 
    DRAM_REG[174] =0x00010303 ; //00000_000 r2r_samecs_dly(RW) 00000_001 r2r_diffcs_dly(RW) 00000_011 axi5_w_priority(RW) 00000_011 axi5_r_priority(RW) 
    DRAM_REG[175] =0x01020202 ; //00000_001 w2w_diffcs_dly(RW) 00000_010 tbst_int_interval(RW) 00000_010 r2w_samecs_dly(RW) 00000_010 r2w_diffcs_dly(RW) 
    DRAM_REG[176] =0x00000000 ; //0000_0000 add_odt_clk_sametype_diffcs(RW) 0000_0000 add_odt_clk_difftype_samecs(RW) 0000_0000 add_odt_clk_difftype_diffcs(RW) 00000_000 w2w_samecs_dly(RW) 
    DRAM_REG[177] =0x02040303 ; //000_00010 tccd(RW) 0000_0100 trp_ab(RW) 0000_0011 cksrx(RW) 0000_0011 cksre(RW) 
    DRAM_REG[178] =0x21002103 ; //0_0100001 axi5_bdw(RW) 0_0000000 axi4_current_bdw(RD) 0_0100001 axi4_bdw(RW) 000_00011 tckesr(RW) 
    DRAM_REG[179] =0x00061200 ; //0000000000_00011000010010 tdfi_phyupd_type1(RW) 0_0000000 axi5_current_bdw(RD) 
    DRAM_REG[180] =0x06120612 ; //00_00011000010010 tdfi_phyupd_type3(RW) 00_00011000010010 tdfi_phyupd_type2(RW) 
    DRAM_REG[181] =0x04420442 ; //0_000010001000010 mr0_data_1(RW) 0_000010001000010 mr0_data_0(RW) 
    DRAM_REG[182] =0x04420442 ; //0_000010001000010 mr0_data_3(RW) 0_000010001000010 mr0_data_2(RW) 
    DRAM_REG[183] =0x00040004 ; //0_000000000000100 mr1_data_1(RW) 0_000000000000100 mr1_data_0(RW) 
    DRAM_REG[184] =0x00040004 ; //0_000000000000100 mr1_data_3(RW) 0_000000000000100 mr1_data_2(RW) 
    DRAM_REG[185] =0x00000000 ; //0_000000000000000 mr2_data_1(RW) 0_000000000000000 mr2_data_0(RW) 
    DRAM_REG[186] =0x00000000 ; //0_000000000000000 mr2_data_3(RW) 0_000000000000000 mr2_data_2(RW) 
    DRAM_REG[187] =0x00000000 ; //0_000000000000000 mr3_data_1(RW) 0_000000000000000 mr3_data_0(RW) 
    DRAM_REG[188] =0x00000000 ; //0_000000000000000 mr3_data_3(RW) 0_000000000000000 mr3_data_2(RW) 
    DRAM_REG[189] =0xffffffff ; //0000000000000001 axi5_en_size_lt_width_instr(RW) 0000000000000001 axi4_en_size_lt_width_instr(RW) 

} 

//-----------------------------------------------------------------------------
//
//  Function:  DVFCDDR2SetpointConfig
//
//  This function write  char to UART output
//
//  Parameters:
//          ch
//          [in] char write to UART.
//
//
//  Returns:
//        None.
//
//-----------------------------------------------------------------------------
VOID DVFCDDR2SetpointConfig()
{
    UINT32 i;
    
    pDDRTiming->pv_HwRegDRAM = pv_HWregDRAM;

    // High DDR2 frequency config
    DDR2EmiController_EDE1116_200MHz();

    for(i = 0; i < DRAMCTRLREGNUM; i++)
        pDDRTiming->DDRTimingoptimized_High[i] = DRAM_REG[i];
    
    // Low DDR2 frequency config
    if(g_SetPointConfig[DDK_DVFC_DOMAIN_CPU][DDK_DVFC_SETPOINT_LOW].setPointInfo.freq[DDK_DVFC_FREQ_EMI] == 166)
    {    
        DDR2EmiController_EDE1116_166MHz();
        //RETAILMSG(1, (_T("DVFCDDR2SetpointConfig 166MHz\r\n")));
    }
    else
    {
        DDR2EmiController_EDE1116_133MHz();  
        //RETAILMSG(1, (_T("DVFCDDR2SetpointConfig 133MHz\r\n")));
    }
    
    for(i = 0; i < DRAMCTRLREGNUM; i++)
        pDDRTiming->DDRTimingoptimized_Low[i] = DRAM_REG[i];
    
}
/*
//-----------------------------------------------------------------------------
//
//  Function:  WriteDebugByte
//
//  This function write  char to UART output
//
//  Parameters:
//          ch
//          [in] char write to UART.
//
//  Returns:
//        None.
//
//-----------------------------------------------------------------------------
VOID WriteDebugByte(UINT8 ch)
{
    UINT32 loop = 0;
    if (!pv_HWregUARTDbg)
    {
        return;
    }

    // Spin if FIFO has more than half data.
    //
    while ( (BF_RD( UARTDBGFR, TXFF)) && (loop < 0x7FFF))
    {
        loop++;
    }

    // Write a character byte to the FIFO.
    //
    if(!BF_RD( UARTDBGFR, TXFF))
        BF_WR( UARTDBGDR, DATA, ch);
}
*/
//-----------------------------------------------------------------------------
//
//  Function:  DVFCSetPowerFetsStrength
//
//  This function Set DCDC FETS mode.
//
//  Parameters:
//      pNewCfg
//          [in] new FETS mode.
//
//
//  Returns:
//        None.
//
//-----------------------------------------------------------------------------
void DVFCSetPowerFetsStrength(PDVFC_SETPOINT_CONFIG pNewCfg)
{
    switch (pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_CPU])
    {
    case BSP_DVFS_CPU_HIGH_ARM_FREQ:
        PmuSetFets(PMU_POWER_DOUBLE_FETS);
        break;

    case BSP_DVFS_CPU_LOW_ARM_FREQ:
        PmuSetFets(PMU_POWER_HALF_FETS);
        break;

    case BSP_DVFS_CPU_MED_ARM_FREQ:
    default:        
        PmuSetFets(PMU_POWER_NORMAL_FETS);
        break;
    }
}

//-----------------------------------------------------------------------------
//
//  Function:  EMIClockChange
//
//  This function updates the current EMI clock.
//
//  Parameters:
//      NewClock
//          [in] new frequency.
//
//      EMIFracDiv
//          [in] New EMI clock fraction divider
//
//      EMIDiv
//          [in] New EMI clock divider.
//
//  Returns:
//      Returns TRUE if the setpoint update was successful, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
VOID EMIClockChange(UINT32 NewClock, UINT32 EMIFracDiv, UINT32 EMIDiv)
{
    UINT32 OldEMIFracDiv, OldEMIDiv;
    UINT32 i;
    volatile UINT32 Reg_Data;

    Reg_Data = NewClock;
    Reg_Data = EMIFracDiv;    
    Reg_Data = EMIDiv;

    //pDEBUGChange('A');

    // Clean DCache 
    while(_MoveFromCoprocessor(15, 0, 7, 14, 3) != 0);

    //Drain write buffer
    _MoveToCoprocessor(0, 15, 0, 7, 10, 4);

    //invalidate TLB single entry
    _MoveToCoprocessor(pDDRTiming->pv_HwRegDRAM, 15, 0, 8, 7, 1);
    _MoveToCoprocessor(pv_HWregCLKCTRL, 15, 0, 8, 7, 1);
    _MoveToCoprocessor(pv_HWregDIGCTL, 15, 0, 8, 7, 1);
    _MoveToCoprocessor(pv_HWregDRAM, 15, 0, 8, 7, 1);

    //set the TLB lockdown register preserve bit
    Reg_Data = _MoveFromCoprocessor(15, 0, 10, 0, 0);
    Reg_Data = Reg_Data | 0x1;
    _MoveToCoprocessor(Reg_Data, 15, 0, 10, 0, 0);

    //TLB will miss, and entry will be loaded
    Reg_Data = HW_DIGCTL_MICROSECONDS_RD();
    Reg_Data = HW_DRAM_CTL00_RD();  
    Reg_Data = HW_CLKCTRL_FRAC0_RD();
    Reg_Data = ((PDWORD) pDDRTiming->pv_HwRegDRAM)[0]; 

    //Clear TLB lockdown register preserve bit
    Reg_Data = _MoveFromCoprocessor(15, 0, 10, 0, 0);
    Reg_Data = Reg_Data & 0xFFFFFE;
    _MoveToCoprocessor(Reg_Data, 15, 0, 10, 0, 0);

    //pDEBUGChange('B');

    //1. Ensure EMI is idle
    while(HW_DRAM_CTL08_RD() & BM_DRAM_CTL08_CONTROLLER_BUSY);

    //pDEBUGChange('C');

    //2. Put DDR EnterSelfrefreshMode
    HW_DRAM_CTL17_SET(BM_DRAM_CTL17_SREFRESH);

    //pDEBUGChange('E');

    //3. Wait Memory device have been placed into self-refresh
    while(!(HW_DRAM_CTL172_RD() & BM_DRAM_CTL172_CKE_STATUS));
    
    //4. Make DLL lock state
    HW_DRAM_CTL58_SET(BF_DRAM_CTL58_INT_MASK(0x100));

    //pDEBUGChange('F');

    //5. Stop memory controller
    HW_DRAM_CTL16_CLR(BM_DRAM_CTL16_START);
    

    //6. Clear DLL lock stated
    HW_DRAM_CTL164_CLR(BF_DRAM_CTL164_INT_ACK(0x3ff));

    //8.1 Update DDR Frequency
    OldEMIFracDiv = HW_CLKCTRL_FRAC0.B.EMIFRAC;
    OldEMIDiv = HW_CLKCTRL_EMI.B.DIV_EMI;

    //Update EMI  set value

    // The fractional divider and integer divider must be written in such
    // an order to guarantee that when going from a lower frequency to a
    // higher frequency that any intermediate frequencies do not exceed
    // the final frequency. For this reason, we must make sure to check
    // the current divider values with the new divider values and write
    // them in the correct order.
    if (EMIFracDiv > OldEMIFracDiv) 
    {
        // Write the PLL fractional divider
        HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_EMIFRAC) |
            BF_CLKCTRL_FRAC0_EMIFRAC(EMIFracDiv));           
    }

    if (EMIDiv > OldEMIDiv)
    {
        // Write the PLL EMI clock divider
        HW_CLKCTRL_EMI_WR((HW_CLKCTRL_EMI_RD() & ~BM_CLKCTRL_EMI_DIV_EMI) |
                BF_CLKCTRL_EMI_DIV_EMI(EMIDiv));             
    }
  
    if (EMIFracDiv < OldEMIFracDiv) 
    {
        // Write the PLL fractional divider
        HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_EMIFRAC) |
            BF_CLKCTRL_FRAC0_EMIFRAC(EMIFracDiv));
     }
   
    if (EMIDiv < OldEMIDiv) 
    {
        // Write the PLL EMI clock divider
        HW_CLKCTRL_EMI_WR((HW_CLKCTRL_EMI_RD() & ~BM_CLKCTRL_EMI_DIV_EMI) |
                BF_CLKCTRL_EMI_DIV_EMI(EMIDiv));   
    }
    //pDEBUGChange('G');

    //8.2 Update DDR Timing setting   
    if(NewClock == BSP_DVFS_CPU_LOW_EMI_FREQ)
    {
        for(i = 0; i < DRAMCTRLREGNUM; i++)
        {
            if(pDDRTiming->DDRTimingoptimized_Low[i]!= 0)
               ((PDWORD)pDDRTiming->pv_HwRegDRAM)[i] = pDDRTiming->DDRTimingoptimized_Low[i];
         }
    }
    else
    {            
        for(i = 0; i < DRAMCTRLREGNUM; i++)
        {
           if(pDDRTiming->DDRTimingoptimized_High[i]!= 0)
                ((PDWORD)pDDRTiming->pv_HwRegDRAM)[i] = pDDRTiming->DDRTimingoptimized_High[i];
        }       
    }

    //Wait for REF_EMI to not be busy        
    while(HW_CLKCTRL_EMI_RD() & BM_CLKCTRL_EMI_BUSY_REF_EMI);
    
    //9. Restart memory controller
    HW_DRAM_CTL16_SET(BM_DRAM_CTL16_START);

    //10 Wait DLL is locked
    while(! (HW_DRAM_CTL21_RD() & BM_DRAM_CTL21_DLLLOCKREG) );

    //11. Exit Memory self-refresh
    HW_DRAM_CTL17_CLR(BM_DRAM_CTL17_SREFRESH); 

   //Wait Memory device exit into self-refresh
   while(HW_DRAM_CTL172_RD() & BM_DRAM_CTL172_CKE_STATUS);

   Reg_Data=HW_DIGCTL_MICROSECONDS_RD();
   for(; (HW_DIGCTL_MICROSECONDS_RD()-Reg_Data) <= 100;); 
    //pDEBUGChange('Z');
    
}

//-----------------------------------------------------------------------------
//
//  Function:  DvfcUpdateEMIClock
//
//  This function updates the current EMI clock.
//
//  Parameters:
//      NewClock
//          [in] new frequency.
//
//      EMIFracDiv
//          [in] New EMI clock fraction divider
//
//      EMIDiv
//          [in] New EMI clock divider.
//
//  Returns:
//      Returns TRUE if the setpoint update was successful, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DvfcUpdateEMIClock(UINT32 NewClock, UINT32 EMIFracDiv, UINT32 EMIDiv)
{    
#ifdef  DVFC_DDR2_FREQ_CHANGE
    UINT32 APBHCTRL_Backup, APBXCTRL_Backup, i;
    volatile UINT32 StateX, StateH;

    //Disable   ARM interrupt  
    DisableIrqInterrupt();

    //freeze APBH, APBX channel 
    APBHCTRL_Backup = HW_APBH_CHANNEL_CTRL_RD();
    HW_APBH_CHANNEL_CTRL_SET(BF_APBH_CHANNEL_CTRL_FREEZE_CHANNEL(0xffff));


    APBXCTRL_Backup = HW_APBX_CHANNEL_CTRL_RD();
    HW_APBX_CHANNEL_CTRL_SET(BF_APBX_CHANNEL_CTRL_FREEZE_CHANNEL(0xffff));

    //Wating all channel data transfers, PIO words and the DMA descriptor fetching stop
    for(i = 0; i < 16; i++)
    {
        StateX = HW_APBX_CHn_DEBUG1_RD(i) & BM_APBX_CHn_DEBUG1_STATEMACHINE;
        while( (StateX != BV_APBX_CHn_DEBUG1_STATEMACHINE__IDLE) &&
        (StateX != BV_APBX_CHn_DEBUG1_STATEMACHINE__WRITE) &&
        (StateX != BV_APBX_CHn_DEBUG1_STATEMACHINE__READ_REQ) &&
        (StateX != BV_APBX_CHn_DEBUG1_STATEMACHINE__CHECK_WAIT))
        {
            StateX = HW_APBX_CHn_DEBUG1_RD(i) & BM_APBX_CHn_DEBUG1_STATEMACHINE;  
        }
    
        StateH = HW_APBH_CHn_DEBUG1_RD(i) & BM_APBH_CHn_DEBUG1_STATEMACHINE;
        while( (StateH != BV_APBH_CHn_DEBUG1_STATEMACHINE__IDLE) &&
        (StateH != BV_APBH_CHn_DEBUG1_STATEMACHINE__WRITE) &&
        (StateH != BV_APBH_CHn_DEBUG1_STATEMACHINE__READ_REQ) &&
        (StateH != BV_APBH_CHn_DEBUG1_STATEMACHINE__CHECK_WAIT))
        {
            StateH = HW_APBH_CHn_DEBUG1_RD(i) & BM_APBH_CHn_DEBUG1_STATEMACHINE;  
        }    
    }

    //ETHNET uDAM0, uDMA1

    //DCP, BCH, 
    //DCP_Backup = HW_DCP_CTRL_RD();
    //BCH_Backup = HW_BCH_CTRL_RD();    
    //HW_DCP_CTRL_SET(BM_DCP_CTRL_CLKGATE);
    //HW_BCH_CTRL_SET(BM_BCH_CTRL_CLKGATE);

    pEMIClockChangeFunc(NewClock, EMIFracDiv, EMIDiv);

    // Restore DCP and BCH
    //HW_DCP_CTRL_CLR(~DCP_Backup & BM_DCP_CTRL_CLKGATE);
    //HW_BCH_CTRL_CLR(~BCH_Backup & BM_BCH_CTRL_CLKGATE);

    //unfreeze APBH, APBX channel 
    HW_APBH_CHANNEL_CTRL_CLR(BF_APBH_CHANNEL_CTRL_FREEZE_CHANNEL(~APBHCTRL_Backup));
    HW_APBX_CHANNEL_CTRL_CLR(BF_APBX_CHANNEL_CTRL_FREEZE_CHANNEL(~APBXCTRL_Backup));

    // Wait till unFreeze happen
    while(HW_APBH_CHANNEL_CTRL_RD() != APBHCTRL_Backup); 
    while(HW_APBX_CHANNEL_CTRL_RD() != APBXCTRL_Backup);       

    EnableIrqInterrupt();    
    //RETAILMSG(TRUE, (_T("DDR2ChangeCount = %d\r\n"), DDR2ChangeCount++));

#else

    UNREFERENCED_PARAMETER(NewClock);
    UNREFERENCED_PARAMETER(EMIFracDiv);
    UNREFERENCED_PARAMETER(EMIDiv); 

#endif

   return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  DvfcUpdateSetpoint
//
//  This function updates the current frequency/voltage setpoint for 
//  the system.
//
//  Parameters:
//      pNewCfg
//          [in] Points to new frequency/voltage setpoint.
//
//      domain
//          [in] Specifies DVFC domain.
//
//      usDelay
//          [in] Specifies a delay in usec to wait for the voltage ramp
//               to occur for the new setpoint.
//
//  Returns:
//      Returns TRUE if the setpoint update was successful, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DvfcUpdateSetpoint(PDVFC_SETPOINT_CONFIG pNewCfg, 
                         PDVFC_SETPOINT_CONFIG pOldCfg,
                         DDK_DVFC_DOMAIN domain)
{
    PDDK_CLK_CONFIG pDdkClkConfig = DDKClockGetSharedConfig();  
    BOOL bPerformHclkDivFirst = FALSE; 
    BOOL bPerformCPUDivFirst = FALSE;
    BOOL bPerformEMIDiv = FALSE;
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(domain);

    //Change CLK_H, CLK_P and CLK_EMI clock
    if(pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_CPU] > pOldCfg->setPointInfo.freq[DDK_DVFC_FREQ_CPU])
        bPerformHclkDivFirst = TRUE; 
    
    if(pNewCfg->parm[DVFC_PARM_ARM_CLK_DIV] > pOldCfg->parm[DVFC_PARM_ARM_CLK_DIV])
        bPerformCPUDivFirst = TRUE;    

    if(pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_EMI] != pOldCfg->setPointInfo.freq[DDK_DVFC_FREQ_EMI])
        bPerformEMIDiv = TRUE;

    if(bPerformHclkDivFirst)
    {
        // config CLK_HBUS
        HW_CLKCTRL_HBUS_WR((BF_CLKCTRL_HBUS_DIV(pNewCfg->parm[DVFC_PARM_AHB_CLK_DIV])    |
                            BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)                               | 
                            BF_CLKCTRL_HBUS_SLOW_DIV(0)                                  | 
                            BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(0)                       | 
                            BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(0)                        | 
                            BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(0)                     | 
                            BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_PXP_AS_ENABLE(0)                             |
                            BF_CLKCTRL_HBUS_DCP_AS_ENABLE(0)                             |
                            BF_CLKCTRL_HBUS_ASM_ENABLE(0)                                |
                            BF_CLKCTRL_HBUS_ASM_EMIPORT_AS_ENABLE(0)));
    }

    if(bPerformCPUDivFirst)
    {
        // config CLK_CPU
        HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(pNewCfg->parm[DVFC_PARM_ARM_CLK_DIV]) |
                           BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)                             |
                           BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)                              |
                           BF_CLKCTRL_CPU_DIV_XTAL(1)                                    |
                           BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));
    
        // set ref.cpu 
        HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATECPU);
        HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_CPUFRAC) |
                           BF_CLKCTRL_FRAC0_CPUFRAC(pNewCfg->parm[DVFC_PARM_ARM_FRAC_CPUFRAC]));
    }
    else
    {
        // set ref.cpu 
        HW_CLKCTRL_FRAC0_CLR(BM_CLKCTRL_FRAC0_CLKGATECPU);
        HW_CLKCTRL_FRAC0_WR((HW_CLKCTRL_FRAC0_RD() & ~BM_CLKCTRL_FRAC0_CPUFRAC) |
                           BF_CLKCTRL_FRAC0_CPUFRAC(pNewCfg->parm[DVFC_PARM_ARM_FRAC_CPUFRAC]));
      
        // config CLK_CPU
        HW_CLKCTRL_CPU_WR((BF_CLKCTRL_CPU_DIV_CPU(pNewCfg->parm[DVFC_PARM_ARM_CLK_DIV])   |
                           BF_CLKCTRL_CPU_DIV_CPU_FRAC_EN(0)                              |
                           BF_CLKCTRL_CPU_INTERRUPT_WAIT(1)                               |
                           BF_CLKCTRL_CPU_DIV_XTAL(1)                                     |
                           BF_CLKCTRL_CPU_DIV_XTAL_FRAC_EN(0)));          
    }

    //--------------------------------------------------------------------------
    // Change HCLK after PCLK.
    //--------------------------------------------------------------------------
    if(!bPerformHclkDivFirst)
    {
        // config CLK_HBUS
        HW_CLKCTRL_HBUS_WR((BF_CLKCTRL_HBUS_DIV(pNewCfg->parm[DVFC_PARM_AHB_CLK_DIV])    |
                            BF_CLKCTRL_HBUS_DIV_FRAC_EN(0)                                 | 
                            BF_CLKCTRL_HBUS_SLOW_DIV(0)                                  | 
                            BF_CLKCTRL_HBUS_CPU_INSTR_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_CPU_DATA_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_TRAFFIC_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_TRAFFIC_JAM_AS_ENABLE(0)                     | 
                            BF_CLKCTRL_HBUS_APBXDMA_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_APBHDMA_AS_ENABLE(0)                         | 
                            BF_CLKCTRL_HBUS_PXP_AS_ENABLE(0)                             |
                            BF_CLKCTRL_HBUS_DCP_AS_ENABLE(0)                             |
                            BF_CLKCTRL_HBUS_ASM_ENABLE(0)                                 |
                            BF_CLKCTRL_HBUS_ASM_EMIPORT_AS_ENABLE(0)));

    }

    if(bPerformEMIDiv)
    {
        //RETAILMSG(DVFC_VERBOSE, (TEXT("NDDRCLK %d \r\n"),pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_EMI]));
        DvfcUpdateEMIClock(pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_EMI],
                           pNewCfg->parm[DVFC_PARM_EMI_FRAC_EMIFRAC], 
                           pNewCfg->parm[DVFC_PARM_EMI_CLK_DIV]);
    }

    //Update CPU AHB clock
    pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_P_CLK] =
                             pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_CPU];
    pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_H_CLK] = 
                             pNewCfg->setPointInfo.freq[DDK_DVFC_FREQ_AHB];;


#if (DVFC_VERBOSE == TRUE)
    DumpSetpoint(pNewCfg);
#endif

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  DvfcWorkerThread
//
//  This is the worker thread to respond to DVFC setpoint change requests.
//
//  Parameters:
//      lpParam
//          [in] Thread data passed to the function using the 
//          lpParameter parameter of the CreateThread function. Not used.
//
//  Returns:
//      Returns thread exit code.
//
//-----------------------------------------------------------------------------
static DWORD WINAPI DvfcWorkerThread (LPVOID lpParam)
{
    DWORD rc = TRUE;
    DDK_DVFC_SETPOINT setpointCur;
    DDK_DVFC_SETPOINT setpointReq;
    DDK_DVFC_SETPOINT setpointLoad;
    DDK_DVFC_SETPOINT setpointMin;
    DDK_DVFC_SETPOINT setpointMax;
    DDK_DVFC_DOMAIN domain;
    BOOL bSignalDdkClk, bLowerVoltage, bUnlock;
    UINT32 mVoltCur, mVoltReq, mVoltReqBO ;
    PDDK_CLK_CONFIG pDdkClkConfig = DDKClockGetSharedConfig();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpParam);
    
    CeSetThreadPriority(GetCurrentThread(), g_IstPriority);

    for (;;)
    {
        if(WaitForSingleObject(g_hDvfcWorkerEvent, INFINITE) == WAIT_OBJECT_0)
        {      
            for (domain = 0; domain < DDK_DVFC_DOMAIN_ENUM_END; domain++)
            {            
                // Reset state variables
                bSignalDdkClk = FALSE;
                bLowerVoltage = FALSE;
                bUnlock = FALSE;
                setpointCur = pDdkClkConfig->setpointCur[domain];
                setpointMin = pDdkClkConfig->setpointMin[domain];
                setpointMax = pDdkClkConfig->setpointMax[domain];
                setpointLoad = pDdkClkConfig->setpointLoad[domain];
                pDdkClkConfig->bSetpointPending = TRUE;

                // Evaluate the required setpoint by inspecting the globally shared 
                // reference count for each setpoint
                for (setpointReq = setpointMax; setpointReq < setpointMin; setpointReq++)
                {
                    if (pDdkClkConfig->setpointReqCount[domain][setpointReq] != 0)
                    {
                        break;
                    }
                }

                // If load tracking request results in higher setpoint,
                // override the request from CSPDDK
                if (setpointLoad < setpointReq)
                {
                    setpointReq = setpointLoad;
                }
             
                // Check if we are already at the required setpoint
                if (setpointCur == setpointReq)
                {
                    bSignalDdkClk = TRUE;
                    goto cleanUp;
                }
              
                // If new setpoint requires higher voltage, submit request to PMIC.  Note
                // that we do this before grabbing the CSPDDK lock since the PMIC may need
                // to enable clocks using the CSPDDK.
                mVoltCur = g_SetPointConfig[domain][setpointCur].setPointInfo.mV;
                mVoltReq = g_SetPointConfig[domain][setpointReq].setPointInfo.mV;
                mVoltReqBO = g_SetPointConfig[domain][setpointReq].setPointInfo.mV_BO;
        
                if (mVoltReq > mVoltCur)
                {
                    DVFCSetPowerFetsStrength(&g_SetPointConfig[domain][setpointReq]);
                    DvfcUpdateSupplyVoltage(mVoltReq, mVoltReqBO, domain);
                }
                // Else new setpoint needs lower voltage
                else if (mVoltReq < mVoltCur)
                {
                    // Set flag to lower voltage when we are done
                    bLowerVoltage = TRUE;
                }

                // Grab CSPDDK lock while updating the setpoint
                CSPDDK_LOCK();
                bUnlock = TRUE;
                if (!DvfcUpdateSetpoint(&g_SetPointConfig[domain][setpointReq],
                                        &g_SetPointConfig[domain][setpointCur],
                                        domain))
                {
                    // We encountered an error, set flag to lower the voltage
                    bLowerVoltage = TRUE;
                    goto cleanUp;
                }
                setpointCur = setpointReq;
                bSignalDdkClk = TRUE;
cleanUp:

                // Update current setpoint to reflect successful transitions
                pDdkClkConfig->setpointCur[domain] = setpointCur;
                pDdkClkConfig->bSetpointPending = FALSE;

                // If we acquired CSPDDK lock, unlock it now
                if (bUnlock)
                {
                    CSPDDK_UNLOCK();
                }

                // If flag was set to lower the voltage
                if (bLowerVoltage)
                {
                    DvfcUpdateSupplyVoltage(g_SetPointConfig[domain][setpointCur].setPointInfo.mV, 
                                            g_SetPointConfig[domain][setpointCur].setPointInfo.mV_BO, 
                                            domain);
                    DVFCSetPowerFetsStrength(&g_SetPointConfig[domain][setpointCur]);                    
                }            
                // Signal drivers blocked on CSPDDK that are waiting for setpoint transition
                if (bSignalDdkClk)
                {
                    SetEvent(g_hSetPointEvent);
                }
            }
        }
        else 
        {
            // Abnormal signal
            rc = FALSE;
            break;
        }
    }

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcIntrServ
//
//  This function is invoked from the DVFC interrupt service thread to
//  perform the frequency/voltage switch.    
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void BSPDvfcIntrServ(void)
{
    // Signal the DVFC driver about our request
    //RETAILMSG(TRUE, (_T("BSPDvfcIntrServ!! ++\r\n")));
    SetEvent(g_hDvfcWorkerEvent);

    // SoftIRQ Interrupt soruce clear in OEMInterruptDisable
}


//-----------------------------------------------------------------------------
//
//  Function: BSPDvfcInit
//
//  This function provides platform-specific initialization for supporting 
//  DVFS/DPTC.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPDvfcInit(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;
    PDDK_CLK_CONFIG pDdkClkConfig = DDKClockGetSharedConfig();    
    DDK_DVFC_DOMAIN domain;

    // Map Power
    phyAddr.QuadPart = CSP_BASE_REG_PA_POWER;
    pv_HWregPOWER= (PVOID) MmMapIoSpace(phyAddr, 0x400,FALSE);
    
    // Check if virtual mapping failed
    if (pv_HWregPOWER == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    // Map CLKCTRL
    phyAddr.QuadPart = CSP_BASE_REG_PA_CLKCTRL;
    pv_HWregCLKCTRL = (PVOID) MmMapIoSpace(phyAddr, 0x400,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregCLKCTRL == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    // Map DRAM
    phyAddr.QuadPart = CSP_BASE_REG_PA_DRAM;
    pv_HWregDRAM= (PVOID) MmMapIoSpace(phyAddr, 0x400,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregDRAM == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    // Map DIGCTL
    phyAddr.QuadPart = CSP_BASE_REG_PA_DIGCTL;
    pv_HWregDIGCTL= (PVOID) MmMapIoSpace(phyAddr, 0x500,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregDIGCTL == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    // Map BCH
    phyAddr.QuadPart = CSP_BASE_REG_PA_BCH;
    pv_HWregBCH= (PVOID) MmMapIoSpace(phyAddr, 0x500,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregBCH == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    // Map DCP
    phyAddr.QuadPart = CSP_BASE_REG_PA_DCP;
    pv_HWregDCP= (PVOID) MmMapIoSpace(phyAddr, 0x500,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregDCP == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }    

    // Map LCDIF
    phyAddr.QuadPart = CSP_BASE_REG_PA_LCDIF;
    pv_HWregLCDIF= (PVOID) MmMapIoSpace(phyAddr, 0x500,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregLCDIF == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }    

    // Map APBH
    phyAddr.QuadPart = CSP_BASE_REG_PA_APBH;
    pv_HWregAPBH= (PVOID) MmMapIoSpace(phyAddr, 0x800,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregAPBH == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    // Map APBX
    phyAddr.QuadPart = CSP_BASE_REG_PA_APBX;
    pv_HWregAPBX= (PVOID) MmMapIoSpace(phyAddr, 0x800,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregAPBX == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }
    
    // Map DRAM clock change Data 
    phyAddr.QuadPart = IMAGE_WINCE_DVFC_IRAM_PA_START;    
    pDDRTiming = (PDVFC_TIMING_OPTIMIZED) MmMapIoSpace(phyAddr, IMAGE_WINCE_DVFC_IRAM_DATA_SIZE,FALSE);
    
    if (pDDRTiming == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    DVFCDDR2SetpointConfig();

    // Map EMI clock change function 
    phyAddr.QuadPart = IMAGE_WINCE_DVFC_IRAM_PA_START + IMAGE_WINCE_DVFC_IRAM_DATA_SIZE;    
    pEMIClockChangeFunc = (FUNC_EMI_CONTROL) MmMapIoSpace(phyAddr, IMAGE_WINCE_DVFC_IRAM_FUNC_SIZE,FALSE);
    
    if (pEMIClockChangeFunc == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }

    //Copy EMIClockChange function to IRAM
    memcpy((void *) pEMIClockChangeFunc, (void *) EMIClockChange, IMAGE_WINCE_DVFC_IRAM_FUNC_SIZE);
     
    // Map UART
    phyAddr.QuadPart = CSP_BASE_REG_PA_UARTDBG;
    pv_HWregUARTDbg= (PVOID) MmMapIoSpace(phyAddr, 0x400,FALSE);

    // Check if virtual mapping failed
    if (pv_HWregUARTDbg == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }
/*
    // Map DEBUG change function 
    phyAddr.QuadPart = IMAGE_WINCE_DEBUG_IRAM_PA_START;    
    pDEBUGChange = (FUNC_DEBUG_CONTROL) MmMapIoSpace(phyAddr, IMAGE_WINCE_DEBUG_IRAM_SIZE,FALSE);
    
    if (pDEBUGChange == NULL)
    {
        ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
        goto cleanUp;
    }
    //Copy DEBUG function to IRAM
    memcpy((void *) pDEBUGChange, (void *) WriteDebugByte, IMAGE_WINCE_DEBUG_IRAM_SIZE);  
*/         
    // Create event to sync with DVFC setpoint transitions
    g_hSetPointEvent = CreateEvent(NULL, TRUE, FALSE, L"EVENT_SETPOINT");

    if (g_hSetPointEvent == NULL)
    {
        ERRORMSG(TRUE, (_T("CreateEvent failed!\r\n")));
        goto cleanUp;
    }

    // Supply default DVFC configuration
    g_IstPriority = DVFC_IST_PRIORITY;

    // Create event to signal DVFC worker thread
    g_hDvfcWorkerEvent = CreateEvent(NULL, FALSE, FALSE, L"EVENT_DVFC_WORKER");

    if (g_hDvfcWorkerEvent == NULL)
    {
        ERRORMSG(TRUE, (_T("CreateEvent failed!\r\n")));
        goto cleanUp;
    }

    // Create worker thread for DVFC setpoint requests
    g_hDvfcWorkerThread = CreateThread(NULL, 0, DvfcWorkerThread, NULL, 0, NULL);      
    if (!g_hDvfcWorkerThread) 
    {
        ERRORMSG(TRUE, (_T("CreateThread failed for DVFC worker thread!\r\n")));
        goto cleanUp;
    }

    for (domain = 0; domain < DDK_DVFC_DOMAIN_ENUM_END; domain++)
    {
        g_dxCurrent[domain] = PwrDeviceUnspecified;
    
        // Set DVFC device power state to D0
        if (!BSPDvfcPowerSet(domain+1, D0))
        {
            ERRORMSG(TRUE, (_T("BSPDvfcPowerSet failed!\r\n")));
            goto cleanUp;
        }

        g_dxCurrent[domain] = D0;
    }

    rc = TRUE;

cleanUp:

    // If initialization succeeded, report that the DVFC is active
    if (pDdkClkConfig)
    {
        pDdkClkConfig->bDvfcActive = rc;
    }
   
    return rc;
}

        
//-----------------------------------------------------------------------------
//
//  Function:  BSPDvfcDeinit
//
//  This function deinitializes the platform-specific DVFS/DPTC support 
//  established by BSPDvfcInit.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPDvfcDeinit(void)
{
    return TRUE;    
}


