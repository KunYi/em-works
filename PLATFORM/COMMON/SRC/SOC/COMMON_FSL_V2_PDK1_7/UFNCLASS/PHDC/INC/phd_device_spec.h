/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************//*!
 *
 * @file phd_device_spec.h
 *
 * @author
 *
 * @version
 *
 * @date June-16-2009

 * @brief This file contains macros and constants required by the various 
 *        device specializations
 *****************************************************************************/

#ifndef _PHD_DEVICE_SPEC_H
#define _PHD_DEVICE_SPEC_H

#include "types_phdc.h"
#include "ieee11073_phd_types.h"    
#include "ieee11073_nom_codes.h"    

#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(push)
/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
/* Weigh Scale */
#define PHD_WSL_CNFG_EVT_RPT_SIZE           166
#define PHD_WSL_DIM_GET_RSP_SIZE            114
#define PHD_WSL_DIM_DATA_TX_SIZE            94

/* Glucometer */
#define PHD_GLUCO_CNFG_EVT_RPT_SIZE         158
#define PHD_GLUCO_DIM_GET_RSP_SIZE          114
#define PHD_GLUCO_DIM_DATA_TX_SIZE          104

/* Blood pressure monitor */
#define PHD_BPM_CNFG_EVT_RPT_SIZE           152
#define PHD_BPM_DIM_GET_RSP_SIZE            114
#define PHD_BPM_DIM_DATA_TX_SIZE            66

/* Thermometer */
#define PHD_THERMO_CNFG_EVT_RPT_SIZE        72
#define PHD_THERMO_DIM_GET_RSP_SIZE         92
#define PHD_THERMO_DIM_DATA_TX_SIZE         46

/* pulse oximeter */
#define PHD_PULSE_CNFG_EVT_RPT_SIZE         172
#define PHD_PULSE_DIM_GET_RSP_SIZE          114
#define PHD_PULSE_DIM_DATA_TX_SIZE          58

/*****************************************************************************
 * Types
 *****************************************************************************/
#pragma pack(1)
/* structure for the measurements that are changing */
typedef struct _phd_measurement
{
    AbsoluteTime msr_time;
    uint_16 weight[2];
    uint_16 bmi[2];
}PHD_MEASUREMENT, *PHD_MEASUREMENT_PTR;

/*****************************************************************************
 * Global variables 
 *****************************************************************************/
extern uint_8 const PHD_WSL_ASSOC_REQ[ASSOC_REQ_SIZE];   
extern uint_8 const PHD_WSL_CNFG_EVT_RPT[PHD_WSL_CNFG_EVT_RPT_SIZE];
extern uint_8 const PHD_WSL_REL_REQ[REL_REQ_SIZE];
extern uint_8 const PHD_WSL_REL_RES[REL_RES_SIZE];   
extern uint_8 const PHD_WSL_DIM_GET_RSP[PHD_WSL_DIM_GET_RSP_SIZE];
extern uint_8 const PHD_WSL_DIM_DATA_TX[PHD_WSL_DIM_DATA_TX_SIZE];   
extern uint_8 const PHD_GLUCO_ASSOC_REQ[ASSOC_REQ_SIZE];
extern uint_8 const PHD_GLUCO_CNFG_EVT_RPT[PHD_GLUCO_CNFG_EVT_RPT_SIZE];
extern uint_8 const PHD_GLUCO_REL_REQ[REL_REQ_SIZE];
extern uint_8 const PHD_GLUCO_REL_RES[REL_RES_SIZE];   
extern uint_8 const PHD_GLUCO_DIM_GET_RSP[PHD_GLUCO_DIM_GET_RSP_SIZE];
extern uint_8 const PHD_GLUCO_DIM_DATA_TX[PHD_GLUCO_DIM_DATA_TX_SIZE];   
extern uint_8 const PHD_BPM_ASSOC_REQ[ASSOC_REQ_SIZE];   
extern uint_8 const PHD_BPM_CNFG_EVT_RPT[PHD_BPM_CNFG_EVT_RPT_SIZE];   
extern uint_8 const PHD_BPM_REL_REQ[REL_REQ_SIZE];   
extern uint_8 const PHD_BPM_REL_RES[REL_RES_SIZE];   
extern uint_8 const PHD_BPM_DIM_GET_RSP[PHD_BPM_DIM_GET_RSP_SIZE];   
extern uint_8 const PHD_BPM_DIM_DATA_TX[PHD_BPM_DIM_DATA_TX_SIZE];   
extern uint_8 const PHD_THERMO_ASSOC_REQ[ASSOC_REQ_SIZE];   
extern uint_8 const PHD_THERMO_CNFG_EVT_RPT[PHD_THERMO_CNFG_EVT_RPT_SIZE];   
extern uint_8 const PHD_THERMO_REL_REQ[REL_REQ_SIZE];   
extern uint_8 const PHD_THERMO_REL_RES[REL_RES_SIZE];   
extern uint_8 const PHD_THERMO_DIM_GET_RSP[PHD_THERMO_DIM_GET_RSP_SIZE];   
extern uint_8 const PHD_THERMO_DIM_DATA_TX[PHD_THERMO_DIM_DATA_TX_SIZE];   
/*****************************************************************************
 * Global Functions - None
 *****************************************************************************/
#pragma pack(pop)
#ifdef __cplusplus
}
#endif
#endif
