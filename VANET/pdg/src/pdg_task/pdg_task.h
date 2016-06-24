/**
 *	@file	pdg_task.c
 *
 *	@brief	Peripheral Data Gateway Task
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

#ifndef PDG_TASK_H
#define PDG_TASK_H

/// Initialize / Start the PDG Task
void app_pdg_task_init(void);

// Functions to make access to the VANET API easier
void app_pdg_accelerometer_event(void);
void app_pdg_button_event(uint8_t button, uint8_t state);

#endif // PDG_TASK_H