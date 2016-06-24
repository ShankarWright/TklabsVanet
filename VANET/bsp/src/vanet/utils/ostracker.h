/**
 *	@file	ostracker.h
 *
 *	@brief	Utilities to track OS behavior
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef OSTRACKER_H
#define OSTRACKER_H

/**
 * @ingroup group_bsp_utils_util
 *
 * OS tracking functions
 *
 * @{
 */

#include <avr32/io.h>
#include "compiler.h"

/** @name API
 *
 *  @{
 */

/// Initialization
extern void bsp_ostracker_init(void);

/// Print the OS configuration
extern void bsp_print_os_config(uint8_t port);

/// Print stack usage
extern void bsp_print_stack_usage(uint8_t port);

/// Print stack switches
extern void bsp_print_stack_switches(uint8_t port);

/// @}

/// @}

#endif
