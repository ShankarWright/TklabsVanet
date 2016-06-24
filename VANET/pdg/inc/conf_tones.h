/**
 *	@file	conf_tones.h
 *
 *	@brief	Configure Axamio Alert Tones
 *
 *  The intent of this configuration file is like the other ASF configuration
 *  files like conf_clock.h, etc.
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

#ifndef CONF_TONE_H_INCLUDED
#define CONF_TONE_H_INCLUDED

// First define a set of tones
enum
{
    BSP_ALERT_STARTUP = 0,          // Startup tone
    BSP_ALERT_CRASH,                // Crash
};

// Next for each tone, define the name, buzzer, sequence, and repetitions
#define BSP_ALERT_NUM_TONES         2

#define BSP_ALERT0_NAME             "Startup Sound"
#define BSP_ALERT0_BUZZER           BUZZER_SURVEY
#define BSP_ALERT0_SEQ              { 600, 200 }, { 900, 100 }, { 1200, 50 }
#define BSP_ALERT0_REP              1

#define BSP_ALERT1_NAME             "Crash"
#define BSP_ALERT1_BUZZER           BUZZER_SURVEY
#define BSP_ALERT1_SEQ              { 800, 50 }, { 0, 50 }
#define BSP_ALERT1_REP              3


#endif /* CONF_TONE_H_INCLUDED */