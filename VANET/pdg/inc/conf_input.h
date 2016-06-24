/**
 *	@file	conf_input.h
 *
 *	@brief	Configure Inputs (keypad, io events, etc.)
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

#ifndef CONF_INPUT_H_INCLUDED
#define CONF_INPUT_H_INCLUDED

/// @weakgroup group_bsp_drivers_pin
/// @{

/**
 * Enum for the IO Input Events
 *
 * @warning The enum order MUST match the BSP_PIN_n_CFG values.
 */
enum bsp_pin_events 
{
    BSP_PIN_EVENT_SW_PB02	= 0x0001,   ///< Push button on PB02
	BSP_PIN_EVENT_ACCEL		= 0x0002,	///< Accelerometer Event
};

#define BSP_PIN_COUNT   2       ///< Number of Pins on Board

//                                                                          active		active	pull up/
// Pin Config                 pin                   event                   time(ms)	level	down
#define BSP_PIN_0_CFG       { AVR32_PIN_PB02,		BSP_PIN_EVENT_SW_PB02,	250,		0,      true }
#define BSP_PIN_1_CFG		{ BSP_ACCEL_INT_PIN,	BSP_PIN_EVENT_ACCEL,	0,			1,		false }

/// @}   

#endif // CONF_INPUT_H_INCLUDED
