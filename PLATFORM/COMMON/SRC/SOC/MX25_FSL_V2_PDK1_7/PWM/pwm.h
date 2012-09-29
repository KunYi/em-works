#ifndef _PWM_H_
#define _PWM_H_

//Registry Defines
#define REG_INDEX_STRING    TEXT("Index")

typedef struct {
    PCSP_PWM_REG					pPwmRegs;
    DWORD									dwIndex;
    CRITICAL_SECTION				hPwmLock;
    CEDEVICE_POWER_STATE		CurrentDx;
	//
	// CS&ZHL AUG-17-2011: variances for EM9170
	//
	DWORD		dwClockSource;		//
	DWORD		dwPreScaler;				// = 1.. 4096
	DWORD		dwLastPreScaler;		// = 1.. 4096
	DWORD		dwLastPeriod;
}PWM_DEVICE_CONTEXT, *PPWM_DEVICE_CONTEXT;

typedef struct {
   PPWM_DEVICE_CONTEXT pPwm;
}PWM_OPEN_CONTEXT, *PPWM_OPEN_CONTEXT;

#endif _PWM_H_
