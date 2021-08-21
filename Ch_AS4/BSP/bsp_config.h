/*****************************************************************************
 * (c) Copyright 2009 Actel Corporation.  All rights reserved.
 *
 *
 *
 * Author : Actel Application Team
 * Rev    : 1.0.0.0
 * Description: Configuration for the ON-BOARD peripherals for SmartFusion KITS.
 *
 *******************************************************************************/
#ifndef BSP_CONFIG_H_
#define BSP_CONFIG_H_

//#include "../BSP/SmartFusion/i2c_driver/i2c.h"
/* Configuration for OLED */
//#define OLED_I2C_INSTANCE    &g_mss_i2c0

/* Configuration for the SPI Flash */
#define SPI_FLASH_ON_SF_DEV_KIT  1
#define SPI_FLASH_ON_SF_EVAL_KIT 0

#define USE_DMA_FOR_SPI_FLASH 1
#define SPI_FLASH_DMA_CHANNEL 0

#define USE_DMA_FOR_SPI_FLASH 1
#define SPI_FLASH_DMA_CHANNEL 0

#define SPI_FLASH_READ_DMA_CHANNEL	1

//=======================================================================================================================
// EEPROM
//=======================================================================================================================
#define AT24C256								0
#define M24512									1
#define MY_EEPROM_DEVICE				AT24C256	// M24512


#endif



