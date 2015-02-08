/**
****************************************************************************************
*
* @file app_sps_scheduler.c
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdlib.h>
#include "uart_sps.h"
#include "app_sps_scheduler.h"
#include "gpio.h"
#include "ke_mem.h"
#if (BLE_SPS_SERVER)
#include "app_spss.h"
#include "app_sps_device_project.h"
#endif //BLE_SPS_SERVER
#if (BLE_SPS_CLIENT)
#include "app_spsc.h"
#include "app_sps_host_project.h"
#endif //BLE_SPS_CLIENT
#if (EXT_SLEEP_ENABLED)
#include "periph_setup.h"
#include "uart_sps.h"
#include "rwip.h"
#include "ke_env.h"
#include "ke.h"
#include "lld_sleep.h"
#endif //EXT_SLEEP_ENABLED

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
* GLOBAL VARIABLE DEFINITIONS
****************************************************************************************
*/
/// UART callback pointers
uint8_t tx_write_pointer[TX_CALLBACK_SIZE+1] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t rx_read_pointer[RX_CALLBACK_SIZE] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t tx_state_ptr __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t rx_state_ptr __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY 

/// Flag to prevent multiple calls to the tx callback function
uint8_t callbackbusy __attribute__((section("retention_mem_area0"),zero_init));  //@RETENTION MEMORY

/// SPS application buffer pointers
RingBuffer bletouart_buffer __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
RingBuffer uarttoble_buffer __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

/// SPS application flag pointers
flags ble_flags  __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
flags uart_flags __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

extern uint8_t app_sleep_flow_off;

 /*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/// Private buffer functions prototypes
static void app_buffer_create(RingBuffer* buffer, int buffer_size, int item_size, int lowWaterMark, int highWaterMark);
static uint8_t app_buffer_initialized(RingBuffer *buffer);
static void app_write_items(RingBuffer* buffer, uint8_t* wrdata, uint8_t writeAmount);
static uint8_t app_read_items(RingBuffer* buffer, uint8_t* rddata, uint8_t readAmount);

/// Private callback functions prototypes
static void uart_rx_callback(uint8_t res, uint32_t read_size);
static void uart_tx_callback(uint8_t res);

/// Private application functions prototypes
static uint8_t app_uart_pull(uint8_t* rddata, uint8_t readamount, uint8_t* state);
static void app_uart_push(uint8_t* wrdata, uint8_t writeAmount, uint8_t state);
#if (EXT_SLEEP_ENABLED)
static void app_sleep_prepare(sleep_mode_t *sleep_mode);
#endif //EXT_SLEEP_ENABLED

/// Private flow control functions prototypes
static void app_check_buffer_almost_empty(RingBuffer* buffer, flags* buffer_flags);
static void app_check_buffer_almost_full(RingBuffer* buffer, flags* buffer_flags);

#if (UART_SW_FLOW_ENABLED)
static void app_get_flow_control_state(flags* flag_buffer , uint8_t *state);
#endif //UART_SW_FLOW_ENABLED

/**
 ****************************************************************************************
 * @brief 		function used to declare a new buffer
 *
 * @param[in] 	buffer_size		(how many items should the buffer store?)
 * @param[in] 	item_size		(how big should each item be)
 * @param[in] 	highWaterMark	(amount of items in buffer where XOFF should be send)
 * @param[in] 	lowWaterMark	(amount of items in buffer where XON should be send)
 *
 * @return 		RingBuffer pointer
 ****************************************************************************************
 */
