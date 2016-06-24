/**
 *	@file	vanet.h
 *
 *	@brief	Common (single) header for all the non-Atmel VANET BSP Files
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


#ifndef VANET_H
#define VANET_H

// Configuration
#include "conf_vanet.h"

// Version
#include "version.h"

// STI
#include "sti.h"

// Boot / Startup
#include "bsp_init.h"
#include "bsp_idle.h"

#ifdef CONFIG_BSP_UCOS
#include "conf_apps.h"
#endif

// Utilities
#include "circ.h"
#include "delay.h"
#include "dlib.h"
#include "fletcher.h"
#include "hackception.h"
#include "str_utils.h"
#include "ostracker.h"

// Drivers
#include "i2c_gpio.h"
#include "clcd.h"
#include "i2c_eeprom.h"
#include "pin.h"
#include "pwm.h"
#include "buzzer.h"
#include "alert.h"
#include "reset.h"

// Extensions
#include "intc_ext.h"
#include "print_funcs_ext.h"
#include "sleepmgr_ext.h"

// Services
#include "rtc.h"
#include "i2c.h"
#include "logcat.h"
#include "mux.h"
#include "termios.h"
#include "tkvs.h"
#include "tkvs_tmr.h"
#include "codeplug.h"
#include "buffers.h"

// Interrupt attribute depending on UCOS or not
#ifdef CONFIG_BSP_UCOS
#define BSP_INT_ATTR
#else
#define BSP_INT_ATTR __attribute__((__interrupt__))
#endif

#endif /* VANET_H */