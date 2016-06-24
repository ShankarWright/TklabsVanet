/**
 *	@file	hw_vanet_daughterboard_rev_a.h
 *
 *	@brief	VANET Daughterboard Rev A
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
 *  (C) Copyright 2013-14 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef HW_VANET_DAUGHTERBOARD_REV_A_H
#define HW_VANET_DAUGHTERBOARD_REV_A_H

/* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */

#include "conf_vanet.h"

#define BOARD_OSC0_HZ							16000000UL          //!< Osc0 frequency: Hz.
#define BOARD_OSC0_STARTUP_US					2000
#define BOARD_OSC0_IS_XTAL						true

#define BOARD_OSC32_HZ							32768
#define BOARD_OSC32_STARTUP_US					71000
#define BOARD_OSC32_IS_XTAL						true

/// @weakgroup sleepmgr_group
/// @{
#define SLEEPMGR_DEFAULT						SLEEPMGR_IDLE         ///< Default Sleep Mode
/// @}

/* 
 * RTC / System tick
 */
#define CONFIG_BSP_RTC_CLOCK_HZ                 BOARD_OSC32_HZ
#define CONFIG_BSP_RTC_TICK_HZ                  32					// must be CONFIG_BSP_RTC_CLOCK_HZ / 2^n

/*
 * Indicator LEDs
 */
#define BSP_LED_POLARITY						GPIO_INIT_HIGH		// LED On when High
#define BSP_INIT_ALIVE_LED						AVR32_PIN_PA12		// LEDs on J720
#define BSP_RTC_LED								AVR32_PIN_PA13		// RTC Blinks This
#define BSP_GPS_SYNC_LED                        AVR32_PIN_PA22		// GPS sync
#define BSP_LED4                                AVR32_PIN_PA23
#define BSP_LED5                                AVR32_PIN_PB21
#define BSP_LED6                                AVR32_PIN_PC11
#define BSP_LED7                                AVR32_PIN_PC20
#define BSP_LED8                                AVR32_PIN_PB03

/*
 * GPS USART
 */
#define GPS_USART               (&AVR32_USART0)
#define GPS_USART_RX_PIN        AVR32_USART0_RXD_PIN
#define GPS_USART_RX_FUNCTION   AVR32_USART0_RXD_FUNCTION
#define GPS_USART_TX_PIN        AVR32_USART0_TXD_PIN
#define GPS_USART_TX_FUNCTION   AVR32_USART0_TXD_FUNCTION
#define GPS_USART_IRQ           AVR32_USART0_IRQ
#define GPS_USART_BAUDRATE      9600
#define GPS_USART_CLOCK_MASK    AVR32_USART0_CLK_PBA
#define GPS_USART_RX_BUF_SIZE	512
#define GPS_USART_TX_BUF_SIZE	16

/*
 * GPS TimePulse
 */
#define GPS_TIMEPULSE_PIN		AVR32_PIN_PC19

/*
 * Accelerometer
 */
#if HARDWARE == HW_VANET_DAUGHTER_REVA
#define BSP_ENABLE_MPU_6050
#define BSP_ACCEL_INT_PIN	AVR32_PIN_PD03
#define BSP_ACCEL_I2C_ADDR	0x68
#endif

#if HARDWARE == HW_VANET_DAUGHTER_REVB
#define BSP_ENABLE_LIS3DSH
#define BSP_ACCEL_INT_PIN	AVR32_PIN_PD03
#define BSP_ACCEL_I2C_ADDR	0x1D
#endif

/*
 * Extra USART
 */
#define MISC_USART              (&AVR32_USART1)
#define MISC_USART_RX_PIN       AVR32_USART1_RXD_0_1_PIN
#define MISC_USART_RX_FUNCTION  AVR32_USART1_RXD_0_1_FUNCTION
#define MISC_USART_TX_PIN       AVR32_USART1_TXD_0_1_PIN
#define MISC_USART_TX_FUNCTION  AVR32_USART1_TXD_0_1_FUNCTION
#define MISC_USART_IRQ          AVR32_USART1_IRQ
#define MISC_USART_BAUDRATE     115200
#define MISC_USART_CLOCK_MASK   AVR32_USART1_CLK_PBC
#define MISC_USART_RX_BUF_SIZE	128
#define MISC_USART_TX_BUF_SIZE	128

