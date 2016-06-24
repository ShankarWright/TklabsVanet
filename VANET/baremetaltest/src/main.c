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
#include "tests.h"
#include "conf_apps.h"

int main(void)
{	
    // Initialize everything
    board_init();

#ifdef CONFIG_BSP_UCOS
	
	// Start multitasking (i.e. give control to uC/OS-II)
	print_dbg("Starting uC/OS-II\r\n");
	OSStart();
	
#else // CONFIG_BSP_UCOS
	// Run Tests
	run_tests();
	
	// Done!
	print_dbg("\r\n\r\nAll Test Complete\r\n\r\n");
	
	while (1)
	{
		// nothing left to do
		show_test_menu();
		sleepmgr_enter_sleep();
	}
#endif // CONFIG_BSP_UCOS
    
    return (0);
}

// Even though uC/OS might be turned off - we are still linking with it... so we need these
void App_TCBInitHook (OS_TCB *ptcb)
{
    (void) ptcb;
}
void App_TaskSwHook(void)
{
}    
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