static void app_buffer_create(RingBuffer* buffer, int buffer_size, int item_size, int lowWaterMark, int highWaterMark)
{
    //initialise circular buffer
    buffer->item_size   = item_size;
    buffer->buffer_size = buffer_size;
    buffer->byte_size   = (long)(buffer_size+1)*(long)item_size;//+1 for empty
    buffer->readIdx 	  = 0;
    buffer->writeIdx 	  = 0;
	
    //memory allocation of circular buffer
    buffer->data_ptr 	= ke_malloc((long)(buffer->byte_size)*sizeof(uint8_t), KE_MEM_NON_RETENTION); 
    
    //flow control
    buffer->HWMReached	= FALSE;
    
    //check to make sure that watermarks are valid
    if(highWaterMark < buffer_size && 
        lowWaterMark <= highWaterMark && 
        lowWaterMark > 0)
    {
        buffer->highWaterMark = highWaterMark;
        buffer->lowWaterMark  = lowWaterMark;
    }
    else
    {
        while(1); //error: watermarks invalid
    }
    return;
}

/**
 ****************************************************************************************
 * @brief 		function used to check if buffer is initialized
 *
 * @param[in] 	buffer		(buffer to check)
 *
 * @return 		TRUE(1) or FALSE(0)
 ****************************************************************************************
 */
static uint8_t app_buffer_initialized(RingBuffer *buffer)
{
    if(buffer != NULL &&
        buffer->data_ptr != NULL)
    {
        return TRUE;			
    }
    return FALSE;
}

/**
 ****************************************************************************************
 * @brief 		function used to count the amount of items in buffer
 *
 * @param[in] 	buffer		(buffer to check)
 *
 * @return 		amount of items
 ****************************************************************************************
 */
int app_item_count(RingBuffer* buffer)
{
    int wr = buffer->writeIdx;
    int rd = buffer->readIdx;
    
    //count items in buffer
    if(wr >= rd)
    {
        return (wr - rd);
    }
    else
    {
        return ((wr + buffer->byte_size) - rd);
    }
}


void app_write_ble_buff(uint8_t* wrdata)
{
	
	if (!tx_busy_flag)
		ble_flags.txAllowed = TRUE;
		
		app_write_items(&uarttoble_buffer, wrdata, sizeof(uint8_t));
	
	
}


/**
****************************************************************************************
* @brief 		function used to push multiple items to buffer **buffer will not check if its full yet**
*
* @param[in] 	buffer                  (buffer to push data)
* @param[in] 	wrdata                  (pointer with data which should be pushed)
* @param[in] 	writeAmount				(amount of bytes which should be pushed to buffer)
*
* @return 		none
****************************************************************************************
*/
static void app_write_items(RingBuffer* buffer, uint8_t* wrdata, uint8_t writeAmount)
{		
    //before writing items check how much room there is available and adjust write amount if there is almost no room
    int items = app_item_count(buffer);
    signed int roomLeft = (buffer->buffer_size - items);
    
    if(roomLeft > 0 )
    {
        if(roomLeft < writeAmount)
        {
            //buffer does not have enough room to store all data.
            writeAmount = roomLeft;
        }	
        
        //check if writeindex has reached his limit
        signed int ptr_overflow = (((signed int)buffer->writeIdx + writeAmount) - (buffer->byte_size));
        
        //writeindex has not reached his limit, therefore its safe to write all data
        if(ptr_overflow <= 0)
        {
            //write all data
            memcpy((buffer->data_ptr)+buffer->writeIdx, wrdata, writeAmount);

            //update write index
            buffer->writeIdx += writeAmount;																									
        }
        else
        {
            //write until limit is reached and write remaining data at wr_idx == 0
            memcpy((buffer->data_ptr)+buffer->writeIdx, wrdata, (writeAmount-ptr_overflow));		
            memcpy((buffer->data_ptr), wrdata+(writeAmount-ptr_overflow), ptr_overflow);
            
            //update write index
            buffer->writeIdx = ptr_overflow;																									
        }	
    }

}