/*
 * MUX USART
 */
#define MUX_USART               (&AVR32_USART2)
#define MUX_USART_RX_PIN        AVR32_USART2_RXD_0_1_PIN
#define MUX_USART_RX_FUNCTION   AVR32_USART2_RXD_0_1_FUNCTION
#define MUX_USART_TX_PIN        AVR32_USART2_TXD_0_1_PIN
#define MUX_USART_TX_FUNCTION   AVR32_USART2_TXD_0_1_FUNCTION
#define MUX_USART_IRQ           AVR32_USART2_IRQ
#define MUX_USART_BAUDRATE      115200
#define MUX_USART_CLOCK_MASK    AVR32_USART2_CLK_PBA
#define MUX_USART_RX_BUF_SIZE	512
#define MUX_USART_TX_BUF_SIZE	512

/*
 * Debug USART
 */
#define DBG_USART               (&AVR32_USART3)
#define DBG_USART_RX_PIN        AVR32_USART3_RXD_0_0_PIN
#define DBG_USART_RX_FUNCTION   AVR32_USART3_RXD_0_0_FUNCTION
#define DBG_USART_TX_PIN        AVR32_USART3_TXD_0_0_PIN
#define DBG_USART_TX_FUNCTION   AVR32_USART3_TXD_0_0_FUNCTION
#define DBG_USART_IRQ           AVR32_USART3_IRQ
#define DBG_USART_BAUDRATE      115200
#define DBG_USART_CLOCK_MASK    AVR32_USART3_CLK_PBA
#define DBG_USART_RX_BUF_SIZE   256		// commands are short
#define DBG_USART_TX_BUF_SIZE   1204	// output can be long

/*
 * Buzzer
 */
#define CONFIG_BSP_BUZZER_REP_TIMER             &AVR32_TC0
#define CONFIG_BSP_BUZZER_REP_CHANNEL           1
#define CONFIG_BSP_BUZZER_REP_IRQ               AVR32_TC0_IRQ1
#define CONFIG_BSP_BUZZER_REP_IRQ_PRI           AVR32_INTC_INT3

#define CONFIG_BSP_BUZZER_COUNT                 1

//                                                name,     externally driven,      timer,      channel
#define CONFIG_BSP_BUZZER0                      { "Survey", true,         { .ex = { &AVR32_TC0, 0 } } }
#define CONFIG_BSP_BUZZER0_GPIO_MAP \
    {AVR32_TC0_A0_PIN, AVR32_TC0_A0_FUNCTION}, \
    {AVR32_TC0_B0_PIN, AVR32_TC0_B0_FUNCTION},
#define BUZZER_SURVEY 0

// FAT config
#include "conf_access.h"
#include "conf_explorer.h"

// These defines are missing from or wrong in the toolchain header file ip_xxx.h or part.h
#	if UC3C
#		if !defined(AVR32_TWIM0_GROUP)
#			define AVR32_TWIM0_GROUP         25
#		else
#			warning "Duplicate define(s) to remove from the ASF"
#		endif
#		if !defined(AVR32_TWIM1_GROUP)
#			define AVR32_TWIM1_GROUP         26
#		else
#			warning "Duplicate define(s) to remove from the ASF"
#		endif
#		if !defined(AVR32_TWIM2_GROUP)
#			define AVR32_TWIM2_GROUP         45
#		else
#			warning "Duplicate define(s) to remove from the ASF"
#		endif
#	endif
#	define CONF_TWIM_IRQ_LINE          AVR32_TWIM0_IRQ
#	define CONF_TWIM_IRQ_GROUP         AVR32_TWIM0_GROUP
#	define CONF_TWIM_IRQ_LEVEL         1
#endif // HW_VANET_DAUGHTERBOARD_REV_A_H
