//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <fmd.h>
#include "common_nandfmd.h"
#include "cspnand.h"
#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions
extern "C" void RdPageAlign1(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void RdPageAlign2(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void RdPageAlign3(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void RdPageAlign4(LPBYTE pSectorBuff, USHORT SectorSize);
extern "C" void WrPageAlign1(LPBYTE pSectorBuff, USHORT SectorSize); 
extern "C" void WrPageAlign2(LPBYTE pSectorBuff, USHORT SectorSize); 
extern "C" void WrPageAlign3(LPBYTE pSectorBuff, USHORT SectorSize); 
extern "C" void WrPageAlign4(LPBYTE pSectorBuff, USHORT SectorSize); 

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Defines
#define NFC_BBI_MAIN_SEGMENT    ((g_CSPfi.fi.wDataBytesPerSector/NFC_MAIN_BUFFER_SIZE)-1)//The location used for BBI fix
#define NFC_SPARE_SEGMENT_LOW   ((g_CSPfi.fi.wDataBytesPerSector/NFC_MAIN_BUFFER_SIZE)-2)//The location used for BBI fix
#define NFC_SPARE_SEGMENT_HIGH  ((g_CSPfi.fi.wDataBytesPerSector/NFC_MAIN_BUFFER_SIZE)-1)//The location used for BBI fix
#define NOT_A_POWER_OF_2 0xff

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
extern "C" PCSP_NFC_AXI_REGS g_pNfcAxi = NULL;
extern "C" PCSP_NFC_IP_REGS  g_pNfcIp = NULL;
FlashInfoExt g_CSPfi;
DWORD   dwInterleaveMode = INTERLEAVE_MODE_BIG_SECTOR;
//-----------------------------------------------------------------------------
// Local Variables
static DWORD gLog2Num = 0;
static BOOL gbECCMode;
static DWORD gUncorNOBER;

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
static UCHAR ComputeLog2(DWORD dwNum)
{
    UCHAR log2 = 0;

    //----- 1. SPECIAL CASE: 2 raised to any exponent never equals 0 ----
    if(dwNum == 0)
    {
        return NOT_A_POWER_OF_2;
    }

    //----- 2. Keep dividing by 2 until the LSB is 1 -----
    while(!(dwNum & 0x000000001))
    {
        dwNum >>= 1;
        log2++;
    }

    //----- 3. If (dwNum>>1) != 0, dwNum wasn't a power of 2 -----
    if(dwNum>>1)
    {
        return NOT_A_POWER_OF_2;
    }

    return log2;
}

//-----------------------------------------------------------------------------
//
//  Function: NFCSwapBBI
//
//  This function implements the BBI(Bad Block Information) swap workaround for
//  the NFC design flaw.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void NFCSwapBBI()
{
    UINT32 MainData,TempMainData;
    UINT32 SpareData,TempSpareData;

    UINT32 ShiftValue,MainDataMask,BBI;

    MainData = INREG32(&g_pNfcAxi->MAIN[NFC_BBI_MAIN_SEGMENT][g_CSPfi.BBIMainAddr/sizeof(UINT32)]);
    SpareData = INREG32(&g_pNfcAxi->SPARE[NFC_SPARE_SEGMENT_HIGH][0]);

    ShiftValue = (g_CSPfi.BBIMainAddr%(sizeof(UINT32)))*8;
    MainDataMask = ~(0xFF<<ShiftValue);
    BBI = (SpareData >>8) & 0xFF;
    TempMainData = (MainData & MainDataMask) | (BBI<<ShiftValue);
    TempSpareData = (SpareData & 0xFFFF00FF) | (((MainData & ~MainDataMask)>>ShiftValue)<<8);

    OUTREG32(&g_pNfcAxi->MAIN[NFC_BBI_MAIN_SEGMENT][g_CSPfi.BBIMainAddr/sizeof(UINT32)], TempMainData);
    OUTREG32(&g_pNfcAxi->SPARE[NFC_SPARE_SEGMENT_HIGH][0], TempSpareData);
}

//-----------------------------------------------------------------------------
//
//  Function: NFCReadMain
//
//  This function reads the NAND flash controller main area and provides this
//  sector data to the upper FMD layers.
//
//  Parameters:
//      pSectorBuff 
//          [out] Buffer containing sector data read.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCReadMain(LPBYTE pSectorBuff)
{        
    if((UINT32)pSectorBuff & 0x1)        
    {
        // Byte-aligned on BYTE[3]
        if((UINT32)pSectorBuff & 0x2)        
        {
            RdPageAlign3(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }

        // Byte-aligned on BYTE[1]
        else
        {
            RdPageAlign1(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }
    }
    else if ((UINT32)pSectorBuff & 0x2)
    {
        // Half-word aligned on BYTE[2]
        RdPageAlign2(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    } 
    else
    {
        // Word-aligned
        RdPageAlign4(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    }
}

//-----------------------------------------------------------------------------
//
//  Function: NFCWriteMain
//
//  This function writes the sector data provided by the upper FMD layers
//  to the NAND flash controller main area.
//
//  Parameters:
//      pSectorBuff 
//          [in] Buffer containing sector data to be written.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCWriteMain(LPBYTE pSectorBuff)
{
    if((UINT32)pSectorBuff & 0x1)        
    {
        // Byte-aligned on BYTE[3]
        if((UINT32)pSectorBuff & 0x2)        
        {
            WrPageAlign3(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }

        // Byte-aligned on BYTE[1]
        else
        {
            WrPageAlign1(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
        }
    }
    else if ((UINT32)pSectorBuff & 0x2)
    {
        // Half-word aligned on BYTE[2]
        WrPageAlign2(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    } 
    else
    {
        // Word-aligned
        WrPageAlign4(pSectorBuff, (USHORT)g_CSPfi.fi.wDataBytesPerSector);
    }   
}

//-----------------------------------------------------------------------------
//
//  Function: NFCReadSpare
//
//  This function reads the NAND flash controller spare area and provides this
//  information to the upper FMD layers.
//
//  Parameters:
//      pSectorInfoBuff 
//          [out] Buffer containing sector information read.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCReadSpare(PSectorInfo pSectorInfoBuff)
{
    pSectorInfoBuff->dwReserved1 = INREG32(&g_pNfcAxi->SPARE[NFC_SPARE_SEGMENT_LOW][0]);
    *((UINT32 *) (&pSectorInfoBuff->bOEMReserved)) = INREG32(&g_pNfcAxi->SPARE[NFC_SPARE_SEGMENT_HIGH][0]);
}

//-----------------------------------------------------------------------------
//
//  Function: NFCClearSpare
//
//  This function clears the NAND flash controller spare area (all
//  ones written to the spare area will prevent the spare area of
//  the NAND memory device from being updated since only zeros are
//  written during a NAND program cycle).
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCClearSpare(VOID)
{
    UINT32 *dest;
    int i,j;

    for(i = 0; i < g_CSPfi.fi.wDataBytesPerSector / NFC_MAIN_BUFFER_SIZE; i++){
        dest = (UINT32 *) &g_pNfcAxi->SPARE[i];
        for(j=0;j<16;j++)
            OUTREG32(&dest[j], 0xFFFFFFFF);
    }
}

//-----------------------------------------------------------------------------
//
//  Function: NFCWriteSpare
//
//  This function writes the spare data provided by the upper MDD layers
//  to the NAND flash controller spare area.
//
//  Parameters:
//      pSectorInfoBuff 
//          [in] Buffer containing sector information to be written.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static VOID NFCWriteSpare(PSectorInfo pSectorInfoBuff)
{   
    NFCClearSpare();
    
    OUTREG32(&g_pNfcAxi->SPARE[NFC_SPARE_SEGMENT_LOW][0], pSectorInfoBuff->dwReserved1);
    OUTREG32(&g_pNfcAxi->SPARE[NFC_SPARE_SEGMENT_HIGH][0], *((UINT32 *) (&pSectorInfoBuff->bOEMReserved)));
}

//-----------------------------------------------------------------------------
//
//  Function: SetCs
//
//  This function sets NFC to certain cs
//
//  Parameters:
//      dwCs:   which cs is needed to set.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
static VOID SetCs(DWORD dwCs)
{
    static DWORD m_cs = 0;
    
    if(m_cs != dwCs && dwCs < 8)
    {
        INSREG32BF(&g_pNfcAxi->NFC_CONFIGURATION1,
            NFC_NFC_CONFIGURATION1_CS, (NFC_NFC_CONFIGURATION1_CS_ACTIVE_CS0 + dwCs));
        m_cs = dwCs;
    }
}

//-----------------------------------------------------------------------------
//
//  Function:  NFCWait
//
//  This functions waits for the pending NANDFC opeation to complete.  
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID NFCWait()
{
    DWORD LoopCount = 0xFFFFFF;
    
    // If too many loops are done, we should quit to avoid endless loop.
    while(LoopCount)
    {
        LoopCount--;
        if((INREG32(&g_pNfcIp->NFC_IPC) & CSP_BITFMASK(NFC_NFC_IPC_INT)))
            break;
    }
    // Clear the NANDFC interrupt
    CLRREG32(&g_pNfcIp->NFC_IPC, CSP_BITFMASK(NFC_NFC_IPC_INT));
}

//-----------------------------------------------------------------------------
//
//  Function: NFCCheckStatus
//
//  This function check status of cs
//
//  Parameters:
//      StartCS 
//          [in] First cs that needs to check
//      NumofCS 
//          [in] Total number of CS needs to check
//  Returns:
//      BOOL: True if the NAND status is pass; 
//            False if the NAND status is fail
//
//-----------------------------------------------------------------------------
static BOOL NFCCheckStatus(UINT32 StartCS, UINT32 NumofCS)
{
    UINT32 cs;
    UINT32 Status = 0;
    
    for (cs = 0; cs < NumofCS; cs++)
    {
        SetCs(cs + StartCS);

        NF_BUF_ADDR(0);
        NF_CMD_ATOMIC(g_CSPfi.CmdReadStatus)          // Send status command
        NF_RD_STATUS();             // Read status into NFC buffer
        Status = EXTREG32BF(&g_pNfcAxi->NFC_CONFIGURATION1, NFC_NFC_CONFIGURATION1_NF_STATUS);
    
        if (Status & (1 << g_CSPfi.StatusErrorBit))
        {
            return FALSE;
        }    
    }
        
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: NFCMapRegister
//
//  This function maps the NFC registers and internal RAM from physical 
//  address to virtual address.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL NFCMapRegister(VOID)
{
    // Map NFC AXI registers and buffer address to virtual address
    g_pNfcAxi = (PCSP_NFC_AXI_REGS) BSPNAND_RemapRegister(CSP_BASE_REG_PA_NFC_AXI, sizeof(CSP_NFC_AXI_REGS));

    // Check if virtual mapping failed
    if (g_pNfcAxi == NULL)
    {
        DEBUGMSG(1, (_T("NFCMapRegister:  BSPNAND_RemapRegister for NFC AXI registers failed!\r\n")));
        return FALSE;
    }
 
    // Map NFC IP registers to virtual address
    g_pNfcIp = (PCSP_NFC_IP_REGS) BSPNAND_RemapRegister(CSP_BASE_REG_PA_NFC_IP,  sizeof(CSP_NFC_IP_REGS));
    
    // Check if virtual mapping failed
    if (g_pNfcIp == NULL)
    {
        DEBUGMSG(1, (_T("NFCMapRegister:  BSPNAND_RemapRegister for NFC IP registers failed!\r\n")));
        return FALSE;
    }
    
    return TRUE;   
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_Reset
//
//  This function reset NAND flash.
//
//  Parameters:
//      None
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID CSPNAND_Reset(DWORD dwCs)
{
    SetCs(dwCs);
    NF_CMD_ATOMIC(g_CSPfi.CmdReset);     // Send flash reset command
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_ReadID
//
//  This function read NAND flash ID.
//
//  Parameters:
//      None
//
//  Returns:
//      UINT32: NAND Device ID.
//
//-----------------------------------------------------------------------------
UINT32 CSPNAND_ReadID(DWORD dwCs)
{
    UINT32 NandDeviceID;
    
    SetCs(dwCs);
    // Get manufacturer and device codes.
    NF_BUF_ADDR(0);
    NF_CMD_ATOMIC(g_CSPfi.CmdReadId);     // Read ID command
    NF_ADDR_ATOMIC(0);      // Required address cycle
    NF_RD_ID();             // Read ID into NFC buffer

    NandDeviceID = INREG32(&g_pNfcAxi->MAIN[0]);
    
    return(NandDeviceID);
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_EnableECC
//
//  This function enable/disable ECC
//
//  Parameters:
//      Enable:   enable/disable
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID CSPNAND_EnableECC(BOOL Enable)
{
    NF_IPC_ENABLE()
    if(Enable)
    {
        OUTREG32(&g_pNfcIp->NFC_CONFIGURATION2,
            INREG32(&g_pNfcIp->NFC_CONFIGURATION2) | 0x8);
    }
    else
    {
        OUTREG32(&g_pNfcIp->NFC_CONFIGURATION2,
            INREG32(&g_pNfcIp->NFC_CONFIGURATION2) & (~0x8));
    }
    NF_IPC_DISABLE()       
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_ILSupport
//
//  This function returns whether NFC supports interleave mode
//
//  Parameters:
//      None
//
//  Returns:
//      TRUE indicates support, FALSE indicates not support
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_ILSupport()
{
    //Elvis does support interleave mode
    return TRUE;
}

VOID CSPNAND_PostInit(PFlashInfoExt pFlashInfo)
{
    UNREFERENCED_PARAMETER(pFlashInfo);
}
//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_Init
//
//  This function initializes the flash memory of a device.
//
//  Parameters:
//      pFlashInfo
//          [in] Pointer to the FlashInfoExt used to pass down information. 
//
//  Returns:
//      A handle that can be used in a call to CSPNAND_Deinit. A value of zero (0) 
//      represents failure.
//
//-----------------------------------------------------------------------------
PVOID CSPNAND_Init(PFlashInfoExt pFlashInfo)
{
    PVOID rc = NULL;
    UINT32 i;
    
    if(!pFlashInfo)
    {
        ERRORMSG(TRUE, (_T("FMD_Init: pFlashInfo is NULL!")));
        return FALSE;   
    }
    
    memcpy(&g_CSPfi, pFlashInfo, sizeof(FlashInfoExt));
    
    if(g_CSPfi.NumberOfChip != 1)
    {
        gLog2Num = ComputeLog2(g_CSPfi.NumberOfChip);
        if(gLog2Num == NOT_A_POWER_OF_2)
        {
            DEBUGMSG(TRUE, (_T("FMD_Init: NAND_NUM_OF_DEVICES is NOT_A_POWER_OF_2 in Interleave mode\r\n")));
            return FALSE;
        }
    }

    // Perform NFC mappings
    if (!NFCMapRegister())
    {
        ERRORMSG(TRUE, (_T("FMD_Init: failed memory allocation/mapping")));
        goto cleanUp;
    }
    
    BSPNAND_ConfigIOMUX(g_CSPfi.NumberOfChip);

    if(g_CSPfi.SpareDataLength > 128)
    {
        gbECCMode = NFC_NFC_CONFIGURATION2_ECC_MODE_8BIT;
        gUncorNOBER = NFC_ECC_STATUS_RESULT_NOSER_8SB_ERR;
    }
    else
    {
        gbECCMode = NFC_NFC_CONFIGURATION2_ECC_MODE_4BIT;
        gUncorNOBER = NFC_ECC_STATUS_RESULT_NOSER_4SB_ERR;

    }        

    
    /* NFC configuration 2      
    ; Set NFC_CONFIGURATION2 register
    ;
    ;   PS - 2KB page size(0x01)                       = 0x00000001     
    ;   SYM - asymmetric mode (0 << 2)                 = 0x00000000
    ;   ECC_EN - enable ECC (1 << 3)                   = 0x00000008
    ;   READ_CONFIRM - needed (1 << 4)                 = 0x00000010
    ;   NUM_OF_ADDR_ERASE - (1 << 5)                   = 0x00000020
    ;   ECC_MODE - 4 bits error correction (1 << 6)    = 0x00000040
    ;   PPB - 128 page per block (0x02<<7)             = 0x00000100
    ;   EDC - Extra dead cycle ( 0 << 9)               = 0x00000000   
    ;   NUM_OF_ADDR_PHASE - 5 phases (2 << 12)         = 0x00002000 
    ;   AUTO_PROG_DONE_MASK - disable(0 <<14)          = 0x00000000
    ;   INT_MSK - mask interrupt (0 << 15)             = 0x00000000
    ;   SPAS - spare area size (0x20<<16)              = 0x00200000
    ;   STATUS_CMD - (0x70 << 24)                      = 0x70000000   
    ;                                                  ------------
    ;                                                    0x70202179     
    */    
    NF_IPC_ENABLE()
    OUTREG32(&g_pNfcIp->NFC_CONFIGURATION2,
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_PS, g_CSPfi.fi.wDataBytesPerSector >> 11) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_SYM, NFC_NFC_CONFIGURATION2_SYM_SYMMETRIC) |       
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_ECC_EN, NFC_NFC_CONFIGURATION2_ECC_EN_ENABLE) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_NUM_CMD_PHASES, 1) |    
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES0, (g_CSPfi.ChipAddrCycleNum - g_CSPfi.NumBlockCycles - 1)) | 
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_ECC_MODE, gbECCMode) |            
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_PPB, g_CSPfi.fi.wSectorsPerBlock >> 6) |           
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_EDC, 7) |        
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1, (g_CSPfi.ChipAddrCycleNum - 3)) |   
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_AUTO_PROG_DONE_MSK, NFC_NFC_CONFIGURATION2_AUTO_PROG_DONE_MSK_UNMASK) |                 
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_INT_MSK, NFC_NFC_CONFIGURATION2_INT_MSK_UNMASK) |       
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_SPAS, g_CSPfi.SpareDataLength>>1) |       
        CSP_BITFVAL(NFC_NFC_CONFIGURATION2_ST_CMD, g_CSPfi.CmdReadStatus));
    NF_IPC_DISABLE()
#ifdef EBOOT_DEBUG      
    KITLOutputDebugString("NFC_CONFIGURATION2 = 0x%x\r\n", INREG32(&g_pNfcIp->NFC_CONFIGURATION2));   
#endif    

    /*Set NFC_CONFIGURATION3 register    
    ;   ADD_OP - address mode (0 << 0)                  = 0x00000000
    ;   TOO - one device for CS (0 << 2)                = 0x00000000
    ;   FW - Flash width ( 1 << 3)                      = 0x00000008      
    ;   SB2R - status bit to record (0 << 4)            = 0x00000000
    ;   NF_BIG - little endian ( 0 << 7)                = 0x00000000     
    ;   SBB - status busy bit  ( 6 << 8)                = 0x00000600
    ;   DMA_MODE - two dma signal  ( 0 << 11)           = 0x00000000
    ;   NUM_OF_DEV - number of devices (0 << 12)        = 0x00000000    
    ;   RBB_MODE - check status reg (0 << 15)           = 0x00000000            
    ;   FMP - 64 (1 << 16)                                     = 0x00010000
    ;   NO_SDMA -  (1 << 20)                            = 0x00100000               
    ;                                                   ------------
    ;                                                     0x00110608
    */      
    NF_IPC_ENABLE()
    OUTREG32(&g_pNfcIp->NFC_CONFIGURATION3,
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_ADD_OP, NFC_NFC_CONFIGURATION3_ADD_OP_ADDR_GROUP0_CS) |    //Is it necessary to enable CE extraction?
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_TOO, NFC_NFC_CONFIGURATION3_TOO_ONE_DEVICE) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_FW, NFC_NFC_CONFIGURATION3_FW_8BIT) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_SB2R, g_CSPfi.StatusErrorBit) | //In Samsung NAND flash, bit 0 of the status register is the Pass/Fail indication.
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_NF_BIG, NFC_CONFIGURATION3_NF_BIG_LITTLE) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_SBB, g_CSPfi.StatusBusyBit) | //In Samsung NAND flash, bit 6 of the status register is the Ready/Busy indication.       
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_DMA_MODE, NFC_NFC_CONFIGURATION3_DMA_MODE_2INT) |  //dma_rd_req | dma_wr_req              
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_NUM_OF_DEVICES, (g_CSPfi.NumberOfChip - 1)) |             
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_RBB_MODE, NFC_NFC_CONFIGURATION3_RBB_MODE_SIGNAL) |//signal method can apply to any operation, but status method can only apply to program and erase       
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_FMP, NFC_NFC_CONFIGURATION3_FMP_NONE) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION3_NO_SDMA, NFC_NFC_CONFIGURATION3_NO_SDMA_ENABLE));
    OUTREG32(&g_pNfcIp->NFC_IPC, 0);
   
    NF_IPC_DISABLE() 
        
    NF_IPC_ENABLE()
    OUTREG32(&g_pNfcIp->NFC_DELAY_LINE,0x00);
    NF_IPC_DISABLE()

