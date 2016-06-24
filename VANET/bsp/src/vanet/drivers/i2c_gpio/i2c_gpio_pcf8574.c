/**
 *	@file	i2c_gpio_pcf8574.c
 *
 *	@brief	PCF8574 8-bit I/O Expander
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

#ifdef CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574

void pcf8574_init(bsp_i2c_gpio_private_t* p)
{
	#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_NOISE, "i2c gpio pcf8574 init %02X", p->i2c_addr);
	#else
	print_dbg("pcf8574 init ");
	print_dbg_hex(p->i2c_addr);
	print_dbg("\r\n");
	#endif
	
	// initialize privates
	p->ddr = 0xff;				// 1==input!
	p->data = 0;
	
	bsp_i2c_write_byte_raw(p->i2c_addr, p->ddr & 0xff);
}

void pcf8574_writePort(bsp_i2c_gpio_private_t *p, uint16_t val)
{
	bsp_i2c_trace_hex("pcf8574 writePort", val);
	
	// change it
	p->data = val;

	// write it!
	bsp_i2c_write_byte_raw(p->i2c_addr, p->data & 0xff);
}

void pcf8574_writePin(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t val)
{
	bsp_i2c_trace("pcf8574 writePin");
	
	if (pin > 8)	return;		// only have 8 pins!?
	
	// change it
	if (val == 1)
		p->data |= (1 << pin);
	else
		p->data &= ~(1 << pin);
	
	// write it!
	bsp_i2c_write_byte_raw(p->i2c_addr, p->data);
}

void pcf8574_pinMode(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t direction, uint8_t initial_value)
{
	bsp_i2c_trace("pcf8574 pinMode");
	
	// PCF8574 doesn't have a direction it is either an output low or other...
}

uint16_t pcf8574_readOutputs(bsp_i2c_gpio_private_t *p)
{
	bsp_i2c_trace("pcf8574 readOutputs");
	return p->data;
}

#endif // CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574