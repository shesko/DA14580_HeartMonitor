/**
****************************************************************************************
*
* @file app_sps_scheduler.h
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

#ifndef APP_SPS_SCHEDULER_H_
#define APP_SPS_SCHEDULER_H_

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
 #include <stdint.h>
 
 /*
 * DEFINES
 ****************************************************************************************
 */
#define FALSE	(uint8_t)0
#define TRUE	(uint8_t)1
#define ERROR	(uint8_t)2

//application defines 
#define TX_CALLBACK_SIZE        (uint8_t)	16	//16 bytes messages
#define RX_CALLBACK_SIZE        (uint8_t)	8

#define TX_BUFFER_ITEM_COUNT    (int)			1800//1450
#define TX_BUFFER_ITEM_SIZE	    (uint8_t)	    1 //**do not change item_size**
#define TX_BUFFER_HWM		    (int)			850     //58%
#define TX_BUFFER_LWM		    (int)			650     //45%

#define RX_BUFFER_ITEM_COUNT    (int)			500   
#define RX_BUFFER_ITEM_SIZE	    (uint8_t)	    1 //**do not change item_size**
#define RX_BUFFER_HWM	        (int)			350     //70%
#define RX_BUFFER_LWM		    (int)			150     //30%

#define TX_SIZE         (128)
#define TX_WAIT_LEVEL   (74)    //3 packets (20 + 2*27)
#define TX_WAIT_ROUNDS  (10)

#define TX_START_FRAME_DATA_SIZE  (20)
#define TX_CONTINUE_FRAME_DATA_SIZE  (27)

/*
 * STRUCTURES
 ****************************************************************************************
 */
typedef struct {
	int     item_size;
	int     buffer_size;
	long    byte_size;
	int	    readIdx;
	int	    writeIdx;
	uint8_t *data_ptr;
	int     lowWaterMark;
	int	    highWaterMark;
	uint8_t HWMReached;
}RingBuffer;

extern RingBuffer bletouart_buffer;
extern RingBuffer uarttoble_buffer;

///Flow control flags struct
typedef struct {
	uint8_t	txAllowed;
	uint8_t	sendXOFF;
	uint8_t sendXON;
}flags;

extern flags ble_flags;

void app_check_uart_xon(void);
 /*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize buffers
 ****************************************************************************************
 */
void app_buffer_init(void);

/**
 ****************************************************************************************
 * @brief Function used to count the amount of items in buffer
 ****************************************************************************************
 */
int app_item_count(RingBuffer* buffer);

/**
 ****************************************************************************************
 * @brief Function used to calculate available items and return address of first item
 ****************************************************************************************
 */
uint8_t app_get_item_address(RingBuffer* buffer, uint8_t** rddata, uint8_t readAmount);

/**
 ****************************************************************************************
 * @brief Function used release items that were previously pulled with app_get_item_address
 ****************************************************************************************
 */
uint8_t app_release_items(RingBuffer* buffer, uint8_t readAmount);

/**
 ****************************************************************************************
 * @brief initialize callback functions for SPS application
 ****************************************************************************************
 */
void app_uart_init(void);


void app_write_ble_buff(uint8_t* wrdata);


/**
 ****************************************************************************************
 * @brief reset flow control flags
 ****************************************************************************************
 */
void app_flowcontrol_init(void);

/**
 ****************************************************************************************
 * @brief push ble data to uart transmit buffer
 ****************************************************************************************
 */
void app_ble_push(uint8_t* wrdata, uint8_t writeAmount);

/**
 ****************************************************************************************
 * @brief Generate messages and provide to stream queue.
 ****************************************************************************************
*/
void app_ble_pull (void);

/**
 ****************************************************************************************
 * @brief 		will be used to disable buffers so that no more data can be send
 *				until they are enabled again.
 ****************************************************************************************
 */
void app_update_transmit_allowed(flags* flag_buffer, uint8_t state);

/**
 ****************************************************************************************
 * @brief Sets directly the uart flow control to xoff and sends state to connected device
 ****************************************************************************************
*/
void app_override_ble_xoff(void);

/**
 ****************************************************************************************
 * @brief Checks buffer level and sends xon to connected device
 ****************************************************************************************
*/
void app_override_ble_xon(void);

/**
 ****************************************************************************************
 * @brief Restores the flow control when device wakes up
 ****************************************************************************************
*/
void app_scheduler_reinit(void);

#if (EXT_SLEEP_ENABLED)
/**
 ****************************************************************************************
 * Sleep Functions
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Check if device can go to sleep
 ****************************************************************************************
 */
void app_sleep_check(void);


#endif //EXT_SLEEP_ENABLED


/// @} APP

#endif //APP_SPS_SCHEDULER_H_
