/**
 *	@file	intc_ext.h
 *
 *	@brief	Extensions to Atmel ASF INTC Interrupt Handler
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

#ifndef BSP_INTC_EXT_H
#define BSP_INTC_EXT_H

/**
 * @weakgroup intc_group
 *
 * Extensions to the INTC Software Driver API for AVR UC3
 *
 * @{
 */

/**
 * Type for GPIO interrupt handlers
 *
 * @warning     Your interrupt handlers should NOT use %__attribute__((__interrupt__))
 */
typedef void (*bsp_gpio_handler)(void);

/**
 * Register GPIO Interrupt Handler
 *
 * @param   handler - Your handler for this GPIO pin
 * @param   pin     - GPIO pin you are registering a handler on
 * @retval            true if registered OK
 * @retval            false if no room to register
 *
 * @todo    Figure out what to do when we return false (I'd like an assert?)
 * @todo    Actual implement the check for running out of space!
 *
 * @warning     Your interrupt handlers should NOT use %__attribute__((__interrupt__))
 */
bool INTC_register_GPIO_interrupt(bsp_gpio_handler handler, uint32_t pin);

/// @}

#endif // BSP_INTC_EXT_H