/**
 *	@file	conf_logcat.h
 *
 *	@brief	Configure Logcat
 *
 *  The intent of this configuration file is like the other ASF configuration
 *  files like conf_clock.h, etc.
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

//      Log levels 0x0000 - 0x0008 are always defined in logcat.h
//      BSP_LOGCAT_NONE			0x0000		///< Don't log anything
//      BSP_LOGCAT_CRITICAL		0x0001		///< Global - Log Critical Conditions (something really bad)
//      BSP_LOGCAT_WARNING		0x0002		///< Global - Log Warning (non-urgent failures)
//      BSP_LOGCAT_INFO			0x0004		///< Global - Log Warnings (informational, reporiting, metrics, etc.)
//      BSP_LOGCAT_DEBUG		0x0008		///< Global - Log Notices (global level debug)
#define BSP_LOGCAT_MUX			0x0010		///< MUX Driver Logging
#define BSP_LOGCAT_CLCD			0x0020		///< CLCD Driver Logging
#define BSP_LOGCAT_ACCEL		0x0040		///< Accelerometer Logging
//      BSP_LOGCAT_NOISE		0x8000		///< Global - Just a bunch of noise

#define BSP_LOGCAT_DEFAULT		(BSP_LOGCAT_CRITICAL | BSP_LOGCAT_WARNING | BSP_LOGCAT_INFO)