/**
****************************************************************************************
*
* @file app_sps_proj.c
*
* @brief SPS project source code.
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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip.h"
#include "arch_sleep.h"
#include "app_task.h"                // Application task Definition
#include "app_sec.h"

#include "gapm_task.h"
#include "gapc.h"
#include "gattc_task.h"

#include "app_sps_scheduler.h"
#include "app_sps_device_project.h"
#include "app_spss.h"

#include "uart_sps.h" 
#include "app_api.h"

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)

#include <co_math.h>

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
void app_task_custom_init(void);
 /**
 ****************************************************************************************
 * @brief 			Initialize SPS application
 *
 * @return 			none
 ****************************************************************************************
 */
void app_init_func(void)
{	
    
    app_set_rxirq_threshold(BLE_RX_BUFFER_CNT/4);
    
    app_sps_init();
}

/**
 ****************************************************************************************
 * @brief Sends a exchange MTU command
 *
 * @return void
 ****************************************************************************************
 */

void gattc_exc_mtu_cmd(void)
{
  struct gattc_exc_mtu_cmd *cmd =  KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
            KE_BUILD_ID(TASK_GATTC,app_env.conidx ), TASK_APP,
            gattc_exc_mtu_cmd);
    
    cmd->req_type = GATTC_MTU_EXCH;
    
    ke_msg_send(cmd);
}

/**
 ****************************************************************************************
 * @brief Send a message enable message when the connection is made
 * 
 * @param[in] param      Connection parameters
 *
 * @return void
 ****************************************************************************************
 */
void app_connection_func(struct gapc_connection_req_ind const *param)
{	
    /*--------------------------------------------------------------
    * ENABLE REQUIRED PROFILES
    *-------------------------------------------------------------*/
    //enable SPS device role or SPS host and set the proper values
    app_sps_enable();	
    
    gattc_exc_mtu_cmd();
    
}

/**
 ****************************************************************************************
 * @brief Configure a start advertising message. Called by app_adv_start
 * 
 * @param[in] cmd     Pointer to message.
 *
 * @return void
 ****************************************************************************************
 */
void app_adv_func(struct gapm_start_advertise_cmd *cmd)
{
    //  Device Name Length
    uint8_t device_name_length;
    uint8_t device_name_avail_space;
    uint8_t device_name_temp_buf[64];
    
    cmd->op.code     = GAPM_ADV_UNDIRECT;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->intv_min    = APP_ADV_INT_MIN;
    cmd->intv_max    = APP_ADV_INT_MAX;
    cmd->channel_map = APP_ADV_CHMAP;
    
    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
    
    /*-----------------------------------------------------------------------------------
     * Set the Advertising Data and the Scan Response Data
     *---------------------------------------------------------------------------------*/
    cmd->info.host.adv_data_len       = APP_ADV_DATA_MAX_SIZE;
    cmd->info.host.scan_rsp_data_len  = APP_SCAN_RESP_DATA_MAX_SIZE;
    
    // Advertising Data
#if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,
                &cmd->info.host.adv_data[0]) != NVDS_OK)
#endif //(NVDS_SUPPORT)
    {
        cmd->info.host.adv_data_len = APP_DFLT_ADV_DATA_LEN;
        memcpy(&cmd->info.host.adv_data[0], APP_DFLT_ADV_DATA, cmd->info.host.adv_data_len);
    }
    
    // Scan Response Data
#if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &cmd->info.host.scan_rsp_data_len,
                &cmd->info.host.scan_rsp_data[0]) != NVDS_OK)
#endif //(NVDS_SUPPORT)
    {
        cmd->info.host.scan_rsp_data_len = APP_SCNRSP_DATA_LENGTH;
        memcpy(&cmd->info.host.scan_rsp_data[0], APP_SCNRSP_DATA, cmd->info.host.scan_rsp_data_len);
    }
    
    // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
    device_name_avail_space = APP_ADV_DATA_MAX_SIZE - cmd->info.host.adv_data_len - 2;
    
    // Check if data can be added to the Advertising data
    if (device_name_avail_space > 0)
    {
        // Get the Device Name to add in the Advertising Data (Default one or NVDS one)
#if (NVDS_SUPPORT)
        device_name_length = NVDS_LEN_DEVICE_NAME;
        if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, &device_name_temp_buf[0]) != NVDS_OK)
#endif //(NVDS_SUPPORT)
        {
            // Get default Device Name (No name if not enough space)
            device_name_length = strlen(APP_DFLT_DEVICE_NAME);
            memcpy(&device_name_temp_buf[0], APP_DFLT_DEVICE_NAME, device_name_length);
        }
        
        if(device_name_length > 0)
        {
            // Check available space
            device_name_length = co_min(device_name_length, device_name_avail_space);
            
            // Fill Length
            cmd->info.host.adv_data[cmd->info.host.adv_data_len]     = device_name_length + 1;
            // Fill Device Name Flag
            cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\x09';
            // Copy device name
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2], device_name_temp_buf, device_name_length);
            // Update Advertising Data Length
            cmd->info.host.adv_data_len += (device_name_length + 2);
        }
    }
 
}

