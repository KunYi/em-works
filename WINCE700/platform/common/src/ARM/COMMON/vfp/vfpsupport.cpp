//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

#include <corecrtstorage.h>
#include "vfpSupport.h"


//------------------------------------------------------------------------------
// VFP globals
//------------------------------------------------------------------------------

static DWORD g_dwFPSID = 0;


//------------------------------------------------------------------------------
// VFP emulation engine definitions
//------------------------------------------------------------------------------

#ifdef DEBUG
DWORD A_REG_INDEX(DWORD r)      { DEBUGCHK(r < 16); return r; }
DWORD S_REG_INDEX(DWORD s)      { DEBUGCHK(s < 32); return s; }
DWORD D_REG_INDEX(DWORD d)      { DEBUGCHK(d < 16); return d; }
#else
#define A_REG_INDEX(r)          (r)
#define S_REG_INDEX(s)          (s)
#define D_REG_INDEX(d)          (d)
#endif

#define ARM_PSR                 (pContext->Psr)
#define ARM_REG(r)              (*( &pContext->R0 + A_REG_INDEX(r) ))
#define VFP_SREG(s)             (*( pContext->S + S_REG_INDEX(s) ))
#define VFP_DLREG(d)            (*( pContext->S + (D_REG_INDEX(d) << 1) ))
#define VFP_DHREG(d)            (*( pContext->S + (D_REG_INDEX(d) << 1) + 1 ))
#define VFP_FPSCR               (pContext->Fpscr)
#define FPEXC                   (pContext->FpExc)
#define VFP_FPSID               g_dwFPSID

#define VFP_SREG_PTR(s)         (((float*)pContext->S) + S_REG_INDEX(s))
#define VFP_DREG_PTR(d)         (((double*)pContext->S) + D_REG_INDEX(d))

#define VFP_SREG_FP(s)          (*VFP_SREG_PTR(s))
#define VFP_DREG_FP(d)          (*VFP_DREG_PTR(d))

#define VFP_SREG_SI(s)          (*(int*)VFP_SREG_PTR(s))

// VFPv2 sub-architecture specific
//
#define VFP_FPINST              (pContext->FpExtra[0])
#define VFP_FPINST2             (pContext->FpExtra[1])
#define VFP_TMP_FPSCR           (pContext->FpExtra[2])

// VFP NaNs
//
#define VFP_IS_F32_NAN(x)       (((x) & 0x7fffffff) > 0x7f800000)
#define VFP_IS_F64_NAN(lo, hi)  (((hi) & 0x7ff00000) == 0x7ff00000 && (((hi) & 0x000fffff) | (lo)))
#define VFP_F32_QNAN_BIT        0x00400000
#define VFP_F64_QNAN_BIT        0x00080000

// VFP compare flags
//
#define VFP_NZCV_EQUAL          0x60000000
#define VFP_NZCV_LT             0x80000000
#define VFP_NZCV_GT             0x20000000
#define VFP_NZCV_UNORDERED      0x30000000

// VFP (ARM) condition codes
//
#define VFP_COND_EQ             0x00000000L // Z set
#define VFP_COND_NE             0x10000000L // Z clear
#define VFP_COND_CS             0x20000000L // C set    // aka HS
#define VFP_COND_CC             0x30000000L // C clear  // aka LO
#define VFP_COND_MI             0x40000000L // N set
#define VFP_COND_PL             0x50000000L // N clear
#define VFP_COND_VS             0x60000000L // V set
#define VFP_COND_VC             0x70000000L // V clear
#define VFP_COND_HI             0x80000000L // C set and Z clear
#define VFP_COND_LS             0x90000000L // C clear or Z set
#define VFP_COND_GE             0xa0000000L // N == V
#define VFP_COND_LT             0xb0000000L // N != V
#define VFP_COND_GT             0xc0000000L // Z clear, and N == V
#define VFP_COND_LE             0xd0000000L // Z set, or N != V
#define VFP_COND_AL             0xe0000000L // Always execute
#define VFP_COND_NV             0xf0000000L // Never - undefined
#define VFP_COND_MASK           0xf0000000L

#define THUMB_MODE              0x20


//------------------------------------------------------------------------------
// The prototypes for the VFP addressing modes operation callbacks
//------------------------------------------------------------------------------

typedef void (*VfpOperationD)(double*, double*, double*);
typedef void (*VfpOperationS)(float*, float*, float*);

typedef void (*VfpMonadicOperationD)(double*, double*);
typedef void (*VfpMonadicOperationS)(float*, float*);


//------------------------------------------------------------------------------
// IMPORTANT: Keep this in sync with the definitions in cruntime.h !!!
//------------------------------------------------------------------------------

extern "C"
{

DWORD* __crt_get_storage_fsr();
DWORD* __crt_get_storage_flags();
DWORD* __crt_get_kernel_flags();

void __crtRaiseException(
    DWORD dwExceptionCode,
    DWORD dwExceptionFlags,
    DWORD nNumberOfArguments,
    const DWORD * lpArguments
    );
    
} // extern "C"


//------------------------------------------------------------------------------
// IMPORTANT: Keep this in sync with the definitions in crttrans.h !!!
//------------------------------------------------------------------------------

extern "C"
{

unsigned int _get_fsr(void);
void _set_fsr(unsigned int);

unsigned int _fpscr2cw(unsigned int fpscr);
unsigned int _cw2fpscr(unsigned int cw);

} // extern "C"


#define SFC_SET(x)  (_set_fsr(x))
#define SFC_GET()   (_get_fsr())

#define FPSCR_STATUS_MASK   0x0000009f
#define FPSCR_CW_MASK       0x01c09f00
#define FPSCR_ROUND_MASK    0x00c00000

#define IMCW_RC             0x00000003
#define IRC_CHOP            0x00000001
#define IRC_UP              0x00000002
#define IRC_DOWN            0x00000003
#define IRC_NEAR            0x00000000


//------------------------------------------------------------------------------
// This is the API to the floating point library used by the VFP emulation
//------------------------------------------------------------------------------

extern "C"
{

float sqrtf(float);
double sqrt(double);

double __addd(double, double);
double __subd(double, double);
double __muld(double, double);
double __divd(double, double);
double __negd(double);

float __adds(float, float);
float __subs(float, float);
float __muls(float, float);
float __divs(float, float);
float __negs(float);

float __dtos(double);
double __stod(float);

float __itos(int);
float __utos(unsigned int);
double __itod(int);
double __utod(unsigned int);

int __stoi(float);
int __dtoi(double);
unsigned int __stou(float);
unsigned int __dtou(double);

int __eqs(float, float);
int __lts(float, float);
int __eqd(double, double);
int __ltd(double, double);

double __absd(double);
float __abss(float);

} // extern "C"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static
BOOL HaveVfpHardware()
{
    DEBUGCHK(VFP_FPSID != 0);
    return VFP_FULL_EMULATION_FPSID != VFP_FPSID;
}


#ifdef DEBUG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static
BOOL IsVfpInstruction (DWORD dwInst)
{
    return (dwInst & 0x00000e00) == 0x00000a00;
}
#endif


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#pragma intrinsic(_lrotl)

static
DWORD ThumbVfpSwapHalfWords(DWORD dwInst)
{
    return _lrotl(dwInst, 16);
}


