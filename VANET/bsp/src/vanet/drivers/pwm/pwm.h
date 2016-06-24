/**
 *	@file	pwm.h
 *
 *	@brief	PWM Driver
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

#ifndef BSP_PWM_H
#define BSP_PWM_H

/**
 * @defgroup group_bsp_drivers_pin PWM Driver
 *
 * PWM driver to make ASF easier to use
 *
 * @{
 */

/// Initialize PWM Driver
void bsp_pwm_init(void);

/// Enable PWM Channel
bool bsp_pwm_enable_channel(uint8_t channel, uint32_t frequency, uint8_t duty);

/// Change PWM Duty Cycle
bool bsp_pwm_update(uint8_t channel, uint8_t new_duty);

/// Disable PWM Channel
bool bsp_pwm_disable_channel(uint8_t channel);

/// @}

#endif // BSP_PWM_H