/**
 *	@file	clcd.c
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


/************************************************************************/
/* LCD Utility                                                          */
/************************************************************************/
void bsp_clcd_delay_us(unsigned long delay)
{
	// make OS aware when we get an OS!
	cpu_delay_us(delay, sysclk_get_cpu_hz());
}

#ifdef BSP_CLCD_TRACE
void bsp_clcd_trace(const char *s)
{
#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_print(BSP_LOGCAT_CLCD, s);
#else
	print_dbg(s);
	print_dbg("\r\n");
#endif
}

void bsp_clcd_trace_ul(const char *s, unsigned long p)
{
#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_CLCD, "%s %u", s, p);
#else
	print_dbg(s);
	print_dbg(" ");
	print_dbg_int(p);
	print_dbg("\r\n");
#endif
}

void bsp_clcd_trace_hex(const char *s, unsigned long p)
{
#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_CLCD, "%s %08X", s, p);
#else
	print_dbg(s);
	print_dbg(" ");
	print_dbg_hex(p);
	print_dbg("\r\n");
#endif
}

void bsp_clcd_trace_2(const char *s, unsigned long p1, unsigned long p2)
{
#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_printf(BSP_LOGCAT_CLCD, "%s %u %u", s, p1, p2);
#else
	print_dbg(s);
	print_dbg(" ");
	print_dbg_int(p1);
	print_dbg(" ");
	print_dbg_int(p2);
	print_dbg("\r\n");
#endif
}
#endif // BSP_CLCD_TRACE

/************************************************************************/
/* HD44780 SPECIFIC                                                     */
/************************************************************************/
#define OP_CLEAR_DISPLAY                            0x01
#define OP_RETURN_HOME                              0x02
#define OP_ENTRY_MODE_SET(ID,S)                     (0x04 | ((ID) << 1) | (S))
#define OP_DISPLAY_ON(D,C,B)                        (0x08 | ((D) << 2) | ((C) << 1) | (B))
#define OP_SHIFT(SC,RL)                             (0x10 | ((SC) << 3) | ((RL) << 2))
#define OP_FUNCTION_SET(DL,N,F)                     (0x20 | ((DL) << 4) | ((N) << 3) | ((F) << 2))
#define OP_CGRAM_ADDR(addr)                         (0x40 | ((addr) & 0x3f))
#define OP_DDRAM_ADDR(addr)                         (0x80 | ((addr) & 0x7f))

// max display is 20x4 for a HD44780
#define BSP_CLCD_HD44780_MAX_ROWS 4
#define BSP_CLCD_HD44780_MAX_COLS 20
static uint8_t bsp_clcd_hd44780_line_address[BSP_CLCD_HD44780_MAX_ROWS];

typedef struct {
	void (*init)(void *p);
	void (*backlight)(void *p, bool on_off);
	void (*command)(void *p, uint8_t command);
	void (*data)(void *p, char *data);
	void (*datab)(void *p, uint8_t value);
} bsp_clcd_hd44780_t;

static void bsp_clcd_hd44780_setup(bsp_clcd_private_t *p, uint8_t cols, uint8_t rows)
{
	bsp_clcd_trace_2("hd44780 setup", cols, rows);
	
	p->cols = cols;
	p->rows = rows;
	
	if (p->rows == 4)
	{
		bsp_clcd_hd44780_line_address[0] = 0;
		bsp_clcd_hd44780_line_address[1] = 0x40;
		bsp_clcd_hd44780_line_address[2] = 0 + cols;
		bsp_clcd_hd44780_line_address[3] = 0x40 + cols;
	}	
	else if (p->rows == 2)
	{
		bsp_clcd_hd44780_line_address[0] = 0;
		bsp_clcd_hd44780_line_address[1] = 0x40;
	}
}

static void bsp_clcd_hd44780_backlight(bsp_clcd_private_t *p, bool on_off)
{
	bsp_clcd_hd44780_t *hd44780 = (bsp_clcd_hd44780_t *)p->driver_params;
	bsp_clcd_trace_ul("hd44780 backlight", on_off);
	
	hd44780->backlight(p->interface_params, on_off);
}

static void bsp_clcd_hd44780_display(bsp_clcd_private_t *p, bool on_off)
{
	bsp_clcd_hd44780_t *hd44780 = (bsp_clcd_hd44780_t *)p->driver_params;
	bsp_clcd_trace_ul("hd44780 display", on_off);
	
	hd44780->command(p->interface_params, OP_DISPLAY_ON(on_off, 0, 0));	// display, no cursor, no blink
}


