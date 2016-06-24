/**
 *	@file	delay.h
 *
 *	@brief	Delay Routines
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

#ifndef _DELAY_H
#define _DELAY_H

/// Delay - Use the OS (non-blocking) if we can, otherwise busy loop (blocking)
void bsp_delay(uint16_t delay_ms);

#endif // _DELAY_H