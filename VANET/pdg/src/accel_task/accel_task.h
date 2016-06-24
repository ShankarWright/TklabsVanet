/**
 *	@file	accel_task.h
 *
 *	@brief	Accelerometer Handler Task
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

#ifndef ACCEL_TASK_H
#define ACCEL_TASK_H

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
	int8_t t;
	uint8_t	r;
	uint16_t b;
	uint16_t th;
} app_accel_out_t;

/// Initialize / Start the Accelerometer Task
void app_accel_task_init(void);

/// Find & Initialize the Accelerometer Device
bool app_accel_init_dev(void);
void app_accel_exercise(void);

/// Basic movement detection using state machine
void app_accel_set_movement_threshold(uint16_t threshold);
uint16_t app_accel_get_movement_threshold(void);

/// Acknowledge a State Machine Event
void app_accel_ack(void);

/// Request (Poll) Current Accelerometer Data
void app_accel_query(app_accel_out_t *out);

/// Read/Write Register
uint8_t app_accel_read_register(uint8_t reg);
void app_accel_write_register(uint8_t reg, uint8_t value);

/// Get/Set Bandwidth
uint16_t app_accel_read_bandwidth(void);
void app_accel_write_bandwidth(uint16_t bandwidth);

/// Get/Set Range
uint8_t app_accel_read_range(void);
void app_accel_write_range(uint8_t range);

#endif // ACCEL_TASK_H