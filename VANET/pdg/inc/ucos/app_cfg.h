/**
 *	@file	app_cfg.h
 *
 *	@brief	OS Task Definitions
 *
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

// System tasks listed in decreasing priority (i.e. 0 is max, OS_LOWEST_PRIO (63u) is lowest)

/*
 * OS Task: Timer
 */
#define  OS_TASK_TMR_PRIO					8u
// stack size is defined in os_cfg.h

/*
 * Termios Task
 */
#define		TASK_TERMIOS_PRIO				16u
#define		TASK_TERMIOS_STACK_SIZE			256u

/*
 * MUX Task
 */
#define		TASK_MUX_PRIO					24u
#define		TASK_MUX_STACK_SIZE			    256u

/*
 * Accelerometer Task
 */
#define		TASK_ACCEL_PRIO					32u
#define		TASK_ACCEL_STACK_SIZE			256u

/*
 * GPS Task
 */
#define		TASK_GPS_PRIO					40u
#define		TASK_GPS_STACK_SIZE				256u

/*
 * PDG Task
 */
#define		TASK_PDG_PRIO					44u
#define		TASK_PDG_STACK_SIZE				256u

/*
 * System Test Interface (STI) Task
 */
#define  CONFIG_BSP_TASK_STI_PRIO           48u
#define  CONFIG_BSP_TASK_STI_STACK_SIZE     384u

// this isn't included from lib_mem.c
#include "lib_cfg.h"