#ifdef EBOOT_DEBUG      
    KITLOutputDebugString("NFC_CONFIGURATION3 = 0x%x\r\n", INREG32(&g_pNfcIp->NFC_CONFIGURATION3));
#endif    
    // NFC configuration 1
    //  - NAND Flash main and spare data is enabled
    //  - CE signal operates normally
    //  - reset
    //  - Default to NFC buffer #0
    //  - Active CS0
    OUTREG32(&g_pNfcAxi->NFC_CONFIGURATION1,
        CSP_BITFVAL(NFC_NFC_CONFIGURATION1_SP_EN, NFC_NFC_CONFIGURATION1_SP_EN_MAIN_SPARE) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION1_NF_CE, NFC_NFC_CONFIGURATION1_NF_CE_UNFORCE) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION1_NFC_RST, NFC_NFC_CONFIGURATION1_NFC_RST_REST) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION1_RBA, NFC_NFC_CONFIGURATION1_RBA_FIRST_RAM) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION1_NUM_OF_ITE, 0) |
        CSP_BITFVAL(NFC_NFC_CONFIGURATION1_CS, NFC_NFC_CONFIGURATION1_CS_ACTIVE_CS0) );

#ifdef EBOOT_DEBUG
    KITLOutputDebugString("NFC_CONFIGURATION1 = 0x%x\r\n", INREG32(&g_pNfcAxi->NFC_CONFIGURATION1));  