/**
 ****************************************************************************************
 * @brief This function handles the disconnection and also restarts the advertising or scanning
 *
 * @param[in] task_id 				Destination task id.
 * @param[in] param 					Pointer to the parameters of the message. 
 *
 * @return void
 ****************************************************************************************
 */
void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param)
{
    uint8_t state = ke_state_get(task_id);
    if ((state == APP_SECURITY) || (state == APP_CONNECTED)  || (state == APP_PARAM_UPD))
    {	
        // Restart Advertising
        app_adv_start();
    }
    else
    {
        // We are not in a Connected State
        ASSERT_ERR(0);
    }
}

/**
 ****************************************************************************************
 * @brief Initialise the database.
 *
 * @return If database initialization completed.
 ****************************************************************************************
 */
bool app_db_init_func(void)
{  
    // Indicate if more services need to be added in the database
    bool end_db_create = false;

    // Check if another should be added in the database
    if (app_env.next_prf_init < APP_PRF_LIST_STOP)
    {
        switch (app_env.next_prf_init)
        {
            case (APP_SPS_TASK):
            {
                // Add SPS services to the database
                app_sps_create_db();
            } break;
            default:
            {
              ASSERT_ERR(0);
            } break;
        }

        // Select following service to add
        app_env.next_prf_init++;
    }
    else
    {
        end_db_create = true;
    }
    return end_db_create;
}

/**
 ****************************************************************************************
 * @brief Set the advertiser role.
 *
 * @param[in] task_id       ke_task_id_t kernel ID
 * @param[in] cmd     	    struct gapm_set_dev_config_cmd 
 *
 * @return void
 ****************************************************************************************
 */
void app_configuration_func(ke_task_id_t const task_id, struct gapm_set_dev_config_cmd *cmd)
{
    // Operation select
    cmd->operation = GAPM_SET_DEV_CONFIG;
   
    // Device Role
    cmd->role = GAP_PERIPHERAL_SLV;
    // Device IRK
    // cmd->irk = ; TODO NOT set
    // Device Appearance
    cmd->appearance = APP_CFG__APPEARANCE;
    // Device Appearance write permission requirements for peer device
    cmd->appearance_write_perm = GAPM_WRITE_DISABLE;
    // Device Name write permission requirements for peer device
    cmd->name_write_perm = GAPM_WRITE_DISABLE;

    // Peripheral only: *****************************************************************
    // Slave preferred Minimum of connection interval
    cmd->con_intv_min = APP_CFG_CON_INTV_MIN;
    // Slave preferred Maximum of connection interval
    cmd->con_intv_max = APP_CFG_CON_INTV_MAX;
    // Slave preferred Connection latency
    cmd->con_latency  = APP_CFG_CON_LATENCY;
    // Slave preferred Link supervision timeout
    cmd->superv_to    = APP_CFG_SUPERV_TO;

    // Privacy settings bit field
    cmd->flags = APP_CFG_FLAGS;
 
    //Defined maximum transmission unit
    cmd->max_mtu = APP_CFG_MAX_MTU;
}

/**
 ****************************************************************************************
 * @brief Called upon device's configuration completion. Starts advertsing or scanning.
 *
 * @return void.
 ****************************************************************************************
*/
void app_set_dev_config_complete_func(void)
{
    // We are now in Initialization State
    ke_state_set(TASK_APP, APP_DB_INIT);

    // Add the first required service in the database
    if (app_db_init())
    {
        // When the data base is already made start advertising	
        app_adv_start();
    }
}

/**
 ****************************************************************************************
 * @brief Called upon connection param's update rejection
 *
 * @param[in] status        Error code
 *
 * @return void.
 ****************************************************************************************
*/
void app_update_params_rejected_func(uint8_t status)
{
	ASSERT_INFO(0, status, APP_PARAM_UPD);
}

/**
 ****************************************************************************************
 * @brief Called upon connection param's update completion
 *
 * @return void.
 ****************************************************************************************
*/
void app_update_params_complete_func(void)
{
    return;
}

/**
 ****************************************************************************************
 * @brief Handles undirect advertising completion.
 *
 * @return void.
 ****************************************************************************************
*/
void app_adv_undirect_complete(uint8_t status)
{
	return;
}

/**
 ****************************************************************************************
 * @brief Handles direct advertising completion.
 *
 * @param[in] status        Error code
 *
 * @return void.
 ****************************************************************************************
*/
void app_adv_direct_complete(uint8_t status)
{
    if (ke_state_get(status) == APP_CONNECTABLE)
    {
        app_adv_start(); //restart advertising
    }
}

/**
 ****************************************************************************************
 * @brief Handles Database creation. Start application.
 *
 * @return void.
 ****************************************************************************************
*/
void app_db_init_complete_func(void)
{
    app_adv_start();
}

