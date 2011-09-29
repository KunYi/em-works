// Copyright (c) 2006, Chips & Media.  All rights reserved.
//-----------------------------------------------------------------------------
// Copyright (C) 2006-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:       vpu_api.h
//  Purpose:    Defines the structure and protoytpe of API for Video Codec module
//
//------------------------------------------------------------------------------

#ifndef __VPU_API_H__
#define __VPU_API_H__

#if __cplusplus
extern "C" {
#endif

#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
//------------------------------------------------------------------------------
// MACRO DEFINITIONS 
//------------------------------------------------------------------------------

#define MAX_FMO_SLICE_SAVE_BUF_SIZE  (32) // Max buffer to save FMO slice is 32KB
#define USER_DATA_INFO_OFFSET        (136) // 8*17
#define MAX_NUM_INSTANCE 4


// The Software Event Name used by User
#define VPU_INT_PIC_RUN_NAME                   L"Vpu PIC Run Command"
//------------------------------------------------------------------------------
// Define Type Enumerations and Structure
//------------------------------------------------------------------------------
typedef unsigned char Uint8;
typedef unsigned long Uint32;
typedef unsigned short Uint16;

typedef Uint32 PhysicalAddress;
typedef PhysicalAddress *PPhysicalAddress;

//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------

typedef enum {
    STD_AVC,
    STD_VC1,
    STD_MPEG2,
    STD_MPEG4,
    STD_H263,
    STD_DIV3,
    STD_MJPG,
    STD_RV
} CodStd;

typedef enum {
    RETCODE_SUCCESS = 0,
    RETCODE_FAILURE,
    RETCODE_INVALID_HANDLE,
    RETCODE_INVALID_PARAM,
    RETCODE_INVALID_COMMAND,
    RETCODE_ROTATOR_OUTPUT_NOT_SET,
    RETCODE_ROTATOR_STRIDE_NOT_SET,
    RETCODE_INVALID_FRAME_BUFFER,
    RETCODE_INSUFFICIENT_FRAME_BUFFERS,
    RETCODE_INVALID_STRIDE,
    RETCODE_WRONG_CALL_SEQUENCE,
    RETCODE_CALLED_BEFORE,
    RETCODE_NOT_INITIALIZED,
    RETCODE_FAILURE_TIMEOUT,
    RETCODE_BUSY,
    RETCODE_IDLE,
    RETCODE_REPORT_BUF_NOT_SET
} RetCode;

typedef enum {
    ENABLE_ROTATION,
    DISABLE_ROTATION,
    ENABLE_MIRRORING,
    DISABLE_MIRRORING,
    SET_MIRROR_DIRECTION,
    SET_ROTATION_ANGLE,
    SET_ROTATOR_OUTPUT,
    SET_ROTATOR_STRIDE,
    DEC_SET_SPS_RBSP,
    DEC_SET_PPS_RBSP,
    ENABLE_DERING,
    DISABLE_DERING,
    DEC_SET_REPORT_BUFSTAT,
    DEC_SET_REPORT_MBINFO,
    DEC_SET_REPORT_MVINFO,
    DEC_SET_REPORT_USERDATA,
    ENC_GET_SPS_RBSP,
    ENC_GET_PPS_RBSP,
    ENC_PUT_MP4_HEADER,
    ENC_PUT_AVC_HEADER,
    ENC_GET_VOS_HEADER,
    ENC_GET_VO_HEADER,
    ENC_GET_VOL_HEADER,
    ENC_SET_INTRA_MB_REFRESH_NUMBER,
    ENC_ENABLE_HEC,
    ENC_DISABLE_HEC,
    ENC_SET_SLICE_INFO,
    ENC_SET_GOP_NUMBER,
    ENC_SET_INTRA_QP,
    ENC_SET_BITRATE,
    ENC_SET_FRAME_RATE,
    ENC_SET_REPORT_MBINFO,
    ENC_SET_REPORT_MVINFO,
    ENC_SET_REPORT_SLICEINFO
} CodecCommand;

typedef struct {
    PhysicalAddress bufY;
    PhysicalAddress bufCb;
    PhysicalAddress bufCr;
    PhysicalAddress bufMvCol;   
} FrameBuffer;

typedef struct {
    Uint32 left;
    Uint32 top;
    Uint32 right;
    Uint32 bottom;
} Rect;

typedef enum {
    MIRDIR_NONE,
    MIRDIR_VER,
    MIRDIR_HOR,
    MIRDIR_HOR_VER
} MirrorDirection;

typedef struct {
    ULONG PhysAdd;
    ULONG VirtAdd;
    UINT Reserved; // Used by driver internally
} VPUMemAlloc;

typedef struct {
    int enable;
    int size; 
    Uint32 *addr;
    union {
        int mvNumPerMb;
        int userDataNum;
        int type;
    };
    union {    
        int userDataBufFull;
        int reserved;
    };
} ReportInfo;

struct CodecInst;

typedef struct CodecInst *CodecHandle;

//------------------------------------------------------------------------------
// Decode struct and definition
//------------------------------------------------------------------------------

typedef struct CodecInst DecInst;
typedef DecInst * DecHandle;

typedef struct {
    CodStd bitstreamFormat;
    PhysicalAddress bitstreamBuffer;
    Uint8 *virt_bitstreamBuffer;
    int bitstreamBufferSize;
    int mp4DeblkEnable;
    int reorderEnable;    
    int filePlayEnable;
    int picWidth;
    int picHeight;
    int dynamicAllocEnable;
    int streamStartByteOffset;
    int mjpg_thumbNailDecEnable;
    PhysicalAddress psSaveBuffer;
    int psSaveBufferSize;
    int interleavedCbCr;
    int mp4Class;
    int check1stDisplayTypeEnable;
} DecOpenParam;

typedef struct {
    int mbInfoBufSize;  // For Mb information saving for Error Concealment 
    int mvInfoBufSize;  // For Motion vector information saving
    int frameBufStatBufSize;  // For Frame Buffer Status saving
    int userDataBufSize; // For User Data saving
} DecReportBufSize;

typedef struct {
    int picWidth;   // {(PicX+15)/16} * 16
    int picHeight;  // {(PicY+15)/16} * 16
    Uint32 frameRateInfo;
    Uint32 picCropEnable;
    Rect picCropRect;    
    int mp4_dataPartitionEnable;
    int mp4_reversibleVlcEnable;
    int mp4_shortVideoHeader;
    int h263_annexJEnable;
    int minFrameBufferCount;
    int frameBufDelay;    
    int normalSliceSize;
    int worstSliceSize;
    int mjpg_thumbNailEnable;
    int mjpg_sourceFormat;
    
    int profile;
    int level;
    int interlace;
    int direct8x8Flag;
    int vc1_psf;
    int aspectRateInfo;
    int constraint_set_flag[4];
    DecReportBufSize reportBufSize;
}DecInitialInfo;

typedef struct {
    PhysicalAddress sliceSaveBuffer;
    int sliceSaveBufferSize;
} DecAvcSliceBufInfo;

typedef struct {
    DecAvcSliceBufInfo avcSliceBufInfo;
} DecBufInfo;


typedef struct {
    int prescanEnable;
    int prescanMode;
    int iframeSearchEnable;
    int skipframeMode;
    int skipframeNum;
    int chunkSize;
    int picStartByteOffset;
    PhysicalAddress picStreamBufferAddr;
} DecParam;

typedef struct {
    int indexFrameDisplay;
    int indexFrameDecoded;
    int picType;
    int numOfErrMBs;
    int hScaleFlag;
    int vScaleFlag;
    int prescanresult;
    int notSufficientPsBuffer;
    int notSufficientSliceBuffer;
    int decodingSuccess;
    int interlacedFrame;
    int mp4PackedPBframe;
    int h264Npf;
    
    int pictureStructure;
    int topFieldFirst;
    int repeatFirstField;
    union { 
            int mp2_progressiveFrame;
            int vc1_repeatFrame;
    };
    
    int fieldSequence;
    ReportInfo mbInfo;
    ReportInfo mvInfo;
    ReportInfo frameBufStat;
    ReportInfo userData;
    
    int decPicHeight;
    int decPicWidth;
    Rect decPicCrop;
} DecOutputInfo;

typedef struct {
    Uint32 *paraSet;
    int sizeInByte;
} DecParamSet;

//------------------------------------------------------------------------------
// encode struct and definition
//------------------------------------------------------------------------------

typedef struct CodecInst EncInst;
typedef EncInst * EncHandle;

typedef struct {
    int mp4_dataPartitionEnable;
    int mp4_reversibleVlcEnable;
    int mp4_intraDcVlcThr;
    int mp4_hecEnable;
    int mp4_verid;
} EncMp4Param;

typedef struct {
    int h263_annexJEnable;
    int h263_annexKEnable;
    int h263_annexTEnable;
} EncH263Param;

typedef struct {
    int avc_constrainedIntraPredFlag;
    int avc_disableDeblk;
    int avc_deblkFilterOffsetAlpha;
    int avc_deblkFilterOffsetBeta;
    int avc_chromaQpOffset;
    int avc_audEnable;
    int avc_fmoEnable;
    int avc_fmoSliceNum;
    int avc_fmoType;    
    int avc_fmoSliceSaveBufSize;
} EncAvcParam;

typedef struct {
    int mjpg_sourceFormat;
    int mjpg_restartInterval;
    int mjpg_thumbNailEnable;
    int mjpg_thumbNailWidth;
    int mjpg_thumbNailHeight;
    Uint8 * mjpg_hufTable;
    Uint8 * mjpg_qMatTable;
} EncMjpgParam;

typedef struct{
    int sliceMode;
    int sliceSizeMode;
    int sliceSize;
} EncSliceMode;

typedef struct {
    CodStd bitstreamFormat;
    PhysicalAddress bitstreamBuffer;
    Uint32 bitstreamBufferSize;
    Uint8 *virt_bitstreamBuffer;    
    int picWidth;
    int picHeight;
    Uint32 frameRateInfo;
    int bitRate;
    int initialDelay;
    int vbvBufferSize;
    int enableAutoSkip;
    int gopSize;
    EncSliceMode slicemode;
    int intraRefresh;    
    int rcIntraQp;  
    int dynamicAllocEnable;
    int ringBufferEnable;
    int interleavedCbCr;
    union {
        EncMp4Param mp4Param;
        EncH263Param h263Param;
        EncAvcParam avcParam;
        EncMjpgParam mjpgParam;
    } EncStdParam;

    int userQpMax;
    Uint32 userGamma;
    int RcIntervalMode; // 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    int MbInterval; // use when RcintervalMode is 3
} EncOpenParam;

typedef struct {
    int mbInfoBufSize;  // For Mb information saving for Error Concealment 
    int mvInfoBufSize;  // For Motion vector Information saving
    int sliceInfoBufSize; // For Slice Information saving
} EncReportBufSize;

typedef struct {
    int minFrameBufferCount;
    EncReportBufSize reportBufSize;
} EncInitialInfo;

typedef struct {
    FrameBuffer * sourceFrame;
    int forceIPicture;
    int skipPicture;
    int quantParam;
    PhysicalAddress picStreamBufferAddr;
    int picStreamBufferSize;
} EncParam;


typedef struct {
    PhysicalAddress bitstreamBuffer;
    Uint32 bitstreamSize;
    int bitstreamWrapAround; 
    int picType;
    int numOfSlices;
    ReportInfo mbInfo;
    ReportInfo mvInfo;
    ReportInfo sliceInfo;
} EncOutputInfo;


typedef struct {
    Uint32 *paraSet;
    int sizeInByte;
} EncParamSet;

typedef struct {
    PhysicalAddress searchRamAddr;
    int SearchRamSize;
} SearchRamParam;

typedef struct {
    PhysicalAddress PhysBuf;
    int size;
    int headerType;
} EncHeaderParam;

typedef enum {
    VOL_HEADER,
    VOS_HEADER,
    VIS_HEADER
} Mp4HeaderType;

typedef enum {
    SPS_RBSP,
    PPS_RBSP
} AvcHeaderType;    

//------------------------------------------------------------------------------
// MX51 CODEC API interfaces
//------------------------------------------------------------------------------
RetCode vpu_Init(void);
RetCode vpu_Deinit(void);
RetCode vpu_IsBusy(void);
RetCode vpu_GetVersionInfo(Uint32 *versionInfo);
RetCode vpu_AllocPhysMem(Uint32 cbSize, VPUMemAlloc *pmemalloc);
RetCode vpu_FreePhysMem(VPUMemAlloc *pmemalloc);
RetCode vpu_GetPhysAddrFromVirtAddr(void* lpvAddress, Uint32 cbSize, PhysicalAddress* lppAddress);

// function for decode
RetCode vpu_DecOpen(DecHandle * pHandle, DecOpenParam * pop);
RetCode vpu_DecClose(DecHandle handle);
RetCode vpu_DecSetEscSeqInit(DecHandle handle, int escape );
RetCode vpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo * info);
RetCode vpu_DecRegisterFrameBuffer(DecHandle handle,
                   FrameBuffer * bufArray, int num, int stride, DecBufInfo * pBufInfo);
