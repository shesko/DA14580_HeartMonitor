/**
****************************************************************************************
*
* @file app_sps_host_project.h
*
* @brief SPS Project application header file.
*
* Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
*
* <bluetooth.support@diasemi.com> and contributors.
*
****************************************************************************************
*/

#ifndef APP_SPS_HOST_PROJECT_H_
#define APP_SPS_HOST_PROJECT_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SPS Application entry point.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  // application task
#include "gapc_task.h"                  // gap functions and messages
#include "gapm_task.h"                  // gap functions and messages
#include "app.h"                       // application definitions
#include "co_error.h"                  // error code definitions
#include "smpc_task.h"                  // error code definitions
#include "app_sps_host_proj_task.h"


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
 
 /****************************************************************************
Define device name. Used in Advertise string
*****************************************************************************/
#define APP_DFLT_DEVICE_NAME "DA14580"

/**
 * Default Advertising data
 * --------------------------------------------------------------------------------------
 * x02 - Length
 * x01 - Flags
 * x06 - LE General Discoverable Mode + BR/EDR Not Supported
 * --------------------------------------------------------------------------------------
 * x11 - Length
 * x07 - Complete list of 128-bit UUIDs available
 * 0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0,
 *      0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07 - SPS Service UUID
 * --------------------------------------------------------------------------------------
 */

#define APP_DFLT_ADV_DATA        "\x11\x07\xb7\x5c\x49\xd2\x04\xa3\x40\x71\xa0\xb5\x35\x85\x3e\xb0\x83\x07"
#define APP_DFLT_ADV_DATA_LEN    (18)

/**
 * Default Scan response data
 * --------------------------------------------------------------------------------------
 * x09                             - Length
 * xFF                             - Vendor specific advertising type
 * x00\x60\x52\x57\x2D\x42\x4C\x45 - "RW-BLE"
 * --------------------------------------------------------------------------------------
 */
#define APP_SCNRSP_DATA          "\x03\x03\x83\x07"
#define APP_SCNRSP_DATA_LENGTH  (0)

//Device configuration parameters
#define APP_CFG_MAX_MTU        160

//Parameter update parameters
#define APP_PARAM_UPDT_INTV_MIN 10	    // N * 1.25ms
#define APP_PARAM_UPDT_INTV_MAX 10	    // N * 1.25ms
#define APP_PARAM_UPDT_LATENCY  0	    // Conn Events skipped
#define APP_PARAM_UPDT_TIMEOUT  1000    // N * 10ms

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief This function handles the disconnection and also restarts the advertising or scanning
 ****************************************************************************************
 */
void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param);

/**
 ****************************************************************************************
 * @brief Initialize SPS application
 ****************************************************************************************
 */
void app_init_func(void);

/**
 ****************************************************************************************
 * @brief Send a message enable message when the connection is made
 ****************************************************************************************
 */
void app_connection_func(struct gapc_connection_req_ind const *param);

/**
 ****************************************************************************************
 * @brief Initialise the database.
 ****************************************************************************************
 */
bool app_db_init_func(void);

/**
 ****************************************************************************************
 * @brief Configure a start advertising message. Called by app_adv_start
 ****************************************************************************************
 */
void app_adv_func(struct gapm_start_advertise_cmd *cmd);

/**
 ****************************************************************************************
 * @brief Update connection parameters
 ****************************************************************************************
 */
void app_param_update_func(void);

/**
 ****************************************************************************************
 * @brief Initialize security environment.
 ****************************************************************************************
 */
void app_sec_init_func(void);

/**
 ****************************************************************************************
 * @brief Handle encryption completed event. 
 ****************************************************************************************
 */
void app_sec_encrypt_complete_func(void);

/**
 ****************************************************************************************
 * @brief 
 ****************************************************************************************
 */
void app_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id); 

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
 * @brief Reset the gapm layer. This function is called when a link loss has occurred
 ****************************************************************************************
 */
void app_reset_app(void);

/**
 ****************************************************************************************
 * @brief Throughput test Function initialise application level buffers.
 ****************************************************************************************
*/
void test_pkt_init (void);

/// @} APP

#endif //APP_SPS_HOST_PROJECT_H_
