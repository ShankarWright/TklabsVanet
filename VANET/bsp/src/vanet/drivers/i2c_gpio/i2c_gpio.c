/**
 *	@file	i2c_gpio.c
 *
 *	@brief	I2C GPIO Expanders
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <asf.h>
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_I2C_GPIO

bsp_i2c_gpio_t * bsp_i2c_gpio(bsp_i2c_gpio_type_t type, uint8_t i2c_addr)
{		
	bsp_i2c_gpio_t *gpio = malloc(sizeof(bsp_i2c_gpio_t));
	gpio->p.i2c_type = type;
	gpio->p.i2c_addr = i2c_addr;
	gpio->init = NULL;				// make sure this gets assigned below!
	
	switch (type)
	{
		#ifdef CONFIG_BSP_ENABLE_I2C_GPIO_PCA9501
		case BSP_I2C_PCA9501:
			gpio->init = &pca9501_init;
			gpio->pinMode = &pca9501_pinMode;
			gpio->writePin = &pca9501_writePin;
			gpio->writePort = &pca9501_writePort;
			gpio->readOutputs = &pca9501_readOutputs;
			break;
		#endif
		#ifdef CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574
		case BSP_I2C_PCF8574:
			gpio->init = &pcf8574_init;
			gpio->pinMode = &pcf8574_pinMode;
			gpio->writePin = &pcf8574_writePin;
			gpio->writePort = &pcf8574_writePort;
			gpio->readOutputs = &pcf8574_readOutputs;
		break;
		#endif
		#ifdef CONFIG_BSP_ENABLE_I2C_GPIO_MCP23017
		case BSP_I2C_MCP23017:
			gpio->init = &mcp23017_init;
			gpio->pinMode = &mcp23017_pinMode;
			gpio->writePin = &mcp23017_writePin;
			gpio->writePort = &mcp23017_writePort;
			gpio->readOutputs = &mcp23017_readOutputs;
			break;
		#endif
		default:
			break;
	}
	
	if (gpio->init == NULL)
		return NULL;
		
	gpio->init(&gpio->p);
	
	#ifdef BSP_I2C_TRACE
	print_dbg("bsp_i2c_gpio_t:\r\n");
	print_dbg_array((const uint8_t *)gpio, sizeof(bsp_i2c_gpio_t));
	#endif
	
	return gpio;
}

#endif