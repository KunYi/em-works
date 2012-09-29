//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: common_nandfmd.h
//
//  Provides definitions for the FlashInfoExt structure and soc/bsp functions
//  that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_NANDFMD_H
#define __COMMON_NANDFMD_H

#if    __cplusplus
extern "C" {
#endif

#pragma warning(push)
#pragma warning(disable:4201)

#include <fmd.h>

#define IOCTL_VERDOR_FLASH_BASE 2048

#define IOCTL_DISK_COPYBACK \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x1, METHOD_BUFFERED, FILE_ANY_ACCESS) 

#define IOCTL_DISK_VENDOR_READ_EBOOT \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x2, METHOD_BUFFERED, FILE_ANY_ACCESS)        

#define IOCTL_DISK_VENDOR_WRITE_EBOOT \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_VENDOR_WRITE_NK \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_VENDOR_OTP_PROGRAM \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x8, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define IOCTL_DISK_VENDOR_OTP_READ \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_GET_NANDBOOT_MODE \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0xA, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_SET_NANDBOOT_MODE \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0xB, METHOD_BUFFERED, FILE_ANY_ACCESS)
        
#define IOCTL_DISK_VENDOR_GET_IMGINFO \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0xC, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
#define IOCTL_DISK_VENDOR_END_WRITE_EBOOT \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0xD, METHOD_BUFFERED, FILE_ANY_ACCESS)    

#define IOCTL_DISK_VENDOR_END_WRITE_NK \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0xE, METHOD_BUFFERED, FILE_ANY_ACCESS)  
    
#define IOCTL_DISK_VENDOR_SET_UPDATE_SIG \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0xF, METHOD_BUFFERED, FILE_ANY_ACCESS)  
    
#define IOCTL_DISK_VENDOR_GET_UPDATE_SIG \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x10, METHOD_BUFFERED, FILE_ANY_ACCESS)  

#define IOCTL_DISK_VENDOR_WRITE_IMAGE \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x11, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_VENDOR_END_WRITE_IMAGE \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x12, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DISK_VENDOR_GET_SBIMGINFO \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x13, METHOD_BUFFERED, FILE_ANY_ACCESS)

// CS&ZHL APR-9-2012: code for EM9280 security check
#define IOCTL_DISK_AUTHENTICATION \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x14, METHOD_BUFFERED, FILE_ANY_ACCESS)
// end of CS&ZHL APR-9-2012: code for EM9280 security check

// CS&ZHL JUN-1-2012: code for EM9280 uce to check OTP mac
#define IOCTL_DISK_GET_OTP_MAC \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x15, METHOD_BUFFERED, FILE_ANY_ACCESS)
// end of CS&ZHL JUN-1-2012: code for EM9280 uce to check OPT mac

// CS&ZHL AUG-13-2012: code for EM9280 uce to format NandFlash
#define IOCTL_DISK_FORMAT \
    CTL_CODE(IOCTL_DISK_BASE, IOCTL_VERDOR_FLASH_BASE + 0x16, METHOD_BUFFERED, FILE_ANY_ACCESS)
// end of CS&ZHL JUN-1-2012: code for EM9280 uce to format NandFlash

#define IOCTL_DISK_VENDOR_READ_SB			IOCTL_DISK_VENDOR_READ_EBOOT
#define IOCTL_DISK_VENDOR_WRITE_SB			IOCTL_DISK_VENDOR_WRITE_EBOOT
#define IOCTL_DISK_VENDOR_END_WRITE_SB		IOCTL_DISK_VENDOR_END_WRITE_EBOOT


// Interleave
#define INTERLEAVE_CS   0xFF
#define INTERLEAVE_MODE_BIG_SECTOR      0
#define INTERLEAVE_MODE_OCQ             1

#define ECC_BCH         0x01
#define ECC_RS          0x00
#define MAX_MARK_NUM    4
#define NANDID_LENGTH   4

#define IMAGE_EBOOT			0
#define IMAGE_NK			1
#define IMAGE_XLDR			2
#define IMAGE_EBOOTCFG		3			// CS&ZHL JAN-9-2012: add a new image type for Eboot CFG
#define IMAGE_SPLASH		4			// CS&ZHL JAN-9-2012: add a new image type for Splash Screen
#define IMAGE_MBR			5			// CS&ZHL MAR-29-2012: add a new image type for MBR.nb0
#define IMAGE_VID			6			// CS&ZHL APR-10-2012: add a new image type for vendor security info
#define IMAGE_UID			7			// CS&ZHL APR-10-2012: add a new image type for emtronix's customer security info
#define IMAGE_RFU			8			// CS&ZHL APR-10-2012: add a new image type for special storage purpose

typedef struct _AutoDetect
{
    BYTE    Enable;
    BYTE    CsSearchRange;
}AutoDetect, *PAutoDetect;

typedef enum _SBPosition
{
    FirstBoot = 1,
    SecondBoot,
    BothBoot  
}SBPosition, *PSBPosition;

typedef struct _NANDIORequest
{
    SBPosition SBPos;
    DWORD      dwIndex;
}NANDIORequest, *PNANDIORequest;

typedef struct _NANDImgInfo
{
    DWORD dwSBLen;
    DWORD dwSBLenUnit;
    DWORD dwNB0Len;
    DWORD dwNB0LenUnit;
    
}NANDImgInfo, *PNANDImgInfo;

typedef struct _NANDWrtImgInfo
{
    DWORD dwImgType;
    DWORD dwIndex;
    DWORD dwImgSizeUnit;
    
}NANDWrtImgInfo, *PNANDWrtImgInfo;


typedef struct _NANDTiming
{
    BYTE DataSetup;
    BYTE DataHold;
    BYTE AddressSetup;
    BYTE DataSample;
}NANDTiming, *PNANDTiming;

typedef struct _NandChipInfo
{
    FlashInfo   fi;                     //@<<info> FlashInfo structure
    
    BYTE        NANDCode[NANDID_LENGTH];//@<<info> NAND full ID
    
    BYTE        NumBlockCycles;         //@<<info> flash erase address cycle
    BYTE        ChipAddrCycleNum;       //@<<info> flash access address cycle
    BYTE        DataWidth;              //@<<info> 8/16 bits data width
    BYTE        BBMarkNum;              //@<<info> MAX_MARK_NUM = 4
    
    BYTE        BBMarkPage[MAX_MARK_NUM];//@<<info> MAX_MARK_NUM = 4
    
    BYTE        StatusBusyBit;          //@<<info> interleave mode support
    BYTE        StatusErrorBit;         //@<<info> interleave mode support
    WORD        SpareDataLength;        //@<<info> spare area size
    
    BYTE        CmdReadStatus;          //@<<command> read status
    BYTE        CmdRead1;               //@<<command> read first 256 bytes data
    BYTE        CmdRead2;               //@<<command> read last 256 bytes data
    BYTE        CmdReadId;              //@<<command> read device ID
    BYTE        CmdReset;               //@<<command> reset nand flash
    BYTE        CmdWrite1;              //@<<command> sequence data input
    BYTE        CmdWrite2;              //@<<command> page program
    BYTE        CmdErase1;              //@<<command> block erase
    BYTE        CmdErase2;              //@<<command> block erase
    
    NANDTiming  timings;                //@<<info> NAND timing parameters
}NandChipInfo, *pNandChipInfo;

typedef struct _FlashInfoExt
{
    union{
    
        struct{
            FlashInfo   fi;                     //@<<info> FlashInfo structure
        
            WORD        NANDIDCode;             //@<<info> NAND ID
            WORD        wReserved;
            
            BYTE        NumBlockCycles;         //@<<info> flash erase address cycle
            BYTE        ChipAddrCycleNum;       //@<<info> flash access address cycle
            BYTE        DataWidth;              //@<<info> 8/16 bits data width
            BYTE        BBMarkNum;              //@<<info> MAX_MARK_NUM = 4
            
            BYTE        *BBMarkPage;            //@<<info> contain Bad block mark page
            
            BYTE        StatusBusyBit;          //@<<info> interleave mode support
            BYTE        StatusErrorBit;         //@<<info> interleave mode support
            WORD        SpareDataLength;        //@<<info> spare area size
            
            BYTE        CmdReadStatus;          //@<<command> read status
            BYTE        CmdRead1;               //@<<command> read first 256 bytes data
            BYTE        CmdRead2;               //@<<command> read last 256 bytes data
            BYTE        CmdReadId;              //@<<command> read device ID
            BYTE        CmdReset;               //@<<command> reset nand flash
            BYTE        CmdWrite1;              //@<<command> sequence data input
            BYTE        CmdWrite2;              //@<<command> page program
            BYTE        CmdErase1;              //@<<command> block erase
            BYTE        CmdErase2;              //@<<command> block erase
            
            NANDTiming  timings;                //@<<info> NAND timing parameters
        };
        NandChipInfo ChipInfo;
    };
    
    WORD        BBIMainAddr;            //@<<info> BBI Addr
    BYTE        NumberOfChip;           //@<<info> flash cs number
    BYTE        ILSupport;              //@<<info> interleave mode support
    BYTE        ClusterCount;           //@<<info> cluster size
    BYTE        ECCType;                //@<<info> defines which ECC type is used
    AutoDetect  AutoDec;               //@<<info> enable auto detect NAND mode
    
}FlashInfoExt, *PFlashInfoExt;

typedef struct _BBTBuffer
{
    BYTE* pSectorBuf;                   // Secotr Buffer
    BYTE* pBBTFirstBuf;                 // The first BBT
    BYTE* pBBTSecondBuf;                // The second BBT
    DWORD dwBBTFirstSize;               // Size of the first BBT
    DWORD dwBBTSecondSize;              // Size of the second BBT
    DWORD dwBBTStartBlock;              // BBT start location
    DWORD dwBBTEndBlock;                // BBT end location
    DWORD dwCs;                         // BBT cs location
}BBTBuffer, *PBBTBuffer;

// SOC functions
PVOID   CSPNAND_Init(PFlashInfoExt pFlashInfo);
VOID    CSPNAND_PostInit(PFlashInfoExt pFlashInfo);
BOOL    CSPNAND_Deinit(PVOID hFMD);
BOOL    CSPNAND_WriteSector(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
                                    PSectorInfo pSectorInfoBuff);
BOOL    CSPNAND_ReadSector(DWORD dwCs, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, 
                                    PSectorInfo pSectorInfoBuff);
BOOL    CSPNAND_EraseBlock(DWORD dwCs, BLOCK_ID blockID);
BOOL    CSPNAND_ILSupport();
VOID    CSPNAND_EnableECC(BOOL Enable);
VOID    CSPNAND_Reset(DWORD dwCs);

UINT32  CSPNAND_ReadID(DWORD dwCs);
UINT32  CSPNAND_GetECCStatus();

// BSP functions
VOID    BSPNAND_GetFlashInfo(PFlashInfoExt pFlashInfo);
VOID    BSPNAND_GetBufferPointer(PBBTBuffer pBBT);
VOID    BSPNAND_ConfigIOMUX(DWORD CsNum);
VOID    BSPNAND_SetClock(BOOL bEnabled);
VOID*   BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size);
VOID    BSPNAND_UnmapRegister(PVOID VirtAddr, DWORD size);

