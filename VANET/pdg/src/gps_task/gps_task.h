/**
 *	@file	gps_task.h
 *
 *	@brief	GPS Handler Task
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

#ifndef GPS_TASK_H
#define GPS_TASK_H

/// Initialize / Start the GPS Task
extern void app_gps_task_init(void);

/// PDG command handler for GPS get state
extern void app_gps_cmd_get_state(uint8_t grp, uint8_t opcode, const uint8_t* payload, uint16_t payload_len);

#endif // GPS_TASK_H