//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ata_time.h
//
//  Define the timing value for ATA module.
//
//------------------------------------------------------------------------------

#ifndef _ATA_TIME_H_
#define _ATA_TIME_H_

// PIO spec
int t0_spec[5]   = {600, 383, 240, 180, 120} ;
int t1_spec[5]   = {70, 50, 30, 30, 25} ;
int t2_8spec[5]  = {290, 290, 290, 80, 70} ;
int t2_16spec[5] = {165, 125, 100, 80, 70} ;
int t2i_spec[5]  = {0,0,0,0,0} ;
int t4_spec[5]   = {30, 20, 15, 10, 10} ;
int t9_spec[5]   = {20, 15, 10, 10, 10} ;
int tA_spec[5]   = {50,50,50,50,50} ;

// MDMA spec
int t0M_spec[3]  = {480, 150, 120} ;
int tD_spec[3]   = {215, 80, 70} ;
int tH_spec[3]   = {20, 15, 10} ;
int tJ_spec[3]   = {20, 5, 5} ;
int tKW_spec[3]  = {215, 50, 25} ;
int tM_spec[3]   = {50, 30, 25} ;
int tN_spec[3]   = {15, 10, 10} ;
int tJNH_spec[3] = {20, 15, 10} ;

// UDMA spec
int t2CYC_spec[6]   = {235, 156, 117, 86, 57, 38} ;
int tCYC_spec[6]    = {114, 75, 55, 39, 25, 17} ;
int tDS_spec[6]     = {15,10,7, 7, 5, 4} ;
int tDH_spec[6]     = {5,5,5,5,5,5} ;
int tDVS_spec[6]    = {70, 48, 34, 20, 7, 5} ;
int tDVH_spec[6]    = {6,6,6,6,6,6} ;
int tCVS_spec[6]    = {70,48,34,20,7,10} ;
int tCVH_spec[6]    = {6,6,6,6,6,10} ;
int tFS_minspec[6]  = {0,0,0,0,0,0} ;
int tLI_maxspec[6]  = {100,100,100,100,100,75} ;
int tMLI_spec[6]    = {20,20,20,20,20,20} ;
int tAZ_spec[6]     = {10,10,10,10,10,10} ;
int tZAH_spec[6]    = {20,20,20,20,20,20} ;
int tENV_minspec[6] = {20,20,20,20,20,20} ;
int tSR_spec[6]     = {50,30,20,20,20,20} ;
int tRFS_spec[6]    = {75, 70, 60, 60, 60, 50} ;
int tRP_spec[6]     = {160,125,100,100,100,85} ;
int tACK_spec[6]    = {20,20,20,20,20,20} ;
int tSS_spec[6]     = {50,50,50,50,50,50} ;
int tDZFS_spec[6]   = {80,63,47,35,25,40} ;

#endif _ATA_TIME_H_
