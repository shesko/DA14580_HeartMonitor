/**
****************************************************************************************
*
* @file  app_spsc_task.c
*
* @brief SPS application Message Handlers.
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

/**
 ****************************************************************************************
 * @addtogroup APPTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"            // SW configuration
#include <stdint.h>

#if (BLE_APP_PRESENT)
#include "app_task.h"               // Application Task API
#include "app_spsc.h" 
#include "sps_client_task.h"        // SPS task functions
#include "app_sps_scheduler.h"
#include "uart_sps.h"

/**
 ****************************************************************************************
 * @brief Handles client enable confirmation
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
 int app_sps_client_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_client_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    app_send_ble_flowctrl(bletouart_buffer.HWMReached == TRUE ? UART_XOFF : UART_XON);
    
    return (KE_MSG_CONSUMED);
}

#endif //(BLE_APP_PRESENT)

/// @} APPTASK

