/**
 *	@file	sleepmgr_ext.h
 *
 *	@brief	Extensions to the Atmel Sleep Manager
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


#ifndef SLEEPMGR_EXT_H
#define SLEEPMGR_EXT_H

#include <sleepmgr.h>
extern uint8_t sleepmgr_locks[];

/**
 * @weakgroup sleepmgr_group
 *
 * Extensions to the Sleep manager
 *
 * @{
 */

/**
 * Sleep Subsystems
 *
 * These are parts of the BSP that can change the sleep behavior of the BSP.
 *
 * @warning This is a bitmask and MUST fit inside a single byte - so we only are allowed 8 sources.
 */
enum sleep_subsystems {
    SYS_DEFAULT = 0x01,     ///< Default Sleep Mode of Device
    SYS_DBG_CONS = 0x02,    ///< Sleep Mode When Debug Console is Used
    SYS_BUZZER = 0x04,      ///< Buzzer vote
    SYS_UNUSED08 = 0x08,    ///< Unused #2
    SYS_UNUSED10 = 0x10,    ///< Unused #3
    SYS_UNUSED20 = 0x20,    ///< Unused #4
    SYS_UNUSED40 = 0x40,    ///< Unused #5
    SYS_APP = 0x80          ///< Application Level Sleep Control
};

/**
 * Abstain (or unvote) Preferred Sleep Mode
 *
 * @param	subsystem_id	Sub-system voting for preferred sleep mode
 */
static inline void sleepmgr_abstain_preferred_sleep(enum sleep_subsystems subsystem_id)
{
		#ifdef CONFIG_SLEEPMGR_ENABLE

		// clear old vote
		for (int i=SLEEPMGR_ACTIVE; i<SLEEPMGR_NR_OF_MODES; i++)
		{
			sleepmgr_locks[i] &= (~subsystem_id);
		}
		
		#endif /* CONFIG_SLEEPMGR_ENABLE */
}

/**
 * Vote for Preferred Sleep State
 *
 * @param	subsystem_id	Sub-system voting for preferred sleep mode
 * @param	mode			Preferred sleep mode
 *
 * @warning	You must use the sleepmgr_mode enum and not the AVR32_PM_SMODE_* defines which are off
 *			in value by 1 (i.e. AVR32_PM_SMODE_* you need to add one)
 */
static inline void sleepmgr_vote_preferred_sleep(enum sleep_subsystems subsystem_id, enum sleepmgr_mode mode)
{
	#ifdef CONFIG_SLEEPMGR_ENABLE

	// clear old vote
	for (int i=SLEEPMGR_ACTIVE; i<SLEEPMGR_NR_OF_MODES; i++)
	{
		sleepmgr_locks[i] &= (~subsystem_id);
	}
	
	// save new vote
	sleepmgr_locks[mode] |= subsystem_id;
	#endif /* CONFIG_SLEEPMGR_ENABLE */
}

#ifdef CONFIG_SLEEPMGR_ENABLE
///> Textual Description of Sleep Modes Available
static const char *sleepmgr_ext_modes[] = {
	"0 ACTIVE   ", 
	"1 IDLE     ", 
	"2 FROZEN   ", 
	"3 STDBY    ", 
	"4 STOP     ", 
	"5 DEEPSTOP ", 
	"6 STATIC   "};
#endif // CONFIG_SLEEPMGR_ENABLE
	
/**
 * Show the current votes for sleep mode
 *
 * @note	When running it over the PTI console, this includes the sleep vote for the Debug Console Enable.
 *			One could argue it should not - just remember to mask out SYS_DBG_CONS in your head.
 */
static inline void sleepmgr_show_votes(void)
{
	#ifdef CONFIG_SLEEPMGR_ENABLE
	for (int i=SLEEPMGR_ACTIVE; i<SLEEPMGR_NR_OF_MODES; i++)
	{
		print_dbg(sleepmgr_ext_modes[i]);
		print_dbg_char_hex(sleepmgr_locks[i]);
		print_dbg("\r\n");
	}
	#endif /* CONFIG_SLEEPMGR_ENABLE */
}

static inline void bsp_sleep(void)
{
#ifdef SLEEP_STATUS_PIN
    #if SLEEP_STATUS_INVERT
    gpio_set_pin_high(SLEEP_STATUS_PIN);    // high when sleeping
    #else
    gpio_set_pin_low(SLEEP_STATUS_PIN);     // low when sleeping
    #endif
#endif // SLEEP_STATUS_PIN

    sleepmgr_enter_sleep();
	
#ifdef SLEEP_STATUS_PIN
    #if SLEEP_STATUS_INVERT
    gpio_set_pin_low(SLEEP_STATUS_PIN);     // low when awake
    #else
    gpio_set_pin_high(SLEEP_STATUS_PIN);    // high when awake
    #endif
#endif // SLEEP_STATUS_PIN
}
//! @}

#endif /* SLEEPMGR_EXT_H */