static bool bsp_clcd_hd44780_write_line(bsp_clcd_private_t *p, uint32_t line, bsp_clcd_align_t align, const char* text)
{
	bsp_clcd_hd44780_t *hd44780 = (bsp_clcd_hd44780_t *)p->driver_params;
	if (line >= p->rows) return false;
	
	uint8_t offset = 0;
	int len = strlen(text);
	char data[BSP_CLCD_HD44780_MAX_COLS+1];
	
	if (len > p->cols) len = p->cols;
	
	switch (align)
	{
		case BSP_CLCD_ALIGN_LEFT:
		offset = 0;
		break;
		
		case BSP_CLCD_ALIGN_CENTER:
		offset = (p->cols/2) - (len+1) / 2;  // round length up to pad right
		break;
		
		case BSP_CLCD_ALIGN_RIGHT:
		offset = p->cols - len;
		break;
	}
	
	memset(data,' ',p->cols);
	memcpy(data+offset,text,len);
	data[p->cols] = '\0';
	
	hd44780->command(p->interface_params, OP_DDRAM_ADDR(bsp_clcd_hd44780_line_address[line]));
	hd44780->data(p->interface_params, data);
	
	return true;
}

static bool bsp_clcd_hd44780_write_pos(bsp_clcd_private_t *p, uint32_t line, uint32_t column, const char* text)
{
	bsp_clcd_hd44780_t *hd44780 = (bsp_clcd_hd44780_t *)p->driver_params;
	if (line >= p->rows || column >= p->cols) return false;
	
	char data[BSP_CLCD_HD44780_MAX_COLS+1];
	memset(data, '\0', sizeof(data));
	
	uint8_t addr = bsp_clcd_hd44780_line_address[line] + column;
	int len = strlen(text);
	
	if (column + len > p->cols) len = p->cols - column;
	strncpy(data, text, len);
	
	hd44780->command(p->interface_params, OP_DDRAM_ADDR(addr));
	hd44780->data(p->interface_params, data);
	
	return true;
}

static void bsp_clcd_hd44780_clear(bsp_clcd_private_t *p)
{
	bsp_clcd_hd44780_t *hd44780 = (bsp_clcd_hd44780_t *)p->driver_params;
	bsp_clcd_trace("hd44780 clear");
	
	hd44780->command(p->interface_params, OP_CLEAR_DISPLAY);
	bsp_clcd_delay_us(3000);
}

static void bsp_clcd_hd44780_create_char(bsp_clcd_private_t *p, uint8_t cgram_loc, uint8_t charmap[])
{
	bsp_clcd_hd44780_t *hd44780 = (bsp_clcd_hd44780_t *)p->driver_params;
	cgram_loc &= 0x07;		// only 8 locations
	
	hd44780->command(p->interface_params, OP_CGRAM_ADDR(cgram_loc << 3));
	for (int i=0; i<8; i++)
	{
		hd44780->datab(p->interface_params, charmap[i]);
	}
}

static void bsp_clcd_hd44780_selftest(bsp_clcd_private_t *p)
{
	bsp_clcd_trace("hd44780 selftest");
	uint8_t rows = p->rows;
	uint8_t cols = p->cols;

	bsp_clcd_hd44780_clear(p);
	bsp_clcd_hd44780_write_line(p, 0, BSP_CLCD_ALIGN_CENTER, "Selftest!");
	for (int i=1; i<rows; i++)
	{
		bsp_clcd_hd44780_write_line(p, i, BSP_CLCD_ALIGN_CENTER, "Line");
	}
	
	uint8_t smiley[8] = {
		0x00,	// B00000,
		0x11,	// B10001,
		0x00,	// B00000,
		0x00,	// B00000,
		0x11,	// B10001,
		0x0e,	// B01110,
		0x00,	// B00000,
	};

	bsp_clcd_hd44780_create_char(p, 0, smiley);
	bsp_clcd_hd44780_write_pos(p, 0, 0, "\x08");
	bsp_clcd_hd44780_write_pos(p, 0, cols-1, "\x08");
	bsp_clcd_hd44780_write_pos(p, rows-1, 0, "\x08");
	bsp_clcd_hd44780_write_pos(p, rows-1, cols-1, "\x08");
}	

