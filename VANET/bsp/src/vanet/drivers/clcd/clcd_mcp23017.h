/**
 *	@file	clcd_mcp23017.h
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


#ifndef CLCD_MSP23017_H
#define CLCD_MSP23017_H

//#include <i2c.h>

#define MCP23017_SEND_COMMAND	0
#define MCP23017_SEND_DATA		1
#define MCP23017_SEND_FOUR_BITS	2		// used to force write just 4 bits!

typedef struct
{
	uint8_t i2c_addr;
	// Even though the MCP23017 has 2x8 gpio - the lcd's are typically wired in 4-bit mode
	uint8_t en;
	uint8_t rw;
	uint8_t rs;
	uint8_t bl;
	uint8_t data[4];
	
	bsp_i2c_gpio_t *gpio;
} bsp_clcd_interface_i2c_mcp23017_t;


extern void bsp_clcd_mcp23017_datab(void *params, uint8_t value);
extern void bsp_clcd_mcp23017_data(void *params, char *data);
extern void bsp_clcd_mcp23017_command(void *params, uint8_t command);
extern void bsp_clcd_mcp23017_backlight(void *params, bool on_off);
extern void bsp_clcd_mcp23017_init(void *params);


#endif /* CLCD_MSP23017_H */