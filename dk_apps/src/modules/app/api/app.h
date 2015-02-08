/**
 ****************************************************************************************
 *
 * @file app.h
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef APP_H_
#define APP_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration
#include "rwble_hl_config.h"
#include "ke_msg.h"

#if (BLE_APP_PRESENT)

#include <stdint.h>          // Standard Integer Definition
#include <co_bt.h>           // Common BT Definitions

#include "arch.h"            // Platform Definitions

/*
 * DEFINES
 ****************************************************************************************
 */


/// Application MTU
#define APP_MTU                 (23)

/**
 * Default Device Name part in ADV Data
 * --------------------------------------------------------------------------------------
 * x09 - Length
 * x09 - Device Name Flag
 * Device Name
 * --------------------------------------------------------------------------------------
 */
//#define APP_DFLT_DEVICE_NAME            ("RW-BLE")

/// Advertising data maximal length
#define APP_ADV_DATA_MAX_SIZE           (ADV_DATA_LEN - 3)
/// Scan Response data maximal length
#define APP_SCAN_RESP_DATA_MAX_SIZE     (SCAN_RSP_DATA_LEN)

/// Local address type
#define APP_ADDR_TYPE     0
/// Advertising channel map
#define APP_ADV_CHMAP     0x07
/// Advertising filter policy
#define APP_ADV_POL       0
/// Advertising minimum interval
#define APP_ADV_INT_MIN   0xA0
/// Advertising maximum interval
#define APP_ADV_INT_MAX   0xA0

//Scanning parameters
#define APP_SCAN_INTERVAL   10
#define APP_SCAN_WINDOW     5

//Connect parameters
#define APP_CON_INTV_MIN        10  // number * 1.25ms
#define APP_CON_INTV_MAX        10  // number * 1.25ms
#define APP_CE_LEN_MIN          0x20
#define APP_CE_LEN_MAX          0x20
#define APP_CON_LATENCY         0
#define APP_CON_SUPERV_TO       100   //1sec
#define APP_CON_SCAN_INTERVAL   0x180
#define APP_CON_SCAN_WINDOW     0x160
#define APP_CON_TIMEOUT         700	//7 sec


#if (BLE_HID_DEVICE)
#undef APP_ADV_INT_MIN
#define APP_ADV_INT_MIN   0x20		// *0.625ms	(+ pseudo random advDelay from 0 to 10ms)
#undef APP_ADV_INT_MAX
#define APP_ADV_INT_MAX   0x20		// *0.625ms (+ pseudo random advDelay from 0 to 10ms)
#endif

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Application environment structure
struct app_env_tag
{
    /// Connection handle
    uint16_t conhdl;
    uint8_t  conidx; // Should be used only with KE_BUILD_ID()

    /// Last initialized profile
    uint8_t next_prf_init;

    /// Security enable
    bool sec_en;
    
    // Last paired peer address type 
    uint8_t peer_addr_type;
    
    // Last paired peer address 
    struct bd_addr peer_addr;
    
    #if BLE_HID_DEVICE
	uint8_t app_state;
	uint8_t app_flags;
    #endif	

};

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */

/// Application environment
extern struct app_env_tag app_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Application task initialization.
 ****************************************************************************************
 */
void app_init(void);


/**
 ****************************************************************************************
 * @brief Profiles's Database initialization sequence.
 ****************************************************************************************
 */
bool app_db_init(void);


/**
 ****************************************************************************************
 * @brief Send BLE disconnect command
 ****************************************************************************************
 */
void app_disconnect(void);


/**
 ****************************************************************************************
 * @brief Sends a connection confirmation message
 ****************************************************************************************
 */
void app_connect_confirm(uint8_t auth);


/**
 ****************************************************************************************
 * @brief Start Advertising. Setup Advertsise and Scan Response Message
 ****************************************************************************************
 */
void app_adv_start(void);


/**
 ****************************************************************************************
 * @brief Stop Advertising
 ****************************************************************************************
 */
void app_adv_stop(void);


/**
 ****************************************************************************************
 * @brief Send a connection param update request message
 ****************************************************************************************
 */
void app_param_update_start(void);


/**
 ****************************************************************************************
 * @brief Start a kernel timer
 ****************************************************************************************
 */
void app_timer_set(ke_msg_id_t const timer_id, ke_task_id_t const task_id, uint16_t delay);

/**
 ****************************************************************************************
 * @brief Start scanning for an advertising device.
 ****************************************************************************************
 */
void app_scanning(void); 

/**
 ****************************************************************************************
 * @brief Start the connection to the device with address connect_bdaddr
 ****************************************************************************************
 */
void app_connect(void);

/**
 ****************************************************************************************
 * @brief Stop scanning of the scanner
 ****************************************************************************************
 */
void app_cancel_scanning(void);

/**
 ****************************************************************************************
 * @brief Sets BLE Rx irq threshold
 * @param[in] Rx buffer irq threshold
 * @return void 
 ****************************************************************************************
 */
 
void app_set_rxirq_threshold(uint32_t rx_threshold);

/// @} APP

#endif //(BLE_APP_PRESENT)

#endif // APP_H_
