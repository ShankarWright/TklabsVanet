/**
 *	@file	i2c_gpio_mcp23017.c
 *
 *	@brief	MCP23017 16-bit I/O Expander
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

#ifdef CONFIG_BSP_ENABLE_I2C_GPIO_MCP23017

#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14
#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

void mcp23017_init(bsp_i2c_gpio_private_t* p)
{
#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_NOISE, "i2c gpio mcp23017 init %02X", p->i2c_addr);
#else
	print_dbg("mcp23017 init ");
	print_dbg_hex(p->i2c_addr);
	print_dbg("\r\n");
#endif
	
	// initialize privates
	p->ddr = 0xffff;			// 1==input!
	p->data = 0;
	
	bsp_i2c_write_bytes(p->i2c_addr, MCP23017_IODIRA, (uint8_t *)&p->ddr, 2);	// IODIRB is next addr
}

void mcp23017_writePort(bsp_i2c_gpio_private_t *p, uint16_t val)
{
	bsp_i2c_trace("mcp23017 writePort");
	
	// change it
	p->data = val;

	// write it!
	uint8_t buf[2];
	buf[0] = p->data & 0xff;			// low byte is A
	buf[1] = p->data >> 8;				// high byte is B
	bsp_i2c_write_bytes(p->i2c_addr, MCP23017_GPIOA, buf, 2);
}

void mcp23017_writePin(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t val)
{
	bsp_i2c_trace("mcp23017 writePin");
	
	uint8_t ioval;
	uint8_t ioaddr;
	
	if (pin > 15)	return;		// only have 16 pins!?
	
	// change it
	if (val == 1)
		p->data |= (1 << pin);
	else
		p->data &= ~(1 << pin);
	
	if (pin < 8)
	{
		ioaddr = MCP23017_GPIOA;
		ioval = p->data & 0xff;
	}
	else
	{
		ioaddr = MCP23017_GPIOB;
		ioval = (p->data & 0xff00) >> 8;
	}
	
	// write it!
	bsp_i2c_write_bytes(p->i2c_addr, ioaddr, &ioval, 1);
}

void mcp23017_pinMode(bsp_i2c_gpio_private_t* p, uint8_t pin, uint8_t direction, uint8_t initial_value)
{
	bsp_i2c_trace("mcp23017 pinMode");
	
	if (pin > 15)	return;		// only have 16 pins!?
	
	// write new value to part, then switch the direction
	mcp23017_writePin(p, pin, initial_value);
	
	// now set the pin direction
	uint8_t iodirval;
	uint8_t iodiraddr;
	
	// change it
	if (direction == GPIO_DIR_INPUT)
	p->ddr |= (1 << pin);	// 1==input
	else
	p->ddr &= ~(1 << pin);	// 0=output
	
	if (pin < 8)
	{
		iodiraddr = MCP23017_IODIRA;
		iodirval = p->ddr & 0xff;
	}
	else
	{
		iodiraddr = MCP23017_IODIRB;
		iodirval = (p->ddr & 0xff00) >> 8;
	}

	// write it!
	bsp_i2c_write_bytes(p->i2c_addr, iodiraddr, &iodirval, 1);
}

uint16_t mcp23017_readOutputs(bsp_i2c_gpio_private_t *p)
{
	bsp_i2c_trace("mcp23017 readOutputs");
	return p->data;
}


#endif // CONFIG_BSP_ENBALE_I2C_GPIO_MCP23017