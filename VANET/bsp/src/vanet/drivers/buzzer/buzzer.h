/**
 *	@file	buzzer.h
 *
 *	@brief	Piezo Buzzer Driver
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

#ifndef BSP_BUZZER_H
#define BSP_BUZZER_H

/**
 * @defgroup group_bsp_drivers_buzzer Piezo Buzzer Driver
 *
 * APIs to control the piezo buzzer
 *
 * @{
 */

/**  
 *  @name Driver Initialization
 *  @{
 */
/// Initialize Buzzer Driver
extern void bsp_buzzer_init(void);
/// @}

/**
 *  @name Public API
 *  @{
 */

/// Definition of a buzzer state
typedef struct
{
    uint16_t freq;      //< Buzz frequency in Hz.  0 for buzzer off
    uint16_t ms;        //< State duration in milliseconds.  0 for indefinite
} bsp_buzzer_state_t;

/**
 * Enable the buzzer
 *
 * @param   buzzer      Which buzzer to play this sequence on
 * @param   states      The sequence of buzzer states
 * @param   state_count The number of states in the sequence
 * @param   reps        Number repetition of the sequence.  Set to -1 for indefinite repetition
*/
extern void bsp_buzzer_start(int buzzer, const bsp_buzzer_state_t* states, int state_count, int reps);

/**
 * Enable the buzzer
 *
 * @param   buzzer      Which buzzer to play this sequence on
 * @param   hz          Frequency to buzz in Hz
 * @param   on_time     Time to keep the buzzer on in milliseconds
 * @param   off_time    Time to keep the buzzer off in milliseconds
 * @param   reps        Number of times to repeat the on/off sequence
*/
extern void bsp_buzz_periodic(int buzzer, uint16_t hz, uint16_t on_time, uint16_t off_time, int reps);

/**
 * Enable the buzzer
 *
 * @param   buzzer      Which buzzer to play this sequence on
 * @param   hz          Frequency to buzz in Hz
 * @param   duration    Time to keep the buzzer active in milliseconds
*/
static inline void bsp_buzz(int buzzer, uint16_t hz, uint16_t duration)
{
    bsp_buzz_periodic(buzzer,hz,duration,0,1);
}

/**
 * Stop the buzzer
 */
extern void bsp_buzzer_stop(void);

/// @}

/**
 * @}
 */

#endif // BSP_BUZZER_H