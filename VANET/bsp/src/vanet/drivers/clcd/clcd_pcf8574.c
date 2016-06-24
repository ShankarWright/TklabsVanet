/**
 *	@file	clcd_pcf8574.c
 *
 *	@brief	Character (Text) LCD Driver
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include <asf.h>
#include "vanet.h"

#ifdef CONFIG_BSP_ENABLE_LCD_I2C_PCF8574

#ifndef CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574
#error "CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574 required"
#endif

/************************************************************************/
/* PCF8574 SPECIFIC                                                     */
/************************************************************************/

static void bsp_clcd_pcf8574_write4bits(bsp_clcd_interface_i2c_pcf8574_t *params, uint8_t value, uint8_t mode)
{
	uint8_t portval = bsp_i2c_gpio_readOutputs(params->gpio);
	
	// move to D7-D4
	portval &= 0xf;						// clear old data4
	portval |= (value & 0xf) << 4;		// new data4
	
	// command or data?
	if (mode == PCF8574_SEND_DATA)
		portval |= params->rs;
	else
		portval &= ~(params->rs);

	portval |= params->en;
	bsp_i2c_gpio_writePort(params->gpio, portval);
	
	portval &= ~(params->en);
	bsp_i2c_gpio_writePort(params->gpio, portval);
}

static void bsp_clcd_pcf8574_send(bsp_clcd_interface_i2c_pcf8574_t *params, uint8_t value, uint8_t mode)
{	
	if (mode == PCF8574_SEND_FOUR_BITS)
	{
		bsp_clcd_pcf8574_write4bits(params, value & 0x0f, PCF8574_SEND_COMMAND);
	}
	else
	{
		bsp_clcd_pcf8574_write4bits(params, value >> 4, mode);
		bsp_clcd_pcf8574_write4bits(params, value & 0x0f, mode);
	}
}

/************************************************************************/
/* "PUBLIC" API                                                         */
/************************************************************************/
void bsp_clcd_pcf8574_init(void *p)
{
	bsp_clcd_interface_i2c_pcf8574_t *params = (bsp_clcd_interface_i2c_pcf8574_t *)p;
#ifdef BSP_CLCD_TRACE
  #ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_CLCD, "clcd pcf8574 init %02X", params->i2c_addr);
  #else
	bsp_clcd_trace_hex("clcd pcf8574 init", params->i2c_addr);
  #endif
#endif
	
#ifdef BSP_CLCD_TRACE
  #ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_print(BSP_LOGCAT_CLCD, "bsp_clcd_interface_i2c_pcf8574_t");
	bsp_logcat_dump(BSP_LOGCAT_CLCD, (const uint8_t *)params, sizeof(bsp_clcd_interface_i2c_pcf8574_t));
  #else
	print_dbg("bsp_clcd_interface_i2c_pcf8574_t\r\n");
	print_dbg_array((const uint8_t *)params, sizeof(bsp_clcd_interface_i2c_pcf8574_t));
  #endif
#endif

	// initialize i2c
	params->gpio = bsp_i2c_gpio(BSP_I2C_PCF8574, params->i2c_addr);

	// all outputs low
	bsp_i2c_gpio_writePort(params->gpio, 0);
	
	// convert provided gpio's into bitmasks
	params->bl = (1 << params->bl);
	params->en = (1 << params->en);
	params->rs = (1 << params->rs);
	
	// set lcd to 4 bit mode
	bsp_clcd_pcf8574_send(params, 0x03, PCF8574_SEND_FOUR_BITS);
	bsp_clcd_delay_us(4500);
	bsp_clcd_pcf8574_send(params, 0x03, PCF8574_SEND_FOUR_BITS);
	bsp_clcd_delay_us(4500);
	bsp_clcd_pcf8574_send(params, 0x03, PCF8574_SEND_FOUR_BITS);
	bsp_clcd_delay_us(150);
	bsp_clcd_pcf8574_send(params, 0x02, PCF8574_SEND_FOUR_BITS);
}

void bsp_clcd_pcf8574_backlight(void *p, bool on_off)
{
	bsp_clcd_interface_i2c_pcf8574_t *params = (bsp_clcd_interface_i2c_pcf8574_t *)p;
	bsp_clcd_trace_ul("pcf8574 backlight", on_off);

	uint8_t portval = bsp_i2c_gpio_readOutputs(params->gpio);
	
	if (on_off)
		portval |= params->bl;
	else
		portval &= ~(params->bl);
		
	bsp_i2c_gpio_writePort(params->gpio, portval);
}

void bsp_clcd_pcf8574_command(void *p, uint8_t command)
{
	bsp_clcd_interface_i2c_pcf8574_t *params = (bsp_clcd_interface_i2c_pcf8574_t *)p;
#ifdef BSP_CLCD_TRACE
	bsp_clcd_trace_hex("pcf8574 command", command);
#endif
	bsp_clcd_pcf8574_send(params, command, PCF8574_SEND_COMMAND);
}

void bsp_clcd_pcf8574_data(void *p, char *data)
{
	bsp_clcd_interface_i2c_pcf8574_t *params = (bsp_clcd_interface_i2c_pcf8574_t *)p;
#ifdef BSP_CLCD_TRACE
  #ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_CLCD, "pcf8574 data [ %s ]", data);
  #else
	print_dbg("pcf8574 data [");
	print_dbg(data);
	print_dbg("]\r\n");
  #endif
#endif

	while (*data != '\0')
		bsp_clcd_pcf8574_send(params, *data++, PCF8574_SEND_DATA);
}

void bsp_clcd_pcf8574_datab(void *p, uint8_t value)
{
	bsp_clcd_interface_i2c_pcf8574_t *params = (bsp_clcd_interface_i2c_pcf8574_t *)p;
	
	bsp_clcd_pcf8574_send(params, value, PCF8574_SEND_DATA);
}

#endif // CONFIG_BSP_ENABLE_LCD_I2C_PCF8574