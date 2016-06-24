/**
 *	@file	dlib.h
 *
 *	@brief	The Doug C Library
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

#ifndef BSP_DLIB_H
#define BSP_DLIB_H

/**
 * @defgroup group_bsp_utils_dlib The Doug C Library
 *
 * Standard C library functions written by the illustrious Doug Lenz
 *
 * @{
 */

#include <avr32/io.h>
#include "compiler.h"
#include <stdarg.h>

/** @name Public API
 *
 *  @{
 */

/**
 * Print formatted output to buffer
 * 
 * @param str - output buffer
 * @param size - size of output buffer
 * @param fmt - output format
 * @param ap - variable argument list
 *
 */
extern int dlib_vsnprintf(char *str, size_t size, const char *fmt, va_list ap);

/**
 * Print formatted output to buffer
 * 
 * @param str - output buffer
 * @param size - size of output buffer
 * @param fmt - output format
 *
 */
static inline int dlib_snprintf(char *str, size_t size, const char *fmt, ...)
{
    va_list va;
    va_start(va,fmt);
    return dlib_vsnprintf(str,size,fmt,va);
}

/**
 * Convert long to string
 *
 * @param str - output string buffer
 * @param size - size of output buffer
 * @param num - number to convert to string
 * @param base - base of output (2=binary, 10=decimal, etc)
 */
extern char* dlib_long2string(char *str, size_t size, uint32_t num, int base);

/**
 * Convert string to long
 *
 * @param sptr - string to convert
 * @param eptr - remainder of non-converted string
 * @param base - base of number to convert
 */
extern long dlib_string2long(const char *sptr, char **eptr, int base);

/// @}

/// @}

#endif  // BSP_DLIB_H
