/**
 *	@file	fletcher.h
 *
 *	@brief	Fletcher's Checksum
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

/// Compute a 16-bit Fletchers checksum
extern uint16_t bsp_util_fletcher16(const uint8_t* buffer, int buffer_length);

/// Data for a running fletchers checksum
typedef struct
{
	uint16_t sum1;
	uint16_t sum2;
	uint8_t count;
} bsp_running_fletcher;

/// Start a running fletcher's checksum
extern void bsp_util_running_xsum_init(bsp_running_fletcher* r);

/// Add data to a running fletcher's checksum
extern void bsp_util_running_xsum_addb(bsp_running_fletcher* r, uint8_t b);

/// Add data to a running fletcher's checksum
extern void bsp_util_running_xsum_add(bsp_running_fletcher* r, const uint8_t* b, int len);

/// Get a running fletcher's checksum result
extern uint16_t bsp_util_running_xsum_result(bsp_running_fletcher* r);