//------------------------------------------------------------------------------
// Advance the Thumb-2 ITSTATE field in CPSR.  This is necessary when the
// program counter is advanced.
//------------------------------------------------------------------------------
static
void AdvanceItState(PCONTEXT pContext)
{
#pragma warning(push)
//Under Microsoft extensions (/Ze), you can specify a structure without 
//a declarator as members of another structure or union. These structures 
//generate W4 warning C4201. 
#pragma warning(disable:4201) 
    union
    {
        DWORD dw;
        struct
        {
            DWORD unused0     : 10;
            DWORD itstate7_2  : 6;
            DWORD unused1     : 9;
            DWORD itstate1_0  : 2;
            DWORD unused2     : 5;
        };
    } u = { pContext->Psr };
#pragma warning(pop)

    DWORD dwItstate = (u.itstate7_2 << 2) | u.itstate1_0;

    if ((dwItstate & 7) == 0)
    {
        // No more predicate bits
        //
        dwItstate = 0;
    }
    else
    {
        // ITSTATE[4:0] <<= 1;
        //
        dwItstate = (dwItstate & 0xE0) | ((dwItstate << 1) & 0x1F);
    }

    u.itstate7_2 = dwItstate >> 2;
    u.itstate1_0 = dwItstate & 3;

    pContext->Psr = u.dw;
}


//------------------------------------------------------------------------------
// The VFP f64 arithmetic operations (used as callbacks for various addressing modes)
//------------------------------------------------------------------------------

static void VfpAddD(double* pResult, double* pA, double* pB)
{
    *pResult = __addd(*pA, *pB);
}


static void VfpSubD(double* pResult, double* pA, double* pB)
{
    *pResult = __subd(*pA, *pB);
}


static void VfpMulD(double* pResult, double* pA, double* pB)
{
    *pResult = __muld(*pA, *pB);
}


static void VfpDivD(double* pResult, double* pA, double* pB)
{
    *pResult = __divd(*pA, *pB);
}


static void VfpNMulD(double* pResult, double* pA, double* pB)
{
    *pResult = __negd(__muld(*pA, *pB));
}


static void VfpMulAddAccD(double* pResult, double* pA, double* pB)
{
    *pResult = __addd(*pResult, __muld(*pA, *pB));
}


static void VfpMulSubAccD(double* pResult, double* pA, double* pB)
{
    *pResult = __subd(*pResult, __muld(*pA, *pB));
}


static void VfpNMulAddAccD(double* pResult, double* pA, double* pB)
{
    *pResult = __subd(__muld(*pA, *pB), *pResult);
}


static void VfpNMulSubAccD(double* pResult, double* pA, double* pB)
{
    *pResult = __subd(__negd(*pResult), __muld(*pA, *pB));
}


//------------------------------------------------------------------------------
// The VFP f32 arithmetic operations (used as callbacks for various addressing modes)
//------------------------------------------------------------------------------

static void VfpAddS(float* pResult, float* pA, float* pB)
{
    *pResult = __adds(*pA, *pB);
}


static void VfpSubS(float* pResult, float* pA, float* pB)
{
    *pResult = __subs(*pA, *pB);
}


static void VfpMulS(float* pResult, float* pA, float* pB)
{
    *pResult = __muls(*pA, *pB);
}


static void VfpDivS(float* pResult, float* pA, float* pB)
{
    *pResult = __divs(*pA, *pB);
}


static void VfpNMulS(float* pResult, float* pA, float* pB)
{
    *pResult = __negs(__muls(*pA, *pB));
}


static void VfpMulAddAccS(float* pResult, float* pA, float* pB)
{
    *pResult = __adds(*pResult, __muls(*pA, *pB));
}


static void VfpMulSubAccS(float* pResult, float* pA, float* pB)
{
    *pResult = __subs(*pResult, __muls(*pA, *pB));
}


static void VfpNMulAddAccS(float* pResult, float* pA, float* pB)
{
    *pResult = __subs(__muls(*pA, *pB), *pResult);
}


static void VfpNMulSubAccS(float* pResult, float* pA, float* pB)
{
    *pResult = __subs(__negs(*pResult), __muls(*pA, *pB));
}


//------------------------------------------------------------------------------
// The VFP f64 misc operations (used as callbacks for various addressing modes)
//------------------------------------------------------------------------------

static void VfpCopyD(double* pResult, double* pX)
{
    *pResult = *pX;
}


static void VfpAbsD(double* pResult, double* pX)
{
    *pResult = __absd(*pX);
}


static void VfpNegD(double* pResult, double* pX)
{
    *pResult = __negd(*pX);
}


static void VfpSqrtD(double* pResult, double* pX)
{
    *pResult = sqrt(*pX);
}


//------------------------------------------------------------------------------
// The VFP f32 misc operations (used as callbacks for various addressing modes)
//------------------------------------------------------------------------------

static void VfpCopyS(float* pResult, float* pX)
{
    *pResult = *pX;
}


static void VfpAbsS(float* pResult, float* pX)
{
    *pResult = __abss(*pX);
}


static void VfpNegS(float* pResult, float* pX)
{
    *pResult = __negs(*pX);
}


static void VfpSqrtS(float* pResult, float* pX)
{
    *pResult = sqrtf(*pX);
}


//------------------------------------------------------------------------------
// Extracts the (len, stride) from a FPSCR value
//------------------------------------------------------------------------------
static
void VfpGetVectorLenStride(DWORD fpscr, DWORD* pLen, DWORD* pStride)
{
#ifdef DEBUG
    // validate len/stride combinations
    //
    switch((fpscr >> 16) & 0x37)
    {
    case 0x00:
    case 0x01:
    case 0x31:
    case 0x02:
    case 0x03:
        break;

    default:
        DEBUGMSG(1, (L"VFP: Invalid (len, stride) combination"));
    }
#endif

    *pLen = ((fpscr >> 16) & 0x7) + 1;
    *pStride = ((fpscr >> 20) & 0x3) ^ 0x1;
}


//------------------------------------------------------------------------------
// Helper functions for VFP vector addressing modes
//------------------------------------------------------------------------------

static
DWORD VfpIncVectorS(DWORD reg, DWORD stride)
{
    return (((reg & 0x7) + stride) & 0x7) | (reg & 0x18);
}


static
DWORD VfpIncVectorD(DWORD reg, DWORD stride)
{
    return (((reg & 0x3) + stride) & 0x3) | (reg & 0xc);
}


//------------------------------------------------------------------------------
// VFP addressing mode 1 : single precision vectors (non-monadic)
//------------------------------------------------------------------------------
static
void VfpAddressingMode_1(DWORD dwInstr, PCONTEXT pContext, VfpOperationS op)
{
    DWORD Fd = ((dwInstr >> 11) & 0x1e) | ((dwInstr >> 22) & 0x1);
    DWORD Fn = ((dwInstr >> 15) & 0x1e) | ((dwInstr >> 7) & 0x1);
    DWORD Fm = ((dwInstr << 1) & 0x1e) | ((dwInstr >> 5) & 0x1);
    
    DWORD stride = 0;
    DWORD len = 0;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0f000f10) == 0x0e000a00);

    // get the len/stride
    //
    VfpGetVectorLenStride(VFP_TMP_FPSCR, &len, &stride);

    // scalar operation?
    //
    if((Fd & 0x18) == 0 || (len == 1 && stride == 1))
    {
        op(VFP_SREG_PTR(Fd), VFP_SREG_PTR(Fn), VFP_SREG_PTR(Fm));
    }
    // mixed vector/scalar operation?
    //
    else if((Fm & 0x18) == 0)
    {
        while(len--)
        {
            op(VFP_SREG_PTR(Fd), VFP_SREG_PTR(Fn), VFP_SREG_PTR(Fm));

            Fd = VfpIncVectorS(Fd, stride);
            Fn = VfpIncVectorS(Fn, stride);
        }
    }
    // vector operation
    //
    else
    {
        while(len--)
        {
            op(VFP_SREG_PTR(Fd), VFP_SREG_PTR(Fn), VFP_SREG_PTR(Fm));

            Fd = VfpIncVectorS(Fd, stride);
            Fn = VfpIncVectorS(Fn, stride);
            Fm = VfpIncVectorS(Fm, stride);
        }
    }
}


