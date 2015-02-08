/*
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 */

#include "arch.h"
#include "app.h"
#include "gpio.h"
#include "ke_event.h"

#if (STREAMDATA_QUEUE)
#include "app_stream_queue.h"
#endif
#include "app_sps_scheduler.h"
#include "app.h"

extern uint8_t app_sleep_flow_off;

/*
 ********************************* Hooks ************************************
 */

/**
 ****************************************************************************************
 * @brief Used for sending messages to kernel tasks generated from
 *         asynchronous events that have been processed in app_asynch_proc.
 *
 * @return true to force calling of schedule(), else false
 ****************************************************************************************
 */

static inline bool app_asynch_trm(void)
{
	bool ret = false;
    
/**************************************************************************************
* User code added here. 
**************************************************************************************/
    
	return ret;
}

/**
 ****************************************************************************************
 * @brief Used for processing of asynchronous events at “user” level. The
 *                   corresponding ISRs should be kept as short as possible and the
 *                   remaining processing should be done at this point.
 *
 * @return true to force calling of schedule(), else false
 ****************************************************************************************
 */

static inline bool app_asynch_proc(void)
{
	bool ret = false;
    
/**************************************************************************************
* User code added here. 
**************************************************************************************/
    if (app_sleep_flow_off)
    {
        if(uarttoble_buffer.HWMReached == FALSE)
        {
    #if (UART_SW_FLOW_ENABLED)
            uart_sps_sw_flow_on();
    #endif //UART_HW_FLOW_ENABLED
    #if (UART_HW_FLOW_ENABLED)
            uart_sps_flow_on();
    #endif //UART_HW_FLOW_ENABLED
        }
    #if (UART_HW_FLOW_ENABLED)
        else
        {
            uart_sps_flow_off();
        }
        
        if(bletouart_buffer.HWMReached == TRUE && GPIO_GetPinStatus(UART1_CTS_PORT,UART1_CTS_PIN)==FALSE)
        {
            app_override_ble_xon();
        }
        else if (bletouart_buffer.HWMReached == FALSE && GPIO_GetPinStatus(UART1_CTS_PORT,UART1_CTS_PIN)==TRUE)
        {
            app_override_ble_xoff();
        }
    #endif //UART_HW_FLOW_ENABLED
        
        app_sleep_flow_off = 0;
   }
    
    app_ble_pull();
    
	return ret;
}

/**
 ****************************************************************************************
 * @brief Used for updating the state of the application just before sleep checking starts.
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_asynch_sleep_proc(void)
{

/**************************************************************************************
* User code added here. 
**************************************************************************************/
#if (EXT_SLEEP_ENABLED)
    app_sleep_check();
#endif //EXT_SLEEP_ENABLED
    return; 
}

/**
 ****************************************************************************************
 * @brief Used to disallow extended or deep sleep based on the current application state. BLE and Radio are still powered off.
 *
 * @param[in] sleep_mode     Sleep Mode
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_sleep_prepare_proc(sleep_mode_t *sleep_mode)
{
    
/**************************************************************************************
* User code added here. 
**************************************************************************************/
    
    return;     
}

/**
 ****************************************************************************************
 * @brief Used for application specific tasks just before entering the low power mode.
 *
 * @param[in] sleep_mode     Sleep Mode 
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_sleep_entry_proc(sleep_mode_t *sleep_mode)
{
/**************************************************************************************
* User code added here. 
**************************************************************************************/
        
    return;
}

/**
 ****************************************************************************************
 * @brief Used for application specific tasks immediately after exiting the low power mode.
 *
 * @param[in] sleep_mode     Sleep Mode 
 *
 * @return void
 ****************************************************************************************
 */

static inline void app_sleep_exit_proc(sleep_mode_t sleep_mode)
{
/**************************************************************************************
* User code added here. 
**************************************************************************************/

    return;
}
