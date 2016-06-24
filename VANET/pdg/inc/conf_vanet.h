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
#define CONFIG_BSP_UCOS

// STI Commands
#define CONFIG_STI_CMD_ALERT
#define CONFIG_STI_CMD_BUF
#define CONFIG_STI_CMD_BUZZ
#define CONFIG_STI_CMD_CLOCK
#define CONFIG_STI_CMD_CP
#define CONFIG_STI_CMD_I2C
#define CONFIG_STI_CMD_MUX
#define CONFIG_STI_CMD_PWM
#define CONFIG_STI_CMD_RESET
#define CONFIG_STI_CMD_TKVS

// Drivers...
#define CONFIG_BSP_ENABLE_ALERT
#define CONFIG_BSP_ENABLE_BUZZER
#define CONFIG_BSP_ENABLE_LCD
#define CONFIG_BSP_ENABLE_LCD_I2C_MCP23017
#define CONFIG_BSP_ENABLE_LCD_I2C_PCF8574
#define CONFIG_BSP_ENABLE_I2C_GPIO
#undef CONFIG_BSP_ENABLE_I2C_GPIO_PCA9501
#define CONFIG_BSP_ENABLE_I2C_GPIO_PCF8574
#define CONFIG_BSP_ENABLE_I2C_GPIO_MCP23017
#define CONFIG_BSP_ENABLE_PIN
#define CONFIG_BSP_ENABLE_PWM
#define CONFIG_BSP_ENABLE_RAM_DUMP
#define CONFIG_BSP_ENABLE_RESET_DECODE

// and Services
#define CONFIG_BSP_ENABLE_BUFFERS
#define CONFIG_BSP_ENABLE_CODEPLUG	
#define CONFIG_BSP_ENABLE_I2C
#define CONFIG_BSP_ENABLE_RTC
#define CONFIG_BSP_ENABLE_MUX		// requires CONFIG_BSP_UCOS
#define CONFIG_BSP_ENABLE_OSTRACKER
#define CONFIG_BSP_ENABLE_STI		// requires CONFIG_BSP_UCOS
#define CONFIG_BSP_ENABLE_TKVS		// requires CONFIG_BSP_UCOS
#define CONFIG_BSP_ENABLE_TERMIOS	// ditto
#define CONFIG_BSP_ENABLE_LOGCAT	// requires somebody to call display (STI will!)

// TKVS Configuration
#define CONFIG_BSP_TKVS_MAX_SUBSCRIPTIONS 32
#undef CONFIG_BSP_TKVS_ENABLE_1STICK

// INTC Configuration
#define CONFIG_BSP_INTC_MAX_INTERRUPTS 16

#endif /* CONF_VANET_H */