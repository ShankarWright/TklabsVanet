/**
 *	@file	conf_vanet.h
 *
 *	@brief	Configure VANET BSP
 *
 *  The intent of this configuration file is like the other ASF configuration
 *  files like conf_clock.h, etc.
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

#ifndef CONF_VANET_H
#define CONF_VANET_H

// Use uC/OS-II
#undef CONFIG_BSP_UCOS

// Drivers...
#define CONFIG_BSP_ENABLE_LCD
#define CONFIG_BSP_ENABLE_LCD_I2C_MCP23017
#define CONFIG_BSP_ENABLE_LCD_I2C_PCF8574
#define CONFIG_BSP_ENABLE_I2C_GPIO
#define CONFIG_BSP_ENABLE_I2C_GPIO_MCP23017
#define CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574

// and Services
#define CONFIG_BSP_ENABLE_I2C
#define CONFIG_BSP_ENABLE_RTC
#ifdef CONFIG_BSP_UCOS
// these require CONFIG_BSP_UCOS
#define CONFIG_BSP_ENABLE_LOGCAT	// requires STI
#define CONFIG_BSP_ENABLE_MUX		// requires LOGCAT, TERMIOS, and TKVS	
#define CONFIG_BSP_ENABLE_STI		// requires TERMIOS and TKVS
#define CONFIG_BSP_ENABLE_TERMIOS
#define CONFIG_BSP_ENABLE_TKVS
#endif

// TKVS Configuration
#ifdef CONFIG_BSP_ENABLE_TKVS
#define CONFIG_BSP_TKVS_MAX_SUBSCRIPTIONS 32
#undef CONFIG_BSP_TKVS_ENABLE_1STICK
#endif

// INTC Configuration
#define CONFIG_BSP_INTC_MAX_INTERRUPTS 16

#endif /* CONF_VANET_H */