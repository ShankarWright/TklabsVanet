/**
 *	@file	clcd_mcp23017.c
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

#ifdef CONFIG_BSP_ENABLE_LCD_I2C_MCP23017

#ifndef CONFIG_BSP_ENABLE_I2C_GPIO_MCP23017
#error "CONFIG_BSP_ENABLE_I2C_GPIO_MCP23017 required"
#endif

/************************************************************************/
/* MCP23017 LCD SPECIFIC                                                */
/************************************************************************/
static void bsp_clcd_mcp23017_write4bits(bsp_clcd_interface_i2c_mcp23017_t *params, uint8_t value, uint8_t mode)
{
	uint16_t out;
	
	// current output
	out = bsp_i2c_gpio_readOutputs(params->gpio);
	
	// get the 4 bit value in value into the output bits
	 for (int i = 0; i < 4; i++) {
		 out &= ~(1 << params->data[i]);
		 out |= ((value >> i) & 0x1) << params->data[i];
	 }
	
	// start with enable low - this *should* be redundant!
	//out &= ~(1 << params->en);
	//mcp23017_writePort(params->i2c_addr, out);
	
	// command or data?
	if (mode == MCP23017_SEND_DATA)
		out |= (1 << params->rs);
	else
		out &= ~(1 << params->rs);
	
	out |= (1 << params->en); 
	bsp_i2c_gpio_writePort(params->gpio, out);
	
	out &= ~(1 << params->en);
	bsp_i2c_gpio_writePort(params->gpio, out);
}

static void bsp_clcd_mcp23017_send(bsp_clcd_interface_i2c_mcp23017_t *params, uint8_t value, uint8_t mode)
{	
	if (mode == MCP23017_SEND_FOUR_BITS)
	{
		bsp_clcd_mcp23017_write4bits(params, value & 0x0f, MCP23017_SEND_COMMAND);
	}
	else
	{
		bsp_clcd_mcp23017_write4bits(params, value >> 4, mode);
		bsp_clcd_mcp23017_write4bits(params, value & 0x0f, mode);
	}
}


/************************************************************************/
/* "PUBLIC" API                                                         */
/************************************************************************/
void bsp_clcd_mcp23017_init(void *p)
{
	bsp_clcd_interface_i2c_mcp23017_t *params = (bsp_clcd_interface_i2c_mcp23017_t *)p;
#ifdef BSP_CLCD_TRACE
  #ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_CLCD, "clcd mcp23017 init %02X", params->i2c_addr);
  #else
	print_dbg("clcd mcp23017 init ");
	print_dbg_hex(params->i2c_addr);
	print_dbg("\r\n");
  #endif	
#endif	
	
	// initialize i2c
	params->gpio = bsp_i2c_gpio(BSP_I2C_MCP23017, params->i2c_addr);
	
	// set output pins 
	bsp_i2c_gpio_pinMode(params->gpio, params->bl, GPIO_DIR_OUTPUT, 1);		// backlight OFF!
	bsp_i2c_gpio_pinMode(params->gpio, params->rs, GPIO_DIR_OUTPUT, 0);
	bsp_i2c_gpio_pinMode(params->gpio, params->en, GPIO_DIR_OUTPUT, 0);
	bsp_i2c_gpio_pinMode(params->gpio, params->rw, GPIO_DIR_OUTPUT, 0);
	for (int i=0; i<4; i++)
		bsp_i2c_gpio_pinMode(params->gpio, params->data[i], GPIO_DIR_OUTPUT, 0);
	
#ifdef BSP_CLCD_TRACE
  #ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_print(BSP_LOGCAT_CLCD, "bsp_clcd_interface_i2c_mcp23017_t");
	bsp_logcat_dump(BSP_LOGCAT_CLCD, (const uint8_t *)params, sizeof(bsp_clcd_interface_i2c_mcp23017_t));
  #else
	print_dbg("bsp_clcd_interface_i2c_mcp23017_t\r\n");
	print_dbg_array((const uint8_t *)params, sizeof(bsp_clcd_interface_i2c_mcp23017_t));
  #endif
#endif
	
	// set lcd to 4 bit mode
	bsp_clcd_mcp23017_send(params, 0x03, MCP23017_SEND_FOUR_BITS);
	bsp_clcd_delay_us(4500);
	bsp_clcd_mcp23017_send(params, 0x03, MCP23017_SEND_FOUR_BITS);
	bsp_clcd_delay_us(4500);
	bsp_clcd_mcp23017_send(params, 0x03, MCP23017_SEND_FOUR_BITS);
	bsp_clcd_delay_us(150);
	bsp_clcd_mcp23017_send(params, 0x02, MCP23017_SEND_FOUR_BITS);
}

void bsp_clcd_mcp23017_backlight(void *p, bool on_off)
{
	bsp_clcd_interface_i2c_mcp23017_t *params = (bsp_clcd_interface_i2c_mcp23017_t *)p;
	bsp_clcd_trace_ul("clcd_mcp23017 backlight", on_off);

	if (on_off)
		bsp_i2c_gpio_writePin(params->gpio, params->bl, 0);		// active low
	else
		bsp_i2c_gpio_writePin(params->gpio, params->bl, 1);
}

void bsp_clcd_mcp23017_command(void *p, uint8_t command)
{
	bsp_clcd_interface_i2c_mcp23017_t *params = (bsp_clcd_interface_i2c_mcp23017_t *)p;
#ifdef BSP_CLCD_TRACE
	bsp_clcd_trace_hex("clcd_mcp23017 command", command);
#endif
	bsp_clcd_mcp23017_send(params, command, MCP23017_SEND_COMMAND);
}

void bsp_clcd_mcp23017_data(void *p, char *data)
{
	bsp_clcd_interface_i2c_mcp23017_t *params = (bsp_clcd_interface_i2c_mcp23017_t *)p;
#ifdef BSP_CLCD_TRACE
  #ifdef CONFIG_BSP_ENABLE_LOGCAT
    bsp_logcat_printf(BSP_LOGCAT_CLCD, "[ %s ]", data);
  #else
	print_dbg("clcd_mcp23017 data [");
	print_dbg(data);
	print_dbg("]\r\n");
  #endif
#endif
	while (*data != '\0')
		bsp_clcd_mcp23017_send(params, *data++, MCP23017_SEND_DATA);
}


void bsp_clcd_mcp23017_datab(void *p, uint8_t value)
{
	bsp_clcd_interface_i2c_mcp23017_t *params = (bsp_clcd_interface_i2c_mcp23017_t *)p;
	bsp_clcd_mcp23017_send(params, value, MCP23017_SEND_DATA);
}

#endif // CONFIG_BSP_ENABLE_LCD_I2C_PCF8574