//------------------------------------------------------------------------------
// VFP addressing mode 2 : double precision vectors (non-monadic)
//------------------------------------------------------------------------------
static
void VfpAddressingMode_2(DWORD dwInstr, PCONTEXT pContext, VfpOperationD op)
{
    DWORD Dd = (dwInstr >> 12) & 0xf;
    DWORD Dn = (dwInstr >> 16) & 0xf;
    DWORD Dm = (dwInstr >> 0) & 0xf;
    
    DWORD stride = 0;
    DWORD len = 0;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0f400fb0) == 0x0e000b00);

    // get the len/stride
    //
    VfpGetVectorLenStride(VFP_TMP_FPSCR, &len, &stride);

    // scalar operation?
    //
    if((Dd & 0xc) == 0 || (len == 1 && stride == 1))
    {
        op(VFP_DREG_PTR(Dd), VFP_DREG_PTR(Dn), VFP_DREG_PTR(Dm));
    }
    // mixed vector/scalar operation?
    //
    else if((Dm & 0xc) == 0)
    {
        while(len--)
        {
            op(VFP_DREG_PTR(Dd), VFP_DREG_PTR(Dn), VFP_DREG_PTR(Dm));

            Dd = VfpIncVectorD(Dd, stride);
            Dn = VfpIncVectorD(Dn, stride);
        }
    }
    // vector operation
    //
    else
    {
        while(len--)
        {
            op(VFP_DREG_PTR(Dd), VFP_DREG_PTR(Dn), VFP_DREG_PTR(Dm));

            Dd = VfpIncVectorD(Dd, stride);
            Dn = VfpIncVectorD(Dn, stride);
            Dm = VfpIncVectorD(Dm, stride);
        }
    }
}


//------------------------------------------------------------------------------
// VFP addressing mode 3 : single precision vectors (monadic)
//------------------------------------------------------------------------------
static
void VfpAddressingMode_3(DWORD dwInstr, PCONTEXT pContext, VfpMonadicOperationS op)
{
    DWORD Fd = ((dwInstr >> 11) & 0x1e) | ((dwInstr >> 22) & 0x1);
    DWORD Fm = ((dwInstr << 1) & 0x1e) | ((dwInstr >> 5) & 0x1);
    
    DWORD stride = 0;
    DWORD len = 0;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0fb00f50) == 0x0eb00a40);

    // get the len/stride
    //
    VfpGetVectorLenStride(VFP_TMP_FPSCR, &len, &stride);

    // scalar operation?
    //
    if((Fd & 0x18) == 0 || (len == 1 && stride == 1))
    {
        op(VFP_SREG_PTR(Fd), VFP_SREG_PTR(Fm));
    }
    // mixed vector/scalar operation?
    //
    else if((Fm & 0x18) == 0)
    {
        while(len--)
        {
            op(VFP_SREG_PTR(Fd), VFP_SREG_PTR(Fm));

            Fd = VfpIncVectorS(Fd, stride);
        }
    }
    // vector operation
    //
    else
    {
        while(len--)
        {
            op(VFP_SREG_PTR(Fd), VFP_SREG_PTR(Fm));

            Fd = VfpIncVectorS(Fd, stride);
            Fm = VfpIncVectorS(Fm, stride);
        }
    }
}


//------------------------------------------------------------------------------
// VFP addressing mode 4 : double precision vectors (monadic)
//------------------------------------------------------------------------------
static
void VfpAddressingMode_4(DWORD dwInstr, PCONTEXT pContext, VfpMonadicOperationD op)
{
    DWORD Dd = (dwInstr >> 12) & 0xf;
    DWORD Dm = (dwInstr >> 0) & 0xf;
    
    DWORD stride = 0;
    DWORD len = 0;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0ff00f70) == 0x0eb00b40);

    // get the len/stride
    //
    VfpGetVectorLenStride(VFP_TMP_FPSCR, &len, &stride);

    // scalar operation?
    //
    if((Dd & 0xc) == 0 || (len == 1 && stride == 1))
    {
        op(VFP_DREG_PTR(Dd), VFP_DREG_PTR(Dm));
    }
    // mixed vector/scalar operation?
    //
    else if((Dm & 0xc) == 0)
    {
        while(len--)
        {
            op(VFP_DREG_PTR(Dd), VFP_DREG_PTR(Dm));

            Dd = VfpIncVectorD(Dd, stride);
        }
    }
    // vector operation
    //
    else
    {
        while(len--)
        {
            op(VFP_DREG_PTR(Dd), VFP_DREG_PTR(Dm));

            Dd = VfpIncVectorD(Dd, stride);
            Dm = VfpIncVectorD(Dm, stride);
        }
    }
}


//------------------------------------------------------------------------------
// Handles VFP conversions for coproc A
//------------------------------------------------------------------------------
static
void VfpHandleConversionA(DWORD dwInstr, PCONTEXT pContext, DWORD opcodeEx)
{
    DWORD Dd = (dwInstr >> 12) & 0xf;
    
    DWORD Sd = ((dwInstr >> 11) & 0x1e) | ((dwInstr >> 22) & 0x1);
    DWORD Sm = ((dwInstr << 1) & 0x1e) | ((dwInstr >> 5) & 0x1);

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0fb00f50) == 0x0eb00a40);

    switch(opcodeEx)
    {
    // FCVTDS (single -> double)
    //
    case 0x0f:
        VFP_DREG_FP(Dd) = __stod(VFP_SREG_FP(Sm));
        break;

    // FUITOS (uint -> single)
    //
    case 0x10:
        VFP_SREG_FP(Sd) = __utos(VFP_SREG(Sm));
        break;

    // FSITOS (int -> single)
    //
    case 0x11:
        VFP_SREG_FP(Sd) = __itos(VFP_SREG_SI(Sm));
        break;

    // FTOUIS (single -> uint)
    //
    case 0x18:
        VFP_SREG(Sd) = __stou(VFP_SREG_FP(Sm));
        break;

    // FTOSIS (single -> int)
    //
    case 0x1a:
        VFP_SREG_SI(Sd) = __stoi(VFP_SREG_FP(Sm));
        break;

    // FTOUIZS (single -> uint, round to zero)
    //
    case 0x19:
        SFC_SET((SFC_GET() & ~IMCW_RC) | IRC_CHOP);
        VFP_SREG(Sd) = __stou(VFP_SREG_FP(Sm));
        break;

    // FTOSIZS (single -> int, round to zero)
    //
    case 0x1b:
        SFC_SET((SFC_GET() & ~IMCW_RC) | IRC_CHOP);
        VFP_SREG_SI(Sd) = __stoi(VFP_SREG_FP(Sm));
        break;

    default:
        DEBUGCHK(FALSE);
    }
}


