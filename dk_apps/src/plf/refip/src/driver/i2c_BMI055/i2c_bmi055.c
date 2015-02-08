/**
 ****************************************************************************************
 *
 * @file i2c_bmi055.c
 *
 * @brief bmi055 driver over i2c interface.
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#include "periph_setup.h"


// macros
#define SEND_I2C_COMMAND(X)             SetWord16(I2C_DATA_CMD_REG, (X))
#define WAIT_WHILE_I2C_FIFO_IS_FULL()   while(!(GetWord16(I2C_STATUS_REG) & TFNF))
#define WAIT_UNTIL_I2C_FIFO_IS_EMPTY()  while(!(GetWord16(I2C_STATUS_REG) & TFE))
#define WAIT_UNTIL_NO_MASTER_ACTIVITY() while(GetWord16(I2C_STATUS_REG) & MST_ACTIVITY)
#define WAIT_FOR_RECEIVED_BYTE()        while(!GetWord16(I2C_RXFLR_REG))


/**
 ****************************************************************************************
 * @brief Initialize I2C controller as a master for BMI055 handling.
 *
 * @param[in] dev_address   Slave device address
 * @param[in] speed         Speed
 * @param[in] address_mode  Addressing mode
 ****************************************************************************************
 */
void i2c_bmi055_init (uint16_t dev_address, uint8_t speed, uint8_t address_mode)
{
    SetBits16(CLK_PER_REG, I2C_ENABLE, 1);                                          // enable  clock for I2C 
    SetWord16(I2C_ENABLE_REG, 0x0);                                                 // Disable the I2C controller	
    SetWord16(I2C_CON_REG, I2C_MASTER_MODE | I2C_SLAVE_DISABLE |I2C_RESTART_EN);    // Slave is disabled
    SetBits16(I2C_CON_REG, I2C_SPEED, speed);                                       // Set speed
    SetBits16(I2C_CON_REG, I2C_10BITADDR_MASTER, address_mode);                     // Set addressing mode
    SetWord16(I2C_TAR_REG, dev_address & 0xFF);                                     // Set Slave device address
    SetWord16(I2C_ENABLE_REG, 0x1);                                                 // Enable the I2C controller
    while(GetWord16(I2C_STATUS_REG) & 0x20);                                        // Wait for I2C master FSM to be IDLE
}

/**
 ****************************************************************************************
 * @brief Disable I2C controller and clock
 ****************************************************************************************
 */
void i2c_bmi055_release(void)
{	
    SetWord16(I2C_ENABLE_REG, 0x0);         // Disable the I2C controller	
    SetBits16(CLK_PER_REG, I2C_ENABLE, 0);  // Disable clock for I2C
}

/**
 ****************************************************************************************
 * @brief Read single byte from I2C EEPROM.
 *
 * @param[in] addr  Memory address to read the byte from.
 *
 * @return Read byte.
 ****************************************************************************************
 */
uint8_t i2c_bmi055_read_byte(uint8_t addr)
{
    WAIT_UNTIL_I2C_FIFO_IS_EMPTY();
	SEND_I2C_COMMAND(addr);                         // Write the address of the register
    WAIT_UNTIL_I2C_FIFO_IS_EMPTY();
    SEND_I2C_COMMAND(0x0100);                       // Write the address of the device
    
    WAIT_FOR_RECEIVED_BYTE();                       // Wait for received data
    return (0xFF & GetWord16(I2C_DATA_CMD_REG));    // Get received byte
}

/**
 ****************************************************************************************
 * @brief Read single series of bytes from I2C EEPROM (for driver's internal use)
 *
 * @param[in] p     Memory address to read the series of bytes from (all in the same page)
 * @param[in] addr  Address of the register
 * @param[in] size  Count of bytes to read (must not cross page)
 ****************************************************************************************
 */
static void bmi055_read_data_single(uint8_t **p, uint8_t addr, uint8_t size)
{
    int j;
	WAIT_UNTIL_I2C_FIFO_IS_EMPTY();
	SEND_I2C_COMMAND(addr);         // Write the address of the register
    WAIT_UNTIL_I2C_FIFO_IS_EMPTY();
    
    // Critical section
    GLOBAL_INT_DISABLE();
    //crappy code Interrupts closed for a long time
    //TODO redesign.
    
    SEND_I2C_COMMAND(0x0100);
    SEND_I2C_COMMAND(0x0100);
    
    // Get the received data
    for (j = 0; j < size-2; j++)                                         
    {
        SEND_I2C_COMMAND(0x0100);
        WAIT_FOR_RECEIVED_BYTE();                   // Wait for received data
        **p =(0xFF & GetWord16(I2C_DATA_CMD_REG));  // Get the received byte
        (*p)++;
    }
    
    WAIT_FOR_RECEIVED_BYTE();                   // Wait for received data
    **p =(0xFF & GetWord16(I2C_DATA_CMD_REG));  // Get the received byte
    (*p)++;

    WAIT_FOR_RECEIVED_BYTE();                   // Wait for received data
    **p =(0xFF & GetWord16(I2C_DATA_CMD_REG));  // Get the received byte
    (*p)++;
    
    // End of critical section
    GLOBAL_INT_RESTORE();
}

/**
 ****************************************************************************************
 * @brief Reads data from I2C EEPROM to memory position of given pointer.
 *
 * @param[in] rd_data_ptr   Read data pointer.
 * @param[in] address       Starting memory address.
 * @param[in] size          Size of the data to be read.
 *
 * @return Bytes that were actually read (due to memory size limitation).
 ****************************************************************************************
 */
uint32_t i2c_bmi055_read_data(uint8_t *rd_data_ptr, uint32_t address, uint32_t size)
{
    uint32_t bytes_read = 0; 

    if (size)
        bmi055_read_data_single(&rd_data_ptr, address, size);

    return bytes_read;
}

/**
 ****************************************************************************************
 * @brief Write single byte to I2C EEPROM.
 *
 * @param[in] address   Memory position to write the byte to.
 * @param[in] wr_data   Byte to be written.
 ****************************************************************************************
 */
void i2c_bmi055_write_byte(uint32_t address, uint8_t wr_data)
{
	WAIT_UNTIL_I2C_FIFO_IS_EMPTY();
	SEND_I2C_COMMAND(address);          // Write the address of the register
	SEND_I2C_COMMAND(wr_data & 0xFF);   // Write the data of the register
    WAIT_UNTIL_I2C_FIFO_IS_EMPTY();     // Wait if I2C Tx FIFO is full
    
    WAIT_UNTIL_NO_MASTER_ACTIVITY();    // Wait until no master activity 
}