/**
 ****************************************************************************************
 * @brief Handles connection request failure.
 *
 * @return void.
 ****************************************************************************************
*/

void app_connect_failed_func(void)
{
    
}

/**
 ****************************************************************************************
 * @brief Reset the gapm layer. This function is called when a link loss has occurred
 * 
 * @return void
 ****************************************************************************************
 */
void app_reset_app(void)
{
    struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD, TASK_GAPM, TASK_APP,gapm_reset_cmd);		
    cmd->operation = GAPM_RESET;// Set GAPM_RESET
    ke_msg_send(cmd);// Send the message
}

/**
 ****************************************************************************************
 * @brief Update connection parameters
 * 
 * @return void
 ****************************************************************************************
 */
void app_param_update_func(void)
{
    struct gapc_param_update_cmd * req = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD, TASK_GAPC, TASK_APP, gapc_param_update_cmd);

    // Fill in the parameter structure
    req->operation = GAPC_UPDATE_PARAMS;
    
#ifndef __DA14581__    
    req->params.intv_min = APP_PARAM_UPDT_INTV_MIN;
    req->params.intv_max = APP_PARAM_UPDT_INTV_MAX;
    req->params.latency  = APP_PARAM_UPDT_LATENCY;
    req->params.time_out = APP_PARAM_UPDT_TIMEOUT;
#else
    req->intv_min = APP_PARAM_UPDT_INTV_MIN;
    req->intv_max = APP_PARAM_UPDT_INTV_MAX;
    req->latency  = APP_PARAM_UPDT_LATENCY;
    req->time_out = APP_PARAM_UPDT_TIMEOUT;
#endif
    
    ke_msg_send(req);
    
    return;
}

/* encryption functions **not used** */

/**
 ****************************************************************************************
 * @brief Initialize security environment.
 * 
 * @return void
 ****************************************************************************************
 */
void app_sec_init_func(void)
{   
		return;
}

/**
 ****************************************************************************************
 * @brief Handle encryption completed event. 
 * 
 * @return void
 ****************************************************************************************
 */
void app_sec_encrypt_complete_func(void)
{
		return;
}

/**
 ****************************************************************************************
 * @brief  
 * 
 * @return void
 ****************************************************************************************
 */
void app_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
		return;
}

#if (BLE_APP_SEC) // the functions of BLE_APP_SEC are not used in the SPS project

/**
 ****************************************************************************************
 * @brief Handle pairring request message. Send a pairing response 
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_pairing_rsp_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    cfm->request = GAPC_PAIRING_RSP;
    cfm->accept = true;

    // OOB information
    cfm->data.pairing_feat.oob            = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    // Encryption key size
    cfm->data.pairing_feat.key_size       = KEY_LEN;
    // IO capabilities
    cfm->data.pairing_feat.iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    // Authentication requirements
    cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_NO_MITM_BOND;
    //Security requirements
    cfm->data.pairing_feat.sec_req        = GAP_NO_SEC;
    //Initiator key distribution
    //GZ cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_NONE;
    cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_SIGNKEY;
    //Responder key distribution
    cfm->data.pairing_feat.rkey_dist      = GAP_KDIST_ENCKEY;
    
    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief Send Temporary key 
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_tk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);
    uint32_t pin_code = app_sec_gen_tk();
    cfm->request = GAPC_TK_EXCH;
    cfm->accept = true;
    
    memset(cfm->data.tk.key, 0, KEY_LEN);
    
    cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);
    cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
    cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
    cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
    
    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief Send IRK
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_irk_exch_func(struct gapc_bond_req_ind *param)
{
    return;
}

/**
 ****************************************************************************************
 * @brief Send CSRK
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_csrk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_CSRK_EXCH;

    cfm->accept = true;

    memset((void *) cfm->data.csrk.key, 0, KEY_LEN);
    memcpy((void *) cfm->data.csrk.key, (void *)"\xAB\xAB\x45\x55\x23\x01", 6);

    ke_msg_send(cfm);

}

/**
 ****************************************************************************************
 * @brief Send Long term key
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_ltk_exch_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_LTK_EXCH;

    cfm->accept = true;

    cfm->data.ltk.key_size = app_sec_env.key_size;
    cfm->data.ltk.ediv = app_sec_env.ediv;

    memcpy(&(cfm->data.ltk.randnb), &(app_sec_env.rand_nb) , RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), &(app_sec_env.ltk) , KEY_LEN);

    ke_msg_send(cfm);

}

/**
 ****************************************************************************************
 * @brief Handle Pairing/Bonding copletion event
 * 
 * @return void
 ****************************************************************************************
 */
void app_paired_func(void)
{
    return;
}

/**
 ****************************************************************************************
 * @brief Handle Pairing/Bonding copletion event
 * 
 * @return bool
 ****************************************************************************************
 */
bool app_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param)
{
    return true;
}

void app_sec_encrypt_ind_func(void)
{
    
    return; 
}

#endif //BLE_APP_SEC



#endif  //BLE_APP_PRESENT
/// @} APP
