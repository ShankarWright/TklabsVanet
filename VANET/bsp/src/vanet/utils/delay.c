/**
 *	@file	delay.c
 *
 *	@brief	Delay Routines
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

#include "asf.h"
#include "vanet.h"

#ifdef CONFIG_BSP_UCOS
// Note: this should be at least 1/OS_TMR_CFG_TICKS_PER_SEC in ms or else the OS delay calls will not happen!
// Currently OS_TMR_CFG_TICKS_PER_SEC == CONFIG_BSP_RTC_TICK_HZ == 32 so this needs to be at least 32ms
#define BSP_DELAY_UCOS_GRANULARITY ((1000 + OS_TMR_CFG_TICKS_PER_SEC)  / OS_TMR_CFG_TICKS_PER_SEC)	// rounded up
#endif // CONFIG_BSP_UCOS
#define BSP_DELAY_BUSY_GRANULARITY 100

void bsp_delay(uint16_t delay_ms)
{
	int delay_cnt;
	
	//uint32_t t0 = Get_sys_count(), t1;
	
#ifdef CONFIG_BSP_UCOS
	if (OSRunning && OSPrioCur != OS_TASK_IDLE_PRIO)
	{
		// if asked for less than minimum OS delay - do minimum
		if (delay_ms < BSP_DELAY_UCOS_GRANULARITY)
		delay_ms = BSP_DELAY_UCOS_GRANULARITY;
		
		OSTimeDly((delay_ms * OS_TMR_CFG_TICKS_PER_SEC + 500) / 1000);	// round up!
	}
	else
#endif // CONFIG_BSP_UCOS
	{
		while (delay_ms > 0)
		{
			delay_cnt = (delay_ms > 100) ? 100: delay_ms;
			cpu_delay_ms(delay_cnt, sysclk_get_cpu_hz());
			delay_ms -= delay_cnt;
			
			// kick the dog
			//bsp_kick_the_dog();
		}
	}
	
	//t1 = Get_sys_count();
	//print_dbg("d ");
	//print_dbg_ulong(cpu_cy_2_ms(t1-t0,sysclk_get_cpu_hz()));
	//print_dbg(" ms\r\n");
}
