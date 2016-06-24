/**
 *	@file	rtc.h
 *
 *	@brief	Real-time Clock Driver
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

#ifndef _RTC_H
#define _RTC_H

/**
 * @defgroup group_bsp_axamio_services_rtc RTC - Real-Time Clock
 *
 * @{
 */

#include <avr32/io.h>
#include "compiler.h"

/** @name RTC
 *
 *  Manage the clock and provide system tick via the AST
 *
 *  @{
 */

/** Because calendar.h didn't do it */
typedef struct calendar_date calendar_date_t;

/// Initialize RTC
extern void bsp_rtc_init(void);

/// The number of seconds since 1970-01-01 00:00:00 UTC
extern uint32_t bsp_rtc_get_clock(void);

/// The microsecond part of the clock
extern uint32_t bsp_rtc_get_clock_us(void);

/// Set the number of seconds since 1970-01-01 00:00:00 UTC
extern void bsp_rtc_set_clock(uint32_t val);

/// The number of milliseconds since powerup
extern uint32_t bsp_rtc_get_uptime(void);

/// The number of ticks since powerup
extern uint32_t bsp_rtc_get_ticks(void);

/// Grab the current calendar
static inline void bsp_rtc_get_calendar(calendar_date_t* date_out)
{
    return calendar_timestamp_to_date(bsp_rtc_get_clock(),date_out);
}

/// Set the clock using a calendar
static inline void bsp_rtc_set_calendar(calendar_date_t* date)
{
    return bsp_rtc_set_clock(calendar_date_to_timestamp(date));
}

/// Kick the RTC psuedo-watchdog
extern void bsp_rtc_idle_kick(void);

/// @}

/// @}

#endif  // _RTC_H