RetCode vpu_DecGetBitstreamBuffer(DecHandle handle,    PhysicalAddress * prdPtr,
                   PhysicalAddress * pwrPtr, Uint32 * size );
RetCode vpu_DecUpdateBitstreamBuffer(DecHandle handle, Uint32 size);
RetCode vpu_DecStartOneFrame(DecHandle handle, DecParam *param); 
RetCode vpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info);
RetCode vpu_DecBitBufferFlush(DecHandle handle);
RetCode vpu_DecClrDispFlag(DecHandle handle, int index);
RetCode vpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * parameter);    

// function for encode
RetCode vpu_EncOpen(EncHandle * pHandle, EncOpenParam *pop);
RetCode vpu_EncClose(DecHandle handle);
RetCode vpu_EncGetInitialInfo(EncHandle handle, EncInitialInfo * info);
RetCode vpu_EncRegisterFrameBuffer(EncHandle handle,
                   FrameBuffer * bufArray, int num, int stride);
RetCode vpu_EncGetBitstreamBuffer(EncHandle handle, PhysicalAddress * prdPrt,
                   PhysicalAddress * pwrPtr, Uint32 * size);
RetCode vpu_EncUpdateBitstreamBuffer(EncHandle handle, Uint32 size);
RetCode vpu_EncStartOneFrame(EncHandle handle, EncParam * param );
RetCode vpu_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info);
RetCode vpu_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * parameter);

// function for both decder and encoder
RetCode vpu_Reset(CodecHandle handle, int index);

#ifdef __cplusplus
}
#endif

#endif //_VPU_API_H__
