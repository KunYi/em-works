/*******************************************************************************
* Global Locate A-GPS chipset Application Programming Interface
*
* Copyright (c) 2001-2007 by Global Locate, Inc. All Rights Reserved.
*
* The information contained herein is confidential property of Global Locate. 
* The use, copying, transfer or disclosure of such information is prohibited 
* except by express written agreement with Global Locate.
*******************************************************************************/
#ifndef GLGPSAPI_H
#define GLGPSAPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* GL API VERSION CONTROL
*******************************************************************************/
#define GL_API_VER              182

/* The double-evaluation of (a) and (b) in this pair of macros converts 
 * GL_API_VER from "func_GL_API_VER" to "func_39". That is the true magic here.
 */
#ifndef GL_FUNC_NAME_AFTER
#define GL_FUNC_NAME_AFTER(a, b)    a ## _ ## b
#endif

#ifndef GL_FUNC_NAME
#define GL_FUNC_NAME(a, b)      GL_FUNC_NAME_AFTER(a, b)
#endif

/*******************************************************************************
* Utility macros
*******************************************************************************/
#ifndef _NUMOF
#define _NUMOF(name)            _NUMOF##name
#endif

#ifndef _DIM
#define _DIM(x) ((int)(sizeof(x)/sizeof(*(x))))
#endif

/*******************************************************************************
* GLGPS_API declaration
*******************************************************************************/
#define GLGPS_API

#ifdef _MSC_VER                 /* { */
    #if ( _MSC_VER == 1201 || (_MSC_VER >= 1400 && _WIN32_WCE))

        /* The following ifdef block is the standard way of creating macros,
         * which make exporting from a DLL simpler. All files within this DLL 
         * are compiled with the GLGPSDLL_EXPORTS symbol defined on the command
         * line. This symbol should not be defined on any project that uses 
         * this DLL. This way any other project whose source files include this
         * file see GLGPS_API functions as being imported from a DLL, whereas
         * this DLL sees symbols defined with this macro as being exported.
         */

        #undef GLGPS_API
        #if defined(GLGPSDLL_STATIC)
            #define GLGPS_API 
        #else
            #ifdef GLGPSDLL_EXPORTS /* { */
                #define GLGPS_API __declspec(dllexport)
            #else                   /* } { */
                #define GLGPS_API __declspec(dllimport)
            #endif                  /* } */
        #endif
    #endif                      /* } _MSC_VER == 1201 */
#endif                          /* } _MSC_VER */

/*******************************************************************************
* GL API return types
*******************************************************************************/
/*<GLML GlGpsApi.h GLRC_Type>*/
typedef enum GLRC
{
    GLRC_OK,       /* OK  */
    GLRC_ERR,      /* General error   */
    GLRC_TIMEOUT   /* Timeout expired, deprecated */
} GLRC;
/*</GLML>*/

/*******************************************************************************
* GLL status
*******************************************************************************/
/*<GLML GlGpsApi.h GL_STAT_Type>*/
typedef unsigned short GL_STAT; /* bitmask of GlStatusFlags, see below */
/*</GLML>*/

/* used in testing the status in a C++ manner using std::bitset */
enum GlStatusBits
{
    GL_DEAD_STAT,
    GL_STOPPED_STAT,
    GL_WAITING_STAT,
    GL_NEED_ACQ_STAT,
    GL_NEED_POS_STAT,
    GL_NEED_NAV_STAT,
    GL_NEED_TIME_STAT,
    /* ... */

    /* volatile flags */
    GL_HAVE_MEAS_STAT   = 8,
    GL_HAVE_FIX_STAT,
    GL_SUPL_END,
    GL_FIX_IN_PROGRESS,
    GL_PRECISE_TIME,
    GL_SYNC_STATUS_CHANGED
};

/*<GLML GlGpsApi.h GlStatusFlags_Type>*/
enum GlStatusFlags
{
    /* sticky flags */
    GL_DEAD_STAT_FLG        = (1 << GL_DEAD_STAT),
    GL_STOPPED_STAT_FLG     = (1 << GL_STOPPED_STAT),
    GL_WAITING_STAT_FLG     = (1 << GL_WAITING_STAT),
    GL_NEED_ACQ_STAT_FLG    = (1 << GL_NEED_ACQ_STAT),
    GL_NEED_POS_STAT_FLG    = (1 << GL_NEED_POS_STAT),
    GL_NEED_NAV_STAT_FLG    = (1 << GL_NEED_NAV_STAT),
    GL_NEED_TIME_STAT_FLG   = (1 << GL_NEED_TIME_STAT),
    /* ... */

    /* volatile flags */
    GL_HAVE_MEAS_STAT_FLG   = (1 << GL_HAVE_MEAS_STAT),
    GL_HAVE_FIX_STAT_FLG    = (1 << GL_HAVE_FIX_STAT),
    GL_SUPL_END_FLG         = (1 << GL_SUPL_END),
    GL_FIX_IN_PROGRESS_FLG  = (1 << GL_FIX_IN_PROGRESS),
    GL_PRECISE_TIME_FLG     = (1 << GL_PRECISE_TIME),
    GL_SYNC_STATUS_CHANGED_FLG  = (1 << GL_SYNC_STATUS_CHANGED),
    /* ... */

    /* Power Mode Status flags */
    GL_POWER_UP_FLG         = (0 << 14),
    GL_STANDBY_FLG          = (1 << 14),
    GL_LOWPWR_STANDBY_FLG   = (2 << 14),
    GL_PWR_DOWN_FLG         = (3 << 14),
    GL_POWER_STAT_MASK      = GL_PWR_DOWN_FLG
                              | GL_STANDBY_FLG
                              | GL_LOWPWR_STANDBY_FLG,
    /* ... */

    GL_NEED_AID_STAT_MASK   = GL_NEED_ACQ_STAT_FLG
                              | GL_NEED_POS_STAT_FLG
                              | GL_NEED_NAV_STAT_FLG
                              | GL_NEED_TIME_STAT_FLG
};
/*</GLML>*/

/*******************************************************************************
* Simulation modes for glDoSim()
*******************************************************************************/
/* <GLML GlGpsApi.h SIM_TYPE_Type>*/
enum SIM_TYPE
{
    SIM_NONE,        /* no simulation - default */
    SIM_FRQ_TEST,    /* GLL returns M3 continuous dump */
    SIM_SNR_TEST,    /* GLL returns SNR and CN0: factory test */
    SIM_TOW_TEST,    /* GLL returns TOW */
    SIM_WER_TEST     /* GLL returns Word Error Rate: clock phase noise test */
};
typedef enum SIM_TYPE SIM_TYPE;
/*</GLML>*/

/*******************************************************************************
* Platform test modes for glStartPlatformTest()
*******************************************************************************/
/* <GLML GlGpsApi.h GlPlatformTests_Type>*/
typedef enum GlPlatformTests 
{
    GL_TST_NONE,          /* 0-No test, or stop current test */
    GL_TST_VER,           /* 1-Minimal test, read version number only */
    GL_TST_R_LRG_PKT,     /* 2-Large packet, write once then read int RAM */
    GL_TST_WR_LRG_PKT,    /* 3-Large packet, write and read int RAM */
    GL_TST_R_JMBO_PKT,    /* 4-Max packet, write once then read all int RAM */
    GL_TST_WWRR_JMBO_PKT, /* 5-Max packet, write all then read all int RAM */
    GL_TST_WRWR_JMBO_PKT, /* 6-Max packet, write one read one on all int RAM */
    GL_TST_CPU_TIMER,     /* 7-Check CPU time vs ASIC time and serial latency */

    GL_TST_LAST
} GlPlatformTests;
/*</GLML>*/

#define GL_TST_INFINITE (0)

/*******************************************************************************
* Platform test status for glcb_OnPlatformTest()
*******************************************************************************/
/* <GLML GlGpsApi.h GlPlatformTestStatus_Type>*/
typedef enum GlPlatformTestStatus 
{
    GL_TST_ONGOING,       /* 0-Test still running successfully */
    GL_TST_ONGOING_FAIL,  /* 1-Test still running but already had failures */
    GL_TST_DONE_SUCCESS,  /* 2-Test done successfully */
    GL_TST_DONE_FAILED    /* 3-Test done with some failures */
} GlPlatformTestStatus;
/*</GLML>*/


/* <NO_GLML GlGpsApi.h GL_HANDLE_Type>*/
typedef long   GL_HANDLE;
/*</NO_GLML>*/

#define     GL_HANDLE_INVALID            (0L)
#define     GL_HANDLE_BUSY              (-1L)
#define     GL_HANDLE_INVALID_PARAMETER (-2L)

/* functions will return positive handle number for successful requests
 * and error code in other cases (values <= 0)
 */
 
/*******************************************************************************
* DESCRIPTION:  Initiate single shot positioning request
*
* INPUTS:   none
*
* RETURN:   GL_HANDLE - handle of created request
*           > 0 - valid handle, otherwise indicates error code
*
* ERRORS:   GL_HANDLE_BUSY - request can not be accepted at this time
*
* NOTES:    1. Positions will be reported over glcb_OnSingleShot()
*              callback function
*
*           2. Request can be stopped by glStopSingleShot()
*
*******************************************************************************/
/*<NO_GLML GlGpsApi.h glStartSingleShot_Proto>*/
GLGPS_API GL_HANDLE glStartSingleShot();
/*</NO_GLML>*/

/*******************************************************************************
* DESCRIPTION:  Initiate periodic positioning request
*
* INPUTS:   lPeriod - desired period of position fixing in milliseconds
*
* RETURN:   GL_HANDLE - handle of created request
*           > 0 - valid handle, otherwise indicates error code
*
* ERRORS:   GL_HANDLE_BUSY - request can not be accepted at this time
*           GL_HANDLE_INVALID_PARAMETER - lPeriod < 0
*
* NOTES:    1. Positions will be reported over glcb_OnPeriodicPosition()
*              callback function
*
*           2. Period of 0 indicates continuous position fixing at the native 
*              measurement rate of the receiver. Updates are produced at short 
*              intervals in strong signal conditions and at longer intervals in
*              weak signal conditions. 
*
*           3. Position fixing continues until a call to 
*              glStopPeriodicPosition().
*
*******************************************************************************/
/*<NO_GLML GlGpsApi.h glStartPeriodicPosition_Proto>*/
GLGPS_API GL_HANDLE glStartPeriodicPosition(long lPeriod);
/*</NO_GLML>*/

/*******************************************************************************
* DESCRIPTION:  Initiate LTO download request
*
* INPUTS:   none
*
* RETURN:   GL_HANDLE - handle of created request
*           > 0 - valid handle, otherwise indicates error code
*
* ERRORS:   GL_HANDLE_BUSY - request can not be accepted at this time
*
* NOTES:    1. Download progress will be reported over glcb_OnLtoDownload()
*              callback function
*
*           2. Request can be stopped by glStopLtoDownload()
*
*******************************************************************************/
/*<NO_GLML GlGpsApi.h glStartLtoDownload_Proto>*/
GLGPS_API GL_HANDLE glStartLtoDownload();
/*</NO_GLML>*/

/*******************************************************************************
* DESCRIPTION:  Initiate Test Mode request
*
* INPUTS:   lPeriod   - desired period of position fixing in milliseconds
*           etSimType - simulation type
*
* RETURN:   GL_HANDLE - handle of created request
*           > 0 - valid handle, otherwise indicates error code
*
* ERRORS:   GL_HANDLE_BUSY - request can not be accepted at this time
*
* NOTES:    1. Test results will be reported over glcb_OnTestMode()
*              callback function
*
*           2. Request can be stopped by glStopTestMode()
*
*******************************************************************************/
/*<NO_GLML GlGpsApi.h glStartTestMode_Proto>*/
GLGPS_API GL_HANDLE glStartTestMode(long lPeriod, SIM_TYPE etSimType);
/*</NO_GLML>*/

/*******************************************************************************
* DESCRIPTION:  Initiate Network Initiated request
*
* INPUTS:   pucBuffer - pointer to a buffer with binary coded 
*                       Network initiated message
*           iBytes    - Size of this message
*           ver       - HMAC of SUPL_INIT (described in SUPL 1.0 spec.)
*                       MUST be 8 bytes array. Refer to glSuplSetVer()
*                       for additional information
*
* RETURN:   GL_HANDLE - handle of created request
*           > 0 - valid handle, otherwise indicates error code
*
* ERRORS:   GL_HANDLE_BUSY - request can not be accepted at this time
*
* NOTES:    Parameter "ver" CAN NOT BE NULL.
*
*******************************************************************************/
/*<NO_GLML GlGpsApi.h glStartNetworkPosition_Proto>*/
GLGPS_API GL_HANDLE glStartNetworkPosition(const unsigned char *pucBuffer,
                                           int iBytes,
                                           const unsigned char *ver);
/*</NO_GLML>*/

/*******************************************************************************
* DESCRIPTION:  Determines the type of platform test to run and starts the test
*
* INPUTS:       enum GlPlatformTests: type of test to run
*               ulNbTest:  number of times the selected test should run
*                          GL_TST_INFINITE for the test to run until stopped
*                          through glStartPlatformTest(GL_TST_NONE)
*               ucVerbose: 0- only log errors and beginning and end of test
*                          1- log result of each test run done.
*
* RETURN:       GL_HANDLE - handle of created request
*               > 0 - valid handle, otherwise indicates error code
*
* NOTES:        If at all, this must be called after glOnStart() as it generates
*               events and, therefore, the state machines must be initialized.
*******************************************************************************/
/*<GLML GlGpsApi.h glStartPlatformTest_Proto>*/
GLGPS_API GL_HANDLE glStartPlatformTest(GlPlatformTests etPltfrmTestType,
                      unsigned long ulNbTestRun, unsigned char ucVerbose);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Stops all pending and running single shot requests
