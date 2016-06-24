/**
 *	@file	logcat.h
 *
 *	@brief	Logging Service
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

#ifndef LOGCAT_H
#define LOGCAT_H

extern void bsp_logcat_init(void);

extern void bsp_logcat_codeplug_ready(void);

extern void bsp_logcat_set_termios(uint8_t t);

extern void bsp_logcat_print(uint16_t mask, const char *msg);

extern void bsp_logcat_printf(uint16_t mask, const char *fmt, ...);

extern void bsp_logcat_dump(uint16_t mask, const uint8_t *addr, int len);

extern void bsp_logcat_start(uint16_t mask);

extern void bsp_logcat_reset_dump(void);

#define BSP_LOGCAT_NONE			0x0000		///< Don't log anything
#define BSP_LOGCAT_CRITICAL		0x0001		///< Global - Log Critical Conditions (something really bad)
#define BSP_LOGCAT_WARNING		0x0002		///< Global - Log Warning (non-urgent failures)
#define BSP_LOGCAT_INFO			0x0004		///< Global - Log Warnings (informational, reporiting, metrics, etc.)
#define BSP_LOGCAT_DEBUG		0x0008		///< Global - Log Notices (global level debug)
#define BSP_LOGCAT_NOISE		0x8000		///< Global - Just a bunch of noise

#define BSP_LOGCAT_ALL		    0xFFFF		///< Mask for all sources

#include "conf_logcat.h"

#endif // LOGCAT_H