/**
****************************************************************************************
* @brief 		function used to pull multiple items from buffer
*
* @param[in] 	buffer                  (buffer to pull from)
* @param[in] 	rddata                  (pulled data)
* @param[in] 	readAmount              (maximum items to pull)
*
* @return 		items pulled
****************************************************************************************
*/
static uint8_t app_read_items(RingBuffer* buffer, uint8_t* rddata, uint8_t readAmount)
{
    //check if there are items to read in the buffer and adjust read amount if the buffer is almost empty
    signed int itemsInBuffer = app_item_count(buffer);	
    if(itemsInBuffer > 0)
    {
        if(itemsInBuffer < readAmount){
            readAmount = itemsInBuffer;
        }	
            
        //check if read index has reached the limit
        signed int ptr_overflow = (((signed int)buffer->readIdx + readAmount) - (buffer->byte_size));
        
        //if not, just read everything at once
        if(ptr_overflow <= 0){
            //pull data and update read index
            memcpy(rddata, (buffer->data_ptr+buffer->readIdx), readAmount);
            buffer->readIdx += readAmount;
                    
            //terminate for safety
            rddata[readAmount] = 0;  
        }
        else
        {
            //pull data until maximum reached and pull remaining data from rd_idx == 0
            memcpy(rddata, (buffer->data_ptr+buffer->readIdx), (readAmount-ptr_overflow));
            memcpy(rddata+(readAmount-ptr_overflow), buffer->data_ptr, ptr_overflow);
                    
            //update index
            buffer->readIdx = ptr_overflow;
                    
            //terminate for safety
            rddata[readAmount] = 0;
        }
    }
    //if there are no items in buffer
    else 
    {
        //reset readamount so that application will know that no bytes are pulled
        readAmount = 0;

        //terminate for safety
        rddata[0] = 0; 		
    }
    
    return readAmount; //return the amount of items pulled from buffer
}

/**
****************************************************************************************
* @brief 		function used to calculate available items and return address of first item
*
* @param[in] 	buffer                  (buffer to pull from)
* @param[in] 	rddata                  (pulled data)
* @param[in] 	maxReadAmount           (maximum items to pull)
*
* @return 		amount of available items
****************************************************************************************
*/
uint8_t app_get_item_address(RingBuffer* buffer, uint8_t** rddata, uint8_t readAmount)
{
    //check if there are items to read in the buffer and adjust read amount if the buffer is almost empty
    signed int itemsInBuffer = app_item_count(buffer);	
    if(itemsInBuffer > 0)
    {
        if(itemsInBuffer < readAmount)
        {
            readAmount = itemsInBuffer;
        }
        //check if read index has reached the limit
        signed int ptr_overflow = (((signed int)buffer->readIdx + readAmount) - (buffer->byte_size));
        
        //if not, just read everything at once
        if (buffer->readIdx == buffer->byte_size)
        {
            *rddata=(buffer->data_ptr);
        }
        else
        {
            if(ptr_overflow <= 0)
            {
                *rddata=(buffer->data_ptr+buffer->readIdx);
            }
            else        
            {
                *rddata=(buffer->data_ptr+buffer->readIdx);
                readAmount=readAmount-ptr_overflow;
            }
        }
    }
    //if there are no items in buffer
    else 
    {
        //reset pointer
        rddata=NULL;
        
        //reset readamount so that application will know that no bytes are pulled
        readAmount = 0;
    }
    return readAmount; //return the amount of items pulled from buffer
}

/**
****************************************************************************************
* @brief 		function used release items that were previously pulled with app_get_item_address
*
* @param[in] 	buffer                  (buffer to pull from)
* @param[in] 	maxReadAmount           (maximum items to pull)
*
* @return 		amount of items released
****************************************************************************************
*/
uint8_t app_release_items(RingBuffer* buffer, uint8_t readAmount)
{
    signed int itemsInBuffer = app_item_count(buffer);	
    if(itemsInBuffer > 0)
    {
        if(itemsInBuffer < readAmount){
            readAmount = itemsInBuffer;
        }
        signed int ptr_overflow = (((signed int)buffer->readIdx + readAmount) - (buffer->byte_size));
        if (buffer->readIdx == buffer->byte_size)
        {
            buffer->readIdx = readAmount;
        }
        else
        {
            if(ptr_overflow <= 0){
                buffer->readIdx += readAmount;
            }
        }
    }
    else 
    {        
        readAmount = 0;
    }
    
    return readAmount; //return the amount of items pulled from buffer
}
/**
 ****************************************************************************************
 * @brief 			receive callback function will handle incoming uart data
 *
 * @param[in] 	res (status: UART_STATUS_OK, UART_STATUS_ERROR, UART_STATUS_TIMEOUT)
 *
 * @return 			none
 ****************************************************************************************
 */

