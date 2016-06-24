/**
 *	@file	clcd.h
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


#ifndef NEW_LCD_H_
#define NEW_LCD_H_

/************************************************************************/
/* api interfaces for lcd types                                         */
/************************************************************************/
#include "clcd_pcf8574.h"
#include "clcd_mcp23017.h"

/************************************************************************/
/* debug macros                                                         */
/************************************************************************/
#undef BSP_CLCD_TRACE
#ifdef BSP_CLCD_TRACE
extern void bsp_clcd_trace(const char *s);
extern void bsp_clcd_trace_ul(const char *s, unsigned long p);
extern void bsp_clcd_trace_hex(const char *s, unsigned long p);
extern void bsp_clcd_trace_2(const char *s, unsigned long p1, unsigned long p2);
#else // BSP_CLCD_TRACE
#define bsp_clcd_trace(s)
#define bsp_clcd_trace_ul(s, p)
#define bsp_clcd_trace_hex(s, p)
#define bsp_clcd_trace_2(s, p1, p2)
#endif // BSP_CLCD_TRACE

/************************************************************************/
/* internal utils                                                       */
/************************************************************************/
extern void bsp_clcd_delay_us(unsigned long delay);

/************************************************************************/
/* types / typedefs                                                     */
/************************************************************************/
typedef enum
{
	BSP_CLCD_ALIGN_LEFT,
	BSP_CLCD_ALIGN_CENTER,
	BSP_CLCD_ALIGN_RIGHT
} bsp_clcd_align_t;

typedef enum
{
	BSP_CLCD_HD44780,
} bsp_clcd_type_t;

typedef enum
{
	BSP_CLCD_I2C_PCF8574,	///< Parallel Interface via PCF8574 I2C Expander (4 bit data, 4 bit control)
	BSP_CLCD_I2C_MCP23017,	///< Parallel Interface via MCP23017 I2C Expander 
} bsp_clcd_interface_t;

typedef struct  
{
	uint8_t interface_type;
	void *driver_params;
	void *interface_params;
	uint8_t rows, cols;
} bsp_clcd_private_t;

typedef struct
{
	// Functions LCD Driver Must Implement
	void (*init)(bsp_clcd_private_t *p);
	void (*setup)(bsp_clcd_private_t *p, uint8_t cols, uint8_t rows);
	void (*display)(bsp_clcd_private_t *p, bool on_off);
	void (*backlight)(bsp_clcd_private_t *p, bool on_off);
	void (*selftest)(bsp_clcd_private_t *p);
	void (*clear)(bsp_clcd_private_t *p);
	bool (*write_line)(bsp_clcd_private_t *p, uint32_t line, bsp_clcd_align_t align, const char *text);
	bool (*write_pos)(bsp_clcd_private_t *p, uint32_t line, uint32_t col, const char *text);

	// private info
	bsp_clcd_private_t	p;
} bsp_clcd_t;

/************************************************************************/
/* API Prototypes                                                       */
/************************************************************************/
bsp_clcd_t * bsp_clcd(bsp_clcd_type_t type, bsp_clcd_interface_t interface, void *interface_params);

static inline void bsp_clcd_setup(bsp_clcd_t *lcd, uint8_t cols, uint8_t rows)
{
	lcd->setup(&lcd->p, cols, rows);
}

static inline void bsp_clcd_backlight(bsp_clcd_t *lcd, bool on_off)
{
	lcd->backlight(&lcd->p, on_off);
}

static inline void bsp_clcd_display(bsp_clcd_t *lcd, bool on_off)
{
	lcd->display(&lcd->p, on_off);
}

static inline void bsp_clcd_on(bsp_clcd_t *lcd)
{
	lcd->backlight(&lcd->p, true);
	lcd->display(&lcd->p ,true);
}

static inline void bsp_clcd_selftest(bsp_clcd_t *lcd)
{
	lcd->selftest(&lcd->p);
}

static inline void bsp_clcd_clear(bsp_clcd_t *lcd)
{
	lcd->clear(&lcd->p);
}

static inline bool bsp_clcd_write_line(bsp_clcd_t *lcd, uint32_t line, bsp_clcd_align_t align, const char *text)
{
	return lcd->write_line(&lcd->p, line, align, text);
}

static inline bool bsp_clcd_write_pos(bsp_clcd_t *lcd, uint32_t line, uint32_t col, const char *text)
{
	return lcd->write_pos(&lcd->p, line, col, text);
}

#endif /* NEW_LCD_H_ */