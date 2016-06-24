/**
 *	@file	termios.h
 *
 *	@brief	General terminal interface to control asynchronous communications ports combined with the low level driver.
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

#ifndef BSP_TERMIOS_H
#define BSP_TERMIOS_H

/// Termios Input Events
enum
{
	BSP_TERMIOS_INPUT_READY		= 0x0001,
	BSP_TERMIOS_INPUT_RX		= 0x0002,
	BSP_TERMIOS_INPUT_SIGNAL	= 0x0004,
};

/// Termios Signals
enum
{
	BSP_TERMIOS_SIGNAL_INTERRUPT,
	BSP_TERMIOS_SIGNAL_UP,
	BSP_TERMIOS_SIGNAL_DOWN,
};

/// Termios Port Mode
typedef enum
{
	BSP_TERMIOS_MODE_RAW,
	BSP_TERMIOS_MODE_CANONICAL,
} bsp_termios_mode;

/// Termios Buffer Mode
typedef enum
{
	BSP_TERMIOS_BUFFER_NONE,
	BSP_TERMIOS_BUFFER_MIN,
} bsp_termios_buffer;

void bsp_termios_init(void);

void bsp_termios_idle_loop(void);

bool bsp_termios_find_port(const char *name, uint8_t *port);

void bsp_termios_set_mode(uint8_t port, bsp_termios_mode mode, bool echo);

void bsp_termios_set_buffer(uint8_t port, bsp_termios_buffer buffer);

void bsp_termios_flush(uint8_t port);

#define BSP_TERMIOS_DRAIN_UNTIL_EMPTY	UINT16_MAX

void bsp_termios_drain(uint8_t port, uint16_t needed);

void bsp_termios_write(uint8_t port, const unsigned char *buf, uint16_t len);

#define bsp_termios_write_str(_port, _string)	bsp_termios_write(_port, (unsigned char *)_string, strlen(_string))

int bsp_termios_printf(uint8_t port, const char *fmt, ...);

bool bsp_termios_set_input(uint8_t port, const char *string);

/// Termios idle handler (per port handler)
typedef void (*bsp_termios_idle_handler_t)(uint8_t port);

/// Register an idle handler
void bsp_termios_register_idle_handler(uint8_t port, bsp_termios_idle_handler_t handler);

/// Map Termios Port to TKVS Source
#define BSP_TERMIOS_PORT_TO_TKVS_SOURCE(_port)		(BSP_TKVS_SRC_TERMIOS_PORT_START + _port)

#ifdef CONFIG_BSP_ENABLE_TERMIOS
// Termios Configuration
#include "conf_termios.h"

extern bsp_sti_command_t termios_command;

/// Special port number to synchronously print to the debug port.  Useful for init, reset dump, etc
#define BSP_TERMIOS_RAW_DEBUG_PORT                  BSP_TERMIOS_COUNT

#endif // CONFIG_BSP_ENABLE_TERMIOS

#endif // BSP_TERMIOS_H