//
// CS&ZHL MAY-13-2011: access NandFlash through KernelIoControl
//
#define FMD_ACCESS_CODE_HWINIT				(0)
#define FMD_ACCESS_CODE_READSECTOR			(1)
#define FMD_ACCESS_CODE_WRITESECTOR			(2)
#define FMD_ACCESS_CODE_ERASEBLOCK			(3)
#define FMD_ACCESS_CODE_GETINFO				(4)		// pMData ->FlashInfo buffer; *pSData = sizeof(FlashInfo)
#define FMD_ACCESS_CODE_GETSTATUS			(5)		// pMData ->&dwStatus
#define FMD_ACCESS_CODE_SETSTATUS			(6)		// pMData ->&dwStatus
//
// CS&ZHL SEP-18-2012: supporting UserID write and verify
//
#define FMD_ACCESS_CODE_WRITEUID			(7)		// pMData ->UserID buffer; *((PDWORD)pSData) = sizeof(UserID buffer)
#define FMD_ACCESS_CODE_VERIFYUID			(8)		// pMData ->UserID buffer; *((PDWORD)pSData) = sizeof(UserID buffer)

typedef struct _FmdAccessInfo
{
	DWORD dwAccessCode;
	DWORD dwStartSector;
	DWORD dwSectorNum;
	VOID * pMData;
	VOID * pSData;
    
} FmdAccessInfo, *PFmdAccessInfo;


#pragma warning(pop)

#ifdef __cplusplus
}
#endif

#endif // __COMMON_NANDFMD_H