//------------------------------------------------------------------------------
// Handles VFP conversions for coproc B
//------------------------------------------------------------------------------
static
void VfpHandleConversionB(DWORD dwInstr, PCONTEXT pContext, DWORD opcodeEx)
{
    DWORD Dd = (dwInstr >> 12) & 0xf;
    DWORD Dm = (dwInstr >> 0) & 0xf;
    
    DWORD Sd = ((dwInstr >> 11) & 0x1e) | ((dwInstr >> 22) & 0x1);
    DWORD Sm = ((dwInstr << 1) & 0x1e) | ((dwInstr >> 5) & 0x1);

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0fb00f50) == 0x0eb00b40);

    switch(opcodeEx)
    {
    // FCVTSD (double -> single)
    //
    case 0x0f:
        VFP_SREG_FP(Sd) = __dtos(VFP_DREG_FP(Dm));
        break;

    // FUITOD (uint -> double)
    //
    case 0x10:
        VFP_DREG_FP(Dd) = __utod(VFP_SREG(Sm));
        break;
        
    // FSITOD (int -> double)
    //
    case 0x11:
        VFP_DREG_FP(Dd) = __itod(VFP_SREG_SI(Sm));
        break;

    // FTOUID (double -> uint)
    //
    case 0x18:
        VFP_SREG(Sd) = __dtou(VFP_DREG_FP(Dm));
        break;

    // FTOSID (double -> int)
    //
    case 0x1a:
        VFP_SREG_SI(Sd) = __dtoi(VFP_DREG_FP(Dm));
        break;

    // FTOUIZD (double -> uint, round to zero)
    //
    case 0x19:
        SFC_SET((SFC_GET() & ~IMCW_RC) | IRC_CHOP);
        VFP_SREG(Sd) = __dtou(VFP_DREG_FP(Dm));
        break;

    // FTOSIZD (double -> int, round to zero)
    //
    case 0x1b:
        SFC_SET((SFC_GET() & ~IMCW_RC) | IRC_CHOP);
        VFP_SREG_SI(Sd) = __dtoi(VFP_DREG_FP(Dm));
        break;

    default:
        DEBUGCHK(FALSE);
    }
}


//------------------------------------------------------------------------------
// Handles single-precision VFP compare instructions
//
// NOTE: The correct exception semantics depend on the floating point
//   supporting functions to raise an exception on _any_ NaN operands
//------------------------------------------------------------------------------
static
void VfpHandleCompareS(DWORD dwInstr, PCONTEXT pContext)
{
    DWORD NZCV = 0;

    DWORD Sd = ((dwInstr >> 11) & 0x1e) | ((dwInstr >> 22) & 0x1);
    DWORD Sm = ((dwInstr << 1) & 0x1e) | ((dwInstr >> 5) & 0x1);

    BOOL CMP_Z = (dwInstr >> 16) & 0x1;
    BOOL CMP_E = (dwInstr >> 7) & 0x1;

    float x = VFP_SREG_FP(Sd);
    float y = CMP_Z ? 0.0f : VFP_SREG_FP(Sm);

    BOOL fHaveQNaNs = FALSE;
    BOOL fHaveSNaNs = FALSE;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0fbe0f50) == 0x0eb40a40);

    // check to see if we have QNaN or SNaN operands
    //
    if(VFP_IS_F32_NAN(VFP_SREG(Sd)))
    {
        if(VFP_SREG(Sd) & VFP_F32_QNAN_BIT)
        {
            fHaveQNaNs = TRUE;
        }
        else
        {
            fHaveSNaNs = TRUE;
        }
    }

    if(!CMP_Z && VFP_IS_F32_NAN(VFP_SREG(Sm)))
    {
        if(VFP_SREG(Sm) & VFP_F32_QNAN_BIT)
        {
            fHaveQNaNs = TRUE;
        }    
        else
        {
            fHaveSNaNs = TRUE;
        }    
    }

    // perform the comparison
    //
    if(!CMP_E && fHaveQNaNs && !fHaveSNaNs)
    {
        NZCV = VFP_NZCV_UNORDERED;
    }
    else if(__lts(x, y))
    {
        NZCV = VFP_NZCV_LT;
    }
    else if(__eqs(x, y))
    {
        NZCV = VFP_NZCV_EQUAL;
    }
    else if(fHaveQNaNs || fHaveSNaNs)
    {
        // we could get here if exceptions are masked
        //
        NZCV = VFP_NZCV_UNORDERED;
    }
    else
    {
        NZCV = VFP_NZCV_GT;
    }

    // update the FPSCR flags
    //
    VFP_FPSCR = (VFP_FPSCR & 0x0fffffff) | NZCV;
}


//------------------------------------------------------------------------------
// Handles double-precision VFP compare instructions
//
// NOTE: The correct exception semantics depend on the floating point
//   supporting functions to raise an exception on _any_ NaN operands
//------------------------------------------------------------------------------
static
void VfpHandleCompareD(DWORD dwInstr, PCONTEXT pContext)
{
    DWORD NZCV = 0;

    DWORD Dd = (dwInstr >> 12) & 0xf;
    DWORD Dm = (dwInstr >> 0) & 0xf;

    BOOL CMP_Z = (dwInstr >> 16) & 0x1;
    BOOL CMP_E = (dwInstr >> 7) & 0x1;

    double x = VFP_DREG_FP(Dd);
    double y = CMP_Z ? 0.0 : VFP_DREG_FP(Dm);

    BOOL fHaveQNaNs = FALSE;
    BOOL fHaveSNaNs = FALSE;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0ffe0f70) == 0x0eb40b40);

    // check to see if we have QNaN or SNaN operands
    //
    if(VFP_IS_F64_NAN(VFP_DLREG(Dd), VFP_DHREG(Dd)))
    {
        if(VFP_DHREG(Dd) & VFP_F64_QNAN_BIT)
        {
            fHaveQNaNs = TRUE;
        }    
        else
        {
            fHaveSNaNs = TRUE;
        }    
    }

    if(!CMP_Z && VFP_IS_F64_NAN(VFP_DLREG(Dm), VFP_DHREG(Dm)))
    {
        if(VFP_DHREG(Dm) & VFP_F64_QNAN_BIT)
        {
            fHaveQNaNs = TRUE;
        }    
        else
        {
            fHaveSNaNs = TRUE;
        }    
    }

    // perform the comparison
    //
    if(!CMP_E && fHaveQNaNs && !fHaveSNaNs)
    {
        NZCV = VFP_NZCV_UNORDERED;
    }
    else if(__ltd(x, y))
    {
        NZCV = VFP_NZCV_LT;
    }
    else if(__eqd(x, y))
    {
        NZCV = VFP_NZCV_EQUAL;
    }
    else if(fHaveQNaNs || fHaveSNaNs)
    {
        // we could get here if exceptions are masked
        //
        NZCV = VFP_NZCV_UNORDERED;
    }
    else
    {
        NZCV = VFP_NZCV_GT;
    }

    // update the FPSCR flags
    //
    VFP_FPSCR = (VFP_FPSCR & 0x0fffffff) | NZCV;
}


//------------------------------------------------------------------------------
// Emulates the VFP v2 "two register" transfer instructions
//------------------------------------------------------------------------------
static
BOOL VfpHandleRegisterTransferEx(DWORD dwInstr, PCONTEXT pContext)
{
    DWORD cpBit = (dwInstr >> 8) & 0x1;
    DWORD Fm = dwInstr & 0xf;
    DWORD Rd = (dwInstr >> 12) & 0xf;
    DWORD Rn = (dwInstr >> 16) & 0xf;
    DWORD M = (dwInstr >> 5) & 0x1;
    DWORD L = (dwInstr >> 20) & 0x1;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0fe00e90) == 0x0c400a10);

    if(Rn == 15 || Rd == 15)
    {
        return FALSE;
    }    

    if(cpBit)
    {
        if(M)
        {
            return FALSE;
        }    

        // FMRRD?
        //
        if(L)
        {
            ARM_REG(Rd) = VFP_DLREG(Fm);
            ARM_REG(Rn) = VFP_DHREG(Fm);
        }
        // FMDRR
        //
        else
        {
            VFP_DLREG(Fm) = ARM_REG(Rd);
            VFP_DHREG(Fm) = ARM_REG(Rn);
        }
    }
    else
    {
        DWORD Sx = (Fm << 1) | M;

        if(31 == Sx)
            return FALSE;

        // FMRRS?
        //
        if(L)
        {
            ARM_REG(Rn) = VFP_SREG(Sx);
            ARM_REG(Rd) = VFP_SREG(Sx + 1);
        }
        // FMSRR
        //
        else
        {
            VFP_SREG(Sx) = ARM_REG(Rn);
            VFP_SREG(Sx + 1) = ARM_REG(Rd);
        }
    }

    return TRUE;
}


