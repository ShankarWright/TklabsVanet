/**
 *	@file	bsp_idle.c
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

#include <asf.h>
#include "vanet.h"

#define BSP_IDLE_MAX_PER_TICK_CALLBACKS	8
static bsp_idle_callback_t s_per_tick_callbacks[BSP_IDLE_MAX_PER_TICK_CALLBACKS];
static uint8_t s_per_tick_cnt = 0;

#define BSP_IDLE_MAX_GENERAL_CALLBACKS	4
static bsp_idle_callback_t s_general_callbacks[BSP_IDLE_MAX_GENERAL_CALLBACKS];
static uint8_t s_general_cnt = 0;

void bsp_idle_register_idle_function(bsp_idle_callback_t func, bsp_idle_callback_type type)
{
	if (type == BSP_IDLE_ONCE_PER_TICK)
	{
		if (s_per_tick_cnt == BSP_IDLE_MAX_PER_TICK_CALLBACKS)
			bsp_reset(0x1d13);
			
		s_per_tick_callbacks[s_per_tick_cnt++] = func;
	}
	else
	{
		if (s_general_cnt == BSP_IDLE_MAX_GENERAL_CALLBACKS)
			bsp_reset(0x1d13);
			
		s_general_callbacks[s_general_cnt++] = func;
	}
}

void bsp_idle_loop(void)
{
    static uint32_t s_last_check = 0;

	// Idle Loop Callbacks that are called maximum of once per tick
    if (bsp_rtc_get_ticks() != s_last_check)
    {
	    s_last_check = bsp_rtc_get_ticks();
		
		for (int i=0; i<s_per_tick_cnt; i++)
		{
			s_per_tick_callbacks[i]();
		}
	}	
	
	// Idle Loop Callbacks called always
	for (int i=0; i<s_general_cnt; i++)
	{
		s_general_callbacks[i]();
	}	

	sleepmgr_enter_sleep();
}