static void uart_rx_callback(uint8_t res, uint32_t read_size)
{			
    //function called from uart receive isr
    if(res == UART_STATUS_OK) 
    {	
        app_uart_push(rx_read_pointer, RX_CALLBACK_SIZE, rx_state_ptr);
    }

    //function called from uart timeout isr	
    else if(res == UART_STATUS_TIMEOUT) 
    {
        app_uart_push(rx_read_pointer, read_size, rx_state_ptr);
    }
    else
    {
        while(1); //error: callback called from unknown source
    }
    
    //reinitiate callback
    uart_sps_read(rx_read_pointer, RX_CALLBACK_SIZE, &rx_state_ptr, &uart_rx_callback); 	
}

/**
 ****************************************************************************************
 * @brief 			transmit callback function will handle uart data transmission
 *
 * @param[in] 	res (status: UART_STATUS_OK, UART_STATUS_ERROR, UART_STATUS_TIMEOUT)
 *
 * @return 			none
 ****************************************************************************************
 */
static void uart_tx_callback(uint8_t res)
{	
    //function gets called from uart transmit isr or application when its not running
    if(res == UART_STATUS_OK)
    {
        //reset state pointer
        tx_state_ptr = UART_NONE;
        
        //get data and pointer
        uint8_t size = app_uart_pull(tx_write_pointer, TX_CALLBACK_SIZE, &tx_state_ptr);
        
        //if there is data available, send data over uart
        if(size > 0)
        {
            uart_sps_write(tx_write_pointer, size, &tx_state_ptr, &uart_tx_callback);
            return;
        }	
        
        //if there is no data but only flow control just send flow control to UART
        else if(tx_state_ptr == UART_XOFF || tx_state_ptr == UART_XON)
        {
            uart_sps_write(0, 0, &tx_state_ptr, &uart_tx_callback);
            return;
        }
    }
    else
    {
        while(1); //error: callback called from unknown source
    }
    
    //there is no data in the buffer so the callback is done
    callbackbusy = FALSE; 
}

/**
 ****************************************************************************************
 * @brief 			This function checks if the buffer is almost empty. If it is disabled because
 *							it was full, it will be enabled again by issueing a XON.
 *
 * @param[in] 	buffer							(buffer to check)
 * @param[in] 	buffer_flags				(flags to issue XON)
 *
 * @return 			none
 ****************************************************************************************
 */
static void app_check_buffer_almost_empty(RingBuffer* buffer, flags *buffer_flags)
{
    // if the buffer has emptied enough to continue receiving data issue a XON
    if(app_item_count(buffer) <= buffer->lowWaterMark)
    {
        if(buffer->HWMReached == TRUE)
        {
            //issue XON
            buffer->HWMReached = FALSE;
            buffer_flags->sendXON = TRUE;
        }	
    }	
}

/**
 ****************************************************************************************
 * @brief 		This function checks if the buffer is almost full. If so, a XOFF will be
 *				issued.
 *
 * @param[in] 	buffer						(buffer to check)
 * @param[in] 	buffer_flags				(flags to issue XOFF)
 *
 * @return 		none
 ****************************************************************************************
 */
static void app_check_buffer_almost_full(RingBuffer* buffer, flags* buffer_flags)
{
    if(app_item_count(buffer) > buffer->highWaterMark && buffer->HWMReached == FALSE)	//if high watermark exceeded
    {
        //issue XOFF
        buffer_flags->sendXOFF = TRUE;				
        buffer->HWMReached = TRUE;
    }		
}

