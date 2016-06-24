/**
 *	@file	conf_termios.h
 *
 *	@brief	Configure Ports Controlled by Termios
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
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#define BSP_TERMIOS_COUNT	4

#define BSP_TERMIOS_0_CFG	{ "Debug", DBG_USART, DBG_USART_BAUDRATE, DBG_USART_IRQ, \
	DBG_USART_RX_BUF_SIZE, DBG_USART_TX_BUF_SIZE, \
	BSP_TERMIOS_MODE_CANONICAL, true, BSP_TERMIOS_BUFFER_MIN, \
	DBG_USART_RX_PIN, DBG_USART_RX_FUNCTION, DBG_USART_TX_PIN, DBG_USART_TX_FUNCTION }

#define BSP_TERMIOS_1_CFG	{ "Extra", MISC_USART, MISC_USART_BAUDRATE, MISC_USART_IRQ, \
	MISC_USART_RX_BUF_SIZE, MISC_USART_TX_BUF_SIZE, \
	BSP_TERMIOS_MODE_CANONICAL, true, BSP_TERMIOS_BUFFER_MIN, \
	MISC_USART_RX_PIN, MISC_USART_RX_FUNCTION, MISC_USART_TX_PIN, MISC_USART_TX_FUNCTION }

#define BSP_TERMIOS_2_CFG	{ "GPS", GPS_USART, GPS_USART_BAUDRATE, GPS_USART_IRQ, \
	GPS_USART_RX_BUF_SIZE, GPS_USART_TX_BUF_SIZE, \
	BSP_TERMIOS_MODE_CANONICAL, false, BSP_TERMIOS_BUFFER_MIN, \
	GPS_USART_RX_PIN, GPS_USART_RX_FUNCTION, GPS_USART_TX_PIN, GPS_USART_TX_FUNCTION }

#define BSP_TERMIOS_3_CFG	{ "MUX", MUX_USART, MUX_USART_BAUDRATE, MUX_USART_IRQ, \
	MUX_USART_RX_BUF_SIZE, MUX_USART_TX_BUF_SIZE, \
	BSP_TERMIOS_MODE_RAW, false, BSP_TERMIOS_BUFFER_NONE, \
	MUX_USART_RX_PIN, MUX_USART_RX_FUNCTION, MUX_USART_TX_PIN, MUX_USART_TX_FUNCTION }
	