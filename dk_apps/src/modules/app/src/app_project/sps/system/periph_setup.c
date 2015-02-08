/**
 ****************************************************************************************
 *
 * @file periph_setup.c
 *
 * @brief Peripherals setup and initialization. 
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
#include "rwip_config.h"             // SW configuration
#include "periph_setup.h"            // peripheral configuration
#include "global_io.h"
#include "gpio.h"
#include "uart_sps.h"                    // UART initialization
#include "app_sps_scheduler.h"

/**
 ****************************************************************************************
 * @brief Each application reserves its own GPIOs here.
 *
 * @return void
 ****************************************************************************************
 */

#if DEVELOPMENT_DEBUG

void GPIO_reservations(void)
{
/*
* Globally reserved GPIOs reservation
*/

/*
* Application specific GPIOs reservation. Used only in Development mode (#if DEVELOPMENT_DEBUG)
    
i.e.  
    RESERVE_GPIO(DESCRIPTIVE_NAME, GPIO_PORT_0, GPIO_PIN_1, PID_GPIO);    //Reserve P_01 as Generic Purpose I/O
*/
    
    RESERVE_GPIO( UART1_TX, UART1_TX_PORT, UART1_TX_PIN, PID_UART1_TX);  
	RESERVE_GPIO( UART1_RX, UART1_RX_PORT, UART1_RX_PIN, PID_UART1_RX);
#if (UART_HW_FLOW_ENABLED) 
    RESERVE_GPIO( UART1_RTS, UART1_RTS_PORT, UART1_RTS_PIN, PID_UART1_RTSN);  
	RESERVE_GPIO( UART1_CTS, UART1_CTS_PORT, UART1_CTS_PIN, PID_UART1_CTSN);
#endif /*UART_HW_FLOW_ENABLED*/

}
#endif //DEVELOPMENT_DEBUG

/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and SPI port pins and GPIO ports are mapped
 ****************************************************************************************
 */
void set_pad_functions(void)        // set gpio port function mode
{


    GPIO_ConfigurePin( UART1_TX_PORT, UART1_TX_PIN, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( UART1_RX_PORT, UART1_RX_PIN, INPUT_PULLUP, PID_UART1_RX, false );
#if (UART_HW_FLOW_ENABLED) 
    GPIO_ConfigurePin( UART1_RTS_PORT, UART1_RTS_PIN, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( UART1_CTS_PORT, UART1_CTS_PIN, INPUT_PULLUP, PID_UART1_CTSN, false );
#endif /*UART_HW_FLOW_ENABLED*/

    
/*
* Configure application ports.
i.e.    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false ); // Set P_01 as Generic purpose Output
*/
}

#if ((UART_HW_FLOW_ENABLED) && !defined(GPIO_DRV_IRQ_HANDLING_DISABLED))
void gpio0_callback(void)
{
    NVIC_DisableIRQ(GPIO0_IRQn);    // Disable this interrupt
    
    if(GPIO_GetPinStatus(UART1_CTS_PORT, UART1_CTS_PIN)==FALSE)
    {
        app_override_ble_xon();
        //Set interrupt to detect rising edge of CTS
        GPIO_EnableIRQ(UART1_CTS_PORT, UART1_CTS_PIN, GPIO0_IRQn, 0, 1, 0);
    }
    else if(GPIO_GetPinStatus(UART1_CTS_PORT, UART1_CTS_PIN)==TRUE)
    {
        app_override_ble_xoff();
        //Set interrupt to detect falling edge of CTS
        GPIO_EnableIRQ(UART1_CTS_PORT, UART1_CTS_PIN, GPIO0_IRQn, 1, 0, 0);
    }
    NVIC_ClearPendingIRQ(GPIO0_IRQn);
	NVIC_EnableIRQ(GPIO0_IRQn);     // Enable this interrupt
}
#endif //((UART_HW_FLOW_ENABLED) && !defined(GPIO_DRV_IRQ_HANDLING_DISABLED))

/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down. The Uart and SPi clocks are set.
 *
 * @return void
 ****************************************************************************************
 */
void periph_init(void) 
{
	// Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP)) ; 
    
	
	//rom patch
	patch_func();
	
	//Init pads
	set_pad_functions();
    
    SetBits16(CLK_PER_REG, UART1_ENABLE, 1);    // enable clock - always @16MHz
	
    // baudr=9-> 115k2
    // mode=3-> no parity, 1 stop bit 8 data length

    uart_sps_init(UART_SPS_BAUDRATE, 3); //exact baud rate defined in uart.h

   // Enable the pads
	SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);

#if (EXT_SLEEP_ENABLED)
    app_scheduler_reinit();
#endif //EXT_SLEEP_ENABLED

#if ((UART_HW_FLOW_ENABLED) && !defined(GPIO_DRV_IRQ_HANDLING_DISABLED))
    GPIO_RegisterCallback(GPIO0_IRQn, gpio0_callback);
    if(GPIO_GetPinStatus(UART1_CTS_PORT,UART1_CTS_PIN)==FALSE)
    {
        GPIO_EnableIRQ(UART1_CTS_PORT, UART1_CTS_PIN, GPIO0_IRQn, 0, 1, 0);
    }
    else if(GPIO_GetPinStatus(UART1_CTS_PORT,UART1_CTS_PIN)==TRUE)
    {
        GPIO_EnableIRQ(UART1_CTS_PORT, UART1_CTS_PIN, GPIO0_IRQn, 1, 0, 0);
    }
#endif //((UART_HW_FLOW_ENABLED) && !defined(GPIO_DRV_IRQ_HANDLING_DISABLED))
}