//------------------------------------------------------------------------------
// Validates the permissions to read or write to a memory location
//------------------------------------------------------------------------------
static
void VfpValidatePtr(BOOL fInKMode, DWORD dwAddress, DWORD dwSize, BOOL fWrite)
{
    if(!fInKMode && !IsValidUsrPtr((void*)dwAddress, dwSize, fWrite))
    {
        __crtRaiseException((DWORD)EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
    }
}


//------------------------------------------------------------------------------
// Root handler for VFP load/store instructions.
//
// NOTE: We handle the V2 "two regsiter transfer" instructions here, because
//   they're encoded in the load/store insctuction space even though they are
//   register transfer instructions.
//------------------------------------------------------------------------------
static
BOOL VfpHandleLoadStore(DWORD dwInstr, PCONTEXT pContext, BOOL fInKMode)
{
    DWORD dwCpNum = (dwInstr >> 8) & 0xf;
    DWORD cpBit = dwCpNum & 0x1;

    DWORD P = (dwInstr >> 24) & 0x1;
    DWORD U = (dwInstr >> 23) & 0x1;
    DWORD D = (dwInstr >> 22) & 0x1;
    DWORD W = (dwInstr >> 21) & 0x1;
    DWORD L = (dwInstr >> 20) & 0x1;
    DWORD Fd = (dwInstr >> 12) & 0xf;
    DWORD Rn = (dwInstr >> 16) & 0xf;
    int offset = dwInstr & 0xff;

    DWORD address = 0;
    DWORD i = 0;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0e000e00) == 0x0c000a00);

    // VFP load/store instructions shouldn't be emulated if we have VFP hardware
    //
    DEBUGCHK(!HaveVfpHardware());

    switch((P << 1) | W)
    {
    case 0: // FSTMx/FLDMx (unindexed)
    case 1: // FSTMx/FLDMx (increment)
    case 3: // FSTMx/FLDMx (decrement)

        // Is it a two-register transfer?
        //
        if((P | U | W) == 0)
        {
            return (D == 1) ? VfpHandleRegisterTransferEx(dwInstr, pContext) : FALSE;
        }
    
        if(P == U || 0 == offset)
        {
            return FALSE;
        }    

        address = ARM_REG(Rn);

        // using PC as base?
        //
        if(15 == Rn)
        {
            if(W)
            {
                return FALSE;
            }    
                
            address += 8;
        }

        if(P)
        {
            // We know P != U, thus U == 0 here
            //
            DEBUGCHK(0 == U);
            address -= offset * 4;
        }

        // FSTMD/FLDMD or FSTMX/FLDMX
        //
        if(cpBit)
        {
            DWORD count = offset / 2;
            
            if(D || Fd + count > 16)
            {
                return FALSE;
            }    

            // FLDMD? (or FLDMX)
            //
            if(L)
            {
                for(i = Fd; i < Fd + count; ++i, address += 8)
                {
                    VfpValidatePtr(fInKMode, address, 8, FALSE);
                    
                    VFP_DLREG(i) = *(ULONG*)address;
                    VFP_DHREG(i) = *(ULONG*)(address + 4);
                }
            }
            // FSTMD (or FSTMX)
            //
            else
            {
                for(i = Fd; i < Fd + count; ++i, address += 8)
                {
                    VfpValidatePtr(fInKMode, address, 8, TRUE);
                    
                    *(ULONG*)address = VFP_DLREG(i);
                    *(ULONG*)(address + 4) = VFP_DHREG(i);
                }
            }
        }
        // FSTMS/FLDMS
        //
        else
        {
            DWORD Sx = (Fd << 1) | D;
            
            if(offset + Sx > 32)
            {
                return FALSE;
            }    

            // FLDMS?
            //
            if(L)
            {
                for(i = Sx; i < offset + Sx; ++i, address += 4)
                {
                    VfpValidatePtr(fInKMode, address, 4, FALSE);
                    VFP_SREG(i) = *(ULONG*)address;
                }
            }
            // FSTMS
            //
            else
            {
                for(i = Sx; i < offset + Sx; ++i, address += 4)
                {
                    VfpValidatePtr(fInKMode, address, 4, TRUE);
                    *(ULONG*)address = VFP_SREG(i);
                }    
            }
        }

        if(W)
        {
            ARM_REG(Rn) += offset * (U ? 4 : -4);
        }
            
        break;
        
    // FSTx/FLDx (one value)
    //
    case 2:
        address = ARM_REG(Rn) + offset * (U ? 4 : -4);

        // using PC as base?
        //
        if(15 == Rn)
        {
            address += 8;
        }    
            
        if(cpBit)
        {
            if(D)
            {
                return FALSE;
            }    
                
            VfpValidatePtr(fInKMode, address, 8, !L);
            
            // FLDD?
            //
            if(L)
            {
                VFP_DLREG(Fd) = *(ULONG*)address;
                VFP_DHREG(Fd) = *(ULONG*)(address + 4);
            }
            // FSTD
            //
            else
            {
                *(ULONG*)address = VFP_DLREG(Fd);
                *(ULONG*)(address + 4) = VFP_DHREG(Fd);
            }
        }
        else
        {
            VfpValidatePtr(fInKMode, address, 4, !L);
            
            // FLDS?
            //
            if(L)
            {
                VFP_SREG((Fd << 1) | D) = *(ULONG*)address;
            }
            // FSTS
            //
            else
            {
                *(ULONG*)address = VFP_SREG((Fd << 1) | D);
            }
        }
        break;

    default:
        DEBUGCHK(FALSE);
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
// Root handler for VFP register transfer instructions
//
// NOTE: The emulation here doesn't strictly follow the MRC special case where
//   Rd is R15 (PC). We only trasfer the condition flags for FMSTAT as that's
//   the only case where the [31:28] bits would have a meaningful value.
//------------------------------------------------------------------------------
static
BOOL VfpHandleRegisterTransfer(DWORD dwInstr, PCONTEXT pContext, BOOL fInKMode)
{
    DWORD dwCpNum = (dwInstr >> 8) & 0xf;
    DWORD cpBit = dwCpNum & 0x1;

    DWORD opcode = (dwInstr >> 21) & 0x7;
    DWORD L = (dwInstr >> 20) & 0x1;
    DWORD Fn = (dwInstr >> 16) & 0xf;
    DWORD N = (dwInstr >> 7) & 0x1;
    DWORD Rd = (dwInstr >> 12) & 0xf;

    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0f000e70) == 0x0e000a10);

    // VFP reg transfer instructions shouldn't be emulated if we have VFP hardware
    //
    DEBUGCHK(!HaveVfpHardware());

    // FMSTAT?
    //
    if((dwInstr & 0x0fffffff) == 0x0ef1fa10)
    {
        ARM_PSR = (ARM_PSR & 0x0fffffff) | (VFP_FPSCR & 0xf0000000);
        return TRUE;
    }

    // Any other uses of R15 are invalid
    //
    if(15 == Rd)
    {
        return FALSE;
    }

    switch(opcode)
    {
    case 0:
        switch((cpBit << 1) | L)
        {
        // FMSR
        //
        case 0:
            VFP_SREG((Fn << 1) | N) = ARM_REG(Rd);
            break;
            
        // FMRS
        //
        case 1:
            ARM_REG(Rd) = VFP_SREG((Fn << 1) | N);
            break;
            
        // FMDLR
        //
        case 2:
            VFP_DLREG(Fn) = ARM_REG(Rd);
            break;
            
        // FMRDL
        //
        case 3:
            ARM_REG(Rd) = VFP_DLREG(Fn);
            break;

        default:
            DEBUGCHK(FALSE);
            return FALSE;
        }
        break;
        
    case 1:
        switch((cpBit << 1) | L)
        {
        // FMDHR
        //
        case 2:
            VFP_DHREG(Fn) = ARM_REG(Rd);
            break;

        // FMRDH
        //
        case 3:
            ARM_REG(Rd) = VFP_DHREG(Fn);
            break;

        default:
            return FALSE;
        }
        break;
        
    case 7:
        switch((cpBit << 1) | L)
        {
        // FMXR
        //
        case 0:
            switch(Fn)
            {
            case 0:
                // FMXR FPSID, <Rd> is a no-op
                break;
                
            case 1:
                VFP_FPSCR = ARM_REG(Rd);
                SFC_SET(_fpscr2cw(VFP_FPSCR));
                break;

            case 8:
                if(!fInKMode)
                    return FALSE;
                FPEXC = ARM_REG(Rd);
                break;

            default:
                return FALSE;
            }
            break;

        // FMRX
        //
        case 1:
            switch(Fn)
            {
            case 0:
                ARM_REG(Rd) = VFP_FPSID;
                break;
                
            case 1:
                ARM_REG(Rd) = VFP_FPSCR;
                break;
                
            case 7:
                if(!fInKMode)
                    return FALSE;
                ARM_REG(Rd) = FPEXC;
                break;

            default:
                return FALSE;
            }
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
// Root handler for VFP data processing (CDP) instructions
//------------------------------------------------------------------------------
static
BOOL VfpHandleDataProcessing(DWORD dwInstr, PCONTEXT pContext)
{
    DWORD dwCpNum = (dwInstr >> 8) & 0xf;
    DWORD cpBit = dwCpNum & 0x1;
    
    DWORD opcode = ((dwInstr >> 20) & 0xb) | ((dwInstr >> 4) & 0x4);
    
    // validate the instruction form
    //
    DEBUGCHK((dwInstr & 0x0f000e10) == 0x0e000a00);

    switch(opcode)
    {
    // FMACx
    //
    case 0x0:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpMulAddAccD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpMulAddAccS);
        break;
        
    // FNMACx
    //
    case 0x4:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpMulSubAccD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpMulSubAccS);
        break;
        
    // FMSCx
    //
    case 0x1:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpNMulAddAccD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpNMulAddAccS);
        break;
        
    // FNMSCx
    //
    case 0x5:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpNMulSubAccD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpNMulSubAccS);
        break;
        
    // FMULx
    //
    case 0x2:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpMulD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpMulS);
        break;
        
    // FNMULx
    //
    case 0x6:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpNMulD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpNMulS);
        break;
        
    // FADDx
    //
    case 0x3:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpAddD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpAddS);
        break;
        
    // FSUBx
    //
    case 0x7:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpSubD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpSubS);
        break;
        
    // FDIVx
    //
    case 0x8:
        if(cpBit)
            VfpAddressingMode_2(dwInstr, pContext, &VfpDivD);
        else
            VfpAddressingMode_1(dwInstr, pContext, &VfpDivS);
        break;
        
    // extended data processing opcode
    //
    case 0xf:
        {
            DWORD opcodeEx = ((dwInstr >> 15) & 0x1e) | ((dwInstr >> 7) & 0x1);
            
            switch(opcodeEx)
            {
            // FCPYx
            //
            case 0x0:
                if(cpBit)
                    VfpAddressingMode_4(dwInstr, pContext, &VfpCopyD);
                else
                    VfpAddressingMode_3(dwInstr, pContext, &VfpCopyS);
                break;

            // FABSx
            //
            case 0x1:
                if(cpBit)
                    VfpAddressingMode_4(dwInstr, pContext, &VfpAbsD);
                else
                    VfpAddressingMode_3(dwInstr, pContext, &VfpAbsS);
                break;

            // FNEGx
            //
            case 0x2:
                if(cpBit)
                    VfpAddressingMode_4(dwInstr, pContext, &VfpNegD);
                else
                    VfpAddressingMode_3(dwInstr, pContext, &VfpNegS);
                break;

            // FSQRTx
            //
            case 0x3:
                if(cpBit)
                    VfpAddressingMode_4(dwInstr, pContext, &VfpSqrtD);
                else
                    VfpAddressingMode_3(dwInstr, pContext, &VfpSqrtS);
                break;

            // compare
            //
            case 0x8:
            case 0x9:
            case 0xa:
            case 0xb:
                if(cpBit)
                    VfpHandleCompareD(dwInstr, pContext);
                else
                    VfpHandleCompareS(dwInstr, pContext);
                break;

            // conversions
            //
            case 0x0f:
            case 0x10:
            case 0x11:
            case 0x18:
            case 0x19:
            case 0x1a:
            case 0x1b:
                if(cpBit)
                    VfpHandleConversionB(dwInstr, pContext, opcodeEx);
                else
                    VfpHandleConversionA(dwInstr, pContext, opcodeEx);
                break;

            default:
                return FALSE;
            }
        }
        break;
        
    // undefined data processing instruction
    //
    default:
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
// Used when a SEH exception is raised inside the VFP emulation code. It copies the
// exeption code and exception parameters from the internal exception (keeps
// the rest of the original exception record, in particular the original address)
//------------------------------------------------------------------------------
static
int VfpInternalExceptionFilter(PEXCEPTION_POINTERS pExPointers, PEXCEPTION_RECORD pOriginalExr, DWORD* pdwReserved)
{
    PEXCEPTION_RECORD pExr = pExPointers->ExceptionRecord;
    DWORD i = 0;

    DEBUGCHK(pExr->NumberParameters <= EXCEPTION_MAXIMUM_PARAMETERS);
    
    // copy the internal exception information into the original exception record
    //
    pOriginalExr->ExceptionCode = pExr->ExceptionCode;
    pOriginalExr->NumberParameters = pExr->NumberParameters;

    for(i = 0; i < pOriginalExr->NumberParameters; ++i)
    {
        pOriginalExr->ExceptionInformation[i] = pExr->ExceptionInformation[i];
    }

    // update and print the exception information
    //
    switch(pExr->ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        DEBUGMSG(1, (L"Data exception, ExceptionCode=%d\r\n", pExr->ExceptionCode));
        break;

    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_UNDERFLOW:
        DEBUGMSG(1, (L"FPU exception, ExceptionCode=%d\r\n", pExr->ExceptionCode));

        // reset extra exception information
        //
        pOriginalExr->NumberParameters = 0;

        for(i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; ++i)
        {
            pOriginalExr->ExceptionInformation[i] = 0;
        }
        
        break;
    }

    // always execute our handler
    //
    return EXCEPTION_EXECUTE_HANDLER;
}


