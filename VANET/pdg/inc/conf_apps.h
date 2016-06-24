/**
 *	@file	conf_apps.h
 *
 *	@brief	Common (single) header for Application Initialization
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

#ifndef CONF_APPS_H
#define CONF_APPS_H

#include "ucos_ii.h"

// GPS Task
#include "gps_task.h"

// Accelerometer Task
#include "accel_task.h"

// PDG Task
#include "pdg_task.h"

// Macros to check return codes
#define OS_SHOW_ERRORS
#define OS_SHOW_VALUES
#if defined OS_SHOW_ERRORS
  #define OS_CHECK_PERR(_perr, _loc)       if (_perr != OS_ERR_NONE) { print_dbg(_loc "="); print_dbg_int(_perr); print_dbg("\r\n"); }
  #define OS_CHECK_NULL(_ptr, _loc)        if (_ptr == NULL) { print_dbg(_loc "=NULL\r\n"); }
#elif defined OS_SHOW_VALUES
  #define OS_CHECK_PERR(_perr, _loc)       { print_dbg(_loc "="); print_dbg_int(_perr); print_dbg("\r\n"); }
  #define OS_CHECK_NULL(_ptr, _loc)        { print_dbg(_loc "="); print_dbg_hex((unsigned long)_ptr); print_dbg("\r\n"); }
#else
  #define OS_CHECK_PERR(_perr, _loc)
  #define OS_CHECK_NULL(_ptr, _loc)
#endif

#endif // CONF_APPS_H