#endif
    
    NF_CMD_ATOMIC(g_CSPfi.CmdReset);     // Send flash reset command
    
    NF_IPC_ENABLE()
    for(i = 0; i < g_CSPfi.NumberOfChip; i++)
    {
        // Unlock all blocks
        INSREG32BF(&g_pNfcIp->UNLOCK_BLK_ADD[i], NFC_UNLOCK_BLK_ADD_USBA, 0);
        INSREG32BF(&g_pNfcIp->UNLOCK_BLK_ADD[i], NFC_UNLOCK_BLK_ADD_UEBA, g_CSPfi.fi.dwNumBlocks - 1);
        
        // Unlock CS0
        /*
            INSREG32BF(&g_pNfcIp->NF_WR_PROT, NFC_NF_WR_PROT_CS2L,
                NFC_NF_WR_PROT_CS2L_CS0LOCK_STATUS+i);
            INSREG32BF(&g_pNfcIp->NF_WR_PROT, NFC_NF_WR_PROT_WPC,
                NFC_NF_WR_PROT_WPC_UNLOCK);  
            INSREG32BF(&g_pNfcIp->NF_WR_PROT, NFC_NF_WR_PROT_BLS,
                NFC_NF_WR_PROT_BLS_UNLOCKED);
        */      

        //Should be 0x00000084 | (0x7 & cs)<<3
        OUTREG32(&g_pNfcIp->NF_WR_PROT,
            CSP_BITFVAL(NFC_NF_WR_PROT_CS2L, (NFC_NF_WR_PROT_CS2L_CS0LOCK_STATUS+i)) |
            CSP_BITFVAL(NFC_NF_WR_PROT_WPC, NFC_NF_WR_PROT_WPC_UNLOCK) |
            CSP_BITFVAL(NFC_NF_WR_PROT_BLS, NFC_NF_WR_PROT_BLS_UNLOCKED));          
    }
    NF_IPC_DISABLE()
    
    rc = g_pNfcAxi;
    
