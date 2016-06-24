/**
 *	@file	init.c
 *
 *	@brief	PTI kernel board_init
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013-14 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <asf.h>
#include "vanet.h"

#ifdef CONFIG_BSP_UCOS
#include "conf_apps.h"
extern void app_task1_init(void);
extern void app_task2_init(void);
#endif

void board_init(void)
{		
#ifdef CONFIG_BSP_UCOS

	// Initialize BSP
	bsp_init();
	
	// Create our test tasks
	app_task1_init();
	app_task2_init();
	
#else // CONFIG_BSP_UCOS
	
    // make sure interrupts are off
    cpu_irq_disable();
	
	// We're Alive!
	gpio_configure_pin(BSP_INIT_ALIVE_LED, GPIO_DIR_OUTPUT | BSP_LED_POLARITY);
    
    // use the external crystal
    sysclk_init();
	//gpio_configure_pin(BSP_INIT_CLOCKS_LED, GPIO_DIR_OUTPUT | BSP_LED_POLARITY);
	
	// IRQ controller
	INTC_init_interrupts();
	
	// Initialize the USART module for trace messages
	init_dbg_rs232(sysclk_get_pba_hz());
	//gpio_configure_pin(BSP_INIT_DEBUG_LED, GPIO_DIR_OUTPUT | BSP_LED_POLARITY);
	
	// Announce we're running
	print_dbg("\r\n\r\nVANET Daughterboard Alive!\r\n");
	print_dbg(VERSION_ABOUT " (Build " ASTRINGZ(VERSION_SOFTWARE_BUILD) ")\r\n");
	print_dbg(VERSION_COMPILER "\r\n");
	print_dbg(VERSION_HARDWARE_STR "\r\n");
	print_dbg(VERSION_BUILD_DATE "\r\n\r\n");
	
	// initialize the sleep manager (IDLE - FROZEN - STANDBY - STOP - DEEPSTOP - STATIC)
	sleepmgr_init();
	sleepmgr_lock_mode(SLEEPMGR_DEFAULT);		// see hw_vanet_daughterboard_rev_a.h to change
	
	// let it rip
	cpu_irq_enable();
	
#endif //CONFIG_BSP_UCOS
}