*
* INPUTS:   none
*
* RETURN:   GLRC    - error code
*
* ERRORS:   none
*
* NOTES:    none
*
*******************************************************************************/
/*<GLML GlGpsApi.h glStopSingleShot_Proto>*/
GLGPS_API GLRC glStopSingleShot();
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Stops all pending and running periodic requests
*
* INPUTS:   none
*
* RETURN:   GLRC    - error code
*
* ERRORS:   none
*
* NOTES:    none
*
*******************************************************************************/
/*<GLML GlGpsApi.h glStopPeriodicPosition_Proto>*/
GLGPS_API GLRC glStopPeriodicPosition();
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Stops all pending and running LTO Download requests
*
* INPUTS:   none
*
* RETURN:   GLRC    - error code
*
* ERRORS:   none
*
* NOTES:    none
*
*******************************************************************************/
/*<GLML GlGpsApi.h glStopLtoDownload_Proto>*/
GLGPS_API GLRC glStopLtoDownload();
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Stops all pending and running Test Mode requests
*
* INPUTS:   none
*
* RETURN:   GLRC    - error code
*
* ERRORS:   none
*
* NOTES:    none
*
*******************************************************************************/
/*<GLML GlGpsApi.h glStopTestMode_Proto>*/
GLGPS_API GLRC glStopTestMode();
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Check current status of GPS Engine
*
* INPUTS:   none
*
* RETURN:   GL_STAT - status
*
* ERRORS:   none
*
* NOTES:    none
*
*******************************************************************************/
/*<GLML GlGpsApi.h glCheckStatus_Proto>*/
GLGPS_API GL_STAT glCheckStatus();
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Ends all pending and running requests
*
* INPUTS:   none
*
* RETURN:   GLRC    - error code
*
* ERRORS:   none
*
* NOTES:    none
*
*******************************************************************************/
/*<GLML GlGpsApi.h glShutdown_Proto>*/
GLGPS_API GLRC glShutdown();
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION: Begin the position fix cycle
*
* INPUTS:  none
*
* RETURN:  none
*
* NOTES:   1. Calling this function is NOT required if the built-in support for 
*             RRLP or SUPL is used. The following notes pertain only to the case
*             when the built-in support for RRLP or SUPL is not used.
*
*          2. If one-shot position fix is requested (see description of 
*             glSetFixPeriod), the clients must call this function to initiate 
*             every position fix.  
*
*          3. If periodic position fixing is requested, the clients must call
*             this function only once to begin the cycle.
*******************************************************************************/
/*<GLML GlGpsApi.h glBeginPositionFix_Proto>*/
GLGPS_API void glBeginPositionFix(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: End position fix cycle (abort if necessary) and put the A-GPS 
*              chipset in low-power mode (typically low-power standby)
*
* INPUTS:  none
*
* RETURN:  returns GLRC_OK if successful
*
* NOTES:   This function can be used either to suspend an ongoing position fix
*          cycle (one-shot or periodic) or to achieve a graceful shutdown of
*          the GLL and the A-GPS chipset before exiting the application.
*          In the latter case, Clients must keep calling glOnEvent() until the
*          returned GLL status bitmask indicates that the GLL is stopped 
*          (the GL_STOPPED_STAT_FLG is set).
*******************************************************************************/
/*<GLML GlGpsApi.h glEndPositionFix_Proto>*/
GLGPS_API GLRC glEndPositionFix(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Sets the position fix period
*
* INPUTS:  lPeriod - desired period of position fixing in milliseconds
*
* RETURN:  returns GLRC_OK if setting the interval was successful
*
* NOTES:   1. Period of -1 indicates a one-shot position fix. Only one position
*             fix is attempted.
*          
*          2. Period of 0 indicates continuous position fixing at the native 
*             measurement rate of the receiver. Updates are produced at short 
*             intervals in strong signal conditions and at longer intervals in
*             weak signal conditions. Position fixing continues until a call to
*             glEndPositionFix().
*
*          3. Period set to a positive value indicates periodic position fixes
*             at a fixed update interval corresponding to the value given in 
*             milliseconds. Position fixing continues until a call to 
*             glEndPositionFix().
*******************************************************************************/
/*<GLML GlGpsApi.h glSetFixPeriod_Proto>*/
GLGPS_API GLRC glSetFixPeriod(long lPeriod);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: This function powers up the chipset and sends a series of 
*              initialization commands to the A-GPS chipset. At this time the
*              application should be prepared to receiving incoming bytes from
*              the chipset.
*
* INPUTS:  none
*
* RETURN:  returns GLRC_OK if successful
*
* NOTES:   This function must be called exactly once. 
*******************************************************************************/
/*<GLML GlGpsApi.h glOnStart_Proto>*/
GLGPS_API GLRC glOnStart(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: This function should be called to deliver the bytes received
*              from the A-GPS chipset to the GLL. Also if no bytes are received
*              from the chipset then this function should be called after a 
*              timeout period.
*
* INPUTS:  pucBuf -- pointer to the buffer containing the bytes (can be NULL
*          if no bytes are delivered, which is indicated by iNBytes == 0).
*               
*          iNBytes -- number of bytes received (zero if no bytes are 
*          available from the A-GPS chipset within the specified timeout)
*
* RETURN:  returns a bitmask representing the status of the GLL (See the
*          enumeration GlStatusFlags for definition of the flags)
*
* NOTES:   1. This function must be called only AFTER initializing the 
*             communication with the A-GPS chipset with glOnStart().
*
*          2. This function always processes all delivered bytes, so the data
*             buffer can be reused for other purposes after this function 
*             returns.
*
*          3. The data block doesn't need to be aligned at the A-GPS chipset
*             packet boundaries. It can contain packet fragments and/or multiple
*             such packets. 
* 
*          4. This function must be called frequently, even if no bytes are 
*             available from the A-GPS chipset. The reason for calling 
*             glOnEvent() frequently is that each call to glOnEvent() causes 
*             the library to refresh numerous internal timers. Among other 
*             tasks, these timers control the generation of periodic position 
*             fixes as well as navigation data decoding in the autonomous mode
*             of the library. The timers also allow the GLL to implement 
*             communication timeouts to handle situations in which return 
*             messages are expected from the A-GPS chipset but for some reason
*             do not arrive (e.g., data loss in serial communications).
*
*          5. The recommended timeout value is 100 milliseconds or less, but 
*             this value is not critical.  
*******************************************************************************/
/*<GLML GlGpsApi.h glOnEvent_Proto>*/
GLGPS_API GL_STAT glOnEvent(const unsigned char *pucBuf, int iNBytes);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION: This function returns the amount of time remaining until the
*              shortest internal GLL timer expires.  This allows the system to
*              schedule a maximal length sleep period for the GPS thread.
*
* INPUTS:  none
*               
* RETURN:  Number of milliseconds until the next GLL timer expires
*
*          - returns zero if an active timer already expired or there
*              is still work to do inside the GLL.
*
*          - returns GL_GLL_NOT_ACTIVE if there is no current or pending work
*              scheduled.
*
*
*******************************************************************************/
#define GL_GLL_NOT_ACTIVE   100000000
/*<GLML GlGpsApi.h glSleepDuration_Proto>*/
GLGPS_API unsigned long glSleepDuration(void);
/*</GLML>*/



/*******************************************************************************
* DESCRIPTION: This function starts the  Global Locate SUPL protocol stack.
* Additionally, the function sets the customized memory buffer
* the ASN.1 runtime. 
*
* INPUTS:  buffer  -- pointer to the buffer
*          size    -- size of the buffer. Recommended size is 3000 bytes.
*
* RETURN:  returns GLRC_OK if successful
*
* NOTES:   1. This API is for Clients who choose to use the built-in support 
*             for SUPL/RRLP.
*
*          2. Choosing the SUPL support automatically selects RRLP support.
*
*          3. Once the Client chooses to use the built-in support for 
*             SUPL/RRLP, the Client must repetitively call glSuplOnEvent()
*             to deliver SUPL messages to the GLL. 
*
*          4. If the Client doesn't choose to use the built-in support for
*             SUPL, by design the rest of the GLL doesn't refer to the 
*             SUPL engine and so no GL SUPL code will be linked into the
*             Client application.  
*
*******************************************************************************/

/*<GLML GlGpsApi.h glSuplOnStart_Proto>*/
GLGPS_API GLRC glSuplOnStart(unsigned char *buffer, unsigned int size);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:     called by the SUPL engine to receive Ver parameter.
*
* INPUTS:  ver -- [in] address to 8 bytes buffer with VER value
*
* <START QUOTE FROM SUPL 1.0>
* Proxy mode network verification of the integrity of the SUPL INIT 
* message is always performed by the H-SLP. The SUPL POS INIT message 
* MUST contain a verification field (VER), which is an HMAC of the complete 
* SUPL INIT message. When the H-SLP receives the SUPL POS INIT message it 
* MUST check the received VER field against the corresponding value 
* calculated over the transmitted SUPL INIT message. If this verification 
* fails the Home SLP MUST terminate the session with the SUPL END message 
* which contains status code ‘authSuplinitFailure? 
*
* HMAC for the verification field MUST be calculated as follows:
*
* VER=H(H-SLP XOR opad, H(H-SLP XOR ipad, SUPL INIT))
*
* where H-SLP is the FQDN of the H-SLP address configured in the SET. 
* Note that the H-SLP address is not considered secret. 
* The HMAC construct used here does not provide any data authentication 
* but is only used as an alternative to a HASH function. 
* SHA-1 MUST be used as the hash (H) function in the HMAC. 
* The output of the HMAC function MUST be truncated to 64 bits, 
* i.e., the HMAC MUST be implemented as HMAC-SHA1-64 [HMAC].
*
* <END QUOTE FROM SUPL 1.0>
*******************************************************************************/

/*<GLML GlGpsApi.h glSuplSetVer_Proto>*/
GLGPS_API void glSuplSetVer(const unsigned char* ver);
/*</GLML>*/

/*******************************************************************************
* this function is OBSOLETE and should not be used
*
* DESCRIPTION: Stop and cleanup the Global Locate SUPL protocol stack.
* 
* INPUTS:  none
*
* RETURN:  none
*
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplOnStop_Proto>*/
GLGPS_API void glSuplOnStop(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: This function should be called to deliver SUPL messages to GLL.
*
* INPUTS:  pucBuf -- pointer to the buffer containing the bytes
*               
*          iNBytes -- number of bytes received (may be zero)
*
* RETURN:  returns GLRC_OK if the data could be successfully decoded
*
* NOTES:   1. This function must be called only AFTER initializing the built-in 
*             SUPL protocol stack with glSuplOnStart().
*
*          2. This function assumes that only one complete SUPL message 
*             is passed to it at a time (e.g., payload of one UDP datagram).
*             In particular, the function cannot process fragments of SUPL
*             messages or multiple SUPL messages at a time.
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplOnEvent_Proto>*/
GLGPS_API GLRC glSuplOnEvent(const unsigned char *pucBuf, int iNBytes);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Sets default capabilities for SUPL GLL SUPL protocol stack
* 
* INPUTS:  none
*
* RETURN:  none
*
* NOTES:   This function can be called at any time.
*******************************************************************************/
/*<GLML GlGpsApi.h GlSuplSetCapabilityFlags_Type>*/
enum GlSuplSetCapabilityFlags
{
    GL_SUPL_MODE_SET_ASSISTED = (1 << 0),
    GL_SUPL_MODE_SET_BASED    = (1 << 1),
    GL_SUPL_MODE_AUTONOMOUS   = (1 << 2),
    GL_SUPL_MODE_ALL          = GL_SUPL_MODE_SET_ASSISTED 
                                | GL_SUPL_MODE_SET_BASED 
                                | GL_SUPL_MODE_AUTONOMOUS,
    GL_SUPL_MODE_DEFAULT      = GL_SUPL_MODE_ALL
};
/*</GLML>*/

/*<GLML GlGpsApi.h glSuplSetDefaultCapabilities_Proto>*/
GLGPS_API void glSuplSetDefaultCapabilities(int fSetCapability);
/*</GLML>*/

/*<GLML GlGpsApi.h glSuplGetDefaultCapabilities_Proto>*/
GLGPS_API int glSuplGetDefaultCapabilities();
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Allows to customize values for timers UT1, UT2, and UT3
* 
* INPUTS:  ut1 - UT1 timer in milliseconds, default value is 10000 ms
*          ut2 - UT2 timer in milliseconds, default value is 10000 ms
*          ut3 - UT3 timer in milliseconds, default value is 10000 ms
*
* RETURN:  none
*
* NOTES:   Setting timer value to 0 means that this timer should not be changed.
*          Must be called as a part of GLL initialization procedure.
*          Please refer to SUPL specification for more information
*
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplSetTimers_Proto>*/
GLGPS_API void glSuplSetTimers(unsigned long ut1,
                               unsigned long ut2,
                               unsigned long ut3);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Sets SUPL version
* 
* INPUTS:  version - SUPL version. Default value is
*          GL_OMA_TS_ULP_V1_0_20050719_C
*
* RETURN:  none
*
* NOTES:   Must be called as a part of GLL initialization procedure.
*
*******************************************************************************/
/*<NO_GLML GlGpsApi.h GlSuplProtocolVersion_Type>*/
typedef enum GLSuplProtocolVersion
{
    GL_OMA_TS_SUPL_V1_0_20050223_D,
    GL_OMA_TS_SUPL_V1_0_20050315_D,
    GL_PRESUPL_80_V8327_1NP_G,
    GL_OMA_TS_ULP_V1_0_20050719_C      /* also known as SUPL 1.0 */
} GLSuplProtocolVersion;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h glSuplSetVersion_Proto>*/
GLGPS_API void glSuplSetVersion(GLSuplProtocolVersion version);
/*</NO_GLML>*/

/*******************************************************************************
* DESCRIPTION: Inform Global Locate SUPL protocol stack that Supl connection 
*              have been established (bSuccess=1) or not (bSuccess=0)
* 
* INPUTS:  none
*
* RETURN:  none
*
* NOTES:   This function should be called after glcb_SuplReqConnection 
*          to inform SUPL protocol stack about result of this request
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplConnect_Proto>*/
GLGPS_API void glSuplConnect(int bSuccess);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Inform Global Locate SUPL protocol stack that Supl connection 
*              have been terminated
* 
* INPUTS:  none
*
* RETURN:  none
*
* NOTES:   This function should be called during active SUPL session 
*          to inform SUPL protocol stack that TCP/IP connection have been 
*          terminated for any reason
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplDisconnected_Proto>*/
GLGPS_API void glSuplDisconnected(void);
/*</GLML>*/

/*******************************************************************************
*
* DESCRIPTION: Sets the SUPL ID in the GL built-in SUPL protocol stack.
*
* INPUTS:  pSuplID -- pointer to structure defining SUPL ID
*
* OUTPUTS: returns GLRC_OK if successful
*
* NOTES:   This function should be called before glSuplOnStart()
*
*******************************************************************************/
/*<NO_GLML GlGpsApi.h GlSuplSetIDType_Type>*/
typedef enum GlSuplSetIDType
{
    GL_SUPLID_MSISDN_BCD,
    GL_SUPLID_MSISDN_ASC,
    GL_SUPLID_MDN,
    GL_SUPLID_MIN,
    GL_SUPLID_IMSI,
    GL_SUPLID_IPv4,
    GL_SUPLID_IPv6,
    GL_SUPLID_NAI
} GlSuplSetIDType;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSuplSetID_Type>*/
typedef struct GlSuplSetID GlSuplSetID;
struct GlSuplSetID
{
    GlSuplSetIDType     eType;      // ID Type
    union
    { 
        /* Mobile Subscriber ISDN Number 0 terminated Ascii string */
        char            msisdn_asc[17];

        /* Mobile Subscriber ISDN Number BCD format   */
        unsigned char   msisdn_bcd[8];

        unsigned char   mdn[8];

        /* Mobile ID number (first 34 bits are used) */
        unsigned char   min[5];

        /* International Mobile Subscriber Identity (IMSI = MCC + MNC + MSIN) */
        unsigned char   imsi[8];

        unsigned char   ipv4[4];    /* IP address v4 */
        unsigned char   ipv6[16];   /* IP address v6 */

        /*  nai must be a pointer to a null terminated string in 
         *  persistent memory 
         */
        const char      *nai;       
    } u;
};
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h glSetSuplSetID_Proto>*/
GLGPS_API GLRC glSetSuplSetID(const GlSuplSetID* pSuplSetID);
/*</NO_GLML>*/

/*******************************************************************************
* DESCRIPTION: Sets the SUPL periodic fix interval
*
* INPUTS:  lPeriod - desired interval between reports in seconds
*                    0  - every successful fix
*                    <0 - 
*
*          lNumber - number of desired reports, 
*                    -1 - unlimited
*                    0  - stop periodic fix
*
* RETURN:  returns GLRC_OK if setting the interval was successful
*
* NOTES:   
*
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplSetPeriodicReport_Proto>*/
GLGPS_API GLRC glSuplSetPeriodicReport(long lPeriod, long lNumber);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Respond to glcb_SuplNotificationVerificarionReq
*
* INPUTS:  bAllow -- 1 - allow positioning info, 0 - deny
*
* OUTPUTS: returns GLRC_OK if successful
*
* NOTES:   This function should be called as a respond to
*          glcb_SuplNotificationVerificarionReq
*
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplVerificationRsp_Proto>*/
GLGPS_API void glSuplVerificationRsp(int bAllow);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Indicate if SUPL protocol stack can accept next 
*              SET initiated request
*
* INPUTS:  
*
* OUTPUTS: 1 - ready, 0 - do not ready
*
* NOTES:   
*
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplIsReadyForRequest_Proto>*/
GLGPS_API int glSuplIsReadyForRequest();
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Indicate if SUPL protocol stack can accept next 
*              Network initiated request
*
* INPUTS:  
*
* OUTPUTS: 1 - ready, 0 - do not ready
*
* NOTES:   
*
*******************************************************************************/
/*<GLML GlGpsApi.h glSuplIsReadyForSuplInit_Proto>*/
GLGPS_API int glSuplIsReadyForSuplInit();
/*</GLML>*/

/*******************************************************************************
*
* DESCRIPTION: Set flag for sending last calculated position on RRLP
*              session timeout
*
* INPUTS:  bAllow -- 1 - send, 0 - send timeout error
*
* NOTES:   This function sets flag for sending last calculated position on 
*          RRLP session timeout
*
*******************************************************************************/
/*<GLML GlGpsApi.h glRrlpSendOldPosOnTimeout_Proto>*/
GLGPS_API void glRrlpSendOldPosOnTimeout(int bAllow);
/*</GLML>*/

/*******************************************************************************
*
* DESCRIPTION: Sets the Location Information for the GLL SUPL protocol stack
*              Structures defined according to OMA-TS-SUPL-V1_0-20050223-D
*
* INPUTS:  pLocationId - pointer to a location information structure.
*
* OUTPUTS: returns GLRC_OK if successful
*
* NOTES:   1. If the location information data is available right away, this
*             function can be called from the callback glcb_SuplReqLocationId().
*
*          2. The location information is requested periodically to cover
*             the case of a roaming mobile station.
*
*******************************************************************************/

/*<NO_GLML GlGpsApi.h GlSupl_NMRelement_Type>*/
typedef struct GlSupl_NMRelement
{
   unsigned short  aRFCN;
   unsigned char   bSIC;
   unsigned char   rxLev;
} GlSupl_NMRelement;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_NMR_Type>*/
typedef struct GlSupl_NMR
{
   /* number of elements in <elem> range must be 1..15 */
   unsigned int      n;
   GlSupl_NMRelement elem[15];
} GlSupl_NMR;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_GsmCellInformation_Type>*/
typedef struct GlSupl_GsmCellInformation
{
    struct {
        unsigned nMRPresent;
        unsigned tAPresent;
    } m;
    unsigned short refMCC;
    unsigned short refMNC;
    unsigned short refLAC;
    unsigned short refCI;
    GlSupl_NMR     nMR;
    unsigned char  tA;
} GlSupl_GsmCellInformation;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_FrequencyInfoFDD_Type>*/
typedef struct GlSupl_FrequencyInfoFDD
{
    struct {
        unsigned uarfcn_ULPresent;
    } m;
    unsigned short  uarfcn_UL;
    unsigned short  uarfcn_DL;
} GlSupl_FrequencyInfoFDD;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_FrequencyInfoTDD_Type>*/
typedef struct GlSupl_FrequencyInfoTDD
{
    unsigned short  uarfcn_Nt;
} GlSupl_FrequencyInfoTDD;
/*</NO_GLML>*/

/* <NO_GLML GlGpsApi.h GlSupl_FrequencyInfoType_Type>*/
typedef enum GlSupl_FrequencyInfoType
{
    GL_SUPL_FRQ_FDD = 1,            /* FDD Frequency information */
    GL_SUPL_FRQ_TDD = 2             /* TDD Frequency information */
} GlSupl_FrequencyInfoType;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_FrequencyInfo_modeSpecificInfo_Type>*/
typedef struct GlSupl_FrequencyInfo_modeSpecificInfo
{
    GlSupl_FrequencyInfoType t;
    union 
    {
        GlSupl_FrequencyInfoFDD fdd;
        GlSupl_FrequencyInfoTDD tdd;
    } u;
} GlSupl_FrequencyInfo_modeSpecificInfo;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl-FrequencyInfo_Type>*/
typedef struct GlSupl_FrequencyInfo
{
   GlSupl_FrequencyInfo_modeSpecificInfo modeSpecificInfo;
} GlSupl_FrequencyInfo;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_TimeslotISCP_List_Type>*/
typedef struct GlSupl_TimeslotISCP_List {
   unsigned char n;
   unsigned char elem[14];
} GlSupl_TimeslotISCP_List;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_CellMeasuredResults_modeSpecificInfo_tdd_Type>*/
typedef struct GlSupl_CellMeasuredResults_modeSpecificInfo_tdd {
   struct {
      unsigned proposedTGSNPresent : 1;
      unsigned primaryCCPCH_RSCPPresent : 1;
      unsigned pathlossPresent : 1;
      unsigned timeslotISCP_ListPresent : 1;
   } m;
   unsigned char cellParametersID;
   unsigned char proposedTGSN;
   unsigned char primaryCCPCH_RSCP;
   unsigned char pathloss;
   GlSupl_TimeslotISCP_List timeslotISCP_List;
} GlSupl_CellMeasuredResults_modeSpecificInfo_tdd;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_PrimaryCPICH_Info_Type>*/
typedef struct GlSupl_PrimaryCPICH_Info {
   unsigned short primaryScramblingCode;
} GlSupl_PrimaryCPICH_Info;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_CellMeasuredResults_modeSpecificInfo_fdd_Type>*/
typedef struct GlSupl_CellMeasuredResults_modeSpecificInfo_fdd {
   struct {
      unsigned cpich_Ec_N0Present : 1;
      unsigned cpich_RSCPPresent : 1;
      unsigned pathlossPresent : 1;
   } m;
   GlSupl_PrimaryCPICH_Info primaryCPICH_Info;
   unsigned char cpich_Ec_N0;
   unsigned char cpich_RSCP;
   unsigned char pathloss;
} GlSupl_CellMeasuredResults_modeSpecificInfo_fdd;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_CellMeasuredResults_modeSpecificInfo_Type>*/
typedef struct GlSupl_CellMeasuredResults_modeSpecificInfo {
   int t;
   union {
      /* t = 1 */
      GlSupl_CellMeasuredResults_modeSpecificInfo_fdd fdd;
      /* t = 2 */
      GlSupl_CellMeasuredResults_modeSpecificInfo_tdd tdd;
   } u;
} GlSupl_CellMeasuredResults_modeSpecificInfo;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_CellMeasuredResults_Type>*/
typedef struct GlSupl_CellMeasuredResults {
   struct {
      unsigned cellIdentityPresent;
   } m;
   unsigned long cellIdentity;
   GlSupl_CellMeasuredResults_modeSpecificInfo modeSpecificInfo;
} GlSupl_CellMeasuredResults;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_CellMeasuredResultsList_Type>*/
typedef struct GlSupl_CellMeasuredResultsList {
   unsigned short n;
   GlSupl_CellMeasuredResults elem[32];
} GlSupl_CellMeasuredResultsList;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_MeasuredResults_Type>*/
typedef struct GlSupl_MeasuredResults {
   struct {
      unsigned frequencyInfoPresent;
      unsigned utra_CarrierRSSIPresent;
      unsigned cellMeasuredResultsListPresent;
   } m;
   GlSupl_FrequencyInfo frequencyInfo;
   unsigned char utra_CarrierRSSI;
   GlSupl_CellMeasuredResultsList cellMeasuredResultsList;
} GlSupl_MeasuredResults;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_MeasuredResultsList_Type>*/
typedef struct GlSupl_MeasuredResultsList {
   unsigned long n;
   GlSupl_MeasuredResults elem[8];
} GlSupl_MeasuredResultsList;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_WcdmaCellInformation_Type>*/
typedef struct GlSupl_WcdmaCellInformation
{
    struct {
        unsigned frequencyInfoPresent;
        unsigned primaryScramblingCodePresent;
        unsigned measuredResultsListPresent;
    } m;
    unsigned short  refMCC;
    unsigned short  refMNC;
    unsigned long   refCI;
    GlSupl_FrequencyInfo frequencyInfo;
    unsigned short  primaryScramblingCode;
    GlSupl_MeasuredResultsList measuredResultsList;
} GlSupl_WcdmaCellInformation;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSuplCellInfoCDMA_Type>*/
typedef struct GlSuplCellInfoCDMA
{
    int MCC;
    int MNC;
    int LAC;
    int CI;
} GlSuplCellInfoCDMA;
/*</NO_GLML>*/

/* <NO_GLML GlGpsApi.h GlSuplCellInfoType_Type>*/
typedef enum GlSuplCellInfoType
{
    GL_SUPL_CELL_INFO_NOT_AVAILABLE = 0,    /* Cell info not available  */ 
    GL_SUPL_CELL_INFO_GSM           = 1,    /* GSM Cell information     */
    GL_SUPL_CELL_INFO_WCDMA         = 2,    /* WCDMA Cell information   */
    GL_SUPL_CELL_INFO_CDMA          = 3     /* CDMA Cell information    */
} GlSuplCellInfoType;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_CdmaCellInformation_Type>*/
typedef struct GlSupl_CdmaCellInformation {
   unsigned short  refNID;
   unsigned short  refSID;
   unsigned short  refBASEID;
   unsigned int    refBASELAT;
   unsigned int    reBASELONG;
   unsigned short  refREFPN;
   unsigned short  refWeekNumber;
   unsigned int    refSeconds;
} GlSupl_CdmaCellInformation;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_CellInfo_Type>*/
typedef struct GlSupl_CellInfo
{
    GlSuplCellInfoType    t;        /* Cell Information type */
    union
    {
        GlSupl_GsmCellInformation    gsmCell;
        GlSupl_WcdmaCellInformation    wcdmaCell;
        GlSupl_CdmaCellInformation  cdmaCell;
    } u;
} GlSupl_CellInfo;

/*<NO_GLML GlGpsApi.h GlSupl_Status_Type>*/
typedef enum GlSupl_Status 
{
   GL_SUPL_STATUS_STALE   = 0,
   GL_SUPL_STATUS_CURRENT = 1,
   GL_SUPL_STATUS_UNKNOWN = 2
} GlSupl_Status;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSupl_LocationId_Type>*/
typedef struct GlSupl_LocationId {
   GlSupl_CellInfo cellInfo;
   GlSupl_Status status;
} GlSupl_LocationId;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h glSuplSetLocationId_Proto>*/
GLGPS_API GLRC glSuplSetLocationId(GlSupl_LocationId* pLocationId);
/*</NO_GLML>*/

/*******************************************************************************
*
* DESCRIPTION: Sets "warm state" flag for PreSUPL
*
* INPUTS:  bEnable - 1 - enable, 0 - disable
*
* OUTPUTS: none
*
* NOTES:   
*                   Warm state will update assistance information to keep
*                   the state warm according to the criteria based on the
*                   handset's movement: Cell ID changes and the GPS
*                   satellites' movement;
*                       (a) each time Cell ID changes, reference position
*                           will be updated but no more than once every 10
*                           minutes (non-update timer),
*                       (b) ephemeris and reference time will be updated
*                           every 30 minutes (expiration timer).
* 
*******************************************************************************/
/*<NO_GLML GlGpsApi.h glPreSUPLSetWarm_Proto>*/
GLGPS_API void glPreSUPLSetWarm(int bEnable);
/*</NO_GLML>*/


/*******************************************************************************
* DESCRIPTION: This function starts the  Global Locate RRLP protocol stack.
* Additionally, the function sets the memory block the ASN.1 runtime. 
*
* INPUTS:  buffer  -- pointer to the buffer
*          size    -- size of the buffer. Recommended size is 3000 bytes.
*
* NOTES:   1. This API is for Clients who choose to use the built-in support 
*             for RRLP (but not for SUPL).
*
*          2. Once the Client chooses to use the built-in support for 
*             RRLP, the Client must repetitively call glRrlpOnEvent()
*             to deliver RRLP messages to the GLL. 
*
*          3. If the Client doesn't choose to use the built-in support for
*             RRLP, by design the rest of the GLL doesn't refer to the 
*             RRLP engine and so no GL RRLP code will be linked into the
*             Client application.  
*******************************************************************************/
/*<GLML GlGpsApi.h glRrlpOnStart_Proto2>*/
GLGPS_API GLRC glRrlpOnStart(unsigned char *buffer, unsigned int size);
/*</GLML>*/

/*******************************************************************************
* this function is OBSOLETE and should not be used
*
* DESCRIPTION: Stop and cleanup the Global Locate RRLP protocol stack.
* 
* INPUTS:  none
*
* RETURN:  none
*
* NOTES:   This API should be only used if the built-in RRLP support had been
*          already selected with glRrlpOnStart().
*******************************************************************************/
/*<GLML GlGpsApi.h glRrlpOnStop_Proto>*/
GLGPS_API void glRrlpOnStop(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: This function should be called to deliver RRLP messages to GLL.
*
* INPUTS:  pucBuf -- pointer to the buffer containing the bytes
*               
*          iNBytes -- number of bytes received (may be zero)
*
* RETURN:  returns GLRC_OK if the data could be successfully decoded
*
* NOTES:   1. This function must be called only AFTER initializing the built-in 
*             RRLP protocol stack with glRrlpOnStart().
*
*          2. This function assumes that only one complete RRLP message 
*             is passed to it at a time (e.g., payload of one UDP datagram).
*             In particular, the function cannot process fragments of RRLP
*             messages or multiple RRLP messages at a time.
*******************************************************************************/
/*<GLML GlGpsApi.h glRrlpOnEvent_Proto>*/
GLGPS_API GLRC glRrlpOnEvent(const unsigned char *pucBuf, int iNBytes);
/*</GLML>*/

/*******************************************************************************
* GPIO IDs
*******************************************************************************/
/*<GLML Gl_PIO.h>*/
enum GLPIOID
{
    GLPIO_CFG_TYPE0,  /* (I) Reserved - Operational config ID (Bit 0) */
    GLPIO_CFG_TYPE1,  /* (I) Reserved - Operational config ID (Bit 1) */
    GLPIO_IF_TYPE0,   /* (I) Reserved - RF/IF subsystem config ID (Bit 0) */
    GLPIO_IF_TYPE1,   /* (I) Reserved - RF/IF subsystem config ID (Bit 1) */
    GLPIO_ASIC_RESET, /* (O) GL-ASIC pin nRESET  - powers up/down  */
    GLPIO_ASIC_STDBY, /* (O) GL-ASIC pin nSTANDBY-puts ASIC in/out of Standby */
    GLPIO_ASIC_CORE,  /* (O) gate power to GL-ASIC pin VDDCORE18 and 
                       * pin VDDPLL18 high: power applied, low: power removed
                       */
    /* NOTE: The GL-LN22 interface below can be controlled directly by the
     * ----  GL-20000 ASIC (recommended implementation). The following GPIOs
     *       only need to be implemented if the GL_LN22 pins are connected to
     *       some CPU GPIO pins instead, and not to the GL-20000. */
    GLPIO_PLL_CLK,    /* (O) GL-LN22 pin SCLK  - clk of 3-wire serial I/O */
    GLPIO_PLL_DATA,   /* (O) GL-LN22 pin SDATA - data of 3-wire serial I/O */
    GLPIO_PLL_LE,     /* (O) GL-LN22 pin LOAD - load enable of 3-wire I/O */
    GLPIO_PLL_LOCK,   /* (I) Not used with LN22 - RF PLL indicator (act LOW) */
    GLPIO_PLL_CHIP_ON,/* (O) GL-LN22 pin CHIP_ON - hold low to power down */
    _NUMOF(GLPIOID)  
};
/*</GLML>*/

/*******************************************************************************
* GL library positioning modes for glSetFixMode()
*******************************************************************************/
/* <GLML GlGpsApi.h GL_FIXMODE_Type>*/
enum GL_FIXMODE
{
    GL_FIXMODE_MS_ASSIST,  /* MS-A: range and doppler measurements */
    GL_FIXMODE_MS_BASED,   /* MS-B: range, doppler, position, velocity */
    GL_FIXMODE_AUTONOMOUS  /* AUTO: range, doppler, position, velocity */
};
typedef enum GL_FIXMODE GL_FIXMODE;
/*</GLML>*/


/*******************************************************************************
* Shape types supported by GL
*******************************************************************************/
/* <GLML GlGpsApi.h GL_SHAPE_TYPE_Type>*/
enum GL_SHAPE_TYPE
{
    GL_SHAPE_ELLIPSOID_POINT                           = 0,
    GL_SHAPE_ELLIPSOID_POINT_UNCRTNTY_CIRCLE           = 1,
    GL_SHAPE_ELLIPSOID_POINT_UNCRTNTY_ELLIPSE          = 3,
    GL_SHAPE_ELLIPSOID_POINT_WITH_ALT                  = 8,
    GL_SHAPE_ELLIPSOID_POINT_WITH_ALT_UNCRTNTY_ELLIPSE = 9 
};
typedef enum GL_SHAPE_TYPE GL_SHAPE_TYPE;
/*</GLML>*/

/*******************************************************************************
*    Packet definitions for communication with GL-ASICs
*******************************************************************************/
/* Special Packet bytes */
#define GL_CMD_FIXED   0xFF   /* Start of Packet for fixed size command */
#define GL_CMD_VAR     0xFE   /* Start of Packet for variable size command */
#define GL_CMD_EOH     0xFD   /* End-Of-Header */
#define GL_CMD_EOP     0xFC   /* End-Of-Packet */

#define GL_COMM_MAX_IN_PAYLOAD 35
/* NOTE: Actual max is 7, but use 20 for corrVec */
#define MAX_GL_COMM_PACKET  ((GL_COMM_MAX_IN_PAYLOAD + 2) *4)
/* NOTE: Maximum size of the packet GL uses through the serial link */
/* In operational mode, the maximum packet size is small */
/* 80 max payload + 2 bytes Packet header + 2 bytes payload header */
/* + 2 byte end-of-header and end-of-packet */

/*******************************************************************************
* Bus type information for glSetBusType()
*******************************************************************************/
/*<GLML GlGpsApi.h BUS_TYPE_Type>*/

/* The legacy apps may use the numbers below directly, so we try to keep them */
/* although this practice must be discouraged                                 */
enum BUS_TYPE
{
    BUS_UART     = 1,  /* UART-based serial interface */
    BUS_PARALLEL = 3,  /* PARALLEL port */
    BUS_SPI      = 4,  /* SPI bus */
    BUS_I2C      = 5   /* I2C bus */
};
typedef enum BUS_TYPE BUS_TYPE;
/*</GLML>*/

/*******************************************************************************
* Low Duty Cycle selection for glSetLowDutyCycle()
*    DUTYCYCLE_DISABLE = Use normal duty cycle,  
*    DUTYCYCLE_AUTO    = GLL determines when to enable low duty cycle
*    DUTYCYCLE_ENABLE  = Use low duty cycle (overrides GLL selection)
*******************************************************************************/
enum GL_LOW_DUTYCYCLE
{
    DUTYCYCLE_DISABLE = 0,  
    DUTYCYCLE_AUTO    = 1,
    DUTYCYCLE_ENABLE  = 2
};

typedef enum GL_LOW_DUTYCYCLE GL_LOW_DUTYCYCLE;

/*******************************************************************************
* Chipset Frequency Plan information for glSFrqPln()
*******************************************************************************/
/*<GLML GlGpsApi.h SysLog_Level_Type>*/

/* Ref Clock group */
/*        50  ppb */
/*        100 ppb */
/*        300 ppb */

/* TCXO  group */


/* CNTIN  group */

enum GL_FREQ_PLAN
{
/*--------------------------------------------------------------*/
/* Ref Clock group */
/*--------------------------------------------------------------*/
/* 25 PPB uncertainty  */
    FRQ_PLAN_10MHZ_25PPB,       /* 3GPP 10MHz Ref +/- 25PPB  */
    FRQ_PLAN_10MHZ_25PPB_TRK,   /* 3GPP 10MHz Ref +/- 25PPB  Frq Tracking HSS */
    FRQ_PLAN_13MHZ_25PPB,       /* 3GPP 13MHz Ref +/- 25PPB  */
    FRQ_PLAN_13MHZ_25PPB_TRK,   /* 3GPP 13MHz Ref +/- 25PPB  Frq Tracking HSS */
    FRQ_PLAN_40MHZ_25PPB,       /* 40MHz Ref +/- 25PPB  */

/* 50 PPB uncertainty  */
    FRQ_PLAN_10MHZ_50PPB,       /* 3GPP 10MHz Ref +/- 50PPB  */
    FRQ_PLAN_13MHZ_50PPB,       /* 3GPP 13MHz Ref +/- 50PPB  */
    FRQ_PLAN_16800_50PPB_TRK,   /* 16.8MHz Ref +/-    50PPB  */
    FRQ_PLAN_26MHZ_50PPB,        /* 26MHz Ref +/-     50PPB */

/* 100 PPB uncertainty  */
    FRQ_PLAN_10MHZ_100PPB,      /* 3GPP 10MHz Ref +/- 100PPB */
    FRQ_PLAN_13MHZ_100PPB,      /* 3GPP 13MHz Ref +/- 100PPB */
    FRQ_PLAN_16800_100PPB_TRK,  /* 16.8MHz Ref +/- 100PPB */
    FRQ_PLAN_26MHZ_100PPB,      /* 26MHz Ref +/- 100PPB */

/* 200 PPB uncertainty  */
    FRQ_PLAN_10MHZ_200PPB_TRK,  /* 3GPP 10MHz Ref +/- 300PPB Frq Tracking HSS */

/* 300 PPB uncertainty  */
    FRQ_PLAN_10MHZ_300PPB,      /* 3GPP 10MHz Ref +/- 300PPB */
    FRQ_PLAN_10MHZ_300PPB_TRK,  /* 3GPP 10MHz Ref +/- 300PPB Frq Tracking HSS */
    FRQ_PLAN_13MHZ_300PPB,      /* 3GPP 13MHz Ref +/- 300PPB */
    FRQ_PLAN_13MHZ_300PPB_TRK,  /* 3GPP 13MHz Ref +/- 300PPB Frq Tracking HSS */
    FRQ_PLAN_16800_300PPB_TRK,  /* 16.8MHz Ref +/- 300PPB */

/*--------------------------------------------------------------*/
/* TCXO  group                                                  */
/*--------------------------------------------------------------*/
/* 500 PPB uncertainty  */
    FRQ_PLAN_10MHZ_500PPB_TRK,  /* TCXO  +- 0.5 ppm Frq Tracking HSS DCD*/
    FRQ_PLAN_10MHZ_500PPB,      /* TCXO  +- 0.5 ppm */
    FRQ_PLAN_13MHZ_500PPB,      /* 13MHz TCXO Ref +/- 0.5 ppm */
    FRQ_PLAN_16_8MHZ_500PPB,    /* 16.8MHz TCXO Ref +/- 0.5 ppm */
    FRQ_PLAN_26MHZ_500PPB,      /* 26MHz TCXO Ref +/- 0.5 ppm */

/* 2000 PPB uncertainty  */
    FRQ_PLAN_13MHZ_2PPM,        /* 13MHz TCXO Ref +/- 2.0 ppm */
    FRQ_PLAN_16_8MHZ_2PPM,      /* 16.8MHz TCXO Ref +/- 2.0 ppm */
    FRQ_PLAN_26MHZ_2PPM,        /* 26MHz TCXO Ref +/- 2.0 ppm */

/* 2500 PPB uncertainty  */
    FRQ_PLAN_10MHZ_2500PPB_TRK,   /* 10MHZ TCXO +- 2.5 ppm Frq Tracking DCD*/
    FRQ_PLAN_16_8MHZ_2500PPB_TRK, /* 16.8MHz TCXO  +- 2.5 ppm Frq Tracking DCD*/

/*--------------------------------------------------------------*/
/* CNTIN  group                                                 */
/*--------------------------------------------------------------*/
/* 50 PPB uncertainty  */
/* 10 MHz TCXO */
    FRQ_PLAN_10MHZ_2PPM_10MHZ_50PPB,
                        /* 10MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 50 ppb  */
    FRQ_PLAN_10MHZ_2PPM_13MHZ_50PPB,
                        /* 10MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 50 ppb  */
    FRQ_PLAN_10MHZ_2PPM_26MHZ_50PPB,

/* 13 MHz TCXO */
    FRQ_PLAN_13MHZ_2PPM_10MHZ_50PPB,
                        /* 13MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 50 ppb  */
    FRQ_PLAN_13MHZ_2PPM_13MHZ_50PPB,
                        /* 13MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 50 ppb  */
    FRQ_PLAN_13MHZ_2PPM_26MHZ_50PPB,
                        /* 13MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 50 ppb  */

/* 16.8 MHz TCXO */
    FRQ_PLAN_16800_2PPM_10MHZ_50PPB,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 50 ppb  */
    FRQ_PLAN_16800_2PPM_10MHZ_50PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 50 ppb  */
    FRQ_PLAN_16800_2PPM_13MHZ_50PPB,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 50 ppb  */
    FRQ_PLAN_16800_2PPM_13MHZ_50PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 50 ppb  */
    FRQ_PLAN_16800_2PPM_13MHZ_50PPB_CALON,
                        /* same as FRQ_PLAN_16800_2PPM_13MHZ_50PPB
                         * except calibration is added.
                         */
    FRQ_PLAN_16800_2PPM_26MHZ_50PPB,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 50 ppb  */
                        /* 10MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 50 ppb  */

/* 19.2 MHz TCXO */
    FRQ_PLAN_19200_2PPM_26MHZ_50PPB,
                        /* 19.2MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 50 ppb  */

/* 20.0 MHz TCXO */
    FRQ_PLAN_20000_2PPM_13MHZ_50PPB,
                        /* 20.0MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 50 ppb  */
    FRQ_PLAN_20000_2PPM_13MHZ_50PPB_TRK,
                        /* 20.0MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 50 ppb  */

/* 26.0 MHz TCXO */
    FRQ_PLAN_26MHZ_2PPM_10MHZ_50PPB,
                        /* 26MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 50 ppb  */
    FRQ_PLAN_26MHZ_2PPM_13MHZ_50PPB,
                        /* 26MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 50 ppb  */
    FRQ_PLAN_26MHZ_2PPM_26MHZ_50PPB,
                        /* 26MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 50 ppb  */

/* 27.456 MHz TCXO */
    FRQ_PLAN_27456_2PPM_26MHZ_50PPB,
                        /* 27.456MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 50 ppb  */

/* 33.6 MHz TCXO */
    FRQ_PLAN_33600_2PPM_26MHZ_50PPB,
                        /* 33.6MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 50 ppb  */

/* 100 PPB uncertainty  */
/* 16.8 MHz TCXO */
    FRQ_PLAN_16800_2PPM_10MHZ_100PPB,
    FRQ_PLAN_16800_2PPM_10MHZ_100PPB_TRK,
    FRQ_PLAN_16800_2PPM_26MHZ_100PPB_TRK,

/* 19.2 MHz TCXO */
    FRQ_PLAN_19200_2PPM_26MHZ_100PPB,
                        /* 19.2MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 100 ppb  */
/* 27.456 MHz TCXO */
    FRQ_PLAN_27456_2PPM_26MHZ_100PPB,
                        /* 27.456MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 100 ppb  */
/* 33.6 MHz TCXO */
    FRQ_PLAN_33600_2PPM_26MHZ_100PPB,
                        /* 33.6MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 100 ppb  */

/* 150 PPB uncertainty  */
/* 10.0 MHz TCXO */
    FRQ_PLAN_10MHZ_2PPM_10MHZ_150PPB_TRK,


/* 200 PPB uncertainty  */
/* 16.8 MHz TCXO */
    FRQ_PLAN_16800_2PPM_10MHZ_200PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 200 ppb  */
    FRQ_PLAN_16800_2PPM_13MHZ_200PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 200 ppb  */
    FRQ_PLAN_16800_2PPM_26MHZ_200PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 200 ppb  */

/* 300 PPB uncertainty  */
/* 10 MHz TCXO */
    FRQ_PLAN_10MHZ_2PPM_10MHZ_300PPB,
                        /* 10MHz Ref +/- 2.0 ppm,  CNTIN 10 MHz,  300 ppb  */
/* 16.8 MHz TCXO */
    FRQ_PLAN_16800_2PPM_10MHZ_300PPB,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 300 ppb  */
    FRQ_PLAN_16800_2PPM_10MHZ_300PPB_TRK,

    FRQ_PLAN_16800_2PPM_13MHZ_300PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 13 MHz, 300 ppb  */
    FRQ_PLAN_16800_2PPM_26MHZ_300PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 26 MHz, 300 ppb  */

/* 400 PPB uncertainty  */
/* 16.8 MHz TCXO */
    FRQ_PLAN_16800_2PPM_10MHZ_400PPB_TRK,
                        /* 16.8MHz Ref +/- 2.0 ppm, CNTIN 10 MHz, 400 ppb  */


    /* YOUR PLAN GOES HERE    */
    /* CONTACT GL FOR DETAILS */

/*--------------------------------------------------------------*/
/* Specialty plans                                             */
/*--------------------------------------------------------------*/
    FRQ_PLAN_HP_20000,          /* HP Signal generator */
    FRQ_PLAN_OCXO_10000,        /* OCXO / RB clk @ 10MHz     */
/*--------------------------------------------------------------*/
/* Deprecated                                                   */
/*--------------------------------------------------------------*/
    FRQ_PLAN_002,
    FRQ_PLAN_005,
    FRQ_PLAN_010,
    FRQ_PLAN_030,
    FRQ_PLAN_GSM_R26B13_TRK,    /* GSM 2 Frq Tracking HSS DCD*/
    FRQ_PLAN_GSM_R26B26_TRK,    /* GSM 3 Frq Tracking HSS DCD*/
    FRQ_PLAN_THUND_CDMA,        /* CDMA */
    FRQ_PLAN_AUTO_GSM,          /* GSM 1 */
    FRQ_PLAN_AUTO_IDEN,         /* IDEN 1 */
    FRQ_PLAN_AUTO_IDEN_AIDED,   /* IDEN 2 */
    FRQ_PLAN_AUTO_PDC,          /* PDC 1 */
    FRQ_PLAN_PDC_10000,         /* PDC 2 */
    FRQ_PLAN_PDC_10000_NOTIFIED,/* PDC 3 */
    FRQ_PLAN_PDC_10000_AIDED,   /* PDC 4 */
    FRQ_PLAN_PDC_12600,         /* PDC 5  */
    FRQ_PLAN_PDC_12600_NOTIFIED,/* PDC 6 */
    FRQ_PLAN_PDC_12600_AIDED,   /* PDC 7 */
    FRQ_PLAN_GSM_R26B13,        /* GSM 2 */
    FRQ_PLAN_GSM_R26B26,        /* GSM 3 */
    FRQ_PLAN_GSM_R13B13,        /* GSM 4 */

    FRQ_PLAN_13000_500PPB_13MHZ_100PPB_TRK,
                        /* 13.0MHz Ref +/- 0.5 ppm, CNTIN 13MHz, 100 ppb */ 
    FRQ_PLAN_13000_500PPB_26MHZ_100PPB_TRK,
                        /* 13.0MHz Ref +/- 0.5 ppm, CNTIN 26MHz, 100 ppb */ 
    FRQ_PLAN_16800_500PPB_13MHZ_100PPB_TRK,
                        /* 16.8MHz Ref +/- 0.5 ppm, CNTIN 13MHz, 100 ppb */ 
    FRQ_PLAN_16800_500PPB_26MHZ_100PPB_TRK,
                        /* 16.8MHz Ref +/- 0.5 ppm, CNTIN 26MHz, 100 ppb */ 
    FRQ_PLAN_26000_500PPB_13MHZ_100PPB_TRK,
                        /* 26.0MHz Ref +/- 0.5 ppm, CNTIN 13MHz, 100 ppb */ 
    FRQ_PLAN_26000_500PPB_26MHZ_100PPB_TRK,
                        /* 26.0MHz Ref +/- 0.5 ppm, CNTIN 26MHz, 100 ppb */ 

    FRQ_PLAN_10MHZ_2500PPB,     /* TCXO  +- 2.5 ppm */

    /* End of deprecated */


    _NUMOF(GL_FREQ_PLAN)
};
typedef enum GL_FREQ_PLAN GL_FREQ_PLAN;
/*</GLML>*/

/*******************************************************************************
* RF chip type information for library
*******************************************************************************/
/*<GLML GlGpsApi.h RF_TYPE_Type>*/
enum RF_TYPE
{
   DEFAULT_RF_TYPE,     /* To set RF type to compilation default */
   RF_LEO         ,     /* Mitel-based RF board */
   RF_LN22OUT     ,     /* GL-LN22 RF chip with OSC_OUT enabled */
   RF_LN22FIB     ,     /* GL-LN22 RF chip */
   RF_HH_TC2L     ,     /* Test chip */
   RF_HH_TC3L     ,     /* Test chip */
   RF_HH_TC4L     ,     /* Test chip */
   RF_HH_2L       ,     /* Test chip */
   RF_HH_4L       ,     /* HammerHead with external LNA */
   RF_LN22EXT     ,     /* GL-LN22 RF orig. IF clk */
   RF_HH_4L_INT   ,     /* HammerHead with internal LNA */
   RF_LN22OUT_EXT ,     /* GL-LN22 RF orig. IF clk & Ref Clock */
   RF_HH_4L_INT_ON      /* HammerHead with internal LNA always on */
};
typedef enum RF_TYPE RF_TYPE;
/*</GLML>*/

/*******************************************************************************
* GL debug output levels
*******************************************************************************/
/*<GLML GlGpsApi.h SysLog_Level_Type>*/
enum GL_SYS_LOG_SEVERITY
{
    LOG_EMERG,          /* system is unusable */
    LOG_ALERT,          /* action must be taken immediately */
    LOG_CRIT,           /* critical condition */
    LOG_ERR,            /* error conditions */
    LOG_WARNING,        /* warning conditions */
    LOG_NOTICE,         /* normal but significant condition */
    LOG_INFO,           /* informational */
    LOG_DEBUG           /* debug-level messages */
};
/*</GLML>*/

/*******************************************************************************
* GL debug output contents
*******************************************************************************/
/*<GLML GlGpsApi.h SysLog_Content_Type>*/
enum GL_SYS_LOG_FACILITY
{
    /* library log types */
    LOG_EPH             = (0 << 3), /* 0x00000100 info from ephemeris manager */
    LOG_ACQ             = (1 << 3), /* 0x00000200 info from acquisition manager */
    LOG_NAVMOD          = (2 << 3), /* 0x00000400 info from navigation manager */
    LOG_SRCH            = (3 << 3), /* 0x00000800 info from search manager */
    LOG_TRLMGR          = (4 << 3), /* 0x00001000 info from trial manager */
    LOG_GLDBG           = (5 << 3), /* 0x00002000 info from debug module */
    LOG_SET_AID_GET_RES = (6 << 3), /* 0x00004000 info from assistance data manager */
    LOG_NVMEM           = (7 << 3), /* 0x00008000 info from non-volatile mem manager */
    LOG_ASICMGR         = (8 << 3), /* 0x00010000 info from ASIC manager */
    LOG_INTEGR          = (9 << 3), /* 0x00020000 integration */
    LOG_SUPL            =(10 << 3), /* 0x00040000 info from GL SUPL codec/manager */
    LOG_RRLP            =(11 << 3), /* 0x00080000 info from GL RRLP codec/manager */
    LOG_ASN1            =(12 << 3), /* 0x00100000 ASN1 communication data */
    LOG_KF              =(13 << 3), /* 0x00200000 info from KF */
    LOG_PLATFORM        =(14 << 3), /* 0x00400000 serial and timer info */

    /* LOG_015-LOG_023 RESERVED for user gps app. */
    LOG_USR0    = (15 << 3),        /* 0x00800000 */
    LOG_USR1    = (16 << 3),        /* 0x01000000 */
    LOG_USR2    = (17 << 3),        /* 0x02000000 */
    LOG_USR3    = (18 << 3),        /* 0x04000000 */
    LOG_USR4    = (19 << 3),        /* 0x08000000 */
    LOG_USR5    = (20 << 3),        /* 0x10000000 */
    LOG_USR6    = (21 << 3),        /* 0x20000000 */
    LOG_USR7    = (22 << 3),        /* 0x40000000 */
    LOG_USR8    = (23 << 3)         /* 0x80000000 */
};
/*</GLML>*/

/*******************************************************************************
* Macro definitions for priority arguments into setlogmask
*******************************************************************************/
/*<GLML GlGpsApi.h SysLogMasks_Macros_Type>*/
/* mask for a priority */
#define LOG_MASK(p)     (1L << (p))

/* mask for all priorities up to p */
#define LOG_UPTO(p)     ((1L << ((p)+1)) - 1)

/* mask to extract facility part */
#define LOG_FACMASK     0xF8

#define LOG_FAC(p)      (((p) & LOG_FACMASK) >> 3)

#define LOG_FCLTY(fac)  ((1L << LOG_FAC(fac)) << 8)
/*</GLML>*/

/*******************************************************************************
* Macro definitions for NMEA formatting utilities
*******************************************************************************/

#define GL_NMEA_GGA_MASK 0x00000001L
#define GL_NMEA_RMC_MASK 0x00000002L
#define GL_NMEA_GSV_MASK 0x00000004L
#define GL_NMEA_GSA_MASK 0x00000008L
#define GL_NMEA_FIX_MASK 0x00000010L
#define GL_NMEA_ULP_MASK 0x00000020L
#define GL_NMEA_LTO_MASK 0x00000040L
#define GL_NMEA_IGR_MASK 0x00000080L

#define GL_NMEA_ALL_MASK (GL_NMEA_GGA_MASK | GL_NMEA_RMC_MASK | GL_NMEA_GSA_MASK \
    | GL_NMEA_GSV_MASK | GL_NMEA_FIX_MASK | GL_NMEA_ULP_MASK | GL_NMEA_LTO_MASK  \
    | GL_NMEA_IGR_MASK )


#define  MAX_SV_COUNT 14

/*******************************************************************************
* Satellite information structure
*******************************************************************************/
/*<GLML GlGpsApi.h GL_SV_INFO_Type>*/
typedef struct GL_SV_INFO GL_SV_INFO;
struct GL_SV_INFO
{
    char  bDetected;         /* Satellite was detected */
    short sPrn;              /* SV PRN (1..32) */
    short sElev;             /* SV elevation [degrees] */
    short sAz;               /* SV azimuth   [degrees] */
    short sSNR;              /* C/No [dBHz] */
    short sSigStrength;      /* Signal strength estimation [dBm] */
    unsigned long ulIntTime; /* Integration time [msec]  */
    short sSNRFT;            /* C/No [1/10 dBHz] for factory test only*/
    short sSigStrengthFT;    /* Signal strength [1/10 dBm] for Factory test only*/
};
/*</GLML>*/

/*******************************************************************************
* GL time information structures
*  - see GSM 04.31 version 7.2.0 Release 1998, Table A.14
*******************************************************************************/
/*<GLML GlGpsApi.h GL_TIME_Type>*/
typedef struct GL_TIME GL_TIME; 
struct GL_TIME 
{
    unsigned short usGpsWeek;   /* Current GPS week [0..1023] */
    unsigned long  ulWeekMs;    /* Millisecond of current GPS week */
    unsigned short usMicrosec;  /* Microsecond portion of ulWeekMs */
    unsigned long  ulPrecUsec;  /* Time precision (us) (1-SIGMA) */
};
/*</GLML>*/

/*<GLML GlGpsApi.h WORLD_TIME_Type>*/
typedef struct WORLD_TIME WORLD_TIME; 
struct  WORLD_TIME
{
    unsigned short usYear;      /* Year     [0..65535] */
    unsigned short usMonth;     /* Month    [1.12] */
    unsigned short usDay;       /* Day      [1..31] */
    unsigned short usHour;      /* Hour     [0..23] */
    unsigned short usMin;       /* Minutes  [0..59] */
    unsigned short usSec;       /* Seconds  [0..60] */
    short          usMiliSec;   /* Milisec  [0..999] */
    unsigned short usMicroSec;  /* Microsec [0..999] */
    unsigned long  ulPrecUsec;  /* Time precision microsec [0..100 000 000] */
} ;
/*</GLML>*/

/*<GLML GlGpsApi.h GL_POS_SOURCE_Type>*/
enum GL_POS_SOURCE
{
    GL_POS_UNKNOWN,         /* Unknown source       (bPosValid = false) */
    GL_POS_UE_ASSISTED_AGPS,/* UA assisted AGPS     (bPosValid = true ) */
    GL_POS_UE_BASED_AGPS,   /* UA based AGPS        (bPosValid = true ) */
    GL_POS_AUTONOMOUS,      /* Autonomous           (bPosValid = true ) */
    GL_POS_CELL_ID,         /* Cell ID              (bPosValid = false) */
    GL_POS_LAST_KNOWN       /* Last known location  (bPosValid = false)  */
};
typedef enum GL_POS_SOURCE GL_POS_SOURCE;
/*</GLML>*/

/*******************************************************************************
* Status information about an ongoing position fix
*******************************************************************************/
/*<GLML GlGpsApi.h GL_FIX_STATUS_Type>*/
typedef struct GL_FIX_STATUS GL_FIX_STATUS;
struct GL_FIX_STATUS
{
    unsigned long  ulElapsedTime;     /* Elapsed fix time [ms] */
    unsigned long  ulWeekMs;          /* Millisecond of current GPS week */
    short          sSvCount;
    GL_SV_INFO     aSvInfo[MAX_SV_COUNT];
    char           bPosValid;         /* Valid position available */
    char           bPosComputed;      /* Position was computed, but may not be valid */
    GL_POS_SOURCE  aPosSource;        /* Position source          */
    unsigned long  ulInternalStatus;  /* see enum GlAidStatusCodes and ET_FIXSTAT_INTERNAL_STATUS */
    double         dLat;              /* Lattitude [degrees] */
    double         dLon;              /* Longitude [degrees] */
    double         dAlt;              /* Altitude  [meters]  */
    double         dHDOP;             /* KF Horizontal Dilution Of Precision */
    double         dEstErr;           /* Estimated position error [meters] */
    double         dEstErrHigh;       /* High reliability EstErr [meters] */
    double         dEstErrAlt;        /* Estimated altitude position error */
                                      /* [meters] */
    long           lTimeTagDelta;     /* Estimated time tag error [ms] */
                                      /* = Time Tag - GPS time */
    char           cSpeedValid;       /* valid speed available */
    double         dSpeedInKnots;     /* speed in knot */
    char           cTrackAngleValid;  /* valid track angle available */
    double         dTrackAngle;       /* track angle in degree */
    signed long    slFreqOffst;       /* Receiver clock offset [ppb]*/
    unsigned short usFreqOffsAcc;     /* Estimated accuracy [ppb] */
    
    short          sUsedSvCount;      /* Sv number used in pos computation */
    unsigned long  ulUsedSvsMask;     /* bit0 - PRN1, ... ,bit31 - PRN22 */
    unsigned short usTimeTagDeltaUs;  /* Sub millisecond portion of time tag */
                                      /* error in units of us: 0-999 microsec */
    unsigned long  ulSyncElapsedTimeUs; /* Elapsed time between ulWeekMs and */
                                        /* the SYNC pulse time in units of us */
    WORLD_TIME     utcTime;
    float          fWer;              /* WER test result in % */
    unsigned long  ulNbWerWrd;        /* Number of words in WER test result */
};
/*</GLML>*/

/*******************************************************************************
* Satellites information indicating the progress of an ongoing position
* fix. This is a subset of struct GL_FIX_STATUS
*******************************************************************************/
/*<GLML GlGpsApi.h GL_FIX_PROGRESS_Type>*/
typedef struct GL_FIX_PROGRESS GL_FIX_PROGRESS;
struct GL_FIX_PROGRESS
{
    unsigned long  ulElapsedTime;     /* Elapsed fix time [ms] */
    unsigned long  ulWeekMs;          /* Millisecond of current GPS week */
    short          sSvCount;
    GL_SV_INFO     aSvInfo[MAX_SV_COUNT];
    char           bPosValid;         /* Valid position available */
    unsigned long  ulUsedSvsMask;     /* bit0 - PRN1, ... ,bit31 - PRN32 */
    WORLD_TIME     utcTime;
};
/*</GLML>*/

/*******************************************************************************
* Library version information
*******************************************************************************/
/*<GLML GlGpsApi.h GL_VERSION_Type>*/
typedef struct GL_VERSION GL_VERSION;
struct GL_VERSION 
{
    unsigned short sApiVer; /* API   Version  [1..65535] */
    unsigned short sMajor;  /* Major revision [1..65535] */
    unsigned short sMinor;  /* Minor revision [1..65535] */
};
/*</GLML>*/


/*******************************************************************************
* GPS Assistance Data Request
*  - see GSM 09.31 version 8.1.0 Release 1999, 10.9 & 10.10
*******************************************************************************/
/*<GLML GlGpsApi.h GL_AID_REQUEST_Type>*/
typedef struct GL_AID_REQUEST GL_AID_REQUEST;
struct GL_AID_REQUEST
{
    signed   long lLat;
    signed   long lLon;
    signed   long lAlt;
    unsigned long ulAidMask;  /* see the enumeration GlAidReqCodes */

    unsigned char ucNSAT;     /* number of satellites GLL already has aiding */
                              /* for.   NOTE: the following data should be */
                              /* ignored if ucNSAT == 0 */
    unsigned short usGpsWeek; /* 10-bit GPS week corresponding to the  */
                              /* most recent ephemeris currently available */
    unsigned char ucToe;      /* GPS time of ephemeris in hours of the  */
                              /* most recent ephemeris currently available */
    unsigned char ucT_ToeLimit;/* Ephemeris age tolerance in hours  */
                               /*(0..10 hours) */
    /* NOTE: the RRLP specification allows maximum of 15 per-satellite data */
    unsigned char ucSatID[31];/* 1-based satellite PRN numbers */
    unsigned char ucIODE[31]; /* Issue Of Data Ephemeris for the PRN */ 
};
/*</GLML>*/


/*******************************************************************************
* Reference Location Assistance Information
*  - see GSM 04.31 version 7.2.0 Release 1998, App. A.4.2.4
*  - see GSM 03.32 version 7.1.0 Release 1998, section 5.5
*******************************************************************************/
/*<GLML GlGpsApi.h GL_ASS_POS_Type>*/
typedef struct GL_ASS_POS GL_ASS_POS;
struct GL_ASS_POS
{
    double dLat;
    double dLon;
    double dAlt;
};
/*</GLML>*/

/*******************************************************************************
* Reference Location Assistance Information
*  - see GSM 04.31 version 7.2.0 Release 1998, App. A.4.2.4
*  - see GSM 03.32 version 7.1.0 Release 1998, section 5.5
*******************************************************************************/
/*<GLML GlGpsApi.h GL_ASS_POS_Type>*/
typedef struct GL_ASS_POS_QUAL GL_ASS_POS_QUAL;
struct GL_ASS_POS_QUAL
{
    GL_ASS_POS otPos;
    double     dHorAcc;
    double     dVerAcc;
};
/*</GLML>*/

/*******************************************************************************
* Shape description of an ellipsoid point with uncertainty ellipse
*  - see GSM 03.32 version 7.1.0 Release 1998, section 7.3.3
*******************************************************************************/
/*<GLML GlGpsApi.h GL_GEO_INFO_Type>*/
typedef struct GL_GEO_INFO GL_GEO_INFO;
struct GL_GEO_INFO
{
    /* 0 - Ellipsoid Point
     * 1 - EP with uncertainty circle
     * 3 - EP with uncertainty Ellipse
     * 8 - EP with altitude
     * 9 - EP with altitude and uncertainty Ellipse
     */
    GL_SHAPE_TYPE  etTypeOfShape;
    unsigned char  ucSignOfLat;    /* 0 - North , 1 - South */
    unsigned long  ulLat;          /* 0 - 90 degrees N = dLatDeg*2^23 / 90 */
    signed   long  slLon;          /* -180 .. +180   N = dLonDeg*2^24 /360 */

    unsigned char  ucUncertaintySemiMajor; /* N = 10.5 * ln(r/10 + 1) */
    unsigned char  ucUncertaintySemiMinor; /* N = 10.5 * ln(r/10 + 1) */
    char           cAngleOfMajorAxis;      /* N = deg/2 deg=[0.0..360.0] */
    unsigned char  ucDirectionOfAltitude;  /* 0 - height, 1 - depth */
    unsigned short usAltitude;             /* Meters [0 .. (2^15 - 1)] */
    unsigned char  ucUncertaintyAltitude;  /* N = 40.5 * ln(h/45 + 1)  */
    unsigned char  ucConfidence;           /* Percent */
};
/*</GLML>*/


/*******************************************************************************
* Reference Location Assistance Information
*  - see GSM 04.31 version 7.2.0 Release 1998, App. A.4.2.4
*  - see GSM 03.32 version 7.1.0 Release 1998, section 5.5
*******************************************************************************/
/*<GLML GlGpsApi.h GL_ASS_POS_Type>*/
typedef struct GL_REF_LOC GL_REF_LOC;
struct GL_REF_LOC
{
    GL_GEO_INFO stThreeDLocation;
};
/*</GLML>*/

/*******************************************************************************
* GPS Acquisition Assistance - nsats times per msg
*  - see GSM 04.31 version 7.2.0 Release 1998, Table A.25
*******************************************************************************/
/*<GLML GlGpsApi.h GL_AID_ASSIST_Type>*/
typedef struct stAiding stAiding;
struct stAiding
{
    short PrnID;        /* SV prn number */
    short Dopp0;        /* Doppler (0th order term) */
                        /*   units: 2.5Hz, [-5120..5117.5]  */
    short Dopp1;        /* Doppler (1st order term) */
                        /*   units: 1Hz/sec [-1..0.5]  */
    short DoppErr;      /* Doppler Uncertainty */
                        /*   units: 12.5 Hz */
    short CodePhase;    /* Code Phase  */
                        /*   units: 1 chip [0..1022] */
    short IntCodePhase; /* Integer Code Phase */
                        /*   units: 1 C/A period [0..19] */
    short GPSBitNumber; /* GPS bit number */
                        /*   units: 1 GPS bit [0..3] */
    short SearchWindow; /* Code Phase Search Window */
                        /*   units: 1 chip [1..192] */
    short Azimuth;      /* Azimuth */
                        /*   units: 11.25 degree [0..348.75] */
    short Elevation;    /* Elevation */
                        /*   units: 11.25 degree [0..78.75] */
};
/*</GLML>*/

/*******************************************************************************
*  GPS Acquisition Assistance Information - once per msg
*  - see GSM 04.31 version 7.2.0 Release 1998, Table A.24
*******************************************************************************/
#define  MAX_ASS_SIZE 14
typedef struct GL_ASS_ACQ GL_ASS_ACQ;
/*<GLML GlGpsApi.h GL_ASS_ACQ_Type>*/
struct GL_ASS_ACQ
{
    short     NumSatsTotal;
    long      GPSTow;
    stAiding  arrStAiding[MAX_ASS_SIZE];
};
/*</GLML>*/

/*******************************************************************************
* Navigation Model - per-satellite information
*  - see GSM 04.31 version 7.2.0 Release 1998, Table A.19
*******************************************************************************/
/*<GLML GlGpsApi.h GlUncmprsdEph_Type>*/
typedef struct GlUncmprsdEph GlUncmprsdEph;
struct GlUncmprsdEph
{
    unsigned long SatID;        /* SV prn number */
    unsigned long ExtensionBit; /* Set to 1 for GL LTO, otherwise 0 */
    unsigned long SatStatus;    /* Status field from GSM 4.31 */
    unsigned long codeL2;       /* codes on L2 channel */
    unsigned long URA;          /* URA Index */
    unsigned long health;       /* Satellite health */
    unsigned long IODC;         /* Issue of data, clock */
    unsigned long L2Pdata;      /* L2 P data flag */
    unsigned long SF1Reserved0; /* 23 bits in ICD-200, sbfrm1 */
    unsigned long SF1Reserved1; /* 24 bits in ICD-200, sbfrm1 */
    unsigned long SF1Reserved2; /* 24 bits in ICD-200, sbfrm1 */
    unsigned long SF1Reserved3; /* 16 bits in ICD-200, sbfrm1 */
    signed   long T_GD;    /* group delay (seconds) */
    unsigned long Toc;     /* time of clock (seconds) */
    signed   long a_f2;    /* SV clock drift rate (sec/sec2) */
    signed   long a_f1;    /* SV clock drift (sec/sec) */
    signed   long a_f0;    /* SV clock bias (seconds) */
    signed   long Crs;     /* Amplitude of sine harmonic correction (meters) */
    signed   long Delta_n; /* Mean motion difference from computed (rad/s) */
    signed   long M0;      /* Mean anomaly at reference time (rad) */
    signed   long Cuc;     /* Amplitude of cosine harmonic correction (rad) */
    unsigned long e;       /* Eccentricity (dimensionless) */
    signed   long Cus;     /* Amplitude of sine harmonic correction (rad) */
    unsigned long Asqrt;   /* Square root of semi-major axis (meters^1/2) */
    unsigned long Toe;     /* Reference time of ephemeris (seconds) */
    unsigned long FitIntervalFlag;/* Fit Interval Flag */
    unsigned long AODO;
    signed   long Cic;     /* Amplitude of sine harmonic correction (rad) */
    signed   long Omega0;  /* Longitude of ascending node (rad) */
    signed   long Cis;     /* Amplitude of sine harmonic correction (rad) */
    signed   long i0;      /* Inclination angle at reference time (rad) */
    signed   long Crc;     /* Amplitude of sine harmonic correction (meters) */
    signed   long Omega;   /* Argument of perigee (radians) */
    signed   long Omega_dot;    /* Rate of right ascension (radians/sec) */
    signed   long IDOT;    /* Rate of inclination angle (radians/sec) */
};
/*</GLML>*/



/*******************************************************************************
* Navigation Model - per-satellite information
*  - see GSM 04.31 version 7.2.0 Release 1998, Table A.19
*******************************************************************************/
/*<GLML GlGpsApi.h GlAlmanac_Type>*/
typedef struct GlAlmanac GlAlmanac;
struct GlAlmanac 
{
    unsigned char  SatID;     /* prn ID */
    unsigned short e;         /* Eccentricity (dimensionless) */
    unsigned char  toa;       /* Reference time of almanac (seconds) */
    signed   short deltai;    /* Inclination delta (semi circles) */
    signed   short Omega_dot; /* Rate of right ascension (radians/sec) */
    unsigned long  Asqrt;     /* Square root of semi-major axis (meters^1/2) */
    signed   long  omega0;    /* Longitude of ascending node (rad) */
    signed   long  omega;     /* Argument of perigee (radians) */
    signed   long  M0;        /* Mean anomaly at reference time (rad) */
    signed   short a_f1;      /* SV clock drift (sec/sec) */
    signed   short a_f0;      /* SV clock bias (seconds) */
    signed   short wn;        /* Almanac reference week */
    unsigned short health;    /* Health*/
};
/*</GLML>*/

/*******************************************************************************
* Reference time  - once per msg
*  - see GSM 04.31 version 8.10.0 Release 2002, Table A.14
*******************************************************************************/
/*<GLML GlGpsApi.h GL_TOW_ASSIST_Type>*/
typedef struct GL_TOW_ASSIST GL_TOW_ASSIST;
struct GL_TOW_ASSIST
{
    unsigned char  ucSatId;             /* 0-63 (1 - 64) */
    unsigned short usTlmMessage;        /* 0 - 16383 */
    unsigned char  ucAntiSpoof;         /* 0 - 1 */
    unsigned char  ucAlert;             /* 0 - 1 */
    unsigned char  ucTLMreserved;       /* 0 - 3 */
};
/*</GLML>*/

#define MAX_TOW_ASSIST_SVS 12
/*<GLML GlGpsApi.h GL_REF_TIME_Type>*/
typedef struct GL_REF_TIME GL_REF_TIME;
struct GL_REF_TIME
{
    unsigned char  ucTowAssistPresent;  /* 1 if the GL_TOW_ASSIST present */
    unsigned long  ulGpsTow;         /* 0.08 sec, 0 - 604799.92 sec */
    unsigned short usGpsWeek;        /* 0 - 1023 */
    unsigned char  ucNumSatsTotal;   /* 0 - 11 ( 0 - 1 SVs, 1 - 2 SVs, ...) */

    /* Optional, if ucTowAssistPresent == 1 */
    GL_TOW_ASSIST astTowAssist[MAX_TOW_ASSIST_SVS];
};
/*</GLML>*/

/*******************************************************************************
* GPS Measurement Information Element
*  - see GSM 04.31 version 7.2.0 Release 1998, Table A.5
*  - see GSM 04.31 version 7.2.0 Release 1998, Table A.8
*******************************************************************************/
/*<GLML GlGpsApi.h stSatPRMeas_Type>*/
#define  MAX_MSMT_SIZE 12
typedef struct stSatPRMeas stSatPRMeas;
struct stSatPRMeas
{
    unsigned long  SatID;
    unsigned long  C2N0;                /* carrier to noise ratio */
    unsigned long  sDopler;
    unsigned long  sWholeChip;
    unsigned long  sFracChip;
    unsigned long  cMultipathIndicator;
    unsigned long  cPrRmsError;         /* pseudo range RMS error */
};
/*</GLML>*/

/*<GLML GlGpsApi.h GL_RES_MEAS_Type>*/
typedef struct GL_RES_MEAS GL_RES_MEAS;
struct GL_RES_MEAS
{
    unsigned long   ulDataSize;         /* will be removed    */
    unsigned long   usBTSRefFrame;
    unsigned long   GPSTow;             /* [0 ..  14399999] ms */
    unsigned long   NumSats;
    stSatPRMeas     aArraySat[MAX_MSMT_SIZE];
    short           sMessSize;          /* will be removed     */
    unsigned long   GPSTowFull;         /* [0 .. 604799999] ms */
};
/*</GLML>*/

/*<GLML GlGpsApi.h stSatPRMeas_Type>*/
typedef struct stSatRawMeas stSatRawMeas;
struct stSatRawMeas
{
    char cPrn;
    unsigned char   ucValidMask;      /* 0x01 - range valid    */
                                      /* 0x02 - doppler valid  */

    /* Range Measurement */
    long            lRangeTimeMs;     /* Offset from GPSTow */
    unsigned long   ulRangeFracChip;  
    unsigned short  cPrRmsErrorMeter; /* pseudo range RMS */

    /* Frequency measurement */
    long            lDopplerTimeMs;    /* Offset from GPSTow */
    short           sDopplerHz;        /* SV Doppler offset (Hz) */
    unsigned short  usDopplerRmsHz;    /* Doppler RMS (Hz)*/
};

/*</GLML>*/

/*<GLML GlGpsApi.h GL_RAW_MEAS_Type>*/
typedef struct GL_RAW_MEAS GL_RAW_MEAS;
struct GL_RAW_MEAS
{
    unsigned long   GPSTow;
    unsigned long   NumSats;
    stSatRawMeas     aArraySat[MAX_MSMT_SIZE];
};
/*</GLML>*/

/*******************************************************************************
* Location Information Element
*  - see GSM 04.31 version 7.2.0 Release 1998, App. A.3.2.4
*  - see GSM 03.32 version 7.2.0 Release 1998, Sect 5.1
*  - ASN.1 DESCRIPTION: Location Information IE
*******************************************************************************/
/*<GLML GlGpsApi.h GL_RES_POS_Type>*/
typedef struct GL_RES_POS GL_RES_POS;
struct GL_RES_POS
{
    unsigned long   ulGPSTow;           /* 0 - 1439999 ms  0-(2^24-1) */
    unsigned char   ucPosMode;          /* fix type 0 - 2D, 1 - 3D    */
    GL_GEO_INFO     stPosEstimate;      /* Position Estimate          */
    unsigned long   GPSTowFull;         /* [0 .. 604799999] ms */
};
/*</GLML>*/

/*******************************************************************************
* Aiding status codes
*******************************************************************************/
enum GlAidStatusCodes
{
    GL_FIX_STAT_BIT_NOAID_TIME = (1 << 3), /* Initial time not available */
    GL_FIX_STAT_BIT_NOAID_POS  = (1 << 4), /* Initial position not available */
    GL_FIX_STAT_BIT_NOAID_NAV  = (1 << 5)  /* Nav assistance not available */
};
typedef enum GlAidStatusCodes GlAidStatusCodes;

/*******************************************************************************
* Aiding requirement codes
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAid_Proto1>*/
enum GlAidReqCodes
{ 
    GL_RQD_ACQ_RQD  = (1 << 0),
    GL_AID_POS_RQD  = (1 << 1),
    GL_AID_NAV_RQD  = (1 << 2),
    GL_AID_TIM_RQD  = (1 << 3),

    GL_RQD_ACQ_OPT  = (1 << 8),
    GL_AID_POS_OPT  = (1 << 9),
    GL_AID_NAV_OPT  = (1 << 10),
    GL_AID_TIM_OPT  = (1 << 11)
};
typedef enum GlAidReqCodes GlAidReqCodes;
/*</GLML>*/


/*<GLML GlGpsApi.h GL_LINK_LATENCY_Type>*/
typedef struct GL_LINK_LATENCY  GL_LINK_LATENCY;
struct GL_LINK_LATENCY
{
  unsigned long   ulNominalMs;   /* Nominal link latency               */
  unsigned long   ulIncrementMs; /* Increment every time error occured */
  unsigned long   ulMaximumMs;   /* Maximum link latency               */
} ;
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Sets the facilities that GlSysLog will mask
*
* INPUTS:       lFacMask - mask for priorities, defined in GL_SYS_LOG_SEVERITY
*               and can be assigned with combination of LOG_FCLTY macro
*
* OUTPUTS:      None
*
* NOTES:        use w/ SysLog Masks Macros
*               There are various modules/facilities within the library.
*               Some modules handle navigation info, some handle signal 
*               acquisition, etc. Users can control what modules send debug
*               information into the syslog buffer calling SetLogFacMask
*               with an argument that masks the library modules of
*               interest. The correct mask can be created by ORing a
*               series of LOG_FCLTY(xxx), where xxx is one of the values
*               in the enumerated type GL_SYS_LOG_FACILITY.
*               Note: 0xFFFFFFFFL ==> all modules send info
*******************************************************************************/
/*<GLML GlGpsApi.h glSetLogFacMask_Proto>*/
GLGPS_API void glSetLogFacMask(unsigned long lFacMask );
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the priorities that GlSysLog will mask
*
* INPUTS:       lPriMask - mask for priorities, defined in GL_SYS_LOG_SEVERITY
*
* OUTPUTS:      None
*
* NOTES:        How much information the library sends into this buffer can be 
*               controlled by LOG_UPTO and LOG_MASK macros. 
*               Example: LOG_UPTO(LOG_DEBUG) => send all info to the log file
*******************************************************************************/
/*<GLML GlGpsApi.h glSetLogPriMask_Proto1>*/
GLGPS_API void glSetLogPriMask(unsigned long lPriMask );
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends GLL & user GPS app. debug data to stream specified by user
*
* INPUTS:       priority - combination of GL_SYS_LOG_SEVERITY and 
*                          GL_SYS_LOG_FACILITY (Ex.: ( LOG_INFO | LOG_USR0 ) )
*
*               fmt      - format string, same as for "printf" function
*
* OUTPUTS:
*
* NOTES:        This function can be used by the application to add information
*               to the SysLog buffer. These messages will be output via the
*               diagnostic facility (glcb_DiagAppend callback function) along 
*               with messages generated by the library itself.
*               Also see glSetDiagBuffer() function
*******************************************************************************/
/*<GLML GlGpsApi.h glSysLog_Proto>*/
GLGPS_API void glSysLog (unsigned long priority, const char *fmt, ...);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the bus type for communication with the GL-ASIC
*
* INPUTS:       etBusType - bus type
*
* OUTPUTS:      None
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetBusType_Proto>*/
GLGPS_API void glSetBusType(BUS_TYPE etBusType);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the GLL to the simulation mode
*
* INPUTS:       const GLCBID callbackID,GLCB glCallBackFunc
*
* OUTPUTS:
*
* NOTES:        Track PRN 27 from GPS simulator
*******************************************************************************/
/*<GLML GlGpsApi.h glSetSimType_Proto>*/
GLGPS_API void glSetSimType(SIM_TYPE etSimType);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Performs the GL Control library environment initialization
*               must be called prior to any other GL function
*
* INPUTS:       const GLCBID callbackID,GLCB glCallBackFunc
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES: glInitEnvironment() is actually defined as a macro, with the GLL 
* version number embedded into it. This is done to ensure version match between
* this header file and the compiled library. If the versions don't match the
* linker would not resolve the glInitEnvironment_VVV() name (VVV stands for
* the version number of the header), so the application won't link.
*******************************************************************************/
#define glInitEnvironment GL_FUNC_NAME(glInitEnvironment, GL_API_VER)
/*<GLML GlGpsApi.h glInitEnvironment_Proto>*/
GLGPS_API void glInitEnvironment(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Obtain the RF input frequency
*
* INPUTS:       none
*
* OUTPUTS:      the frequency used in the RF input
*
* NOTES: 
*******************************************************************************/
GLGPS_API unsigned short glGetRFInputFreq(void);

/*******************************************************************************
* DESCRIPTION:  Sets the RF chip type for the Control Library. This function
*               must be called before calling glOnStart().
*
* INPUTS:       RF_TYPE ucRfType
*
* OUTPUTS:      none
*******************************************************************************/
/*<GLML GlGpsApi.h glSetRfType_Proto>*/
GLGPS_API void glSetRfType(RF_TYPE etRfType);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets a frequency plan
*
* INPUTS:       GL_FREQ_PLAN etFP
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSFrPln_Proto>*/
GLGPS_API GLRC glSFrPln(GL_FREQ_PLAN etFP);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets low duty cycle for correlations
*               DUTYCYCLE_DISABLE = Use normal duty cycle,  
*               DUTYCYCLE_AUTO    = GLL determines when to enable low duty cycle
*               DUTYCYCLE_ENABLE  = Use low duty cycle (overrides GLL selection)
*
* INPUTS:       GL_LOW_DUTYCYCLE etLDC
*******************************************************************************/
/*<GLML GlGpsApi.h glSetLowDutyCycle_Proto>*/
GLGPS_API void glSetLowDutyCycle(GL_LOW_DUTYCYCLE etLDC);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Returns the current version of the GL library
*
* INPUTS:       GL_VERSION *pVersion (empty)
*
* OUTPUTS:      GL_VERSION *pVersion (full)
*               GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetVersion_Proto>*/
GLGPS_API GLRC glGetVersion(GL_VERSION *pVersion);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the library to GPS simulator mode
*
* INPUTS:
*
* OUTPUTS:      GLRC stat
*
* NOTES:        see also glSetSimType()
*******************************************************************************/
/*<GLML GlGpsApi.h glDoSim_Proto>*/
GLGPS_API GLRC glDoSim(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the mode of operation of the GL control library
*
* INPUTS:       glFixMode; options are:
*               GL_FIXMODE_RANGE        (MS-assisted, faster)
*               GL_FIXMODE_RANGEDOPPLER (MS-assisted, slower)
*               GL_FIXMODE_POS          (MS-based, faster)
*               GL_FIXMODE_POSVEL       (MS-based, slower)
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        1. GL_FIXMODE will soon only have MS-A, MS-B, and AUTO
*******************************************************************************/
/*<GLML GlGpsApi.h glSetFixMode_Proto>*/
GLGPS_API GLRC glSetFixMode(GL_FIXMODE glFixMode);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  This API sets whether GLL is requested to generate
*               multiple GPS Measurement Information sets
*
* INPUTS:       iEnable; options are:
*               0 (sending of multiple sets is not allowed (default)
*               1 (multiple IEs can be set)
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR
*******************************************************************************/
/*<GLML GlGpsApi.h glEnableMultipeSets_Proto>*/
GLGPS_API GLRC glEnableMultipeSets(int iEnable);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Provides RRLP aiding request information for SMLC
*
* INPUTS:       GL_AID_REQUEST *glAidRequest
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 09.31 ver. 7.2.0 Rel. 1998, Sect. 10.9 & 10.10
*******************************************************************************/
/*<GLML GlGpsApi.h glGetAidRequest_Proto>*/
GLGPS_API GLRC glGetAidRequest(GL_AID_REQUEST *pAidRequest);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the RRLP initial reference location data into the GLL
*
* INPUTS:       const GL_ASS_POS
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, A.4.2.4
*               and with GSM 03.32 version 7.1.0 Release 1998, section 5.5
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAidPos_Proto>*/
GLGPS_API GLRC glSetAidPos(const GL_ASS_POS *pPosAssist);

/*******************************************************************************
* DESCRIPTION:  Sends the RRLP initial reference location data into the GLL
*
* INPUTS:       const GL_ASS_POS_QUAL 
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, A.4.2.4
*               and with GSM 03.32 version 7.1.0 Release 1998, section 5.5
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAidPosQual_Proto>*/
GLGPS_API GLRC glSetAidPosQual(const GL_ASS_POS_QUAL *pPosAssistQual);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the RRLP initial reference location data into the GLL
*
* INPUTS:       const GL_REF_LOC
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, A.4.2.4
*               and with GSM 03.32 version 7.1.0 Release 1998, section 5.5
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAidLoc_Proto>*/
GLGPS_API GLRC glSetRefLoc(const GL_REF_LOC *pRefLocation);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Provides the RRLP acquisition assistance data into the GLL
*
* INPUTS:       const GL_ASS_ACQ *pAcqAssist
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, Table A.24
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAidAcq_Proto>*/
GLGPS_API GLRC glSetAidAcq(const GL_ASS_ACQ *pAcqAssist);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the Uncompressed Ephemeris for one satellite into the GLL.
*
* INPUTS:       pEphemeris - navigation data buffer
*               isLast    - flag informing the GLL if this is the last element
*                           completing the dataset. Zero value indicates that 
*                           GLL should expect more data. 
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, Table A.19
*******************************************************************************/
/*<GLML GlGpsApi.h glSetEphemeris_Proto>*/
GLGPS_API GLRC glSetEphemeris(const GlUncmprsdEph *pEphemeris, 
                              unsigned char isLast);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the LTO ID into the GLL
*
* INPUTS:       ulLtoId is the unique ID of the LTO file used to set ephemeris 
*               via glSetEphemeris() API. Usually it's a full cycle time 
*               of the first LTO record in the file.
*
* OUTPUTS:      GLRC status
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glLtoId_Proto>*/
GLGPS_API GLRC glSetLtoId(unsigned long ulLtoId);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the Almanac for one satellite into the GLL.
*
* INPUTS:       pAlmanac  - Almanac data structure
*               isLast    - flag informing the GLL if this is the last element
*                           completing the dataset. Zero value indicates that 
*                           GLL should expect more data. 
*
*               If both parameters are set to 0, GLL will use the Almanac 
*               hard coded into the ROM.
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, Table A.23
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAlmanac_Proto>*/
GLGPS_API GLRC glSetAlmanac(const GlAlmanac *pAlmanac, 
                            unsigned char isLast);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the current time information into the library
*
* INPUTS:       const GL_TIME *pglTime    - time tag info in GL_TIME format
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, Table A.14
*
*******************************************************************************/
/*<GLML GlGpsApi.h glSetGpsTime_Proto>*/
GLGPS_API GLRC glSetGpsTime(const GL_TIME *pglTime);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the Reference Time assistance to the library
*
* INPUTS:       const GL_REF_TIME *pRefTime    - pointer to the Reference Time
*               assistance structure
*
* OUTPUTS:      GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, Table A.14
*******************************************************************************/
/*<GLML GlGpsApi.h glSetRefTime_Proto>*/
GLGPS_API GLRC glSetRefTime(const GL_REF_TIME *pRefTime);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Provides measurement results for a fix
*
* INPUTS:       GL_RES_MEAS    *pMeasResults (empty) - MS-Assisted
*
* OUTPUTS:      GL_RES_MEAS    *pMeasResults (full)
*               GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, Table A.5
*               and GSM 04.31 version 7.2.0 Release 1998, Table A.8
*******************************************************************************/
/*<GLML GlGpsApi.h glGetResultsMeas_Proto>*/
GLGPS_API GLRC glGetResultsMeas(GL_RES_MEAS *pMeasResults);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Provides measurement results for a fix
*
* INPUTS:       GL_RAW_MEAS    *pMeasResults (empty) 
*
* OUTPUTS:      GL_RAW_MEAS    *pMeasResults (full)
*               GLRC status
*
* NOTES:        Not implemented yet.
*               
*******************************************************************************/
/*<GLML GlGpsApi.h glGetRawMeas_Proto2>*/
/* This function is not implemented */
/* GLGPS_API GLRC glGetRawMeas(GL_RAW_MEAS *pMeasResults); */
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Provides measurement results for a fix
*
* INPUTS:       GL_RES_MEAS    *pMeasResults (empty) - MS-Assisted
*
* OUTPUTS:      GL_RES_MEAS    *pMeasResults (full)
*               GLRC status
*
* NOTES:        Aligns with GSM 04.31 version 7.2.0 Release 1998, Table A.5
*               and GSM 04.31 version 7.2.0 Release 1998, Table A.8
*******************************************************************************/
/*<GLML GlGpsApi.h glGetResultsMultiMeas_Proto>*/
GLGPS_API GLRC glGetResultsMultiMeas(GL_RES_MEAS *pMeasResults, 
                                     short *psMaxMeas);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Provides position results for a fix
*
* INPUTS:       GL_RES_POS    *pPosResults (empty) - MS-Based
*
* OUTPUTS:      GL_RES_POS     *pPosResults (full)
*               Return value:  GLRC_OK - Position valid, 
*               GLRC_ERR - Position not valid
*
* NOTES:     1. Aligns with GSM 04.31 version 7.2.0 Release 1998, 
*               App. A.3.2.4 and GSM 03.32 version 7.2.0 Release 1998, Sect 5.1
*            2. see ASN.1 DESCRIPTION: Location Information IE
*******************************************************************************/
/*<GLML GlGpsApi.h glGetResultsPos_Proto>*/
GLGPS_API GLRC glGetResultsPos(GL_RES_POS *pPosResults);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Returns current precision of time kept in GLL [microsec]
*
* INPUTS:
*
* OUTPUTS:      unsigned long ulMicroSecPrecision
*
* NOTES:        If external time source is better, call glSetGpsTime()
*******************************************************************************/
/*<GLML GlGpsApi.h glGetGlTimePrecision_Proto>*/
GLGPS_API unsigned long glGetGlTimePrecision(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Provides status information about the latest position fix
*
* INPUTS:       GL_FIX_STATUS *pFixStatus (empty)
*
* OUTPUTS:      GL_FIX_STATUS *pFixStatus (full)
*               GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetFixStatus_Proto>*/
GLGPS_API GLRC glGetFixStatus(GL_FIX_STATUS *pFixStatus);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Provides Satellites information to indicate the progress
*               of an ongoing position fix
*
* INPUTS:       GL_FIX_PROGRESS *pFixProgress (empty)
*
* OUTPUTS:      GL_FIX_PROGRESS *pFixProgress (full)
*               GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetFixProgress_Proto>*/
GLGPS_API GLRC glGetFixProgress(GL_FIX_PROGRESS *pFixProgress);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the current GPS time information into the library
*
* INPUTS:       pworldTime    - time tag info in WORLD_TIME format
*               useSync       - use time tag to sync with GPS time
*               lMaxLatencyMs - max delay between PPS latch and
*                               internal GPS time sync
*               lTimeOutMs    - timeout for PPS latching signal
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetTimeWdFmt_Proto>*/
GLGPS_API GLRC glSetTimeWdFmt(const WORLD_TIME *pworldTime, 
                    char useSync, long lMaxLatencyMs, long lTimeOutMs);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the current UTC time information into the library
*
* INPUTS:       pworldTime          - time tag info in WORLD_TIME format
*               char useSync        - use time tag to sync with GPS time
*               long lMaxLatencyMs  - max delay between PPS latch and
*                                     internal GPS time sync
*               long lTimeOutMs     - timeout for PPS latching signal
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetUtcTimeWdFmt_Proto>*/
GLGPS_API GLRC glSetUtcTimeWdFmt (const WORLD_TIME *pworldTime, 
                        char useSync, long lMaxLatencyMs, long lTimeOutMs);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the buffer where the library syslog will send diagnostics
*
* INPUTS:       glDiagAddress - ptr to the beginning of the buffer
*               Size            - size of the buffer
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        The GL library will use this buffer to output debug info. The
*               library keeps internal pointers to wrap around the buffer, but
*               the user is responsible for emptying the buffer as it gets
*               filled by the GLL.
*******************************************************************************/
/*<GLML GlGpsApi.h glSetDiagBuffer_Proto>*/
GLGPS_API GLRC glSetDiagBuffer(void *glDiagAddress, long lSize);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Gets the latest character sent to the diagnostics character
*
* INPUTS:
*
* OUTPUTS:      ch - the character gotten
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetDiagChar_Proto>*/
GLGPS_API char glGetDiagChar(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Gets the latest characters sent to the diagnostics character
*
* INPUTS:       pChars - where the diag characters should go.
*               maxFill - the maximum number of bytes that pChars can hold.
*
* OUTPUTS:      unsigned long - the number of bytes put into pChar.
*******************************************************************************/
/*<GLML GlGpsApi.h glGetDiagChars_Proto>*/
GLGPS_API unsigned long glGetDiagChars(char* pChars, unsigned long maxFill);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Gets one line of diagnostic log file information.
*
* INPUTS:       pChars - where the diag characters should go.
*               maxFill - the maximum number of bytes that pChars can hold.
*               searchChar - typically '\n' to read diag output at this
*                           character.
*
* OUTPUTS:      unsigned long - the number of bytes put into pChar.
*******************************************************************************/
/*<GLML GlGpsApi.h glGetDiagLine_Proto>*/
GLGPS_API unsigned long glGetDiagLine(char* pChars, unsigned long maxFill, char searchChar);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Flags the satellite with corresponding PRN as unhealthy.
*
* INPUTS:       sPrn
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        Will be replaced with glSetRealTimeIntegrity(), which 
*               aligns with 4.31
*******************************************************************************/
/*<GLML GlGpsApi.h glSetBadSv_Proto>*/
GLGPS_API GLRC glSetBadSv(short sPrn);
/*</GLML>*/

/*******************************************************************************
* NAME:           glSetLinkLatency
*
* DESCRIPTION:    Sets the link latency parameters
*
* INPUTS:        glSetLinkLatency
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glSetLinkLatency_Proto>*/
GLGPS_API GLRC glSetLinkLatency(const GL_LINK_LATENCY *pLinkLatency);
/*</GLML>*/

/******************************************************************************
* ADVANCED ******************************************************************** 
*******************************************************************************/

/*******************************************************************************
* GL Position Instructions Element
*  - see 04.31 version 7.2.0 Release 1998, section A.2.2.1
*******************************************************************************/
/*<GLML GlGpsApi.h GL_POS_INSTRUCT_Type>*/
typedef struct GL_POS_INSTRUCT GL_POS_INSTRUCT;
struct GL_POS_INSTRUCT
{
    unsigned long ulAccMand;  /* 0-Accuracy Optional 1 -accuracy is mandatory */
    unsigned long ulAccIdx;   /*  Accuracy = 10*((1 + 0.1)^N -1)  [0 .. 127] */
    unsigned long ulMeasureResponseTime; /* 2^N seconds [0 7] */
    unsigned long ulUseMultipleSets; /* 0 - Multiple IE can be sent */
    unsigned long ulEnvironment;     /* 0 - bad area, 1 - not bad, 2 mixed */
};
/*</GLML>*/

/*******************************************************************************
* GL library time sync actions
*******************************************************************************/
/*<GLML GlGpsApi.h GL_ARM_Type>*/
enum GL_ARM_ACTION
{
    GL_CHECKSTATUS,        /* Check if time sync is armed|clear */
    GL_ARM,                /* Arm the time synch */
    GL_CLEAR               /* Clear the time sych */
};
typedef enum GL_ARM_ACTION GL_ARM_ACTION;
/*</GLML>*/

/*<GLML GlGpsApi.h GL_SYNC_MODE_Type>*/
enum GL_SYNC_MODE
{
    GL_SYNC_NONE,           /* No SYNC */
    GL_SYNC_DOWNLINK,       /* Accurate provided with SYNC pulse */
    GL_SYNC_UPLINK          /* Request calibration of SYNC pulse */
};
typedef enum GL_SYNC_MODE GL_SYNC_MODE;
/*</GLML>*/

/*<GLML GlGpsApi.h GL_SYNC_STATUS_Type>*/
enum GL_SYNC_STATUS
{
    GL_SYNC_STAT_SYNC_IDLE    = 0,     /* No ongoing SYNC operation */
    GL_SYNC_STAT_SYNC_RDY     = 1<<0,  /* Ready to receive SYNC pulse */
    GL_SYNC_STAT_SYNC_RCVD    = 1<<1,  /* Received SYNC pulse */
    GL_SYNC_STAT_SYNC_VALID   = 1<<2,  /* SYNC successful */
    GL_SYNC_STAT_PRECISE_TIME = 1<<3,  /* Got Precise Time (Uplink SYNC) */
    GL_SYNC_STAT_SYNC_TOUT    = 1<<4,  /* Timed out waiting for SYNC pulse */
    GL_SYNC_STAT_SYNC_FAIL    = 1<<5,  /* Failed to arm SYNC capture */
    GL_SYNC_STAT_SYNC_ABORT   = 1<<6,  /* SYNC capture was aborted by user */
    GL_SYNC_STAT_REPORT_TIME  = 1<<7,  /* Precise time can be reported */
    GL_SYNC_STAT_REPORTED     = 1<<8,  /* Precise time was reported */
    GL_SYNC_STAT_ALL          = 0xFF   /* All flags mask */
};
typedef enum GL_SYNC_STATUS GL_SYNC_STATUS;
/*</GLML>*/

/*******************************************************************************
 *  Reference or CNTIN clock request                          
 *  Passed into the glcb_ChipsetFreqUpdate callback                  
*******************************************************************************/
/*<GLML GlGpsApi.h GL_REFCLK_REQUEST_Type>*/
enum GL_REFCLK_REQUEST
{
   GL_STARTHOLD,      /* start holding the adjustments VCO */
   GL_BREAKHOLD,      /* resume the normal VCO adjustments */
   GL_FREQ_REQUEST,   /* ask to turn the clock on */
   GL_FREQ_FORCE,     /* force the clock to be on */
   GL_FREQ_QUERY,     /* Ask what the clock is doing now */
   GL_FREQ_OFF        /* Turn off the clock */
};
typedef enum GL_REFCLK_REQUEST GL_REFCLK_REQUEST;
/*</GLML>*/

/*******************************************************************************
* Reference clk current status information
*******************************************************************************/
/*<GLML GlGpsApi.h GL_REFCLK_STAT_Type>*/
enum GL_REFCLK_STAT
{
    REFCLKSTAT_NO_INFO,   /* no info regarding the state of the ref. clk */
    REFCLKSTAT_ADJUSTING, /* refclk is being corrected with VCO adjustments */
    REFCLKSTAT_SEARCHING, /* refclk is currently searching for a network */
    REFCLKSTAT_HOLDING /* refclk is being held constant (no VCO adjustments) */
};
typedef enum GL_REFCLK_STAT GL_REFCLK_STAT;
/*</GLML>*/

/*******************************************************************************
* Reference clk status change information
*******************************************************************************/
/*<GLML GlGpsApi.h GL_REFCLK_HIST_Type>*/
enum GL_REFCLK_CHANGE
{
    REFCLKHIST_NO_INFO     = 1, /* no info regarding history of the ref. clk */
    REFCLKHIST_BTS_CHANGED = 2, /* a BTS change occurred since last glSetFreq
                                 * call
                                 */
    REFCLKHIST_TRNS_SEARCH = 4, /* a SEARCH occurred since last glSetFreq call */
    REFCLKHIST_TRNS_ADJUST = 8  /* VCO adjustments occurred since last
                                 * glSetFreq call
                                 */
};
/*</GLML>*/

/*******************************************************************************
* Platform Specificity definition
*******************************************************************************/
/*<GLML GlGpsApi.h GL_PLATFORM_SPEC_Type>*/
enum GL_PLATFORM_SPEC
{
    PLTFRM_NO_INFO         = 0x00, /* no info regarding the platform used */
    PLTFRM_SLOW_POS        = 0x01, /* Position computation takes a long time */
    PLTFRM_LRG_LATENCY     = 0x02, /* Large latency between GLL calls */
    PLTFRM_FLWCTRL_WRKARND = 0x04  /* No serial flow control: use workaround to
                                    * reduce loss of packets */
};
typedef enum GL_PLATFORM_SPEC GL_PLATFORM_SPEC;
/*</GLML>*/

/*******************************************************************************
* Freq. offset information structure
*******************************************************************************/
/*<GLML GlGpsApi.h GL_FREQ_Type>*/
typedef struct GL_FREQ GL_FREQ;
struct GL_FREQ
{
    long lFreqOff;                  /* Freq. offset from nominal [ppb] */
    GL_REFCLK_STAT ulFreqStat;      /* Current ref. clk status information */
    unsigned long ulFreqChange;     /* Ref. clk status change bit field */
};
/*</GLML>*/

/*******************************************************************************
*
*       CNTIN Status Structure
*
*******************************************************************************/

/*<GLML GlGpsApi.h GL_CNTIN_STATUS_Type>*/
typedef struct GL_CNTIN_STATUS GL_CNTIN_STATUS;
struct GL_CNTIN_STATUS
{
    long lCntinStatus;              /* 0 == OK, otherwise CNTIN is invalid */
    long lCntinOffset;              /* CNTIN frequency offset in ppb */
};
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Get the CNTIN status
*
* INPUTS:       GL_CNTIN_STATUS* pCntinStatus
*
* OUTPUTS:      GL_CNTIN_STATUS* pCntinStatus is filled with status.
*
* RETURNS:      GLRC_ERR if pCntinStatus is 0.
*               GLRC_ERR if CNTIN is not in the frequency plan.
*                           pCntinStatus->lCntinStatus = non-zero
*                           pCntinStatus->lCntinOffset = 0
*               GLRC_OK if CNTIN is in the frequency plan.
*                   If CNTIN is OK:
*                           pCntinStatus->lCntinStatus = 0
*                           pCntinStatus->lCntinOffset = the CNTIN offset
*                   If CNTIN is not OK:
*                           pCntinStatus->lCntinStatus = non-zero
*                           pCntinStatus->lCntinOffset = 0.
*
*******************************************************************************/

/*<GLML GlGpsApi.h glGetCntinStatus>*/
GLGPS_API GLRC glGetCntinStatus(GL_CNTIN_STATUS* pCntinStatus);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Set CNTIN indication flag for SUPL sessions
*
* INPUTS:       bEnable -   1 - Enable (default)
*                           0 - Disable
*
* OUTPUTS:      none
*
* RETURNS:      none
*
* NOTE:         When enabled, GLL will read current connection status of
*               SUPL session and when freq status equals to
*               REFCLKSTAT_NO_INFO, frequency will be treated as valid.
*               
*               Most likely, if we have active SUPL (TCP/IP) connection
*               we also can get CNTIN
*
*               This flag has effect only when REFCLKSTAT_NO_INFO is set
*
*******************************************************************************/

/*<GLML GlGpsApi.h glUseSuplCntInInd_Proto>*/
GLGPS_API void glUseSuplCntInInd(int bEnable);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Force CNTIN
*
* INPUTS:       None
*
* OUTPUTS:      GLRC_OK
*
*******************************************************************************/

/*<GLML GlGpsApi.h glForceCntin>*/
GLGPS_API GLRC glForceCntin();
/*</GLML>*/

/*******************************************************************************
* Definition of the MMI status bits sent for display to the application
* via the GLCB_CORR_ON call back function
*******************************************************************************/
enum GlMmiStatus
{
    MMI_CORR_ON         = 0x0001,  /* Correlation ongoing */
    MMI_PWR_ST_DOWN     = 0x0002,
    MMI_PWR_ST_PEAK     = 0x0004,
    MMI_PWR_ST_SAVING   = 0x0008,
    MMI_PWR_ST_STANDBY  = 0x0010,
    MMI_PWR_ST_MSK      = 0x003E,
    MMI_PWR_REDUCED     = 0x0040,
    MMI_PWR_32KHZ       = 0x0080,
    MMI_SER_ERR         = 0x0100
};

/*******************************************************************************
* DESCRIPTION:  Sets the estimated frequency reference offset
*
* INPUTS:       pglFreq
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetFreq_Proto>*/
GLGPS_API GLRC     glSetFreq(const GL_FREQ *pglFreq);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: Set the desired Quality of Service 
*
* INPUTS:
*
* OUTPUTS:
*
* NOTES:        This function is currently not supported
*******************************************************************************/
/*<GLML GlGpsApi.h glSetQoS_Proto2>*/
/* this function is not implemented */
/* GLGPS_API GLRC     glSetQoS(const GL_POS_INSTRUCT *pPosInstruct); */
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the maximal allowed time for the position fix
*
* INPUTS:       usTimeOut - response timeout in seconds
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetTimeOut_Proto>*/
GLGPS_API GLRC glSetTimeOut(unsigned short usTimeOut);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the maximum allowed time for the autonomous position fix  
*               for case when positioning switches from SET-based to autonomous,
*               because of SLP connection problem or inability to receive  
*               requested assistance data from RRLP server
*
* INPUTS:       usSeconds - timeout in seconds
*
* OUTPUTS:      None
*
* NOTES:        This function should not be called if SUPL is not used
*               and glSuplOnStart() was never called
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAutonomousTimeoutSec_ProtoSupl>*/
GLGPS_API void glSetAutonomousTimeoutSec(unsigned short usSeconds);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets response time grace period 
*
* INPUTS:       sPercent - allowed to exceed response timeout by that percentage
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetTimeOut_Proto>*/
GLGPS_API GLRC glSetTimeOutGracePeriod(short sPercent);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Sets the minimum time GLL will spend to obtain the fix
*
* INPUTS:       usMinFixTimeMs - Minimum time spend to obtain the fix
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetMinFixTime_Proto>*/
GLRC glSetMinFixTime(unsigned short usMinFixTimeMs);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the time spent in Standby before engaging Low Power Standby
*
* INPUTS:       usTimeOut - Standby timeout in seconds
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetStandbyTimeOut_Proto>*/
GLGPS_API GLRC glSetStandbyTimeOut(unsigned short usTimeOut);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Power down the GL chipset to its lowest power consumption
*               level.
*
* INPUTS:       None
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        OBSOLETE
*******************************************************************************/
/*<GLML GlGpsApi.h glPwrDown_Proto>*/
GLGPS_API GLRC     glPwrDown(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Latches the value of the internal millisecond counter
*
* INPUTS:       iLatchIdx - latching index (=0 or =1)
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glLatchTime_Proto1>*/
GLGPS_API GLRC glLatchTime(int iLatchIdx);
/*</GLML>*/

/****************************************************************************
* DESCRIPTION:    Provides a time tag for when the counter was latched
*
* INPUTS:        GL_TIME *pglTime    - time tag information
*                int iLatchIdx        - latching index (=0 or =1)
*
* OUTPUTS:        GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        This function is obsolete
*
*****************************************************************************/
/*<GLML GlGpsApi.h glGetLacthedTime_Proto2>*/
GLGPS_API GLRC     glGetLacthedTime( GL_TIME    *pglTime,int iLatchIdx);
/*</GLML>*/

/****************************************************************************
* DESCRIPTION:    Provides a time tag for when the counter was latched
*
* INPUTS:        WORLD_TIME *pglTime    - time tag information
*                int iLatchIdx        - latching index (=0 or =1)
*
* OUTPUTS:        GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        This function is obsolete
*               KOM:: Jul 3, 2004 
*               Renamed from glGetLacthedTime() to glGetLacthedWdTime()
*****************************************************************************/
/*<GLML GlGpsApi.h glGetLacthedWdTime_Proto>*/
GLGPS_API GLRC     glGetLacthedWdTime(WORLD_TIME *pglTime,int iLatchIdx);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Returns the current GPS time stamp
*
* INPUTS:       pglTime    - (empty) time in GL format
*
*
* OUTPUTS:      GL_TIME    *pglTime    - (full) time in GL format
*               GLRC status
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetTimeGlFmt_Proto>*/
GLGPS_API GLRC glGetTimeGlFmt(GL_TIME *pglTime);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Returns the current GPS time stamp
*
* INPUTS:       WORLD_TIME *pglTime - (empty) time in world format
*
*
* OUTPUTS:      WORLD_TIME *pglTime - (full) time in world format
*               GLRC status
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetTimeWdFmt_Proto>*/
GLGPS_API GLRC glGetTimeWdFmt(WORLD_TIME *pglTime);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Returns the current UTC time stamp
*
* INPUTS:       WORLD_TIME *pglTime - (empty) time in world format
*
*
* OUTPUTS:      WORLD_TIME *pglTime - (full) time in world format
*               GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetUtcTimeWdFmt_Proto>*/
GLGPS_API GLRC glGetUtcTimeWdFmt(WORLD_TIME *pglTime);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends the current GPS time information into the library
*
* INPUTS:       pglTime         - time tag info in GL_TIME format
*               useSync         - use time tag to sync with GPS time
*               lMaxLatencyMs   - max delay between PPS latch and
*                                 internal GPS time sync
*               long lTimeOutMs - timeout for PPS latching signal
*
* OUTPUTS:      GLRC status
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSetTimeGlFmt_Proto>*/
GLGPS_API GLRC glSetTimeGlFmt(const GL_TIME *pglTime, char useSync, 
                              long lMaxLatencyMs, long lTimeOutMs);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Gets the current Status of the pulse SYNC operation (whether
*               uplink SYNC or sdownlinkl SYNC)
*
* INPUTS:       none
*
* OUTPUTS:      GL_SYNC_STATUS SYNC status (see enum GL_SYNC_STATUS)
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGetSyncStatus_Proto>*/
GL_SYNC_STATUS glGetSyncStatus(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sends input to the latching state machine for a PPS sync input
*
* INPUTS:       glAction - action to perform, options are:
*               GL_CHECKSTATUS  - to check if latch is armed or disarmed
*               GL_ARM          - to arm the latch
*               GL_CLEAR        - to disarm the latch
*
* OUTPUTS:      GLRC status
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glArmSyncInput_Proto>*/
GLGPS_API GLRC glArmSyncInput(GL_ARM_ACTION glAction);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Converts GPS time to UTC time
*
* INPUTS:
*
* OUTPUTS:
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glGps2Utc_Proto>*/
GLGPS_API GLRC glGps2Utc(const GL_TIME *pglGpsTime, WORLD_TIME *pworldUtcTime);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Converts UTS time to GPS time
*
* INPUTS:
*
* OUTPUTS:
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glUtc2Gps_Proto>*/
GLGPS_API GLRC glUtc2Gps(const WORLD_TIME *pworldUtcTime, GL_TIME *pglGpsTime);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Returns the difference between 2 time stamps, in milliseconds
*
* INPUTS:       WORLD_TIME *pglTime1 - late time tag
*               WORLD_TIME *pglTime2 - early time tag
*
* OUTPUTS:      long Time1 - Time2
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glTimeDiff_Proto>*/
GLGPS_API long glTimeDiff(const WORLD_TIME *pglTime1, 
                          const WORLD_TIME *pglTime2);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Enables operation of the library in autonomous mode
*
* INPUTS:       bEnable
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        Function will be obsolete soon.
*               Autonomous will be enabled via glSetFixMode(GL_FIXMODE).
*******************************************************************************/
/*<GLML GlGpsApi.h glEnableAutonomous_Proto2>*/
GLGPS_API GLRC     glEnableAutonomous(char bEnable);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the frequency parameters of a frequency plan
*
* INPUTS:       bEnable
*
* OUTPUTS:      GLRC status
*
* NOTES:        The prototype will be changed to include 
*               double dPPMrate argument
*******************************************************************************/
/*<GLML GlGpsApi.h GlSFrParam_Proto>*/
GLGPS_API GLRC glSFrParam(double dPPM, unsigned short usRfFreqIn, 
                          unsigned short usAsFreqIn);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  
*
* INPUTS:       
*
* OUTPUTS:      
*
* NOTES:        
*               
*******************************************************************************/
/*<GLML GlGpsApi.h glGetAvgPwr_Proto>*/
GLGPS_API unsigned short glGetAvgPwr(void);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION: GREEN MODE
*
* INPUTS:
*
* OUTPUTS:
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h GLGFrOff_Proto>*/
GLGPS_API GLRC GLGFrOff(GL_FREQ *pglFrq);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Resets all freq dependent functions in the library
*
* INPUTS:       sClkSS - 0..99
*
* OUTPUTS:      GLRC rcStatus
*
* NOTES:        Deprecated: This function will disappear soon
*******************************************************************************/
/*<GLML GlGpsApi.h GLSClkSS_Proto>*/
GLGPS_API GLRC     GLSClkSS(short sClkSS);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Send the RRLP position results from the SMLC back into the GLL
*
* INPUTS:       const GL_RES_POS  *pPRes - position results
*
* OUTPUTS:      GLRC status
*
* NOTES:
*******************************************************************************/
/*<GLML GlGpsApi.h glSPRes_Proto>*/
GLGPS_API GLRC glSPRes(const GL_RES_POS  *pPRes);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Sets the accuracy mask in meters
*
* INPUTS:       unsigned long ulAccuracyMaskInMeters
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        GL USE ONLY
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAccuracyMask_Proto>*/
GLGPS_API GLRC glSetAccuracyMask(unsigned long ulAccuracyMaskInMeters);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Sets the accuracy mask in meters
*
* INPUTS:       short sAccThrOpenSky - Accuracy mask if open sky is detected
*               short sAccThrUrban   - Accuracy mask if urban    is detected
*               short sAccThrIndoors - Accuracy mask if indoors  is detected
* 
*               if environment is not recognized the accuracy mask set by 
*               glSetAccuracyMask() is used
*
*               Calling glSetAccuracyMask() sets all three of these masks 
*               to one specified by glSetAccuracyMask().
*               
*               To use this API call glSetAccuracyMask() first and 
*               glSetEnvirAccuracyMasks() second
*
* OUTPUTS:      GLRC status; options are GLRC_OK, GLRC_ERR, GLRC_TIMEOUT
*
* NOTES:        GL USE ONLY
*******************************************************************************/
/*<GLML GlGpsApi.h glSetAccuracyMask_Proto>*/
GLGPS_API GLRC glSetEnvirAccuracyMasks(short sAccThrOpenSky, 
                                       short sAccThrUrban, 
                                       short sAccThrIndoors);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Enables/Disables the update of NVRAM. It's enabled by default.
*
* INPUTS:       
*
* OUTPUTS:      
*
* NOTES:        TEST ONLY
*******************************************************************************/
/*<GLML GlGpsApi.h glEnableNvRamStorageUpdate_Proto>*/
GLGPS_API void glEnableNvRamStorageUpdate(int iEnable);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Sets duration of CNTIN
*
* INPUTS:       
*
* OUTPUTS:      
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glSetCntInDuration_Proto>*/
GLGPS_API void glSetCntInDuration(short sCntInDurationMs);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Enables navigation only mode 
*
* INPUTS:       sEnable - 1 enables   navigation only mode 
*                         0 disables  navigation only mode 
*
* OUTPUTS:      
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glSetNavOnly_Proto>*/
GLGPS_API GLRC glSetNavOnly(short sEnable);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  This function stores the specificities of the platform used
*               so that the GLL can tune itself to them.
*
* INPUTS        etPltfrmSpec: Specificities of current platform
*
* OUTPUTS:      none
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glSetPlatformSpec_Proto>*/
void glSetPlatformSpec(GL_PLATFORM_SPEC etPltfrmSpec);
/*</GLML>*/

/*******************************************************************************
* DESCRIPTION:  Formats the NMEA sentence
*
* INPUTS:       
*               
*
* OUTPUTS:      Returns 0 if no more NMEA sentences to be formatted for this epoch
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glFormatNmea_Proto>*/
GLGPS_API int glFormatNmea(const GL_FIX_STATUS *pfixStatus, unsigned long ulMsgMask,
                 char *pcOutBuff, short sBufSize);
/*</GLML>*/

/* <GLML GlGpsApi.h GlPeriodModes_Type>*/
typedef enum GlPeriodModes 
{
    GL_PER_NORMAL,  /* Produce fixes every  glSetFixPeriod() interval  */

    GL_PER_FILL_IN, /* Produce fixes as fast as possible. */
                    /* If position is not available */ 
                    /* the GL_HAVE_FIX_STAT_FLG  flag  would be */
                    /* set every glSetFixPeriod() interval */   

    GL_PER_NO_FILL /* Produce fixes not faster than specified by glSetFixPeriod() */   
                    /* If position is not available */ 
                    /* the GL_HAVE_FIX_STAT_FLG  flag  would be */
                    /* set upon glSetTimeOut() timeout expiration  */   
} GlPeriodModes;
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Controls the Low Power Tracking Mode
*
* INPUTS:       1 - Enable
*               0 - Disable
*               
*
* OUTPUTS:      None
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glSetLowPwrTrkMode_Proto>*/
void glSetLowPwrTrkMode(int iEnable);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Sets the mode of periodic output
*
* INPUTS:       GlPeriodModes ePeriodMode
*               
*     GL_PER_NORMAL,  Produce fixes every  glSetFixPeriod() interval  
*
*     GL_PER_FILL_IN, Produce fixes as fast as possible. 
*                      If position is not available 
*                      the GL_HAVE_FIX_STAT_FLG  flag  would be 
*                      set every glSetFixPeriod() interval 
*
*     GL_PER_NO_FILL   Produce fixes not faster than specified by glSetFixPeriod() 
*                      If position is not available 
*                      the GL_HAVE_FIX_STAT_FLG  flag  would be 
*                      set upon glSetTimeOut() timeout expiration 
*
* OUTPUTS:      None
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glEnableSmartPeriod>*/
GLGPS_API void glEnableSmartPeriod(GlPeriodModes ePeriodMode);
/*</GLML>*/


/*******************************************************************************
* DESCRIPTION:  Sets the PRN for the 1ch factory test
*
* INPUTS:       
*               
*
* OUTPUTS:      
*
* NOTES:        
*******************************************************************************/
/*<GLML GlGpsApi.h glSetSimPrn_Proto>*/
void glSetSimPrn(short sPrn);
/*</GLML>*/

enum GL_NVMEM_STATUS
{
    GL_NVMEM_STATUS_NO_INFO        = 0,  // No information is available               
    GL_NVMEM_STATUS_INTEGRITY_LOST = 1,  // The previous content of NVMEM is not 
                                         // coherent with the current status of 
                                         // the ASIC. The operation will be broken if 
                                         // this NVMEM will be read back by GLL.
    GL_NVMEM_STATUS_INTEGRITY_OK   = 2,  // The NVMEM was updated, but the previous 
                                         // state of NVMEM is still coherent.
    GL_NVMEM_STATUS_CRITICAL_DECAY = 4   // The state of NVMEM is still coherent, but 
                                         
};
typedef enum GL_NVMEM_STATUS GL_NVMEM_STATUS;

/*******************************************************************************
* DESCRIPTION:  Returns the status of the NVMEM data 
*
* INPUTS:       None
*               
*
* OUTPUTS:      Status of the NVMEM data. 
*
* NOTES:        Status is reset to 0 each time this API is called.
*******************************************************************************/
/*<GLML GlGpsApi.h glGetNvMemStatus>*/
GL_NVMEM_STATUS glGetNvMemStatus(void);
/*</GLML>*/




/***************************************************************************
*
*
* DESCRIPTION:  Back door functions
*
*       These back door functions manipulate the contents of NVRAM storage 
*       for testing purposes.
*
* INPUTS:       various
*
* OUTPUTS:      None
*
* NOTES:        GL USE ONLY
*
****************************************************************************/

/*<GLML GlGpsApi.h glbdNvramPosToPast_Proto>*/
GLGPS_API void glbdNvramPosToPast(short sh);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdNvramSetLLA_Proto>*/
GLGPS_API void glbdNvramSetLLA(double d1, double d2, double d3);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdNvramDelNav_Proto>*/
GLGPS_API void glbdNvramDelNav(void);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdNvramDelStdby_Proto>*/
void glbdNvramDelStdby(void);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdNvramDelAlm_Proto>*/
void glbdNvramDelAlm(void);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdTrAid>*/
GLGPS_API void glbdTrAid(short sTrAid);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdOffClk>*/
void glbdOffClk(short sOffset);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdIgnoreStdbyTime_Proto>*/
GLGPS_API void glbdIgnoreStdbyTime(void);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdLowPwrStdby>*/
void glbdLowPwrStdby(short sEnable);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdEnableClamp>*/
void glbdEnableClamp(short sEnable);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdSetLowSnrVlitMode>*/
void glbdSetLowSnrVlitMode(char cEnable);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdSetCloseOpenLoop>*/
void glbdSetCloseOpenLoop(char cCloseOpenLoop);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdSetSoftAcc>*/
void glbdSetSoftAcc(unsigned char ucSoftAcc);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdSetRfTestMode>*/
void glbdSetRfTestMode(unsigned char ucRfTestMode);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdEnableGsvArm>*/
void glbdEnableGsvArm(int bEnable);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdFrcNvUp>*/
void glbdFrcNvUp(int bForce);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdForceFarStart>*/
void glbdForceFarStart(void);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdSimBitErr>*/
void glbdSimBitErr(int bEnable);
/*</GLML>*/

/*<GLML GlGpsApi.h glbdSetIfClk_Proto>*/
GLGPS_API void glbdSetIfClk(unsigned long ulIfClk);
/*</GLML>*/

#define GL_NVRAM_MAX_SIZE 50000 /*in bytes */

/*******************************************************************************
* ASIC type information for library ********************************************
*******************************************************************************/

#define GL_RAW_MSMT_SIZE ((2 + 8 + 30*14)*4)
/*<GLML GlGpsApi.h GL_RAW_MSMT_Type>*/
typedef struct GL_RAW_MSMT
{
    char  acMsmtBuff[GL_RAW_MSMT_SIZE];
    short sSize;
} GL_RAW_MSMT;
/*</GLML>*/

#define GL_RAW_NAV_SIZE  ((2 + 36)*4)
/*<GLML GlGpsApi.h GL_RAW_NAV_Type>*/
typedef struct GL_RAW_NAV
{
    char acNavBuff[GL_RAW_NAV_SIZE];
    short sSize;
} GL_RAW_NAV;
/*</GLML>*/

#define GL_RAW_POS_SIZE  ((2 + 19 + 6)*4)
/*<GLML GlGpsApi.h GL_RAW_POS_Type>*/
typedef struct GL_RAW_POS
{
    char acPosBuff[GL_RAW_POS_SIZE];
    short sSize;
} GL_RAW_POS;
/*</GLML>*/


/*<GLML GlGpsApi.h GL_TIME_KEEPER_MASK_Type>*/
typedef enum GL_TIME_KEEPER_MASK
{
    GL_KEEP_ALL = 0x0000,
    GL_DROP_US  = 0x0001,   /* uSecs */
    GL_DROP_MS  = 0x0002,   /* mSecs */
    GL_DROP_HS  = 0x0004,   /* hundredth of seconds */
    GL_DROP_SS  = 0x0008    /* Seconds */
} GL_TIME_KEEPER_MASK;
/*</GLML>*/


/*<GLML GlGpsApi.h glGRM_Proto>*/
GLGPS_API GLRC glGRM(GL_RAW_MSMT *pglRM);
/*</GLML>*/
/*<GLML GlGpsApi.h glGRN_Proto>*/
GLGPS_API GLRC glGRN(GL_RAW_NAV  *pglRN);
/*</GLML>*/
/*<GLML GlGpsApi.h glGRP_Proto>*/
GLGPS_API GLRC glGRP(GL_RAW_POS  *pglRP);
/*</GLML>*/

/*******************************************************************************
* GLL Callbacks ****************************************************************
*******************************************************************************/

/*******************************************************************************
* FUNCTIONAL AREA: Memory allocation
* OPERATION:       Memory allocation
* DESCRIPTION:     Request pointer to allocated memory (static or dynamic).
* NOTE:            Reauested memory will be always the same for particular GLL
*                  version and it will always request the same amount of memory 
*                  in the same sequence
*
*                  In case if memory allocation callbacks are not implemented, 
*                  internal static memory will be used
*******************************************************************************/
void *glcb_Allocate(unsigned int size);

/*******************************************************************************
* FUNCTIONAL AREA: Memory allocation
* OPERATION:       Memory allocation
* DESCRIPTION:     Previously allocated memory can be freed and used for other 
*                  purposes by application
*
*                  In case if memory allocation callbacks are not implemented, 
*                  internal static memory will be used
*******************************************************************************/
void glcb_Deallocate(void *memblock);

/*******************************************************************************
* FUNCTIONAL AREA: NMEA
* OPERATION:       Report NMEA sentence
* DESCRIPTION:     Report NMEA sentence
*******************************************************************************/
void glcb_Nmea(const char *nmea);

/*******************************************************************************
* FUNCTIONAL AREA: Reporting
* OPERATION:       report
* DESCRIPTION:     Reports resulting position from single shot request
*******************************************************************************/
void glcb_OnSingleShot(GL_HANDLE hRequestHandle);

/*******************************************************************************
* FUNCTIONAL AREA: Reporting
* OPERATION:       report
* DESCRIPTION:     Reports resulting positions from periodic request
*******************************************************************************/
void glcb_OnPeriodicPosition(GL_HANDLE hRequestHandle);

/*******************************************************************************
* FUNCTIONAL AREA: Reporting
* OPERATION:       report
* DESCRIPTION:     Reports LTO download progress
*******************************************************************************/
void glcb_OnLtoDownload(int iProgressPercent);

/*******************************************************************************
* FUNCTIONAL AREA: Reporting
* OPERATION:       report
* DESCRIPTION:     Reports test mode results
*******************************************************************************/
void glcb_OnTestMode(GL_HANDLE hRequestHandle);

/*******************************************************************************
* FUNCTIONAL AREA: Reporting
* OPERATION:       report
* DESCRIPTION:     Reports Platform test mode results
* INPUTS:          hRequestHandle: handle of the test request
*                  etStatus:       see definition in enum GlPlatformTestStatus
*                  ulNbTestDone:   nb of test that were performed
*                  ulNbFailed:     nb of test that failed
*                  ulTotalTestTime Total test time in ms
*                  lCpuTimeDrift   CPU timer ms drift (GL_TST_CPU_TIMER only)
*                  usAvgLat:       average serial latency (GL_TST_CPU_TIMER only)
*******************************************************************************/
void glcb_OnPlatformTest(GL_HANDLE hRequestHandle, GlPlatformTestStatus etStatus,
                    unsigned long ulNbTestDone, unsigned long ulNbFailed,
                    unsigned long ulTotalTestTime, long lCpuTimeDrift,
                    unsigned short usAvgLat);

/*******************************************************************************
* FUNCTIONAL AREA: Exception handling 
* OPERATION:       assert
* DESCRIPTION:     hanlde assertion failure in the GLL
*******************************************************************************/
void glcb_ExceptionAssert(char const *file, unsigned line);

/*******************************************************************************
* FUNCTIONAL AREA: Diagnostics 
* OPERATION:       append
* DESCRIPTION:     append nBytes to the GLL diagnostics buffer 
*******************************************************************************/
void glcb_DiagAppend(unsigned nBytes);

/*******************************************************************************
* FUNCTIONAL AREA: Diagnostics 
* OPERATION:       lock
* DESCRIPTION:     lock the diagnostics buffer 
*******************************************************************************/
void glcb_DiagLock(void);

/*******************************************************************************
* FUNCTIONAL AREA: Diagnostics 
* OPERATION:       unlock
* DESCRIPTION:     unlock the diagnostics buffer 
*******************************************************************************/
void glcb_DiagUnlock(void);

/*******************************************************************************
* FUNCTIONAL AREA: Timer 
* OPERATION:       get
* DESCRIPTION:     Get system 32-bit timer in milliseconds
*******************************************************************************/
unsigned long glcb_TimerGet(void);

/*******************************************************************************
* FUNCTIONAL AREA: Timer 
* OPERATION:       micro sleep
* DESCRIPTION:     sleep for the given number of microseconds 
*                  (never more than 10)
* NOTE:            this callback is typically implemented as a busy-wait
*                  loop that depends on the target CPU speed.
* WARNING: This callback is not used anymore by the GLL and doesn't need to be
*          implemented with newer GLL version. It is left here for backward
*          compatibility to prevent compilation error in applications that
*          were originally developped with older GLL.
*******************************************************************************/
void glcb_TimerMicroSleep(unsigned microsec);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset/RF 
* OPERATION:       initialize
* DESCRIPTION:     Initialize 3rd-party (non GL) RF frontend.
*                  Return 1 if initialization performed, 0 if not
*******************************************************************************/
int glcb_ChipsetRfInit(void);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset/RF 
* OPERATION:       set
* DESCRIPTION:     Set the input frequency to the RF board PLL
*                  Return 1 if input frequency set, 0 if not
*******************************************************************************/
int glcb_ChipsetRfPllSet(unsigned short pllInputFreq);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset 
* OPERATION:       write
* DESCRIPTION:     write block of data to the GL Baseband Processor
*******************************************************************************/
void glcb_ChipsetWrite (const unsigned char *data, unsigned nBytes);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset 
* OPERATION:       read
* DESCRIPTION:     read block of data from the GL Baseband Processor
*******************************************************************************/
int glcb_ChipsetRead(unsigned char **ppasicBytes);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset 
* OPERATION:       provide status
* DESCRIPTION:     invoked by the GLL when Baseband processor status changes
*                  (e.g., Baseband Processor changes power level)
*******************************************************************************/
void glcb_ChipsetStatus(unsigned long status);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset/PIO 
* OPERATION:       set
* DESCRIPTION:     set the nBit bit 
*******************************************************************************/
void glcb_ChipsetPioSet(unsigned nBit);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset/PIO 
* OPERATION:       clear
* DESCRIPTION:     clear the nBit bit 
*******************************************************************************/
void glcb_ChipsetPioClear(unsigned nBit);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset/PIO 
* OPERATION:       read
* DESCRIPTION:     read the PIO bit nBit and return 1 if set, 0 if cleared 
* NOTE: This is an obsolete callback, not used in the GLL anymore
*******************************************************************************/
int glcb_ChipsetPioRead(unsigned nBit);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset/PIO 
* OPERATION:       availability check
* DESCRIPTION:     check if the nBit PIO bit is available
*                  Return 1 if the PIO bit available, 0 if not
* NOTE: This is an obsolete callback, not used in the GLL anymore
*******************************************************************************/
int glcb_ChipsetPioAvailable(unsigned nBit);

/*******************************************************************************
* FUNCTIONAL AREA: Non-Volatile Memory 
* OPERATION:       seek into the NV-Memory
* DESCRIPTION:     Set the input frequency to the RF board PLL
*                  Return 1 if the operation performed, 0 if not
*******************************************************************************/
int glcb_NvSeek(unsigned offset);

/*******************************************************************************
* FUNCTIONAL AREA: Non-volatile RAM 
* OPERATION:       read byte
* DESCRIPTION:     read byte from the NV-RAM file at the current position
*                  and advance the current position 
*                  Return 0x100 if the byte cannot be read
*******************************************************************************/
int glcb_NvReadByte(void);

/*******************************************************************************
* FUNCTIONAL AREA: Non-Volatile Memory 
* OPERATION:       write byte
* DESCRIPTION:     write byte 'b' to the NV-RAM file at the current position
*                  and advance the current position 
*                  Return 0 if the byte cannot be written
*******************************************************************************/
int glcb_NvWriteByte(unsigned char byte);

/*******************************************************************************
* FUNCTIONAL AREA: Non-Volatile Memory 
* OPERATION:       flush
* DESCRIPTION:     flush the NV-RAM 
*                  Return 1 if the operation performed, 0 if not
*******************************************************************************/
int glcb_NvFlush(void);

/*******************************************************************************
* FUNCTIONAL AREA: Debugging
* OPERATION:       report
* DESCRIPTION:     reports the rawdata used by GLL for subsequent processing 
*                  by GL analyzis tools 
*******************************************************************************/
void glcb_RawDataReport(void);

/*******************************************************************************
* FUNCTIONAL AREA: RRLP 
* OPERATION:       get buffer
* DESCRIPTION:     called by the RRLP engine to encode the outgoing PDU 
*******************************************************************************/
GLRC glcb_RrlpGetBuffer(unsigned char **pBuf, unsigned *pMaxBytes);

/*******************************************************************************
* FUNCTIONAL AREA: RRLP 
* OPERATION:       encode
* DESCRIPTION:     called by the RRLP engine to write the encoded bytes
*******************************************************************************/
GLRC glcb_RrlpWrite(const unsigned char *data, unsigned nBytes);

/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       get buffer
* DESCRIPTION:     Called by the SUPL engine when encoding an outgoing 
*                  PDU (Protocol Data Unit). The GLL SUPL protocol stack fills 
*                  this buffer with SUPL PDU, then sends the PDU to the network 
*                  using glcb_SuplWrite(). 
* NOTE:            This buffer must be big enough to handle maximal possible 
*                  PDU size.  Recommended size is at least 2048 bytes.
*                  The HAL can understand the size  actually used by examining 
*                  parameter  "nBytes"  of the next glcb_SuplWrite callback.
*                  This buffer not used by SUPL Engine after glcb_SuplWrite.
*******************************************************************************/
GLRC glcb_SuplGetBuffer(unsigned char **pBuf, unsigned *pMaxBytes);

/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       encode
* DESCRIPTION:     called by the SUPL engine to write the encoded bytes  to 
*                  the network. 
* NOTE:            Parameter "data" is an address to a buffer allocated
*                  in glcb_SuplGetBuffer.
*                  This buffer not used by SUPL Engine after glcb_SuplWrite.
*******************************************************************************/
GLRC glcb_SuplWrite(const unsigned char *data, unsigned nBytes);

/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       request CID
* DESCRIPTION:     called by the SUPL engine to request the Cell Information
*******************************************************************************/
GLRC glcb_SuplReqLocationId(void);

/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       report event when GLL is switching to autonomous mode
* DESCRIPTION:     called by the SUPL engine to report event when GLL is 
*                  switching to autonomous mode
*******************************************************************************/
void glcb_SuplSwitchedToAutonomousMode(void);

/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       request TCP/IP Connection to SLP server
* DESCRIPTION:     called by the SUPL engine to request SUPL Connection
*                  SUPL Protocol stack waiting for glSuplConnect call which 
*                  will inform about the result of this request
*******************************************************************************/
/*<NO_GLML GlGpsApi.h GlSuplConnectionType_Type>*/
typedef enum GlSuplConnectionType
{
    /* SLP address should be considered by client.  */
    /* This type can be used with SET originated    */
    /* connections when SLP address is unknown to   */
    /* build-in SUPL engine                         */
    GL_SUPL_CONNECTION_DEFAULT,

    GL_SUPL_CONNECTION_URL,         /* URL */
    GL_SUPL_CONNECTION_IPv4,        /* IP Address v4 */
    GL_SUPL_CONNECTION_IPv6         /* IP Address v6 */
} GlSuplConnectionType;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSuplConnectionPriority_Type>*/
typedef enum GlSuplConnectionPriority
{
    GL_SUPL_CONNECTION_PRIORITY_LOW,
    GL_SUPL_CONNECTION_PRIORITY_HIGH
} GlSuplConnectionPriority;
/*</NO_GLML>*/

/*<NO_GLML GlGpsApi.h GlSuplConnection_Type>*/
typedef struct GlSuplConnection GlSuplConnection;
struct GlSuplConnection
{
    GlSuplConnectionType        eType;      /* Connection Type      */
    GlSuplConnectionPriority    ePriority;  /* Connection priority  */
    union
    {
        /* URL, memory controlled by GLL. Client should     */
        /* make a copy if he wants to preserve a this value */
        /* Just a pointer can not be used for this purpose  */
        const char      *url;

        unsigned char   ipv4[4];    /* IP address v4 */
        unsigned char   ipv6[16];   /* IP address v6 */
    } u;
};
/*</NO_GLML>*/

GLRC glcb_SuplReqConnection(const GlSuplConnection *pSuplConnection);

/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       inform that SLP requests for Notification & Verification
* DESCRIPTION:     called by the SUPL engine after receiving SUPL-INIT message
*
* NOTE:            requestorText and notificationText are allocated
*                  on stack and should be stored by application
*                  both pointers will be invalid after end of this function
*
*                  Application should response with glSuplVerificationRsp only 
*                  if notification types are 
*                  GL_SUPL_NOTIFICATION_AND_VERFICATION_ALLOWED or 
*                  GL_SUPL_NOTIFICATION_AND_VERFICATION_DENIED
*
*******************************************************************************/
/*<GLML GlGpsApi.h GlSuplNotificationType_Type>*/
typedef enum GlSuplNotificationType
{
    GL_SUPL_NO_NOTIFICATION_NO_VERIFICATION,
    GL_SUPL_NOTIFICATION_ONLY,
    GL_SUPL_NOTIFICATION_AND_VERFICATION_ALLOWED,
    GL_SUPL_NOTIFICATION_AND_VERFICATION_DENIED,
    GL_SUPL_PRIVACY_OVERRIDE,
    GL_SUPL_NOTIFICATION_DONE = -1
} GlSuplNotificationType;
/*</GLML>*/

/*<GLML GlGpsApi.h GlSuplNotificationEncodingType_Type>*/
typedef enum GlSuplNotificationEncodingType
{
    GL_SUPL_NOTIF_ENCODE_UCS2,             /* UCS2 encoding            */
    GL_SUPL_NOTIF_ENCODE_GSM               /* GSM default encoding     */
} GlSuplNotificationEncodingType;
/*</GLML>*/

void glcb_SuplNotificationVerificarionReq(
        GlSuplNotificationType         eNotificationType, 
        GlSuplNotificationEncodingType eEncodingType,
        const unsigned char *requestorText,   int requestorTextLength,
        const unsigned char *notificationText,int notificationTextLength);

/* correction for typo */
#define glcb_SuplNotificationVerificationReq glcb_SuplNotificationVerificarionReq


/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       inform that SLP requests for Notification & Verification
* DESCRIPTION:     called by the PreSUPL engine after receiving LCSINIT message
*
* NOTE:            requestorText and notificationText are allocated
*                  on stack and should be stored by application
*                  both pointers will be invalid after end of this function
*
*                  Application should response with glSuplVerificationRsp only 
*                  if notification type is 
*                  PreSUPL_notifyAndVerify_locationNotAllowedIfNoResponse
*
*******************************************************************************/
/*<GLML GlGpsApi.h GlPreSuplNotificationType_Type>*/
typedef enum GlPreSuplNotificationType
{
    GL_PreSUPL_noNotification_NoVerification = 0,
    GL_PreSUPL_notificationOnly = 1,
    GL_PreSUPL_notifyAndVerify_locationNotAllowedIfNoResponse = 2,
    GL_PreSUPL_privacyOverride = 3
} GlPreSuplNotificationType;
/*</GLML>*/

/*<GLML GlGpsApi.h GlPreSuplNotificationEncodingType_Type>*/
typedef enum GlPreSuplNotificationEncodingType
{
    GL_PreSUPL_iso646irv = 0,
    GL_PreSUPL_iso8859 = 1,
    GL_PreSUPL_utf8 = 2,
    GL_PreSUPL_utf16 = 3,
    GL_PreSUPL_ucs2 = 4,
    GL_PreSUPL_gsm_default = 5,
    GL_PreSUPL_shift_jis = 6,
    GL_PreSUPL_jis = 7,
    GL_PreSUPL_euc = 8,
    GL_PreSUPL_gb2312 = 9,
    GL_PreSUPL_cns11643 = 10,
    GL_PreSUPL_ksc1001 = 11
} GlPreSuplNotificationEncodingType;
/*</GLML>*/

void glcb_PreSuplNotificationVerificarionReq(
        GlPreSuplNotificationType         eNotificationType, 
        GlPreSuplNotificationEncodingType eEncodingType,
        const unsigned char *notificationText,int notificationTextLength);

/* correction for typo */
#define glcb_PreSuplNotificationVerificationReq glcb_PreSuplNotificationVerificarionReq

/*******************************************************************************
* FUNCTIONAL AREA: SUPL 
* OPERATION:       inform that connection to SLP server can be closed
* DESCRIPTION:     called by the SUPL engine to inform that SUPL connection
*                  is no longer necessary. 
*******************************************************************************/
void glcb_SuplEndConnection(void);

/*******************************************************************************
* FUNCTIONAL AREA: GPS Chipset 
* OPERATION:       
* DESCRIPTION:     Communicates to app the GLL needs on Ref or CNTIN clock
*                  Returns GLRC_ERR if not supported or unable to perform an 
*                  operation
*******************************************************************************/
GLRC  glcb_ChipsetFreqUpdate(GL_REFCLK_REQUEST etRefClkReq);

/****************************************************************************
* NAME:           glDistance()
*
* DESCRIPTION:    Calculates the geodetic distance between the two points 
*                 according to the ellipsoid model of WGS84.
*                 Altitude is neglected from calculations
*
* INPUTS          coordinates of two points
*
* OUTPUTS:        the distance to the destination in meters
* NOTES:          return -1 indicates the invalid passing parameters.
*
*****************************************************************************/
double glDistance(GL_ASS_POS *pFrom, GL_ASS_POS *pTo);

/******************************************************************************
*
* NAME:           glAzimuthTo()
*
* DESCRIPTION:
*                 Calculates the azimuth between the two points according to 
*                 the ellipsoid model of WGS84. The azimuth is relative to
*                 true north.  The Coordinates object on which this method
*                 is called is considered the origin for the calculation and
*                 the Coordinates object passed as a parameter is the
*                 destination which the azimuth is calculated to.  When the
*                 origin is the North pole and the destination is not the
*                 North pole, this method returns 180.0.  When the origin
*                 is the South pole and the destination is not the South
*                 Pole, this method returns 0.0.  If the origin is equal to
*                 the destination, this method returns 0.0.  The implemen-
*                 tation shall calculate the result as exactly as it can.
*                 However, it is required that the result is within 1 degree
*                 of the correct result
*
* INPUTS          coordinates of two points
*
* OUTPUTS:        the azimuth to the destination in degrees. 
*                 Result is within the range [0.0 ,360.0).
* NOTES:          return -1 indicates the invalid passing parameters.
*
*******************************************************************************/
double glAzimuthTo(GL_ASS_POS *pFrom, GL_ASS_POS *pTo);

/*****************************************************************************
* DESCRIPTION:  Set the time advance to apply to Standby time recovery
*               mechanism to simulate a large time gap.
*
*
*****************************************************************************/
char glGetLtoDownloadMode(void);

/****************************************************************************
* NAME:            glClearLto
*
* DESCRIPTION:     This function clears all Lto slices in NVRAM.
* INPUTS           None
*
* OUTPUTS:         None
*
* NOTES:
*
*****************************************************************************/
void glClearLto(void);

/*****************************************************************************
* DESCRIPTION:  Set the time advance to apply to Standby time recovery
*               mechanism to simulate a large time gap.
*
* INPUTS:       ulTimeAdv: time advance to apply to the Standby elapsed time.
*
* OUTPUTS:      none
*****************************************************************************/
void glSetStdbyTimeAdvance(unsigned long ulTimeAdv);

/*****************************************************************************
* DESCRIPTION:  Get the time advance to apply in glcb_timerGet() to
*               simulate a large time gap.
*
* INPUTS:       none
*
* OUTPUTS:      ulTimerAdvance: time advance to apply in glcb_timerGet()
*****************************************************************************/
unsigned long glGetTimerAdvance(void);

#ifdef __cplusplus
}
#endif

#ifdef GLL_COMP_LVL /* did Clients request a certain compatiblity level? */
#include "glgpsold.h"
#endif

/* FIXME-SP Must be gone... */
#define SetLogFacMask glSetLogFacMask
#define SetLogPriMask glSetLogPriMask
#define GlSysLog      glSysLog
#define GlSFrPln      glSFrPln
#define GlSFrParam    glSFrParam

#endif /* GLGPSAPI_H */