cleanUp:

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_Deinit
//
//  This function de-initializes the flash chip.
//
//  Parameters:
//      hFMD 
//          [in] The handle returned from CSPNAND_Init. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_Deinit(PVOID hFMD)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hFMD);
    return(TRUE);
}

BOOL ReadSectorCsX(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
                    PSectorInfo pSectorInfoBuff)
{
    BOOL rc = FALSE;
    SECTOR_ADDR SectorAddr = startSectorAddr;
    UINT32 ColumnAddr = 0;
    UINT32 PageAddr;    
    UINT32 BufStart = 0;
    UINT32 UsedBuf = g_CSPfi.fi.wDataBytesPerSector / NFC_MAIN_BUFFER_SIZE;
    
    if (!pSectorBuff && !pSectorInfoBuff)
    {
        DEBUGMSG(TRUE, (_T("Sector/Info buffers are NULL!\r\n")));
        goto cleanUp;
    }
    if(pSectorBuff == NULL)
    {
        //FAL only needs to read sector info area
        
        if(UsedBuf > 2)
        {
            ColumnAddr = (UsedBuf - 2) * (g_CSPfi.fi.wDataBytesPerSector + ((g_CSPfi.SpareDataLength == 218)? 208:g_CSPfi.SpareDataLength)) / UsedBuf;
            BufStart = UsedBuf - 2;
        }
    }
    
    {
        PageAddr = (SectorAddr << gLog2Num) | dwCs;
        NF_CMD(g_CSPfi.CmdRead1 | (g_CSPfi.CmdRead2 << 8));
        NF_ADDR(ColumnAddr, PageAddr);
        
        NF_BUF_ADDR(BufStart);
        NF_CLEAR_STATUS();
        NF_AUTO_LAUNCH(NFC_LAUNCH_NFC_AUTO_READ);
        NFCWait();

        UINT32 ECCStat = INREG32(&g_pNfcAxi->ECC_STATUS_RESULT);
        do {
            UINT32 ECCError = ECCStat & NFC_ECC_BIT_UNCORRECTABLE_ERROR;
            //If the number of ECC bit error is equal to 4 (in 4-bit ECC mode) or 8 (in 8-bit ECC mode), it should also be treated as uncorrectable
            //ECC error because NFC may report the number of ECC bit error as 4 (in 4-bit ECC mode) or 8 (in 8-bit ECC mode) 
            //when an actual uncorrectable ECC error occurs.
            if (ECCError >= gUncorNOBER) {
                DEBUGMSG(TRUE, (_T("Uncorrectable ECC error found during read! CS = %d, PageAddr=0x%x, ECCResult=0x%x.\r\n") \
                    ,dwCs, PageAddr, INREG32(&g_pNfcAxi->ECC_STATUS_RESULT)));
                NF_CLEAR_STATUS();
                goto cleanUp;
            }
            ECCStat >>= NFC_ECC_SECTION_BITS;
        } while (--UsedBuf);        
        
        NFCSwapBBI();
        
        // Move page data from NFC buffer to sector buffer
        if (pSectorBuff != NULL)
        {
             NFCReadMain(pSectorBuff);
        }
        
        if (pSectorInfoBuff != NULL)
        {
            NFCReadSpare(pSectorInfoBuff);
        }
    }
    rc = TRUE;
    
cleanUp:
    return rc;    
}
//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_ReadSector
//
//  This function reads the requested sector data and metadata from the 
//  flash media.
//
//  Parameters:
//      
//      startSectorAddr 
//          [in] The starting physical sector address to read.
//
//      pSectorBuff 
//          [out] Pointer to the buffer that contains the sector data read 
//          from flash memory. Set to NULL if this data is not needed.
//
//      pSectorInfoBuff 
//          [out] Buffer for an array of sector information structures. There 
//          is one sector information entry for every sector that is to be read. 
//          Set to NULL if this data is not needed. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_ReadSector(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
    PSectorInfo pSectorInfoBuff)
{
    DWORD cs;
    
    if (!pSectorBuff && !pSectorInfoBuff)
    {
        DEBUGMSG(TRUE, (_T("Sector/Info buffers are NULL!\r\n")));
        return FALSE;
    }
    
    if(dwCs == INTERLEAVE_CS)
    {
        for(cs = 0; cs < g_CSPfi.NumberOfChip; cs++)
        {
            if(!ReadSectorCsX(cs, startSectorAddr, pSectorBuff, pSectorInfoBuff))
            {
                return FALSE;
            }
            if(pSectorBuff)
            {
                pSectorBuff += g_CSPfi.fi.wDataBytesPerSector;
            }
            
            if(dwInterleaveMode == INTERLEAVE_MODE_OCQ)
            {
                if(pSectorInfoBuff)
                {
                    pSectorInfoBuff ++;
                }           
            }
        }
        return TRUE;
    }
    else
    {
        return ReadSectorCsX(dwCs, startSectorAddr, pSectorBuff, pSectorInfoBuff);
    }
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_EraseBlock
//
//  This function erases the specified flash block.
//
//  Parameters:
//      blockID 
//          [in] The block number to erase.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_EraseBlock(DWORD dwCs, BLOCK_ID blockID)
{
    if(dwCs == INTERLEAVE_CS)
    {
        //we have interleave operation here
        //We need reverse the order of CS to workaround the bug of TO1
        int cs = 0;
        
        for(cs =  0; cs < g_CSPfi.NumberOfChip; cs++)
        {
            UINT32 addr = (blockID * g_CSPfi.fi.wSectorsPerBlock);
            SetCs(cs);
            NF_CMD(g_CSPfi.CmdErase1 | (g_CSPfi.CmdErase2 << 8));
            NF_ADDR_ERASE((addr << gLog2Num) | cs);
            
            NF_AUTO_LAUNCH(NFC_LAUNCH_NFC_AUTO_ERASE);
            NFCWait();    
            //HW Workaround
            NFCCheckStatus(cs, 1);
        }
    }
    else
    {
        UINT32 addr = (blockID * g_CSPfi.fi.wSectorsPerBlock);    
        SetCs(dwCs);
#ifndef  ATOMIC_MODE        
        NF_CMD(g_CSPfi.CmdErase1 | (g_CSPfi.CmdErase2 << 8));
        NF_ADDR_ERASE((addr << gLog2Num) | dwCs);
        
        NF_AUTO_LAUNCH(NFC_LAUNCH_NFC_AUTO_ERASE);
        NFCWait();    
        NFCCheckStatus(dwCs, 1);
#else//  ATOMIC_MODE
        NF_CMD_ATOMIC(g_CSPfi.CmdErase1);
        NF_ADDR_ATOMIC(((addr << gLog2Num) | dwCs));
        NF_ADDR_ATOMIC(((addr << gLog2Num) | dwCs)>>8);
        NF_ADDR_ATOMIC(((addr << gLog2Num) | dwCs)>>16);
        NF_CMD_ATOMIC(g_CSPfi.CmdErase2);
        NFCWait(TRUE);    
        NFCCheckStatus(dwCs, 1);    
#endif      

    }
    return TRUE;
}

BOOL WriteSectorCsX(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
                    PSectorInfo pSectorInfoBuff)
{
    BOOL rc = FALSE;
    UINT32 ColumnAddr = 0;
    UINT32 PageAddr;    
    
    if (pSectorBuff == NULL && pSectorInfoBuff == NULL)
    {
        DEBUGMSG(TRUE, (_T("Sector/Info buffers are NULL\r\n")));
        goto cleanUp;
    }
     
    {
        PageAddr = (startSectorAddr << gLog2Num) | dwCs;
        if (pSectorInfoBuff)
        {
            // Update spare area with sector information provided
            NFCWriteSpare(pSectorInfoBuff);
        }
        else
        {
            // Clear out the spare area
            NFCClearSpare();
        }
        
        if (pSectorBuff)
        {
            // Move page data from sector buffer to NFC buffer
            NFCWriteMain(pSectorBuff);
        }
        NFCSwapBBI();

        NF_CMD(g_CSPfi.CmdWrite1 | (g_CSPfi.CmdWrite2 << 8));  // Send sequential data input command
        NF_ADDR(ColumnAddr, PageAddr);
        // Write out the page data
        NF_BUF_ADDR(0);   
        
        NF_CLEAR_STATUS();
        NF_AUTO_LAUNCH(NFC_LAUNCH_NFC_AUTO_PROG);
        NFCWait();
        NF_GET_RB_STATUS();
        
        if(!NFCCheckStatus(dwCs, 1))
        {
            DEBUGMSG(TRUE, (_T("NAND Program failed, dwCs=%d (PageAddr = 0x%x)\r\n"), dwCs, PageAddr)); 
            goto cleanUp;
        }
        
        if (pSectorInfoBuff)
        {
            // Move to next sector      
            pSectorInfoBuff++;
        }
    }
    rc = TRUE;
    
cleanUp:
    return  rc;
}
//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_WriteSector
//
//  This function writes the requested sector data and metadata to the 
//  flash media.
//
//  Parameters:
//
//      startSectorAddr 
//          [in] The starting physical sector address to write to.
//
//      pSectorBuff 
//          [in] Pointer to the buffer that contains the sector data to write. 
//          Set to NULL if no data is to be written.
//
//      pSectorInfoBuff 
//          [in] Buffer for an array of sector information structures. There 
//          must be one sector information entry for each sector that is to be 
//          written. Set to NULL if this data is not written.
//
//      dwNumSectors 
//          [in] Number of sectors to write.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL CSPNAND_WriteSector(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
    PSectorInfo pSectorInfoBuff)
{
    if (pSectorBuff == NULL && pSectorInfoBuff == NULL)
    {
        DEBUGMSG(TRUE, (_T("Sector/Info buffers are NULL\r\n")));
        return FALSE;
    }
        
    if(dwCs == INTERLEAVE_CS)
    {
        //interleave write
        UINT32 cs;
        BOOL rc = FALSE;
        UINT32 ColumnAddr = 0;
        UINT32 PageAddr = startSectorAddr;  
       
        for(cs = 0; cs < g_CSPfi.NumberOfChip; cs++)
        {
            PageAddr = (startSectorAddr << gLog2Num) | cs;
            
            if (pSectorInfoBuff)
            {
                // Update spare area with sector information provided
                NFCWriteSpare(pSectorInfoBuff);

                if(dwInterleaveMode == INTERLEAVE_MODE_OCQ)
                {
                    pSectorInfoBuff ++;
                }
            }
            else
            {
                // Clear out the spare area
                NFCClearSpare();
            }
            
            if (pSectorBuff)
            {
                // Move page data from sector buffer to NFC buffer
                NFCWriteMain(pSectorBuff);
                pSectorBuff += g_CSPfi.fi.wDataBytesPerSector;
            }
            
            NFCSwapBBI();
    
            NF_CMD(g_CSPfi.CmdWrite1 | (g_CSPfi.CmdWrite2 << 8));  // Send sequential data input command
            NF_ADDR(ColumnAddr, PageAddr);
            
            // Write out the page data
            NF_BUF_ADDR(0);   
            
            NF_CLEAR_STATUS();
            NF_AUTO_LAUNCH(NFC_LAUNCH_NFC_AUTO_PROG);
            NF_WAIT_PROG_DONE();
        }
        NFCWait();

        //We must poll the status of RB bit in IPC, or write may be not really finished.
        NF_GET_RB_STATUS();

        if(!NFCCheckStatus(0, g_CSPfi.NumberOfChip))
        {
            DEBUGMSG(TRUE, (_T("NAND Program failed: PageAddr = 0x%x.\r\n"), PageAddr)); 
            goto cleanUp;
        }
        
        rc = TRUE;
cleanUp:
        return  rc;
    }
    else
    {
        return WriteSectorCsX(dwCs, startSectorAddr, pSectorBuff, pSectorInfoBuff);
    }
}

//-----------------------------------------------------------------------------
//
//  Function: CSPNAND_GetECCStatus
//
//  This function retrieve the ECC status
//
//  Parameters:
//      None
//
//  Returns:  
//      ECC status
//
//-----------------------------------------------------------------------------
UINT32 CSPNAND_GetECCStatus()
{
    return INREG32(&g_pNfcAxi->ECC_STATUS_RESULT);
}