#if (UART_SW_FLOW_ENABLED)
/**
 ****************************************************************************************
 * @brief 			This function consumes the sendXON and sendXOFF flag of uart or ble and
 *							change the state according to that. the state will eventually be send
 *							over UART or BLE.
 *
 * @param[in] 	flag_buffer					(which side should be disabled (UART or BLE))
 * @param[in] 	state								(flow control state)
 *
 * @return 			none
 ****************************************************************************************
 */
static void app_get_flow_control_state(flags* flag_buffer , uint8_t *state)
{		
    // check if the flag is not equal
    if((flag_buffer->sendXON != flag_buffer->sendXOFF) && *state == UART_NONE)
    {
        if (flag_buffer->sendXON == TRUE)
        { 
            // set state to transmit a XON
            *state = UART_XON;
        }
        else 
        {
            // set state to transmit a XOFF
            *state = UART_XOFF;
        } 
    } 
    else
    {
        *state = UART_NONE; //reset
    }
    // consume flags
    flag_buffer->sendXON  = FALSE;
    flag_buffer->sendXOFF = FALSE;
}
#endif //UART_SW_FLOW_ENABLED
/**
 ****************************************************************************************
 * @brief 			will be used to disable buffers so that no more data can be send
 *							until they are enabled again.
 *
 * @param[in] 	flag_buffer					(which side should be disabled (uart or ble))
 * @param[in] 	state								(flow control state)
 *
 * @return 			none
 ****************************************************************************************
 */
void app_update_transmit_allowed(flags* flag_buffer, uint8_t state)
{	
    if(state == UART_XOFF) 
    {
        //disable buffer if xoff is received
        flag_buffer->txAllowed = FALSE;
    }	
    else if(state == UART_XON) 
    {
        //if xon is received, enable buffer
        flag_buffer->txAllowed = TRUE;
    }
    else 
    {
        return; //do nothing
    }
}

/**
 ****************************************************************************************
 * @brief 		pull data to transmit over UART
 *
 * @param[in] 	rddata			(pulled data, will be transmitted by callback)
 * @param[in] 	readAmount	    (how much bytes should be pulled?)
 * @param[in] 	state			(flow control state from the SDK to be send over UART)
 *
 * @return 		readcount		(how many items are actually pulled?)
 ****************************************************************************************
 */
static uint8_t app_uart_pull(uint8_t* rddata, uint8_t readamount, uint8_t *state)
{
    uint8_t readcount = 0;
		
#if (UART_SW_FLOW_ENABLED)
    app_get_flow_control_state(&uart_flags, state);
#endif /*UART_SW_FLOW_ENABLED*/
    
    //only pull data if it's allowed
    if(uart_flags.txAllowed == TRUE)
    {
        //pull data
        readcount = app_read_items(&bletouart_buffer, rddata, readamount);
        //check if buffer is almost empty and send XON if neccesary
        app_check_buffer_almost_empty(&bletouart_buffer, &ble_flags);
        
        //if XON should be send, make sure it's send as soon as possible
        if(ble_flags.sendXON == TRUE)
        {
            app_send_ble_flowctrl(UART_XON);
            ble_flags.sendXON = UART_NONE;
		}
	}
    return readcount;
}

/**
 ****************************************************************************************
 * @brief 		push uart data to buffer and update flow control 
 *
 * @param[in] 	wrdata				(data)
 * @param[in] 	writeAmount			(how much bytes should be stored)
 * @param[in] 	state				(has the uart driver received a flow control state?)
 *
 * @return 		none
 ****************************************************************************************
 */
