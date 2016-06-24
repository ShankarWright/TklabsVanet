/**
 *	@file	clcd_pcf8574.h
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


#ifndef CLCD_PCF8574_H
#define CLCD_PCF8574_H

#define PCF8574_SEND_COMMAND	0
#define PCF8574_SEND_DATA		1
#define PCF8574_SEND_FOUR_BITS	2		// used to force write just 4 bits!

typedef struct
{
	uint8_t i2c_addr;
	// PCF8574 only has 8 gpio so we have to assume 4 for control and 4-bit data to lcd
	uint8_t en;
	uint8_t rw;
	uint8_t rs;
	uint8_t bl;
	uint8_t data[4];
	
	bsp_i2c_gpio_t *gpio;
} bsp_clcd_interface_i2c_pcf8574_t;

extern void bsp_clcd_pcf8574_datab(void *params, uint8_t value);
extern void bsp_clcd_pcf8574_data(void *params, char *data);
extern void bsp_clcd_pcf8574_command(void *params, uint8_t command);
extern void bsp_clcd_pcf8574_backlight(void *params, bool on_off);
extern void bsp_clcd_pcf8574_init(void *params);


#endif /* CLCD_PCF8574_H */