/**
 ****************************************************************************************
 *
 * @file app_task.c
 *
 * @brief RW APP Task implementation
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

#include "rwip_config.h"               // SW configuration
#include <string.h> 

#if (BLE_APP_PRESENT)

#include "app_task.h"                  // Application Task API
#include "app.h"                       // Application Definition
#include "app_api.h"                   // Application Definition **added by barry
#include "app_sec.h"                   // Application Definition
#include "app_sec_task.h"              // Application Security Task API
#include "gapc_task.h"                 // GAP Controller Task API
#include "gapm_task.h"                 // GAP Manager Task API
#include "gap.h"                       // GAP Definitions
#include "gapc.h"                      // GAPC Definitions
#include "co_error.h"                  // Error Codes Definition
#include "arch.h"                      // Platform Definitions
#include "co_utils.h"									 // BD Address comparison

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)


#define APP_TASK_HANDLERS_INCLUDE
#include "app_task_handlers.h"
#undef APP_TASK_HANDLERS_INCLUDE

#if (BLE_SPS_CLIENT)
#include "app_sps_host_project.h"
#endif

#if (BLE_SPS_SERVER)
#include "app_sps_device_project.h"
#endif



/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles ready indication from the GAP. - Reset the stack
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapm_device_ready_ind_handler(ke_msg_id_t const msgid,
                                         void const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DISABLED)
    {
        // reset the lower layers.
        struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD, TASK_GAPM, TASK_APP,
                gapm_reset_cmd);

        cmd->operation = GAPM_RESET;

        ke_msg_send(cmd);
    }
    else
    {
        // APP_DISABLED state is used to wait the GAP_READY_EVT message
        ASSERT_ERR(0);
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles GAP manager command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapm_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{	
    switch(param->operation)
    {
        // reset completed
        case GAPM_RESET:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
            else
            {

                // set device configuration
                struct gapm_set_dev_config_cmd* cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
                        TASK_GAPM, TASK_APP, gapm_set_dev_config_cmd);

                app_configuration_func(dest_id, cmd);

                ke_msg_send(cmd);
            }
        }
        break;

        // device configuration updated
        case GAPM_SET_DEV_CONFIG:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
            else
            {
              app_set_dev_config_complete_func();
            }
        }
        break;

        // Advertising finished
        case GAPM_ADV_UNDIRECT:
        {
            app_adv_undirect_complete(param->status);
        }
        break;
        
        // Directed advertising finished
        case GAPM_ADV_DIRECT:
        {
            app_adv_direct_complete(param->status);  
        }
        break;
				
        case GAPM_SCAN_PASSIVE:
		case GAPM_SCAN_ACTIVE:
        {
            if (param->status == GAP_ERR_CANCELED)
            {
                app_connect();
            }						
            else 
            {
                app_scanning();
            }
        }				
        break;
        /* end */
				
        case GAPM_CANCEL:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
        }
		case GAPM_CONNECTION_DIRECT:
            if (param->status == GAP_ERR_CANCELED)
            {
				app_connect_failed_func();
            }
				break;
        default:
        {
            ASSERT_ERR(0); // unexpected error
        }
        break;
    }
		
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles connection complete event from the GAP. Will enable profile.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_connection_req_ind_handler(ke_msg_id_t const msgid,
                                           struct gapc_connection_req_ind const *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    // Connection Index
    if (ke_state_get(dest_id) == APP_CONNECTABLE)
    {
        app_env.conidx = KE_IDX_GET(src_id);
        
        if (app_env.conidx != GAP_INVALID_CONIDX)
        {
            //Disable timer of the establishment of the connection
            ke_timer_clear(APP_CONN_TIMER, TASK_APP);
            
            ke_state_set(dest_id, APP_CONNECTED);
            
            app_connection_func(param);
            
            // Retrieve the connection info from the parameters
            app_env.conhdl = param->conhdl;
            
            app_env.peer_addr_type = param->peer_addr_type;
            memcpy(app_env.peer_addr.addr, param->peer_addr.addr, BD_ADDR_LEN);
            
            // send connection confirmation
            app_connect_confirm(GAP_AUTH_REQ_NO_MITM_NO_BOND);            
            
            // Start param update
            app_param_update_start();         
        }
        else
        {
            // No connection has been establish, restart advertising
            app_adv_start();
        }
    }
    else
    {
        // APP_CONNECTABLE state is used to wait the GAP_LE_CREATE_CONN_REQ_CMP_EVT message
        ASSERT_ERR(0);
    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles GAP controller command complete events.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapc_cmp_evt const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    switch(param->operation)
    {
        // reset completed
        case GAPC_UPDATE_PARAMS:
        {
            if ((param->status != CO_ERROR_NO_ERROR))
            {
                // it's application specific what to do when the Param Upd request is rejected
                app_update_params_rejected_func(param->status);
            }
            else
            {
                // Go to Connected State
                ke_state_set(dest_id, APP_CONNECTED);
                
                // if state is APP_CONNECTED then the request was accepted
                app_update_params_complete_func();
            }
        }
        break;
        
        default:
        {
            if(param->status != GAP_ERR_NO_ERROR)
            {
                ASSERT_ERR(0); // unexpected error
            }
        }
        break;
    }
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles disconnection complete event from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_disconnect_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_disconnect_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    app_disconnect_func(dest_id, param);
    
    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles update parameter event from the GAP.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapc_param_update_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gapc_param_update_req_ind const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    struct gapc_param_update_cfm* cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM, KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                    gapc_param_update_cfm);
    
    cmd->accept = false;
    
    ke_msg_send(cmd);
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the APP_MODULE_INIT_CMP_EVT messgage
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_module_init_cmp_evt_handler(ke_msg_id_t const msgid,
                                           const struct app_module_init_cmp_evt *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        if (param->status == CO_ERROR_NO_ERROR)
        {
            // Add next required service in the database
            if (app_db_init())
            {
                // No more service to add in the database, start application
                app_db_init_complete_func();
            }
        }
        else
        {
            // An error has occurred during database creation
            ASSERT_ERR(0);
        }
    }
    else
    {
        // APP_DB_INIT state is used to wait the APP_MODULE_INIT_CMP_EVT message
        ASSERT_ERR(0);
    }
    
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles the received advertisement message
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int gapm_adv_report_ind_handler(ke_msg_id_t const msgid,
                                           const struct gapm_adv_report_ind *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    if(!memcmp(&param->report.data[3], APP_DFLT_ADV_DATA, APP_DFLT_ADV_DATA_LEN))
    {
        //Save found bd_addr to global variable
        memcpy(&connect_bdaddr, param->report.adv_addr.addr, sizeof(struct bd_addr));
        
        app_cancel_scanning();
    }
    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLES DEFINITION
 ****************************************************************************************
 */


/* Specifies the message handlers that are common to all states. */

const struct ke_state_handler app_default_handler = KE_STATE_HANDLER(app_default_state);

/* Defines the place holder for the states of all the task instances. */
ke_state_t app_state[APP_IDX_MAX] __attribute__((section("retention_mem_area0"), zero_init)); //RETENTION MEMORY 

/*Specifies the bd address that device will connect to*/
struct bd_addr connect_bdaddr __attribute__((section("retention_mem_area0"), zero_init)); //RETENTION MEMORY

#endif //(BLE_APP_PRESENT)

/// @} APPTASK
