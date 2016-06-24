/**
 *	@file	pin.h
 *
 *	@brief	I/O Pin Driver
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

#ifndef BSP_PIN_H
#define BSP_PIN_H

#ifdef CONFIG_BSP_ENABLE_PIN
#include "conf_input.h"

/**
 * @defgroup group_bsp_drivers_pin I/O Pin Driver
 *
 * Driver reports changes to I/O pins as events
 *
 * @{
 */

/// Initialize PIN Driver
void bsp_pin_init(void);

/// Read current PIN event state - Generates the TKVS Pin Event Again
void bsp_pin_read(enum bsp_pin_events event);

/// Generate Pin Events
void bsp_pin_check(void);

/// @}

#endif // CONFIG_BSP_ENABLE_PIN
#endif // BSP_PIN_H