#if !defined(TYPES_TEMP_H)
#define TYPES_TEMP_H

#if defined (__cplusplus)
extern "C" {
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed char         s8;
typedef short               s16;
typedef int                 s32;
typedef long long           s64;

#ifndef bool
typedef BOOL                bool;
#endif

#if 0
typedef unsigned char       UInt8;
typedef unsigned short      UInt16;
typedef unsigned int        UInt32;
typedef signed char         Int8;
typedef short               Int16;
typedef int                 Int32;

#ifdef BOOL
#ifndef Bool
typedef BOOL                Bool;
#endif
#endif
#endif

#if 0
#ifndef FALSE
#define FALSE   false
#endif

#ifndef TRUE
#define TRUE    true
#endif
#else
#ifndef false
#define false   FALSE
#endif

#ifndef true
#define true    TRUE
#endif
#endif

#define ssize_t size_t

// From Linux stdio.h 
#define ENOMEM      12  /* Out of Memory */
#define EINVAL      22  /* Invalid argument */
#define ENOSPC      28  /* No space left on device */

#define GFP_DMA     (0x01u)
#define GFP_WAIT    (0x10u) /* Can wait and reschedule? */
#define GFP_IO      (0x40u) /* Can start physical IO? */
#define GFP_FS      (0x80u) /* Can call down to low-level FS? */
#define GFP_KERNEL  (GFP_WAIT | GFP_IO | GFP_FS)

#if 0
#define min(x,y) ({ \
    typeof(x) _x = (x);     \
    typeof(y) _y = (y);     \
    (void) (&_x == &_y);    \
    _x < _y ? _x : _y; })

#define max(x,y) ({ \
    typeof(x) _x = (x);     \
    typeof(y) _y = (y);     \
    (void) (&_x == &_y);    \
    _x > _y ? _x : _y; })
#endif

#if defined (__cplusplus)
}
#endif

#endif
