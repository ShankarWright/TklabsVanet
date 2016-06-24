/**
 *	@file	sti.h
 *
 *	@brief	System Test Interface (STI)
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

#ifndef BSP_STI_H
#define BSP_STI_H

/// Help function for help providers
#define STI_HELP(x)                 .help = (x),

/// The maximum number of args STI will process
#define STI_MAX_ARGS                8

/// History command length
#define MAX_COMMAND_LENGTH          96

/// Depth of history
#define STI_CMD_HISTORY_LEN			16

/// STI command handler.  The first arg is the command name...
typedef void (*bsp_sti_handler_t)(int argc, char** argv, uint8_t port);

/// STI command
typedef struct _bsp_sti_command_t
{
	const char* name;                               ///< The command name
	bsp_sti_handler_t handler;                      ///< The command handler
	uint8_t minArgs;                                ///< The minimum number of args to this command
	uint8_t maxArgs;                                ///< The maximum number of args to this command
	const char* help;                               ///< The help output
	struct _bsp_sti_command_t* next;                ///< Internal use
} bsp_sti_command_t;

/// Initialize STI
extern void bsp_sti_init(void);

/// Display Build Info
extern void bsp_sti_show_build_info(uint8_t port);

/**
 * Register a STI command
 *
 * @param cmd The command
 */
extern void bsp_sti_register_command(bsp_sti_command_t* cmd);

#endif // BSP_STI_H