//------------------------------------------------------------------------------
// Executes one VFP instruction, honoring the flags and vector settings from VFP_TMP_FPSCR.
// (the return state is in VFP_FPSCR)
//
// Returns TRUE if the instruction was executed.
//------------------------------------------------------------------------------
BOOL VfpExecuteInstruction(DWORD dwInstr, PEXCEPTION_RECORD pExr, PCONTEXT pContext, DWORD* pdwReserved, BOOL fInKMode)
{
#ifdef DEBUG
    DWORD dwCpNum = (dwInstr >> 8) & 0xf;
#endif
    BOOL isHandled = FALSE;
    unsigned int savedSFC = 0;

    DEBUGCHK(dwCpNum == 10 || dwCpNum == 11);
    
    // update the CRT FpLib flags from VFP_TMP_FPSCR
    //
    savedSFC = SFC_GET();
    SFC_SET(_fpscr2cw(VFP_TMP_FPSCR));

    // save the kernel flags and disable the exception reporting
    //
    DWORD tlsKernBackup = *__crt_get_kernel_flags();
    DEBUGCHK(!(tlsKernBackup & TLSKERN_IN_FPEMUL));
    *__crt_get_kernel_flags() |= TLSKERN_NOFAULT | TLSKERN_NOFAULTMSG | TLSKERN_IN_FPEMUL;
    
    __try
    {
        switch((dwInstr >> 24) & 0xf)
        {
        // load/store
        //
        case 0x0c:
        case 0x0d:
            isHandled = VfpHandleLoadStore(dwInstr, pContext, fInKMode);
            break;

        // data processing / register transfer
        //
        case 0x0e:
            // register transfer
            //
            if((dwInstr >> 4) & 0x1)
            {
                isHandled = VfpHandleRegisterTransfer(dwInstr, pContext, fInKMode);
            }
            // data processing
            //
            else
            {
                isHandled = VfpHandleDataProcessing(dwInstr, pContext);
            }
            break;

        // unknown/invalid instruction
        //
        default:
            isHandled = FALSE;
        }
    }
    __except(VfpInternalExceptionFilter(GetExceptionInformation(), pExr, pdwReserved))
    {
        isHandled = FALSE;
    }
    
    // restore the kernel flags
    //
    *__crt_get_kernel_flags() = tlsKernBackup;

    // update the FPSCR status bits from the floating point library status
    //
    VFP_FPSCR = (VFP_FPSCR & ~FPSCR_STATUS_MASK) | (_cw2fpscr(SFC_GET()) & FPSCR_STATUS_MASK);

    // restore the floating point library control word
    //
    SFC_SET(savedSFC);

    return isHandled;
}