static void bsp_clcd_hd44780_init(bsp_clcd_private_t *p)
{
	bsp_clcd_trace("hd44780 init ");
	
	bsp_clcd_hd44780_t *hd44780 = (bsp_clcd_hd44780_t *)p->driver_params;
	hd44780->init(p->interface_params);
}

/************************************************************************/
/* HIGH LEVEL                                                           */
/************************************************************************/
// make our own copy of bcp_lcd_t and params
bsp_clcd_t * bsp_clcd(bsp_clcd_type_t type, bsp_clcd_interface_t interface, void *interface_params)
{	
	bsp_clcd_t *lcd = malloc(sizeof(bsp_clcd_t));
	
	// Hitachi HD44780 Based Display
	if (type == BSP_CLCD_HD44780)
	{
		bsp_clcd_hd44780_t *hd44780;
		
		lcd->init = &bsp_clcd_hd44780_init;
		lcd->setup = &bsp_clcd_hd44780_setup;
		lcd->backlight = &bsp_clcd_hd44780_backlight;
		lcd->display = &bsp_clcd_hd44780_display;
		lcd->selftest = &bsp_clcd_hd44780_selftest;
		lcd->clear = &bsp_clcd_hd44780_clear;
		lcd->write_line = &bsp_clcd_hd44780_write_line;
		lcd->write_pos = &bsp_clcd_hd44780_write_pos;
		
		switch (interface)
		{
#ifdef CONFIG_BSP_ENABLE_LCD_I2C_PCF8574
			case BSP_CLCD_I2C_PCF8574:
				// make a copy of interface params for this interface
				lcd->p.interface_type = interface;
				lcd->p.interface_params = malloc(sizeof(bsp_clcd_interface_i2c_pcf8574_t));
				memcpy(lcd->p.interface_params, interface_params, sizeof(bsp_clcd_interface_i2c_pcf8574_t));
				
				// setup pointers to interface
				lcd->p.driver_params = malloc(sizeof(bsp_clcd_hd44780_t));
				hd44780 = (bsp_clcd_hd44780_t *) lcd->p.driver_params;
				hd44780->init = &bsp_clcd_pcf8574_init;
				hd44780->backlight = &bsp_clcd_pcf8574_backlight;
				hd44780->data = &bsp_clcd_pcf8574_data;
				hd44780->command = &bsp_clcd_pcf8574_command;
				hd44780->datab = &bsp_clcd_pcf8574_datab;
				break;
#endif
#ifdef CONFIG_BSP_ENABLE_LCD_I2C_MCP23017
			case BSP_CLCD_I2C_MCP23017:
				// make a copy of interface params for this interface
				lcd->p.interface_type = interface;
				lcd->p.interface_params = malloc(sizeof(bsp_clcd_interface_i2c_mcp23017_t));
				memcpy(lcd->p.interface_params, interface_params, sizeof(bsp_clcd_interface_i2c_mcp23017_t));
			
				// setup pointers to interface
				lcd->p.driver_params = malloc(sizeof(bsp_clcd_hd44780_t));
				hd44780 = (bsp_clcd_hd44780_t *) lcd->p.driver_params;
				hd44780->init = &bsp_clcd_mcp23017_init;
				hd44780->backlight = &bsp_clcd_mcp23017_backlight;
				hd44780->data = &bsp_clcd_mcp23017_data;
				hd44780->command = &bsp_clcd_mcp23017_command;
				hd44780->datab = &bsp_clcd_mcp23017_datab;
				break;
#endif
			default:
				free(lcd);
				lcd = NULL;
				break;
		}
	}
	else
	{
		free(lcd);
		lcd = NULL;
	}	
	
	// actually initialize lcd
	if (lcd)	
		lcd->init(&lcd->p);
	
#ifdef BSP_CLCD_TRACE
  #ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_print(BSP_LOGCAT_CLCD, "bsp_lcd_t");
	bsp_logcat_dump(BSP_LOGCAT_CLCD, (const uint8_t *)lcd, sizeof(bsp_clcd_t));
  #else
	print_dbg("bsp_lcd_t:\r\n");
	print_dbg_array((const uint8_t *)lcd, sizeof(bsp_clcd_t));
  #endif
#endif
	
	return lcd;
}