static void app_uart_push(uint8_t* wrdata, uint8_t writeAmount, uint8_t state)
{	
	//save old pullAllowed state and update pullAllowed depending on incoming state
	uint8_t pullAllowedOld = uart_flags.txAllowed;
	app_update_transmit_allowed(&uart_flags, state);
	
	//write items to buffer
	app_write_items(&uarttoble_buffer, wrdata, writeAmount);
	
	//check if buffer is almost full and issue to send a XOFF if so
	app_check_buffer_almost_full(&uarttoble_buffer, &uart_flags);

	//make sure that XOFF is send as fast as possible
	if(uart_flags.sendXOFF == TRUE || pullAllowedOld != uart_flags.txAllowed)
	{
#if (UART_HW_FLOW_ENABLED)
        uart_sps_force_flow_off();
        uart_flags.sendXOFF = FALSE;
#else
		//Send XOFF character
        __disable_irq();
		if(!callbackbusy){
			callbackbusy = TRUE;
			uart_tx_callback(UART_STATUS_OK);
		}
		__enable_irq();
#endif
	}
}

/**
 ****************************************************************************************
 * @brief 		Push Bluetooth data to buffer or update flow control depending on packet
 *				type.
 * @param[in] 	wrdata				(data)
 * @param[in] 	writeAmount			(how much bytes should be stored)
 *
 * @return 		none
 ****************************************************************************************
 */
void app_ble_push(uint8_t* wrdata, uint8_t writeAmount)
{    
    //write items to buffer;
    app_write_items(&bletouart_buffer, wrdata, writeAmount);
    
    //check if buffer is almost full and issue to send a XOFF if so
    app_check_buffer_almost_full(&bletouart_buffer, &ble_flags);
    
    //if XOFF must be send, send asap
    if(ble_flags.sendXOFF == TRUE)
    {
        app_send_ble_flowctrl(UART_XOFF);
        ble_flags.sendXOFF = UART_NONE;
    }
    //start transmitting
    __disable_irq();
    if(!callbackbusy)
    {
        callbackbusy = TRUE;
        uart_tx_callback(UART_STATUS_OK);
    }        
    __enable_irq();
}

/**
 ****************************************************************************************
 * @brief 			Initialise buffers
 *
 * @return 			none
 ****************************************************************************************
 */
void app_buffer_init(void)
{
    if(!app_buffer_initialized(&bletouart_buffer) && !app_buffer_initialized(&uarttoble_buffer))
    {
        //initialize buffers
        app_buffer_create(&bletouart_buffer, TX_BUFFER_ITEM_COUNT, TX_BUFFER_ITEM_SIZE, TX_BUFFER_LWM, TX_BUFFER_HWM);
        app_buffer_create(&uarttoble_buffer, RX_BUFFER_ITEM_COUNT, RX_BUFFER_ITEM_SIZE, RX_BUFFER_LWM, RX_BUFFER_HWM);
    }
}

/**
 ****************************************************************************************
 * @brief 			Enable uart communication and callbacks
 *
 * @return 			none
 ****************************************************************************************
 */
void app_uart_init()
{
    //call read function once to initialize uart driver environment
    uart_sps_read(rx_read_pointer, RX_CALLBACK_SIZE, &rx_state_ptr, &uart_rx_callback);	
}

/**
 ****************************************************************************************
 * @brief 			Initialize streaming application flow control flags
 *
 * @return 			none
 ****************************************************************************************
 */
void app_flowcontrol_init()
{
    //set flags to default
    ble_flags.sendXOFF		= FALSE;
    ble_flags.sendXON		= FALSE;
    ble_flags.txAllowed		= TRUE;
    
    uart_flags.sendXOFF		= FALSE;
    uart_flags.sendXON		= FALSE;
    uart_flags.txAllowed	= TRUE;
}

/**
 ****************************************************************************************
 * @brief 			Checks if data available in buffer and initiates a transmittion.
 *
 * @return 			none
 ****************************************************************************************
 */
void app_ble_pull (void)
{
    static uint8_t rounds_waiting=0;
    int read_amount;
    
    if(tx_busy_flag == 0 && ble_flags.txAllowed == TRUE)
    {
        if((read_amount = app_item_count(&uarttoble_buffer)) > 0)
        {
            if (read_amount >= TX_WAIT_LEVEL || (rounds_waiting++) >= TX_WAIT_ROUNDS || 1)
            {
                rounds_waiting = 0;
                app_init_ble_tx();
            }
        }
    }
}