//------------------------------------------------------------------------------
// Create a temporary FPSCR by replacing the vector len FPSCR.LEN [18:16] with FPEXC.VECITR [10:8]
//------------------------------------------------------------------------------
static
ULONG VfpMakeExFpscr(ULONG fpscr, ULONG fpexc)
{
    // FPEXC.VECITR doesn't include the interation
    // in which the exception occurred, so add 1 here
    //
    return (fpscr & ~0x00070000) | (((fpexc + (1 << 8)) << 8) & 0x00070000);
}


//------------------------------------------------------------------------------
// Checks the condition field in an ARM instruction against the condition
// code flags in CPSR. Returns TRUE if the condition indicate "execute"
//------------------------------------------------------------------------------
static
BOOL VfpCheckConditionCode(DWORD dwInstr, DWORD psr)
{
    BOOL N = psr & (1 << 31);
    BOOL Z = psr & (1 << 30);
    BOOL C = psr & (1 << 29);
    BOOL V = psr & (1 << 28);
    
    switch(dwInstr & VFP_COND_MASK)
    {
    case VFP_COND_EQ: return Z;
    case VFP_COND_NE: return !Z;
    case VFP_COND_CS: return C;
    case VFP_COND_CC: return !C;
    case VFP_COND_MI: return N;
    case VFP_COND_PL: return !N;
    case VFP_COND_VS: return V;
    case VFP_COND_VC: return !V;
    case VFP_COND_HI: return C && !Z;
    case VFP_COND_LS: return !C || Z;
    case VFP_COND_GE: return N == V;
    case VFP_COND_LT: return N != V;
    case VFP_COND_GT: return !Z && (N == V);
    case VFP_COND_LE: return Z || (N != V);
    case VFP_COND_AL: return TRUE;
    }

    DEBUGCHK(FALSE);
    return FALSE;
}


//------------------------------------------------------------------------------
// The VFP exception handling callback. Returns true if the exception was handled.
//
// NOTE: The logic here is based on the VFP v2 commmon sub-architecture specification.
//------------------------------------------------------------------------------
BOOL VfpHandleException(ULONG fpexc, EXCEPTION_RECORD* pExr, CONTEXT* pContext, DWORD* pdwReserved, BOOL fInKMode)
{
    BOOL fThumb = ARM_PSR & THUMB_MODE;
    DWORD dwInstr = 0;

    DEBUGCHK(fThumb || ((pContext->Pc & 0x03) == 0));

    // by default use the vector length from FPSCR
    //
    VFP_TMP_FPSCR = VFP_FPSCR;

    // imprecise exception?
    //
    if ((fpexc & FPEXC_EX_BIT) &&

        // For the version 1 of the Common VFP sub-architecture (FPSID[22:16] == 1),
        // if FPSCR.IXE[12] is set then FPEXC.EX indicates a precise exception
        //
        !((VFP_FPSID & 0x007f0000) == 0x00010000 && (VFP_FPSCR & (1 << 12)))

        )
    {
        DEBUGCHK(HaveVfpHardware());

        // if in thumb mode, reverse halfwords
        //
        dwInstr = fThumb ? ThumbVfpSwapHalfWords(VFP_FPINST) : VFP_FPINST;

        // Execute FPINST (use the vector length from FPEXC [10:8])
        //
        VFP_TMP_FPSCR = VfpMakeExFpscr(VFP_FPSCR, fpexc);

        DEBUGCHK((dwInstr & 0xff000010) == 0xee000000);
        DEBUGCHK(IsVfpInstruction(dwInstr));

        if(!VfpExecuteInstruction(dwInstr, pExr, pContext, pdwReserved, fInKMode))
        {
            return FALSE;
        }

        // Do we have a "bypassed" instruction?
        //
        if(fpexc & FPEXC_FP2V_BIT)
        {
            dwInstr = fThumb ? ThumbVfpSwapHalfWords(VFP_FPINST2) : VFP_FPINST2;

            // Execute FPINST2 (using the vector length from FPSCR)
            //
            VFP_TMP_FPSCR = VFP_FPSCR;

            DEBUGCHK((dwInstr & 0xff000010) == 0xee000000);
            DEBUGCHK(IsVfpInstruction(dwInstr));

            if(!VfpExecuteInstruction(dwInstr, pExr, pContext, pdwReserved, fInKMode))
            {
                return FALSE;
            }
        }

        // Resume execution with the trigger instruction, which will be re-executed
        //
        return TRUE;
    }
    // precise exception
    //
    else
    {
        // Thumb-2: unaligned accesses plus halfwords swapping
        //
        if (fThumb)
        {
            USHORT *pEvenPc = (USHORT*)(pContext->Pc & ~1);
            dwInstr = ((*pEvenPc) << 16) | (*(pEvenPc + 1));
        }
        else
        {
            dwInstr = *(DWORD*)pContext->Pc;
        }

        // check the condition field
        //
        if((dwInstr & VFP_COND_MASK) == VFP_COND_NV)
        {
            // invalid condition code
            //
            return FALSE;
        }
        else if(!VfpCheckConditionCode(dwInstr, pContext->Psr))
        {
            DEBUGCHK(!fThumb);

            // condition code was not passed, skip the instruction
            //
            pContext->Pc += 4;
            return TRUE;
        }

        // VV bit set? (use FPEXC.VECITR)
        //
        if(fpexc & FPEXC_VV_BIT)
        {
            DEBUGCHK(fpexc & FPEXC_DEX_BIT);
            VFP_TMP_FPSCR = VfpMakeExFpscr(VFP_FPSCR, fpexc);
        }

        if(VfpExecuteInstruction(dwInstr, pExr, pContext, pdwReserved, fInKMode))
        {
            // advance PC to the next instruction
            //
            pContext->Pc += 4;

            // advance ITSTATE if in Thumb mode
            //
            if (fThumb)
            {
                AdvanceItState(pContext);
            }

            return TRUE;
        }
    }

    return FALSE;
}


