#ifndef __EXTERN_ETA108_H__   
#define __EXTERN_ETA108_H__

#ifdef __cplusplus
extern "C" {
#endif

#define  ADCHA1	1<<0
#define  ADCHA2	1<<1
#define  ADCHA3	1<<2
#define  ADCHA4	1<<3
#define  ADCHA5	1<<4
#define  ADCHA6	1<<5
#define  ADCHA7	1<<6
#define  ADCHA8	1<<7

typedef struct  
{
	DWORD dwSamplingRate;			//Sampling rate
	DWORD dwSamplingLength;			//Sampling length: Count should be in UINT16
	DWORD dwSamplingChannel;		//Sampling channel
	LPVOID lpContrlWord;			//Reserved
	DWORD  dwContrlWordLength;		//Reserved
} ADS_CONFIG, *PADS_CONFIG;
	

#ifdef __cplusplus
}
#endif

#endif __EXTERN_ETA108_H__