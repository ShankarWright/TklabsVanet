/**
 *	@file	reset.h
 *
 *	@brief	BSP Reset Handling
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

#ifndef _RESET_H
#define _RESET_H

#include <asf.h>
#include "vanet.h"
#include "ucos_ii.h"

/// BSP Reset Reasons via bsp_reset()
typedef enum
{
    // reset value of 0 is reserved since that is the powerup state of the SCIF GPLP registers
    
    BSP_RESET_SOFT =            0x0200,         ///< Intentional software reset, e.g., via PTI 'reset' command
    BSP_RESET_RTC_DOG =         0x0201,         ///< Intentional software reset, e.g., via PTI 'reset' command
    BSP_RESET_ALLOC_NULL =      0x0202,         ///< Attempt to allocation a 0-byte buffer
    BSP_RESET_ALLOC_FAIL =      0x0203,         ///< Out of memory buffers
    BSP_RESET_FREE_INVALID =    0x0204,         ///< Attempt to free an invalid pointer
    BSP_RESET_FREE_BOUNDARY =   0x0205,         ///< Free boundary check fail
    
    BSP_RESET_ROLLING_TEST =    0x0268,         ///< Test a rolling reset
    
    BSP_RESET_WATCHDOG =        0x0400,         ///< The watchdog timer reset us
    BSP_RESET_ROLLING =         0x0401,         ///< Rolling reset
    
    BSP_EXCEPTION_BASE =        0x8000,         ///< CPU exceptions begin here
} bsp_reset_reason_t;

/// Check reset conditions at powerup
extern void bsp_reset_init(void);

/// Print the full prompt shown at power up
extern void bsp_reset_show_init_prompt(void);

/// Report a software exception and (maybe) reset the board
extern void bsp_reset(bsp_reset_reason_t reason);

/// Perform a soft reset
static inline void bsp_soft_reset(void)
{
    bsp_reset(BSP_RESET_SOFT);
}

/// Report a hardware exception and (maybe) reset the board
extern void bsp_exception(uint16_t vector, uint32_t addr);

/// Stop everything but don't reset
extern void bsp_stop(void);

#endif // _RESET_H