//------------------------------------------------------------------------------
// The VFP/Neon specific feature helper for IsProcessorFeaturePresent()
//
// NOTE: this implementation is based on ARM's Common VFP sub-architecture. Different
//   VFP implementation may have to replace this with a custom implementation
//   (see: VfpOemInitEx())
//------------------------------------------------------------------------------
BOOL IsVfpFeaturePresent(DWORD dwFeature)
{
    DWORD dwSubarch = (VFP_FPSID >> 16) & 0xf;
    DWORD dwMVFR0 = 0;
    DWORD dwMVFR1 = 0;

    // generic features (relevant to hardware implementations and OS support)
    //
    switch(dwFeature)
    {
    case PF_ARM_VFP_SUPPORT:
        return TRUE;
        
    case PF_ARM_VFP_HARDWARE:
        return HaveVfpHardware();
    }

    // the rest of the features depend on hardware being present
    //
    if(!HaveVfpHardware())
    {
        return FALSE;
    }

    // if we have a VFPv3+ implementation read the MVFRx registers
    //
    if(dwSubarch > 1)
    {
        dwMVFR0 = VfpReadMVFR(0);
        dwMVFR1 = VfpReadMVFR(1);
    }

    // features relevant to hardware VFP implementations
    //
    switch(dwFeature)
    {
    case PF_ARM_VFP_V1:
        return dwSubarch == 0;
            
    case PF_ARM_VFP_V2:
        return dwSubarch == 1;
            
    case PF_ARM_VFP_V3:
        return dwSubarch > 1;
    
    case PF_ARM_VFP_SINGLE_PRECISION:
        return dwSubarch <= 1 || (dwMVFR0 & 0x000000f0) != 0;
        
    case PF_ARM_VFP_DOUBLE_PRECISION:
        return dwSubarch <= 1 ?
            (VFP_FPSID & (1<< 20)) == 0 :
            (dwMVFR0 & 0x00000f00) != 0;
    
    case PF_ARM_VFP_ALL_ROUNDING_MODES:
        return dwSubarch <= 1 || (dwMVFR0 & 0xf0000000) != 0;
        
    case PF_ARM_VFP_SHORT_VECTORS:
        return dwSubarch <= 1 || (dwMVFR0 & 0x0f000000) != 0;
        
    case PF_ARM_VFP_SQUARE_ROOT:
        return dwSubarch <= 1 || (dwMVFR0 & 0x00f00000) != 0;
        
    case PF_ARM_VFP_DIVIDE:
        return dwSubarch <= 1 || (dwMVFR0 & 0x000f0000) != 0;
        
    case PF_ARM_VFP_FP_EXCEPTIONS:
        return dwSubarch <= 1 || (dwMVFR0 & 0x0000f000) != 0;
        
    case PF_ARM_VFP_EXTENDED_REGISTERS:
        return dwSubarch > 1 && (dwMVFR0 & 0x0000000f) > 1;
    
    case PF_ARM_VFP_HALF_PRECISION:
        return dwSubarch > 1 && (dwMVFR1 & 0x0f000000) != 0;

    case PF_ARM_NEON:
        return dwSubarch > 1 && (dwMVFR1 & 0x0000f000) != 0;

    case PF_ARM_NEON_HALF_PRECISION:
        return dwSubarch > 1 && (dwMVFR1 & 0x00f00000) != 0;

    case PF_ARM_NEON_SINGLE_PRECISION:
        return dwSubarch > 1 && (dwMVFR1 & 0x000f0000) != 0;

    case PF_ARM_NEON_LOAD_STORE:
        return dwSubarch > 1 && (dwMVFR1 & 0x00000f00) != 0;

    case PF_ARM_VFP_DENORMALS:
        return dwSubarch > 1 && (dwMVFR1 & 0x0000000f) != 0;

    default:
        DEBUGCHK(!"unexpected VFP/Neon feature id");
        return FALSE;
    }    
}


//------------------------------------------------------------------------------
// The BSP support initialization
//------------------------------------------------------------------------------
void VfpOemInitEx(OEMGLOBAL* pOemGlobal, DWORD dwFPSID,
    PFN_VfpEnableCoproc         pfnVfpEnableCoproc,
    PFN_SaveRestoreVFPCtrlRegs  pfnSaveVFPCtrlRegs,
    PFN_SaveRestoreVFPCtrlRegs  pfnRestoreVFPCtrlRegs,
    PFN_HandleVFPException      pfnHandleVFPException,
    PFN_IsVFPFeaturePresent     pfnIsVFPFeaturePresent)
{
    // set the FPSID
    //
    if(VFP_AUTO_DETECT_FPSID == dwFPSID)
    {
        pfnVfpEnableCoproc();
        dwFPSID = VfpReadFpsid();
    }

    // set the platform flags
    //
    DEBUGCHK(0 == (pOemGlobal->dwPlatformFlags & OAL_HAVE_VFP_HARDWARE));
    DEBUGCHK(0 == (pOemGlobal->dwPlatformFlags & OAL_HAVE_VFP_SUPPORT));

    pOemGlobal->dwPlatformFlags |= OAL_HAVE_VFP_SUPPORT;
    
    if(VFP_FULL_EMULATION_FPSID != dwFPSID)
    {
        // validate the FPSID.SW bit value
        //
        DEBUGCHK(0 == (dwFPSID & (1 << 23)));
        
        pfnVfpEnableCoproc();
        pOemGlobal->dwPlatformFlags |= OAL_HAVE_VFP_HARDWARE;
    }

    g_dwFPSID = dwFPSID;

    // setup the OEM callbacks for VFP / Neon support
    //
    pOemGlobal->pfnSaveVFPCtrlRegs = pfnSaveVFPCtrlRegs;
    pOemGlobal->pfnRestoreVFPCtrlRegs = pfnRestoreVFPCtrlRegs;
    pOemGlobal->pfnHandleVFPExcp = pfnHandleVFPException;
    pOemGlobal->pfnIsVFPFeaturePresent = pfnIsVFPFeaturePresent;

    // test for extended register bank
    //
    if(pfnIsVFPFeaturePresent(PF_ARM_VFP_EXTENDED_REGISTERS))
    {
        pOemGlobal->dwPlatformFlags |= OAL_EXTENDED_VFP_REGISTERS;
    }

    DEBUGMSG(1, (L"Initializing VFP, FPSID=%08x\r\n", g_dwFPSID));
}


//------------------------------------------------------------------------------
// The BSP support initialization using the reference VFP support
// for ARM's Common VFP sub-architecture
//------------------------------------------------------------------------------
void VfpOemInit(OEMGLOBAL* pOemGlobal, DWORD dwFPSID)
{
    VfpOemInitEx(pOemGlobal, dwFPSID,
        VfpEnableCoproc,
        SaveVfpCtrlRegs,
        RestoreVfpCtrlRegs,
        VfpHandleException,
        IsVfpFeaturePresent);
}


