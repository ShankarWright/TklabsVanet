/**
 *	@file	alert.h
 *
 *	@brief	Alert tones
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of CWSI, LLC. Sunrise FL, USA
 *
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#ifndef BSP_ALERT_H
#define BSP_ALERT_H

/**
 * @defgroup group_bsp_drivers_alert Alert Tones Driver
 *
 * APIs to control alert tones
 *
 * @{
 */

/**  
 *  @name Driver Initialization
 *  @{
 */
/// Initialize Buzzer Driver
extern void bsp_alert_init(void);
/// @}

/**
 *  @name Public API
 *  @{
 */

/**
 * Start an alert tone
 *
 * @param   tone        Which alert tone sequence to play
*/
extern void bsp_alert(int tone);

/**
 * Stop alert tone
 */
extern void bsp_alert_stop(void);

/// @}

/**
 * @}
 */

#endif // BSP_BUZZER_H
