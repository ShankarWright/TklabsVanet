/**
 *	@file	init.c
 *
 *	@brief	PTI kernel board_init
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2013-14 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <asf.h>
#include "vanet.h"

void board_init(void)
{		
	// Initialize BSP
	bsp_init();
	
	// Initialize GPS Task
	app_gps_task_init();
	
	// Initialize Accelerometer Task
	app_accel_task_init();
	
	// Initialize PDG Task
	app_pdg_task_init();
}
