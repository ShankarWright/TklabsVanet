/**
 *	@file	bsp_init.c
 *
 *	@brief	BSP Initialization
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

void bsp_init(void)
{
	// make sure interrupts are off
	cpu_irq_disable();
		
	// We're Alive!
	gpio_configure_pin(BSP_INIT_ALIVE_LED, GPIO_DIR_OUTPUT | BSP_LED_POLARITY);
	gpio_configure_pin(BSP_RTC_LED, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
	gpio_configure_pin(BSP_GPS_SYNC_LED, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
	gpio_configure_pin(BSP_LED4, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
	gpio_configure_pin(BSP_LED5, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
	gpio_configure_pin(BSP_LED6, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
	gpio_configure_pin(BSP_LED7, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
	gpio_configure_pin(BSP_LED8, GPIO_DIR_OUTPUT | GPIO_INIT_LOW);
		
	// use the external crystal
	sysclk_init();
			
    // Initialize the USART module for trace messages
    init_dbg_rs232(sysclk_get_pba_hz());
    
    // check why we reset
    bsp_reset_init();
    
	// IRQ controller
	INTC_init_interrupts();
    
	#ifdef CONFIG_BSP_UCOS
	// Initialize the OS
	OSInit();
	#endif
		
	// initialize the sleep manager (IDLE - FROZEN - STANDBY - STOP - DEEPSTOP - STATIC)
	sleepmgr_init();
	sleepmgr_lock_mode(SLEEPMGR_DEFAULT);
	
	#ifdef CONFIG_BSP_ENABLE_TKVS
	bsp_tkvs_init();
	#endif
		
	#ifdef CONFIG_BSP_ENABLE_TERMIOS
	// Warning: print_dbg() will not work until here!
	bsp_termios_init();
	#endif
	
	// Initilaize STI
	#ifdef CONFIG_BSP_ENABLE_STI
	// Warning: We cannot register STI commands until after this - this affects TKVS mainly...
	bsp_sti_init();
	#endif
    
    #ifdef CONFIG_BSP_ENABLE_BUFFERS
    bsp_buffers_init();
    #endif
    
	#if defined CONFIG_BSP_ENABLE_TKVS && defined CONFIG_BSP_ENABLE_STI
	bsp_tkvs_sti_init();
	#endif 
	
	#ifdef CONFIG_BSP_ENABLE_LOGCAT
	bsp_logcat_init();
	#endif
		
	
	//
	// Once we get here - we should be able to configure anything and everything as STI, TKVS, and Logcat are all up
	//
	
	// Initialize Codeplug
	#ifdef CONFIG_BSP_ENABLE_CODEPLUG
	bsp_cp_init();
	#endif 
	
	// Initialize RTC/AST
	#ifdef CONFIG_BSP_ENABLE_RTC
	bsp_rtc_init();
	#endif
	
	// Initialize I2C
	#ifdef CONFIG_BSP_ENABLE_I2C
	bsp_i2c_init();
	#endif
	
	// Initialzie MUX
	#ifdef CONFIG_BSP_ENABLE_MUX
	bsp_mux_init();
	#endif
	
	// Initialize Pin Driver
	#ifdef CONFIG_BSP_ENABLE_PIN
	bsp_pin_init();
	#endif
    
    // Initialize Buzzer
    #ifdef CONFIG_BSP_ENABLE_BUZZER
    bsp_buzzer_init();
    #endif
    
    // Initialize Alert Tones
    #ifdef CONFIG_BSP_ENABLE_ALERT
    bsp_alert_init();
    #endif
	
	// Initialize PWM
	#ifdef CONFIG_BSP_ENABLE_PWM
	bsp_pwm_init();
	#endif
    
    #ifdef CONFIG_BSP_ENABLE_OSTRACKER
    bsp_ostracker_init();
    #endif
}