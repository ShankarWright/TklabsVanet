/**
 *	@file	main.c
 *
 *  @brief	Bare Metal Test
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

#include <asf.h>
#include "vanet.h"

int main(void)
{	
    // Initialize everything
    board_init();

	// Start multitasking (i.e. give control to uC/OS-II)
	print_dbg("Starting uC/OS-II\r\n");
	OSStart();
	
    return (0);
}

void App_TCBInitHook (OS_TCB *ptcb)
{
    (void) ptcb;
}

#ifndef CONFIG_BSP_ENABLE_OSTRACKER
void App_TaskSwHook(void)
{
}
#endif
void App_TaskCreateHook (OS_TCB *ptcb)
{
    (void) ptcb;
}
void App_TimeTickHook(void)
{
}
void App_TaskIdleHook(void)
{
	bsp_idle_loop();
}
