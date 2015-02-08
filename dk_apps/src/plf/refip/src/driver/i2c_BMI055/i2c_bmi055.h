/**
 ****************************************************************************************
 *
 * @file i2c_bmi055.h
 *
 * @brief eeprom driver over i2c interface header file.
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

#ifndef _I2C_BMI055_H
    #define _I2C_BMI055_H

    #include <stdint.h>

    #ifndef _I2C_EEPROM_H
enum I2C_SPEED_MODES{
  I2C_STANDARD = 1,
  I2C_FAST,
};

enum I2C_ADDRESS_MODES{
  I2C_7BIT_ADDR,
  I2C_10BIT_ADDR,
};

enum I2C_ADRESS_BYTES_COUNT{
  I2C_1BYTE_ADDR,
  I2C_2BYTES_ADDR,
  I2C_3BYTES_ADDR,
};
    #endif

    //TODO Linux Driver is available  for bmg160
    //Cross-check 
    #define BMI055_BGW_CHIPID       0x00
    #define BMI055_ACCD_X_LSB       0x02
    #define BMI055_ACCD_X_MSB       0x03
    #define BMI055_ACCD_Y_LSB       0x04
    #define BMI055_ACCD_Y_MSB       0x05
    #define BMI055_ACCD_Z_LSB       0x06
    #define BMI055_ACCD_Z_MSB       0x07
    #define BMI055_ACCD_TEMP        0x08
    #define BMI055_INT_STATUS_0     0x09
    #define BMI055_INT_STATUS_1     0x0A
    #define BMI055_INT_STATUS_2     0x0B
    #define BMI055_INT_STATUS_3     0x0C
    #define BMI055_FIFO_STATUS      0x0E
    #define BMI055_PMU_RANGE        0x0F
    #define BMI055_PMU_BW           0x10
    #define BMI055_PMU_LPW          0x11
    #define BMI055_PMU_LOW_POWER    0x12
    #define BMI055_ACCD_HBW         0x13
    #define BMI055_BGW_SOFTRESET    0x14
    #define BMI055_INT_EN_0         0x16
    #define BMI055_INT_EN_1         0x17
    #define BMI055_INT_EN_2         0x18
    #define BMI055_INT_MAP_0        0x19
    #define BMI055_INT_MAP_1        0x1A
    #define BMI055_INT_MAP_2        0x1B
    #define BMI055_INT_SRC          0x1E
    #define BMI055_INT_OUT_CTRL     0x20
    #define BMI055_INT_RST_LATCH    0x21
    #define BMI055_INT_0            0x22
    #define BMI055_INT_1            0x23
    #define BMI055_INT_2            0x24
    #define BMI055_INT_3            0x25
    #define BMI055_INT_4            0x26
    #define BMI055_INT_5            0x27
    #define BMI055_INT_6            0x28
    #define BMI055_INT_7            0x29
    #define BMI055_INT_8            0x2A
    #define BMI055_INT_9            0x2B
    #define BMI055_INT_A            0x2C
    #define BMI055_INT_B            0x2D
    #define BMI055_INT_C            0x2E
    #define BMI055_INT_D            0x2F
    #define BMI055_FIFO_CONFIG_0    0x30
    #define BMI055_PMU_SELF_TEST    0x32
    #define BMI055_TRIM_NVM_CTRL    0x33
    #define BMI055_BGW_SPI3_WDT     0x34
    #define BMI055_OFC_CTRL         0x36
    #define BMI055_OFC_SETTING      0x37
    #define BMI055_OFC_OFFSET_X     0x38
    #define BMI055_OFC_OFFSET_Y     0x39
    #define BMI055_OFC_OFFSET_Z     0x3A
    #define BMI055_TRIM_GP0         0x3B
    #define BMI055_TRIM_GP1         0x3C
    #define BMI055_FIFO_CONFIG_1    0x3E
    #define BMI055_FIFO_DATA        0x3F


/**
 ****************************************************************************************
 * @brief Initialize I2C controller as a master for BMI055 handling.
 *
 * @param[in] dev_address   Slave device address
 * @param[in] speed         Speed
 * @param[in] address_mode  Addressing mode
 ****************************************************************************************
 */
void i2c_bmi055_init (uint16_t dev_address, uint8_t speed, uint8_t address_mode);


/**
 ****************************************************************************************
 * @brief Disable I2C controller and clock
 ****************************************************************************************
 */
void i2c_bmi055_release(void);


 /****************************************************************************************
 * @brief Read single byte from I2C EEPROM.
 *
 * @param[in] addr  Memory address to read the byte from.
 *
 * @return Read byte.
 ****************************************************************************************
 */
uint8_t i2c_bmi055_read_byte(uint8_t addr);

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
uint32_t i2c_bmi055_read_data(uint8_t *rd_data_ptr, uint32_t address, uint32_t size);

/**
 ****************************************************************************************
 * @brief Write single byte to I2C EEPROM.
 *
 * @param[in] address   Memory position to write the byte to.
 * @param[in] wr_data   Byte to be written.
 ****************************************************************************************
 */
void i2c_bmi055_write_byte(uint32_t address, uint8_t wr_data);


#endif // _I2C_BMI055_H