/**
 ****************************************************************************************
 * @brief Sets directly the uart flow control to xoff and sends state to connected device
 *
 * @return void.
 ****************************************************************************************
*/
void app_override_ble_xoff(void)
{
    if (ke_state_get(KE_BUILD_ID(TASK_APP, app_env.conidx)) == APP_CONNECTED)
    {
        app_send_ble_flowctrl(UART_XOFF);
        app_update_transmit_allowed(&uart_flags, UART_XOFF);
        bletouart_buffer.HWMReached = TRUE;
    }
}

/**
 ****************************************************************************************
 * @brief Checks buffer level and sends xon to connected device
 *
 * @return void.
 ****************************************************************************************
*/
void app_override_ble_xon(void)
{
    app_update_transmit_allowed(&uart_flags, UART_XON);
    app_check_buffer_almost_empty(&bletouart_buffer, &ble_flags);
    if(ble_flags.sendXON == TRUE)
    {
        app_send_ble_flowctrl(UART_XON);
        ble_flags.sendXON = UART_NONE;
    }
}

/**
 ****************************************************************************************
 * @brief Checks buffer level and sends flow on signal to UART
 *
 * @return void.
 ****************************************************************************************
*/
void app_check_uart_xon(void)
{
    //check if buffer is almost empty and send XON if neccesary
    app_check_buffer_almost_empty(&uarttoble_buffer, &uart_flags);
    
    //if XON should be send, make sure it's send as soon as possible
    if(uart_flags.sendXON == TRUE)
    {
#if ((UART_HW_FLOW_ENABLED) && !defined(GPIO_DRV_IRQ_HANDLING_DISABLED))
        uart_sps_flow_on();
        uart_flags.sendXON = FALSE;
#else
        //send XON character
        __disable_irq();
        if(!callbackbusy)
        {
            callbackbusy = TRUE;
            uart_tx_callback(UART_STATUS_OK);
        }
        __enable_irq();
#endif
    }
}

#if (EXT_SLEEP_ENABLED)
/**
 ****************************************************************************************
 * @brief Restores the flow control when device wakes up
 *
 * @return void.
 ****************************************************************************************
*/

/**
 ****************************************************************************************
 * Sleep Functions
 ****************************************************************************************
 */

