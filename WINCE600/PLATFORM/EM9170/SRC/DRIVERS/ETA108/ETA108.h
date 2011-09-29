#ifndef __EXTERN_ETA108_H__   
#define __EXTERN_ETA108_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  
{
	DWORD dwSamplingRate;			//Sampling rate
	DWORD dwSamplingLength;			//Sampling length: Count should be in UINT16
	DWORD dwSamplingChannel;		//Sampling channel
	LPWSTR lpADCompleteEvent;		//AD conversion complete event
	DWORD  dwADCompleteEventLength;
	LPVOID lpContrlWord;			//Reserved
	DWORD  dwContrlWordLength;		//Reserved
} ADS_CONFIG, *PADS_CONFIG;
	

#ifdef __cplusplus
}
#endif

#endif __EXTERN_ETA108_H__