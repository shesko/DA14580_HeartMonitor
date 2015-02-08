/**
 ****************************************************************************************
 *
 * @file da14580_config.h
 *
 * @brief Compile configuration file.
 *
 * Copyright (C) 2014. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef DA14580_CONFIG_H_
#define DA14580_CONFIG_H_

#include "da14580_stack_config.h"

/////////////////////////////////////////////////////////////
/*FullEmbedded - FullHosted*/
#define CFG_APP  
/////////////////////////////////////////////////////////////
/* Proprietary streamdata host profile role */
#define CFG_PRF_SPS_CLIENT
/* Proprietary streamdata device profile role */
#undef CFG_PRF_SPS_SERVER
/////////////////////////////////////////////////////////////
/*UART flow control configuration. One must be enabled when*/
/*sleep is enabled.*/
#define CFG_UART_HW_FLOW_CTRL
#undef CFG_UART_SW_FLOW_CTRL

#ifdef CFG_UART_HW_FLOW_CTRL
    #undef CFG_UART_SW_FLOW_CTRL
#endif
/////////////////////////////////////////////////////////////
/*Misc*/

/* NVDS structure */
#define CFG_NVDS

/* BLE Security  */
#undef CFG_APP_SEC

/* Coarse calibration */
#define CFG_LUT_PATCH

/*Watchdog*/
#undef CFG_WDOG 

/*Sleep modes*/
#define CFG_EXT_SLEEP
#undef CFG_DEEP_SLEEP  

/*Maximum user connections*/
#define BLE_CONNECTION_MAX_USER 1

/*Build for OTP or JTAG*/
#define DEVELOPMENT_DEBUG     1       //0: code at OTP, 1: code via JTAG
/*Application boot from OTP memory - Bootloader copies OTP Header to sysRAM */
#undef APP_BOOT_FROM_OTP
/*NVDS struture is padded with 0 - NVDS struture data must be written in OTP in production procedure*/
#undef READ_NVDS_STRUCT_FROM_OTP

/*Low power clock selection*/
#define CFG_LP_CLK              0x00    //0x00: XTAL32, 0xAA: RCX20, 0xFF: Select from OTP Header

/*Fab Calibration - Must be defined for calibrated devices*/
#define CFG_CALIBRATED_AT_FAB  

/* Use a default trim value for XTAL16M if a trim value has not been programmed in OTP */
#define CFG_USE_DEFAULT_XTAL16M_TRIM_VALUE_IF_NOT_CALIBRATED

/*
 * Scatterfile: Memory maps
 */
//#if defined(CFG_EXT_SLEEP) || !defined(CFG_DEEP_SLEEP)
//#define REINIT_DESCRIPT_BUF     0       //0: keep in RetRAM, 1: re-init is required (set to 0 when Extended Sleep is used)
//#define USE_MEMORY_MAP          EXT_SLEEP_SETUP

//#else
// set the connection on two in the excel tool and use the numbers, then we changed some of those nummbers to increase the attribute database 
/*The following are valid for BLE_CONNECTION_MAX_USER == 1. Please use the results of the excel tool! */
#define REINIT_DESCRIPT_BUF     0       //0: keep in RetRAM, 1: re-init is required (set to 0 when Extended Sleep is used)
#define USE_MEMORY_MAP          EXT_SLEEP_SETUP
#define DB_HEAP_SZ             	1524 // add 500 bytes because we didnt have enough space for the  original:1024
#define ENV_HEAP_SZ             656 // original: 328		
#define MSG_HEAP_SZ             2048//1516 //  remove 500 bytes because we didnt have enough space for the attribute of the streamdataproject Original: 1312	
#define NON_RET_HEAP_SZ         2048 // Original: 1024	
//#endif

#endif // DA14580_CONFIG_H_
