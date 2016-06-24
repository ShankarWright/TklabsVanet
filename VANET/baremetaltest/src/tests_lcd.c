/**
 *	@file	tests.c
 *
 *	@brief	Test routines code
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
#include "tests.h"
#include "vanet.h"




/************************************************************************/
/* LCD over TWI/I2C Tests                                               */
/************************************************************************/

bsp_clcd_t *lcd1= NULL, *lcd2 = NULL, *lcd3 = NULL;

void i2c_init_lcd1(void)
{	
	bsp_clcd_interface_i2c_pcf8574_t lcd1_i2c_params =
	{
		.i2c_addr = 0x3e,
		.en = 2,
		.rw = 1,
		.rs = 0,
		.bl = 3,				// gpio 3
	};
	
	twim_init();
	if (lcd1 == NULL)
	{
		lcd1 = bsp_clcd(BSP_CLCD_HD44780, BSP_CLCD_I2C_PCF8574, (void *)&lcd1_i2c_params);
		bsp_clcd_setup(lcd1, 20, 4);
		bsp_clcd_backlight(lcd1, true);
		bsp_clcd_display(lcd1, true);
	}		
}

void i2c_init_lcd2(void)
{
	bsp_clcd_interface_i2c_pcf8574_t lcd2_i2c_params =
	{
		.i2c_addr = 0x27,
		.en = 2,
		.rw = 1,
		.rs = 0,
		.bl = 3,				// gpio 3
	};
	
	twim_init();
	if (lcd2 == NULL) {
		lcd2 = bsp_clcd(BSP_CLCD_HD44780, BSP_CLCD_I2C_PCF8574, (void *)&lcd2_i2c_params);
		bsp_clcd_setup(lcd2, 20, 4);
		bsp_clcd_backlight(lcd2, true);
		bsp_clcd_display(lcd2, true);
	}		
}

void i2c_init_lcd3(void)
{
	bsp_clcd_interface_i2c_mcp23017_t lcd3_i2c_params =
	{
		.i2c_addr = 0x20,
		.en = 13,
		.rw = 14,
		.rs = 15,
		.bl = 6,
		.data[0] = 12,	// d4
		.data[1] = 11,	// d5
		.data[2] = 10,	// d6
		.data[3] = 9,	// d7
	};
	
	twim_init();
	if (lcd3 == NULL) {
		lcd3 = bsp_clcd(BSP_CLCD_HD44780, BSP_CLCD_I2C_MCP23017, (void *)&lcd3_i2c_params);
		bsp_clcd_setup(lcd3, 16, 2);
		bsp_clcd_backlight(lcd3, true);
		bsp_clcd_display(lcd3, true);
	}
}

void i2c_lcd_test_line_pos(void)
{
	i2c_init_lcd1();
	i2c_init_lcd2();
	
	char data[2] = { 'a', '\0' };
	char ldata[21];
	memset(ldata, '\0', 21);	
	
	for (int i='A'; i<='Z'; i++)
	{
		memset(ldata, i+0x20, 20);
		data[0] = i;
		
		// update lcd1 character at a time
		// update lcd2 line at a time
		for (int row=0; row<20; row++)
		{
			for (int col=0; col<20; col++)
			{
				bsp_clcd_write_pos(lcd1, row, col, data);
			}
			bsp_clcd_write_line(lcd2, row, BSP_CLCD_ALIGN_LEFT, ldata);
		}
	}
}

void i2c_lcd_test_moving(void)
{
	#define WIPE_SPEED 10
	
	i2c_init_lcd1();
	i2c_init_lcd2();

	for (int j=0; j<4; j++)
	{
	
		bsp_clcd_write_pos(lcd1, j, 0, "**");
		cpu_delay_ms(WIPE_SPEED, sysclk_get_cpu_hz());
		for (int i=0; i<18; i++)
		{
			bsp_clcd_write_pos(lcd1, j, i, " **");
			cpu_delay_ms(WIPE_SPEED, sysclk_get_cpu_hz());
		}
		bsp_clcd_write_pos(lcd1, j, 18, "  ");
		bsp_clcd_write_pos(lcd2, j, 0, "**");
		cpu_delay_ms(WIPE_SPEED, sysclk_get_cpu_hz());
		for (int i=0; i<18; i++)
		{
			bsp_clcd_write_pos(lcd2, j, i, " **");
			cpu_delay_ms(WIPE_SPEED, sysclk_get_cpu_hz());
		}
		bsp_clcd_write_pos(lcd2, j, 18, "  ");
	}	
}

void i2c_lcd_basic(void)
{
	i2c_init_lcd1();
	i2c_init_lcd2();
	i2c_init_lcd3();
		

	bsp_clcd_clear(lcd1);
	bsp_clcd_write_line(lcd1, 0, BSP_CLCD_ALIGN_CENTER, "LCD #1");
	bsp_clcd_write_line(lcd1, 2, BSP_CLCD_ALIGN_CENTER, "Alive!");
	bsp_clcd_clear(lcd2);
	
	bsp_clcd_write_line(lcd2, 0, BSP_CLCD_ALIGN_CENTER, "LCD #2");
	bsp_clcd_write_line(lcd2, 2, BSP_CLCD_ALIGN_CENTER, "Alive!");

	bsp_clcd_clear(lcd3);
	bsp_clcd_write_line(lcd3, 0, BSP_CLCD_ALIGN_CENTER, "LCD #3");
	bsp_clcd_write_line(lcd3, 1, BSP_CLCD_ALIGN_CENTER, "Alive!");
}


void i2c_abstract_class(void)
{
	twim_init();
	i2c_init_lcd3();
	bsp_clcd_write_line(lcd3, 0, BSP_CLCD_ALIGN_CENTER, "LCD #3");
	bsp_clcd_write_line(lcd3, 1, BSP_CLCD_ALIGN_CENTER, "Alive!");
}



