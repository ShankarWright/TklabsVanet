/**
 *	@file	tkvs_timer.h
 *
 *	@brief	TKVS Timer Routines
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

#ifndef BSP_TKVS_TMR_H
#define BSP_TKVS_TMR_H

// requires uC/OS!
#ifdef CONFIG_BSP_UCOS

/**
 * @weakgroup group_bsp_services_tkvs
 *
 * @{
 */

/**
 *  @name AVMS Timers
 *  @{
 *
 *  Example Usage:
 *  @code
 
     // initialize my timer
     bsp_tkvs_init_timer(&my_timer, true, 0, BSP_TKVS_SRC_CLOCK, BSP_CLOCK_EVENT_TIMER, BSP_TKVS_TIMER_MS(20000));
     
     // start the timer in your code
     bsp_tkvs_start_timer(&my_timer);
     
     // you can cancel a timer before it expires
     bsp_tkvs_stop_timer(&my_timer);
 
 *  @endcode
 */

#define TKVS_TIMER_SIGNATURE    0xC0D1F1ED

/// Source BSP_TKVS_SRC_CLOCK Events
enum
{
    BSP_CLOCK_EVENT_TICK1S = 0x0001,        ///< 1s Tick - @warning - This should NOT be used to keep time
    BSP_CLOCK_EVENT_UPDATE = 0x0002,        ///< Clock was updated
    BSP_CLOCK_EVENT_TIMER  = 0x0004,        ///< Task Timer Expired
};

/// Timer Types
enum
{
    BSP_TKVS_TIMER_PERIODIC,                ///< Periodic timer
    BSP_TKVS_TIMER_ONE_SHOT,                ///< One-Shot timer
};
    
/// TKVS Timer Control Block
typedef struct
{
    bool        active;                     ///< Caller should set this to false prior to bsp_tkvs_timer_start()
    uint8_t     task;                       ///< Caller should set this to the source/task to publish to
    uint8_t     id;                         ///< Caller can use this to identify timers - included in the TKVS immed_data
    uint8_t     type;                       ///< Caller should set timer type
    uint16_t    timeout;                    ///< Caller should set timeout / period
    uint16_t    event;                      ///< Caller should set the Event to Send
   
    // this is internal
    OS_TMR      *timer;                     ///< Actual OS Timer - Caller should not set
    uint32_t    signature;                  ///< Signature - set via bsp_tkvs_init_timer()
    uint16_t    previous_timeout;           ///< Previous Timeout - Allows us to reuse timers
    
} bsp_tkvs_timer_t;

/// Specify TKVS timer is milliseonds (ms) rather than OS TMR Ticks
#define BSP_TKVS_TIMER_MS_TO_TICKS(_ms)     ((_ms) * OS_TMR_CFG_TICKS_PER_SEC / 1000)

/// Convert a timer timeout (OS TMR Ticks) back to milliseconds (ms)
#define BSP_TKVS_TIMER_TICKS_TO_MS(_ticks)  (1000 * (_ticks) / OS_TMR_CFG_TICKS_PER_SEC)

/**
 *  Initialize TKVS Timer
 *
 *  Helper function so Application doesn't have to manually fill out bsp_tkvs_timer_t structure
 *
 *  @param timer    - Timer to initialize
 *  @param one_shot - True if a one-stop timer, False for a periodic timer
 *  @param id       - ID returned in the Immediate Data of the Event sent to task
 *  @param task     - Task to send Event to
 *  @param event    - Event to send to Taks
 *  @param timeout  - Timer timeout (Note: This is in OS Ticks! Use BSP_TKVS_TIMER_MS_TO_TICKS for milliseconds)
 */
void bsp_tkvs_init_timer(bsp_tkvs_timer_t *timer, uint8_t type, uint8_t id, uint8_t task, uint16_t event, uint16_t timeout);

/**
 *  Set TKVS Timer Timeout
 */
void bsp_tkvs_set_timer(bsp_tkvs_timer_t *timer, uint16_t timeout);

/**
 *  Get TKVS Timer Timeout
 */
uint16_t bsp_tkvs_get_timer(bsp_tkvs_timer_t *timer);

/**
 *  Start TKVS Timer
 *
 *  Allocates an OS Timer as needed then (re)starts timer
 */
void bsp_tkvs_start_timer(bsp_tkvs_timer_t *timer);

/**
 *  Stop TKVS Timer
 *
 *  @todo   Decide if this should auto free the timer or not
 */
void bsp_tkvs_stop_timer(bsp_tkvs_timer_t *timer);

/**
 *  Check if TKVS Timer is Active
 *
 */
static inline bool bsp_tkvs_is_timer_active(bsp_tkvs_timer_t *timer) 
{ 
    return timer->active; 
}
/// @

/// @

#endif // CONFIG_BSP_UCOS

#endif // BSP_TKVS_TMR_H