void app_scheduler_reinit(void)
{
    app_uart_init();
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

/**
 ****************************************************************************************
 * @brief  Performs a pre-check of the kernel an ble events in order to flow off serial interface. 
 *
 * @return sleep_mode_t Sleep mode
 ****************************************************************************************
 */

sleep_mode_t app_rwip_sleep_check(void)
{
    sleep_mode_t proc_sleep = mode_active;
   
    uint32_t sleep_duration = jump_table_struct[max_sleep_duration_external_wakeup_pos];//MAX_SLEEP_DURATION_EXTERNAL_WAKEUP;
#ifndef DEVELOPMENT_DEBUG
    uint32_t sleep_lp_cycles;
#endif

#if (BLE_APP_PRESENT)
    if ( app_ble_ext_wakeup_get() || (rwip_env.ext_wakeup_enable == 2) )  // sleep forever!
        sleep_duration = 0;
#else
# if (!EXTERNAL_WAKEUP) // sleep_duration will remain as it was set above....
    if (rwip_env.ext_wakeup_enable == 2)
        sleep_duration = 0;
#endif		
#endif    
    
    do
    {
        
        // Do not allow sleep if system is in startup period
        if (check_sys_startup_period())
            break;
        
        /************************************************************************
         **************            CHECK KERNEL EVENTS             **************
         ************************************************************************/
        // Check if some kernel processing is ongoing
        if (!ke_sleep_check())
            break;
        
        // Processor sleep can be enabled
        proc_sleep = mode_idle;
        
        /************************************************************************
         **************           CHECK RADIO POWER DOWN           **************
         ************************************************************************/
        // Check if BLE + Radio are still sleeping
        if(GetBits16(SYS_STAT_REG, RAD_IS_DOWN)) {
            // If BLE + Radio are in sleep return the appropriate mode for ARM
            proc_sleep = mode_sleeping;
            break;
        }

        /************************************************************************
         **************              CHECK RW FLAGS                **************
         ************************************************************************/
        // First check if no pending procedure prevents us from going to sleep
        if (rwip_prevent_sleep_get() != 0)
            break;

        /************************************************************************
         **************           CHECK EXT WAKEUP FLAG            **************
         ************************************************************************/
        /* If external wakeup is enabled, sleep duration can be set to maximum, otherwise
         * the system must be woken-up periodically to poll incoming packets from HCI */
        if((BLE_APP_PRESENT == 0) || (BLE_INTEGRATED_HOST_GTL == 1 )) // No need for periodic wakeup if we have full-hosted system
        {
            if(!rwip_env.ext_wakeup_enable)
                sleep_duration = jump_table_struct[max_sleep_duration_periodic_wakeup_pos]; // MAX_SLEEP_DURATION_PERIODIC_WAKEUP;
        }

        
        
        /************************************************************************
         *                                                                      *
         *                   CHECK DURATION UNTIL NEXT EVENT                    *
         *                                                                      *
         ************************************************************************/
        // If there's any timer pending, compute the time to wake-up to serve it
        if (ke_env.queue_timer.first != NULL)
            sleep_duration = jump_table_struct[max_sleep_duration_external_wakeup_pos];
#ifdef USE_POWER_OPTIMIZATIONS
        // Store sleep_duration calculated so far. Check below if sleep would be allowed.
        // If not, there's no reason to verify / ensure the available time for SLP...
        uint32_t tmp_dur = sleep_duration;
#endif

        /************************************************************************
         **************            CHECK KERNEL TIMERS             **************
         ************************************************************************/
        // Compute the duration up to the next software timer expires
        if (!ke_timer_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        
        #if (BLE_EMB_PRESENT)
        /************************************************************************
         **************                 CHECK BLE                  **************
         ************************************************************************/
        // Compute the duration up to the next BLE event
        if (!lld_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;
        #endif // BLE_EMB_PRESENT
        
        proc_sleep = mode_sleeping;
     } while(0);
     
     return proc_sleep;
}

/**
 ****************************************************************************************
 * @brief Prepare device for sleep
 *
 * @return void
 ****************************************************************************************
 */

static void app_sleep_prepare(sleep_mode_t *sleep_mode)
{
    if(*sleep_mode == mode_sleeping)
    {
#if (UART_SW_FLOW_ENABLED)
        if (!uart_sps_sw_flow_off())
#endif //UART_SW_FLOW_ENABLED

#if (UART_HW_FLOW_ENABLED)
        if (!uart_sps_flow_off())
#endif //UART_HW_FLOW_ENABLED
        {
            *sleep_mode = mode_idle;
        }
        else
            app_sleep_flow_off = 1;

    }
}

/**
 ****************************************************************************************
 * @brief Check if device can go to sleep
 *
 * @return void
 ****************************************************************************************
 */

void app_sleep_check(void)
{
    
    sleep_mode_t sleep_mode = app_rwip_sleep_check();
        
    if (sleep_mode == mode_sleeping)
    {    
        if ((app_item_count(&bletouart_buffer) != 0)
            || (app_item_count(&uarttoble_buffer) != 0)
            || (tx_buffer_size() != 0))
        {
            if (ke_state_get(KE_BUILD_ID(TASK_APP, app_env.conidx)) == APP_CONNECTED)
            {
                app_disable_sleep();
            }
        }
        else
        {
            // check UART. If a Tx or Rx Proceess is in progress disable sleep
            if (uart_sps_fifo_check())
                app_disable_sleep();
            else
            {
                
                app_sleep_prepare(&sleep_mode);
                
                if (sleep_mode == mode_sleeping)
                    app_set_extended_sleep();
                
            }
        }
    }
}

#endif //EXT_SLEEP_ENABLED

/// @} APP
