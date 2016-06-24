/**
 *	@file	bsp_idle.h
 *
 *	@brief	BSP Idle Loop
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013-2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef BSP_IDLE_H
#define BSP_IDLE_H

/// Run idle loop
extern void bsp_idle_loop(void);

/// Idle Loop Function Prototype
typedef void (*bsp_idle_callback_t)(void);

typedef enum
{
	BSP_IDLE_ONCE_PER_TICK,
	BSP_IDLE_ALWAYS,
} bsp_idle_callback_type;

/// Register an idle function to be called
void bsp_idle_register_idle_function(bsp_idle_callback_t func, bsp_idle_callback_type type);

#endif // BSP_IDLE_H