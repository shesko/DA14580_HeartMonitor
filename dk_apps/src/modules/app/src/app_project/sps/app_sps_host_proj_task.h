/**
****************************************************************************************
*
* @file app_sps_proj_task.h
*
* @brief SPS application
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

#ifndef APP_SPS_PROJ_TASK_H_
#define APP_SPS_PROJ_TASK_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "ke_msg.h"
#include "sps_client_task.h"
#include "l2cc_task.h"

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
                                      ke_task_id_t const src_id);

/// @} APP

#endif // PRJ1_PROJ_TASK
